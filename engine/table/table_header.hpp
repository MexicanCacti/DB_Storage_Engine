#pragma once
#include "../schema/schema.hpp"


class TableHeader {
    private:
        std::string tableName;
        std::int16_t tableID;
        Schema tableSchema;
    public:
        TableHeader(std::string& tableName, std::int16_t tableID, const Schema& tableSchema);
        const Schema& getSchema() const { return tableSchema;}
        const std::int16_t& getTableID() const { return tableID; }
        const std::vector<std::byte> encodeTableHeader(); 
        static TableHeader decodeTableHeader(std::vector<std::byte>& tableHeaderBytes);
};