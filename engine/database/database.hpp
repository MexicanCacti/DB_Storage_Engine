#pragma once

#include "file_manager.hpp"
#include "buffer_pool.hpp"
#include "tuple.hpp"
#include "basic_types.hpp"
#include "basic_types_encode_decode.hpp"
#include "schema.hpp"
#include "helpers.hpp"
#include "table_header.hpp"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <unordered_set>
#include <stdexcept>
#include <memory>

class Database
{
    private:
        FileManager fileManager;
        std::int64_t nextTableID = 0;
        std::unordered_map<std::string, std::int64_t> tableNameToID;
        std::unordered_map<std::int64_t, std::string> tableIDToFile;
        std::unordered_map<std::int64_t, Schema> tableIDToSchema;
        std::unique_ptr<BufferPool> bufferPool;
        std::filesystem::path catalogPath;
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

        void closePage(const PageID& pageID, bool isDirty = false) {bufferPool->unpinPage(pageID, isDirty);}
        void flushPage(const PageID& pageID) {bufferPool->flushPage(pageID);}

        std::string getTableFileName(const std::string& tableName) const;

        /*
            openTable takes in the table's name and then grabs the ID using the saved map
            Then using the ID, it will grab the fileName
            Will Read the bytes from the filename:
                1. First decode the tableHeader
                2. Then copy the bytes to a Page
                3. The Page will be placed in the buffer pool w/ the PageID from page as the key
        */
        Page& openTable(const std::string& tableName);

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
        void createTable(const std::string& tableName, const Schema& schema);

        RID insertTuple(const std::string& tableName, Tuple tuple); // For now only 1 page (so always use page # 1)
        Tuple readTuple(const std::string& tableName, RID recordID);
        RID updateTuple(const std::string& tableName, RID recordID, Tuple updatedTuple);
        void deleteTuple(const std::string& tableName, RID recordID);
        std::vector<std::pair<Tuple, RID>> scan(const std::string& tableName);
        void flushAllPages();
        ~Database();
};