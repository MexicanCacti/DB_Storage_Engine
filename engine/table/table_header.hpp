#pragma once

#include "schema.hpp"

// This will be Page 0 for the heap file
// So when we read... read from file -> Page... THen if first page, use Page Bytes -> TableHeader

class TableHeader 
{
    private:
        std::string tableName;
        std::int64_t tableID;
        Schema tableSchema;
    public:
        TableHeader(const std::string& tableName, std::int64_t tableID, const Schema& tableSchema);

        const Schema& getSchema() const {return tableSchema;}
        const std::int64_t& getTableID() const {return tableID;}
        const std::vector<std::byte> encode();
        static TableHeader decode(const std::vector<std::byte>& tableHeaderBytes);
};