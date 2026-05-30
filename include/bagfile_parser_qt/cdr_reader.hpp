#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <stdexcept>
#include "bagfile_parser_qt/schema_parser.hpp"

class CdrReader
{
public:
    CdrReader(const uint8_t* data, size_t size);

    double readAsDouble(PrimitiveType type);
    std::string readString();
    uint32_t readSequenceLength();
    void align(size_t boundary);
    void skipBytes(size_t n);

    size_t offset() const { return this->offset_; }
    bool hasData() const { return this->offset_ < this->size_; }

private:
    template <typename T>
    T read()
    {
        align(sizeof(T));
        if (this->offset_ + sizeof(T) > this->size_)
        {
            throw std::runtime_error("CDR buffer overrun");
        }
        T value;
        std::memcpy(&value, this->data_ + this->offset_, sizeof(T));
        this->offset_ += sizeof(T);
        return value;
    }

    const uint8_t* data_;
    size_t size_;
    size_t offset_;
};
