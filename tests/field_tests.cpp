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

TEST(IntTypeSizeTest, ReturnsCorrectSize)
{
    EXPECT_EQ(getIntTypeSize(IntType::INT64), 8);
    EXPECT_EQ(getIntTypeSize(IntType::INT32), 4);
    EXPECT_EQ(getIntTypeSize(IntType::INT16), 2);
}

TEST(IntTypeSizeTest, RejectsInvalidType)
{
    EXPECT_THROW(getIntTypeSize(static_cast<IntType>(999)), std::invalid_argument);
}

TEST(EncodeIntTypeTest, EncodesZero)
{
    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int64_t{0}}),
        (std::vector<std::byte>(8, std::byte{0}))
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int32_t{0}}),
        (std::vector<std::byte>(4, std::byte{0}))
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int16_t{0}}),
        (std::vector<std::byte>(2, std::byte{0}))
    );
}

TEST(EncodeIntTypeTest, EncodesPositiveIntegersLittleEndian)
{
    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int64_t{0x0102030405060708LL}}),
        (std::vector<std::byte>{
            std::byte{0x08}, std::byte{0x07},
            std::byte{0x06}, std::byte{0x05},
            std::byte{0x04}, std::byte{0x03},
            std::byte{0x02}, std::byte{0x01}
        })
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int32_t{0x12345678}}),
        (std::vector<std::byte>{
            std::byte{0x78}, std::byte{0x56},
            std::byte{0x34}, std::byte{0x12}
        })
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int16_t{0x1234}}),
        (std::vector<std::byte>{
            std::byte{0x34}, std::byte{0x12}
        })
    );
}

TEST(EncodeIntTypeTest, EncodesNegativeOne)
{
    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int64_t{-1}}),
        (std::vector<std::byte>(8, std::byte{0xFF}))
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int32_t{-1}}),
        (std::vector<std::byte>(4, std::byte{0xFF}))
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int16_t{-1}}),
        (std::vector<std::byte>(2, std::byte{0xFF}))
    );
}

TEST(EncodeIntTypeTest, EncodesNegativeTwo)
{
    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int64_t{-2}}),
        (std::vector<std::byte>{
            std::byte{0xFE}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF}
        })
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int32_t{-2}}),
        (std::vector<std::byte>{
            std::byte{0xFE}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF}
        })
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{std::int16_t{-2}}),
        (std::vector<std::byte>{
            std::byte{0xFE}, std::byte{0xFF}
        })
    );
}

TEST(EncodeIntTypeTest, EncodesIntegerBoundaries)
{
    EXPECT_EQ(
        encodeIntType(SupportedIntType{
            std::numeric_limits<std::int64_t>::min()
        }),
        (std::vector<std::byte>{
            std::byte{0x00}, std::byte{0x00},
            std::byte{0x00}, std::byte{0x00},
            std::byte{0x00}, std::byte{0x00},
            std::byte{0x00}, std::byte{0x80}
        })
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{
            std::numeric_limits<std::int32_t>::max()
        }),
        (std::vector<std::byte>{
            std::byte{0xFF}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0x7F}
        })
    );

    EXPECT_EQ(
        encodeIntType(SupportedIntType{
            std::numeric_limits<std::int16_t>::min()
        }),
        (std::vector<std::byte>{
            std::byte{0x00}, std::byte{0x80}
        })
    );
}

TEST(DecodeIntTypeTest, DecodesPositiveIntegers)
{
    auto result64 = decodeIntType(
        {
            std::byte{0x08}, std::byte{0x07},
            std::byte{0x06}, std::byte{0x05},
            std::byte{0x04}, std::byte{0x03},
            std::byte{0x02}, std::byte{0x01}
        },
        IntType::INT64
    );

    ASSERT_TRUE(std::holds_alternative<std::int64_t>(result64));
    EXPECT_EQ(std::get<std::int64_t>(result64), 0x0102030405060708LL);

    auto result32 = decodeIntType(
        {
            std::byte{0x78}, std::byte{0x56},
            std::byte{0x34}, std::byte{0x12}
        },
        IntType::INT32
    );

    ASSERT_TRUE(std::holds_alternative<std::int32_t>(result32));
    EXPECT_EQ(std::get<std::int32_t>(result32), 0x12345678);

    auto result16 = decodeIntType(
        {std::byte{0x34}, std::byte{0x12}},
        IntType::INT16
    );

    ASSERT_TRUE(std::holds_alternative<std::int16_t>(result16));
    EXPECT_EQ(std::get<std::int16_t>(result16), 0x1234);
}

TEST(DecodeIntTypeTest, DecodesNegativeIntegers)
{
    auto result64 = decodeIntType(
        std::vector<std::byte>(8, std::byte{0xFF}),
        IntType::INT64
    );

    ASSERT_TRUE(std::holds_alternative<std::int64_t>(result64));
    EXPECT_EQ(std::get<std::int64_t>(result64), -1);

    auto result32 = decodeIntType(
        {
            std::byte{0xFE}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF}
        },
        IntType::INT32
    );

    ASSERT_TRUE(std::holds_alternative<std::int32_t>(result32));
    EXPECT_EQ(std::get<std::int32_t>(result32), -2);

    auto result16 = decodeIntType(
        {std::byte{0x00}, std::byte{0x80}},
        IntType::INT16
    );

    ASSERT_TRUE(std::holds_alternative<std::int16_t>(result16));
    EXPECT_EQ(
        std::get<std::int16_t>(result16),
        std::numeric_limits<std::int16_t>::min()
    );
}

TEST(DecodeIntTypeTest, RejectsIncorrectByteCounts)
{
    EXPECT_THROW(
        decodeIntType(std::vector<std::byte>(7), IntType::INT64),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntType(std::vector<std::byte>(9), IntType::INT64),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntType(std::vector<std::byte>(3), IntType::INT32),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntType(std::vector<std::byte>(5), IntType::INT32),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntType(std::vector<std::byte>(1), IntType::INT16),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntType(std::vector<std::byte>(3), IntType::INT16),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntType({}, IntType::INT16),
        std::invalid_argument
    );
}

TEST(DecodeIntTypeTest, RejectsInvalidType)
{
    EXPECT_THROW(
        decodeIntType(
            std::vector<std::byte>(8),
            static_cast<IntType>(999)
        ),
        std::invalid_argument
    );
}

TEST(IntTypeRoundTripTest, Int64Values)
{
    const std::vector<std::int64_t> values = {
        0,
        1,
        -1,
        123456789,
        -123456789,
        std::numeric_limits<std::int64_t>::min(),
        std::numeric_limits<std::int64_t>::max()
    };

    for(const auto value : values)
    {
        SupportedIntType original = value;

        auto encoded = encodeIntType(original);
        auto decoded = decodeIntType(encoded, IntType::INT64);

        ASSERT_TRUE(std::holds_alternative<std::int64_t>(decoded));
        EXPECT_EQ(std::get<std::int64_t>(decoded), value);
    }
}

TEST(IntTypeRoundTripTest, Int32Values)
{
    const std::vector<std::int32_t> values = {
        0,
        1,
        -1,
        123456,
        -123456,
        std::numeric_limits<std::int32_t>::min(),
        std::numeric_limits<std::int32_t>::max()
    };

    for(const auto value : values)
    {
        SupportedIntType original = value;

        auto encoded = encodeIntType(original);
        auto decoded = decodeIntType(encoded, IntType::INT32);

        ASSERT_TRUE(std::holds_alternative<std::int32_t>(decoded));
        EXPECT_EQ(std::get<std::int32_t>(decoded), value);
    }
}

TEST(IntTypeRoundTripTest, Int16Values)
{
    const std::vector<std::int16_t> values = {
        0,
        1,
        -1,
        12345,
        -12345,
        std::numeric_limits<std::int16_t>::min(),
        std::numeric_limits<std::int16_t>::max()
    };

    for(const auto value : values)
    {
        SupportedIntType original = value;

        auto encoded = encodeIntType(original);
        auto decoded = decodeIntType(encoded, IntType::INT16);

        ASSERT_TRUE(std::holds_alternative<std::int16_t>(decoded));
        EXPECT_EQ(std::get<std::int16_t>(decoded), value);
    }
}