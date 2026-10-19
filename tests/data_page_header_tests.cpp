
#include <gtest/gtest.h>

#include "data_page_header.hpp"

#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <limits>

// ---------------------------------------------------------
// Constructor Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, ConstructorValidTupleWidth)
{
    EXPECT_NO_THROW(PageHeader(8));
    EXPECT_NO_THROW(PageHeader(32));
    EXPECT_NO_THROW(PageHeader(40));
}

TEST(PageHeaderTest, ConstructorInvalidTupleWidth)
{
    EXPECT_THROW(PageHeader(0), std::invalid_argument);
    EXPECT_THROW(PageHeader(-1), std::invalid_argument);
    EXPECT_THROW(PageHeader(-100), std::invalid_argument);
}

TEST(PageHeaderTest, ConstructorInitializesDefaults)
{
    PageHeader header(8);

    EXPECT_EQ(header.getLogSequenceNumber(), 0);
    EXPECT_EQ(header.getFormatVersion(), FORMAT_VERSION);
    EXPECT_EQ(header.getFormatID(), FORMAT_ID);
    EXPECT_EQ(header.getTupleWidth(), 8);
    EXPECT_EQ(header.getAllocatedSlotCount(), 0);
}

TEST(PageHeaderTest, ConstructorCalculatesSlotCapacity)
{
    PageHeader header(8);

    EXPECT_EQ(
        header.getSlotCapacity(),
        PageHeader::calcSlotCapacity(8)
    );
}

// ---------------------------------------------------------
// Getter Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, GetTupleWidth)
{
    PageHeader header(40);

    EXPECT_EQ(header.getTupleWidth(), 40);
}

TEST(PageHeaderTest, GetSlotCapacity)
{
    PageHeader header(8);

    EXPECT_EQ(header.getSlotCapacity(), 493);
}

TEST(PageHeaderTest, GetAllocatedSlotCount)
{
    PageHeader header(8);

    EXPECT_EQ(header.getAllocatedSlotCount(), 0);
}

// ---------------------------------------------------------
// Bitmap Size Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, BitmapSizeZeroSlots)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(0), 0);
}

TEST(PageHeaderTest, BitmapSizeOneSlot)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(1), 8);
}

TEST(PageHeaderTest, BitmapSize63Slots)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(63), 8);
}

TEST(PageHeaderTest, BitmapSize64Slots)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(64), 8);
}

TEST(PageHeaderTest, BitmapSize65Slots)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(65), 16);
}

TEST(PageHeaderTest, BitmapSize128Slots)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(128), 16);
}

TEST(PageHeaderTest, BitmapSize129Slots)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(129), 24);
}

TEST(PageHeaderTest, BitmapSize1024Slots)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(1024), 128);
}

// ---------------------------------------------------------
// Slot Capacity Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, SlotCapacityEightByteTuple)
{
    EXPECT_EQ(PageHeader::calcSlotCapacity(8), 493);
}

TEST(PageHeaderTest, SlotCapacity16ByteTuple)
{
    EXPECT_EQ(PageHeader::calcSlotCapacity(16), 250);
}

TEST(PageHeaderTest, SlotCapacity32ByteTuple)
{
    EXPECT_EQ(PageHeader::calcSlotCapacity(32), 126);
}

TEST(PageHeaderTest, SlotCapacity40ByteTuple)
{
    EXPECT_EQ(PageHeader::calcSlotCapacity(40), 101);
}

TEST(PageHeaderTest, SlotCapacity64ByteTuple)
{
    EXPECT_EQ(PageHeader::calcSlotCapacity(64), 63);
}

TEST(PageHeaderTest, SlotCapacity128ByteTuple)
{
    EXPECT_EQ(PageHeader::calcSlotCapacity(128), 31);
}

TEST(PageHeaderTest, SlotCapacity256ByteTuple)
{
    EXPECT_EQ(PageHeader::calcSlotCapacity(256), 15);
}

TEST(PageHeaderTest, SlotCapacity512ByteTuple)
{
    EXPECT_EQ(PageHeader::calcSlotCapacity(512), 7);
}

TEST(PageHeaderTest, SlotCapacityRejectsInvalidTupleWidth)
{
    EXPECT_THROW(
        PageHeader::calcSlotCapacity(0),
        std::invalid_argument
    );

    EXPECT_THROW(
        PageHeader::calcSlotCapacity(-1),
        std::invalid_argument
    );
}

TEST(PageHeaderTest, SlotCapacityFitsWithinPage)
{
    std::vector<std::int16_t> tupleWidths = {
        8, 16, 32, 40, 64, 128, 256, 512
    };

    for (std::int16_t tupleWidth : tupleWidths)
    {
        std::size_t capacity =
            PageHeader::calcSlotCapacity(tupleWidth);

        std::size_t bitmapSize =
            PageHeader::calcBitmapSize(capacity);

        std::size_t totalSize =
            HEADER_BYTE_LENGTH +
            2 * bitmapSize +
            capacity * tupleWidth;

        EXPECT_LE(totalSize, PAGE_BYTE_LENGTH)
            << "Tuple width: " << tupleWidth;
    }
}

TEST(PageHeaderTest, SlotCapacityIsMaximumPossible)
{
    std::vector<std::int16_t> tupleWidths = {
        8, 16, 32, 40, 64, 128, 256, 512
    };

    for (std::int16_t tupleWidth : tupleWidths)
    {
        std::size_t capacity =
            PageHeader::calcSlotCapacity(tupleWidth);

        std::size_t nextCapacity = capacity + 1;

        std::size_t totalSize =
            HEADER_BYTE_LENGTH +
            2 * PageHeader::calcBitmapSize(nextCapacity) +
            nextCapacity * tupleWidth;

        EXPECT_GT(totalSize, PAGE_BYTE_LENGTH)
            << "Tuple width: " << tupleWidth;
    }
}

// ---------------------------------------------------------
// Allocated Slot Count Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, SetAllocatedSlotCountValid)
{
    PageHeader header(8);

    header.setAllocatedSlotCount(10);

    EXPECT_EQ(header.getAllocatedSlotCount(), 10);
}

TEST(PageHeaderTest, SetAllocatedSlotCountZero)
{
    PageHeader header(8);

    header.setAllocatedSlotCount(0);

    EXPECT_EQ(header.getAllocatedSlotCount(), 0);
}

TEST(PageHeaderTest, SetAllocatedSlotCountAtCapacity)
{
    PageHeader header(8);

    std::int16_t capacity = header.getSlotCapacity();

    EXPECT_NO_THROW(header.setAllocatedSlotCount(capacity));

    EXPECT_EQ(header.getAllocatedSlotCount(), capacity);
}

TEST(PageHeaderTest, SetAllocatedSlotCountExceedsCapacity)
{
    PageHeader header(8);

    std::int16_t capacity = header.getSlotCapacity();

    EXPECT_THROW(
        header.setAllocatedSlotCount(capacity + 1),
        std::invalid_argument
    );
}

TEST(PageHeaderTest, SetAllocatedSlotCountNegative)
{
    PageHeader header(8);

    EXPECT_THROW(
        header.setAllocatedSlotCount(-1),
        std::invalid_argument
    );
}

TEST(PageHeaderTest, InvalidAllocationPreservesPreviousCount)
{
    PageHeader header(8);

    header.setAllocatedSlotCount(10);

    EXPECT_THROW(
        header.setAllocatedSlotCount(500),
        std::invalid_argument
    );

    EXPECT_EQ(header.getAllocatedSlotCount(), 10);
}

// ---------------------------------------------------------
// Encoding Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, EncodeProducesCorrectHeaderSize)
{
    PageHeader header(8);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(bytes.size(), HEADER_BYTE_LENGTH);
}

TEST(PageHeaderTest, EncodeIsDeterministic)
{
    PageHeader header(8);

    header.setAllocatedSlotCount(10);

    std::vector<std::byte> first = header.encode();
    std::vector<std::byte> second = header.encode();

    EXPECT_EQ(first, second);
}

TEST(PageHeaderTest, EncodeReservedPaddingIsZero)
{
    PageHeader header(8);

    std::vector<std::byte> bytes = header.encode();

    ASSERT_EQ(bytes.size(), HEADER_BYTE_LENGTH);

    for (std::size_t i = 18; i < HEADER_BYTE_LENGTH; ++i)
    {
        EXPECT_EQ(bytes[i], std::byte{0})
            << "Padding byte: " << i;
    }
}

TEST(PageHeaderTest, EncodeTupleWidthLittleEndian)
{
    PageHeader header(256);

    std::vector<std::byte> bytes = header.encode();

    ASSERT_EQ(bytes.size(), HEADER_BYTE_LENGTH);

    // Tuple width occupies bytes [12, 13].
    // 256 = 0x0100.
    EXPECT_EQ(bytes[12], std::byte{0x00});
    EXPECT_EQ(bytes[13], std::byte{0x01});
}

TEST(PageHeaderTest, EncodeAllocatedSlotCountLittleEndian)
{
    PageHeader header(8);

    header.setAllocatedSlotCount(258);

    std::vector<std::byte> bytes = header.encode();

    ASSERT_EQ(bytes.size(), HEADER_BYTE_LENGTH);

    // Allocated count occupies bytes [16, 17].
    // 258 = 0x0102.
    EXPECT_EQ(bytes[16], std::byte{0x02});
    EXPECT_EQ(bytes[17], std::byte{0x01});
}

// ---------------------------------------------------------
// Static Byte Getter Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, StaticGetLogSequenceNumber)
{
    PageHeader header(8);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(PageHeader::getLogSequenceNumber(bytes), 0);
}

TEST(PageHeaderTest, StaticGetFormatVersion)
{
    PageHeader header(8);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(
        PageHeader::getFormatVersion(bytes),
        FORMAT_VERSION
    );
}

TEST(PageHeaderTest, StaticGetFormatID)
{
    PageHeader header(8);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(
        PageHeader::getFormatID(bytes),
        FORMAT_ID
    );
}

TEST(PageHeaderTest, StaticGetTupleWidth)
{
    PageHeader header(40);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(PageHeader::getTupleWidth(bytes), 40);
}

TEST(PageHeaderTest, StaticGetSlotCapacity)
{
    PageHeader header(8);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(PageHeader::getSlotCapacity(bytes), 493);
}

TEST(PageHeaderTest, StaticGetAllocatedSlotCount)
{
    PageHeader header(8);

    header.setAllocatedSlotCount(100);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(
        PageHeader::getAllocatedSlotCount(bytes),
        100
    );
}

TEST(PageHeaderTest, StaticGettersReadCorrectOffsets)
{
    std::vector<std::byte> bytes(
        HEADER_BYTE_LENGTH,
        std::byte{0}
    );

    // Log sequence number = 42
    bytes[0] = std::byte{0x2A};

    // Format version = 3
    bytes[8] = std::byte{0x03};

    // Format ID = 7
    bytes[10] = std::byte{0x07};

    // Tuple width = 40
    bytes[12] = std::byte{0x28};

    // Slot capacity = 101
    bytes[14] = std::byte{0x65};

    // Allocated slots = 25
    bytes[16] = std::byte{0x19};

    EXPECT_EQ(PageHeader::getLogSequenceNumber(bytes), 42);
    EXPECT_EQ(PageHeader::getFormatVersion(bytes), 3);
    EXPECT_EQ(PageHeader::getFormatID(bytes), 7);
    EXPECT_EQ(PageHeader::getTupleWidth(bytes), 40);
    EXPECT_EQ(PageHeader::getSlotCapacity(bytes), 101);
    EXPECT_EQ(PageHeader::getAllocatedSlotCount(bytes), 25);
}

// ---------------------------------------------------------
// Decode Validation Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, DecodeEmptyBuffer)
{
    std::vector<std::byte> bytes;

    EXPECT_THROW(
        PageHeader::decode(bytes),
        std::invalid_argument
    );
}

TEST(PageHeaderTest, DecodeBufferTooSmall)
{
    std::vector<std::byte> bytes(
        HEADER_BYTE_LENGTH - 1,
        std::byte{0}
    );

    EXPECT_THROW(
        PageHeader::decode(bytes),
        std::invalid_argument
    );
}

TEST(PageHeaderTest, DecodeBufferTooLarge)
{
    std::vector<std::byte> bytes(
        HEADER_BYTE_LENGTH + 1,
        std::byte{0}
    );

    EXPECT_THROW(
        PageHeader::decode(bytes),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Round-Trip Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, EncodeDecodeRoundTrip)
{
    PageHeader original(8);

    std::vector<std::byte> bytes = original.encode();

    PageHeader decoded = PageHeader::decode(bytes);

    EXPECT_EQ(
        decoded.getLogSequenceNumber(),
        original.getLogSequenceNumber()
    );

    EXPECT_EQ(
        decoded.getFormatVersion(),
        original.getFormatVersion()
    );

    EXPECT_EQ(
        decoded.getFormatID(),
        original.getFormatID()
    );

    EXPECT_EQ(
        decoded.getTupleWidth(),
        original.getTupleWidth()
    );

    EXPECT_EQ(
        decoded.getSlotCapacity(),
        original.getSlotCapacity()
    );

    EXPECT_EQ(
        decoded.getAllocatedSlotCount(),
        original.getAllocatedSlotCount()
    );
}

TEST(PageHeaderTest, RoundTripWithAllocatedSlots)
{
    PageHeader original(40);

    original.setAllocatedSlotCount(50);

    std::vector<std::byte> bytes = original.encode();

    PageHeader decoded = PageHeader::decode(bytes);

    EXPECT_EQ(decoded.getTupleWidth(), 40);
    EXPECT_EQ(decoded.getSlotCapacity(), 101);
    EXPECT_EQ(decoded.getAllocatedSlotCount(), 50);
}

TEST(PageHeaderTest, RoundTripPreservesAllFields)
{
    PageHeader original(32);

    original.setAllocatedSlotCount(25);

    std::vector<std::byte> bytes = original.encode();

    PageHeader decoded = PageHeader::decode(bytes);

    EXPECT_EQ(decoded.getLogSequenceNumber(), 0);
    EXPECT_EQ(decoded.getFormatVersion(), FORMAT_VERSION);
    EXPECT_EQ(decoded.getFormatID(), FORMAT_ID);
    EXPECT_EQ(decoded.getTupleWidth(), 32);
    EXPECT_EQ(decoded.getSlotCapacity(), 126);
    EXPECT_EQ(decoded.getAllocatedSlotCount(), 25);
}

TEST(PageHeaderTest, ReencodingDecodedHeaderProducesSameBytes)
{
    PageHeader original(32);

    original.setAllocatedSlotCount(25);

    std::vector<std::byte> originalBytes = original.encode();

    PageHeader decoded = PageHeader::decode(originalBytes);

    std::vector<std::byte> decodedBytes = decoded.encode();

    EXPECT_EQ(originalBytes, decodedBytes);
}
