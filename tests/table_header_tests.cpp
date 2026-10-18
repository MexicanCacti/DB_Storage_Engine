
#include <gtest/gtest.h>

#include "table_header.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------
// Constructor Tests
// ---------------------------------------------------------

TEST(TableHeaderTest, ValidConstructor)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    EXPECT_NO_THROW(TableHeader("users", 1, schema));
}

TEST(TableHeaderTest, EmptyTableName)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    EXPECT_NO_THROW(TableHeader("", 1, schema));
}

TEST(TableHeaderTest, Exactly32ByteTableName)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::string tableName(32, 'a');

    EXPECT_NO_THROW(TableHeader(tableName, 1, schema));
}

TEST(TableHeaderTest, TableNameExceeds32Bytes)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::string tableName(33, 'a');

    EXPECT_THROW(
        TableHeader(tableName, 1, schema),
        std::invalid_argument
    );
}

TEST(TableHeaderTest, TableNameContainsNullByte)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::string tableName("abc\0def", 7);

    EXPECT_THROW(
        TableHeader(tableName, 1, schema),
        std::invalid_argument
    );
}

TEST(TableHeaderTest, InvalidUTF8TableName)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::string tableName(1, static_cast<char>(0xFF));

    EXPECT_THROW(
        TableHeader(tableName, 1, schema),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Getter Tests
// ---------------------------------------------------------

TEST(TableHeaderTest, GetTableID)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    EXPECT_EQ(header.getTableID(), 42);
}

TEST(TableHeaderTest, GetSchema)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    TableHeader header("users", 1, schema);

    const auto& expectedFields = schema.getFieldList();
    const auto& actualFields = header.getSchema().getFieldList();

    ASSERT_EQ(expectedFields.size(), actualFields.size());

    for (std::size_t i = 0; i < expectedFields.size(); ++i)
    {
        EXPECT_EQ(expectedFields[i].name,
                  actualFields[i].name);

        EXPECT_EQ(expectedFields[i].type,
                  actualFields[i].type);
    }
}

// ---------------------------------------------------------
// Encoding Tests
// ---------------------------------------------------------

TEST(TableHeaderTest, EncodeProducesPageSize)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 1, schema);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(bytes.size(), PAGE_BYTE_LENGTH);
}

TEST(TableHeaderTest, EncodeTableName)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 1, schema);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(bytes[0], std::byte{'u'});
    EXPECT_EQ(bytes[1], std::byte{'s'});
    EXPECT_EQ(bytes[2], std::byte{'e'});
    EXPECT_EQ(bytes[3], std::byte{'r'});
    EXPECT_EQ(bytes[4], std::byte{'s'});

    for (std::size_t i = 5; i < 32; ++i)
    {
        EXPECT_EQ(bytes[i], std::byte{0});
    }
}

TEST(TableHeaderTest, EncodeTableIDLittleEndian)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header(
        "users",
        0x0102030405060708LL,
        schema
    );

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(bytes[32], std::byte{0x08});
    EXPECT_EQ(bytes[33], std::byte{0x07});
    EXPECT_EQ(bytes[34], std::byte{0x06});
    EXPECT_EQ(bytes[35], std::byte{0x05});
    EXPECT_EQ(bytes[36], std::byte{0x04});
    EXPECT_EQ(bytes[37], std::byte{0x03});
    EXPECT_EQ(bytes[38], std::byte{0x02});
    EXPECT_EQ(bytes[39], std::byte{0x01});
}

TEST(TableHeaderTest, EncodeDeterministic)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    TableHeader header("users", 42, schema);

    std::vector<std::byte> first = header.encode();
    std::vector<std::byte> second = header.encode();

    EXPECT_EQ(first, second);
}

// ---------------------------------------------------------
// Decoding Tests
// ---------------------------------------------------------

TEST(TableHeaderTest, DecodeEmptyBuffer)
{
    std::vector<std::byte> bytes;

    EXPECT_THROW(
        TableHeader::decode(bytes),
        std::invalid_argument
    );
}

TEST(TableHeaderTest, DecodeBufferTooSmall)
{
    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH - 1,
        std::byte{0}
    );

    EXPECT_THROW(
        TableHeader::decode(bytes),
        std::invalid_argument
    );
}

TEST(TableHeaderTest, DecodeBufferTooLarge)
{
    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH + 1,
        std::byte{0}
    );

    EXPECT_THROW(
        TableHeader::decode(bytes),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Round-Trip Tests
// ---------------------------------------------------------

TEST(TableHeaderTest, EncodeDecodeRoundTrip)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64}
    });

    TableHeader original("users", 42, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.getTableID(), 42);

    const auto& expectedFields = schema.getFieldList();
    const auto& actualFields = decoded.getSchema().getFieldList();

    ASSERT_EQ(expectedFields.size(), actualFields.size());

    for (std::size_t i = 0; i < expectedFields.size(); ++i)
    {
        EXPECT_EQ(expectedFields[i].name,
                  actualFields[i].name);

        EXPECT_EQ(expectedFields[i].type,
                  actualFields[i].type);
    }
}

TEST(TableHeaderTest, RoundTripNegativeTableID)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader original("users", -12345, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.getTableID(), -12345);
}

TEST(TableHeaderTest, RoundTripZeroTableID)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader original("users", 0, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.getTableID(), 0);
}

TEST(TableHeaderTest, RoundTripMaxInt64)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::int64_t maxID = std::numeric_limits<std::int64_t>::max();

    TableHeader original("users", maxID, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.getTableID(), maxID);
}

TEST(TableHeaderTest, RoundTripMinInt64)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::int64_t minID = std::numeric_limits<std::int64_t>::min();

    TableHeader original("users", minID, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.getTableID(), minID);
}

TEST(TableHeaderTest, RoundTripExactly32ByteName)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::string tableName(32, 'x');

    TableHeader original(tableName, 42, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    std::vector<std::byte> reencoded = decoded.encode();

    EXPECT_EQ(bytes, reencoded);
}

TEST(TableHeaderTest, RoundTripPreservesSchemaFieldOrder)
{
    Schema schema({
        {"first", FieldType::STRING32},
        {"second", FieldType::INT64},
        {"third", FieldType::STRING32}
    });

    TableHeader original("records", 7, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    const auto& expectedFields = schema.getFieldList();
    const auto& actualFields = decoded.getSchema().getFieldList();

    ASSERT_EQ(expectedFields.size(), actualFields.size());

    for (std::size_t i = 0; i < expectedFields.size(); ++i)
    {
        EXPECT_EQ(expectedFields[i].name,
                  actualFields[i].name);

        EXPECT_EQ(expectedFields[i].type,
                  actualFields[i].type);
    }
}

TEST(TableHeaderTest, ReencodingDecodedHeaderProducesSameBytes)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    TableHeader original("users", 42, schema);

    std::vector<std::byte> originalBytes = original.encode();

    TableHeader decoded = TableHeader::decode(originalBytes);

    std::vector<std::byte> decodedBytes = decoded.encode();

    EXPECT_EQ(originalBytes, decodedBytes);
}
