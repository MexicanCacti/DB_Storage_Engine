#include "tuple.hpp"

const FieldVariant& Tuple::getTupleValueIndex(std::size_t index)
{
    if(index >= tupleValues.size()) throw std::invalid_argument("[getTupleValueIndex]: Invalid index into value list");
    return tupleValues[index];
}

std::vector<std::byte> Tuple::encode() const
{
    return encodeFieldVariantList(tupleValues);
}

std::vector<FieldVariant> Tuple::decode(const std::vector<std::byte>& bytes, const std::vector<Field>& schemaFieldList)
{
    return decodeFieldVariantList(bytes, schemaFieldList);
}