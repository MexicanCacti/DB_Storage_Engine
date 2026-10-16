
#include <gtest/gtest.h>
#include "basic_types.hpp"

// ==========================================
// FieldType Tests
// ==========================================

TEST(FieldTypeTests, GetCorrectSizes)
{
    EXPECT_EQ(getFieldTypeSize(FieldType::INT64), sizeof(std::int64_t));
    EXPECT_EQ(getFieldTypeSize(FieldType::STRING32), sizeof(char[32]));
}

TEST(FieldTypeTests, RejectInvalidFieldType)
{
    EXPECT_THROW(getFieldTypeSize(static_cast<FieldType>(-1)), std::invalid_argument);
    EXPECT_THROW(getFieldTypeSize(static_cast<FieldType>(2)), std::invalid_argument);
    EXPECT_THROW(getFieldTypeSize(static_cast<FieldType>(127)), std::invalid_argument);
}

// ==========================================
// IntType Tests
// ==========================================

TEST(IntTypeTests, GetCorrectSizes)
{
    EXPECT_EQ(getIntTypeSize(IntType::INT64), sizeof(std::int64_t));
    EXPECT_EQ(getIntTypeSize(IntType::INT32), sizeof(std::int32_t));
    EXPECT_EQ(getIntTypeSize(IntType::INT16), sizeof(std::int16_t));
    EXPECT_EQ(getIntTypeSize(IntType::INT8), sizeof(std::int8_t));
}

TEST(IntTypeTests, RejectInvalidIntType)
{
    EXPECT_THROW(getIntTypeSize(static_cast<IntType>(-1)), std::invalid_argument);
    EXPECT_THROW(getIntTypeSize(static_cast<IntType>(4)), std::invalid_argument);
    EXPECT_THROW(getIntTypeSize(static_cast<IntType>(127)), std::invalid_argument);
}

// ==========================================
// String32 Size Tests
// ==========================================

TEST(String32ValidationTests, BadSizes)
{
    std::string string33Bytes(33, '0');
    EXPECT_FALSE(isValidString32(string33Bytes));

    std::string string32Bytes(32, '0');
    EXPECT_TRUE(isValidString32(string32Bytes));
}

TEST(String32ValidationTests, AcceptEmptyString)
{
    std::string emptyString;
    EXPECT_TRUE(isValidString32(emptyString));
}

TEST(String32ValidationTests, AcceptShortASCIIStrings)
{
    EXPECT_TRUE(isValidString32("A"));
    EXPECT_TRUE(isValidString32("Hello"));
    EXPECT_TRUE(isValidString32("Hello World!"));
    EXPECT_TRUE(isValidString32("123456789"));
}

TEST(String32ValidationTests, RejectEmbeddedZeroBytes)
{
    std::string embeddedString(32, '0');
    embeddedString[1] = '\0';
    EXPECT_FALSE(isValidString32(embeddedString));

    embeddedString = std::string("Hello\0World", 11);
    EXPECT_FALSE(isValidString32(embeddedString));
}

TEST(String32ValidationTests, RejectSingleZeroByte)
{
    std::string zeroString(1, '\0');
    EXPECT_FALSE(isValidString32(zeroString));
}

// ==========================================
// Valid UTF-8 Sequence Tests
// ==========================================

TEST(String32ValidationTests, AcceptValidOneByteSequences)
{
    EXPECT_TRUE(isValidString32(std::string{static_cast<char>(0x01)}));
    EXPECT_TRUE(isValidString32(std::string{static_cast<char>(0x7F)}));
}

TEST(String32ValidationTests, AcceptValidTwoByteSequences)
{
    // Lowest valid 2-byte code point: U+0080
    std::string lowest{
        static_cast<char>(0xC2),
        static_cast<char>(0x80)
    };
    EXPECT_TRUE(isValidString32(lowest));

    // Highest valid 2-byte code point: U+07FF
    std::string highest{
        static_cast<char>(0xDF),
        static_cast<char>(0xBF)
    };
    EXPECT_TRUE(isValidString32(highest));
}

TEST(String32ValidationTests, AcceptValidThreeByteSequences)
{
    // Lowest valid 3-byte code point: U+0800
    std::string lowest{
        static_cast<char>(0xE0),
        static_cast<char>(0xA0),
        static_cast<char>(0x80)
    };
    EXPECT_TRUE(isValidString32(lowest));

    // Highest valid 3-byte code point: U+FFFF
    std::string highest{
        static_cast<char>(0xEF),
        static_cast<char>(0xBF),
        static_cast<char>(0xBF)
    };
    EXPECT_TRUE(isValidString32(highest));

    // Valid sequence immediately before surrogate range
    std::string beforeSurrogates{
        static_cast<char>(0xED),
        static_cast<char>(0x9F),
        static_cast<char>(0xBF)
    };
    EXPECT_TRUE(isValidString32(beforeSurrogates));

    // Valid sequence immediately after surrogate range
    std::string afterSurrogates{
        static_cast<char>(0xEE),
        static_cast<char>(0x80),
        static_cast<char>(0x80)
    };
    EXPECT_TRUE(isValidString32(afterSurrogates));
}

TEST(String32ValidationTests, AcceptValidFourByteSequences)
{
    // Lowest valid 4-byte code point: U+10000
    std::string lowest{
        static_cast<char>(0xF0),
        static_cast<char>(0x90),
        static_cast<char>(0x80),
        static_cast<char>(0x80)
    };
    EXPECT_TRUE(isValidString32(lowest));

    // Highest valid Unicode code point: U+10FFFF
    std::string highest{
        static_cast<char>(0xF4),
        static_cast<char>(0x8F),
        static_cast<char>(0xBF),
        static_cast<char>(0xBF)
    };
    EXPECT_TRUE(isValidString32(highest));
}

TEST(String32ValidationTests, AcceptMixedUTF8Sequences)
{
    std::string mixed{
        'A',
        static_cast<char>(0xC2), static_cast<char>(0xA2),
        static_cast<char>(0xE2), static_cast<char>(0x82), static_cast<char>(0xAC),
        static_cast<char>(0xF0), static_cast<char>(0x9F),
        static_cast<char>(0x98), static_cast<char>(0x80),
        'Z'
    };

    EXPECT_TRUE(isValidString32(mixed));
}

TEST(String32ValidationTests, AcceptExactly32BytesOfUTF8)
{
    // Eight valid 4-byte characters = 32 bytes
    std::string fourByteCharacter{
        static_cast<char>(0xF0),
        static_cast<char>(0x9F),
        static_cast<char>(0x98),
        static_cast<char>(0x80)
    };

    std::string string32Bytes;
    for(int i = 0; i < 8; ++i)
    {
        string32Bytes += fourByteCharacter;
    }

    EXPECT_EQ(string32Bytes.size(), 32);
    EXPECT_TRUE(isValidString32(string32Bytes));
}

TEST(String32ValidationTests, Reject33BytesOfValidUTF8)
{
    std::string fourByteCharacter{
        static_cast<char>(0xF0),
        static_cast<char>(0x9F),
        static_cast<char>(0x98),
        static_cast<char>(0x80)
    };

    std::string string33Bytes = "A";
    for(int i = 0; i < 8; ++i)
    {
        string33Bytes += fourByteCharacter;
    }

    EXPECT_EQ(string33Bytes.size(), 33);
    EXPECT_FALSE(isValidString32(string33Bytes));
}

// ==========================================
// Invalid UTF-8 Leading Byte Tests
// ==========================================

TEST(String32ValidationTests, RejectStandaloneContinuationBytes)
{
    EXPECT_FALSE(isValidString32(std::string{static_cast<char>(0x80)}));
    EXPECT_FALSE(isValidString32(std::string{static_cast<char>(0xA0)}));
    EXPECT_FALSE(isValidString32(std::string{static_cast<char>(0xBF)}));
}

TEST(String32ValidationTests, RejectInvalidLeadingBytes)
{
    EXPECT_FALSE(isValidString32(std::string{static_cast<char>(0xF8)}));
    EXPECT_FALSE(isValidString32(std::string{static_cast<char>(0xFC)}));
    EXPECT_FALSE(isValidString32(std::string{static_cast<char>(0xFF)}));
}

// ==========================================
// Invalid Continuation Byte Tests
// ==========================================

TEST(String32ValidationTests, RejectInvalidTwoByteContinuation)
{
    std::string invalid{
        static_cast<char>(0xC2),
        static_cast<char>(0x41)
    };
    EXPECT_FALSE(isValidString32(invalid));

    invalid = {
        static_cast<char>(0xC2),
        static_cast<char>(0xC2)
    };
    EXPECT_FALSE(isValidString32(invalid));
}

TEST(String32ValidationTests, RejectInvalidThreeByteContinuation)
{
    // Invalid second byte
    std::string invalid{
        static_cast<char>(0xE2),
        static_cast<char>(0x41),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(invalid));

    // Invalid third byte
    invalid = {
        static_cast<char>(0xE2),
        static_cast<char>(0x82),
        static_cast<char>(0x41)
    };
    EXPECT_FALSE(isValidString32(invalid));
}

TEST(String32ValidationTests, RejectInvalidFourByteContinuation)
{
    // Invalid second byte
    std::string invalid{
        static_cast<char>(0xF0),
        static_cast<char>(0x41),
        static_cast<char>(0x80),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(invalid));

    // Invalid third byte
    invalid = {
        static_cast<char>(0xF0),
        static_cast<char>(0x90),
        static_cast<char>(0x41),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(invalid));

    // Invalid fourth byte
    invalid = {
        static_cast<char>(0xF0),
        static_cast<char>(0x90),
        static_cast<char>(0x80),
        static_cast<char>(0x41)
    };
    EXPECT_FALSE(isValidString32(invalid));
}

// ==========================================
// Truncated UTF-8 Sequence Tests
// ==========================================

TEST(String32ValidationTests, RejectTruncatedTwoByteSequence)
{
    std::string truncated{
        static_cast<char>(0xC2)
    };
    EXPECT_FALSE(isValidString32(truncated));
}

TEST(String32ValidationTests, RejectTruncatedThreeByteSequence)
{
    std::string truncated{
        static_cast<char>(0xE2)
    };
    EXPECT_FALSE(isValidString32(truncated));

    truncated = {
        static_cast<char>(0xE2),
        static_cast<char>(0x82)
    };
    EXPECT_FALSE(isValidString32(truncated));
}

TEST(String32ValidationTests, RejectTruncatedFourByteSequence)
{
    std::string truncated{
        static_cast<char>(0xF0)
    };
    EXPECT_FALSE(isValidString32(truncated));

    truncated = {
        static_cast<char>(0xF0),
        static_cast<char>(0x90)
    };
    EXPECT_FALSE(isValidString32(truncated));

    truncated = {
        static_cast<char>(0xF0),
        static_cast<char>(0x90),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(truncated));
}

// ==========================================
// Overlong UTF-8 Encoding Tests
// ==========================================

TEST(String32ValidationTests, RejectOverlyLongTwoBytes)
{
    std::string overlyLongString{
        static_cast<char>(0xC1),
        static_cast<char>(0x81)
    };
    EXPECT_FALSE(isValidString32(overlyLongString));

    overlyLongString = {
        static_cast<char>(0xC0),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(overlyLongString));
}

TEST(String32ValidationTests, RejectOverlyLongThreeBytes)
{
    std::string overlyLongString{
        static_cast<char>(0xE0),
        static_cast<char>(0x81),
        static_cast<char>(0x81)
    };
    EXPECT_FALSE(isValidString32(overlyLongString));

    overlyLongString = {
        static_cast<char>(0xE0),
        static_cast<char>(0x9F),
        static_cast<char>(0xBF)
    };
    EXPECT_FALSE(isValidString32(overlyLongString));
}

TEST(String32ValidationTests, RejectOverlyLongFourBytes)
{
    std::string overlyLongString{
        static_cast<char>(0xF0),
        static_cast<char>(0x80),
        static_cast<char>(0x80),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(overlyLongString));

    overlyLongString = {
        static_cast<char>(0xF0),
        static_cast<char>(0x8F),
        static_cast<char>(0xBF),
        static_cast<char>(0xBF)
    };
    EXPECT_FALSE(isValidString32(overlyLongString));
}

// ==========================================
// Unicode Surrogate Tests
// ==========================================

TEST(String32ValidationTests, RejectSurrogates)
{
    // First surrogate: U+D800
    std::string surrogateString{
        static_cast<char>(0xED),
        static_cast<char>(0xA0),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(surrogateString));

    // Last surrogate: U+DFFF
    surrogateString = {
        static_cast<char>(0xED),
        static_cast<char>(0xBF),
        static_cast<char>(0xBF)
    };
    EXPECT_FALSE(isValidString32(surrogateString));
}

TEST(String32ValidationTests, RejectMiddleSurrogate)
{
    std::string surrogateString{
        static_cast<char>(0xED),
        static_cast<char>(0xB0),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(surrogateString));
}

// ==========================================
// Unicode Range Tests
// ==========================================

TEST(String32ValidationTests, RejectOverUnicodeRange)
{
    std::string aboveRangeString{
        static_cast<char>(0xF5),
        static_cast<char>(0x80),
        static_cast<char>(0x80),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(aboveRangeString));
}

TEST(String32ValidationTests, RejectImmediatelyAboveUnicodeRange)
{
    // U+110000, first code point above U+10FFFF
    std::string aboveRangeString{
        static_cast<char>(0xF4),
        static_cast<char>(0x90),
        static_cast<char>(0x80),
        static_cast<char>(0x80)
    };
    EXPECT_FALSE(isValidString32(aboveRangeString));
}

TEST(String32ValidationTests, RejectMaximumInvalidLeadingByte)
{
    std::string invalid{
        static_cast<char>(0xFF),
        static_cast<char>(0xBF),
        static_cast<char>(0xBF),
        static_cast<char>(0xBF)
    };
    EXPECT_FALSE(isValidString32(invalid));
}

// ==========================================
// Mixed Valid and Invalid UTF-8 Tests
// ==========================================

TEST(String32ValidationTests, RejectInvalidSequenceAfterValidASCII)
{
    std::string mixed = "Hello";

    mixed += static_cast<char>(0xC0);
    mixed += static_cast<char>(0x80);

    EXPECT_FALSE(isValidString32(mixed));
}

TEST(String32ValidationTests, RejectInvalidSequenceBetweenValidCharacters)
{
    std::string mixed = "ABC";

    mixed += static_cast<char>(0xED);
    mixed += static_cast<char>(0xA0);
    mixed += static_cast<char>(0x80);

    mixed += "XYZ";

    EXPECT_FALSE(isValidString32(mixed));
}

TEST(String32ValidationTests, RejectTruncatedSequenceAtEnd)
{
    std::string mixed = "Hello World";

    mixed += static_cast<char>(0xE2);
    mixed += static_cast<char>(0x82);

    EXPECT_FALSE(isValidString32(mixed));
}
