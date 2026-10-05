#include "schema.hpp"

Schema::Schema(const std::vector<Field>& fieldList)
    : fields(fieldList)
{
    if(fieldList.empty()) throw std::invalid_argument("Schema Field List Empty!");
    std::size_t width = 0;

    // Validate Schema: Unique Names, Tuple Width Fits
    std::unordered_set<std::string> fieldNames;

    for(const Field& field : fieldList)
    {
        if(fieldNames.find(field.name) != fieldNames.end()) throw std::invalid_argument("Duplicate Field Name: " + field.name);
        std::size_t fieldSize = getFieldTypeSize(field.type);
        if(fieldSize == 0) throw std::invalid_argument("Invalid Field Type: " + field.name);
        width += fieldSize;
        fieldNames.insert(field.name);
    }

    if(width > MAX_TUPLE_WIDTH)
    {
        std::string errorMsg = "Tuple Width = " + std::to_string(width);
        errorMsg += "\tMax Width = " + std::to_string(MAX_TUPLE_WIDTH);
        throw std::invalid_argument(errorMsg);
    }
}

bool Schema::isValidTuple(const Tuple& tuple) const {
    
    if(tuple.getTupleValues().size() != fields.size()) return false;

    for(size_t i = 0 ; i < fields.size(); ++i)
    {
        const TupleValue* val = tuple.getTupleValue(i);
        if(!val) return false;

        if(fields[i].type == FieldType::INT64 && !std::holds_alternative<std::int64_t>(*val)) return false;
        if(fields[i].type == FieldType::STRING32 && (!std::holds_alternative<std::string>(*val) || !isValidString32(std::get<std::string>(*val))) ) return false;
    }

    return true;
}

std::size_t Schema::tupleWidth() const
{
    std::size_t width = 0;

    for(const Field& field : fields)
    {
        width += getFieldTypeSize(field.type);
    }

    return width;
}

std::vector<std::byte> Schema::encode(const std::vector<TupleValue>& tupleValueList) const
{
    return {};
}

std::vector<TupleValue> Schema::decode(const std::vector<std::byte>& bytes) const
{
    return {};
}