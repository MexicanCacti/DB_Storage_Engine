#pragma once

#include <vector>
#include <string>
#include <cstddef> // byte type
#include <cstdint> // int64 type
#include <unordered_set> // For checking field name uniqueness upon creation
#include <stdexcept>

#include "../tuple/tuple.hpp"

const std::size_t MAX_TUPLE_WIDTH = 4096;

enum class FieldType {
    INT64,
    STRING32
};

struct Field {
    std::string name;
    FieldType type;
};

class Schema {
    private:
        std::vector<Field> fields;
        std::size_t getFieldSize(const FieldType& fieldType) const;

    public:
        Schema(const std::vector<Field>& fieldList);
        bool isValidTuple(const Tuple& tuple) const;
        std::vector<std::byte> encode(const Tuple& tuple) const;
        Tuple decode(const std::vector<std::byte>& bytes) const;
        std::size_t tupleWidth() const; // Total width of all columns in table in bytes

};