#pragma once
#include <string>
#include <vector>

enum class PrimitiveType
{
    BOOL, INT8, UINT8, BYTE, CHAR,
    INT16, UINT16, INT32, UINT32,
    INT64, UINT64, FLOAT32, FLOAT64,
    STRING, WSTRING
};

struct FieldDesc
{
    std::string name;
    PrimitiveType primitive{};
    bool is_primitive_type = false;
    std::vector<FieldDesc> fields;
    int array_size = 0;
    bool skip = false;
    int padded_max = 0;
    std::string skip_reason;  // why adjustSchema left this field skipped
};

FieldDesc parseSchema(const std::string& schema_text, const std::string& type_name);

bool isPrimitive(const std::string& type_str);
PrimitiveType toPrimitive(const std::string& type_str);
size_t primitiveCdrAlign(PrimitiveType p);
