
#include <gtest/gtest.h>

#include "file_manager.hpp"

#include <filesystem>
#include <fstream>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <chrono>

// ---------------------------------------------------------
// Test Directory Helpers
// ---------------------------------------------------------

namespace fs = std::filesystem;

fs::path createTestDirectory()
{
    static std::uint64_t counter = 0;

    fs::path directory =
        fs::temp_directory_path() /
        ("db_file_manager_test_" +
         std::to_string(
             std::chrono::steady_clock::now()
                 .time_since_epoch().count()
         ) +
         "_" + std::to_string(counter++));

    fs::create_directories(directory);

    return directory;
}

class TestDirectory
{
private:
    fs::path directory;

public:
    TestDirectory()
        : directory(createTestDirectory())
    {
    }

    ~TestDirectory()
    {
        std::error_code ec;
        fs::remove_all(directory, ec);
    }

    const fs::path& getPath() const
    {
        return directory;
    }

    TestDirectory(const TestDirectory&) = delete;
    TestDirectory& operator=(const TestDirectory&) = delete;
};

// ---------------------------------------------------------
// Constructor Tests
// ---------------------------------------------------------

TEST(FileManagerTest, ConstructorCreatesDirectory)
{
    TestDirectory temp;

    fs::path directory = temp.getPath() / "database";

    EXPECT_FALSE(fs::exists(directory));

    FileManager manager(directory);

    EXPECT_TRUE(fs::exists(directory));
    EXPECT_TRUE(fs::is_directory(directory));
}

TEST(FileManagerTest, ConstructorCreatesNestedDirectories)
{
    TestDirectory temp;

    fs::path directory =
        temp.getPath() / "database" / "tables" / "heap";

    FileManager manager(directory);

    EXPECT_TRUE(fs::is_directory(directory));
}

TEST(FileManagerTest, ConstructorAcceptsExistingDirectory)
{
    TestDirectory temp;

    EXPECT_NO_THROW(FileManager manager(temp.getPath()));
}

// ---------------------------------------------------------
// File Creation Tests
// ---------------------------------------------------------

TEST(FileManagerTest, CreateFileValid)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    EXPECT_NO_THROW(manager.createFile("users.db"));

    EXPECT_TRUE(manager.fileExists("users.db"));
}

TEST(FileManagerTest, CreateFileProducesEmptyFile)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    EXPECT_EQ(
        fs::file_size(temp.getPath() / "users.db"),
        0
    );
}

TEST(FileManagerTest, CreateMultipleFiles)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");
    manager.createFile("orders.db");
    manager.createFile("products.db");

    EXPECT_TRUE(manager.fileExists("users.db"));
    EXPECT_TRUE(manager.fileExists("orders.db"));
    EXPECT_TRUE(manager.fileExists("products.db"));
}

TEST(FileManagerTest, CreateDuplicateFileThrows)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    EXPECT_THROW(
        manager.createFile("users.db"),
        std::invalid_argument
    );
}

TEST(FileManagerTest, CreateDuplicatePreservesExistingData)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    manager.writePage("users.db", 0, page);

    EXPECT_THROW(
        manager.createFile("users.db"),
        std::invalid_argument
    );

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage("users.db", 0, result);

    EXPECT_EQ(result, page);
}

// ---------------------------------------------------------
// File Existence Tests
// ---------------------------------------------------------

TEST(FileManagerTest, FileExistsReturnsTrue)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    EXPECT_TRUE(manager.fileExists("users.db"));
}

TEST(FileManagerTest, FileExistsReturnsFalse)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    EXPECT_FALSE(manager.fileExists("missing.db"));
}

TEST(FileManagerTest, FileExistsReturnsFalseForDirectory)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    fs::create_directory(temp.getPath() / "subdirectory");

    EXPECT_FALSE(manager.fileExists("subdirectory"));
}

// ---------------------------------------------------------
// Write Page Tests
// ---------------------------------------------------------

TEST(FileManagerTest, WriteFirstPage)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    EXPECT_NO_THROW(
        manager.writePage("users.db", 0, page)
    );

    EXPECT_EQ(
        fs::file_size(temp.getPath() / "users.db"),
        PAGE_BYTE_LENGTH
    );
}

TEST(FileManagerTest, WriteMultiplePages)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> first(
        PAGE_BYTE_LENGTH, std::byte{0x11}
    );

    std::vector<std::byte> second(
        PAGE_BYTE_LENGTH, std::byte{0x22}
    );

    std::vector<std::byte> third(
        PAGE_BYTE_LENGTH, std::byte{0x33}
    );

    manager.writePage("users.db", 0, first);
    manager.writePage("users.db", 1, second);
    manager.writePage("users.db", 2, third);

    EXPECT_EQ(
        fs::file_size(temp.getPath() / "users.db"),
        3 * PAGE_BYTE_LENGTH
    );
}

TEST(FileManagerTest, WritePageOverwritesExistingPage)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> original(
        PAGE_BYTE_LENGTH, std::byte{0xAA}
    );

    std::vector<std::byte> updated(
        PAGE_BYTE_LENGTH, std::byte{0xBB}
    );

    manager.writePage("users.db", 0, original);
    manager.writePage("users.db", 0, updated);

    EXPECT_EQ(
        fs::file_size(temp.getPath() / "users.db"),
        PAGE_BYTE_LENGTH
    );

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH, std::byte{0}
    );

    manager.readPage("users.db", 0, result);

    EXPECT_EQ(result, updated);
}

TEST(FileManagerTest, WriteMiddlePagePreservesOtherPages)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> first(
        PAGE_BYTE_LENGTH, std::byte{0x11}
    );

    std::vector<std::byte> second(
        PAGE_BYTE_LENGTH, std::byte{0x22}
    );

    std::vector<std::byte> third(
        PAGE_BYTE_LENGTH, std::byte{0x33}
    );

    manager.writePage("users.db", 0, first);
    manager.writePage("users.db", 1, second);
    manager.writePage("users.db", 2, third);

    std::vector<std::byte> updated(
        PAGE_BYTE_LENGTH, std::byte{0xFF}
    );

    manager.writePage("users.db", 1, updated);

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH, std::byte{0}
    );

    manager.readPage("users.db", 0, result);
    EXPECT_EQ(result, first);

    manager.readPage("users.db", 1, result);
    EXPECT_EQ(result, updated);

    manager.readPage("users.db", 2, result);
    EXPECT_EQ(result, third);

    EXPECT_EQ(
        fs::file_size(temp.getPath() / "users.db"),
        3 * PAGE_BYTE_LENGTH
    );
}

TEST(FileManagerTest, WritePageRejectsNegativePageNumber)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH, std::byte{0}
    );

    EXPECT_THROW(
        manager.writePage("users.db", -1, page),
        std::invalid_argument
    );
}

TEST(FileManagerTest, WritePageRejectsMissingFile)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH, std::byte{0}
    );

    EXPECT_THROW(
        manager.writePage("missing.db", 0, page),
        std::invalid_argument
    );
}

TEST(FileManagerTest, WritePageRejectsSmallBuffer)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH - 1,
        std::byte{0}
    );

    EXPECT_THROW(
        manager.writePage("users.db", 0, page),
        std::invalid_argument
    );
}

TEST(FileManagerTest, WritePageRejectsLargeBuffer)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH + 1,
        std::byte{0}
    );

    EXPECT_THROW(
        manager.writePage("users.db", 0, page),
        std::invalid_argument
    );
}

// ---------------------------------------------------------
// Read Page Tests
// ---------------------------------------------------------

TEST(FileManagerTest, ReadFirstPage)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> original(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    manager.writePage("users.db", 0, original);

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage("users.db", 0, result);

    EXPECT_EQ(result, original);
}

TEST(FileManagerTest, ReadMultiplePages)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    for (std::int64_t i = 0; i < 5; ++i)
    {
        std::vector<std::byte> page(
            PAGE_BYTE_LENGTH,
            static_cast<std::byte>(i + 1)
        );

        manager.writePage("users.db", i, page);
    }

    for (std::int64_t i = 0; i < 5; ++i)
    {
        std::vector<std::byte> result(
            PAGE_BYTE_LENGTH,
            std::byte{0}
        );

        manager.readPage("users.db", i, result);

        EXPECT_EQ(
            result,
            std::vector<std::byte>(
                PAGE_BYTE_LENGTH,
                static_cast<std::byte>(i + 1)
            )
        );
    }
}

TEST(FileManagerTest, ReadPagePreservesBytePattern)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> original(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    for (std::size_t i = 0; i < original.size(); ++i)
    {
        original[i] = static_cast<std::byte>(i % 256);
    }

    manager.writePage("users.db", 0, original);

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage("users.db", 0, result);

    EXPECT_EQ(result, original);
}

TEST(FileManagerTest, ReadPageRejectsNegativePageNumber)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    EXPECT_THROW(
        manager.readPage("users.db", -1, result),
        std::invalid_argument
    );
}

TEST(FileManagerTest, ReadPageRejectsMissingFile)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    EXPECT_THROW(
        manager.readPage("missing.db", 0, result),
        std::invalid_argument
    );
}

TEST(FileManagerTest, ReadPageRejectsNonexistentPage)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    manager.writePage("users.db", 0, page);

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    EXPECT_THROW(
        manager.readPage("users.db", 1, result),
        std::out_of_range
    );
}

TEST(FileManagerTest, ReadPageRejectsEmptyFile)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    EXPECT_THROW(
        manager.readPage("users.db", 0, result),
        std::out_of_range
    );
}

TEST(FileManagerTest, ReadPageRejectsSmallDestination)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    manager.writePage("users.db", 0, page);

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH - 1,
        std::byte{0}
    );

    EXPECT_THROW(
        manager.readPage("users.db", 0, result),
        std::invalid_argument
    );
}

TEST(FileManagerTest, ReadPageRejectsLargeDestination)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    manager.writePage("users.db", 0, page);

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH + 1,
        std::byte{0}
    );

    EXPECT_THROW(
        manager.readPage("users.db", 0, result),
        std::invalid_argument
    );
}

TEST(FileManagerTest, FailedReadPreservesDestination)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    std::vector<std::byte> original = result;

    EXPECT_THROW(
        manager.readPage("users.db", 0, result),
        std::out_of_range
    );

    EXPECT_EQ(result, original);
}

// ---------------------------------------------------------
// Page Count Tests
// ---------------------------------------------------------

TEST(FileManagerTest, GetPageCountEmptyFile)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::string fileName = "users.db";

    EXPECT_EQ(manager.getPageCount(fileName), 0);
}

TEST(FileManagerTest, GetPageCountOnePage)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.writePage("users.db", 0, page);

    std::string fileName = "users.db";

    EXPECT_EQ(manager.getPageCount(fileName), 1);
}

TEST(FileManagerTest, GetPageCountMultiplePages)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    std::vector<std::byte> page(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    for (std::int64_t i = 0; i < 10; ++i)
    {
        manager.writePage("users.db", i, page);
    }

    std::string fileName = "users.db";

    EXPECT_EQ(manager.getPageCount(fileName), 10);
}

TEST(FileManagerTest, GetPageCountRejectsPartialPage)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");

    fs::path filePath = temp.getPath() / "users.db";

    std::ofstream output(
        filePath,
        std::ios::binary | std::ios::app
    );

    char byte = 0;
    output.write(&byte, 1);
    output.close();

    std::string fileName = "users.db";

    EXPECT_THROW(
        manager.getPageCount(fileName),
        std::runtime_error
    );
}

TEST(FileManagerTest, GetPageCountMissingFile)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    std::string fileName = "missing.db";

    EXPECT_THROW(
        manager.getPageCount(fileName),
        fs::filesystem_error
    );
}

// ---------------------------------------------------------
// Persistence Tests
// ---------------------------------------------------------

TEST(FileManagerTest, DataPersistsAcrossManagerInstances)
{
    TestDirectory temp;

    std::vector<std::byte> original(
        PAGE_BYTE_LENGTH,
        std::byte{0xAB}
    );

    {
        FileManager manager(temp.getPath());

        manager.createFile("users.db");
        manager.writePage("users.db", 0, original);
    }

    {
        FileManager manager(temp.getPath());

        std::vector<std::byte> result(
            PAGE_BYTE_LENGTH,
            std::byte{0}
        );

        manager.readPage("users.db", 0, result);

        EXPECT_EQ(result, original);
    }
}

TEST(FileManagerTest, MultipleFilesMaintainIndependentPages)
{
    TestDirectory temp;
    FileManager manager(temp.getPath());

    manager.createFile("users.db");
    manager.createFile("orders.db");

    std::vector<std::byte> usersPage(
        PAGE_BYTE_LENGTH,
        std::byte{0xAA}
    );

    std::vector<std::byte> ordersPage(
        PAGE_BYTE_LENGTH,
        std::byte{0xBB}
    );

    manager.writePage("users.db", 0, usersPage);
    manager.writePage("orders.db", 0, ordersPage);

    std::vector<std::byte> result(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage("users.db", 0, result);
    EXPECT_EQ(result, usersPage);

    manager.readPage("orders.db", 0, result);
    EXPECT_EQ(result, ordersPage);
}
