#include "basic_types.hpp"

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
        case IntType::INT8:
            return sizeof(std::int8_t);
    }
    throw std::invalid_argument("[getIntTypeSize]: Could not match intType arg");
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
    throw std::invalid_argument("[getFieldTypeSize]: Could not match fieldType arg");
}

bool isValidString32(const std::string& str)
{
    if(str.size() > 32) return false; // only 32 byte strings allowed w/ no silent shortening

    /*
    UTF-8 Char can be 1 -> 4 Bytes Long
    Source: https://www.rfc-editor.org/rfc/rfc3629.txt

      Char. number range|        UTF-8 octet sequence
      (hexadecimal)     |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx                             
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx                    
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx           
    
        1-byte: 0xxxxxxx = 0X00 ... So first bit should be 0 ... 10 000000
        2-byte: 110xxxxx = 0XC0 ... So first 2 bits should be 1 w/ 3rd 0 ... 111 00000 
        3-byte: 1110xxxx = 0XE0 ... So first 3 bits should be 1 w/ 4th 0 ... 1111 0000 
        4-byte: 11110xxx = 0xF0 ... So first 4 bits should be 1 w/ 5th 0 ... 11111 000
        Note: notice how they share the same continuation bytes
    */

    std::size_t index = 0;
    while(index < str.size())
    {
        std::uint8_t byte = static_cast<std::uint8_t>(str[index]);

        if(byte == 0x00) return false; // Embedded 0 byte

        // Get the byte sequence length
        std::size_t byteSequenceLength = 0;

        // See above table, basically just checking the leading 1 -> 4 bits to see if 1->4 byte char
        if( (byte & 0x80) == 0x00) byteSequenceLength = 1;
        else if( (byte & 0xE0) == 0xC0 ) byteSequenceLength = 2;
        else if( (byte & 0xF0) == 0xE0 ) byteSequenceLength = 3;
        else if( (byte & 0xF8) == 0xF0 ) byteSequenceLength = 4;
        else return false; // Leading byte doesn't match the 1->4 byte leading pattern

        if(index + byteSequenceLength > str.size()) return false; // Out of bounds, something is funky with the string!

        // Check structure of next bytes in sequence for correctness
        std::uint8_t nextByte = byte;
        for(std::size_t next = 1; next < byteSequenceLength; ++next)
        {
            nextByte = static_cast<std::uint8_t>(str[index + next]);
            if(nextByte == 0x00 || ( (nextByte & 0xC0) != 0x80)) return false; // Embedded 0 or not correct format (10xxxxxx)
        }

        /*
            Checking for invalid UTF-8 Ranges / Overly long
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
            nextByte = static_cast<std::uint8_t>(str[index + 1]);

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

            nextByte = static_cast<std::uint8_t>(str[index + 1]);

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

            nextByte = static_cast<std::uint8_t>(str[index + 1]);

            if(byte == 0xF0 && nextByte < 0x90) return false;
            
            if(byte == 0xF4 && nextByte > 0x8F) return false;

            if(byte > 0xF4) return false;

        }

        index += byteSequenceLength;
    }
    
    return true;
}