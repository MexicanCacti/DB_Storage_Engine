#pragma once

// This header file defines the valid field types of a column

#include <cstddef> // byte type
#include <cstdint> // int64 type
#include <string>
#include <variant>
#include <vector>
#include <cstring> // Byte array
#include <bitset>
#include <stdexcept>

// INT64 & STRING32... Add as needed
using TupleValue = std::variant<std::int64_t, std::string>;

enum class FieldType {
    INT64,
    STRING32
};

struct Field {
    std::string name;
    FieldType type;
};

std::size_t getFieldTypeSize(FieldType fieldType);

bool isValidString32(const std::string& str);

std::vector<std::byte> encodeTupleValue(const TupleValue& tupleValue);

TupleValue decodeBytes(const std::vector<std::byte>& bytes, FieldType fieldType);