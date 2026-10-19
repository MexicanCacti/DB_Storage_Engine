#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <stdexcept>

using FieldVariant = std::variant<std::int64_t, std::string>;
using IntVariant = std::variant<std::int64_t, std::int32_t, std::int16_t, std::int8_t>;

const std::size_t PAGE_BYTE_LENGTH = 4096;
const std::size_t HEADER_BYTE_LENGTH = 24;

/*
    H + 2 * B(n) + n * R <= 4096
    So since the max tuple width would be n = 1 & H is always 24 & so then B(1) = 8 x ceil(1/64) = 8
    24 + 2 * 8 + R <= 4096
    24 + 16 + R <= 4096
    R <= 4056
    So therefore the max_tuple_width is 4056, any larger no room for a tuple slot
*/
const std::size_t MAX_TUPLE_WIDTH = 4056;

const std::int16_t FORMAT_VERSION = 1;
const std::int16_t FORMAT_ID = 0;

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

