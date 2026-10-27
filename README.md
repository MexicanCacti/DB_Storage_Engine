### Github Link: https://github.com/MexicanCacti/DB_Storage_Engine

# Development Environment

This project is implemented in C++ using CMake.

# Build requirements

C++17-compatible compiler or newer

CMake 3.14 or newer

GoogleTest for unit testing

Windows or another supported operating system

# Run Instructions 
(From Top-Level Directory [...\DB_Storage_Engine] )

- cmake -S . -B build
- cmake --build build --config Debug

# Running Tests
(From Top-Level Directory [...\DB_Storage_Engine] )
- [Google_Tests] <br> ./google_tests/Debug/all_tests
- [4_6_Tests] <br> ./tests/Debug/4_6_tests
- [5_6_Tests] <br> ./tests/Debug/5_6_tests
- [storage_demo] <br> ./tests/Debug/storage_demo

# My Development Environment Info
- Operating System: Windows
- System Architecture: AMD64
- C++ Compiler: MSVC
- Compiler Version: 19.51.36246.0
- C++ Standard: 17
- CMake Version: 4.3.3
- Build Generator: Visual Studio 18 2026

# Project Organization
- engine/ - Database storage engine code
- google_tests/ - Unit tests written with GoogleTests for testing class functionality
- tests/ - Hand-written tests used to demonstrate engine correctness

# Database Storage Engine API

## Overview

The storage engine is accessed through the `Database` class, defined in `database.hpp` and implemented in `database.cpp`.

The API provides functionality for table creation, tuple insertion, retrieval, modification, deletion, scanning, and persistent storage management.

## 1. Database Initialization

### `Database(const std::filesystem::path& directory, std::size_t bufferFrameCount)`

Initializes the database storage engine.

**Parameters:**
- `directory` — Filesystem path where the engine stores and retrieves persistent database files.
- `bufferFrameCount` — Number of frames allocated to the buffer pool. Defaults to `3`.

## 2. Table Management

### `createTable(const std::string& tableName, const Schema& schema)`

Creates a new database table with the specified name and schema.

**Parameters:**
- `tableName` — Name of the table to create.
- `schema` — Schema defining the table's fields and their data types.

**Behavior:**
- Creates a dedicated file for the table.
- Stores the provided schema information in the table's metadata.

### `TableHeader queryTableInfo(std::string& tableName) const`

Retrieves the metadata associated with a database table.

**Parameters:**
- `tableName` — Name of the table to query.

**Returns:**
- A copy of the table's `TableHeader`, containing metadata stored in the table's file.

## 3. Tuple Operations

### `RID insertTuple(const std::string& tableName, Tuple tuple)`

Inserts a tuple into the specified table.

**Parameters:**
- `tableName` — Name of the target table.
- `tuple` — Tuple containing the values to insert.

**Behavior:**
- Fetches the table's data page into the buffer pool.
- Locates the next available tuple slot.
- Writes the encoded tuple into the selected slot.

**Returns:**
- An `RID` (Record Identifier) identifying the inserted tuple.

**Current limitation:** Only one data page per table is supported. All tuples are stored on page `1`.

### `Tuple readTuple(const std::string& tableName, RID recordID)`

Retrieves a tuple using its record identifier.

**Parameters:**
- `tableName` — Name of the table containing the tuple.
- `recordID` — Record identifier of the tuple to retrieve.

**Behavior:**
- Fetches the corresponding page into memory.
- Uses the `RID` to locate the tuple's offset within the page.
- Reads and decodes the tuple's stored bytes.

**Returns:**
- A copy of the decoded `Tuple` object.

### `RID updateTuple(const std::string& tableName, RID recordID, Tuple updatedTuple)`

Updates an existing tuple.

**Parameters:**
- `tableName` — Name of the table containing the tuple.
- `recordID` — Record identifier of the tuple to update.
- `updatedTuple` — Tuple containing the replacement values.

**Behavior:**
- Fetches the page containing the target tuple.
- Overwrites the tuple's existing bytes with the encoded replacement.
- Marks the modified page as dirty so that its changes can be persisted.

**Returns:**
- The `RID` associated with the updated tuple.

### `void deleteTuple(const std::string& tableName, RID recordID)`

Logically deletes a tuple from the specified table.

**Parameters:**
- `tableName` — Name of the table containing the tuple.
- `recordID` — Record identifier of the tuple to delete.

**Behavior:**
- Locates the tuple using its `RID`.
- Sets the corresponding deletion bit to mark the tuple as deleted.

### `std::vector<std::pair<Tuple, RID>> scan(const std::string& tableName)`

Scans a table and retrieves its tuples.

**Parameters:**
- `tableName` — Name of the table to scan.

**Behavior:**
- Iterates through the table's stored tuples.
- Decodes the tuple data.
- Associates each decoded tuple with its corresponding `RID`.

**Returns:**
- A `std::vector<std::pair<Tuple, RID>>` containing the decoded tuples and their record identifiers.

## 4. Buffer Pool and Persistence

### `void flushAllPages()`

Flushes modified pages from the buffer pool to persistent storage.

**Behavior:**
- Iterates through the buffer pool.
- Identifies pages marked as dirty.
- Writes dirty pages back to their corresponding files.

## 5. Database Destruction

### `~Database()`

Destroys the `Database` instance.

**Behavior:**
- Calls `flushAllPages()` to persist outstanding modifications before the database instance is destroyed.

---

## Current Implementation Limitations

- **Single data page per table:** Tuple storage is currently restricted to page `1` of each table file.
- **Fixed-size buffer pool:** The number of buffer frames is configured during database initialization.


### Installing the library (optional)
- cmake --build build --config Debug --target install
