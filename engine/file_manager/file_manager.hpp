#pragma once
#include <unordered_map>

// For creating tables / allocating pages... need way to track page # and table ID
// Catalog file!
// Would contain...
// TableID ... TableName ... File
// Then cache this into memory

// Buffer Pool... Probably a map for RID: Data

// {TableName, FileName}
using TableInfo = std::pair<std::string, std::string>;

class FileManager {
    private:
        std::string dataFileDirectory;
        std::int64_t NEXT_TABLE_ID;
        std::unordered_map<std::int8_t, TableInfo> tableLookup;
    public: 
        /*
            This function will first grab the catalog file (catalog.xD)
            Will also set the dataFileDirectory string

            Catalog file format will have rows that look like so:
                NEXT_TABLE_ID
                TABLE_ID    TABLE_NAME  FILE_NAME
                
            The NEXT_TABLE_ID will be used so track what the next table ID assignment will be
            The rows after will be used to build the tableLookup map
        */
        FileManager(void* pathToDBDirectory); // For now, will always be ./files/

        /*
            CreateTable takes in the table's name and desired schema then will create TableHeader object.
            The TableHeader has its own encode method...
                Its encode will be used to create a new file with the title: [ID]_[TABLE_NAME].xD
                Where [ID] is replaced w/ the assigned tableID (grabbed from NEXT_TABLE_ID)
                Where [TABLE_NAME] is replaced w/ the tableName

            Then a new page will be created w/ a pageID = 0
            Then using the schema info...
            Page will be passed in to Init a DataPage [DataPage(Page& page); DataPage.Initialize]
        */
        TableHeader createTable(const std::string& tableName, const Schema& schema);

        TableHeader openTable(const std::string& tableName);

        Page allocatePage(std::uint8_t tableID);
        Page readPage(const PageID& pageID);
        void writePage(const Page& page);
        void closeTable(std::uint8_t tableID);
};