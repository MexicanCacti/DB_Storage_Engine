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

bool Schema::isValidTuple(const std::vector<TupleValue>& tupleValueList) const {
    
    if(tupleValueList.size() != fields.size()) return false;

    for(size_t i = 0 ; i < fields.size(); ++i)
    {
        TupleValue val = tupleValueList[i];

        if(fields[i].type == FieldType::INT64 && !std::holds_alternative<std::int64_t>(val)) return false;
        if(fields[i].type == FieldType::STRING32 && (!std::holds_alternative<std::string>(val) || !isValidString32(std::get<std::string>(val))) ) return false;
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
    std::vector<std::byte> encodedTuple;
    std::size_t tupleValueListLength = tupleValueList.size();

    if(!isValidTuple(tupleValueList)) return {}; 

    for(std::size_t i = 0 ; i < tupleValueList.size(); ++ i)
    {
        try
        {
            std::vector<std::byte> tupleBytes = encodeTupleValue(tupleValueList[i]);
            for(std::byte b : tupleBytes) encodedTuple.push_back(b);

        } catch(...)
        {
            return {};
        }
    }

    return encodedTuple;
}

std::vector<TupleValue> Schema::decode(const std::vector<std::byte>& bytes) const
{
    std::vector<TupleValue> tupleValueList;
    std::size_t currentByte = 0;
    for(std::size_t i = 0 ; i < fields.size(); ++i)
    {
        if(fields[i].type == FieldType::INT64)
        {
            if(currentByte + 8 > bytes.size()) return {}; // Not all bytes are present!
            std::vector<std::byte> intBytes(bytes.begin() + currentByte, bytes.begin() + currentByte + 8);
            TupleValue decodedInt = decodeBytes(intBytes, FieldType::INT64);
            tupleValueList.push_back(decodedInt);
            currentByte += 8;
        }   
        else if(fields[i].type == FieldType::STRING32)
        {
            if(currentByte + 32 > bytes.size()) return {}; // Not all bytes are present!
            std::vector<std::byte> string32Bytes(bytes.begin() + currentByte, bytes.begin() + currentByte + 32);
            TupleValue decodedInt = decodeBytes(string32Bytes, FieldType::STRING32);
            tupleValueList.push_back(decodedInt);
            currentByte += 32;
        }
    }

    return tupleValueList;
}