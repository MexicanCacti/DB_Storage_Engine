
#include <gtest/gtest.h>
#include "basic_types_encode_decode.hpp"

// ==========================================
// encodeIntVariant Tests
// ==========================================

TEST(EncodeIntVariantTests, EncodeInt64)
{
    IntVariant value = std::int64_t{0x0102030405060708LL};

    std::vector<std::byte> expected{
        std::byte{0x08}, std::byte{0x07},
        std::byte{0x06}, std::byte{0x05},
        std::byte{0x04}, std::byte{0x03},
        std::byte{0x02}, std::byte{0x01}
    };

    EXPECT_EQ(encodeIntVariant(value), expected);
}

TEST(EncodeIntVariantTests, EncodeInt32)
{
    IntVariant value = std::int32_t{0x01020304};

    std::vector<std::byte> expected{
        std::byte{0x04}, std::byte{0x03},
        std::byte{0x02}, std::byte{0x01}
    };

    EXPECT_EQ(encodeIntVariant(value), expected);
}

TEST(EncodeIntVariantTests, EncodeInt16)
{
    IntVariant value = std::int16_t{0x0102};

    std::vector<std::byte> expected{
        std::byte{0x02}, std::byte{0x01}
    };

    EXPECT_EQ(encodeIntVariant(value), expected);
}

TEST(EncodeIntVariantTests, EncodeInt8)
{
    IntVariant value = std::int8_t{0x7F};

    std::vector<std::byte> expected{
        std::byte{0x7F}
    };

    EXPECT_EQ(encodeIntVariant(value), expected);
}

TEST(EncodeIntVariantTests, EncodeNegativeIntegers)
{
    EXPECT_EQ(
        encodeIntVariant(IntVariant{std::int64_t{-1}}),
        std::vector<std::byte>(8, std::byte{0xFF})
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{std::int32_t{-1}}),
        std::vector<std::byte>(4, std::byte{0xFF})
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{std::int16_t{-1}}),
        std::vector<std::byte>(2, std::byte{0xFF})
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{std::int8_t{-1}}),
        std::vector<std::byte>(1, std::byte{0xFF})
    );
}

TEST(EncodeIntVariantTests, EncodeZero)
{
    EXPECT_EQ(
        encodeIntVariant(IntVariant{std::int64_t{0}}),
        std::vector<std::byte>(8, std::byte{0})
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{std::int32_t{0}}),
        std::vector<std::byte>(4, std::byte{0})
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{std::int16_t{0}}),
        std::vector<std::byte>(2, std::byte{0})
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{std::int8_t{0}}),
        std::vector<std::byte>(1, std::byte{0})
    );
}

TEST(EncodeIntVariantTests, EncodeMinimumValues)
{
    EXPECT_EQ(
        encodeIntVariant(IntVariant{
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
        encodeIntVariant(IntVariant{
            std::numeric_limits<std::int32_t>::min()
        }),
        (std::vector<std::byte>{
            std::byte{0x00}, std::byte{0x00},
            std::byte{0x00}, std::byte{0x80}
        })
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{
            std::numeric_limits<std::int16_t>::min()
        }),
        (std::vector<std::byte>{
            std::byte{0x00}, std::byte{0x80}
        })
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{
            std::numeric_limits<std::int8_t>::min()
        }),
        (std::vector<std::byte>{std::byte{0x80}})
    );
}

TEST(EncodeIntVariantTests, EncodeMaximumValues)
{
    EXPECT_EQ(
        encodeIntVariant(IntVariant{
            std::numeric_limits<std::int64_t>::max()
        }),
        (std::vector<std::byte>{
            std::byte{0xFF}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0x7F}
        })
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{
            std::numeric_limits<std::int32_t>::max()
        }),
        (std::vector<std::byte>{
            std::byte{0xFF}, std::byte{0xFF},
            std::byte{0xFF}, std::byte{0x7F}
        })
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{
            std::numeric_limits<std::int16_t>::max()
        }),
        (std::vector<std::byte>{
            std::byte{0xFF}, std::byte{0x7F}
        })
    );

    EXPECT_EQ(
        encodeIntVariant(IntVariant{
            std::numeric_limits<std::int8_t>::max()
        }),
        (std::vector<std::byte>{std::byte{0x7F}})
    );
}

// ==========================================
// decodeIntVariant Tests
// ==========================================

TEST(DecodeIntVariantTests, DecodeInt64)
{
    std::vector<std::byte> bytes{
        std::byte{0x08}, std::byte{0x07},
        std::byte{0x06}, std::byte{0x05},
        std::byte{0x04}, std::byte{0x03},
        std::byte{0x02}, std::byte{0x01}
    };

    IntVariant decoded = decodeIntVariant(bytes, IntType::INT64);

    ASSERT_TRUE(std::holds_alternative<std::int64_t>(decoded));
    EXPECT_EQ(std::get<std::int64_t>(decoded), 0x0102030405060708LL);
}

TEST(DecodeIntVariantTests, DecodeInt32)
{
    std::vector<std::byte> bytes{
        std::byte{0x04}, std::byte{0x03},
        std::byte{0x02}, std::byte{0x01}
    };

    IntVariant decoded = decodeIntVariant(bytes, IntType::INT32);

    ASSERT_TRUE(std::holds_alternative<std::int32_t>(decoded));
    EXPECT_EQ(std::get<std::int32_t>(decoded), 0x01020304);
}

TEST(DecodeIntVariantTests, DecodeInt16)
{
    std::vector<std::byte> bytes{
        std::byte{0x02}, std::byte{0x01}
    };

    IntVariant decoded = decodeIntVariant(bytes, IntType::INT16);

    ASSERT_TRUE(std::holds_alternative<std::int16_t>(decoded));
    EXPECT_EQ(std::get<std::int16_t>(decoded), 0x0102);
}

TEST(DecodeIntVariantTests, DecodeInt8)
{
    std::vector<std::byte> bytes{std::byte{0x7F}};

    IntVariant decoded = decodeIntVariant(bytes, IntType::INT8);

    ASSERT_TRUE(std::holds_alternative<std::int8_t>(decoded));
    EXPECT_EQ(std::get<std::int8_t>(decoded), 127);
}

TEST(DecodeIntVariantTests, DecodeNegativeOne)
{
    EXPECT_EQ(
        std::get<std::int64_t>(decodeIntVariant(
            std::vector<std::byte>(8, std::byte{0xFF}),
            IntType::INT64)),
        -1
    );

    EXPECT_EQ(
        std::get<std::int32_t>(decodeIntVariant(
            std::vector<std::byte>(4, std::byte{0xFF}),
            IntType::INT32)),
        -1
    );

    EXPECT_EQ(
        std::get<std::int16_t>(decodeIntVariant(
            std::vector<std::byte>(2, std::byte{0xFF}),
            IntType::INT16)),
        -1
    );

    EXPECT_EQ(
        std::get<std::int8_t>(decodeIntVariant(
            std::vector<std::byte>(1, std::byte{0xFF}),
            IntType::INT8)),
        -1
    );
}

TEST(DecodeIntVariantTests, RejectIncorrectByteLengths)
{
    EXPECT_THROW(
        decodeIntVariant(std::vector<std::byte>(7), IntType::INT64),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntVariant(std::vector<std::byte>(5), IntType::INT32),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntVariant(std::vector<std::byte>(1), IntType::INT16),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntVariant(std::vector<std::byte>(2), IntType::INT8),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntVariant({}, IntType::INT64),
        std::invalid_argument
    );
}

TEST(DecodeIntVariantTests, RejectInvalidIntType)
{
    std::vector<std::byte> bytes(8, std::byte{0});

    EXPECT_THROW(
        decodeIntVariant(bytes, static_cast<IntType>(100)),
        std::invalid_argument
    );
}

// ==========================================
// Integer Round-Trip Tests
// ==========================================

TEST(IntVariantRoundTripTests, Int64Boundaries)
{
    std::vector<std::int64_t> values{
        0,
        1,
        -1,
        123456789012345LL,
        -123456789012345LL,
        std::numeric_limits<std::int64_t>::min(),
        std::numeric_limits<std::int64_t>::max()
    };

    for(std::int64_t value : values)
    {
        IntVariant original = value;

        auto encoded = encodeIntVariant(original);
        auto decoded = decodeIntVariant(encoded, IntType::INT64);

        ASSERT_TRUE(std::holds_alternative<std::int64_t>(decoded));
        EXPECT_EQ(std::get<std::int64_t>(decoded), value);
    }
}

TEST(IntVariantRoundTripTests, Int32Boundaries)
{
    std::vector<std::int32_t> values{
        0, 1, -1, 123456, -123456,
        std::numeric_limits<std::int32_t>::min(),
        std::numeric_limits<std::int32_t>::max()
    };

    for(std::int32_t value : values)
    {
        auto encoded = encodeIntVariant(IntVariant{value});
        auto decoded = decodeIntVariant(encoded, IntType::INT32);

        ASSERT_TRUE(std::holds_alternative<std::int32_t>(decoded));
        EXPECT_EQ(std::get<std::int32_t>(decoded), value);
    }
}

TEST(IntVariantRoundTripTests, Int16Boundaries)
{
    std::vector<std::int16_t> values{
        0, 1, -1, 1234, -1234,
        std::numeric_limits<std::int16_t>::min(),
        std::numeric_limits<std::int16_t>::max()
    };

    for(std::int16_t value : values)
    {
        auto encoded = encodeIntVariant(IntVariant{value});
        auto decoded = decodeIntVariant(encoded, IntType::INT16);

        ASSERT_TRUE(std::holds_alternative<std::int16_t>(decoded));
        EXPECT_EQ(std::get<std::int16_t>(decoded), value);
    }
}

TEST(IntVariantRoundTripTests, Int8Boundaries)
{
    std::vector<std::int8_t> values{
        0, 1, -1, 42, -42,
        std::numeric_limits<std::int8_t>::min(),
        std::numeric_limits<std::int8_t>::max()
    };

    for(std::int8_t value : values)
    {
        auto encoded = encodeIntVariant(IntVariant{value});
        auto decoded = decodeIntVariant(encoded, IntType::INT8);

        ASSERT_TRUE(std::holds_alternative<std::int8_t>(decoded));
        EXPECT_EQ(std::get<std::int8_t>(decoded), value);
    }
}

// ==========================================
// encodeFieldType / decodeFieldType Tests
// ==========================================

TEST(FieldTypeEncodingTests, EncodeValidTypes)
{
    EXPECT_EQ(encodeFieldType(FieldType::INT64), std::byte{0x00});
    EXPECT_EQ(encodeFieldType(FieldType::STRING32), std::byte{0x01});
}

TEST(FieldTypeEncodingTests, DecodeValidTypes)
{
    EXPECT_EQ(decodeFieldType(std::byte{0x00}), FieldType::INT64);
    EXPECT_EQ(decodeFieldType(std::byte{0x01}), FieldType::STRING32);
}

TEST(FieldTypeEncodingTests, RoundTrip)
{
    for(FieldType type : {FieldType::INT64, FieldType::STRING32})
    {
        EXPECT_EQ(decodeFieldType(encodeFieldType(type)), type);
    }
}

TEST(FieldTypeEncodingTests, RejectInvalidTypes)
{
    EXPECT_THROW(
        encodeFieldType(static_cast<FieldType>(2)),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeFieldType(std::byte{0x02}),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeFieldType(std::byte{0xFF}),
        std::invalid_argument
    );
}

// ==========================================
// encodeIntType / decodeIntType Tests
// ==========================================

TEST(IntTypeEncodingTests, EncodeValidTypes)
{
    EXPECT_EQ(encodeIntType(IntType::INT64), std::byte{0x00});
    EXPECT_EQ(encodeIntType(IntType::INT32), std::byte{0x01});
    EXPECT_EQ(encodeIntType(IntType::INT16), std::byte{0x02});
    EXPECT_EQ(encodeIntType(IntType::INT8), std::byte{0x03});
}

TEST(IntTypeEncodingTests, DecodeValidTypes)
{
    EXPECT_EQ(decodeIntType(std::byte{0x00}), IntType::INT64);
    EXPECT_EQ(decodeIntType(std::byte{0x01}), IntType::INT32);
    EXPECT_EQ(decodeIntType(std::byte{0x02}), IntType::INT16);
    EXPECT_EQ(decodeIntType(std::byte{0x03}), IntType::INT8);
}

TEST(IntTypeEncodingTests, RoundTrip)
{
    for(IntType type : {
        IntType::INT64,
        IntType::INT32,
        IntType::INT16,
        IntType::INT8
    })
    {
        EXPECT_EQ(decodeIntType(encodeIntType(type)), type);
    }
}

TEST(IntTypeEncodingTests, RejectInvalidTypes)
{
    EXPECT_THROW(
        encodeIntType(static_cast<IntType>(4)),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntType(std::byte{0x04}),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeIntType(std::byte{0xFF}),
        std::invalid_argument
    );
}

// ==========================================
// encodeFieldVariant Tests
// ==========================================

TEST(EncodeFieldVariantTests, EncodeInt64)
{
    FieldVariant value = std::int64_t{123456};

    auto encoded = encodeFieldVariant(value);

    EXPECT_EQ(encoded.size(), 8);
    EXPECT_EQ(
        std::get<std::int64_t>(
            decodeFieldVariant(encoded, FieldType::INT64)
        ),
        123456
    );
}

TEST(EncodeFieldVariantTests, EncodeEmptyString)
{
    FieldVariant value = std::string{};

    auto encoded = encodeFieldVariant(value);

    EXPECT_EQ(encoded.size(), 32);
    EXPECT_EQ(encoded, std::vector<std::byte>(32, std::byte{0}));
}

TEST(EncodeFieldVariantTests, EncodeShortString)
{
    FieldVariant value = std::string{"ABC"};

    auto encoded = encodeFieldVariant(value);

    ASSERT_EQ(encoded.size(), 32);

    EXPECT_EQ(encoded[0], std::byte{0x41});
    EXPECT_EQ(encoded[1], std::byte{0x42});
    EXPECT_EQ(encoded[2], std::byte{0x43});

    for(std::size_t i = 3; i < 32; ++i)
    {
        EXPECT_EQ(encoded[i], std::byte{0x00});
    }
}

TEST(EncodeFieldVariantTests, EncodeExactly32Bytes)
{
    std::string value(32, 'A');

    auto encoded = encodeFieldVariant(FieldVariant{value});

    ASSERT_EQ(encoded.size(), 32);

    for(std::byte byte : encoded)
    {
        EXPECT_EQ(byte, std::byte{0x41});
    }
}

TEST(EncodeFieldVariantTests, Reject33ByteString)
{
    std::string value(33, 'A');

    EXPECT_THROW(
        encodeFieldVariant(FieldVariant{value}),
        std::invalid_argument
    );
}

TEST(EncodeFieldVariantTests, RejectEmbeddedZero)
{
    std::string value = std::string("AB\0CD", 5);

    EXPECT_THROW(
        encodeFieldVariant(FieldVariant{value}),
        std::invalid_argument
    );
}

TEST(EncodeFieldVariantTests, RejectInvalidUTF8)
{
    std::string invalid{
        static_cast<char>(0xC0),
        static_cast<char>(0x80)
    };

    EXPECT_THROW(
        encodeFieldVariant(FieldVariant{invalid}),
        std::invalid_argument
    );
}

// ==========================================
// decodeFieldVariant Tests
// ==========================================

TEST(DecodeFieldVariantTests, DecodeInt64)
{
    auto encoded = encodeFieldVariant(
        FieldVariant{std::int64_t{-12345}}
    );

    auto decoded = decodeFieldVariant(encoded, FieldType::INT64);

    ASSERT_TRUE(std::holds_alternative<std::int64_t>(decoded));
    EXPECT_EQ(std::get<std::int64_t>(decoded), -12345);
}

TEST(DecodeFieldVariantTests, DecodeEmptyString)
{
    std::vector<std::byte> bytes(32, std::byte{0});

    auto decoded = decodeFieldVariant(bytes, FieldType::STRING32);

    ASSERT_TRUE(std::holds_alternative<std::string>(decoded));
    EXPECT_EQ(std::get<std::string>(decoded), "");
}

TEST(DecodeFieldVariantTests, DecodePaddedString)
{
    std::vector<std::byte> bytes(32, std::byte{0});

    bytes[0] = std::byte{0x41};
    bytes[1] = std::byte{0x42};
    bytes[2] = std::byte{0x43};

    auto decoded = decodeFieldVariant(bytes, FieldType::STRING32);

    EXPECT_EQ(std::get<std::string>(decoded), "ABC");
}

TEST(DecodeFieldVariantTests, DecodeExactly32Bytes)
{
    std::string original(32, 'X');

    auto encoded = encodeFieldVariant(FieldVariant{original});
    auto decoded = decodeFieldVariant(encoded, FieldType::STRING32);

    EXPECT_EQ(std::get<std::string>(decoded), original);
}

TEST(DecodeFieldVariantTests, RejectNonZeroAfterPadding)
{
    std::vector<std::byte> bytes(32, std::byte{0});

    bytes[0] = std::byte{0x41};
    bytes[1] = std::byte{0x00};
    bytes[2] = std::byte{0x42};

    EXPECT_THROW(
        decodeFieldVariant(bytes, FieldType::STRING32),
        std::invalid_argument
    );
}

TEST(DecodeFieldVariantTests, RejectInvalidUTF8)
{
    std::vector<std::byte> bytes(32, std::byte{0});

    bytes[0] = std::byte{0xED};
    bytes[1] = std::byte{0xA0};
    bytes[2] = std::byte{0x80};

    EXPECT_THROW(
        decodeFieldVariant(bytes, FieldType::STRING32),
        std::invalid_argument
    );
}

TEST(DecodeFieldVariantTests, RejectIncorrectLength)
{
    EXPECT_THROW(
        decodeFieldVariant(
            std::vector<std::byte>(31),
            FieldType::STRING32
        ),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeFieldVariant(
            std::vector<std::byte>(7),
            FieldType::INT64
        ),
        std::invalid_argument
    );
}

TEST(DecodeFieldVariantTests, RejectInvalidFieldType)
{
    EXPECT_THROW(
        decodeFieldVariant(
            std::vector<std::byte>(32),
            static_cast<FieldType>(99)
        ),
        std::invalid_argument
    );
}

// ==========================================
// FieldVariant Round-Trip Tests
// ==========================================

TEST(FieldVariantRoundTripTests, Int64)
{
    for(std::int64_t value : {
        std::int64_t{0},
        std::int64_t{1},
        std::int64_t{-1},
        std::numeric_limits<std::int64_t>::min(),
        std::numeric_limits<std::int64_t>::max()
    })
    {
        auto encoded = encodeFieldVariant(FieldVariant{value});
        auto decoded = decodeFieldVariant(encoded, FieldType::INT64);

        EXPECT_EQ(std::get<std::int64_t>(decoded), value);
    }
}

TEST(FieldVariantRoundTripTests, UTF8Strings)
{
    std::vector<std::string> values{
        "",
        "A",
        "Hello",
        std::string(32, 'Z'),
        std::string{
            static_cast<char>(0xC2),
            static_cast<char>(0xA2)
        },
        std::string{
            static_cast<char>(0xE2),
            static_cast<char>(0x82),
            static_cast<char>(0xAC)
        },
        std::string{
            static_cast<char>(0xF0),
            static_cast<char>(0x9F),
            static_cast<char>(0x98),
            static_cast<char>(0x80)
        }
    };

    for(const auto& value : values)
    {
        auto encoded = encodeFieldVariant(FieldVariant{value});
        auto decoded = decodeFieldVariant(encoded, FieldType::STRING32);

        EXPECT_EQ(std::get<std::string>(decoded), value);
    }
}

// ==========================================
// encodeField / decodeField Tests
// ==========================================

TEST(FieldEncodingTests, EncodeInt64Field)
{
    Field field{"id", FieldType::INT64};

    auto encoded = encodeField(field);

    ASSERT_EQ(encoded.size(), fieldEncodeLength);

    EXPECT_EQ(encoded[0], std::byte{0x69});
    EXPECT_EQ(encoded[1], std::byte{0x64});

    for(std::size_t i = 2; i < 32; ++i)
    {
        EXPECT_EQ(encoded[i], std::byte{0x00});
    }

    EXPECT_EQ(encoded[32], std::byte{0x00});
}

TEST(FieldEncodingTests, EncodeString32Field)
{
    Field field{"name", FieldType::STRING32};

    auto encoded = encodeField(field);

    ASSERT_EQ(encoded.size(), fieldEncodeLength);

    EXPECT_EQ(encoded[0], std::byte{0x6E});
    EXPECT_EQ(encoded[1], std::byte{0x61});
    EXPECT_EQ(encoded[2], std::byte{0x6D});
    EXPECT_EQ(encoded[3], std::byte{0x65});

    EXPECT_EQ(encoded[32], std::byte{0x01});
}

TEST(FieldEncodingTests, RoundTripInt64Field)
{
    Field original{"id", FieldType::INT64};

    auto encoded = encodeField(original);
    Field decoded = decodeField(encoded);

    EXPECT_EQ(decoded.name, original.name);
    EXPECT_EQ(decoded.type, original.type);
}

TEST(FieldEncodingTests, RoundTripString32Field)
{
    Field original{"username", FieldType::STRING32};

    auto encoded = encodeField(original);
    Field decoded = decodeField(encoded);

    EXPECT_EQ(decoded.name, original.name);
    EXPECT_EQ(decoded.type, original.type);
}

TEST(FieldEncodingTests, RoundTrip32ByteName)
{
    Field original{std::string(32, 'A'), FieldType::STRING32};

    auto encoded = encodeField(original);
    Field decoded = decodeField(encoded);

    EXPECT_EQ(decoded.name, original.name);
    EXPECT_EQ(decoded.type, original.type);
}

TEST(FieldEncodingTests, RejectInvalidFieldName)
{
    Field field{std::string(33, 'A'), FieldType::INT64};

    EXPECT_THROW(encodeField(field), std::invalid_argument);
}

TEST(FieldEncodingTests, RejectInvalidFieldType)
{
    Field field{"id", static_cast<FieldType>(100)};

    EXPECT_THROW(encodeField(field), std::invalid_argument);
}

TEST(FieldEncodingTests, RejectIncorrectEncodedLength)
{
    EXPECT_THROW(
        decodeField(std::vector<std::byte>(32)),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeField(std::vector<std::byte>(34)),
        std::invalid_argument
    );
}

TEST(FieldEncodingTests, RejectInvalidEncodedType)
{
    auto encoded = encodeField(Field{"id", FieldType::INT64});

    encoded[32] = std::byte{0xFF};

    EXPECT_THROW(decodeField(encoded), std::invalid_argument);
}

// ==========================================
// encodeFieldList / decodeFieldList Tests
// ==========================================

TEST(FieldListEncodingTests, EncodeSingleField)
{
    std::vector<Field> fields{
        {"id", FieldType::INT64}
    };

    auto encoded = encodeFieldList(fields);

    ASSERT_EQ(encoded.size(), 2 + fieldEncodeLength);

    EXPECT_EQ(encoded[0], std::byte{0x01});
    EXPECT_EQ(encoded[1], std::byte{0x00});

    EXPECT_EQ(encoded[2], std::byte{0x69});
    EXPECT_EQ(encoded[3], std::byte{0x64});
    EXPECT_EQ(encoded[34], std::byte{0x00});
}

TEST(FieldListEncodingTests, EncodeMultipleFields)
{
    std::vector<Field> fields{
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64}
    };

    auto encoded = encodeFieldList(fields);

    ASSERT_EQ(encoded.size(), 2 + 3 * fieldEncodeLength);

    EXPECT_EQ(encoded[0], std::byte{0x03});
    EXPECT_EQ(encoded[1], std::byte{0x00});

    EXPECT_EQ(encoded[2], std::byte{0x69});
    EXPECT_EQ(encoded[35], std::byte{0x6E});
    EXPECT_EQ(encoded[68], std::byte{0x61});
}

TEST(FieldListEncodingTests, EncodeEmptyFieldList)
{
    std::vector<Field> fields;

    auto encoded = encodeFieldList(fields);

    ASSERT_EQ(encoded.size(), 2);
    EXPECT_EQ(encoded[0], std::byte{0x00});
    EXPECT_EQ(encoded[1], std::byte{0x00});
}

TEST(FieldListEncodingTests, RoundTripSingleField)
{
    std::vector<Field> original{
        {"id", FieldType::INT64}
    };

    auto encoded = encodeFieldList(original);
    auto decoded = decodeFieldList(encoded);

    ASSERT_EQ(decoded.size(), original.size());

    EXPECT_EQ(decoded[0].name, original[0].name);
    EXPECT_EQ(decoded[0].type, original[0].type);
}

TEST(FieldListEncodingTests, RoundTripMultipleFields)
{
    std::vector<Field> original{
        {"id", FieldType::INT64},
        {"username", FieldType::STRING32},
        {"balance", FieldType::INT64},
        {"email", FieldType::STRING32}
    };

    auto encoded = encodeFieldList(original);
    auto decoded = decodeFieldList(encoded);

    ASSERT_EQ(decoded.size(), original.size());

    for(std::size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(decoded[i].name, original[i].name);
        EXPECT_EQ(decoded[i].type, original[i].type);
    }
}

TEST(FieldListEncodingTests, RoundTripEmptyFieldList)
{
    std::vector<Field> original;

    auto encoded = encodeFieldList(original);
    auto decoded = decodeFieldList(encoded);

    EXPECT_TRUE(decoded.empty());
}

TEST(FieldListEncodingTests, RejectMissingFieldCount)
{
    EXPECT_THROW(
        decodeFieldList({}),
        std::invalid_argument
    );

    EXPECT_THROW(
        decodeFieldList({std::byte{0x01}}),
        std::invalid_argument
    );
}

TEST(FieldListEncodingTests, RejectNegativeFieldCount)
{
    std::vector<std::byte> encoded{
        std::byte{0xFF},
        std::byte{0xFF}
    };

    EXPECT_THROW(
        decodeFieldList(encoded),
        std::invalid_argument
    );
}

TEST(FieldListEncodingTests, RejectTruncatedFieldList)
{
    std::vector<Field> fields{
        {"id", FieldType::INT64}
    };

    auto encoded = encodeFieldList(fields);

    // Declare two fields while providing only one.
    encoded[0] = std::byte{0x02};
    encoded[1] = std::byte{0x00};

    EXPECT_THROW(
        decodeFieldList(encoded),
        std::out_of_range
    );
}

TEST(FieldListEncodingTests, RejectInvalidFieldWithinList)
{
    std::vector<Field> fields{
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    };

    auto encoded = encodeFieldList(fields);

    // Second field type byte: 2 + 33 + 32 = 67
    encoded[67] = std::byte{0xFF};

    EXPECT_THROW(
        decodeFieldList(encoded),
        std::invalid_argument
    );
}

TEST(FieldListEncodingTests, RejectInvalidFieldNameWithinList)
{
    std::vector<Field> fields{
        {"id", FieldType::INT64}
    };

    auto encoded = encodeFieldList(fields);

    // Corrupt padding after the first zero byte.
    encoded[5] = std::byte{0x41};

    EXPECT_THROW(
        decodeFieldList(encoded),
        std::invalid_argument
    );
}

// ==========================================
// Field List Count Boundary Tests
// ==========================================

TEST(FieldListEncodingTests, EncodeCountGreaterThan255)
{
    std::vector<Field> fields;

    for(int i = 0; i < 256; ++i)
    {
        fields.push_back({
            "field_" + std::to_string(i),
            FieldType::INT64
        });
    }

    auto encoded = encodeFieldList(fields);

    ASSERT_EQ(encoded.size(), 2 + 256 * fieldEncodeLength);

    // 256 = 0x0100, little-endian.
    EXPECT_EQ(encoded[0], std::byte{0x00});
    EXPECT_EQ(encoded[1], std::byte{0x01});

    auto decoded = decodeFieldList(encoded);

    ASSERT_EQ(decoded.size(), 256);

    for(std::size_t i = 0; i < fields.size(); ++i)
    {
        EXPECT_EQ(decoded[i].name, fields[i].name);
        EXPECT_EQ(decoded[i].type, fields[i].type);
    }
}