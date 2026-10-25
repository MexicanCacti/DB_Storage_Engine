#include "database.hpp"

void Database::loadCatalog()
{
    std::fstream fileInput;
    fileInput.open(catalogPath, std::fstream::in);

    if(!fileInput.is_open()) throw std::runtime_error("[loadCatalog]: Failed to open file");

    std::string readLine;

    // Get the next_table_id
    if(!std::getline(fileInput, readLine)) throw std::runtime_error("[loadCatalog]: No next_table_id line");

    nextTableID = std::stoll(readLine);
    if(nextTableID < 1) throw std::runtime_error("[loadCatalog]: Invalid next_table_id");

    std::istringstream splitLine;

    while(std::getline(fileInput, readLine))
    {
        if(readLine.empty()) continue;
        std::istringstream splitLine(readLine);

        std::int64_t tableID;
        std::string tableName;
        std::string fileName;

        splitLine >> tableID;
        splitLine >> tableName;
        splitLine >> fileName;

        tableNameToID[tableName] = tableID;
        tableIDToFile[tableID] = fileName;
    }
}

void Database::saveCatalog()
{
    std::fstream fileOutput;
    fileOutput.open(catalogPath, std::ios::out | std::ios::trunc);

    if(!fileOutput.is_open()) throw std::runtime_error("[saveCatalog]: Failed to open catalog");

    fileOutput << nextTableID << "\n";

    for(auto& [tableName, tableID] : tableNameToID)
    {
        auto fileName = tableIDToFile.at(tableID);

        fileOutput << tableID << CATALOG_DELIM << tableName << CATALOG_DELIM << fileName << "\n";

        if(!fileOutput) throw std::runtime_error("[saveCatalog]: Failed to write catalog entry");
    }

    fileOutput.flush();
    fileOutput.close();
}

std::string Database::getTableFileName(const std::string& tableName) const
{
    std::int64_t tableID = tableNameToID.at(tableName);
    return tableIDToFile.at(tableID);
}

Database::Database(const std::filesystem::path& directory, std::size_t bufferFrameCount)
    : fileManager(directory)
{

    if(bufferFrameCount <= 0) throw std::invalid_argument("[Database]: Invalid bufferFrameCount");

    std::unordered_set<std::string> tableFiles;

    for(auto& dirEntry : std::filesystem::directory_iterator(directory))
    {
        if(!dirEntry.is_regular_file()) continue;

        const auto filePath = dirEntry.path();

        if(filePath.extension() == CATALOG_FILE_EXTENSION)
        {   
            catalogPath = filePath;
        }
        else if(filePath.extension() == TABLE_FILE_EXTENSION)
        {
            tableFiles.insert(filePath.filename().string());
        }
    }

    std::size_t tableFileCount = 0;
    if(catalogPath.empty())
    {
        // create a new catalog file
        catalogPath = directory / ("catalog" + CATALOG_FILE_EXTENSION);
        std::fstream fileOutput;
        fileOutput.open(catalogPath, std::ios::out | std::ios::trunc);

        if(!fileOutput.is_open()) throw std::runtime_error("[saveCatalog]: Failed to open catalog");
        fileOutput << 1 << "\n";
    }
    loadCatalog();

    for(auto& [tableID, fileName] : tableIDToFile)
    {
        if(tableFiles.find(fileName) == tableFiles.end() ) throw std::runtime_error("[Database]: Reading file that shouldn't exist");
        std::vector<std::byte> headerBytes(PAGE_BYTE_LENGTH, std::byte{0});
        fileManager.readPage(fileName, 0, headerBytes);
        TableHeader tableHeader = TableHeader::decode(headerBytes);

        if(tableHeader.getTableID() != tableID) throw std::runtime_error("[Database]: Table ID doesn't match");
        tableIDToSchema[tableID] = tableHeader.getSchema();
        ++tableFileCount;
    }

    if(tableFileCount != tableFiles.size()) throw std::runtime_error("[Database]: Table files found & processed differ");

    bufferPool = std::make_unique<BufferPool>(fileManager, tableIDToFile, bufferFrameCount);

    // For every file in the directory
    // If it ends in xD = Catalog File
    // If it end in x3 = Database File

    // Get the catalog file path
    // Get a list of Database files

    // First populate the tableNameToID & tableIDToFile w/ the Catalog File
    // Then go through Database files, read Page 0, extract the encoded TableHeader's tuplewidth to populate the map

    // Init Bufferpool with (fileManager, tableIDToFile, bufferFrameCount)
}

void Database::createTable(const std::string& tableName, const Schema& schema)
{
    try
    {
        if(tableNameToID.find(tableName) != tableNameToID.end()) throw std::invalid_argument("[createTable]: Table exists");
        std::int64_t tableID = nextTableID;
        std::string fileName = std::to_string(tableID) + "_" + tableName + TABLE_FILE_EXTENSION;

        TableHeader tableHeader(tableName, nextTableID, schema);

        std::int64_t firstPageNumber = tableHeader.allocatedNextPageNumber();

        Page tablePage(PageID{tableID, 0});

        copyBytes(tablePage.getPageBytes(), tableHeader.encode());

        Page firstPage(PageID{tableID, firstPageNumber});
        DataPage dataPage(firstPage);
        dataPage.Initialize(schema.getTupleWidth());

        fileManager.createFile(tableName);

        fileManager.writePage(fileName, 0, tablePage.getPageBytesConst());
        fileManager.writePage(fileName, 1, firstPage.getPageBytesConst());

        tableNameToID[tableName] = nextTableID;
        tableIDToFile[tableID] = fileName;
        tableIDToSchema[tableID] = schema;
        nextTableID++;
    
        saveCatalog();
    }
    catch(std::exception& e)
    {
        std::cerr << "[createTable]: " << e.what() << "\n";
    }
}

// Grabs a page & pins & caches
// Whoever uses this better unpin!
Page& Database::openTable(const std::string& tableName)
{
    if(tableNameToID.find(tableName) == tableNameToID.end()) throw std::invalid_argument("[openTable]: Table doesn't exist");
    std::int64_t tableID = tableNameToID.at(tableName);
    PageID pageID{tableID, 1};
    try
    {
        Page& page = bufferPool->fetchPage(pageID);
        DataPage dataPage(page);
        dataPage.Open(tableIDToSchema.at(tableID).getTupleWidth());
        return page;
    }
    catch(std::exception& e)
    {
        std::cerr << "[openTable]: " << e.what() << "\n";
    }
}

// Returns RID w/ tuple slot = -1 on table full
RID Database::insertTuple(const std::string& tableName, Tuple tuple)
{
    try
    {
        Page& page = openTable(tableName);
        DataPage dataPage(page);
        RID rid = dataPage.insertTuple(tuple.encode());
        bufferPool->unpinPage(dataPage.getPageID(), true);
        return rid;
    }
    catch(std::exception& e)
    {
        std::cerr << "[insertTuple]: " << e.what() << "\n";
    }
    return RID{{-1, -1}, -1};
}

Tuple Database::readTuple(const std::string& tableName, RID recordID)
{
    try
    {
        Page& page = openTable(tableName);
        DataPage dataPage(page);
        std::vector<std::byte> tupleBytes = dataPage.readTuple(recordID.tupleSlot);
        bufferPool->unpinPage(dataPage.getPageID(), false);
        return Tuple(Tuple::decode(tupleBytes, tableIDToSchema.at(recordID.pageID.tableID).getFieldList()));
    }
    catch(std::exception& e)
    {
        std::cerr << "[insertTuple]: " << e.what() << "\n";
    }
    return Tuple({});
}

RID Database::updateTuple(const std::string& tableName, RID recordID, Tuple updatedTuple)
{
    try
    {
        Page& page = openTable(tableName);
        DataPage dataPage(page);
        RID rid = dataPage.updateTuple(recordID.tupleSlot, updatedTuple.encode());
        bufferPool->unpinPage(dataPage.getPageID(), true);
        return rid;
    }
    catch(std::exception& e)
    {
        std::cerr << "[updateTuple]: " << e.what() << "\n";
    }
    return RID{{-1, -1}, -1};
}

void Database::deleteTuple(const std::string& tableName, RID recordID)
{
    try
    {
        Page& page = openTable(tableName);
        DataPage dataPage(page);
        dataPage.deleteTuple(recordID.tupleSlot);
        bufferPool->unpinPage(dataPage.getPageID(), true);
    }
    catch(std::exception& e)
    {
        std::cerr << "[deleteTuple]: " << e.what() << "\n";
    }
}

std::vector<Tuple> Database::scan(const std::string& tableName)
{
    std::vector<Tuple> tupleList;
    try
    {
        Page& page = openTable(tableName);
        DataPage dataPage(page);
        std::size_t slotCap = dataPage.getSlotCapacity();
        for(std::size_t index = 0 ; index < slotCap; ++index)
        {
            try
            {
                std::vector<std::byte> tupleBytes = dataPage.readTuple(index);
                tupleList.push_back(Tuple(Tuple::decode(tupleBytes, tableIDToSchema.at(tableNameToID.at(tableName)).getFieldList())));
            }
            catch(std::exception& e)
            {

            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "[scan]: " << e.what() << "\n";
    }

    return tupleList;

}

void Database::flushAllPages()
{
    bufferPool->flushAllPages();
}

Database::~Database()
{
    flushAllPages();
}