#include <gtest/gtest.h>
#include "schema.hpp"

// ==========================================
// Construction Tests
// ==========================================

TEST(ConstructionTests, DuplicateNames)
{
    std::vector<Field> sameTypeSameNames = std::vector<Field>{
        {"Name", FieldType::STRING32},
        {"Name", FieldType::STRING32}
    };

    std::vector<Field> differentTypeSameNames = std::vector<Field>{
        {"Name", FieldType::STRING32},
        {"Name", FieldType::INT64}
    };
    
    EXPECT_THROW(
      Schema s(sameTypeSameNames), std::invalid_argument
    );
    
    EXPECT_THROW(
      Schema s(differentTypeSameNames), std::invalid_argument
    );
}

TEST(ConstructionTests, TooLargeTupleWidth)
{
    std::vector<Field> TooLargeTupleWidth;

    size_t currentWidth = 0;
    size_t typeSize = getFieldTypeSize(FieldType::STRING32);
    size_t nameNumber = 0;
    while(currentWidth <= MAX_TUPLE_WIDTH)
    {
        std::string name = "Name " + std::to_string(nameNumber++);
        TooLargeTupleWidth.push_back({name, FieldType::STRING32});
        currentWidth += typeSize;
    }

    EXPECT_THROW(
        Schema s(TooLargeTupleWidth), std::invalid_argument
    );
}

TEST(ConstructionTests, CorrectConstruction)
{
    std::vector<Field> SchemaField { 
      {"Int64Field", FieldType::INT64}, 
      {"String32Field", FieldType::STRING32} 
    };
  
    EXPECT_NO_THROW(Schema s(SchemaField));
}

TEST(ConstructionTests, ExpectedConstruction)
{
    std::vector<Field> SchemaField { 
        {"Int64Field", FieldType::INT64}, 
        {"String32Field", FieldType::STRING32} 
    };
  
    Schema s(SchemaField);

    std::vector<Field> saveFieldList = s.getFieldList();

    ASSERT_EQ(saveFieldList.size(), SchemaField.size());

    for(std::size_t currentField = 0 ; currentField < saveFieldList.size(); ++currentField)
    {
        EXPECT_EQ(saveFieldList[currentField].name, SchemaField[currentField].name);
        EXPECT_EQ(saveFieldList[currentField].type, SchemaField[currentField].type);
    }
}

// ==========================================
// getFieldList / getField Tests
// ==========================================

TEST(FieldAccessTests, GetFieldList)
{
    std::vector<Field> schemaFields{
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64}
    };

    Schema schema(schemaFields);

    const auto& fields = schema.getFieldList();

    ASSERT_EQ(fields.size(), 3);

    EXPECT_EQ(fields[0].name, "id");
    EXPECT_EQ(fields[1].name, "name");
    EXPECT_EQ(fields[2].name, "age");

    EXPECT_EQ(fields[0].type, FieldType::INT64);
    EXPECT_EQ(fields[1].type, FieldType::STRING32);
    EXPECT_EQ(fields[2].type, FieldType::INT64);
}

TEST(FieldAccessTests, GetFieldByIndex)
{
    std::vector<Field> schemaFields{
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    };

    Schema schema(schemaFields);

    const Field& firstField = schema.getField(0);
    const Field& secondField = schema.getField(1);

    EXPECT_EQ(firstField.name, "id");
    EXPECT_EQ(firstField.type, FieldType::INT64);

    EXPECT_EQ(secondField.name, "name");
    EXPECT_EQ(secondField.type, FieldType::STRING32);
}

// ==========================================
// isValidTuple Tests
// ==========================================

TEST(TupleValidationTests, AcceptCorrectTuple)
{
    Schema schema({
        {"Int64Field", FieldType::INT64},
        {"String32Field", FieldType::STRING32}
    });

    std::vector<FieldVariant> tupleValues{
        std::int64_t{2},
        std::string{"blah"}
    };

    EXPECT_TRUE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, RejectIncorrectFieldOrder)
{
    Schema schema({
        {"Int64Field", FieldType::INT64},
        {"String32Field", FieldType::STRING32}
    });

    std::vector<FieldVariant> tupleValues{
        std::string{"blah"},
        std::int64_t{2}
    };

    EXPECT_FALSE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, RejectTooFewValues)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    std::vector<FieldVariant> tupleValues{
        std::int64_t{10}
    };

    EXPECT_FALSE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, RejectTooManyValues)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    std::vector<FieldVariant> tupleValues{
        std::int64_t{10},
        std::string{"Alice"},
        std::int64_t{20}
    };

    EXPECT_FALSE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, RejectEmptyTuple)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::vector<FieldVariant> tupleValues;

    EXPECT_FALSE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, AcceptNegativeInteger)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::vector<FieldVariant> tupleValues{
        std::int64_t{-123456}
    };

    EXPECT_TRUE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, AcceptInt64Boundaries)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    EXPECT_TRUE(schema.isValidTuple({
        std::numeric_limits<std::int64_t>::min()
    }));

    EXPECT_TRUE(schema.isValidTuple({
        std::numeric_limits<std::int64_t>::max()
    }));
}

TEST(TupleValidationTests, AcceptEmptyString)
{
    Schema schema({
        {"name", FieldType::STRING32}
    });

    std::vector<FieldVariant> tupleValues{
        std::string{""}
    };

    EXPECT_TRUE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, AcceptExactly32ByteString)
{
    Schema schema({
        {"name", FieldType::STRING32}
    });

    std::vector<FieldVariant> tupleValues{
        std::string(32, 'A')
    };

    EXPECT_TRUE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, Reject33ByteString)
{
    Schema schema({
        {"name", FieldType::STRING32}
    });

    std::vector<FieldVariant> tupleValues{
        std::string(33, 'A')
    };

    EXPECT_FALSE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, RejectEmbeddedZeroByte)
{
    Schema schema({
        {"name", FieldType::STRING32}
    });

    std::string invalidString = std::string("AB\0CD", 5);

    std::vector<FieldVariant> tupleValues{
        invalidString
    };

    EXPECT_FALSE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, RejectInvalidUTF8)
{
    Schema schema({
        {"name", FieldType::STRING32}
    });

    std::string invalidUTF8{
        static_cast<char>(0xC0),
        static_cast<char>(0x80)
    };

    std::vector<FieldVariant> tupleValues{
        invalidUTF8
    };

    EXPECT_FALSE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, AcceptValidMultibyteUTF8)
{
    Schema schema({
        {"name", FieldType::STRING32}
    });

    // UTF-8 encoding of the Euro symbol
    std::string validUTF8{
        static_cast<char>(0xE2),
        static_cast<char>(0x82),
        static_cast<char>(0xAC)
    };

    std::vector<FieldVariant> tupleValues{
        validUTF8
    };

    EXPECT_TRUE(schema.isValidTuple(tupleValues));
}

TEST(TupleValidationTests, RejectWrongVariantType)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::vector<FieldVariant> tupleValues{
        std::string{"123"}
    };

    EXPECT_FALSE(schema.isValidTuple(tupleValues));
}

// ==========================================
// Schema Encoding Tests
// ==========================================

TEST(SchemaEncodeTests, EncodeSingleField)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::vector<std::byte> encoded = schema.encode();

    ASSERT_EQ(encoded.size(), 2 + 33);

    // Field count = 1 (little-endian)
    EXPECT_EQ(encoded[0], std::byte{0x01});
    EXPECT_EQ(encoded[1], std::byte{0x00});

    // Field name = "id"
    EXPECT_EQ(encoded[2], std::byte{0x69});
    EXPECT_EQ(encoded[3], std::byte{0x64});

    // Field type = INT64
    EXPECT_EQ(encoded[34], std::byte{0x00});
}

TEST(SchemaEncodeTests, EncodeMultipleFields)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64}
    });

    std::vector<std::byte> encoded = schema.encode();

    ASSERT_EQ(encoded.size(), 2 + 3 * 33);

    // Field count = 3
    EXPECT_EQ(encoded[0], std::byte{0x03});
    EXPECT_EQ(encoded[1], std::byte{0x00});

    // Field 0 starts at offset 2
    EXPECT_EQ(encoded[2], std::byte{0x69});

    // Field 1 starts at offset 35
    EXPECT_EQ(encoded[35], std::byte{0x6E});

    // Field 2 starts at offset 68
    EXPECT_EQ(encoded[68], std::byte{0x61});

    // Field types
    EXPECT_EQ(encoded[34], std::byte{0x00});
    EXPECT_EQ(encoded[67], std::byte{0x01});
    EXPECT_EQ(encoded[100], std::byte{0x00});
}

TEST(SchemaEncodeTests, EncodedSizeMatchesFieldCount)
{
    std::vector<Field> fields{
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64},
        {"email", FieldType::STRING32}
    };

    Schema schema(fields);

    std::vector<std::byte> encoded = schema.encode();

    EXPECT_EQ(encoded.size(), 2 + fields.size() * 33);
}

TEST(SchemaEncodeTests, EncodingIsDeterministic)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    auto firstEncoding = schema.encode();
    auto secondEncoding = schema.encode();

    EXPECT_EQ(firstEncoding, secondEncoding);
}

// ==========================================
// Schema Decoding Tests
// ==========================================

TEST(SchemaDecodeTests, DecodeSingleField)
{
    Schema original({
        {"id", FieldType::INT64}
    });

    std::vector<std::byte> encoded = original.encode();

    std::vector<Field> decoded = Schema::decode(encoded);

    ASSERT_EQ(decoded.size(), 1);

    EXPECT_EQ(decoded[0].name, "id");
    EXPECT_EQ(decoded[0].type, FieldType::INT64);
}

TEST(SchemaDecodeTests, DecodeMultipleFields)
{
    Schema original({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64}
    });

    std::vector<std::byte> encoded = original.encode();

    std::vector<Field> decoded = Schema::decode(encoded);

    ASSERT_EQ(decoded.size(), 3);

    EXPECT_EQ(decoded[0].name, "id");
    EXPECT_EQ(decoded[0].type, FieldType::INT64);

    EXPECT_EQ(decoded[1].name, "name");
    EXPECT_EQ(decoded[1].type, FieldType::STRING32);

    EXPECT_EQ(decoded[2].name, "age");
    EXPECT_EQ(decoded[2].type, FieldType::INT64);
}

TEST(SchemaDecodeTests, RejectMissingFieldCount)
{
    EXPECT_THROW(
        Schema::decode({}),
        std::invalid_argument
    );

    EXPECT_THROW(
        Schema::decode({std::byte{0x01}}),
        std::invalid_argument
    );
}

TEST(SchemaDecodeTests, RejectNegativeFieldCount)
{
    std::vector<std::byte> encoded{
        std::byte{0xFF},
        std::byte{0xFF}
    };

    EXPECT_THROW(
        Schema::decode(encoded),
        std::invalid_argument
    );
}

TEST(SchemaDecodeTests, RejectInvalidFieldType)
{
    Schema original({
        {"id", FieldType::INT64}
    });

    auto encoded = original.encode();

    // Corrupt field type at offset 34
    encoded[34] = std::byte{0xFF};

    EXPECT_THROW(
        Schema::decode(encoded),
        std::invalid_argument
    );
}

TEST(SchemaDecodeTests, RejectTruncatedFieldList)
{
    Schema original({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    auto encoded = original.encode();

    // Remove final byte from encoded field list
    encoded.pop_back();

    EXPECT_THROW(
        Schema::decode(encoded),
        std::out_of_range
    );
}

// ==========================================
// Schema Round-Trip Tests
// ==========================================

TEST(SchemaRoundTripTests, EncodeDecodeSingleField)
{
    std::vector<Field> originalFields{
        {"id", FieldType::INT64}
    };

    Schema original(originalFields);

    auto encoded = original.encode();
    auto decodedFields = Schema::decode(encoded);

    ASSERT_EQ(decodedFields.size(), originalFields.size());

    Schema reconstructed(decodedFields);

    EXPECT_EQ(
        reconstructed.getTupleWidth(),
        original.getTupleWidth()
    );

    EXPECT_EQ(
        reconstructed.getField(0).name,
        original.getField(0).name
    );

    EXPECT_EQ(
        reconstructed.getField(0).type,
        original.getField(0).type
    );
}

TEST(SchemaRoundTripTests, EncodeDecodeMultipleFields)
{
    std::vector<Field> originalFields{
        {"id", FieldType::INT64},
        {"firstName", FieldType::STRING32},
        {"lastName", FieldType::STRING32},
        {"age", FieldType::INT64}
    };

    Schema original(originalFields);

    auto encoded = original.encode();
    auto decodedFields = Schema::decode(encoded);

    Schema reconstructed(decodedFields);

    const auto& originalList = original.getFieldList();
    const auto& reconstructedList = reconstructed.getFieldList();

    ASSERT_EQ(originalList.size(), reconstructedList.size());

    for(std::size_t i = 0; i < originalList.size(); ++i)
    {
        EXPECT_EQ(originalList[i].name, reconstructedList[i].name);
        EXPECT_EQ(originalList[i].type, reconstructedList[i].type);
    }

    EXPECT_EQ(
        original.getTupleWidth(),
        reconstructed.getTupleWidth()
    );

    EXPECT_EQ(
        original.encode(),
        reconstructed.encode()
    );
}

TEST(SchemaRoundTripTests, EncodeDecodeUTF8FieldNames)
{
    std::string utf8Name{
        static_cast<char>(0xE2),
        static_cast<char>(0x82),
        static_cast<char>(0xAC)
    };

    std::vector<Field> originalFields{
        {utf8Name, FieldType::STRING32}
    };

    Schema original(originalFields);

    auto encoded = original.encode();
    auto decodedFields = Schema::decode(encoded);

    ASSERT_EQ(decodedFields.size(), 1);

    EXPECT_EQ(decodedFields[0].name, utf8Name);
    EXPECT_EQ(decodedFields[0].type, FieldType::STRING32);
}

TEST(SchemaRoundTripTests, PreserveFieldOrder)
{
    std::vector<Field> originalFields{
        {"zeta", FieldType::INT64},
        {"alpha", FieldType::STRING32},
        {"gamma", FieldType::INT64},
        {"beta", FieldType::STRING32}
    };

    Schema original(originalFields);

    auto decodedFields = Schema::decode(original.encode());

    ASSERT_EQ(decodedFields.size(), originalFields.size());

    for(std::size_t i = 0; i < originalFields.size(); ++i)
    {
        EXPECT_EQ(decodedFields[i].name, originalFields[i].name);
        EXPECT_EQ(decodedFields[i].type, originalFields[i].type);
    }
}
