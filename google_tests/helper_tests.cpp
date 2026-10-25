
#include <gtest/gtest.h>

#include "helpers.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

// ---------------------------------------------------------
// appendBytes Tests
// ---------------------------------------------------------

TEST(HelperTest, AppendBytesValid)
{
    std::vector<std::byte> dest = {
        std::byte{0x01},
        std::byte{0x02}
    };

    std::vector<std::byte> src = {
        std::byte{0x03},
        std::byte{0x04}
    };

    appendBytes(dest, src);

    std::vector<std::byte> expected = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}
    };

    EXPECT_EQ(dest, expected);
}

TEST(HelperTest, AppendBytesToEmptyDestination)
{
    std::vector<std::byte> dest;

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02}
    };

    appendBytes(dest, src);

    EXPECT_EQ(dest, src);
}

TEST(HelperTest, AppendBytesEmptySource)
{
    std::vector<std::byte> dest = {
        std::byte{0x01},
        std::byte{0x02}
    };

    std::vector<std::byte> src;
    std::vector<std::byte> original = dest;

    appendBytes(dest, src);

    EXPECT_EQ(dest, original);
}

TEST(HelperTest, AppendBytesBothEmpty)
{
    std::vector<std::byte> dest;
    std::vector<std::byte> src;

    appendBytes(dest, src);

    EXPECT_TRUE(dest.empty());
}

TEST(HelperTest, AppendBytesSelfAppend)
{
    std::vector<std::byte> bytes = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    appendBytes(bytes, bytes);

    std::vector<std::byte> expected = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    EXPECT_EQ(bytes, expected);
}

TEST(HelperTest, AppendBytesSelfAppendEmpty)
{
    std::vector<std::byte> bytes;

    appendBytes(bytes, bytes);

    EXPECT_TRUE(bytes.empty());
}

TEST(HelperTest, AppendBytesDoesNotModifySource)
{
    std::vector<std::byte> dest = {
        std::byte{0x01}
    };

    std::vector<std::byte> src = {
        std::byte{0x02},
        std::byte{0x03}
    };

    std::vector<std::byte> originalSrc = src;

    appendBytes(dest, src);

    EXPECT_EQ(src, originalSrc);
}

// ---------------------------------------------------------
// copyBytes Tests - No Offset
// ---------------------------------------------------------

TEST(HelperTest, CopyBytesValid)
{
    std::vector<std::byte> dest(5, std::byte{0});

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    copyBytes(dest, src);

    std::vector<std::byte> expected = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0},
        std::byte{0}
    };

    EXPECT_EQ(dest, expected);
}

TEST(HelperTest, CopyBytesExactFit)
{
    std::vector<std::byte> dest(3, std::byte{0});

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    EXPECT_NO_THROW(copyBytes(dest, src));
    EXPECT_EQ(dest, src);
}

TEST(HelperTest, CopyBytesSourceTooLarge)
{
    std::vector<std::byte> dest(2, std::byte{0});
    std::vector<std::byte> src(3, std::byte{0xAB});

    std::vector<std::byte> original = dest;

    EXPECT_THROW(
        copyBytes(dest, src),
        std::out_of_range
    );

    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyBytesSelfCopy)
{
    std::vector<std::byte> bytes(3, std::byte{0xAB});

    EXPECT_THROW(
        copyBytes(bytes, bytes),
        std::invalid_argument
    );
}

TEST(HelperTest, CopyBytesEmptySource)
{
    std::vector<std::byte> dest(3, std::byte{0xAB});
    std::vector<std::byte> src;

    std::vector<std::byte> original = dest;

    copyBytes(dest, src);

    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyBytesBothEmpty)
{
    std::vector<std::byte> dest;
    std::vector<std::byte> src;

    EXPECT_NO_THROW(copyBytes(dest, src));
    EXPECT_TRUE(dest.empty());
}

TEST(HelperTest, CopyBytesPreservesRemainingDestination)
{
    std::vector<std::byte> dest(5, std::byte{0xFF});

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02}
    };

    copyBytes(dest, src);

    std::vector<std::byte> expected = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0xFF},
        std::byte{0xFF},
        std::byte{0xFF}
    };

    EXPECT_EQ(dest, expected);
}

// ---------------------------------------------------------
// copyBytes Tests - Reference Offset
// ---------------------------------------------------------

TEST(HelperTest, CopyBytesOffsetValid)
{
    std::vector<std::byte> dest(5, std::byte{0});

    std::vector<std::byte> src = {
        std::byte{0xAA},
        std::byte{0xBB}
    };

    std::size_t offset = 2;

    copyBytes(dest, src, offset);

    std::vector<std::byte> expected = {
        std::byte{0},
        std::byte{0},
        std::byte{0xAA},
        std::byte{0xBB},
        std::byte{0}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 4);
}

TEST(HelperTest, CopyBytesOffsetZero)
{
    std::vector<std::byte> dest(4, std::byte{0});

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02}
    };

    std::size_t offset = 0;

    copyBytes(dest, src, offset);

    EXPECT_EQ(offset, 2);
    EXPECT_EQ(dest[0], std::byte{0x01});
    EXPECT_EQ(dest[1], std::byte{0x02});
}

TEST(HelperTest, CopyBytesOffsetExactFit)
{
    std::vector<std::byte> dest(5, std::byte{0});
    std::vector<std::byte> src(2, std::byte{0xAB});

    std::size_t offset = 3;

    EXPECT_NO_THROW(copyBytes(dest, src, offset));

    EXPECT_EQ(offset, 5);
    EXPECT_EQ(dest[3], std::byte{0xAB});
    EXPECT_EQ(dest[4], std::byte{0xAB});
}

TEST(HelperTest, CopyBytesOffsetSequential)
{
    std::vector<std::byte> dest(6, std::byte{0});

    std::vector<std::byte> first = {
        std::byte{0x01},
        std::byte{0x02}
    };

    std::vector<std::byte> second = {
        std::byte{0x03},
        std::byte{0x04}
    };

    std::size_t offset = 0;

    copyBytes(dest, first, offset);
    copyBytes(dest, second, offset);

    std::vector<std::byte> expected = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0},
        std::byte{0}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 4);
}

TEST(HelperTest, CopyBytesOffsetExceedsBounds)
{
    std::vector<std::byte> dest(5, std::byte{0});
    std::vector<std::byte> src(2, std::byte{0xAB});

    std::size_t offset = 4;

    std::vector<std::byte> original = dest;

    EXPECT_THROW(
        copyBytes(dest, src, offset),
        std::out_of_range
    );

    EXPECT_EQ(offset, 4);
    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyBytesOffsetBeyondDestination)
{
    std::vector<std::byte> dest(5, std::byte{0});
    std::vector<std::byte> src(1, std::byte{0xAB});

    std::size_t offset = 6;

    EXPECT_THROW(
        copyBytes(dest, src, offset),
        std::out_of_range
    );

    EXPECT_EQ(offset, 6);
}

TEST(HelperTest, CopyBytesOffsetEmptySourceAtEnd)
{
    std::vector<std::byte> dest(5, std::byte{0xAA});
    std::vector<std::byte> src;

    std::size_t offset = 5;

    std::vector<std::byte> original = dest;

    EXPECT_NO_THROW(copyBytes(dest, src, offset));

    EXPECT_EQ(dest, original);
    EXPECT_EQ(offset, 5);
}

TEST(HelperTest, CopyBytesOffsetRejectsSelfCopy)
{
    std::vector<std::byte> bytes(5, std::byte{0xAB});

    std::size_t offset = 0;

    EXPECT_THROW(
        copyBytes(bytes, bytes, offset),
        std::invalid_argument
    );

    EXPECT_EQ(offset, 0);
}

// ---------------------------------------------------------
// copyNBytes Tests - No Offset
// ---------------------------------------------------------

TEST(HelperTest, CopyNBytesValid)
{
    std::vector<std::byte> dest(5, std::byte{0});

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    copyNBytes(dest, src, std::size_t{2});

    std::vector<std::byte> expected = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0},
        std::byte{0},
        std::byte{0}
    };

    EXPECT_EQ(dest, expected);
}

TEST(HelperTest, CopyNBytesExactFit)
{
    std::vector<std::byte> dest(3, std::byte{0});
    std::vector<std::byte> src(5, std::byte{0xAB});

    EXPECT_NO_THROW(
        copyNBytes(dest, src, std::size_t{3})
    );

    EXPECT_EQ(
        dest,
        std::vector<std::byte>(3, std::byte{0xAB})
    );
}

TEST(HelperTest, CopyNBytesZero)
{
    std::vector<std::byte> dest(3, std::byte{0xAA});
    std::vector<std::byte> src(3, std::byte{0xBB});

    std::vector<std::byte> original = dest;

    copyNBytes(dest, src, std::size_t{0});

    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyNBytesExceedsSource)
{
    std::vector<std::byte> dest(5, std::byte{0});
    std::vector<std::byte> src(2, std::byte{0xAB});

    EXPECT_THROW(
        copyNBytes(dest, src, std::size_t{3}),
        std::out_of_range
    );
}

TEST(HelperTest, CopyNBytesExceedsDestination)
{
    std::vector<std::byte> dest(2, std::byte{0});
    std::vector<std::byte> src(5, std::byte{0xAB});

    EXPECT_THROW(
        copyNBytes(dest, src, std::size_t{3}),
        std::out_of_range
    );
}

TEST(HelperTest, CopyNBytesRejectsSelfCopy)
{
    std::vector<std::byte> bytes(5, std::byte{0xAB});

    EXPECT_THROW(
        copyNBytes(bytes, bytes, std::size_t{3}),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// copyNBytes Tests - Reference Offset
// ---------------------------------------------------------

TEST(HelperTest, CopyNBytesOffsetValid)
{
    std::vector<std::byte> dest(6, std::byte{0});

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    std::size_t offset = 2;

    copyNBytes(dest, src, offset, 2);

    std::vector<std::byte> expected = {
        std::byte{0},
        std::byte{0},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0},
        std::byte{0}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 4);
}

TEST(HelperTest, CopyNBytesOffsetExactFit)
{
    std::vector<std::byte> dest(5, std::byte{0});
    std::vector<std::byte> src(3, std::byte{0xAB});

    std::size_t offset = 2;

    EXPECT_NO_THROW(
        copyNBytes(dest, src, offset, 3)
    );

    EXPECT_EQ(offset, 5);
    EXPECT_EQ(dest[4], std::byte{0xAB});
}

TEST(HelperTest, CopyNBytesOffsetSequential)
{
    std::vector<std::byte> dest(6, std::byte{0});

    std::vector<std::byte> first = {
        std::byte{0x01},
        std::byte{0x02}
    };

    std::vector<std::byte> second = {
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05}
    };

    std::size_t offset = 0;

    copyNBytes(dest, first, offset, 2);
    copyNBytes(dest, second, offset, 3);

    EXPECT_EQ(offset, 5);

    std::vector<std::byte> expected = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05},
        std::byte{0}
    };

    EXPECT_EQ(dest, expected);
}

TEST(HelperTest, CopyNBytesOffsetExceedsSource)
{
    std::vector<std::byte> dest(5, std::byte{0});
    std::vector<std::byte> src(2, std::byte{0xAB});

    std::size_t offset = 0;

    EXPECT_THROW(
        copyNBytes(dest, src, offset, 3),
        std::out_of_range
    );

    EXPECT_EQ(offset, 0);
}

TEST(HelperTest, CopyNBytesOffsetExceedsDestination)
{
    std::vector<std::byte> dest(5, std::byte{0});
    std::vector<std::byte> src(3, std::byte{0xAB});

    std::size_t offset = 3;
    std::vector<std::byte> original = dest;

    EXPECT_THROW(
        copyNBytes(dest, src, offset, 3),
        std::out_of_range
    );

    EXPECT_EQ(offset, 3);
    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyNBytesOffsetZeroAtEnd)
{
    std::vector<std::byte> dest(5, std::byte{0xAA});
    std::vector<std::byte> src(3, std::byte{0xBB});

    std::size_t offset = 5;
    std::vector<std::byte> original = dest;

    EXPECT_NO_THROW(
        copyNBytes(dest, src, offset, 0)
    );

    EXPECT_EQ(offset, 5);
    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyNBytesOffsetRejectsSelfCopy)
{
    std::vector<std::byte> bytes(5, std::byte{0xAB});

    std::size_t offset = 0;

    EXPECT_THROW(
        copyNBytes(bytes, bytes, offset, 3),
        std::invalid_argument
    );

    EXPECT_EQ(offset, 0);
}

// ---------------------------------------------------------
// copyBytesSrcOffset Tests - No Offset
// ---------------------------------------------------------

TEST(HelperTest, CopyBytesSrcOffsetValid)
{
    std::vector<std::byte> dest(5, std::byte{0});

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    copyBytesSrcOffset(dest, src);

    std::vector<std::byte> expected = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0},
        std::byte{0}
    };

    EXPECT_EQ(dest, expected);
}

TEST(HelperTest, CopyBytesSrcOffsetExactFit)
{
    std::vector<std::byte> dest(3, std::byte{0});

    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    EXPECT_NO_THROW(copyBytesSrcOffset(dest, src));

    EXPECT_EQ(dest, src);
}

TEST(HelperTest, CopyBytesSrcOffsetSourceTooLarge)
{
    std::vector<std::byte> dest(2, std::byte{0});
    std::vector<std::byte> src(3, std::byte{0xAB});

    std::vector<std::byte> original = dest;

    EXPECT_THROW(
        copyBytesSrcOffset(dest, src),
        std::out_of_range
    );

    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyBytesSrcOffsetEmptySource)
{
    std::vector<std::byte> dest(3, std::byte{0xAA});
    std::vector<std::byte> src;

    std::vector<std::byte> original = dest;

    copyBytesSrcOffset(dest, src);

    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyBytesSrcOffsetRejectsSelfCopy)
{
    std::vector<std::byte> bytes(5, std::byte{0xAB});

    EXPECT_THROW(
        copyBytesSrcOffset(bytes, bytes),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// copyBytesSrcOffset Tests - Reference Offset
// ---------------------------------------------------------

TEST(HelperTest, CopyBytesSrcOffsetReferenceValid)
{
    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05}
    };

    std::vector<std::byte> dest(3, std::byte{0});

    std::size_t offset = 2;

    copyBytesSrcOffset(dest, src, offset);

    std::vector<std::byte> expected = {
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 5);
}

TEST(HelperTest, CopyBytesSrcOffsetReferenceExactFit)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(2, std::byte{0});

    std::size_t offset = 3;

    EXPECT_NO_THROW(
        copyBytesSrcOffset(dest, src, offset)
    );

    EXPECT_EQ(offset, 5);
    EXPECT_EQ(dest[0], std::byte{0xAB});
    EXPECT_EQ(dest[1], std::byte{0xAB});
}

TEST(HelperTest, CopyBytesSrcOffsetReferenceAtEnd)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(3, std::byte{0});

    std::size_t offset = 5;
    std::vector<std::byte> original = dest;

    EXPECT_NO_THROW(
        copyBytesSrcOffset(dest, src, offset)
    );

    EXPECT_EQ(offset, 5);
    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyBytesSrcOffsetReferenceBeyondSource)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(3, std::byte{0});

    std::size_t offset = 6;

    EXPECT_THROW(
        copyBytesSrcOffset(dest, src, offset),
        std::out_of_range
    );

    EXPECT_EQ(offset, 6);
}

TEST(HelperTest, CopyBytesSrcOffsetReferenceDestinationTooSmall)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(2, std::byte{0});

    std::size_t offset = 2;
    std::vector<std::byte> original = dest;

    EXPECT_THROW(
        copyBytesSrcOffset(dest, src, offset),
        std::out_of_range
    );

    EXPECT_EQ(offset, 2);
    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyBytesSrcOffsetReferenceRejectsSelfCopy)
{
    std::vector<std::byte> bytes(5, std::byte{0xAB});

    std::size_t offset = 2;

    EXPECT_THROW(
        copyBytesSrcOffset(bytes, bytes, offset),
        std::invalid_argument
    );

    EXPECT_EQ(offset, 2);
}

// ---------------------------------------------------------
// copyNBytesSrcOffset Tests - Reference Offset
// ---------------------------------------------------------

TEST(HelperTest, CopyNBytesSrcOffsetValid)
{
    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05}
    };

    std::vector<std::byte> dest(2, std::byte{0});

    std::size_t offset = 2;

    copyNBytesSrcOffset(dest, src, offset, 2);

    std::vector<std::byte> expected = {
        std::byte{0x03},
        std::byte{0x04}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 4);
}

TEST(HelperTest, CopyNBytesSrcOffsetExactFit)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(2, std::byte{0});

    std::size_t offset = 3;

    EXPECT_NO_THROW(
        copyNBytesSrcOffset(dest, src, offset, 2)
    );

    EXPECT_EQ(offset, 5);
    EXPECT_EQ(dest[0], std::byte{0xAB});
    EXPECT_EQ(dest[1], std::byte{0xAB});
}

TEST(HelperTest, CopyNBytesSrcOffsetSequential)
{
    std::vector<std::byte> src = {
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05}
    };

    std::vector<std::byte> first(2, std::byte{0});
    std::vector<std::byte> second(3, std::byte{0});

    std::size_t offset = 0;

    copyNBytesSrcOffset(first, src, offset, 2);
    copyNBytesSrcOffset(second, src, offset, 3);

    EXPECT_EQ(offset, 5);

    EXPECT_EQ(first[0], std::byte{0x01});
    EXPECT_EQ(first[1], std::byte{0x02});

    EXPECT_EQ(second[0], std::byte{0x03});
    EXPECT_EQ(second[1], std::byte{0x04});
    EXPECT_EQ(second[2], std::byte{0x05});
}

TEST(HelperTest, CopyNBytesSrcOffsetZeroBytes)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(3, std::byte{0xAA});

    std::size_t offset = 5;
    std::vector<std::byte> original = dest;

    EXPECT_NO_THROW(
        copyNBytesSrcOffset(dest, src, offset, 0)
    );

    EXPECT_EQ(offset, 5);
    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyNBytesSrcOffsetExceedsSource)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(3, std::byte{0});

    std::size_t offset = 4;
    std::vector<std::byte> original = dest;

    EXPECT_THROW(
        copyNBytesSrcOffset(dest, src, offset, 3),
        std::out_of_range
    );

    EXPECT_EQ(offset, 4);
    EXPECT_EQ(dest, original);
}

TEST(HelperTest, CopyNBytesSrcOffsetExceedsDestination)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(2, std::byte{0});

    std::size_t offset = 1;

    EXPECT_THROW(
        copyNBytesSrcOffset(dest, src, offset, 3),
        std::out_of_range
    );

    EXPECT_EQ(offset, 1);
}

TEST(HelperTest, CopyNBytesSrcOffsetBeyondSource)
{
    std::vector<std::byte> src(5, std::byte{0xAB});
    std::vector<std::byte> dest(3, std::byte{0});

    std::size_t offset = 6;

    EXPECT_THROW(
        copyNBytesSrcOffset(dest, src, offset, 1),
        std::out_of_range
    );

    EXPECT_EQ(offset, 6);
}

TEST(HelperTest, CopyNBytesSrcOffsetRejectsSelfCopy)
{
    std::vector<std::byte> bytes(5, std::byte{0xAB});

    std::size_t offset = 0;

    EXPECT_THROW(
        copyNBytesSrcOffset(bytes, bytes, offset, 3),
        std::invalid_argument
    );

    EXPECT_EQ(offset, 0);
}

// ---------------------------------------------------------
// Cross-Function Integration Tests
// ---------------------------------------------------------

TEST(HelperTest, EncodeDecodeSequentialByteRegions)
{
    std::vector<std::byte> page(16, std::byte{0});

    std::vector<std::byte> first = {
        std::byte{0x11},
        std::byte{0x22}
    };

    std::vector<std::byte> second = {
        std::byte{0x33},
        std::byte{0x44},
        std::byte{0x55}
    };

    std::size_t writeOffset = 0;

    copyBytes(page, first, writeOffset);
    copyBytes(page, second, writeOffset);

    EXPECT_EQ(writeOffset, 5);

    std::vector<std::byte> decodedFirst(2, std::byte{0});
    std::vector<std::byte> decodedSecond(3, std::byte{0});

    std::size_t readOffset = 0;

    copyNBytesSrcOffset(decodedFirst, page, readOffset, 2);
    copyNBytesSrcOffset(decodedSecond, page, readOffset, 3);

    EXPECT_EQ(decodedFirst, first);
    EXPECT_EQ(decodedSecond, second);
    EXPECT_EQ(readOffset, writeOffset);
}

TEST(HelperTest, CopyBytesSrcOffsetCopiesRemainingPage)
{
    std::vector<std::byte> page(4096, std::byte{0});

    for (std::size_t i = 48; i < page.size(); ++i)
    {
        page[i] = static_cast<std::byte>(i % 256);
    }

    std::size_t offset = 48;

    std::vector<std::byte> remainingBytes(
        page.size() - offset,
        std::byte{0}
    );

    copyBytesSrcOffset(remainingBytes, page, offset);

    EXPECT_EQ(offset, 4096);
    EXPECT_EQ(remainingBytes.size(), 4048);

    for (std::size_t i = 0; i < remainingBytes.size(); ++i)
    {
        EXPECT_EQ(
            remainingBytes[i],
            page[48 + i]
        );
    }
}
