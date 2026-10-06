#pragma once

// Defines & validates what a table's record should look like.

#include <vector>
#include <string>
#include <unordered_set> // For checking field name uniqueness upon creation
#include <stdexcept>
#include <typeinfo>
#include <iostream>

#include "../tuple/tuple.hpp"
#include "../field/field.hpp"

const std::size_t MAX_TUPLE_WIDTH = 40; // Note value will change once math done

class Schema {
    private:
        std::vector<Field> fields;
        Schema() = default;
    public:
        Schema(const std::vector<Field>& fieldList);
        bool isValidTuple(const std::vector<TupleValue>& tupleValueList) const;
        const std::vector<Field>& getFieldList() const {return fields;}
        const Field& getField(std::size_t index) const {return {};} // TODO implement
        std::size_t tupleWidth() const; // Total width of all columns in table in bytes
        std::vector<std::byte> encode(const std::vector<TupleValue>& tupleValueList) const;
        std::vector<TupleValue> decode(const std::vector<std::byte>& bytes) const;
};