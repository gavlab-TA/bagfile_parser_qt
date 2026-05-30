#pragma once
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "bagfile_parser_qt/schema_parser.hpp"
#include "bagfile_parser_qt/cdr_reader.hpp"

struct CollectedData
{
    enum Kind { SCALAR, FIXED_ARRAY, DYN_ARRAY, STRING, STRING_ARRAY, STRUCT, SKIPPED };
    Kind kind = STRUCT;

    std::vector<double> values;
    int width = 0;

    std::vector<std::vector<double>> dyn_arrays;

    std::vector<std::string> strings;

    std::vector<std::vector<std::string>> string_arrays;

    std::vector<std::pair<std::string, CollectedData>> children;

    CollectedData* child(const std::string& name);
};

void allocateCollectors(CollectedData& data, const FieldDesc& desc, size_t n);
void fillMessage(CollectedData& data, const FieldDesc& desc, CdrReader& reader, size_t idx);

void scanMessage(const FieldDesc& root_desc, CdrReader& reader, std::unordered_map<std::string, uint32_t>& stats);

FieldDesc adjustSchema(const FieldDesc& root_desc, const std::unordered_map<std::string, uint32_t>& stats, int byte_threshold, int msg_threshold, size_t msg_count);

void writeMat(const std::string& filepath, const std::string& var_name, const CollectedData& data, const std::vector<double>& timestamps);

std::string sanitizeName(const std::string& topic);
