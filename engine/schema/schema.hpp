#pragma once

// Defines & validates what a table's record should look like.

#include <vector>
#include <string>
#include <unordered_set> // For checking field name uniqueness upon creation
#include <stdexcept>
#include <typeinfo>

#include "../tuple/tuple.hpp"
#include "../field/field.hpp"

const std::size_t MAX_TUPLE_WIDTH = 40; // Note value will change once math done

class Schema {
    private:
        std::vector<Field> fields;
        std::size_t getFieldSize(const FieldType& fieldType) const;
        Schema() = default;
    public:
        Schema(const std::vector<Field>& fieldList);
        bool isValidTuple(const Tuple& tuple) const;
        std::size_t tupleWidth() const; // Total width of all columns in table in bytes
        std::vector<std::byte> encode(const Tuple& tuple) const;
        Tuple decode(const std::vector<std::byte>& bytes) const;
};