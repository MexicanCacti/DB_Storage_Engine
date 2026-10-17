#include "basic_types_encode_decode.hpp"

std::vector<std::byte> encodeFieldVariant(const FieldVariant& fieldVariant)
{
    if(std::holds_alternative<std::int64_t>(fieldVariant))
    {
        return encodeIntVariant(std::get<std::int64_t>(fieldVariant));
    }
    else if(std::holds_alternative<std::string>(fieldVariant))
    {
        std::string stringVal = std::get<std::string>(fieldVariant);
        if(!isValidString32(stringVal)) throw std::invalid_argument("[encodeFieldVariant]: string is not a valid String32");

        std::vector<std::byte> byteArray(32, std::byte{0});
        for(size_t i = 0 ; i < stringVal.size(); ++i)
        {
            byteArray[i] = static_cast<std::byte>(stringVal[i]);
        }

        return byteArray;
    }
    else throw std::invalid_argument("[encodeFieldVariant]: Could not match fieldVariant arg");
}

std::vector<std::byte> encodeIntVariant(const IntVariant& intVariant)
{
    if(std::holds_alternative<std::int64_t>(intVariant))
    {   
        std::size_t byteCount = 8;
        std::int64_t int64Type = std::get<std::int64_t>(intVariant);
        std::uint64_t uint64Type = static_cast<std::uint64_t>(int64Type); // Needed for encoding correctly!
        std::vector<std::byte> byteArray(byteCount, std::byte{0}); // 8 Byte empty array all init to 0
        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            byteArray[currentByte] = static_cast<std::byte>(uint64Type >> (currentByte * 8) & 0xFF);
        }

        return byteArray;
    }
    else if(std::holds_alternative<std::int32_t>(intVariant))
    {
        std::size_t byteCount = 4;
        std::int32_t int32Type = std::get<std::int32_t>(intVariant);
        std::uint32_t uint32Type = static_cast<std::uint32_t>(int32Type); // Needed for encoding correctly!
        std::vector<std::byte> byteArray(byteCount, std::byte{0}); // 4 Byte empty array all init to 0
        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            byteArray[currentByte] = static_cast<std::byte>(uint32Type >> (currentByte * 8) & 0xFF);
        }

        return byteArray;
    }
    else if(std::holds_alternative<std::int16_t>(intVariant))
    {
        std::size_t byteCount = 2;
        std::int16_t int16Type = std::get<std::int16_t>(intVariant);
        std::uint16_t uint16Type = static_cast<std::uint16_t>(int16Type); // Needed for encoding correctly!
        std::vector<std::byte> byteArray(byteCount, std::byte{0}); // 2 Byte empty array all init to 0
        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            byteArray[currentByte] = static_cast<std::byte>(uint16Type >> (currentByte * 8) & 0xFF);
        }

        return byteArray;
    }
    else if(std::holds_alternative<std::int8_t>(intVariant))
    {
        std::size_t byteCount = 1;
        std::int8_t int8Type = std::get<std::int8_t>(intVariant);
        std::uint8_t uint8Type = static_cast<std::uint8_t>(int8Type); // Needed for encoding correctly!
        std::vector<std::byte> byteArray(byteCount, std::byte{0}); // 1 Byte empty array all init to 0
        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            byteArray[currentByte] = static_cast<std::byte>(uint8Type >> (currentByte * 8) & 0xFF);
        }

        return byteArray;
    }
    else throw std::invalid_argument("[encodeIntVariant]: Could not match intVariant arg");
}

std::byte encodeFieldType(const FieldType fieldType)
{
    switch(fieldType)
    {
        case FieldType::INT64:
        case FieldType::STRING32:
            return static_cast<std::byte>(fieldType);
    }
    throw std::invalid_argument("[encodeFieldType]: Could not match fieldType arg");
}

std::byte encodeIntType(const IntType intType)
{
    switch(intType)
    {
        case IntType::INT64:
        case IntType::INT32:
        case IntType::INT16:
        case IntType::INT8:
            return static_cast<std::byte>(intType);
    }
    throw std::invalid_argument("[encodeIntType]: Could not match intType arg");
}

std::vector<std::byte> encodeField(const Field& field)
{
    // 32 Byte for field name, 1 Byte for type
    std::vector<std::byte> byteArray(fieldEncodeLength, std::byte{0});

    std::byte fieldTypeByte = encodeFieldType(field.type);
    std::vector<std::byte> fieldNameBytes = encodeFieldVariant(field.name);

    std::size_t offset = 0;
    copyBytes(byteArray, fieldNameBytes, offset);
    byteArray[offset] = fieldTypeByte;
    return byteArray;
}

std::vector<std::byte> encodeFieldList(const std::vector<Field>& fieldList)
{
    std::int16_t fieldCount = static_cast<std::int16_t>(fieldList.size());
    // FieldList: 2 Bytes for length of fieldList, Then each field uses 33 bytes [32 : fieldName, 1: fieldType]
    std::vector<std::byte> byteArray(2 + fieldEncodeLength * fieldCount, std::byte{0});
    
    std::size_t offset = 0;
    std::vector<std::byte> fieldCountBytes = encodeIntVariant(fieldCount);
    copyBytes(byteArray, fieldCountBytes, offset);

    for(const Field& field : fieldList)
    {
        std::vector<std::byte> fieldBytes = encodeField(field);
        copyBytes(byteArray, fieldBytes, offset);
    }
    return byteArray;
}

std::vector<std::byte> encodeFieldVariantList(const std::vector<FieldVariant>& fieldVariantList)
{
    std::vector<std::byte> byteArray;
    for(const FieldVariant& fieldVariant : fieldVariantList)
    {
        std::vector<std::byte> fieldVariantBytes = encodeFieldVariant(fieldVariant);
        appendBytes(byteArray, fieldVariantBytes);
    }
    return byteArray;
}

FieldVariant decodeFieldVariant(const std::vector<std::byte>& bytes, FieldType fieldType)
{
    if(bytes.size() != getFieldTypeSize(fieldType)) throw std::invalid_argument("[decodeFieldVariant]: Byte length doesn't match fieldType");

    if(fieldType == FieldType::INT64)
    {
        return std::get<std::int64_t>(decodeIntVariant(bytes, IntType::INT64));
    } 
    else if(fieldType == FieldType::STRING32)
    {
        std::string stringVal = "";
        std::size_t currentByte = 0;
        for(currentByte ; currentByte < 32; ++currentByte)
        {
            if(bytes[currentByte] == std::byte{0x00}) break; //beginning of trailing padding
            stringVal.push_back(static_cast<char>(bytes[currentByte]));
        }

        while(currentByte < 32)
        {
            if(bytes[currentByte++] != std::byte{0x00}) throw std::invalid_argument("[decodeFieldVariant]: Embedded 0");
        }

        if(!isValidString32(stringVal)) throw std::invalid_argument("[decodeFieldVariant]: Decoded non-valid String32");

        return stringVal;
    }

    throw std::invalid_argument("Could not match field type");
}

IntVariant decodeIntVariant(const std::vector<std::byte>& bytes, IntType intType)
{
    if(bytes.size() != getIntTypeSize(intType)) throw std::invalid_argument("Byte length doesn't match intVariant");

    if(intType == IntType::INT64)
    {
        std::size_t byteCount = 8;
        std::uint64_t value = 0; // Bit-wise, everything is set to 0

        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            std::uint64_t byteValue = std::to_integer<std::uint64_t>(bytes[currentByte]);
            byteValue = byteValue << (currentByte * 8); // Want to shift the byteValue to the left s.t. it get's OR'd into its correct position
            
            // Because every bit in value is default set to 0, any OR with a retrieved byte will set the bit... meaning we get the correct bytes
            value |= byteValue;
        }

        std::int64_t int64Value;
        memcpy(&int64Value, &value, sizeof(std::int64_t));
        return int64Value;
    }
    else if(intType == IntType::INT32)
    {
        std::size_t byteCount = 4;
        std::uint32_t value = 0; // Bit-wise, everything is set to 0

        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            std::uint32_t byteValue = std::to_integer<std::uint32_t>(bytes[currentByte]);
            byteValue = byteValue << (currentByte * 8); // Want to shift the byteValue to the left s.t. it get's OR'd into its correct position
            
            // Because every bit in value is default set to 0, any OR with a retrieved byte will set the bit... meaning we get the correct bytes
            value |= byteValue;
        }

        std::int32_t int32Value;
        memcpy(&int32Value, &value, sizeof(std::int32_t));
        return int32Value;
    }
    else if(intType == IntType::INT16)
    {
        std::size_t byteCount = 2;
        std::uint16_t value = 0; // Bit-wise, everything is set to 0

        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            std::uint16_t byteValue = std::to_integer<std::uint16_t>(bytes[currentByte]);
            byteValue = byteValue << (currentByte * 8); // Want to shift the byteValue to the left s.t. it get's OR'd into its correct position
            
            // Because every bit in value is default set to 0, any OR with a retrieved byte will set the bit... meaning we get the correct bytes
            value |= byteValue;
        }

        std::int16_t int16Value;
        memcpy(&int16Value, &value, sizeof(std::int16_t));
        return int16Value;
    }
    else if(intType == IntType::INT8)
    {
        std::size_t byteCount = 1;
        std::uint8_t value = 0; // Bit-wise, everything is set to 0

        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            std::uint8_t byteValue = std::to_integer<std::uint8_t>(bytes[currentByte]);
            byteValue = byteValue << (currentByte * 8); // Want to shift the byteValue to the left s.t. it get's OR'd into its correct position
            
            // Because every bit in value is default set to 0, any OR with a retrieved byte will set the bit... meaning we get the correct bytes
            value |= byteValue;
        }

        std::int8_t int8Value;
        memcpy(&int8Value, &value, sizeof(std::int8_t));
        return int8Value;
    }
    else throw std::invalid_argument("[decodeIntVariant]: Could not match intType");
}

FieldType decodeFieldType(const std::byte& byte)
{
    std::uint8_t fieldTypeValue = std::to_integer<std::uint8_t>(byte);
    switch(fieldTypeValue)
    {
        case 0:
            return FieldType::INT64;
        case 1:
            return FieldType::STRING32;
    }
    throw std::invalid_argument("[decodeFieldType]: Could not match fieldType");
}

IntType decodeIntType(const std::byte& byte)
{
    std::uint8_t intTypeValue = std::to_integer<std::uint8_t>(byte);
    switch(intTypeValue)
    {
        case 0:
            return IntType::INT64;
        case 1:
            return IntType::INT32;
        case 2:
            return IntType::INT16;
        case 3:
            return IntType::INT8;
    }
    throw std::invalid_argument("[decodeIntType]: Could not match intType");
}

Field decodeField(const std::vector<std::byte>& bytes)
{
    if(bytes.size() != fieldEncodeLength) throw std::invalid_argument("[decodeField]: Byte length doesn't match field encoding length");
    
    std::size_t offset = 0;
    std::vector<std::byte> fieldStringBytes(32, std::byte{0});
    copyNBytesSrcOffset(fieldStringBytes, bytes, offset, 32);
    std::byte fieldTypeByte = bytes[offset];

    std::string fieldName = std::get<std::string>(decodeFieldVariant(fieldStringBytes, FieldType::STRING32));
    FieldType fieldType = decodeFieldType(fieldTypeByte);

    return Field{fieldName, fieldType};
}

std::vector<Field> decodeFieldList(const std::vector<std::byte>& bytes)
{
    if(bytes.size() < 2) throw std::invalid_argument("[decodeFieldList]: Missing field count");

    std::vector<std::byte> fieldCountBytes(2, std::byte{0});
    std::size_t offset = 0;
    copyNBytesSrcOffset(fieldCountBytes, bytes, offset, 2);
    
    std::int16_t fieldCount = std::get<std::int16_t>(decodeIntVariant(fieldCountBytes, IntType::INT16));
    if(fieldCount < 0) throw std::invalid_argument("[decodeFieldList]: Negative field count");

    std::vector<Field> fieldList(fieldCount);
    for(std::size_t currentField = 0; currentField < fieldCount; ++currentField)
    {
        std::vector<std::byte> fieldBytes(fieldEncodeLength, std::byte{0});
        copyNBytesSrcOffset(fieldBytes, bytes, offset, fieldEncodeLength);
        Field field = decodeField(fieldBytes);
        fieldList[currentField] = field;
    }

    return fieldList;
}

std::vector<FieldVariant> decodeFieldVariantList(const std::vector<std::byte>& bytes, const std::vector<Field>& fieldList)
{
    size_t currentField = 0;
    size_t offset = 0;
    std::vector<FieldVariant> fieldVariantList;
    while(currentField < fieldList.size())
    {
        FieldType fieldType = fieldList[currentField++].type;
        std::vector<std::byte> fieldByteArray(getFieldTypeSize(fieldType), std::byte{0});
        copyNBytesSrcOffset(fieldByteArray, bytes, offset, getFieldTypeSize(fieldType));   
        fieldVariantList.push_back(decodeFieldVariant(fieldByteArray, fieldType));
    }

    return fieldVariantList;
}