#pragma once
#include "../schema/schema.hpp"

class TableHeader {
    private:
        std::uint8_t tableID;
        Schema tableSchema;
        std::uint8_t nextPageNumber;
    public:
        TableHeader(std::uint8_t tableID, const Schema& tableSchema, std::uint8_t nextPage) : tableID(tableID), tableSchema(tableSchema), nextPageNumber(nextPage) {;}
        const Schema& getSchema() const { return tableSchema;}
        const std::uint8_t& getTableID() const { return tableID; }
        const std::uint8_t& getNextPageNumber() const { return nextPageNumber;}
        const std::vector<std::byte> encodeTableHeader(); 
        static TableHeader decodeTableHeader(std::vector<std::byte>& tableHeaderBytes);
};