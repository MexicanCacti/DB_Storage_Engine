
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

TEST(TableHeaderTest, ConstructorValidTableName)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    EXPECT_NO_THROW(TableHeader("users", 1, schema));
}

TEST(TableHeaderTest, ConstructorEmptyTableName)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    EXPECT_NO_THROW(TableHeader("", 1, schema));
}

TEST(TableHeaderTest, ConstructorExactly32ByteTableName)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::string tableName(32, 'a');

    EXPECT_NO_THROW(TableHeader(tableName, 1, schema));
}

TEST(TableHeaderTest, ConstructorRejectsLongTableName)
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

TEST(TableHeaderTest, ConstructorRejectsEmbeddedNull)
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

TEST(TableHeaderTest, ConstructorRejectsInvalidUTF8)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::string tableName(
        1, static_cast<char>(0xFF)
    );

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

    const auto& expected = schema.getFieldList();
    const auto& actual = header.getSchema().getFieldList();

    ASSERT_EQ(expected.size(), actual.size());

    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(
            expected[i].name,
            actual[i].name
        );

        EXPECT_EQ(
            expected[i].type,
            actual[i].type
        );
    }
}

TEST(TableHeaderTest, GetNextPageNumberInitiallyOne)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    EXPECT_EQ(header.getNextPageNumber(), 1);
}

// ---------------------------------------------------------
// Page Number Allocation Tests
// ---------------------------------------------------------

TEST(TableHeaderTest, AllocateFirstPageNumber)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    std::int64_t pageNumber =
        header.allocatedNextPageNumber();

    EXPECT_EQ(pageNumber, 1);
    EXPECT_EQ(header.getNextPageNumber(), 2);
}

TEST(TableHeaderTest, AllocateSequentialPageNumbers)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    EXPECT_EQ(header.allocatedNextPageNumber(), 1);
    EXPECT_EQ(header.allocatedNextPageNumber(), 2);
    EXPECT_EQ(header.allocatedNextPageNumber(), 3);
    EXPECT_EQ(header.allocatedNextPageNumber(), 4);

    EXPECT_EQ(header.getNextPageNumber(), 5);
}

TEST(TableHeaderTest, AllocateManyPageNumbers)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    for (std::int64_t i = 1; i <= 1000; ++i)
    {
        EXPECT_EQ(
            header.allocatedNextPageNumber(),
            i
        );
    }

    EXPECT_EQ(header.getNextPageNumber(), 1001);
}

TEST(TableHeaderTest, GetNextPageNumberDoesNotAllocate)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    EXPECT_EQ(header.getNextPageNumber(), 1);
    EXPECT_EQ(header.getNextPageNumber(), 1);
    EXPECT_EQ(header.getNextPageNumber(), 1);
}

TEST(TableHeaderTest, AllocationDoesNotModifyTableID)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    header.allocatedNextPageNumber();
    header.allocatedNextPageNumber();

    EXPECT_EQ(header.getTableID(), 42);
}

TEST(TableHeaderTest, AllocationDoesNotModifySchema)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    TableHeader header("users", 42, schema);

    header.allocatedNextPageNumber();

    const auto& expected = schema.getFieldList();
    const auto& actual = header.getSchema().getFieldList();

    ASSERT_EQ(expected.size(), actual.size());

    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(expected[i].name,
                  actual[i].name);

        EXPECT_EQ(expected[i].type,
                  actual[i].type);
    }
}

// ---------------------------------------------------------
// Encoding Tests
// ---------------------------------------------------------

TEST(TableHeaderTest, EncodeProducesFullPage)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(bytes.size(), PAGE_BYTE_LENGTH);
}

TEST(TableHeaderTest, EncodeTableNameAtCorrectOffset)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

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

TEST(TableHeaderTest, EncodeNextPageNumberInitiallyOne)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    std::vector<std::byte> bytes = header.encode();

    // nextPageNumber occupies bytes [40, 47].
    EXPECT_EQ(bytes[40], std::byte{0x01});

    for (std::size_t i = 41; i < 48; ++i)
    {
        EXPECT_EQ(bytes[i], std::byte{0});
    }
}

TEST(TableHeaderTest, EncodeNextPageNumberLittleEndian)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    // Allocate 258 pages.
    for (std::int64_t i = 0; i < 258; ++i)
    {
        header.allocatedNextPageNumber();
    }

    // nextPageNumber = 259 = 0x0103.
    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(bytes[40], std::byte{0x03});
    EXPECT_EQ(bytes[41], std::byte{0x01});

    for (std::size_t i = 42; i < 48; ++i)
    {
        EXPECT_EQ(bytes[i], std::byte{0});
    }
}

TEST(TableHeaderTest, EncodeIsDeterministic)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    TableHeader header("users", 42, schema);

    header.allocatedNextPageNumber();

    std::vector<std::byte> first = header.encode();
    std::vector<std::byte> second = header.encode();

    EXPECT_EQ(first, second);
}

TEST(TableHeaderTest, AllocationChangesEncodedPageNumber)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    std::vector<std::byte> before = header.encode();

    header.allocatedNextPageNumber();

    std::vector<std::byte> after = header.encode();

    EXPECT_NE(before, after);

    EXPECT_EQ(before[40], std::byte{0x01});
    EXPECT_EQ(after[40], std::byte{0x02});
}

// ---------------------------------------------------------
// Decode Validation Tests
// ---------------------------------------------------------

TEST(TableHeaderTest, DecodeRejectsEmptyBuffer)
{
    std::vector<std::byte> bytes;

    EXPECT_THROW(
        TableHeader::decode(bytes),
        std::invalid_argument
    );
}

TEST(TableHeaderTest, DecodeRejectsSmallBuffer)
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

TEST(TableHeaderTest, DecodeRejectsLargeBuffer)
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


TEST(TableHeaderTest, DecodeRejectsZeroNextPageNumber)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    std::vector<std::byte> bytes = header.encode();

    // Set nextPageNumber to zero.
    for (std::size_t i = 40; i < 48; ++i)
    {
        bytes[i] = std::byte{0};
    }

    EXPECT_THROW(
        TableHeader::decode(bytes),
        std::invalid_argument
    );
}

TEST(TableHeaderTest, DecodeRejectsNegativeNextPageNumber)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader header("users", 42, schema);

    std::vector<std::byte> bytes = header.encode();

    // -1 encoded as signed INT64.
    for (std::size_t i = 40; i < 48; ++i)
    {
        bytes[i] = std::byte{0xFF};
    }

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
    EXPECT_EQ(decoded.getNextPageNumber(), 1);

    const auto& expected = schema.getFieldList();
    const auto& actual = decoded.getSchema().getFieldList();

    ASSERT_EQ(expected.size(), actual.size());

    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(expected[i].name,
                  actual[i].name);

        EXPECT_EQ(expected[i].type,
                  actual[i].type);
    }
}

TEST(TableHeaderTest, RoundTripPreservesAllocatedPageNumber)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader original("users", 42, schema);

    original.allocatedNextPageNumber();
    original.allocatedNextPageNumber();
    original.allocatedNextPageNumber();

    EXPECT_EQ(original.getNextPageNumber(), 4);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.getNextPageNumber(), 4);
}

TEST(TableHeaderTest, AllocationContinuesAfterDecode)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader original("users", 42, schema);

    original.allocatedNextPageNumber();
    original.allocatedNextPageNumber();
    original.allocatedNextPageNumber();

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.allocatedNextPageNumber(), 4);
    EXPECT_EQ(decoded.allocatedNextPageNumber(), 5);
    EXPECT_EQ(decoded.allocatedNextPageNumber(), 6);

    EXPECT_EQ(decoded.getNextPageNumber(), 7);
}

TEST(TableHeaderTest, RoundTripPreservesNegativeTableID)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader original("users", -42, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.getTableID(), -42);
}

TEST(TableHeaderTest, RoundTripPreservesMaxTableID)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::int64_t maxID =
        std::numeric_limits<std::int64_t>::max();

    TableHeader original("users", maxID, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.getTableID(), maxID);
}

TEST(TableHeaderTest, RoundTripPreserves32ByteName)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    std::string tableName(32, 'x');

    TableHeader original(tableName, 42, schema);

    std::vector<std::byte> bytes = original.encode();

    TableHeader decoded = TableHeader::decode(bytes);

    EXPECT_EQ(decoded.encode(), bytes);
}

TEST(TableHeaderTest, ReencodingDecodedHeaderProducesSameBytes)
{
    Schema schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32}
    });

    TableHeader original("users", 42, schema);

    for (int i = 0; i < 100; ++i)
    {
        original.allocatedNextPageNumber();
    }

    std::vector<std::byte> originalBytes = original.encode();

    TableHeader decoded = TableHeader::decode(originalBytes);

    std::vector<std::byte> decodedBytes = decoded.encode();

    EXPECT_EQ(originalBytes, decodedBytes);
}

TEST(TableHeaderTest, IndependentHeadersHaveIndependentCounters)
{
    Schema schema({
        {"id", FieldType::INT64}
    });

    TableHeader first("users", 1, schema);
    TableHeader second("orders", 2, schema);

    first.allocatedNextPageNumber();
    first.allocatedNextPageNumber();

    second.allocatedNextPageNumber();

    EXPECT_EQ(first.getNextPageNumber(), 3);
    EXPECT_EQ(second.getNextPageNumber(), 2);
}
