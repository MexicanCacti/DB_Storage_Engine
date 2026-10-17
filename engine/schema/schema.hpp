#pragma once

#include <vector>
#include <unordered_set>
#include <string>

#include "basic_types.hpp"
#include "basic_types_encode_decode.hpp"

const std::size_t MAX_TUPLE_WIDTH = 200;

class Schema
{
    private:
        std::vector<Field> schemaField;
    public:
        Schema(const std::vector<Field>& fieldList);
        const std::vector<Field>& getFieldList() const {return schemaField;}
        const Field& getField(std::size_t index) const;
        std::size_t getTupleWidth() const; // Total width of all columns in a row
        std::vector<std::byte> encode() const; // Encodes schema
        static std::vector<Field> decode(const std::vector<std::byte>& bytes); // Decodes schema from byte array

        bool isValidTuple(const std::vector<FieldVariant>& tupleValueList) const;
};