
#include <gtest/gtest.h>

#include "page.hpp"

#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <limits>

// ---------------------------------------------------------
// Constructor Tests
// ---------------------------------------------------------

TEST(PageTest, DefaultConstructorValidPage)
{
    PageID pageID{1, 0};

    EXPECT_NO_THROW(Page(pageID));
}

TEST(PageTest, ConstructorWithValidBytes)
{
    PageID pageID{1, 0};

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    EXPECT_NO_THROW(Page(pageID, bytes));
}

TEST(PageTest, ConstructorWithEmptyBytes)
{
    PageID pageID{1, 0};

    std::vector<std::byte> bytes;

    EXPECT_THROW(
        Page(pageID, bytes),
        std::invalid_argument
    );
}

TEST(PageTest, ConstructorWithBytesTooSmall)
{
    PageID pageID{1, 0};

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH - 1,
        std::byte{0}
    );

    EXPECT_THROW(
        Page(pageID, bytes),
        std::invalid_argument
    );
}

TEST(PageTest, ConstructorWithBytesTooLarge)
{
    PageID pageID{1, 0};

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH + 1,
        std::byte{0}
    );

    EXPECT_THROW(
        Page(pageID, bytes),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Page Initialization Tests
// ---------------------------------------------------------

TEST(PageTest, DefaultConstructorAllocatesFullPage)
{
    PageID pageID{1, 0};

    Page page(pageID);

    EXPECT_EQ(page.getPageBytesConst().size(), PAGE_BYTE_LENGTH);
}

TEST(PageTest, DefaultConstructorZeroInitializesPage)
{
    PageID pageID{1, 0};

    Page page(pageID);

    const auto& bytes = page.getPageBytesConst();

    for (std::size_t i = 0; i < PAGE_BYTE_LENGTH; ++i)
    {
        EXPECT_EQ(bytes[i], std::byte{0}) << "Index: " << i;
    }
}

TEST(PageTest, ConstructorCopiesProvidedBytes)
{
    PageID pageID{1, 0};

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    Page page(pageID, bytes);

    EXPECT_EQ(page.getPageBytesConst(), bytes);
}

TEST(PageTest, ConstructorPreservesBytePattern)
{
    PageID pageID{1, 0};

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    for (std::size_t i = 0; i < bytes.size(); ++i)
    {
        bytes[i] = static_cast<std::byte>(i % 256);
    }

    Page page(pageID, bytes);

    EXPECT_EQ(page.getPageBytesConst(), bytes);
}

TEST(PageTest, ConstructorCopiesBytesIndependently)
{
    PageID pageID{1, 0};

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    Page page(pageID, bytes);

    bytes[0] = std::byte{0xFF};

    EXPECT_EQ(page.getPageBytesConst()[0], std::byte{0xAB});
}

// ---------------------------------------------------------
// Getter Tests
// ---------------------------------------------------------

TEST(PageTest, GetPageID)
{
    PageID pageID{10, 25};

    Page page(pageID);

    EXPECT_EQ(page.getPageID().tableID, 10);
    EXPECT_EQ(page.getPageID().pageNumber, 25);
}

TEST(PageTest, GetTableID)
{
    PageID pageID{42, 5};

    Page page(pageID);

    EXPECT_EQ(page.getTableID(), 42);
}

TEST(PageTest, GetPageNumber)
{
    PageID pageID{42, 15};

    Page page(pageID);

    EXPECT_EQ(page.getPageNumber(), 15);
}

TEST(PageTest, GetPageSize)
{
    PageID pageID{1, 0};

    Page page(pageID);

    EXPECT_EQ(page.getPageSize(), PAGE_BYTE_LENGTH);
}

TEST(PageTest, GetPageBytesConst)
{
    PageID pageID{1, 0};

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0x12}
    );

    Page page(pageID, bytes);

    const auto& retrievedBytes = page.getPageBytesConst();

    EXPECT_EQ(retrievedBytes, bytes);
}

// ---------------------------------------------------------
// Mutable Byte Access Tests
// ---------------------------------------------------------

TEST(PageTest, GetPageBytesAllowsModification)
{
    PageID pageID{1, 0};

    Page page(pageID);

    std::vector<std::byte>& bytes = page.getPageBytes();

    bytes[0] = std::byte{0xAB};
    bytes[1] = std::byte{0xCD};

    EXPECT_EQ(page.getPageBytesConst()[0], std::byte{0xAB});
    EXPECT_EQ(page.getPageBytesConst()[1], std::byte{0xCD});
}

TEST(PageTest, ModifyFirstByte)
{
    PageID pageID{1, 0};

    Page page(pageID);

    page.getPageBytes()[0] = std::byte{0xFF};

    EXPECT_EQ(
        page.getPageBytesConst()[0],
        std::byte{0xFF}
    );
}

TEST(PageTest, ModifyLastByte)
{
    PageID pageID{1, 0};

    Page page(pageID);

    page.getPageBytes()[PAGE_BYTE_LENGTH - 1] = std::byte{0xFF};

    EXPECT_EQ(
        page.getPageBytesConst()[PAGE_BYTE_LENGTH - 1],
        std::byte{0xFF}
    );
}

TEST(PageTest, MutableBytesReflectInConstGetter)
{
    PageID pageID{1, 0};

    Page page(pageID);

    page.getPageBytes()[10] = std::byte{0x42};

    const auto& bytes = page.getPageBytesConst();

    EXPECT_EQ(bytes[10], std::byte{0x42});
}

// ---------------------------------------------------------
// Boundary Value Tests
// ---------------------------------------------------------

TEST(PageTest, ZeroPageID)
{
    PageID pageID{0, 0};

    Page page(pageID);

    EXPECT_EQ(page.getTableID(), 0);
    EXPECT_EQ(page.getPageNumber(), 0);
}

TEST(PageTest, NegativePageID)
{
    PageID pageID{-1, -5};

    Page page(pageID);

    EXPECT_EQ(page.getTableID(), -1);
    EXPECT_EQ(page.getPageNumber(), -5);
}

TEST(PageTest, MaxInt64PageID)
{
    std::int64_t maxValue =
        std::numeric_limits<std::int64_t>::max();

    PageID pageID{maxValue, maxValue};

    Page page(pageID);

    EXPECT_EQ(page.getTableID(), maxValue);
    EXPECT_EQ(page.getPageNumber(), maxValue);
}

TEST(PageTest, MinInt64PageID)
{
    std::int64_t minValue =
        std::numeric_limits<std::int64_t>::min();

    PageID pageID{minValue, minValue};

    Page page(pageID);

    EXPECT_EQ(page.getTableID(), minValue);
    EXPECT_EQ(page.getPageNumber(), minValue);
}

// ---------------------------------------------------------
// Copy Semantics Tests
// ---------------------------------------------------------

TEST(PageTest, CopyConstructorPreservesData)
{
    PageID pageID{1, 5};

    Page original(pageID);

    original.getPageBytes()[0] = std::byte{0xAB};

    Page copied(original);

    EXPECT_EQ(copied.getTableID(), original.getTableID());
    EXPECT_EQ(copied.getPageNumber(), original.getPageNumber());
    EXPECT_EQ(copied.getPageBytesConst(),
              original.getPageBytesConst());
}

TEST(PageTest, CopyConstructorCreatesIndependentBytes)
{
    PageID pageID{1, 5};

    Page original(pageID);

    original.getPageBytes()[0] = std::byte{0xAB};

    Page copied(original);

    copied.getPageBytes()[0] = std::byte{0xFF};

    EXPECT_EQ(
        original.getPageBytesConst()[0],
        std::byte{0xAB}
    );

    EXPECT_EQ(
        copied.getPageBytesConst()[0],
        std::byte{0xFF}
    );
}

TEST(PageTest, CopyAssignmentPreservesData)
{
    Page original(PageID{1, 5});
    Page destination(PageID{2, 10});

    original.getPageBytes()[0] = std::byte{0xAB};

    destination = original;

    EXPECT_EQ(destination.getTableID(), 1);
    EXPECT_EQ(destination.getPageNumber(), 5);
    EXPECT_EQ(destination.getPageBytesConst(),
              original.getPageBytesConst());
}
