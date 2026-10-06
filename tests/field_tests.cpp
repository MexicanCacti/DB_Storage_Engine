#include <gtest/gtest.h>
#include "field.hpp"

TEST(FieldTypeTests, GetCorrectSizes)
{
    EXPECT_EQ(getFieldTypeSize(FieldType::INT64), sizeof(std::int64_t));
    EXPECT_EQ(getFieldTypeSize(FieldType::STRING32), sizeof(char[32]));
}

TEST(String32ValidationTests, BadSizes)
{
    std::string string33Bytes(33, '0');
    EXPECT_FALSE(isValidString32(string33Bytes));
    std::string string32Bytes(32, '0');
    EXPECT_TRUE(isValidString32(string32Bytes));
}

TEST(String32ValidationTests, RejectEmbeddedZeroBytes)
{
    std::string embeddedString(32, '0');
    embeddedString[1] = '\0';
    EXPECT_FALSE(isValidString32(embeddedString));
}

TEST(String32ValidationTests, RejectSurrogates)
{
    std::string surrogateString{ static_cast<char>(0xED), static_cast<char>(0xA0), static_cast<char>(0x80)};   
    EXPECT_FALSE(isValidString32(surrogateString));

    surrogateString = { static_cast<char>(0xED), static_cast<char>(0xBF), static_cast<char>(0xBF)};
    EXPECT_FALSE(isValidString32(surrogateString));
}

TEST(String32ValidationTests, RejectOverlyLongTwoBytes)
{
    std::string overlyLongString{ static_cast<char>(0xC1), static_cast<char>(0x81)};
    EXPECT_FALSE(isValidString32(overlyLongString));
}

TEST(String32ValidationTests, RejectOverlyLongThreeBytes)
{
    std::string overlyLongString{ static_cast<char>(0xE0), static_cast<char>(0x81), static_cast<char>(0x81)};
    EXPECT_FALSE(isValidString32(overlyLongString));
}

TEST(String32ValidationTests, RejectOverlyLongFourBytes)
{
    std::string overlyLongString{ static_cast<char>(0xF0), static_cast<char>(0x80), static_cast<char>(0x80), static_cast<char>(0x80)};
    EXPECT_FALSE(isValidString32(overlyLongString));
}

TEST(String32ValidationTests, RejectOverUnicodeRange)
{
    std::string aboveRangeString{ static_cast<char>(0xF5), static_cast<char>(0x80), static_cast<char>(0x80), static_cast<char>(0x80)};
    EXPECT_FALSE(isValidString32(aboveRangeString));
}

TEST(String32ValidationTests, AcceptTwoByteBoundaries)
{
    std::string lowerBoundString{ static_cast<char>(0xC2), static_cast<char>(0x80) };
    std::string upperBoundString{ static_cast<char>(0xDF), static_cast<char>(0xBF) };
    EXPECT_TRUE(isValidString32(lowerBoundString));
    EXPECT_TRUE(isValidString32(upperBoundString));
}

TEST(String32ValidationTests, AcceptThreeByteBoundaries)
{
    std::string lowerBoundString{ static_cast<char>(0xE0), static_cast<char>(0xA0), static_cast<char>(0x80) };
    std::string upperBoundString{ static_cast<char>(0xEF), static_cast<char>(0xBF), static_cast<char>(0xBF) };
    EXPECT_TRUE(isValidString32(lowerBoundString));
    EXPECT_TRUE(isValidString32(upperBoundString));
}

TEST(String32ValidationTests, AcceptFourByteBoundaries)
{
    std::string lowerBoundString{ static_cast<char>(0xF0), static_cast<char>(0x90), static_cast<char>(0x80), static_cast<char>(0x80) };
    std::string upperBoundString{ static_cast<char>(0xF4), static_cast<char>(0x8F), static_cast<char>(0xBF), static_cast<char>(0xBF) };
    EXPECT_TRUE(isValidString32(lowerBoundString));
    EXPECT_TRUE(isValidString32(upperBoundString));
}

TEST(EncodeTests, INT64Encode)
{
    TupleValue val = std::int64_t(1);
    std::vector<std::byte> encodedBytes = encodeTupleValue(val);
    ASSERT_FALSE(encodedBytes.empty());
    ASSERT_EQ(encodedBytes.size(), 8);

    // Little endian, LSB first
    EXPECT_EQ(encodedBytes[0], std::byte{0x01});
    for(size_t currentByte = 1; currentByte < encodedBytes.size(); ++currentByte)
    {
        EXPECT_EQ(encodedBytes[currentByte], std::byte{0x00});
    }

    val = std::int64_t(-2);
    encodedBytes = encodeTupleValue(val);
    ASSERT_FALSE(encodedBytes.empty());
    ASSERT_EQ(encodedBytes.size(), 8);

    EXPECT_EQ(encodedBytes[0], std::byte{0xFE});
    for(size_t currentByte = 1; currentByte < encodedBytes.size(); ++currentByte)
    {
        EXPECT_EQ(encodedBytes[currentByte], std::byte{0xFF});
    }
}

TEST(EncodeTests, STRING32Encode)
{
    TupleValue val = std::string("A");
    std::vector<std::byte> encodedBytes = encodeTupleValue(val);
    ASSERT_FALSE(encodedBytes.empty());
    ASSERT_TRUE(encodedBytes.size() == 32);
    
    EXPECT_EQ(encodedBytes[0], std::byte{0x41});
    for(size_t currentByte = 1; currentByte < encodedBytes.size(); ++currentByte)
    {
        EXPECT_EQ(encodedBytes[currentByte], std::byte{0x00});
    }
}

TEST(EncodeTests, STRING32MultibyteEncode)
{
    TupleValue val = std::string("¥");
    std::vector<std::byte> encodedBytes = encodeTupleValue(val);
    ASSERT_FALSE(encodedBytes.empty());
    ASSERT_TRUE(encodedBytes.size() == 32);
    
    EXPECT_EQ(encodedBytes[0], std::byte{0xC2});
    EXPECT_EQ(encodedBytes[1], std::byte{0xA5});
    for(size_t currentByte = 2; currentByte < encodedBytes.size(); ++currentByte)
    {
        EXPECT_EQ(encodedBytes[currentByte], std::byte{0x00});
    }
}

TEST(EncodeTests, STRING32EmptyEncode)
{
    TupleValue val = std::string("");
    std::vector<std::byte> encodedBytes = encodeTupleValue(val);
    ASSERT_FALSE(encodedBytes.empty());
    ASSERT_TRUE(encodedBytes.size() == 32);
    for(size_t currentByte = 0; currentByte < encodedBytes.size(); ++currentByte)
    {
        EXPECT_EQ(encodedBytes[currentByte], std::byte{0x00});
    }
    
}

TEST(DecodeTests, INT64Decode)
{
    std::vector<std::byte> encodedBytes;
    // Stores 1 in little endian
    encodedBytes.push_back(std::byte{0x01});
    for(size_t i = 1; i < 8; ++i)
    {
        encodedBytes.push_back(std::byte{0x00});
    }

    TupleValue decodedValue = decodeBytes(encodedBytes, FieldType::INT64);

    ASSERT_TRUE(std::holds_alternative<std::int64_t>(decodedValue));
    EXPECT_EQ(std::get<std::int64_t>(decodedValue), 1);
}

TEST(DecodeTests, INT64DecodeNegative)
{
    // Also do the same for -2 ... so because signed, use two-complement... 1111 1110 .... FE FF FF FF ...
    std::vector<std::byte> encodedBytes;
    encodedBytes.push_back(std::byte{0xFE});
    for(size_t i = 1 ; i < 8 ; ++i)
    {
        encodedBytes.push_back(std::byte{0xFF});
    }

    TupleValue decodedValue = decodeBytes(encodedBytes, FieldType::INT64);
    ASSERT_TRUE(std::holds_alternative<std::int64_t>(decodedValue));
    EXPECT_EQ(std::get<std::int64_t>(decodedValue), -2);
}


TEST(DecodeTests, STRING32Decode)
{
    std::vector<std::byte> encodedBytes(32, std::byte{0x00});
    encodedBytes[0] = std::byte{'T'};
    encodedBytes[1] = std::byte{'E'};
    encodedBytes[2] = std::byte{'S'};
    encodedBytes[3] = std::byte{'T'};

    TupleValue decodedValue = decodeBytes(encodedBytes, FieldType::STRING32);

    ASSERT_TRUE(std::holds_alternative<std::string>(decodedValue));
    EXPECT_EQ(std::get<std::string>(decodedValue), "TEST");
}

TEST(DecodeTests, STRING32MaxByteDecode)
{
    std::string maxSize32String = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF";
    ASSERT_EQ(maxSize32String.size(), 32);

    std::vector<std::byte> encodedBytes(32);
    for(std::size_t i = 0 ; i < 32; ++i)
    {
        encodedBytes[i] = static_cast<std::byte>(maxSize32String[i]);
    }

    TupleValue decodedValue = decodeBytes(encodedBytes, FieldType::STRING32);

    ASSERT_TRUE(std::holds_alternative<std::string>(decodedValue));
    EXPECT_EQ(std::get<std::string>(decodedValue), maxSize32String);

}

TEST(DecodeTests, STRING32MultiByteDecode)
{
    std::vector<std::byte> encodedBytes(32, std::byte{0x00});
    encodedBytes[0] = std::byte{0xC2};
    encodedBytes[1] = std::byte{0xA5};

    TupleValue decodedValue = decodeBytes(encodedBytes, FieldType::STRING32);

    ASSERT_TRUE(std::holds_alternative<std::string>(decodedValue));
    EXPECT_EQ(std::get<std::string>(decodedValue), "¥");
}

TEST(DecodeTests, STRING32DecodeEmpty)
{
    std::vector<std::byte> encodedBytes(32, std::byte{0x00});

    TupleValue decodedValue = decodeBytes(encodedBytes, FieldType::STRING32);

    ASSERT_TRUE(std::holds_alternative<std::string>(decodedValue));
    EXPECT_EQ(std::get<std::string>(decodedValue), "");
}

TEST(RoundTripTests, INT64)
{
    TupleValue val = std::int64_t(2);
    std::vector<std::byte> encodedBytes = encodeTupleValue(val);
    TupleValue decodedVal = decodeBytes(encodedBytes, FieldType::INT64);

    EXPECT_EQ(std::get<std::int64_t>(val), std::get<std::int64_t>(decodedVal));
}

TEST(RoundTripTests, INT64Negative)
{
    TupleValue val = std::int64_t(-3);
    std::vector<std::byte> encodedBytes = encodeTupleValue(val);
    TupleValue decodedVal = decodeBytes(encodedBytes, FieldType::INT64);

    EXPECT_EQ(std::get<std::int64_t>(val), std::get<std::int64_t>(decodedVal));
}

TEST(RoundTripTests, STRING32)
{
    TupleValue stringVal = std::string("TEST");
    std::vector<std::byte> encodedBytes = encodeTupleValue(stringVal);
    TupleValue decodedVal = decodeBytes(encodedBytes, FieldType::STRING32);

    EXPECT_EQ(std::get<std::string>(stringVal), std::get<std::string>(decodedVal));
}

TEST(RoundTripTests, STRING32MaxByte)
{
    TupleValue stringVal = std::string("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEF");
    std::vector<std::byte> encodedBytes = encodeTupleValue(stringVal);
    TupleValue decodedVal = decodeBytes(encodedBytes, FieldType::STRING32);

    EXPECT_EQ(std::get<std::string>(stringVal), std::get<std::string>(decodedVal));
}

TEST(RoundTripTests, STRING32Empty)
{
    TupleValue stringVal = std::string("");
    std::vector<std::byte> encodedBytes = encodeTupleValue(stringVal);
    TupleValue decodedVal = decodeBytes(encodedBytes, FieldType::STRING32);

    EXPECT_EQ(std::get<std::string>(stringVal), std::get<std::string>(decodedVal));
}

TEST(RoundTripTests, STRING32MultiByte)
{
    TupleValue stringVal = std::string("¥");
    std::vector<std::byte> encodedBytes = encodeTupleValue(stringVal);
    TupleValue decodedVal = decodeBytes(encodedBytes, FieldType::STRING32);

    EXPECT_EQ(std::get<std::string>(stringVal), std::get<std::string>(decodedVal));
}