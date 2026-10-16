#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <stdexcept>

using FieldVariant = std::variant<std::int64_t, std::string>;
using IntVariant = std::variant<std::int64_t, std::int32_t, std::int16_t, std::int8_t>;

// enum types guaranteed to be 1 byte in size
enum class FieldType : int8_t 
{
    INT64 = 0,
    STRING32 = 1
};

enum class IntType : int8_t
{
    INT64 = 0,
    INT32 = 1,
    INT16 = 2,
    INT8 = 3
};

struct Field
{
    std::string name;
    FieldType type;
};

std::size_t getFieldTypeSize(FieldType fieldType);
std::size_t getIntTypeSize(IntType intType);

bool isValidString32(const std::string& str);
