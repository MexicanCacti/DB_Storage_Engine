#pragma once

#include <vector>
#include <unordered_set>
#include <string>

#include "basic_types.hpp"
#include "basic_types_encode_decode.hpp"

class Schema
{
    private:
        std::vector<Field> schemaField;
        std::size_t tupleWidth;
    public:
        Schema() = default;
        Schema(const std::vector<Field>& fieldList);
        const std::vector<Field>& getFieldList() const {return schemaField;}
        const Field& getField(std::size_t index) const;
        std::size_t getTupleWidth() const {return tupleWidth;} // Total width of all columns in a row
        std::vector<std::byte> encode() const; // Encodes schema
        static std::vector<Field> decode(const std::vector<std::byte>& bytes); // Decodes schema from byte array

        bool isValidTuple(const std::vector<FieldVariant>& tupleValueList) const;
};