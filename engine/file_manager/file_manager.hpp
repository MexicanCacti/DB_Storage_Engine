#pragma once

// For creating tables / allocating pages... need way to track page # and table ID
// Catalog file!
// Would contain...
// TableID ... TableName ... File
// Then cache this into memory
class FileManager {
    private:
        // Path to store DB files
        // Buffer Pool... Probably a map for RID: Data

    public:
        FileManager(void* pathToDBDirectory); // Then go through all the files and set everything up!

        Table createTable(const std::string& tableName, const Schema& schema);

        Table openTable(const std::string& tableName);

        Page allocatePage(std::uint8_t tableID);
        Page readPage(const PageID& pageID);
        void writePage(const Page& page);
        void closeTable(std::uint8_t tableID);
};