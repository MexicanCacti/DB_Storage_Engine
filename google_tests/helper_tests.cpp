
#include <gtest/gtest.h>
#include "helpers.hpp"

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <limits>

// ==========================================
// appendBytes Tests
// ==========================================

TEST(AppendBytesTests, AppendToNonEmptyVector)
{
    std::vector<std::byte> dest{
        std::byte{0x01},
        std::byte{0x02}
    };

    std::vector<std::byte> src{
        std::byte{0x03},
        std::byte{0x04}
    };

    appendBytes(dest, src);

    std::vector<std::byte> expected{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}
    };

    EXPECT_EQ(dest, expected);
}

TEST(AppendBytesTests, AppendToEmptyVector)
{
    std::vector<std::byte> dest;

    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02}
    };

    appendBytes(dest, src);

    EXPECT_EQ(dest, src);
}

TEST(AppendBytesTests, AppendEmptySource)
{
    std::vector<std::byte> dest{
        std::byte{0x01},
        std::byte{0x02}
    };

    std::vector<std::byte> src;
    std::vector<std::byte> expected = dest;

    appendBytes(dest, src);

    EXPECT_EQ(dest, expected);
}

TEST(AppendBytesTests, AppendTwoEmptyVectors)
{
    std::vector<std::byte> dest;
    std::vector<std::byte> src;

    appendBytes(dest, src);

    EXPECT_TRUE(dest.empty());
}

TEST(AppendBytesTests, AppendSameVector)
{
    std::vector<std::byte> bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    appendBytes(bytes, bytes);

    std::vector<std::byte> expected{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    EXPECT_EQ(bytes, expected);
}

TEST(AppendBytesTests, AppendSameEmptyVector)
{
    std::vector<std::byte> bytes;

    appendBytes(bytes, bytes);

    EXPECT_TRUE(bytes.empty());
}

TEST(AppendBytesTests, SourceRemainsUnchanged)
{
    std::vector<std::byte> dest{
        std::byte{0x01}
    };

    std::vector<std::byte> src{
        std::byte{0x02},
        std::byte{0x03}
    };

    std::vector<std::byte> originalSrc = src;

    appendBytes(dest, src);

    EXPECT_EQ(src, originalSrc);
}

// ==========================================
// copyBytes Tests
// ==========================================

TEST(CopyBytesTests, CopyAtBeginning)
{
    std::vector<std::byte> dest(5, std::byte{0x00});

    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02}
    };

    std::size_t offset = 0;

    copyBytes(dest, src, offset);

    std::vector<std::byte> expected{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x00}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 2);
}

TEST(CopyBytesTests, CopyAtMiddle)
{
    std::vector<std::byte> dest(5, std::byte{0x00});

    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02}
    };

    std::size_t offset = 2;

    copyBytes(dest, src, offset);

    std::vector<std::byte> expected{
        std::byte{0x00},
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x00}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 4);
}

TEST(CopyBytesTests, CopyExactlyToEnd)
{
    std::vector<std::byte> dest(4, std::byte{0x00});

    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02}
    };

    std::size_t offset = 2;

    copyBytes(dest, src, offset);

    EXPECT_EQ(dest[2], std::byte{0x01});
    EXPECT_EQ(dest[3], std::byte{0x02});
    EXPECT_EQ(offset, dest.size());
}

TEST(CopyBytesTests, CopyEntireVector)
{
    std::vector<std::byte> dest(3, std::byte{0x00});

    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    std::size_t offset = 0;

    copyBytes(dest, src, offset);

    EXPECT_EQ(dest, src);
    EXPECT_EQ(offset, 3);
}

TEST(CopyBytesTests, RejectSourceLargerThanDestination)
{
    std::vector<std::byte> dest(2, std::byte{0x00});
    std::vector<std::byte> src(3, std::byte{0x01});

    std::size_t offset = 0;
    auto originalDest = dest;

    EXPECT_THROW(copyBytes(dest, src, offset), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 0);
}

TEST(CopyBytesTests, RejectOffsetBeyondDestination)
{
    std::vector<std::byte> dest(3, std::byte{0x00});
    std::vector<std::byte> src{std::byte{0x01}};

    std::size_t offset = 4;
    auto originalDest = dest;

    EXPECT_THROW(copyBytes(dest, src, offset), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 4);
}

TEST(CopyBytesTests, RejectCopyExceedingRemainingSpace)
{
    std::vector<std::byte> dest(5, std::byte{0x00});
    std::vector<std::byte> src(3, std::byte{0x01});

    std::size_t offset = 3;
    auto originalDest = dest;

    EXPECT_THROW(copyBytes(dest, src, offset), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 3);
}

TEST(CopyBytesTests, RejectSameVector)
{
    std::vector<std::byte> bytes(5, std::byte{0x01});

    std::size_t offset = 0;
    auto originalBytes = bytes;

    EXPECT_THROW(copyBytes(bytes, bytes, offset), std::invalid_argument);

    EXPECT_EQ(bytes, originalBytes);
    EXPECT_EQ(offset, 0);
}

TEST(CopyBytesTests, CopyEmptySource)
{
    std::vector<std::byte> dest(3, std::byte{0x01});
    std::vector<std::byte> src;

    std::size_t offset = 2;
    auto originalDest = dest;

    copyBytes(dest, src, offset);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 2);
}

TEST(CopyBytesTests, EmptySourceAtEnd)
{
    std::vector<std::byte> dest(3, std::byte{0x01});
    std::vector<std::byte> src;

    std::size_t offset = 3;

    EXPECT_NO_THROW(copyBytes(dest, src, offset));
    EXPECT_EQ(offset, 3);
}

// ==========================================
// copyNBytes Tests
// ==========================================

TEST(CopyNBytesTests, CopyPartialSource)
{
    std::vector<std::byte> dest(5, std::byte{0x00});

    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}
    };

    std::size_t offset = 1;

    copyNBytes(dest, src, offset, 2);

    std::vector<std::byte> expected{
        std::byte{0x00},
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x00},
        std::byte{0x00}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 3);
}

TEST(CopyNBytesTests, CopyEntireSource)
{
    std::vector<std::byte> dest(4, std::byte{0x00});

    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02}
    };

    std::size_t offset = 1;

    copyNBytes(dest, src, offset, src.size());

    EXPECT_EQ(dest[1], std::byte{0x01});
    EXPECT_EQ(dest[2], std::byte{0x02});
    EXPECT_EQ(offset, 3);
}

TEST(CopyNBytesTests, CopyZeroBytes)
{
    std::vector<std::byte> dest(4, std::byte{0x01});
    std::vector<std::byte> src(3, std::byte{0x02});

    std::size_t offset = 2;
    auto originalDest = dest;

    copyNBytes(dest, src, offset, 0);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 2);
}

TEST(CopyNBytesTests, RejectNExceedingSourceSize)
{
    std::vector<std::byte> dest(10, std::byte{0x00});
    std::vector<std::byte> src(3, std::byte{0x01});

    std::size_t offset = 0;
    auto originalDest = dest;

    EXPECT_THROW(copyNBytes(dest, src, offset, 4), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 0);
}

TEST(CopyNBytesTests, RejectNExceedingDestinationSpace)
{
    std::vector<std::byte> dest(4, std::byte{0x00});
    std::vector<std::byte> src(5, std::byte{0x01});

    std::size_t offset = 2;
    auto originalDest = dest;

    EXPECT_THROW(copyNBytes(dest, src, offset, 3), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 2);
}

TEST(CopyNBytesTests, RejectOffsetBeyondDestination)
{
    std::vector<std::byte> dest(4, std::byte{0x00});
    std::vector<std::byte> src(3, std::byte{0x01});

    std::size_t offset = 5;
    auto originalDest = dest;

    EXPECT_THROW(copyNBytes(dest, src, offset, 2), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 5);
}

TEST(CopyNBytesTests, RejectSameVector)
{
    std::vector<std::byte> bytes(5, std::byte{0x01});

    std::size_t offset = 0;
    auto originalBytes = bytes;

    EXPECT_THROW(copyNBytes(bytes, bytes, offset, 2), std::invalid_argument);

    EXPECT_EQ(bytes, originalBytes);
    EXPECT_EQ(offset, 0);
}

TEST(CopyNBytesTests, CopyExactlyToDestinationEnd)
{
    std::vector<std::byte> dest(4, std::byte{0x00});
    std::vector<std::byte> src(5, std::byte{0x01});

    std::size_t offset = 2;

    EXPECT_NO_THROW(copyNBytes(dest, src, offset, 2));

    EXPECT_EQ(offset, 4);
    EXPECT_EQ(dest[2], std::byte{0x01});
    EXPECT_EQ(dest[3], std::byte{0x01});
}

// ==========================================
// copyBytesSrcOffset Tests
// ==========================================

TEST(CopyBytesSrcOffsetTests, CopyFromBeginning)
{
    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    std::vector<std::byte> dest(3, std::byte{0x00});

    std::size_t offset = 0;

    copyBytesSrcOffset(dest, src, offset);

    EXPECT_EQ(dest, src);
    EXPECT_EQ(offset, 3);
}

TEST(CopyBytesSrcOffsetTests, CopyFromMiddle)
{
    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}
    };

    std::vector<std::byte> dest(2, std::byte{0x00});

    std::size_t offset = 2;

    copyBytesSrcOffset(dest, src, offset);

    std::vector<std::byte> expected{
        std::byte{0x03},
        std::byte{0x04}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 4);
}

TEST(CopyBytesSrcOffsetTests, DestinationLargerThanRemainingSource)
{
    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    std::vector<std::byte> dest(4, std::byte{0xFF});

    std::size_t offset = 1;

    copyBytesSrcOffset(dest, src, offset);

    std::vector<std::byte> expected{
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0xFF},
        std::byte{0xFF}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 3);
}

TEST(CopyBytesSrcOffsetTests, RejectDestinationTooSmall)
{
    std::vector<std::byte> src(5, std::byte{0x01});
    std::vector<std::byte> dest(2, std::byte{0x00});

    std::size_t offset = 1;
    auto originalDest = dest;

    EXPECT_THROW(copyBytesSrcOffset(dest, src, offset), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 1);
}

TEST(CopyBytesSrcOffsetTests, RejectOffsetBeyondSource)
{
    std::vector<std::byte> src(3, std::byte{0x01});
    std::vector<std::byte> dest(5, std::byte{0x00});

    std::size_t offset = 4;
    auto originalDest = dest;

    EXPECT_THROW(copyBytesSrcOffset(dest, src, offset), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 4);
}

TEST(CopyBytesSrcOffsetTests, RejectSameVector)
{
    std::vector<std::byte> bytes(5, std::byte{0x01});

    std::size_t offset = 0;
    auto originalBytes = bytes;

    EXPECT_THROW(copyBytesSrcOffset(bytes, bytes, offset), std::invalid_argument);

    EXPECT_EQ(bytes, originalBytes);
    EXPECT_EQ(offset, 0);
}

TEST(CopyBytesSrcOffsetTests, OffsetAtSourceEnd)
{
    std::vector<std::byte> src(3, std::byte{0x01});
    std::vector<std::byte> dest(3, std::byte{0x00});

    std::size_t offset = 3;
    auto originalDest = dest;

    copyBytesSrcOffset(dest, src, offset);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 3);
}

TEST(CopyBytesSrcOffsetTests, EmptySource)
{
    std::vector<std::byte> src;
    std::vector<std::byte> dest(3, std::byte{0x00});

    std::size_t offset = 0;

    EXPECT_NO_THROW(copyBytesSrcOffset(dest, src, offset));

    EXPECT_EQ(offset, 0);
}

// ==========================================
// copyNBytesSrcOffset Tests
// ==========================================

TEST(CopyNBytesSrcOffsetTests, CopyFromBeginning)
{
    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03}
    };

    std::vector<std::byte> dest(2, std::byte{0x00});

    std::size_t offset = 0;

    copyNBytesSrcOffset(dest, src, offset, 2);

    std::vector<std::byte> expected{
        std::byte{0x01},
        std::byte{0x02}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 2);
}

TEST(CopyNBytesSrcOffsetTests, CopyFromMiddle)
{
    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05}
    };

    std::vector<std::byte> dest(3, std::byte{0x00});

    std::size_t offset = 2;

    copyNBytesSrcOffset(dest, src, offset, 3);

    std::vector<std::byte> expected{
        std::byte{0x03},
        std::byte{0x04},
        std::byte{0x05}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 5);
}

TEST(CopyNBytesSrcOffsetTests, CopyPartialSource)
{
    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}
    };

    std::vector<std::byte> dest(4, std::byte{0xFF});

    std::size_t offset = 1;

    copyNBytesSrcOffset(dest, src, offset, 2);

    std::vector<std::byte> expected{
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0xFF},
        std::byte{0xFF}
    };

    EXPECT_EQ(dest, expected);
    EXPECT_EQ(offset, 3);
}

TEST(CopyNBytesSrcOffsetTests, CopyZeroBytes)
{
    std::vector<std::byte> src(4, std::byte{0x01});
    std::vector<std::byte> dest(3, std::byte{0x00});

    std::size_t offset = 2;
    auto originalDest = dest;

    copyNBytesSrcOffset(dest, src, offset, 0);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 2);
}

TEST(CopyNBytesSrcOffsetTests, RejectNExceedingRemainingSource)
{
    std::vector<std::byte> src(5, std::byte{0x01});
    std::vector<std::byte> dest(5, std::byte{0x00});

    std::size_t offset = 3;
    auto originalDest = dest;

    EXPECT_THROW(copyNBytesSrcOffset(dest, src, offset, 3), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 3);
}

TEST(CopyNBytesSrcOffsetTests, RejectNExceedingDestination)
{
    std::vector<std::byte> src(5, std::byte{0x01});
    std::vector<std::byte> dest(2, std::byte{0x00});

    std::size_t offset = 0;
    auto originalDest = dest;

    EXPECT_THROW(copyNBytesSrcOffset(dest, src, offset, 3), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 0);
}

TEST(CopyNBytesSrcOffsetTests, RejectOffsetBeyondSource)
{
    std::vector<std::byte> src(3, std::byte{0x01});
    std::vector<std::byte> dest(5, std::byte{0x00});

    std::size_t offset = 4;
    auto originalDest = dest;

    EXPECT_THROW(copyNBytesSrcOffset(dest, src, offset, 1), std::out_of_range);

    EXPECT_EQ(dest, originalDest);
    EXPECT_EQ(offset, 4);
}

TEST(CopyNBytesSrcOffsetTests, RejectSameVector)
{
    std::vector<std::byte> bytes(5, std::byte{0x01});

    std::size_t offset = 0;
    auto originalBytes = bytes;

    EXPECT_THROW(copyNBytesSrcOffset(bytes, bytes, offset, 2), std::invalid_argument);

    EXPECT_EQ(bytes, originalBytes);
    EXPECT_EQ(offset, 0);
}

TEST(CopyNBytesSrcOffsetTests, CopyExactlyToSourceEnd)
{
    std::vector<std::byte> src{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
        std::byte{0x04}
    };

    std::vector<std::byte> dest(2, std::byte{0x00});

    std::size_t offset = 2;

    copyNBytesSrcOffset(dest, src, offset, 2);

    EXPECT_EQ(dest[0], std::byte{0x03});
    EXPECT_EQ(dest[1], std::byte{0x04});
    EXPECT_EQ(offset, src.size());
}