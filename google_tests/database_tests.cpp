
#include <gtest/gtest.h>

#include "database.hpp"
#include "file_manager.hpp"
#include "data_page.hpp"
#include "table_header.hpp"

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <chrono>

namespace fs = std::filesystem;

// ---------------------------------------------------------
// Test Helpers
// ---------------------------------------------------------

class DatabaseTestDirectory
{
private:
    fs::path directory;

public:
    DatabaseTestDirectory()
    {
        static std::uint64_t counter = 0;

        directory = fs::temp_directory_path() /
            ("db_database_test_" +
             std::to_string(
                 std::chrono::steady_clock::now()
                     .time_since_epoch().count()
             ) +
             "_" + std::to_string(counter++));

        fs::create_directories(directory);
    }

    ~DatabaseTestDirectory()
    {
        std::error_code ec;
        fs::remove_all(directory, ec);
    }

    const fs::path& getPath() const
    {
        return directory;
    }

    DatabaseTestDirectory(const DatabaseTestDirectory&) = delete;
    DatabaseTestDirectory& operator=(const DatabaseTestDirectory&) = delete;
};

Schema createDatabaseTestSchema()
{
    return Schema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64}
    });
}

Tuple createDatabaseTestTuple(
    std::int64_t id,
    const std::string& name,
    std::int64_t age
)
{
    return Tuple({
        FieldVariant{id},
        FieldVariant{name},
        FieldVariant{age}
    });
}

void expectTuplesEqual(
    const Tuple& actual,
    const Tuple& expected
)
{
    EXPECT_EQ(actual.encode(), expected.encode());
}

std::size_t countFilesWithExtension(
    const fs::path& directory,
    const std::string& extension
)
{
    std::size_t count = 0;

    for (const auto& entry : fs::directory_iterator(directory))
    {
        if (entry.is_regular_file() &&
            entry.path().extension() == extension)
        {
            ++count;
        }
    }

    return count;
}

// ---------------------------------------------------------
// Constructor Tests
// ---------------------------------------------------------

TEST(DatabaseTest, ConstructorCreatesDatabaseDirectory)
{
    DatabaseTestDirectory temp;

    fs::path directory = temp.getPath() / "database";

    EXPECT_FALSE(fs::exists(directory));

    Database database(directory, 3);

    EXPECT_TRUE(fs::is_directory(directory));
}

TEST(DatabaseTest, ConstructorCreatesCatalogFile)
{
    DatabaseTestDirectory temp;

    Database database(temp.getPath(), 3);

    EXPECT_EQ(
        countFilesWithExtension(
            temp.getPath(),
            CATALOG_FILE_EXTENSION
        ),
        1
    );
}

TEST(DatabaseTest, ConstructorRejectsZeroBufferFrames)
{
    DatabaseTestDirectory temp;

    EXPECT_THROW(
        Database(temp.getPath(), 0),
        std::invalid_argument
    );
}

TEST(DatabaseTest, ConstructorLoadsExistingEmptyCatalog)
{
    DatabaseTestDirectory temp;

    {
        Database database(temp.getPath(), 3);
    }

    EXPECT_NO_THROW(
        Database database(temp.getPath(), 3)
    );
}

TEST(DatabaseTest, ConstructorRejectsInvalidCatalogNextID)
{
    DatabaseTestDirectory temp;

    fs::path catalogPath =
        temp.getPath() /
        ("catalog" + CATALOG_FILE_EXTENSION);

    {
        std::ofstream output(catalogPath);
        output << "0\n";
    }

    EXPECT_THROW(
        Database database(temp.getPath(), 3),
        std::exception
    );
}

// ---------------------------------------------------------
// Table Creation Tests
// ---------------------------------------------------------

TEST(DatabaseTest, CreateTableValid)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Schema schema = createDatabaseTestSchema();

    EXPECT_NO_THROW(
        database.createTable("users", schema)
    );
}

TEST(DatabaseTest, CreateTableCreatesPhysicalFile)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Schema schema = createDatabaseTestSchema();

    database.createTable("users", schema);

    EXPECT_TRUE(
        fs::is_regular_file(
            temp.getPath() /
            ("1_users" + TABLE_FILE_EXTENSION)
        )
    );
}

TEST(DatabaseTest, CreateTableWritesTwoPages)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Schema schema = createDatabaseTestSchema();

    database.createTable("users", schema);

    fs::path filePath =
        temp.getPath() /
        ("1_users" + TABLE_FILE_EXTENSION);

    ASSERT_TRUE(fs::exists(filePath));

    EXPECT_EQ(
        fs::file_size(filePath),
        2 * PAGE_BYTE_LENGTH
    );
}

TEST(DatabaseTest, CreateTableInitializesTableHeader)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Schema schema = createDatabaseTestSchema();

    database.createTable("users", schema);

    FileManager manager(temp.getPath());

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage(
        "1_users" + TABLE_FILE_EXTENSION,
        0,
        bytes
    );

    TableHeader header = TableHeader::decode(bytes);

    EXPECT_EQ(header.getTableID(), 1);
    EXPECT_EQ(header.getNextPageNumber(), 2);

    EXPECT_EQ(
        header.getSchema().encode(),
        schema.encode()
    );
}

TEST(DatabaseTest, CreateTableInitializesFirstDataPage)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Schema schema = createDatabaseTestSchema();

    database.createTable("users", schema);

    FileManager manager(temp.getPath());

    std::vector<std::byte> bytes(
        PAGE_BYTE_LENGTH,
        std::byte{0}
    );

    manager.readPage(
        "1_users" + TABLE_FILE_EXTENSION,
        1,
        bytes
    );

    Page page(PageID{1, 1}, bytes);
    DataPage dataPage(page);

    EXPECT_NO_THROW(
        dataPage.Open(schema.getTupleWidth())
    );

    EXPECT_EQ(
        dataPage.getTupleWidth(),
        schema.getTupleWidth()
    );

    EXPECT_EQ(dataPage.getAllocatedSlotCount(), 0);
    EXPECT_TRUE(dataPage.hasFreeSlot());
}

TEST(DatabaseTest, CreateDuplicateTableDoesNotCreateAnotherFile)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Schema schema = createDatabaseTestSchema();

    database.createTable("users", schema);
    database.createTable("users", schema);

    EXPECT_EQ(
        countFilesWithExtension(
            temp.getPath(),
            TABLE_FILE_EXTENSION
        ),
        1
    );
}

TEST(DatabaseTest, CreateMultipleTables)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Schema schema = createDatabaseTestSchema();

    database.createTable("users", schema);
    database.createTable("orders", schema);
    database.createTable("products", schema);

    EXPECT_EQ(
        countFilesWithExtension(
            temp.getPath(),
            TABLE_FILE_EXTENSION
        ),
        3
    );
}

// ---------------------------------------------------------
// Catalog Persistence Tests
// ---------------------------------------------------------

TEST(DatabaseTest, ReopenDatabasePreservesTables)
{
    DatabaseTestDirectory temp;
    Schema schema = createDatabaseTestSchema();

    {
        Database database(temp.getPath(), 3);

        database.createTable("users", schema);
        database.createTable("orders", schema);
    }

    {
        Database database(temp.getPath(), 3);

        EXPECT_NO_THROW(
            database.insertTuple(
                "users",
                createDatabaseTestTuple(1, "Alice", 35)
            )
        );
    }
}

TEST(DatabaseTest, ReopenDatabaseContinuesTableIDs)
{
    DatabaseTestDirectory temp;
    Schema schema = createDatabaseTestSchema();

    {
        Database database(temp.getPath(), 3);

        database.createTable("users", schema);
        database.createTable("orders", schema);
    }

    {
        Database database(temp.getPath(), 3);

        database.createTable("products", schema);
    }

    EXPECT_TRUE(
        fs::exists(
            temp.getPath() /
            ("3_products" + TABLE_FILE_EXTENSION)
        )
    );
}

TEST(DatabaseTest, ReopenPreservesTableSchema)
{
    DatabaseTestDirectory temp;
    Schema schema = createDatabaseTestSchema();

    {
        Database database(temp.getPath(), 3);
        database.createTable("users", schema);
    }

    {
        Database database(temp.getPath(), 3);

        Tuple tuple = createDatabaseTestTuple(
            100,
            "Alice",
            35
        );

        RID rid = database.insertTuple("users", tuple);

        ASSERT_GE(rid.tupleSlot, 0);

        Tuple result = database.readTuple("users", rid);

        expectTuplesEqual(result, tuple);
    }
}

// ---------------------------------------------------------
// Insert Tests
// ---------------------------------------------------------

TEST(DatabaseTest, InsertTupleValid)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    Tuple tuple = createDatabaseTestTuple(1, "Alice", 35);

    RID rid = database.insertTuple("users", tuple);

    EXPECT_EQ(rid.pageID.tableID, 1);
    EXPECT_EQ(rid.pageID.pageNumber, 1);
    EXPECT_EQ(rid.tupleSlot, 0);
}

TEST(DatabaseTest, InsertMultipleTuples)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    for (std::int64_t i = 0; i < 10; ++i)
    {
        Tuple tuple = createDatabaseTestTuple(
            i,
            "Alice",
            35
        );

        RID rid = database.insertTuple("users", tuple);

        EXPECT_EQ(rid.tupleSlot, i);
        EXPECT_EQ(rid.pageID.tableID, 1);
        EXPECT_EQ(rid.pageID.pageNumber, 1);
    }
}

TEST(DatabaseTest, InsertRejectsWrongTupleWidth)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    Tuple invalidTuple({
        FieldVariant{std::int64_t{1}}
    });

    RID rid = database.insertTuple("users", invalidTuple);

    EXPECT_EQ(rid.tupleSlot, -1);
}

TEST(DatabaseTest, InsertIntoMissingTableFails)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Tuple tuple = createDatabaseTestTuple(1, "Alice", 35);

    RID rid = database.insertTuple("missing", tuple);

    EXPECT_EQ(rid.tupleSlot, -1);
}

// ---------------------------------------------------------
// Read Tests
// ---------------------------------------------------------

TEST(DatabaseTest, ReadInsertedTuple)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    Tuple original = createDatabaseTestTuple(
        1001,
        "Alice",
        35
    );

    RID rid = database.insertTuple("users", original);

    ASSERT_GE(rid.tupleSlot, 0);

    Tuple result = database.readTuple("users", rid);

    expectTuplesEqual(result, original);
}

TEST(DatabaseTest, ReadMultipleDifferentTuples)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    std::vector<Tuple> tuples = {
        createDatabaseTestTuple(1, "Alice", 35),
        createDatabaseTestTuple(2, "Bob", 40),
        createDatabaseTestTuple(3, "Charlie", 25)
    };

    std::vector<RID> rids;

    for (const Tuple& tuple : tuples)
    {
        rids.push_back(
            database.insertTuple("users", tuple)
        );
    }

    for (std::size_t i = 0; i < tuples.size(); ++i)
    {
        ASSERT_GE(rids[i].tupleSlot, 0);

        Tuple result = database.readTuple(
            "users",
            rids[i]
        );

        expectTuplesEqual(result, tuples[i]);
    }
}

// ---------------------------------------------------------
// Update Tests
// ---------------------------------------------------------

TEST(DatabaseTest, UpdateTupleValid)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    Tuple original = createDatabaseTestTuple(
        1, "Alice", 35
    );

    Tuple updated = createDatabaseTestTuple(
        1, "Bob", 40
    );

    RID originalRID = database.insertTuple(
        "users",
        original
    );

    RID updatedRID = database.updateTuple(
        "users",
        originalRID,
        updated
    );

    EXPECT_EQ(
        updatedRID.pageID.tableID,
        originalRID.pageID.tableID
    );

    EXPECT_EQ(
        updatedRID.pageID.pageNumber,
        originalRID.pageID.pageNumber
    );

    EXPECT_EQ(
        updatedRID.tupleSlot,
        originalRID.tupleSlot
    );

    Tuple result = database.readTuple(
        "users",
        originalRID
    );

    expectTuplesEqual(result, updated);
}

TEST(DatabaseTest, UpdateDoesNotModifyOtherTuples)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    Tuple first = createDatabaseTestTuple(1, "Alice", 35);
    Tuple second = createDatabaseTestTuple(2, "Bob", 40);
    Tuple updated = createDatabaseTestTuple(1, "Carol", 50);

    RID firstRID = database.insertTuple("users", first);
    RID secondRID = database.insertTuple("users", second);

    database.updateTuple("users", firstRID, updated);

    expectTuplesEqual(
        database.readTuple("users", firstRID),
        updated
    );

    expectTuplesEqual(
        database.readTuple("users", secondRID),
        second
    );
}

// ---------------------------------------------------------
// Delete Tests
// ---------------------------------------------------------

TEST(DatabaseTest, DeleteTupleRemovesFromScan)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    RID rid = database.insertTuple(
        "users",
        createDatabaseTestTuple(1, "Alice", 35)
    );

    ASSERT_GE(rid.tupleSlot, 0);

    database.deleteTuple("users", rid);

    auto results = database.scan("users");

    EXPECT_TRUE(results.empty());
}

TEST(DatabaseTest, DeleteOneTuplePreservesOthers)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    RID first = database.insertTuple(
        "users",
        createDatabaseTestTuple(1, "Alice", 35)
    );

    RID second = database.insertTuple(
        "users",
        createDatabaseTestTuple(2, "Bob", 40)
    );

    database.deleteTuple("users", first);

    auto results = database.scan("users");

    ASSERT_EQ(results.size(), 1);

    EXPECT_EQ(
        results[0].second.tupleSlot,
        second.tupleSlot
    );

    expectTuplesEqual(
        results[0].first,
        createDatabaseTestTuple(2, "Bob", 40)
    );
}

// ---------------------------------------------------------
// Scan Tests
// ---------------------------------------------------------

TEST(DatabaseTest, ScanEmptyTable)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    auto results = database.scan("users");

    EXPECT_TRUE(results.empty());
}

TEST(DatabaseTest, ScanReturnsAllInsertedTuples)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    for (std::int64_t i = 0; i < 10; ++i)
    {
        database.insertTuple(
            "users",
            createDatabaseTestTuple(i, "Alice", 35)
        );
    }

    auto results = database.scan("users");

    ASSERT_EQ(results.size(), 10);

    for (std::size_t i = 0; i < results.size(); ++i)
    {
        EXPECT_EQ(results[i].second.pageID.tableID, 1);
        EXPECT_EQ(results[i].second.pageID.pageNumber, 1);
        EXPECT_EQ(results[i].second.tupleSlot, i);
    }
}

TEST(DatabaseTest, ScanDoesNotLeakPagePins)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 1);

    database.createTable(
        "users",
        createDatabaseTestSchema()
    );

    database.createTable(
        "orders",
        createDatabaseTestSchema()
    );

    database.insertTuple(
        "users",
        createDatabaseTestTuple(1, "Alice", 35)
    );

    // Repeated scans must release their page pins.
    for (int i = 0; i < 10; ++i)
    {
        auto results = database.scan("users");
        ASSERT_EQ(results.size(), 1);
    }

    // A one-frame buffer pool must be able to evict
    // the users page and fetch the orders page.
    RID rid = database.insertTuple(
        "orders",
        createDatabaseTestTuple(2, "Bob", 40)
    );

    EXPECT_GE(rid.tupleSlot, 0);
    EXPECT_EQ(rid.pageID.tableID, 2);
}

// ---------------------------------------------------------
// Persistence Tests
// ---------------------------------------------------------

TEST(DatabaseTest, InsertedTuplePersistsAfterRestart)
{
    DatabaseTestDirectory temp;
    Schema schema = createDatabaseTestSchema();

    RID rid{};

    Tuple original = createDatabaseTestTuple(
        1001, "Alice", 35
    );

    {
        Database database(temp.getPath(), 3);

        database.createTable("users", schema);

        rid = database.insertTuple("users", original);

        ASSERT_GE(rid.tupleSlot, 0);

        database.flushAllPages();
    }

    {
        Database database(temp.getPath(), 3);

        Tuple result = database.readTuple("users", rid);

        expectTuplesEqual(result, original);
    }
}

TEST(DatabaseTest, UpdatedTuplePersistsAfterRestart)
{
    DatabaseTestDirectory temp;
    Schema schema = createDatabaseTestSchema();

    RID rid{};

    Tuple updated = createDatabaseTestTuple(
        1, "Updated", 50
    );

    {
        Database database(temp.getPath(), 3);

        database.createTable("users", schema);

        rid = database.insertTuple(
            "users",
            createDatabaseTestTuple(1, "Alice", 35)
        );

        database.updateTuple("users", rid, updated);

        database.flushAllPages();
    }

    {
        Database database(temp.getPath(), 3);

        Tuple result = database.readTuple("users", rid);

        expectTuplesEqual(result, updated);
    }
}

TEST(DatabaseTest, DeletedTupleStaysDeletedAfterRestart)
{
    DatabaseTestDirectory temp;
    Schema schema = createDatabaseTestSchema();

    {
        Database database(temp.getPath(), 3);

        database.createTable("users", schema);

        RID rid = database.insertTuple(
            "users",
            createDatabaseTestTuple(1, "Alice", 35)
        );

        database.deleteTuple("users", rid);

        database.flushAllPages();
    }

    {
        Database database(temp.getPath(), 3);

        auto results = database.scan("users");

        EXPECT_TRUE(results.empty());
    }
}

TEST(DatabaseTest, MultipleTablesRemainIndependent)
{
    DatabaseTestDirectory temp;
    Database database(temp.getPath(), 3);

    Schema schema = createDatabaseTestSchema();

    database.createTable("users", schema);
    database.createTable("orders", schema);

    Tuple userTuple = createDatabaseTestTuple(
        1, "Alice", 35
    );

    Tuple orderTuple = createDatabaseTestTuple(
        100, "Order", 20
    );

    RID userRID = database.insertTuple("users", userTuple);
    RID orderRID = database.insertTuple("orders", orderTuple);

    ASSERT_GE(userRID.tupleSlot, 0);
    ASSERT_GE(orderRID.tupleSlot, 0);

    EXPECT_NE(
        userRID.pageID.tableID,
        orderRID.pageID.tableID
    );

    expectTuplesEqual(
        database.readTuple("users", userRID),
        userTuple
    );

    expectTuplesEqual(
        database.readTuple("orders", orderRID),
        orderTuple
    );
}
