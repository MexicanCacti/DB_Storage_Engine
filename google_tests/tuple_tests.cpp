
#include <gtest/gtest.h>

#include "tuple.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

// ==========================================
// Tuple Construction Tests
// ==========================================

TEST(TupleConstructionTests, ConstructEmptyTuple)
{
    std::vector<FieldVariant> values;

    Tuple tuple(values);

    EXPECT_TRUE(tuple.getTupleValues().empty());
}

TEST(TupleConstructionTests, ConstructSingleInt64Tuple)
{
    std::vector<FieldVariant> values{
        std::int64_t{42}
    };

    Tuple tuple(values);

    const auto& storedValues = tuple.getTupleValues();

    ASSERT_EQ(storedValues.size(), 1);
    EXPECT_EQ(std::get<std::int64_t>(storedValues[0]), 42);
}

TEST(TupleConstructionTests, ConstructSingleString32Tuple)
{
    std::vector<FieldVariant> values{
        std::string{"Alice"}
    };

    Tuple tuple(values);

    const auto& storedValues = tuple.getTupleValues();

    ASSERT_EQ(storedValues.size(), 1);
    EXPECT_EQ(std::get<std::string>(storedValues[0]), "Alice");
}

TEST(TupleConstructionTests, ConstructMixedTuple)
{
    std::vector<FieldVariant> values{
        std::int64_t{100},
        std::string{"Alice"},
        std::int64_t{25}
    };

    Tuple tuple(values);

    const auto& storedValues = tuple.getTupleValues();

    ASSERT_EQ(storedValues.size(), 3);

    EXPECT_EQ(std::get<std::int64_t>(storedValues[0]), 100);
    EXPECT_EQ(std::get<std::string>(storedValues[1]), "Alice");
    EXPECT_EQ(std::get<std::int64_t>(storedValues[2]), 25);
}

TEST(TupleConstructionTests, PreserveValueOrder)
{
    std::vector<FieldVariant> values{
        std::string{"First"},
        std::int64_t{10},
        std::string{"Third"},
        std::int64_t{20}
    };

    Tuple tuple(values);

    EXPECT_EQ(tuple.getTupleValues(), values);
}

TEST(TupleConstructionTests, ConstructorCopiesValues)
{
    std::vector<FieldVariant> values{
        std::int64_t{42},
        std::string{"Original"}
    };

    Tuple tuple(values);

    values[0] = std::int64_t{100};
    values[1] = std::string{"Modified"};

    EXPECT_EQ(
        std::get<std::int64_t>(tuple.getTupleValues()[0]),
        42
    );

    EXPECT_EQ(
        std::get<std::string>(tuple.getTupleValues()[1]),
        "Original"
    );
}

// ==========================================
// getTupleValues Tests
// ==========================================

TEST(TupleAccessTests, GetTupleValues)
{
    std::vector<FieldVariant> values{
        std::int64_t{10},
        std::string{"Hello"},
        std::int64_t{-20}
    };

    Tuple tuple(values);

    const std::vector<FieldVariant>& storedValues =
        tuple.getTupleValues();

    EXPECT_EQ(storedValues, values);
}

TEST(TupleAccessTests, GetTupleValuesReturnsReference)
{
    std::vector<FieldVariant> values{
        std::int64_t{42}
    };

    Tuple tuple(values);

    const auto& first = tuple.getTupleValues();
    const auto& second = tuple.getTupleValues();

    EXPECT_EQ(&first, &second);
}

// ==========================================
// getTupleValueIndex Tests
// ==========================================

TEST(TupleAccessTests, GetFirstValue)
{
    Tuple tuple({
        std::int64_t{42},
        std::string{"Alice"}
    });

    const FieldVariant& value = tuple.getTupleValueIndex(0);

    ASSERT_TRUE(std::holds_alternative<std::int64_t>(value));
    EXPECT_EQ(std::get<std::int64_t>(value), 42);
}

TEST(TupleAccessTests, GetMiddleValue)
{
    Tuple tuple({
        std::int64_t{42},
        std::string{"Alice"},
        std::int64_t{25}
    });

    const FieldVariant& value = tuple.getTupleValueIndex(1);

    ASSERT_TRUE(std::holds_alternative<std::string>(value));
    EXPECT_EQ(std::get<std::string>(value), "Alice");
}

TEST(TupleAccessTests, GetLastValue)
{
    Tuple tuple({
        std::int64_t{42},
        std::string{"Alice"},
        std::int64_t{25}
    });

    const FieldVariant& value = tuple.getTupleValueIndex(2);

    ASSERT_TRUE(std::holds_alternative<std::int64_t>(value));
    EXPECT_EQ(std::get<std::int64_t>(value), 25);
}

TEST(TupleAccessTests, RejectIndexEqualToSize)
{
    Tuple tuple({
        std::int64_t{42},
        std::string{"Alice"}
    });

    EXPECT_THROW(
        tuple.getTupleValueIndex(2),
        std::invalid_argument
    );
}

TEST(TupleAccessTests, RejectIndexGreaterThanSize)
{
    Tuple tuple({
        std::int64_t{42},
        std::string{"Alice"}
    });

    EXPECT_THROW(
        tuple.getTupleValueIndex(100),
        std::invalid_argument
    );
}

TEST(TupleAccessTests, RejectIndexIntoEmptyTuple)
{
    Tuple tuple(std::vector<FieldVariant>{});

    EXPECT_THROW(
        tuple.getTupleValueIndex(0),
        std::invalid_argument
    );
}

TEST(TupleAccessTests, RejectMaximumSizeTIndex)
{
    Tuple tuple({
        std::int64_t{42}
    });

    std::size_t index =
        std::numeric_limits<std::size_t>::max();

    EXPECT_THROW(
        tuple.getTupleValueIndex(index),
        std::invalid_argument
    );
}

TEST(TupleAccessTests, GetValueReturnsReference)
{
    Tuple tuple({
        std::int64_t{42}
    });

    const FieldVariant& indexedValue =
        tuple.getTupleValueIndex(0);

    const FieldVariant& vectorValue =
        tuple.getTupleValues()[0];

    EXPECT_EQ(&indexedValue, &vectorValue);
}

// ==========================================
// Tuple Encoding Tests
// ==========================================

TEST(TupleEncodingTests, EncodeSingleInt64)
{
    Tuple tuple({
        std::int64_t{42}
    });

    std::vector<std::byte> expected{
        std::byte{0x2A},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00}
    };

    EXPECT_EQ(tuple.encode(), expected);
}

TEST(TupleEncodingTests, EncodeNegativeInt64)
{
    Tuple tuple({
        std::int64_t{-1}
    });

    std::vector<std::byte> expected(
        8,
        std::byte{0xFF}
    );

    EXPECT_EQ(tuple.encode(), expected);
}

TEST(TupleEncodingTests, EncodeSingleString32)
{
    Tuple tuple({
        std::string{"ABC"}
    });

    auto encoded = tuple.encode();

    ASSERT_EQ(encoded.size(), 32);

    EXPECT_EQ(encoded[0], std::byte{0x41});
    EXPECT_EQ(encoded[1], std::byte{0x42});
    EXPECT_EQ(encoded[2], std::byte{0x43});

    for(std::size_t i = 3; i < 32; ++i)
    {
        EXPECT_EQ(encoded[i], std::byte{0x00});
    }
}

TEST(TupleEncodingTests, EncodeMixedTuple)
{
    Tuple tuple({
        std::int64_t{42},
        std::string{"ABC"},
        std::int64_t{-1}
    });

    auto encoded = tuple.encode();

    ASSERT_EQ(encoded.size(), 48);

    // First INT64: bytes 0-7
    EXPECT_EQ(encoded[0], std::byte{0x2A});

    for(std::size_t i = 1; i < 8; ++i)
    {
        EXPECT_EQ(encoded[i], std::byte{0x00});
    }

    // STRING32: bytes 8-39
    EXPECT_EQ(encoded[8], std::byte{0x41});
    EXPECT_EQ(encoded[9], std::byte{0x42});
    EXPECT_EQ(encoded[10], std::byte{0x43});

    for(std::size_t i = 11; i < 40; ++i)
    {
        EXPECT_EQ(encoded[i], std::byte{0x00});
    }

    // Final INT64: bytes 40-47
    for(std::size_t i = 40; i < 48; ++i)
    {
        EXPECT_EQ(encoded[i], std::byte{0xFF});
    }
}

TEST(TupleEncodingTests, EncodeEmptyTuple)
{
    Tuple tuple(std::vector<FieldVariant>{});

    auto encoded = tuple.encode();

    EXPECT_TRUE(encoded.empty());
}

TEST(TupleEncodingTests, EncodeEmptyString)
{
    Tuple tuple({
        std::string{""}
    });

    auto encoded = tuple.encode();

    EXPECT_EQ(
        encoded,
        std::vector<std::byte>(32, std::byte{0})
    );
}

TEST(TupleEncodingTests, EncodeExactly32ByteString)
{
    Tuple tuple({
        std::string(32, 'A')
    });

    auto encoded = tuple.encode();

    ASSERT_EQ(encoded.size(), 32);

    for(std::byte byte : encoded)
    {
        EXPECT_EQ(byte, std::byte{0x41});
    }
}

TEST(TupleEncodingTests, RejectStringLongerThan32Bytes)
{
    Tuple tuple({
        std::string(33, 'A')
    });

    EXPECT_THROW(
        tuple.encode(),
        std::invalid_argument
    );
}

TEST(TupleEncodingTests, RejectEmbeddedZeroByte)
{
    Tuple tuple({
        std::string("AB\0CD", 5)
    });

    EXPECT_THROW(
        tuple.encode(),
        std::invalid_argument
    );
}

TEST(TupleEncodingTests, RejectInvalidUTF8)
{
    std::string invalidUTF8{
        static_cast<char>(0xC0),
        static_cast<char>(0x80)
    };

    Tuple tuple({
        invalidUTF8
    });

    EXPECT_THROW(
        tuple.encode(),
        std::invalid_argument
    );
}

TEST(TupleEncodingTests, EncodingIsDeterministic)
{
    Tuple tuple({
        std::int64_t{42},
        std::string{"Alice"}
    });

    auto firstEncoding = tuple.encode();
    auto secondEncoding = tuple.encode();

    EXPECT_EQ(firstEncoding, secondEncoding);
}

TEST(TupleEncodingTests, EncodingDoesNotModifyValues)
{
    std::vector<FieldVariant> original{
        std::int64_t{42},
        std::string{"Alice"}
    };

    Tuple tuple(original);

    tuple.encode();

    EXPECT_EQ(tuple.getTupleValues(), original);
}

// ==========================================
// Tuple Decoding Tests
// ==========================================

TEST(TupleDecodingTests, DecodeSingleInt64)
{
    std::vector<Field> schemaFields{
        {"id", FieldType::INT64}
    };

    std::vector<std::byte> bytes{
        std::byte{0x2A},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00}
    };

    auto decoded = Tuple::decode(bytes, schemaFields);

    ASSERT_EQ(decoded.size(), 1);

    EXPECT_EQ(
        std::get<std::int64_t>(decoded[0]),
        42
    );
}

TEST(TupleDecodingTests, DecodeSingleString32)
{
    std::vector<Field> schemaFields{
        {"name", FieldType::STRING32}
    };

    std::vector<std::byte> bytes(32, std::byte{0});

    bytes[0] = std::byte{0x41};
    bytes[1] = std::byte{0x42};
    bytes[2] = std::byte{0x43};

    auto decoded = Tuple::decode(bytes, schemaFields);

    ASSERT_EQ(decoded.size(), 1);

    EXPECT_EQ(
        std::get<std::string>(decoded[0]),
        "ABC"
    );
}

TEST(TupleDecodingTests, DecodeMixedTuple)
{
    std::vector<Field> schemaFields{
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64}
    };

    Tuple original({
        std::int64_t{42},
        std::string{"Alice"},
        std::int64_t{25}
    });

    auto encoded = original.encode();
    auto decoded = Tuple::decode(encoded, schemaFields);

    ASSERT_EQ(decoded.size(), 3);

    EXPECT_EQ(std::get<std::int64_t>(decoded[0]), 42);
    EXPECT_EQ(std::get<std::string>(decoded[1]), "Alice");
    EXPECT_EQ(std::get<std::int64_t>(decoded[2]), 25);
}

TEST(TupleDecodingTests, DecodeEmptyTuple)
{
    std::vector<Field> schemaFields;
    std::vector<std::byte> bytes;

    auto decoded = Tuple::decode(bytes, schemaFields);

    EXPECT_TRUE(decoded.empty());
}

TEST(TupleDecodingTests, RejectTruncatedInt64)
{
    std::vector<Field> schemaFields{
        {"id", FieldType::INT64}
    };

    std::vector<std::byte> bytes(7, std::byte{0});

    EXPECT_THROW(
        Tuple::decode(bytes, schemaFields),
        std::out_of_range
    );
}

TEST(TupleDecodingTests, RejectTruncatedString32)
{
    std::vector<Field> schemaFields{
        {"name", FieldType::STRING32}
    };

    std::vector<std::byte> bytes(31, std::byte{0});

    EXPECT_THROW(
        Tuple::decode(bytes, schemaFields),
        std::out_of_range
    );
}

TEST(TupleDecodingTests, RejectInvalidFieldType)
{
    std::vector<Field> schemaFields{
        {"id", static_cast<FieldType>(100)}
    };

    std::vector<std::byte> bytes(8, std::byte{0});

    EXPECT_THROW(
        Tuple::decode(bytes, schemaFields),
        std::invalid_argument
    );
}

TEST(TupleDecodingTests, RejectInvalidStringPadding)
{
    std::vector<Field> schemaFields{
        {"name", FieldType::STRING32}
    };

    std::vector<std::byte> bytes(32, std::byte{0});

    bytes[0] = std::byte{0x41};
    bytes[1] = std::byte{0x00};
    bytes[2] = std::byte{0x42};

    EXPECT_THROW(
        Tuple::decode(bytes, schemaFields),
        std::invalid_argument
    );
}

TEST(TupleDecodingTests, RejectInvalidUTF8)
{
    std::vector<Field> schemaFields{
        {"name", FieldType::STRING32}
    };

    std::vector<std::byte> bytes(32, std::byte{0});

    bytes[0] = std::byte{0xED};
    bytes[1] = std::byte{0xA0};
    bytes[2] = std::byte{0x80};

    EXPECT_THROW(
        Tuple::decode(bytes, schemaFields),
        std::invalid_argument
    );
}

// ==========================================
// Tuple Round-Trip Tests
// ==========================================

TEST(TupleRoundTripTests, SingleInt64)
{
    std::vector<Field> schemaFields{
        {"id", FieldType::INT64}
    };

    std::vector<FieldVariant> originalValues{
        std::int64_t{-123456789}
    };

    Tuple original(originalValues);

    auto encoded = original.encode();
    auto decoded = Tuple::decode(encoded, schemaFields);

    EXPECT_EQ(decoded, originalValues);
}

TEST(TupleRoundTripTests, SingleString32)
{
    std::vector<Field> schemaFields{
        {"name", FieldType::STRING32}
    };

    std::vector<FieldVariant> originalValues{
        std::string{"Hello World"}
    };

    Tuple original(originalValues);

    auto encoded = original.encode();
    auto decoded = Tuple::decode(encoded, schemaFields);

    EXPECT_EQ(decoded, originalValues);
}

TEST(TupleRoundTripTests, MixedFieldTypes)
{
    std::vector<Field> schemaFields{
        {"id", FieldType::INT64},
        {"firstName", FieldType::STRING32},
        {"lastName", FieldType::STRING32},
        {"balance", FieldType::INT64}
    };

    std::vector<FieldVariant> originalValues{
        std::int64_t{1001},
        std::string{"Alice"},
        std::string{"Smith"},
        std::int64_t{-500}
    };

    Tuple original(originalValues);

    auto encoded = original.encode();
    auto decoded = Tuple::decode(encoded, schemaFields);

    EXPECT_EQ(decoded, originalValues);
}

TEST(TupleRoundTripTests, Int64Boundaries)
{
    std::vector<Field> schemaFields{
        {"minimum", FieldType::INT64},
        {"maximum", FieldType::INT64},
        {"zero", FieldType::INT64},
        {"negativeOne", FieldType::INT64}
    };

    std::vector<FieldVariant> originalValues{
        std::numeric_limits<std::int64_t>::min(),
        std::numeric_limits<std::int64_t>::max(),
        std::int64_t{0},
        std::int64_t{-1}
    };

    Tuple original(originalValues);

    auto encoded = original.encode();
    auto decoded = Tuple::decode(encoded, schemaFields);

    EXPECT_EQ(decoded, originalValues);
}

TEST(TupleRoundTripTests, EmptyAndFullStrings)
{
    std::vector<Field> schemaFields{
        {"empty", FieldType::STRING32},
        {"full", FieldType::STRING32}
    };

    std::vector<FieldVariant> originalValues{
        std::string{""},
        std::string(32, 'A')
    };

    Tuple original(originalValues);

    auto encoded = original.encode();
    auto decoded = Tuple::decode(encoded, schemaFields);

    EXPECT_EQ(decoded, originalValues);
}

TEST(TupleRoundTripTests, MultibyteUTF8)
{
    std::vector<Field> schemaFields{
        {"twoByte", FieldType::STRING32},
        {"threeByte", FieldType::STRING32},
        {"fourByte", FieldType::STRING32}
    };

    std::vector<FieldVariant> originalValues{
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

    Tuple original(originalValues);

    auto encoded = original.encode();
    auto decoded = Tuple::decode(encoded, schemaFields);

    EXPECT_EQ(decoded, originalValues);
}

TEST(TupleRoundTripTests, PreserveFieldOrder)
{
    std::vector<Field> schemaFields{
        {"first", FieldType::STRING32},
        {"second", FieldType::INT64},
        {"third", FieldType::STRING32},
        {"fourth", FieldType::INT64}
    };

    std::vector<FieldVariant> originalValues{
        std::string{"First"},
        std::int64_t{100},
        std::string{"Third"},
        std::int64_t{200}
    };

    Tuple original(originalValues);

    auto encoded = original.encode();
    auto decoded = Tuple::decode(encoded, schemaFields);

    EXPECT_EQ(decoded, originalValues);
}

TEST(TupleRoundTripTests, ReconstructTuple)
{
    std::vector<Field> schemaFields{
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    };

    Tuple original({
        std::int64_t{42},
        std::string{"Alice"}
    });

    auto encoded = original.encode();

    Tuple reconstructed(
        Tuple::decode(encoded, schemaFields)
    );

    EXPECT_EQ(
        reconstructed.getTupleValues(),
        original.getTupleValues()
    );

    EXPECT_EQ(
        reconstructed.encode(),
        original.encode()
    );
}
