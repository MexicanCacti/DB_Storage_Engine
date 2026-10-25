
#include <gtest/gtest.h>

#include "buffer_pool.hpp"
#include "file_manager.hpp"

#include <filesystem>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <chrono>

namespace fs = std::filesystem;

// ---------------------------------------------------------
// Test Helpers
// ---------------------------------------------------------

class BufferPoolTestDirectory
{
private:
    fs::path directory;

public:
    BufferPoolTestDirectory()
    {
        static std::uint64_t counter = 0;

        directory = fs::temp_directory_path() /
            ("db_buffer_pool_test_" +
             std::to_string(
                 std::chrono::steady_clock::now()
                     .time_since_epoch().count()
             ) +
             "_" + std::to_string(counter++));

        fs::create_directories(directory);
    }

    ~BufferPoolTestDirectory()
    {
        std::error_code ec;
        fs::remove_all(directory, ec);
    }

    const fs::path& getPath() const
    {
        return directory;
    }

    BufferPoolTestDirectory(const BufferPoolTestDirectory&) = delete;
    BufferPoolTestDirectory& operator=(const BufferPoolTestDirectory&) = delete;
};

void createBufferPoolTestPages(
    FileManager& manager,
    const std::string& fileName,
    std::size_t pageCount)
{
    manager.createFile(fileName);

    for (std::size_t i = 0; i < pageCount; ++i)
    {
        std::vector<std::byte> bytes(
            PAGE_BYTE_LENGTH,
            static_cast<std::byte>(i + 1)
        );

        manager.writePage(
            fileName,
            static_cast<std::int64_t>(i),
            bytes
        );
    }
}

// ---------------------------------------------------------
// Constructor Tests
// ---------------------------------------------------------

TEST(BufferPoolTest, ConstructorValidCapacity)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    std::unordered_map<std::int64_t, std::string> tableFiles;

    EXPECT_NO_THROW(BufferPool(manager, tableFiles, 3));
}

TEST(BufferPoolTest, ConstructorRejectsZeroCapacity)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    std::unordered_map<std::int64_t, std::string> tableFiles;

    EXPECT_THROW(
        BufferPool(manager, tableFiles, 0),
        std::invalid_argument
    );
}

TEST(BufferPoolTest, ConstructorRejectsNegativeCapacity)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    std::unordered_map<std::int64_t, std::string> tableFiles;

    EXPECT_THROW(
        BufferPool(manager, tableFiles, -1),
        std::invalid_argument
    );
}

TEST(BufferPoolTest, ConstructorInitializesCapacity)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    std::unordered_map<std::int64_t, std::string> tableFiles;

    BufferPool pool(manager, tableFiles, 3);

    EXPECT_EQ(pool.getPoolCapacity(), 3);
    EXPECT_EQ(pool.getOccupiedFramesCount(), 0);
}

// ---------------------------------------------------------
// Page Fetch Tests
// ---------------------------------------------------------

TEST(BufferPoolTest, FetchExistingPage)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 3);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    Page& page = pool.fetchPage(PageID{1, 0});

    EXPECT_EQ(page.getTableID(), 1);
    EXPECT_EQ(page.getPageNumber(), 0);

    EXPECT_TRUE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_TRUE(pool.isPagePinned(PageID{1, 0}));
    EXPECT_EQ(pool.getOccupiedFramesCount(), 1);
}

TEST(BufferPoolTest, FetchPageLoadsCorrectBytes)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 3);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    Page& page = pool.fetchPage(PageID{1, 1});

    std::vector<std::byte> expected(
        PAGE_BYTE_LENGTH,
        std::byte{0x02}
    );

    EXPECT_EQ(page.getPageBytesConst(), expected);
}

TEST(BufferPoolTest, FetchSamePageUsesExistingFrame)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 3);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    Page& first = pool.fetchPage(PageID{1, 0});
    Page& second = pool.fetchPage(PageID{1, 0});

    EXPECT_EQ(&first, &second);
    EXPECT_EQ(pool.getOccupiedFramesCount(), 1);

    pool.unpinPage(PageID{1, 0}, false);
    EXPECT_TRUE(pool.isPagePinned(PageID{1, 0}));

    pool.unpinPage(PageID{1, 0}, false);
    EXPECT_FALSE(pool.isPagePinned(PageID{1, 0}));
}

TEST(BufferPoolTest, FetchMultiplePages)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 3);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});
    pool.fetchPage(PageID{1, 1});
    pool.fetchPage(PageID{1, 2});

    EXPECT_EQ(pool.getOccupiedFramesCount(), 3);

    EXPECT_TRUE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 1}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 2}));
}

TEST(BufferPoolTest, FetchRejectsMissingFile)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "missing.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    EXPECT_THROW(
        pool.fetchPage(PageID{1, 0}),
        std::invalid_argument
    );

    EXPECT_EQ(pool.getOccupiedFramesCount(), 0);
}

TEST(BufferPoolTest, FetchRejectsUnknownTableID)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    std::unordered_map<std::int64_t, std::string> tableFiles;

    BufferPool pool(manager, tableFiles, 3);

    EXPECT_THROW(
        pool.fetchPage(PageID{99, 0}),
        std::out_of_range
    );
}

TEST(BufferPoolTest, FetchRejectsNonexistentPage)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 1);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    EXPECT_THROW(
        pool.fetchPage(PageID{1, 5}),
        std::out_of_range
    );

    EXPECT_EQ(pool.getOccupiedFramesCount(), 0);
}

// ---------------------------------------------------------
// Pin Count Tests
// ---------------------------------------------------------

TEST(BufferPoolTest, FetchAutomaticallyPinsPage)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 1);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});

    EXPECT_TRUE(pool.isPagePinned(PageID{1, 0}));
}

TEST(BufferPoolTest, UnpinPage)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 1);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});
    pool.unpinPage(PageID{1, 0}, false);

    EXPECT_FALSE(pool.isPagePinned(PageID{1, 0}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 0}));
}

TEST(BufferPoolTest, UnpinPageNotInPoolThrows)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    std::unordered_map<std::int64_t, std::string> tableFiles;

    BufferPool pool(manager, tableFiles, 3);

    EXPECT_THROW(
        pool.unpinPage(PageID{1, 0}, false),
        std::runtime_error
    );
}

TEST(BufferPoolTest, UnpinPageWithZeroPinCountThrows)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 1);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});
    pool.unpinPage(PageID{1, 0}, false);

    EXPECT_THROW(
        pool.unpinPage(PageID{1, 0}, false),
        std::runtime_error
    );
}

TEST(BufferPoolTest, UnpinMultipleReferences)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 1);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});
    pool.fetchPage(PageID{1, 0});
    pool.fetchPage(PageID{1, 0});

    pool.unpinPage(PageID{1, 0}, false);
    EXPECT_TRUE(pool.isPagePinned(PageID{1, 0}));

    pool.unpinPage(PageID{1, 0}, false);
    EXPECT_TRUE(pool.isPagePinned(PageID{1, 0}));

    pool.unpinPage(PageID{1, 0}, false);
    EXPECT_FALSE(pool.isPagePinned(PageID{1, 0}));
}

// ---------------------------------------------------------
// Buffer Capacity Tests
// ---------------------------------------------------------

TEST(BufferPoolTest, OccupiedCountNeverExceedsCapacity)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 10);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    for (std::int64_t i = 0; i < 10; ++i)
    {
        pool.fetchPage(PageID{1, i});

        EXPECT_LE(
            pool.getOccupiedFramesCount(),
            pool.getPoolCapacity()
        );

        pool.unpinPage(PageID{1, i}, false);
    }
}

TEST(BufferPoolTest, AllFramesPinnedPreventsEviction)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 4);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});
    pool.fetchPage(PageID{1, 1});
    pool.fetchPage(PageID{1, 2});

    EXPECT_THROW(
        pool.fetchPage(PageID{1, 3}),
        std::runtime_error
    );

    EXPECT_EQ(pool.getOccupiedFramesCount(), 3);

    EXPECT_TRUE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 1}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 2}));
    EXPECT_FALSE(pool.isPageInPool(PageID{1, 3}));
}

TEST(BufferPoolTest, UnpinAllowsEviction)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 4);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});
    pool.fetchPage(PageID{1, 1});
    pool.fetchPage(PageID{1, 2});

    pool.unpinPage(PageID{1, 0}, false);

    EXPECT_NO_THROW(pool.fetchPage(PageID{1, 3}));

    EXPECT_FALSE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 3}));
}

// ---------------------------------------------------------
// LRU Eviction Tests
// ---------------------------------------------------------

TEST(BufferPoolTest, EvictsLeastRecentlyUsedUnpinnedPage)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 4);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});
    pool.unpinPage(PageID{1, 0}, false);

    pool.fetchPage(PageID{1, 1});
    pool.unpinPage(PageID{1, 1}, false);

    pool.fetchPage(PageID{1, 2});
    pool.unpinPage(PageID{1, 2}, false);

    pool.fetchPage(PageID{1, 3});

    EXPECT_FALSE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 1}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 2}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 3}));
}

TEST(BufferPoolTest, FetchUpdatesLRUOrder)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 4);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});
    pool.unpinPage(PageID{1, 0}, false);

    pool.fetchPage(PageID{1, 1});
    pool.unpinPage(PageID{1, 1}, false);

    pool.fetchPage(PageID{1, 2});
    pool.unpinPage(PageID{1, 2}, false);

    pool.fetchPage(PageID{1, 0});
    pool.unpinPage(PageID{1, 0}, false);

    pool.fetchPage(PageID{1, 3});

    EXPECT_TRUE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_FALSE(pool.isPageInPool(PageID{1, 1}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 2}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 3}));
}

TEST(BufferPoolTest, LRUOnlyEvictsUnpinnedFrames)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 4);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    pool.fetchPage(PageID{1, 0});

    pool.fetchPage(PageID{1, 1});
    pool.unpinPage(PageID{1, 1}, false);

    pool.fetchPage(PageID{1, 2});
    pool.unpinPage(PageID{1, 2}, false);

    pool.fetchPage(PageID{1, 3});

    EXPECT_TRUE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_FALSE(pool.isPageInPool(PageID{1, 1}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 2}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 3}));
}

// ---------------------------------------------------------
// Dirty Page Tests
// ---------------------------------------------------------

TEST(BufferPoolTest, FlushDirtyPagePersistsChanges)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 1);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    Page& page = pool.fetchPage(PageID{1, 0});

    page.getPageBytes()[0] = std::byte{0xFF};

    pool.unpinPage(PageID{1, 0}, true);
    pool.flushPage(PageID{1, 0});

    std::vector<std::byte> diskBytes(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage("users.db", 0, diskBytes);

    EXPECT_EQ(diskBytes[0], std::byte{0xFF});
}

TEST(BufferPoolTest, FlushAllPagesPersistsDirtyPages)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 3);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    for (std::int64_t i = 0; i < 3; ++i)
    {
        Page& page = pool.fetchPage(PageID{1, i});

        page.getPageBytes()[0] =
            static_cast<std::byte>(0xA0 + i);

        pool.unpinPage(PageID{1, i}, true);
    }

    pool.flushAllPages();

    for (std::int64_t i = 0; i < 3; ++i)
    {
        std::vector<std::byte> result(
            PAGE_BYTE_LENGTH,
            std::byte{0}
        );

        manager.readPage("users.db", i, result);

        EXPECT_EQ(
            result[0],
            static_cast<std::byte>(0xA0 + i)
        );
    }
}

TEST(BufferPoolTest, DirtyPageIsFlushedBeforeEviction)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 2);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 1);

    Page& page = pool.fetchPage(PageID{1, 0});

    page.getPageBytes()[0] = std::byte{0xFF};

    pool.unpinPage(PageID{1, 0}, true);

    pool.fetchPage(PageID{1, 1});

    EXPECT_FALSE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 1}));

    std::vector<std::byte> diskBytes(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage("users.db", 0, diskBytes);

    EXPECT_EQ(diskBytes[0], std::byte{0xFF});
}

TEST(BufferPoolTest, CleanPageDoesNotOverwriteDisk)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 1);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    Page& page = pool.fetchPage(PageID{1, 0});

    page.getPageBytes()[0] = std::byte{0xFF};

    pool.unpinPage(PageID{1, 0}, false);
    pool.flushPage(PageID{1, 0});

    std::vector<std::byte> diskBytes(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage("users.db", 0, diskBytes);

    EXPECT_EQ(diskBytes[0], std::byte{0x01});
}

// ---------------------------------------------------------
// Page Identity Tests
// ---------------------------------------------------------

TEST(BufferPoolTest, DifferentTablesSamePageNumber)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 1);
    createBufferPoolTestPages(manager, "orders.db", 1);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"},
        {2, "orders.db"}
    };

    BufferPool pool(manager, tableFiles, 3);

    Page& usersPage = pool.fetchPage(PageID{1, 0});
    Page& ordersPage = pool.fetchPage(PageID{2, 0});

    EXPECT_EQ(pool.getOccupiedFramesCount(), 2);

    EXPECT_TRUE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_TRUE(pool.isPageInPool(PageID{2, 0}));

    EXPECT_NE(&usersPage, &ordersPage);
}

TEST(BufferPoolTest, PageIDHashDistinguishesOrder)
{
    PageID first{1, 2};
    PageID second{2, 1};

    std::unordered_map<
        PageID,
        std::size_t,
        PageID::PageIDHasher
    > lookup;

    lookup[first] = 10;
    lookup[second] = 20;

    EXPECT_EQ(lookup.size(), 2);
    EXPECT_EQ(lookup.at(first), 10);
    EXPECT_EQ(lookup.at(second), 20);
}

// ---------------------------------------------------------
// Persistence Tests
// ---------------------------------------------------------

TEST(BufferPoolTest, EvictedPageCanBeFetchedAgain)
{
    BufferPoolTestDirectory temp;
    FileManager manager(temp.getPath());

    createBufferPoolTestPages(manager, "users.db", 2);

    std::unordered_map<std::int64_t, std::string> tableFiles{
        {1, "users.db"}
    };

    BufferPool pool(manager, tableFiles, 1);

    Page& first = pool.fetchPage(PageID{1, 0});

    EXPECT_EQ(first.getPageBytesConst()[0], std::byte{0x01});

    pool.unpinPage(PageID{1, 0}, false);

    pool.fetchPage(PageID{1, 1});
    pool.unpinPage(PageID{1, 1}, false);

    Page& reloaded = pool.fetchPage(PageID{1, 0});

    EXPECT_EQ(reloaded.getPageBytesConst()[0], std::byte{0x01});
    EXPECT_TRUE(pool.isPageInPool(PageID{1, 0}));
    EXPECT_FALSE(pool.isPageInPool(PageID{1, 1}));
}
