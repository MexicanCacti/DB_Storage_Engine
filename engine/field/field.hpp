#pragma once

// This header file defines the valid field types of a column

#include <cstddef> // byte type
#include <cstdint> // int64 type
#include <string>
#include <variant>

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

bool isValidString32(const std::string& str);