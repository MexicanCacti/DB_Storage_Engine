#include "schema.hpp"

Schema::Schema(const std::vector<Field>& fieldList)
    : schemaField(fieldList)
{
    std::size_t width = 0;

    // Validate Schema... Unique names & Tuple Width fits
    std::unordered_set<std::string> fieldNames;

    for(const Field& field : schemaField)
    {
        if(fieldNames.find(field.name) != fieldNames.end()) throw std::invalid_argument("[Schema]: Duplicate field name: " + field.name);
        std::size_t fieldSize = getFieldTypeSize(field.type);
        width += fieldSize;
        fieldNames.insert(field.name);   
    }

    if(width > MAX_TUPLE_WIDTH)
    {
        std::string errorMsg = "Tuple Width = " + std::to_string(width);
        errorMsg += "\tMax Width = " + std::to_string(MAX_TUPLE_WIDTH);
        throw std::invalid_argument("[Schema]: " + errorMsg);
    }
}

const Field& Schema::getField(std::size_t index) const
{
    if(index < 0 || index > schemaField.size()) throw std::invalid_argument("[getField]: Invalid index into field list");
    return schemaField[index];
}

std::size_t Schema::getTupleWidth() const
{
    std::size_t width = 0;

    for(const Field& field : schemaField)
    {
        width += getFieldTypeSize(field.type);
    }

    return width;
}

std::vector<std::byte> Schema::encode() const
{   
    return encodeFieldList(schemaField);
}

std::vector<Field> Schema::decode(const std::vector<std::byte>& bytes)
{
    return decodeFieldList(bytes);
}

bool Schema::isValidTuple(const std::vector<FieldVariant>& tupleValueList) const 
{
    if(tupleValueList.size() != schemaField.size()) return false;

    for(size_t i = 0 ; i < schemaField.size(); ++i)
    {
        FieldVariant tupleVal = tupleValueList[i];

        if(schemaField[i].type == FieldType::INT64 && !std::holds_alternative<std::int64_t>(tupleVal)) return false;
        if(schemaField[i].type == FieldType::STRING32 && (!std::holds_alternative<std::string>(tupleVal) || !isValidString32(std::get<std::string>(tupleVal))) ) return false;
    }

    return true;
}