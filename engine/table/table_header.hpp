#pragma once

#include "schema.hpp"
#include "bitmap.hpp"

class TableHeader 
{
    private:
        std::string tableName;
        std::int64_t tableID;
        Schema tableSchema;
        std::int64_t nextPageNumber = 1;
        TableHeader(const std::string& tableName, std::int64_t tableID, std::int64_t nextPageNumber, const Schema& tableSchema);
    public:
        TableHeader(const std::string& tableName, std::int64_t tableID, const Schema& tableSchema);

        const Schema& getSchema() const {return tableSchema;}
        const std::int64_t& getTableID() const {return tableID;}
        std::int64_t getNextPageNumber() const {return nextPageNumber;}
        std::int64_t allocatedNextPageNumber() {return nextPageNumber++;}
        const std::vector<std::byte> encode();
        static TableHeader decode(const std::vector<std::byte>& tableHeaderBytes);
};