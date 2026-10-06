#include "field.hpp"

#include <stdlib.h>

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

// Throws invalid_argument if tupleValue doesn't match a variant
std::vector<std::byte> encodeTupleValue(const TupleValue& tupleValue)
{
    if(std::holds_alternative<std::int64_t>(tupleValue))
    {
        std::int64_t int64TupleVal = std::get<std::int64_t>(tupleValue);
        std::uint64_t uint64TupleVal = static_cast<std::uint64_t>(int64TupleVal); // Needed for encoding correctly!
        std::vector<std::byte> byteArray(8, std::byte{0}); // 8 Byte empty array all init to 0
        for(std::size_t currentByte = 0 ; currentByte < 8; ++currentByte)
        {
            byteArray[currentByte] = static_cast<std::byte>(int64TupleVal >> (currentByte * 8) & 0xFF);
        }

        return byteArray;
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

TupleValue decodeBytes(const std::vector<std::byte>& bytes, FieldType fieldType)
{
    if(fieldType == FieldType::INT64)
    {
        std::uint64_t value = 0; // Bit-wise, everything is set to 0

        for(std::size_t currentByte = 0 ; currentByte < 8; ++currentByte)
        {
            std::uint64_t byteValue = static_cast<std::uint64_t>(bytes[currentByte]);
            byteValue = byteValue << (currentByte * 8); // Want to shift the byteValue to the left s.t. it get's OR'd into its correct position
            
            // Because every bit in value is default set to 0, any OR with a retrieved byte will set the bit... meaning we get the correct bytes
            value |= byteValue;
        }

        std::int64_t tupleValue;
        memcpy(&tupleValue, &value, sizeof(std::int64_t));
        return tupleValue;
    }
    else if(fieldType == FieldType::STRING32)
    {
        std::string tupleValue = "";
        for(std::size_t currentByte = 0 ; currentByte < 32; ++currentByte)
        {
            if(bytes[currentByte] == std::byte{0x00}) break; // Beginning of trailing padding, we don't wanted embedded zeroes
            tupleValue.push_back(static_cast<char>(bytes[currentByte]));
        }

        return tupleValue;
    }
    else throw std::invalid_argument("Could not match field type!");
}