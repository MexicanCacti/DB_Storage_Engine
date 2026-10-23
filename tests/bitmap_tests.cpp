
#include <gtest/gtest.h>

#include "bitmap.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

class BitmapTest : public ::testing::Test
{
protected:
    static constexpr std::size_t PAGE_SIZE = 4096;
    static constexpr std::size_t BITMAP_OFFSET = 24;
    static constexpr std::int16_t SLOT_COUNT = 128;

    std::vector<std::byte> pageBytes;

    void SetUp() override
    {
        pageBytes.assign(PAGE_SIZE, std::byte{0});
    }
};

// ============================================================
// Constructor Tests
// ============================================================

TEST_F(BitmapTest, ConstructorAcceptsValidBitmap)
{
    EXPECT_NO_THROW(
        Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT)
    );
}

TEST_F(BitmapTest, ConstructorAcceptsExactBoundary)
{
    // 64 slots require 8 bytes.
    std::vector<std::byte> bytes(32, std::byte{0});

    EXPECT_NO_THROW(
        Bitmap bitmap(bytes, 24, 64)
    );
}

TEST_F(BitmapTest, ConstructorRejectsInsufficientSpace)
{
    std::vector<std::byte> bytes(31, std::byte{0});

    EXPECT_THROW(
        Bitmap bitmap(bytes, 24, 64),
        std::invalid_argument
    );
}

TEST_F(BitmapTest, ConstructorRejectsOffsetBeyondPage)
{
    EXPECT_THROW(
        Bitmap bitmap(pageBytes, PAGE_SIZE + 1, 64),
        std::invalid_argument
    );
}

TEST_F(BitmapTest, ConstructorRejectsBitmapExceedingPage)
{
    // 128 slots require 16 bytes.
    EXPECT_THROW(
        Bitmap bitmap(pageBytes, PAGE_SIZE - 8, 128),
        std::invalid_argument
    );
}

// ============================================================
// readBit Tests
// ============================================================

TEST_F(BitmapTest, InitiallyAllBitsAreZero)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    for (std::size_t i = 0; i < SLOT_COUNT; ++i)
    {
        EXPECT_FALSE(bitmap.readBit(i)) << "Index: " << i;
    }
}

TEST_F(BitmapTest, ReadBitReturnsCorrectValues)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    bitmap.setBit(0);
    bitmap.setBit(3);
    bitmap.setBit(7);

    EXPECT_TRUE(bitmap.readBit(0));
    EXPECT_FALSE(bitmap.readBit(1));
    EXPECT_FALSE(bitmap.readBit(2));
    EXPECT_TRUE(bitmap.readBit(3));
    EXPECT_FALSE(bitmap.readBit(4));
    EXPECT_TRUE(bitmap.readBit(7));
}

TEST_F(BitmapTest, ReadBitRejectsOutOfBoundsIndex)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_THROW(bitmap.readBit(SLOT_COUNT), std::invalid_argument);
    EXPECT_THROW(bitmap.readBit(SLOT_COUNT + 1), std::invalid_argument);
    EXPECT_THROW(bitmap.readBit(std::size_t(-1)), std::invalid_argument);
}

// ============================================================
// setBit Tests
// ============================================================

TEST_F(BitmapTest, SetBitChangesZeroToOne)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_FALSE(bitmap.readBit(5));

    bool previous = bitmap.setBit(5);

    EXPECT_FALSE(previous);
    EXPECT_TRUE(bitmap.readBit(5));
}

TEST_F(BitmapTest, SetBitReturnsPreviousValue)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_FALSE(bitmap.setBit(10));
    EXPECT_TRUE(bitmap.setBit(10));
    EXPECT_TRUE(bitmap.readBit(10));
}

TEST_F(BitmapTest, SetBitDoesNotModifyAdjacentBits)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    bitmap.setBit(4);

    for (std::size_t i = 0; i < 8; ++i)
    {
        if (i == 4)
            EXPECT_TRUE(bitmap.readBit(i));
        else
            EXPECT_FALSE(bitmap.readBit(i));
    }
}

TEST_F(BitmapTest, SetAllBitsInSingleByte)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    for (std::size_t i = 0; i < 8; ++i)
        bitmap.setBit(i);

    EXPECT_EQ(pageBytes[BITMAP_OFFSET], std::byte{0xFF});
}

TEST_F(BitmapTest, SetBitAcrossByteBoundaries)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    bitmap.setBit(7);
    bitmap.setBit(8);
    bitmap.setBit(15);
    bitmap.setBit(16);

    EXPECT_TRUE(bitmap.readBit(7));
    EXPECT_TRUE(bitmap.readBit(8));
    EXPECT_TRUE(bitmap.readBit(15));
    EXPECT_TRUE(bitmap.readBit(16));

    EXPECT_EQ(pageBytes[BITMAP_OFFSET], std::byte{0x80});
    EXPECT_EQ(pageBytes[BITMAP_OFFSET + 1], std::byte{0x81});
    EXPECT_EQ(pageBytes[BITMAP_OFFSET + 2], std::byte{0x01});
}

TEST_F(BitmapTest, SetLastValidBit)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_FALSE(bitmap.setBit(SLOT_COUNT - 1));
    EXPECT_TRUE(bitmap.readBit(SLOT_COUNT - 1));
}

TEST_F(BitmapTest, SetBitRejectsInvalidIndex)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_THROW(bitmap.setBit(SLOT_COUNT), std::invalid_argument);
    EXPECT_THROW(bitmap.setBit(std::size_t(-1)), std::invalid_argument);
}

// ============================================================
// clearBit Tests
// ============================================================

TEST_F(BitmapTest, ClearBitChangesOneToZero)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    bitmap.setBit(12);

    EXPECT_TRUE(bitmap.clearBit(12));
    EXPECT_FALSE(bitmap.readBit(12));
}

TEST_F(BitmapTest, ClearBitReturnsPreviousValue)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_FALSE(bitmap.clearBit(5));

    bitmap.setBit(5);

    EXPECT_TRUE(bitmap.clearBit(5));
    EXPECT_FALSE(bitmap.clearBit(5));
}

TEST_F(BitmapTest, ClearBitPreservesOtherBits)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    for (std::size_t i = 0; i < 8; ++i)
        bitmap.setBit(i);

    bitmap.clearBit(3);

    EXPECT_EQ(pageBytes[BITMAP_OFFSET], std::byte{0xF7});

    for (std::size_t i = 0; i < 8; ++i)
    {
        EXPECT_EQ(bitmap.readBit(i), i != 3);
    }
}

TEST_F(BitmapTest, ClearBitAcrossByteBoundary)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    bitmap.setBit(7);
    bitmap.setBit(8);

    bitmap.clearBit(7);

    EXPECT_FALSE(bitmap.readBit(7));
    EXPECT_TRUE(bitmap.readBit(8));
}

TEST_F(BitmapTest, ClearBitRejectsInvalidIndex)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_THROW(bitmap.clearBit(SLOT_COUNT), std::invalid_argument);
    EXPECT_THROW(bitmap.clearBit(std::size_t(-1)), std::invalid_argument);
}

// ============================================================
// Bitmap Offset and Shared Storage Tests
// ============================================================

TEST_F(BitmapTest, OperationsRespectBitmapOffset)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    bitmap.setBit(0);
    bitmap.setBit(8);

    EXPECT_EQ(pageBytes[BITMAP_OFFSET], std::byte{0x01});
    EXPECT_EQ(pageBytes[BITMAP_OFFSET + 1], std::byte{0x01});

    // Bytes before the bitmap must remain unchanged.
    for (std::size_t i = 0; i < BITMAP_OFFSET; ++i)
    {
        EXPECT_EQ(pageBytes[i], std::byte{0});
    }
}

TEST_F(BitmapTest, SeparateBitmapsDoNotInterfere)
{
    constexpr std::size_t allocationOffset = 24;
    constexpr std::size_t deletionOffset = 40;

    Bitmap allocation(pageBytes, allocationOffset, 128);
    Bitmap deletion(pageBytes, deletionOffset, 128);

    allocation.setBit(5);
    allocation.setBit(20);
    deletion.setBit(5);

    EXPECT_TRUE(allocation.readBit(5));
    EXPECT_TRUE(allocation.readBit(20));

    EXPECT_TRUE(deletion.readBit(5));
    EXPECT_FALSE(deletion.readBit(20));

    allocation.clearBit(5);

    EXPECT_FALSE(allocation.readBit(5));
    EXPECT_TRUE(deletion.readBit(5));
}

TEST_F(BitmapTest, ChangesAreVisibleThroughAnotherBitmapInstance)
{
    Bitmap first(pageBytes, BITMAP_OFFSET, SLOT_COUNT);
    Bitmap second(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    first.setBit(25);

    EXPECT_TRUE(second.readBit(25));

    second.clearBit(25);

    EXPECT_FALSE(first.readBit(25));
}

// ============================================================
// findFirstZeroBitIndex Tests
// ============================================================

TEST_F(BitmapTest, FindFirstZeroInEmptyBitmap)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_EQ(bitmap.findFirstZeroBitIndex(0), 0);
    EXPECT_EQ(bitmap.findFirstZeroBitIndex(10), 10);
    EXPECT_EQ(bitmap.findFirstZeroBitIndex(127), 127);
}

TEST_F(BitmapTest, FindFirstZeroAfterAllocatedBits)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    bitmap.setBit(0);
    bitmap.setBit(1);
    bitmap.setBit(2);
    bitmap.setBit(3);

    EXPECT_EQ(bitmap.findFirstZeroBitIndex(0), 4);
}

TEST_F(BitmapTest, FindFirstZeroStartingFromMiddle)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    bitmap.setBit(20);
    bitmap.setBit(21);
    bitmap.setBit(22);

    EXPECT_EQ(bitmap.findFirstZeroBitIndex(20), 23);
}

TEST_F(BitmapTest, FindFirstZeroAcrossByteBoundary)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    for (std::size_t i = 0; i < 16; ++i)
        bitmap.setBit(i);

    EXPECT_EQ(bitmap.findFirstZeroBitIndex(0), 16);
}

TEST_F(BitmapTest, FindFirstZeroWrapsAround)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    // All bits allocated except slot 3.
    for (std::size_t i = 0; i < SLOT_COUNT; ++i)
        bitmap.setBit(i);

    bitmap.clearBit(3);

    EXPECT_EQ(bitmap.findFirstZeroBitIndex(100), 3);
}

TEST_F(BitmapTest, FindFirstZeroAtLastSlot)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    for (std::size_t i = 0; i < SLOT_COUNT - 1; ++i)
        bitmap.setBit(i);

    EXPECT_EQ(bitmap.findFirstZeroBitIndex(0), SLOT_COUNT - 1);
}

TEST_F(BitmapTest, FindFirstZeroWhenAllBitsSet)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    for (std::size_t i = 0; i < SLOT_COUNT; ++i)
        bitmap.setBit(i);

    EXPECT_EQ(
        bitmap.findFirstZeroBitIndex(0),
        std::numeric_limits<std::size_t>::max()
    );
}

TEST_F(BitmapTest, FindFirstZeroRejectsInvalidStartIndex)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, SLOT_COUNT);

    EXPECT_THROW(
        bitmap.findFirstZeroBitIndex(SLOT_COUNT),
        std::invalid_argument
    );

    EXPECT_THROW(
        bitmap.findFirstZeroBitIndex(std::size_t(-1)),
        std::invalid_argument
    );
}

// ============================================================
// Bitmap Size Boundary Tests
// ============================================================

TEST_F(BitmapTest, BitmapSupportsNonMultipleOf64SlotCount)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, 65);

    bitmap.setBit(64);

    EXPECT_TRUE(bitmap.readBit(64));
    EXPECT_FALSE(bitmap.readBit(63));

    EXPECT_THROW(bitmap.readBit(65), std::invalid_argument);
}

TEST_F(BitmapTest, BitmapSupportsSingleSlot)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, 1);

    EXPECT_FALSE(bitmap.readBit(0));
    EXPECT_FALSE(bitmap.setBit(0));
    EXPECT_TRUE(bitmap.readBit(0));
    EXPECT_TRUE(bitmap.clearBit(0));
    EXPECT_FALSE(bitmap.readBit(0));

    EXPECT_THROW(bitmap.readBit(1), std::invalid_argument);
}

TEST_F(BitmapTest, BitmapSupports64Slots)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, 64);

    bitmap.setBit(0);
    bitmap.setBit(63);

    EXPECT_TRUE(bitmap.readBit(0));
    EXPECT_TRUE(bitmap.readBit(63));
}

TEST_F(BitmapTest, BitmapSupports128Slots)
{
    Bitmap bitmap(pageBytes, BITMAP_OFFSET, 128);

    bitmap.setBit(0);
    bitmap.setBit(63);
    bitmap.setBit(64);
    bitmap.setBit(127);

    EXPECT_TRUE(bitmap.readBit(0));
    EXPECT_TRUE(bitmap.readBit(63));
    EXPECT_TRUE(bitmap.readBit(64));
    EXPECT_TRUE(bitmap.readBit(127));
}

TEST_F(BitmapTest, BitmapDoesNotModifyBytesOutsideRegion)
{
    std::fill(pageBytes.begin(), pageBytes.end(), std::byte{0xAA});

    // Reset bitmap region to zero.
    for (std::size_t i = 24; i < 40; ++i)
        pageBytes[i] = std::byte{0};

    Bitmap bitmap(pageBytes, 24, 128);

    bitmap.setBit(0);
    bitmap.setBit(127);

    EXPECT_EQ(pageBytes[23], std::byte{0xAA});
    EXPECT_EQ(pageBytes[40], std::byte{0xAA});
}
