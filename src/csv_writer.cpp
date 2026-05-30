#include "bagfile_parser_qt/csv_writer.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <functional>
#include <ostream>
#include <sstream>

struct CsvCol
{
    std::string header;
    std::function<void(std::ostream&, size_t)> emit;
};

static std::string escape(const std::string& s)
{
    bool needs = false;
    for (char c : s)
    {
        if (c == ',' || c == '"' || c == '\n' || c == '\r')
        {
            needs = true;
            break;
        }
    }
    if (!needs)
    {
        return s;
    }
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('"');
    for (char c : s)
    {
        if (c == '"')
        {
            out.push_back('"');
        }
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

static void emitDouble(std::ostream& os, double v)
{
    if (std::isnan(v))
    {
        return;
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.17g", v);
    os << buf;
}

static void collectCols(const CollectedData& data, const std::string& prefix, std::vector<CsvCol>& cols)
{
    switch (data.kind)
    {
        case CollectedData::SCALAR:
        {
            CsvCol c;
            c.header = prefix;
            c.emit = [&data](std::ostream& os, size_t row)
            {
                emitDouble(os, data.values[row]);
            };
            cols.push_back(std::move(c));
            break;
        }
        case CollectedData::FIXED_ARRAY:
        {
            int width = data.width;
            for (int j = 0; j < width; j++)
            {
                CsvCol c;
                c.header = prefix + "_" + std::to_string(j);
                c.emit = [&data, j, width](std::ostream& os, size_t row)
                {
                    emitDouble(os, data.values[row * width + j]);
                };
                cols.push_back(std::move(c));
            }
            break;
        }
        case CollectedData::DYN_ARRAY:
        {
            size_t max_w = 0;
            for (const std::vector<double>& a : data.dyn_arrays)
            {
                max_w = std::max(max_w, a.size());
            }
            for (size_t j = 0; j < max_w; j++)
            {
                CsvCol c;
                c.header = prefix + "_" + std::to_string(j);
                c.emit = [&data, j](std::ostream& os, size_t row)
                {
                    if (j < data.dyn_arrays[row].size())
                    {
                        emitDouble(os, data.dyn_arrays[row][j]);
                    }
                };
                cols.push_back(std::move(c));
            }
            break;
        }
        case CollectedData::STRING:
        {
            CsvCol c;
            c.header = prefix;
            c.emit = [&data](std::ostream& os, size_t row)
            {
                os << escape(data.strings[row]);
            };
            cols.push_back(std::move(c));
            break;
        }
        case CollectedData::STRING_ARRAY:
        {
            size_t max_w = 0;
            for (const std::vector<std::string>& a : data.string_arrays)
            {
                max_w = std::max(max_w, a.size());
            }
            for (size_t j = 0; j < max_w; j++)
            {
                CsvCol c;
                c.header = prefix + "_" + std::to_string(j);
                c.emit = [&data, j](std::ostream& os, size_t row)
                {
                    if (j < data.string_arrays[row].size())
                    {
                        os << escape(data.string_arrays[row][j]);
                    }
                };
                cols.push_back(std::move(c));
            }
            break;
        }
        case CollectedData::STRUCT:
        {
            for (const std::pair<std::string, CollectedData>& entry : data.children)
            {
                std::string sub = prefix.empty() ? entry.first : prefix + "." + entry.first;
                collectCols(entry.second, sub, cols);
            }
            break;
        }
        case CollectedData::SKIPPED:
            break;
    }
}

void writeCsv(const std::string& filepath, const CollectedData& data, const std::vector<double>& timestamps)
{
    std::ofstream f(filepath);
    if (!f)
    {
        return;
    }

    std::vector<CsvCol> cols;
    CsvCol tcol;
    tcol.header = "t";
    tcol.emit = [&timestamps](std::ostream& os, size_t row)
    {
        emitDouble(os, timestamps[row]);
    };
    cols.push_back(std::move(tcol));

    collectCols(data, "", cols);

    for (size_t i = 0; i < cols.size(); i++)
    {
        if (i)
        {
            f << ',';
        }
        f << cols[i].header;
    }
    f << '\n';

    size_t n = timestamps.size();
    for (size_t row = 0; row < n; row++)
    {
        for (size_t i = 0; i < cols.size(); i++)
        {
            if (i)
            {
                f << ',';
            }
            cols[i].emit(f, row);
        }
        f << '\n';
    }
}
