#include "field.hpp"

#include <stdlib.h>

void appendBytes(std::vector<std::byte>& dest, std::vector<std::byte>& src)
{
    for(std::byte& byte : src)
    {
        dest.push_back(src);
    }
}

void copyBytes(std::vector<std:byte>& dest, std::vector<std::byte>& src, std::size_t& offset)
{
    if(offset > dest.size() || src.size() > dest.size() - offset) throw std::out_of_range("src bytes larger than dest");
    std::copy(src.begin(), src.end(), dest.begin() + offset);
    offset += src.size();
}

std::size_t getIntTypeSize(IntType intType)
{
    switch(intType)
    {
        case IntType::INT64:
            return sizeof(std::int64_t);
        case IntType::INT32:
            return sizeof(std::int32_t);
        case IntType::INT16:
            return sizeof(std::int16_t);
    }
    return 0;
}

std::size_t getFieldTypeSize(FieldType fieldType)
{
    switch(fieldType)
    {
        case FieldType::INT64:
            return sizeof(std::int64_t);
        case FieldType::STRING32:
            return sizeof(char[32]);
    }

    return 0; // Should only be 0 if invalid
    
}

bool isValidString32(const std::string& str)
{

    if(str.size() > 32) return false;

    // UTF-8 Char can be 1 -> 4 Bytes Long

    /*
    Source: https://www.rfc-editor.org/rfc/rfc3629.txt

      Char. number range|        UTF-8 octet sequence
      (hexadecimal)     |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx                             
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx                    
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx           
    
    */
    // 1-byte: 0xxxxxxx = 0X00 ... So first bit should be 0 ... 10 000000
    // 2-byte: 110xxxxx = 0XC0 ... So first 2 bits should be 1 w/ 3rd 0 ... 111 00000 
    // 3-byte: 1110xxxx = 0XE0 ... So first 3 bits should be 1 w/ 4th 0 ... 1111 0000 
    // 4-byte: 11110xxx = 0xF0 ... So first 4 bits should be 1 w/ 5th 0 ... 11111 000
    // Note: notice how they share the same continuation bytes

    std::size_t index = 0;
    while(index < str.size())
    {
        // Check how many bytes this UTF-8 says it should be & if its an embedded 0
        unsigned char byte = static_cast<unsigned char>(str[index]);
        if(byte == 0x00) return false; // Embedded 0 Byte

        // Get the byte sequence length
        std::size_t byteSequenceLength = 0;
        if( (byte & 0x80) == 0x00 ) byteSequenceLength = 1;
        else if( (byte & 0xE0) == 0xC0 ) byteSequenceLength = 2;
        else if( (byte & 0xF0) == 0xE0 ) byteSequenceLength = 3;
        else if( (byte & 0xF8) == 0xF0 ) byteSequenceLength = 4;
        else return false; // Leading byte doesn't match the 1->4 byte leading pattern

        if(index + byteSequenceLength > str.size()) return false; // Out of bounds, something is wrong!

        // Check structure of next bytes in sequence for correctness
        unsigned char nextByte = byte;
        for(std::size_t next = 1; next < byteSequenceLength; ++next)
        {
            nextByte = static_cast<unsigned char>(str[index + next]);
            if(nextByte == 0x00 || ( (nextByte & 0xC0) != 0x80)) return false; // Embedded 0 or not correct format
        }

        // Now check for invalid UTF-8 Ranges/ overly long
        /* 
            RFC States U+D800 (0xED 0xA0 0x80) -> U+DFFF (0xED 0xBF 0xBF) Are not legal Unicode (surrogates)
            So that must be checked to for 3 Byte lengths for the surrogates
        */
        if(byteSequenceLength == 2)
        {
            //First Code Point: U+0080 (0xC2 0x80), Last: U+07FF (0xDF 0xBF)
            /*
                First Byte should not be < C2
                If First Byte == C2 , 2nd better be >= 80
                If First Byte == DF , 2nd Better be <= BF
            */
            nextByte = static_cast<unsigned char>(str[index + 1]);

            if(byte < 0xC2) return false;

            if(byte == 0xC2 && nextByte < 0x80) return false;
            
            if(byte == 0xDF && nextByte > 0xBF) return false;

        }
        else if (byteSequenceLength == 3)
        {
            //First Code Point: U+0800 (0xE0 0xA0 0x80), Last: U+FFFF (0xEF 0xBF 0xBF)

            /*
                If First Byte == E0 , 2nd better be >= A0
                If First Byte == EF , 2nd Better be <= BF
                0xED 0xA0 0x80 -> 0xED 0xBF 0xBF not valid ... If First Byte == ED, 2nd better be < A0
            */

            nextByte = static_cast<unsigned char>(str[index + 1]);

            if(byte == 0xE0 && nextByte < 0xA0) return false;
            
            if(byte == 0xEF && nextByte > 0xBF) return false;

            if(byte == 0xED && nextByte >= 0xA0) return false;

        }
        else if (byteSequenceLength == 4)
        {
            //First Code Point: U+010000 (0xF0 0x90 0x80 0x80), Last: U+10FFFF (0xF4 0x8F 0xBF 0xBF)

            /*
                If First Byte == F0 , 2nd better be >= 90
                If First Byte == F4 , 2nd Better be <= 8F
                First Byte should not be > F4
            */

            nextByte = static_cast<unsigned char>(str[index + 1]);

            if(byte == 0xF0 && nextByte < 0x90) return false;
            
            if(byte == 0xF4 && nextByte > 0x8F) return false;

            if(byte > 0xF4) return false;

        }

        index += byteSequenceLength;

    }
    
    return true;
}

std::byte encodeFieldType(const FieldType& fieldType)
{
    switch(fieldType)
    {
        case FieldType::INT64:
        case FieldType::STRING32:
            return static_cast<std::byte>(fieldType);
    }
    throw std::invalid_argument("Invalid FieldType!");
}

std::vector<std::byte> encodeIntType(const SupportIntType& intValue)
{
    if(std::holds_alternative<std::int64_t>(intValue))
    {   
        std::size_t byteCount = 8;
        std::int64_t int64Type = std::get<std::int64_t>(intValue);
        std::uint64_t uint64Type = static_cast<std::uint64_t>(int64Type); // Needed for encoding correctly!
        std::vector<std::byte> byteArray(byteCount, std::byte{0}); // 8 Byte empty array all init to 0
        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            byteArray[currentByte] = static_cast<std::byte>(uint64Type >> (currentByte * 8) & 0xFF);
        }

        return byteArray;
    }
    else if(std::holds_alternative<std::int32_t>(intValue))
    {
        std::size_t byteCount = 4;
        std::int32_t int32Type = std::get<std::int32_t>(intValue);
        std::uint32_t uint32Type = static_cast<std::uint32_t>(int32Type); // Needed for encoding correctly!
        std::vector<std::byte> byteArray(byteCount, std::byte{0}); // 4 Byte empty array all init to 0
        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            byteArray[currentByte] = static_cast<std::byte>(uint32Type >> (currentByte * 8) & 0xFF);
        }

        return byteArray;
    }
    else if(std::holds_alternative<std::int16_t>(intValue))
    {
        std::size_t byteCount = 2;
        std::int16_t int16Type = std::get<std::int16_t>(intValue);
        std::uint16_t uint16Type = static_cast<std::uint16_t>(int16Type); // Needed for encoding correctly!
        std::vector<std::byte> byteArray(byteCount, std::byte{0}); // 2 Byte empty array all init to 0
        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            byteArray[currentByte] = static_cast<std::byte>(uint16Type >> (currentByte * 8) & 0xFF);
        }

        return byteArray;
    }
    else throw std::invalid_argument("Could not match int type!");
}

// Throws invalid_argument if tupleValue doesn't match a variant
// Basically just creates a byte vector for the tuple... no header info just pure raw byte tuple data
std::vector<std::byte> encodeTupleValue(const TupleValue& tupleValue)
{
    if(std::holds_alternative<std::int64_t>(tupleValue))
    {
        return encodeIntType(std::get<std::int64_t>(tupleValue));
    }
    else if(std::holds_alternative<std::string>(tupleValue))
    {
        std::string stringTupleVal = std::get<std::string>(tupleValue);
        if(!isValidString32(stringTupleVal)) return {};

        std::vector<std::byte> byteArray(32, std::byte{0}); // 32 Byte empty array all init to 0
        std::memcpy(byteArray.data() , stringTupleVal.data(), stringTupleVal.size());
        return byteArray;
    }
    else throw std::invalid_argument("Could not match tupleValue!");
    
}

std::vector<std::byte> encodeSchemaField(const std::vector<Field>& schemaField)
{
    // 2 Bytes for # of fields. ( 34 Bytes : 32 Bytes for field name + 2 Bytes for field type ) * # of fields
    std::size_t schemaFieldByteSize = 2 + 34 * schemaField.size();
    std::vector<std::byte> encodedSchemaField;
    std::uint16_t fieldCount = schemaField.size();
    std::vector<std::byte> fieldCountBytes = encodeIntType(fieldCount);
    appendBytes(encodedSchemaField, fieldCountBytes);
    
    for(Field& field : schemaField)
    {
        std::vector<std::byte> fieldNameBytes = encodeTupleValue(field.name);
        std::byte fieldTypeByte = encodeFieldType(field.type);
        appendBytes(encodedSchemaField, fieldNameBytes);
        encodedSchemaField.push_back(fieldTypeByte);
    }

    if(encodedSchemaField.size() != schemaFieldByteSize) throw std::invalid_argument("encoded schema field byte size doesn't match mathmatically");

    return encodedSchemaField;
}

SupportedIntType decodeIntType(const std::vector<std::byte>& bytes, IntType intType)
{
    if(bytes.size() != getIntTypeSize(intType)) throw std::invalid_argument("Byte length doesn't match intType!");

    if(intType == IntType::INT64)
    {
        std::size_t byteCount = 8;
        std::uint64_t value = 0; // Bit-wise, everything is set to 0

        for(std::size_t currentByte = 0 ; currentByte < byteCount; ++currentByte)
        {
            std::uint64_t byteValue = static_cast<std::uint64_t>(bytes[currentByte]);
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
            std::uint32_t byteValue = static_cast<std::uint32_t>(bytes[currentByte]);
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
            std::uint16_t byteValue = static_cast<std::uint16_t>(bytes[currentByte]);
            byteValue = byteValue << (currentByte * 8); // Want to shift the byteValue to the left s.t. it get's OR'd into its correct position
            
            // Because every bit in value is default set to 0, any OR with a retrieved byte will set the bit... meaning we get the correct bytes
            value |= byteValue;
        }

        std::int16_t int16Value;
        memcpy(&int16Value, &value, sizeof(std::int16_t));
        return int16Value;
    }
    else throw std::invalid_argument("Could not match int type!");
}

// Takes in the raw bytes and based on the field type passed in, attempts to create a TupleValue of that type
TupleValue decodeBytes(const std::vector<std::byte>& bytes, FieldType fieldType)
{
    if(bytes.size() != getFieldTypeSize(fieldType)) throw std::invalid_argument("Byte length doesn't match fieldType!");

    if(fieldType == FieldType::INT64)
    {
        return decodeIntType(bytes, IntType::INT64);
    }
    else if(fieldType == FieldType::STRING32)
    {
        std::string tupleValue = "";
        std::size_t currentByte = 0;
        for(currentByte ; currentByte < 32; ++currentByte)
        {
            if(bytes[currentByte] == std::byte{0x00}) break; // Beginning of trailing padding, we don't wanted embedded zeroes
            tupleValue.push_back(static_cast<char>(bytes[currentByte]));
        }

        while(currentByte < 32)
        {
            if(bytes[currentByte++] != std::byte{0x00}) throw std::invalid_argument("Malformed String32!");
        }

        if(!isValidString32(tupleValue)) throw std::invalid_argument("Decoded non-valid String32!");

        return tupleValue;
    }
    else throw std::invalid_argument("Could not match field type!");
}

FieldType decodeFieldType(std::byte& byte)
{
    std::uint8_t fieldTypeValue = static_cast<std::uint8_t>(byte);
    switch(fieldTypeValue)
    {
        case 0:
            return FieldType::INT64;
        case 1:
            return FieldType::STRING32;
    }
    throw std::invalid_argument("Could not match field type!");
}

std::vector<Field> decodeSchemaField(const std::vector<std::byte>& bytes)
{
    // First 2 bytes are the # of schema fields
    // Use that to determine bytes to read
    // std::size_t schemaFieldByteSize = 33 * # of fields 
}