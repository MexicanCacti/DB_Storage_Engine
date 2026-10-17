#pragma once

#include "basic_types.hpp"
#include "basic_types_encode_decode.hpp"
#include <vector>

// Contains the values that correspond to its schema. TupleValues order & Schema Field order should correspond 1:1

class Tuple
{
    private:
        std::vector<FieldVariant> tupleValues;

    public:
        Tuple(const std::vector<FieldVariant>& valueList) : tupleValues(valueList) {}
        const std::vector<FieldVariant>& getTupleValues() const {return tupleValues;}
        const FieldVariant& getTupleValueIndex(std::size_t index);

        std::vector<std::byte> encode() const;
        static std::vector<FieldVariant> decode(const std::vector<std::byte>& bytes, const std::vector<Field>& schemaFieldList); // Decodes tuple from bytes
};