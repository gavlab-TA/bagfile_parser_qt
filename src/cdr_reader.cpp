#include "bagfile_parser_qt/cdr_reader.hpp"

CdrReader::CdrReader(const uint8_t* data, size_t size)
{
    if (size < 4)
    {
        throw std::runtime_error("CDR data too short");
    }
    if (data[1] != 0x01)
    {
        throw std::runtime_error("Only little-endian CDR is supported");
    }
    this->data_ = data + 4;
    this->size_ = size - 4;
    this->offset_ = 0;
}

void CdrReader::align(size_t boundary)
{
    size_t rem = this->offset_ % boundary;
    if (rem != 0)
    {
        this->offset_ += boundary - rem;
    }
}

double CdrReader::readAsDouble(PrimitiveType type)
{
    switch (type)
    {
        case PrimitiveType::BOOL:    return read<uint8_t>() ? 1.0 : 0.0;
        case PrimitiveType::INT8:    return static_cast<double>(read<int8_t>());
        case PrimitiveType::UINT8:
        case PrimitiveType::BYTE:
        case PrimitiveType::CHAR:    return static_cast<double>(read<uint8_t>());
        case PrimitiveType::INT16:   return static_cast<double>(read<int16_t>());
        case PrimitiveType::UINT16:  return static_cast<double>(read<uint16_t>());
        case PrimitiveType::INT32:   return static_cast<double>(read<int32_t>());
        case PrimitiveType::UINT32:  return static_cast<double>(read<uint32_t>());
        case PrimitiveType::INT64:   return static_cast<double>(read<int64_t>());
        case PrimitiveType::UINT64:  return static_cast<double>(read<uint64_t>());
        case PrimitiveType::FLOAT32: return static_cast<double>(read<float>());
        case PrimitiveType::FLOAT64: return read<double>();
        case PrimitiveType::STRING:
        case PrimitiveType::WSTRING:
            throw std::runtime_error("Cannot read string as double");
    }
    return 0.0;
}

std::string CdrReader::readString()
{
    uint32_t length = read<uint32_t>();
    if (length == 0)
    {
        return "";
    }
    if (this->offset_ + length > this->size_)
    {
        throw std::runtime_error("CDR string overrun");
    }
    std::string result(reinterpret_cast<const char*>(this->data_ + this->offset_), length - 1);
    this->offset_ += length;
    return result;
}

uint32_t CdrReader::readSequenceLength()
{
    return read<uint32_t>();
}

void CdrReader::skipBytes(size_t n)
{
    if (this->offset_ + n > this->size_)
    {
        throw std::runtime_error("CDR skip overrun");
    }
    this->offset_ += n;
}
