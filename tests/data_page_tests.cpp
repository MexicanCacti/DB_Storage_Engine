
#include <gtest/gtest.h>

#include "data_page.hpp"

#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

// ---------------------------------------------------------
// Initialization Tests
// ---------------------------------------------------------

TEST(DataPageTest, InitializeValidPage)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    EXPECT_NO_THROW(dataPage.Initialize(8));
}

TEST(DataPageTest, InitializeSetsCorrectHeader)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_EQ(dataPage.getTupleWidth(), 8);
    EXPECT_EQ(dataPage.getSlotCapacity(), 493);
    EXPECT_EQ(dataPage.getAllocatedSlotCount(), 0);
}

TEST(DataPageTest, InitializeZeroesEntirePage)
{
    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0xFF}
    );

    Page page(PageID{1, 0}, bytes);
    DataPage dataPage(page);

    dataPage.Initialize(8);

    const auto& pageBytes = page.getPageBytesConst();

    // Header contains nonzero metadata.
    // All bytes after the header must be zero.
    for (std::size_t i = HEADER_BYTE_LENGTH;
         i < PAGE_BYTE_LENGTH;
         ++i)
    {
        EXPECT_EQ(pageBytes[i], std::byte{0})
            << "Byte offset: " << i;
    }
}

TEST(DataPageTest, InitializeRejectsZeroTupleWidth)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    EXPECT_THROW(
        dataPage.Initialize(0),
        std::invalid_argument
    );
}

TEST(DataPageTest, InitializeRejectsNegativeTupleWidth)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    EXPECT_THROW(
        dataPage.Initialize(-8),
        std::invalid_argument
    );
}

TEST(DataPageTest, InitializeInvalidWidthPreservesBytes)
{
    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    Page page(PageID{1, 0}, bytes);
    DataPage dataPage(page);

    std::vector<std::byte> originalBytes =
        page.getPageBytesConst();

    EXPECT_THROW(
        dataPage.Initialize(0),
        std::invalid_argument
    );

    EXPECT_EQ(page.getPageBytesConst(), originalBytes);
}

TEST(DataPageTest, InitializeRejectsInvalidPageSize)
{
    Page page(PageID{1, 0});

    DataPage dataPage(page);

    page.getPageBytes().resize(PAGE_BYTE_LENGTH - 1);

    EXPECT_THROW(
        dataPage.Initialize(8),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Page ID Tests
// ---------------------------------------------------------

TEST(DataPageTest, GetPageID)
{
    Page page(PageID{42, 15});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_EQ(dataPage.getPageID().tableID, 42);
    EXPECT_EQ(dataPage.getPageID().pageNumber, 15);
}

TEST(DataPageTest, GetPageReturnsOriginalPage)
{
    Page page(PageID{1, 5});
    DataPage dataPage(page);

    EXPECT_EQ(&dataPage.getPage(), &page);
    EXPECT_EQ(&dataPage.getPageConst(), &page);
}

// ---------------------------------------------------------
// Bitmap State Tests
// ---------------------------------------------------------

TEST(DataPageTest, AllSlotsInitiallyUnallocated)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    for (std::size_t i = 0;
         i < static_cast<std::size_t>(dataPage.getSlotCapacity());
         ++i)
    {
        EXPECT_FALSE(dataPage.isAllocated(i));
    }
}

TEST(DataPageTest, AllSlotsInitiallyNotDeleted)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    for (std::size_t i = 0;
         i < static_cast<std::size_t>(dataPage.getSlotCapacity());
         ++i)
    {
        EXPECT_FALSE(dataPage.isDeleted(i));
    }
}

TEST(DataPageTest, InitiallyHasFreeSlot)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_TRUE(dataPage.hasFreeSlot());
}

// ---------------------------------------------------------
// Slot Offset Tests
// ---------------------------------------------------------

TEST(DataPageTest, FirstSlotOffset)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::size_t bitmapSize =
        PageHeader::calcBitmapSize(493);

    std::size_t expectedOffset =
        HEADER_BYTE_LENGTH + 2 * bitmapSize;

    EXPECT_EQ(dataPage.getSlotOffset(0), expectedOffset);
}

TEST(DataPageTest, ConsecutiveSlotOffsets)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(40);

    std::size_t firstOffset = dataPage.getSlotOffset(0);
    std::size_t secondOffset = dataPage.getSlotOffset(1);
    std::size_t thirdOffset = dataPage.getSlotOffset(2);

    EXPECT_EQ(secondOffset - firstOffset, 40);
    EXPECT_EQ(thirdOffset - secondOffset, 40);
}

TEST(DataPageTest, LastSlotFitsInsidePage)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::size_t lastSlot =
        dataPage.getSlotCapacity() - 1;

    std::size_t offset =
        dataPage.getSlotOffset(lastSlot);

    EXPECT_LE(offset + dataPage.getTupleWidth(),
              PAGE_BYTE_LENGTH);
}

TEST(DataPageTest, RejectsSlotIndexAtCapacity)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_THROW(
        dataPage.getSlotOffset(dataPage.getSlotCapacity()),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Tuple Insertion Tests
// ---------------------------------------------------------

TEST(DataPageTest, InsertValidTuple)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    EXPECT_NO_THROW(dataPage.insertTuple(tuple));
}

TEST(DataPageTest, InsertReturnsCorrectRID)
{
    Page page(PageID{42, 5});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    RID rid = dataPage.insertTuple(tuple);

    EXPECT_EQ(rid.pageID.tableID, 42);
    EXPECT_EQ(rid.pageID.pageNumber, 5);
    EXPECT_EQ(rid.tupleSlot, 0);
}

TEST(DataPageTest, InsertMarksSlotAllocated)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    RID rid = dataPage.insertTuple(tuple);

    EXPECT_TRUE(dataPage.isAllocated(rid.tupleSlot));
    EXPECT_FALSE(dataPage.isDeleted(rid.tupleSlot));
}

TEST(DataPageTest, InsertIncrementsAllocatedCount)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    dataPage.insertTuple(tuple);

    EXPECT_EQ(dataPage.getAllocatedSlotCount(), 1);

    dataPage.insertTuple(tuple);

    EXPECT_EQ(dataPage.getAllocatedSlotCount(), 2);
}

TEST(DataPageTest, InsertUsesConsecutiveFreeSlots)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    RID first = dataPage.insertTuple(tuple);
    RID second = dataPage.insertTuple(tuple);
    RID third = dataPage.insertTuple(tuple);

    EXPECT_EQ(first.tupleSlot, 0);
    EXPECT_EQ(second.tupleSlot, 1);
    EXPECT_EQ(third.tupleSlot, 2);
}

TEST(DataPageTest, InsertRejectsWrongTupleSize)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tooSmall(7, std::byte{0});
    std::vector<std::byte> tooLarge(9, std::byte{0});

    EXPECT_THROW(
        dataPage.insertTuple(tooSmall),
        std::invalid_argument
    );

    EXPECT_THROW(
        dataPage.insertTuple(tooLarge),
        std::invalid_argument
    );
}

TEST(DataPageTest, InsertPreservesTupleBytes)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
        std::byte{0x06},
        std::byte{0x07},
        std::byte{0x08}
    };

    RID rid = dataPage.insertTuple(tuple);

    std::size_t offset =
        dataPage.getSlotOffset(rid.tupleSlot);

    const auto& bytes = page.getPageBytesConst();

    for (std::size_t i = 0; i < tuple.size(); ++i)
    {
        EXPECT_EQ(bytes[offset + i], tuple[i]);
    }
}

TEST(DataPageTest, InsertIntoFullPageThrows)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(512);

    std::vector<std::byte> tuple(512, std::byte{0xAB});

    std::size_t capacity = dataPage.getSlotCapacity();

    for (std::size_t i = 0; i < capacity; ++i)
    {
        dataPage.insertTuple(tuple);
    }

    EXPECT_FALSE(dataPage.hasFreeSlot());

    EXPECT_THROW(
        dataPage.insertTuple(tuple),
        std::exception
    );
}

// ---------------------------------------------------------
// Tuple Read Tests
// ---------------------------------------------------------

TEST(DataPageTest, ReadInsertedTuple)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
        std::byte{0x06},
        std::byte{0x07},
        std::byte{0x08}
    };

    RID rid = dataPage.insertTuple(tuple);

    std::vector<std::byte> result =
        dataPage.readTuple(rid.tupleSlot);

    EXPECT_EQ(result, tuple);
}

TEST(DataPageTest, ReadUnallocatedSlotThrows)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_THROW(
        dataPage.readTuple(0),
        std::invalid_argument
    );
}

TEST(DataPageTest, ReadInvalidSlotIndexThrows)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_THROW(
        dataPage.readTuple(dataPage.getSlotCapacity()),
        std::exception
    );
}

// ---------------------------------------------------------
// Tuple Update Tests
// ---------------------------------------------------------

TEST(DataPageTest, UpdateExistingTuple)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> original(8, std::byte{0xAA});
    std::vector<std::byte> updated(8, std::byte{0xBB});

    RID rid = dataPage.insertTuple(original);

    dataPage.updateTuple(rid.tupleSlot, updated);

    EXPECT_EQ(dataPage.readTuple(rid.tupleSlot), updated);
}

TEST(DataPageTest, UpdatePreservesRID)
{
    Page page(PageID{42, 5});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> original(8, std::byte{0xAA});
    std::vector<std::byte> updated(8, std::byte{0xBB});

    RID originalRID = dataPage.insertTuple(original);

    RID updatedRID =
        dataPage.updateTuple(originalRID.tupleSlot, updated);

    EXPECT_EQ(updatedRID.pageID.tableID, 42);
    EXPECT_EQ(updatedRID.pageID.pageNumber, 5);
    EXPECT_EQ(updatedRID.tupleSlot, originalRID.tupleSlot);
}

TEST(DataPageTest, UpdateDoesNotChangeAllocatedCount)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> original(8, std::byte{0xAA});
    std::vector<std::byte> updated(8, std::byte{0xBB});

    RID rid = dataPage.insertTuple(original);

    dataPage.updateTuple(rid.tupleSlot, updated);

    EXPECT_EQ(dataPage.getAllocatedSlotCount(), 1);
}

TEST(DataPageTest, UpdateUnallocatedSlotThrows)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    EXPECT_THROW(
        dataPage.updateTuple(0, tuple),
        std::invalid_argument
    );
}

TEST(DataPageTest, UpdateRejectsWrongTupleSize)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> original(8, std::byte{0xAA});
    std::vector<std::byte> invalid(7, std::byte{0xBB});

    RID rid = dataPage.insertTuple(original);

    EXPECT_THROW(
        dataPage.updateTuple(rid.tupleSlot, invalid),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Tuple Deletion Tests
// ---------------------------------------------------------

TEST(DataPageTest, DeleteMarksTupleDeleted)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    RID rid = dataPage.insertTuple(tuple);

    dataPage.deleteTuple(rid.tupleSlot);

    EXPECT_TRUE(dataPage.isDeleted(rid.tupleSlot));
}

TEST(DataPageTest, DeletePreservesAllocationBit)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    RID rid = dataPage.insertTuple(tuple);

    dataPage.deleteTuple(rid.tupleSlot);

    EXPECT_TRUE(dataPage.isAllocated(rid.tupleSlot));
}

TEST(DataPageTest, ReadDeletedTupleThrows)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    RID rid = dataPage.insertTuple(tuple);

    dataPage.deleteTuple(rid.tupleSlot);

    EXPECT_THROW(
        dataPage.readTuple(rid.tupleSlot),
        std::invalid_argument
    );
}

TEST(DataPageTest, DeleteUnallocatedSlotThrows)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_THROW(
        dataPage.deleteTuple(0),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Open Tests
// ---------------------------------------------------------

TEST(DataPageTest, OpenValidInitializedPage)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_NO_THROW(dataPage.Open(8));
}

TEST(DataPageTest, OpenRejectsWrongTupleWidth)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    EXPECT_THROW(
        dataPage.Open(40),
        std::invalid_argument
    );
}

TEST(DataPageTest, OpenDoesNotModifyPageBytes)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::vector<std::byte> originalBytes =
        page.getPageBytesConst();

    dataPage.Open(8);

    EXPECT_EQ(page.getPageBytesConst(), originalBytes);
}

TEST(DataPageTest, OpenRejectsInvalidPageSize)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    page.getPageBytes().resize(PAGE_BYTE_LENGTH - 1);

    EXPECT_THROW(
        dataPage.Open(8),
        std::invalid_argument
    );
}

TEST(DataPageTest, OpenRejectsInvalidAllocatedCount)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    // Corrupt allocated-slot count at bytes [16, 17].
    page.getPageBytes()[16] = std::byte{0x01};

    EXPECT_THROW(
        dataPage.Open(8),
        std::exception
    );
}

TEST(DataPageTest, OpenRejectsDeletedUnallocatedSlot)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    std::size_t deletionBitmapOffset =
        HEADER_BYTE_LENGTH +
        PageHeader::calcBitmapSize(dataPage.getSlotCapacity());

    // Mark slot 0 deleted without marking it allocated.
    page.getPageBytes()[deletionBitmapOffset] =
        std::byte{0x01};

    EXPECT_THROW(
        dataPage.Open(8),
        std::exception
    );
}

TEST(DataPageTest, OpenRejectsCorruptedSlotCapacity)
{
    Page page(PageID{1, 0});
    DataPage dataPage(page);

    dataPage.Initialize(8);

    // Slot capacity is stored at bytes [14, 15].
    page.getPageBytes()[14] = std::byte{0x01};
    page.getPageBytes()[15] = std::byte{0x00};

    EXPECT_THROW(
        dataPage.Open(8),
        std::exception
    );
}

// ---------------------------------------------------------
// Persistence Tests
// ---------------------------------------------------------

TEST(DataPageTest, ReopenCopiedPagePreservesTuple)
{
    Page originalPage(PageID{1, 5});
    DataPage originalDataPage(originalPage);

    originalDataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    RID rid = originalDataPage.insertTuple(tuple);

    std::vector<std::byte> copiedBytes =
        originalPage.getPageBytesConst();

    Page copiedPage(PageID{1, 5}, copiedBytes);
    DataPage reopenedDataPage(copiedPage);

    EXPECT_NO_THROW(reopenedDataPage.Open(8));

    EXPECT_EQ(reopenedDataPage.getAllocatedSlotCount(), 1);
    EXPECT_TRUE(reopenedDataPage.isAllocated(rid.tupleSlot));
    EXPECT_EQ(reopenedDataPage.readTuple(rid.tupleSlot), tuple);
}

TEST(DataPageTest, ReopenCopiedPagePreservesDeletion)
{
    Page originalPage(PageID{1, 5});
    DataPage originalDataPage(originalPage);

    originalDataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});

    RID rid = originalDataPage.insertTuple(tuple);

    originalDataPage.deleteTuple(rid.tupleSlot);

    std::vector<std::byte> copiedBytes =
        originalPage.getPageBytesConst();

    Page copiedPage(PageID{1, 5}, copiedBytes);
    DataPage reopenedDataPage(copiedPage);

    EXPECT_NO_THROW(reopenedDataPage.Open(8));

    EXPECT_TRUE(reopenedDataPage.isAllocated(rid.tupleSlot));
    EXPECT_TRUE(reopenedDataPage.isDeleted(rid.tupleSlot));
}

TEST(DataPageTest, ReopenCopiedPageDoesNotModifyBytes)
{
    Page originalPage(PageID{1, 5});
    DataPage originalDataPage(originalPage);

    originalDataPage.Initialize(8);

    std::vector<std::byte> tuple(8, std::byte{0xAB});
    originalDataPage.insertTuple(tuple);

    std::vector<std::byte> copiedBytes =
        originalPage.getPageBytesConst();

    Page copiedPage(PageID{1, 5}, copiedBytes);
    DataPage reopenedDataPage(copiedPage);

    reopenedDataPage.Open(8);

    EXPECT_EQ(copiedPage.getPageBytesConst(), copiedBytes);
}
