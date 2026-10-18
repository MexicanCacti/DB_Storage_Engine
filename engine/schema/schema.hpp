#pragma once

#include <vector>
#include <unordered_set>
#include <string>

#include "basic_types.hpp"
#include "basic_types_encode_decode.hpp"

/*
    H + 2 * B(n) + n * R <= 4096
    So since the max tuple width would be n = 1 & H is always 24 & so then B(1) = 8 x ceil(1/64) = 8
    24 + 2 * 8 + R <= 4096
    24 + 16 + R <= 4096
    R <= 4056
    So therefore the max_tuple_width is 4056, any larger no room for a tuple slot


*/
const std::size_t MAX_TUPLE_WIDTH = 4056;

class Schema
{
    private:
        std::vector<Field> schemaField;
        std::size_t tupleWidth;
    public:
        Schema(const std::vector<Field>& fieldList);
        const std::vector<Field>& getFieldList() const {return schemaField;}
        const Field& getField(std::size_t index) const;
        std::size_t getTupleWidth() const {return tupleWidth;} // Total width of all columns in a row
        std::vector<std::byte> encode() const; // Encodes schema
        static std::vector<Field> decode(const std::vector<std::byte>& bytes); // Decodes schema from byte array

        bool isValidTuple(const std::vector<FieldVariant>& tupleValueList) const;
};