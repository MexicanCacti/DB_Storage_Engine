#pragma once

#include "file_manager.hpp"
#include "buffer_pool.hpp"

#include <filesystem>

class Database
{
    private:
        FileManager fileManager;
        BufferPool bufferPool;
        std::int64_t nextTableID = 0;
        std::unordered_map<std::string, std::int64_t> tableNameToID;
        std::unordered_map<std::int64_t, std::string> tableIDToFile;
        std::unordered_map<std::int64_t, std::int16_t> tableIDToTupleWidth; // Needed for DataPage Init/Open

        /*
                Catalog file format will have rows that look like so:
                NEXT_TABLE_ID
                TABLE_ID    TABLE_NAME  FILE_NAME
                
            The NEXT_TABLE_ID will be used so track what the next table ID assignment will be
            The rows after will be used to build the tableNameToID & tableIDToFile & tableIDToTupleWidth map

            Will then initialize the Maps by going through every DB file and read the header (Page 0)
        */
        void loadCatalog();
        void saveCatalog();

    public:
        Database(const std::filesystem::path& directory, std::size_t bufferFrameCount = DEFAULT_BUFFER_FRAME_COUNT);
        
        /*
            CreateTable takes in the table's name and desired schema then will create TableHeader object.
            The TableHeader has its own encode method...
                Its encode will be used to create a new file with the title: [ID]_[TABLE_NAME].xD
                Where [ID] is replaced w/ the assigned tableID (grabbed from NEXT_TABLE_ID)
                Where [TABLE_NAME] is replaced w/ the tableName

            Then a new page will be created w/ a pageID = 0

            Then using the schema info...
            Page will Init w/ DataPage: {DataPage(Page& page); DataPage.Initialize(tupleWidth)}
        */
        TableHeader createTable(const std::string& tableName, const Schema& schema);

        /*
            openTable takes in the table's name and then grabs the ID using the saved map
            Then using the ID, it will grab the fileName
            Will Read the bytes from the filename:
                1. First decode the tableHeader
                2. Then copy the bytes to a Page
                3. The Page will be placed in the buffer pool w/ the PageID from page as the key
        */
        TableHeader openTable(const std::string& tableName);
        PageID allocatePage(const std::string& tableName);
        Page& openOpen(const PageID& pageID);
        void closePage(const PageID& pageID, bool isDirty = false);
        void flushPage(const PageID& pageID);
        void flushAllPages();
        // Getters for tableID lookup??
};