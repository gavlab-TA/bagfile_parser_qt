#include "bagfile_parser_qt/schema_parser.hpp"
#include <sstream>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <functional>

static std::string trim(const std::string& s)
{
    std::string::size_type start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }
    std::string::size_type end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::vector<std::string> splitWhitespace(const std::string& s)
{
    std::vector<std::string> parts;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token)
    {
        parts.push_back(token);
    }
    return parts;
}

static bool isSeparator(const std::string& line)
{
    std::string t = trim(line);
    return t.size() >= 3 && t.find_first_not_of('=') == std::string::npos;
}

bool isPrimitive(const std::string& type_str)
{
    static const char* prims[] = {
        "bool", "int8", "uint8", "byte", "char",
        "int16", "uint16", "int32", "uint32",
        "int64", "uint64", "float32", "float64",
        "string", "wstring"
    };
    for (const char* p : prims)
    {
        if (type_str == p)
        {
            return true;
        }
    }
    return false;
}

PrimitiveType toPrimitive(const std::string& type_str)
{
    if (type_str == "bool")    return PrimitiveType::BOOL;
    if (type_str == "int8")    return PrimitiveType::INT8;
    if (type_str == "uint8")   return PrimitiveType::UINT8;
    if (type_str == "byte")    return PrimitiveType::BYTE;
    if (type_str == "char")    return PrimitiveType::CHAR;
    if (type_str == "int16")   return PrimitiveType::INT16;
    if (type_str == "uint16")  return PrimitiveType::UINT16;
    if (type_str == "int32")   return PrimitiveType::INT32;
    if (type_str == "uint32")  return PrimitiveType::UINT32;
    if (type_str == "int64")   return PrimitiveType::INT64;
    if (type_str == "uint64")  return PrimitiveType::UINT64;
    if (type_str == "float32") return PrimitiveType::FLOAT32;
    if (type_str == "float64") return PrimitiveType::FLOAT64;
    if (type_str == "string")  return PrimitiveType::STRING;
    if (type_str == "wstring") return PrimitiveType::WSTRING;
    throw std::runtime_error("Unknown primitive type: " + type_str);
}

size_t primitiveCdrAlign(PrimitiveType p)
{
    switch (p)
    {
        case PrimitiveType::BOOL:
        case PrimitiveType::INT8:
        case PrimitiveType::UINT8:
        case PrimitiveType::BYTE:
        case PrimitiveType::CHAR:    return 1;
        case PrimitiveType::INT16:
        case PrimitiveType::UINT16:  return 2;
        case PrimitiveType::INT32:
        case PrimitiveType::UINT32:
        case PrimitiveType::FLOAT32:
        case PrimitiveType::STRING:
        case PrimitiveType::WSTRING: return 4;
        case PrimitiveType::INT64:
        case PrimitiveType::UINT64:
        case PrimitiveType::FLOAT64: return 8;
    }
    return 1;
}

struct RawField
{
    std::string type_str;
    std::string name;
    int array_size = 0;
};

struct RawMsgDef
{
    std::string type_name;
    std::vector<RawField> fields;
};

static std::string normalizeType(const std::string& type_str)
{
    if (type_str.find("/msg/") != std::string::npos)
    {
        return type_str;
    }
    std::string::size_type slash = type_str.find('/');
    if (slash != std::string::npos)
    {
        return type_str.substr(0, slash) + "/msg/" + type_str.substr(slash + 1);
    }
    return type_str;
}

static std::vector<RawField> parseFields(const std::string& definition)
{
    std::vector<RawField> fields;
    std::istringstream stream(definition);
    std::string line;

    while (std::getline(stream, line))
    {
        std::string::size_type comment = line.find('#');
        if (comment != std::string::npos)
        {
            line = line.substr(0, comment);
        }
        line = trim(line);
        if (line.empty())
        {
            continue;
        }
        if (line.find('=') != std::string::npos)
        {
            continue;
        }

        std::vector<std::string> parts = splitWhitespace(line);
        if (parts.size() < 2)
        {
            continue;
        }

        RawField f;
        std::string type = parts[0];
        f.name = parts[1];

        std::string::size_type le = type.find("<=");
        if (le != std::string::npos && type.find('[') == std::string::npos)
        {
            type = type.substr(0, le);
        }

        std::string::size_type bracket = type.find('[');
        if (bracket != std::string::npos)
        {
            std::string::size_type close = type.find(']');
            std::string size_str = type.substr(bracket + 1, close - bracket - 1);
            if (size_str.empty() || size_str.find("<=") != std::string::npos)
            {
                f.array_size = -1;
            }
            else
            {
                f.array_size = std::stoi(size_str);
            }
            type = type.substr(0, bracket);
        }

        le = type.find("<=");
        if (le != std::string::npos)
        {
            type = type.substr(0, le);
        }

        f.type_str = type;
        fields.push_back(std::move(f));
    }

    return fields;
}

static std::map<std::string, RawMsgDef> splitDefinitions(const std::string& schema_text, const std::string& main_type)
{
    std::map<std::string, RawMsgDef> defs;
    std::istringstream stream(schema_text);
    std::string line;
    std::string current_type = main_type;
    std::string current_def;

    std::function<void()> flush = [&]()
    {
        if (!current_type.empty())
        {
            RawMsgDef def;
            def.type_name = current_type;
            def.fields = parseFields(current_def);
            defs[current_type] = std::move(def);
        }
    };

    while (std::getline(stream, line))
    {
        if (isSeparator(line))
        {
            flush();
            current_def.clear();
            current_type.clear();
            continue;
        }

        std::string trimmed = trim(line);
        if (trimmed.substr(0, 5) == "MSG: ")
        {
            current_type = normalizeType(trim(trimmed.substr(5)));
            current_def.clear();
            continue;
        }

        if (current_type.empty() && !trimmed.empty())
        {
            current_type = main_type;
        }

        current_def += line + "\n";
    }

    flush();
    return defs;
}

static FieldDesc resolveType(const std::map<std::string, RawMsgDef>& defs, const std::string& type_name)
{
    FieldDesc desc;

    if (isPrimitive(type_name))
    {
        desc.is_primitive_type = true;
        desc.primitive = toPrimitive(type_name);
        return desc;
    }

    std::string normalized = normalizeType(type_name);

    std::map<std::string, RawMsgDef>::const_iterator it = defs.find(normalized);
    if (it == defs.end())
    {
        for (const std::pair<const std::string, RawMsgDef>& kv : defs)
        {
            const std::string& key = kv.first;
            std::string::size_type last_slash = key.rfind('/');
            if (last_slash != std::string::npos && key.substr(last_slash + 1) == type_name)
            {
                it = defs.find(key);
                break;
            }
        }
    }

    if (it == defs.end())
    {
        std::cerr << "Warning: unknown type '" << type_name << "', skipping\n";
        return desc;
    }

    desc.is_primitive_type = false;
    for (const RawField& raw : it->second.fields)
    {
        FieldDesc child = resolveType(defs, raw.type_str);
        child.name = raw.name;
        child.array_size = raw.array_size;
        desc.fields.push_back(std::move(child));
    }

    return desc;
}

static bool isBytePrimitive(PrimitiveType p)
{
    return p == PrimitiveType::UINT8 || p == PrimitiveType::BYTE || p == PrimitiveType::INT8;
}

static FieldDesc expandMessageArrays(const FieldDesc& desc)
{
    FieldDesc result;
    result.name = desc.name;
    result.is_primitive_type = desc.is_primitive_type;
    result.primitive = desc.primitive;
    result.array_size = desc.array_size;
    result.skip = desc.skip;

    for (const FieldDesc& field : desc.fields)
    {
        if (field.is_primitive_type && field.array_size < 0 && isBytePrimitive(field.primitive))
        {
            FieldDesc skipped = field;
            skipped.skip = true;
            result.fields.push_back(std::move(skipped));
            continue;
        }

        if (!field.is_primitive_type && field.array_size > 0)
        {
            for (int i = 0; i < field.array_size; i++)
            {
                FieldDesc expanded = expandMessageArrays(field);
                expanded.name = field.name + "_" + std::to_string(i);
                expanded.array_size = 0;
                result.fields.push_back(std::move(expanded));
            }
        }
        else if (!field.is_primitive_type && field.array_size < 0)
        {
            FieldDesc skipped = expandMessageArrays(field);
            skipped.skip = true;
            result.fields.push_back(std::move(skipped));
        }
        else
        {
            result.fields.push_back(expandMessageArrays(field));
        }
    }

    return result;
}

FieldDesc parseSchema(const std::string& schema_text, const std::string& type_name)
{
    std::string normalized = normalizeType(type_name);
    std::map<std::string, RawMsgDef> defs = splitDefinitions(schema_text, normalized);
    FieldDesc desc = resolveType(defs, normalized);
    desc.name = "root";
    return expandMessageArrays(desc);
}
