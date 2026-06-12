#include "bagfile_parser_qt/mat_writer.hpp"
#include <matio.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>

// Mat_VarCreateStruct was deprecated in libmatio 1.5.28 in favor of Mat_VarCreateStruct2.
// Use the new API where available; fall back for older distros (e.g. Ubuntu 22.04 ships 1.5.21).
static matvar_t* createStructVar(const char* name, int rank, const size_t* dims)
{
#if defined(MATIO_RELEASE_LEVEL) && MATIO_RELEASE_LEVEL >= 28
    return Mat_VarCreateStruct2(name, rank, dims, nullptr);
#else
    return Mat_VarCreateStruct(name, rank, dims, nullptr, 0);
#endif
}

CollectedData* CollectedData::child(const std::string& name)
{
    for (std::pair<std::string, CollectedData>& entry : this->children)
    {
        if (entry.first == name)
        {
            return &entry.second;
        }
    }
    return nullptr;
}

static FieldDesc makeElemDesc(const FieldDesc& field)
{
    FieldDesc elem_desc;
    elem_desc.is_primitive_type = false;
    elem_desc.fields = field.fields;
    return elem_desc;
}

void allocateCollectors(CollectedData& data, const FieldDesc& desc, size_t n)
{
    data.kind = CollectedData::STRUCT;

    for (const FieldDesc& field : desc.fields)
    {
        CollectedData child_data;

        if (field.padded_max > 0 && !field.is_primitive_type)
        {
            child_data.kind = CollectedData::STRUCT;
            FieldDesc elem_desc = makeElemDesc(field);
            for (int i = 0; i < field.padded_max; i++)
            {
                CollectedData elem;
                allocateCollectors(elem, elem_desc, n);
                child_data.children.emplace_back("e" + std::to_string(i), std::move(elem));
            }
            data.children.emplace_back(field.name, std::move(child_data));
            continue;
        }

        if (field.skip)
        {
            child_data.kind = CollectedData::SKIPPED;
        }
        else if (field.is_primitive_type)
        {
            bool is_str = (field.primitive == PrimitiveType::STRING ||
                           field.primitive == PrimitiveType::WSTRING);
            if (is_str)
            {
                if (field.array_size == 0)
                {
                    child_data.kind = CollectedData::STRING;
                    child_data.strings.resize(n);
                }
                else
                {
                    child_data.kind = CollectedData::STRING_ARRAY;
                    child_data.string_arrays.resize(n);
                }
            }
            else if (field.array_size == 0)
            {
                child_data.kind = CollectedData::SCALAR;
                child_data.values.resize(n);
            }
            else if (field.array_size > 0)
            {
                child_data.kind = CollectedData::FIXED_ARRAY;
                child_data.width = field.array_size;
                child_data.values.resize(n * field.array_size);
            }
            else
            {
                child_data.kind = CollectedData::DYN_ARRAY;
                child_data.dyn_arrays.resize(n);
            }
        }
        else
        {
            allocateCollectors(child_data, field, n);
        }

        data.children.emplace_back(field.name, std::move(child_data));
    }
}

static void drainPrimitive(PrimitiveType p, CdrReader& reader)
{
    if (p == PrimitiveType::STRING || p == PrimitiveType::WSTRING) reader.readString();
    else reader.readAsDouble(p);
}

static void nanFillMessage(CollectedData& data, const FieldDesc& desc, size_t idx);

static void nanFillField(CollectedData& child, const FieldDesc& field, size_t idx)
{
    if (field.padded_max > 0 && !field.is_primitive_type)
    {
        FieldDesc elem_desc = makeElemDesc(field);
        for (int i = 0; i < field.padded_max; i++)
        {
            nanFillMessage(child.children[i].second, elem_desc, idx);
        }
        return;
    }
    if (child.kind == CollectedData::SKIPPED) return;
    if (field.is_primitive_type)
    {
        bool is_str = (field.primitive == PrimitiveType::STRING || field.primitive == PrimitiveType::WSTRING);
        if (is_str)
        {
            if (field.array_size == 0 && idx < child.strings.size()) child.strings[idx] = "";
        }
        else if (field.array_size == 0)
        {
            if (idx < child.values.size()) child.values[idx] = std::nan("");
        }
        else if (field.array_size > 0)
        {
            for (int i = 0; i < field.array_size; i++)
            {
                size_t pos = idx * static_cast<size_t>(field.array_size) + i;
                if (pos < child.values.size()) child.values[pos] = std::nan("");
            }
        }
    }
    else
    {
        nanFillMessage(child, field, idx);
    }
}

static void nanFillMessage(CollectedData& data, const FieldDesc& desc, size_t idx)
{
    for (size_t f = 0; f < desc.fields.size() && f < data.children.size(); f++)
    {
        nanFillField(data.children[f].second, desc.fields[f], idx);
    }
}

static void drainField(const FieldDesc& field, CdrReader& reader)
{
    if (field.is_primitive_type)
    {
        if (field.array_size == 0)
        {
            drainPrimitive(field.primitive, reader);
        }
        else if (field.array_size > 0)
        {
            for (int i = 0; i < field.array_size; i++) drainPrimitive(field.primitive, reader);
        }
        else
        {
            uint32_t count = reader.readSequenceLength();
            PrimitiveType p = field.primitive;
            if (p == PrimitiveType::UINT8 || p == PrimitiveType::BYTE || p == PrimitiveType::INT8 || p == PrimitiveType::BOOL)
            {
                reader.skipBytes(count);
            }
            else
            {
                for (uint32_t i = 0; i < count; i++) drainPrimitive(p, reader);
            }
        }
    }
    else
    {
        size_t count = 1;
        if (field.array_size > 0) count = static_cast<size_t>(field.array_size);
        else if (field.array_size < 0) count = reader.readSequenceLength();
        for (size_t i = 0; i < count; i++)
        {
            for (const FieldDesc& sub : field.fields) drainField(sub, reader);
        }
    }
}

void fillMessage(CollectedData& data, const FieldDesc& desc, CdrReader& reader, size_t idx)
{
    for (size_t f = 0; f < desc.fields.size(); f++)
    {
        const FieldDesc& field = desc.fields[f];
        CollectedData& child = data.children[f].second;

        if (field.padded_max > 0 && !field.is_primitive_type)
        {
            uint32_t count = reader.readSequenceLength();
            FieldDesc elem_desc = makeElemDesc(field);
            uint32_t kept = std::min(count, static_cast<uint32_t>(field.padded_max));
            for (uint32_t i = 0; i < kept; i++)
            {
                fillMessage(child.children[i].second, elem_desc, reader, idx);
            }
            for (uint32_t i = kept; i < count; i++)
            {
                for (const FieldDesc& sub : field.fields) drainField(sub, reader);
            }
            for (int i = static_cast<int>(kept); i < field.padded_max; i++)
            {
                nanFillMessage(child.children[i].second, elem_desc, idx);
            }
            continue;
        }

        if (field.skip)
        {
            drainField(field, reader);
            continue;
        }

        if (field.is_primitive_type)
        {
            bool is_str = (field.primitive == PrimitiveType::STRING ||
                           field.primitive == PrimitiveType::WSTRING);

            if (is_str)
            {
                if (field.array_size == 0)
                {
                    child.strings[idx] = reader.readString();
                }
                else
                {
                    uint32_t count = (field.array_size > 0)
                        ? static_cast<uint32_t>(field.array_size)
                        : reader.readSequenceLength();
                    child.string_arrays[idx].resize(count);
                    for (uint32_t i = 0; i < count; i++)
                    {
                        child.string_arrays[idx][i] = reader.readString();
                    }
                }
            }
            else if (field.array_size == 0)
            {
                child.values[idx] = reader.readAsDouble(field.primitive);
            }
            else if (field.array_size > 0)
            {
                for (int i = 0; i < field.array_size; i++)
                {
                    child.values[idx * field.array_size + i] =
                        reader.readAsDouble(field.primitive);
                }
            }
            else
            {
                uint32_t count = reader.readSequenceLength();
                child.dyn_arrays[idx].resize(count);
                for (uint32_t i = 0; i < count; i++)
                {
                    child.dyn_arrays[idx][i] = reader.readAsDouble(field.primitive);
                }
            }
        }
        else
        {
            fillMessage(child, field, reader, idx);
        }
    }
}

std::string sanitizeName(const std::string& topic)
{
    std::string name;
    for (char c : topic)
    {
        if (c == '/')
        {
            if (!name.empty())
            {
                name += '_';
            }
        }
        else if (std::isalnum(static_cast<unsigned char>(c)) || c == '_')
        {
            name += c;
        }
        else
        {
            name += '_';
        }
    }
    if (name.empty())
    {
        name = "unnamed";
    }
    if (std::isdigit(static_cast<unsigned char>(name[0])))
    {
        name = "t_" + name;
    }
    if (name.size() > 63)
    {
        name = name.substr(0, 63);
    }
    return name;
}

// --- matio helpers ---

static matvar_t* collectedToMatvar(const std::string& name, const CollectedData& data);

static matvar_t* makeDoubleArray(const std::string& name, const double* vals, size_t rows, size_t cols)
{
    size_t dims[2] = {rows, cols};
    std::vector<double> col_major(rows * cols);
    for (size_t r = 0; r < rows; r++)
    {
        for (size_t c = 0; c < cols; c++)
        {
            col_major[c * rows + r] = vals[r * cols + c];
        }
    }
    return Mat_VarCreate(name.c_str(), MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, col_major.data(), 0);
}

static matvar_t* makeScalarArray(const std::string& name, const std::vector<double>& vals)
{
    size_t dims[2] = {1, vals.size()};
    return Mat_VarCreate(name.c_str(), MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, const_cast<double*>(vals.data()), 0);
}

static matvar_t* makeStringCell(const std::string& name, const std::vector<std::string>& strs)
{
    size_t dims[2] = {1, strs.size()};
    matvar_t* cell = Mat_VarCreate(name.c_str(), MAT_C_CELL, MAT_T_CELL, 2, dims, nullptr, 0);
    for (size_t i = 0; i < strs.size(); i++)
    {
        size_t sdims[2] = {1, strs[i].size()};
        matvar_t* s = Mat_VarCreate(nullptr, MAT_C_CHAR, MAT_T_UTF8, 2, sdims, const_cast<char*>(strs[i].c_str()), 0);
        Mat_VarSetCell(cell, static_cast<int>(i), s);
    }
    return cell;
}

static matvar_t* collectedToMatvar(const std::string& name, const CollectedData& data)
{
    switch (data.kind)
    {
        case CollectedData::SKIPPED:
            return nullptr;

        case CollectedData::SCALAR:
            return makeScalarArray(name, data.values);

        case CollectedData::FIXED_ARRAY:
        {
            size_t n = data.values.size() / data.width;
            return makeDoubleArray(name, data.values.data(), n, data.width);
        }

        case CollectedData::DYN_ARRAY:
        {
            if (data.dyn_arrays.empty())
            {
                size_t dims[2] = {0, 0};
                return Mat_VarCreate(name.c_str(), MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, nullptr, 0);
            }
            size_t max_width = 0;
            for (const std::vector<double>& a : data.dyn_arrays)
            {
                max_width = std::max(max_width, a.size());
            }
            size_t n = data.dyn_arrays.size();
            std::vector<double> padded(n * max_width, std::nan(""));
            for (size_t i = 0; i < n; i++)
            {
                for (size_t j = 0; j < data.dyn_arrays[i].size(); j++)
                {
                    padded[i * max_width + j] = data.dyn_arrays[i][j];
                }
            }
            return makeDoubleArray(name, padded.data(), n, max_width);
        }

        case CollectedData::STRING:
            return makeStringCell(name, data.strings);

        case CollectedData::STRING_ARRAY:
        {
            size_t dims[2] = {1, data.string_arrays.size()};
            matvar_t* cell = Mat_VarCreate(name.c_str(), MAT_C_CELL, MAT_T_CELL, 2, dims, nullptr, 0);
            for (size_t i = 0; i < data.string_arrays.size(); i++)
            {
                matvar_t* inner = makeStringCell("", data.string_arrays[i]);
                Mat_VarSetCell(cell, static_cast<int>(i), inner);
            }
            return cell;
        }

        case CollectedData::STRUCT:
        {
            std::vector<std::pair<std::string, matvar_t*>> built;
            for (const std::pair<std::string, CollectedData>& entry : data.children)
            {
                if (entry.second.kind == CollectedData::SKIPPED) continue;
                matvar_t* field_var = collectedToMatvar(entry.first, entry.second);
                if (field_var)
                {
                    built.emplace_back(entry.first, field_var);
                }
            }
            if (built.empty())
            {
                return nullptr;
            }

            size_t dims[2] = {1, 1};
            matvar_t* s = createStructVar(name.c_str(), 2, dims);
            for (const std::pair<std::string, matvar_t*>& fv : built)
            {
                Mat_VarAddStructField(s, fv.first.c_str());
                Mat_VarSetStructFieldByName(s, fv.first.c_str(), 0, fv.second);
            }
            return s;
        }
    }
    return nullptr;
}

void writeMat(const std::string& filepath, const std::string& var_name, const CollectedData& data, const std::vector<double>& timestamps)
{
    mat_t* matfp = Mat_CreateVer(filepath.c_str(), nullptr, MAT_FT_MAT5);
    if (!matfp)
    {
        std::cerr << "Error: cannot create " << filepath << "\n";
        return;
    }

    size_t dims[2] = {1, 1};
    matvar_t* root = createStructVar(var_name.c_str(), 2, dims);

    Mat_VarAddStructField(root, "t");
    matvar_t* t_var = makeScalarArray("t", timestamps);
    Mat_VarSetStructFieldByName(root, "t", 0, t_var);

    for (const std::pair<std::string, CollectedData>& entry : data.children)
    {
        if (entry.second.kind == CollectedData::SKIPPED) continue;
        matvar_t* field_var = collectedToMatvar(entry.first, entry.second);
        if (field_var)
        {
            Mat_VarAddStructField(root, entry.first.c_str());
            Mat_VarSetStructFieldByName(root, entry.first.c_str(), 0, field_var);
        }
    }

    Mat_VarWrite(matfp, root, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(root);
    Mat_Close(matfp);
}

// --- Stats pass ---

static void scanField(const FieldDesc& field, CdrReader& reader, std::unordered_map<std::string, uint32_t>& stats, const std::string& path)
{
    if (field.is_primitive_type)
    {
        if (field.array_size == 0)
        {
            drainPrimitive(field.primitive, reader);
        }
        else if (field.array_size > 0)
        {
            for (int i = 0; i < field.array_size; i++) drainPrimitive(field.primitive, reader);
        }
        else
        {
            uint32_t count = reader.readSequenceLength();
            uint32_t& cur = stats[path];
            if (count > cur) cur = count;
            PrimitiveType p = field.primitive;
            if (p == PrimitiveType::UINT8 || p == PrimitiveType::BYTE || p == PrimitiveType::INT8 || p == PrimitiveType::BOOL)
            {
                reader.skipBytes(count);
            }
            else
            {
                for (uint32_t i = 0; i < count; i++) drainPrimitive(p, reader);
            }
        }
    }
    else
    {
        size_t count = 1;
        if (field.array_size > 0)
        {
            count = static_cast<size_t>(field.array_size);
        }
        else if (field.array_size < 0)
        {
            uint32_t c = reader.readSequenceLength();
            uint32_t& cur = stats[path];
            if (c > cur) cur = c;
            count = c;
        }
        for (size_t i = 0; i < count; i++)
        {
            for (const FieldDesc& sub : field.fields)
            {
                std::string subpath = path.empty() ? sub.name : path + "." + sub.name;
                scanField(sub, reader, stats, subpath);
            }
        }
    }
}

void scanMessage(const FieldDesc& root_desc, CdrReader& reader, std::unordered_map<std::string, uint32_t>& stats)
{
    for (const FieldDesc& field : root_desc.fields)
    {
        scanField(field, reader, stats, field.name);
    }
}

// --- Schema adjustment based on stats ---

static bool isByteP(PrimitiveType p)
{
    return p == PrimitiveType::UINT8 || p == PrimitiveType::BYTE || p == PrimitiveType::INT8;
}

static const int kMaxPadDepth = 3;
static const uint64_t kMaxPadElems = 2000000;

static FieldDesc adjustField(const FieldDesc& orig, const std::unordered_map<std::string, uint32_t>& stats, int byte_threshold, int msg_threshold, const std::string& path, int pad_depth, uint64_t mult, size_t msg_count)
{
    FieldDesc result = orig;
    std::unordered_map<std::string, uint32_t>::const_iterator it = stats.find(path);
    uint32_t max_count = (it != stats.end()) ? it->second : 0;

    int child_pad_depth = pad_depth;
    uint64_t child_mult = mult;

    if (orig.skip && orig.is_primitive_type && orig.array_size < 0 && isByteP(orig.primitive))
    {
        if (pad_depth == 0 && max_count <= static_cast<uint32_t>(byte_threshold))
        {
            result.skip = false;
        }
    }
    else if (orig.skip && !orig.is_primitive_type && orig.array_size < 0)
    {
        uint64_t instances = static_cast<uint64_t>(msg_count) * mult * max_count;
        if (pad_depth < kMaxPadDepth && max_count > 0 &&
            max_count <= static_cast<uint32_t>(msg_threshold) &&
            instances <= kMaxPadElems)
        {
            result.skip = false;
            result.padded_max = static_cast<int>(max_count);
            child_pad_depth = pad_depth + 1;
            child_mult = mult * max_count;
        }
    }

    if (!result.skip && !orig.is_primitive_type)
    {
        result.fields.clear();
        for (const FieldDesc& sub : orig.fields)
        {
            std::string subpath = path.empty() ? sub.name : path + "." + sub.name;
            result.fields.push_back(adjustField(sub, stats, byte_threshold, msg_threshold, subpath, child_pad_depth, child_mult, msg_count));
        }
    }

    return result;
}

FieldDesc adjustSchema(const FieldDesc& root_desc, const std::unordered_map<std::string, uint32_t>& stats, int byte_threshold, int msg_threshold, size_t msg_count)
{
    FieldDesc result = root_desc;
    result.fields.clear();
    for (const FieldDesc& sub : root_desc.fields)
    {
        result.fields.push_back(adjustField(sub, stats, byte_threshold, msg_threshold, sub.name, 0, 1, msg_count));
    }
    return result;
}
