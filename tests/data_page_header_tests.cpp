
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

    EXPECT_GT(header.getSlotCapacity(), 0);
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

TEST(PageHeaderTest, BitmapSizeLargeCapacity)
{
    EXPECT_EQ(PageHeader::calcBitmapSize(1024), 128);
}

// ---------------------------------------------------------
// Slot Capacity Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, SlotCapacityEightByteTuple)
{
    std::int16_t capacity = PageHeader::calcSlotCapacity(8);

    EXPECT_EQ(capacity, 493);
}

TEST(PageHeaderTest, SlotCapacity32ByteTuple)
{
    std::int16_t capacity = PageHeader::calcSlotCapacity(32);

    EXPECT_EQ(capacity, 126);
}

TEST(PageHeaderTest, SlotCapacity40ByteTuple)
{
    std::int16_t capacity = PageHeader::calcSlotCapacity(40);

    EXPECT_EQ(capacity, 101);
}

TEST(PageHeaderTest, SlotCapacity64ByteTuple)
{
    std::int16_t capacity = PageHeader::calcSlotCapacity(64);

    EXPECT_EQ(capacity, 63);
}

TEST(PageHeaderTest, SlotCapacityFitsWithinPage)
{
    std::vector<std::int16_t> tupleWidths = {
        8, 16, 32, 40, 64, 128, 256, 512
    };

    for (std::int16_t tupleWidth : tupleWidths)
    {
        std::int16_t capacity =
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
        std::int16_t capacity =
            PageHeader::calcSlotCapacity(tupleWidth);

        std::size_t nextCapacity =
            static_cast<std::size_t>(capacity) + 1;

        std::size_t bitmapSize =
            PageHeader::calcBitmapSize(nextCapacity);

        std::size_t totalSize =
            HEADER_BYTE_LENGTH +
            2 * bitmapSize +
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

TEST(PageHeaderTest, SetAllocatedSlotCountPreservesPreviousValueOnError)
{
    PageHeader header(8);

    header.setAllocatedSlotCount(10);

    std::int16_t capacity = header.getSlotCapacity();

    EXPECT_THROW(
        header.setAllocatedSlotCount(capacity + 1),
        std::invalid_argument
    );

    EXPECT_EQ(header.getAllocatedSlotCount(), 10);
}

TEST(PageHeaderTest, SetAllocatedSlotCountRejectsNegative)
{
    PageHeader header(8);

    EXPECT_THROW(
        header.setAllocatedSlotCount(-1),
        std::invalid_argument
    );
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
// Encoding Tests
// ---------------------------------------------------------

TEST(PageHeaderTest, EncodeProducesHeaderSize)
{
    PageHeader header(8);

    std::vector<std::byte> bytes = header.encode();

    EXPECT_EQ(bytes.size(), HEADER_BYTE_LENGTH);
}

TEST(PageHeaderTest, EncodeIsDeterministic)
{
    PageHeader header(8);

    std::vector<std::byte> first = header.encode();
    std::vector<std::byte> second = header.encode();

    EXPECT_EQ(first, second);
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
    EXPECT_EQ(decoded.getAllocatedSlotCount(), 50);
    EXPECT_EQ(decoded.getSlotCapacity(),
              original.getSlotCapacity());
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
