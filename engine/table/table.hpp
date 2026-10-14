#pragma once

#include "./table_header.hpp"

class Table {
    private:
        TableHeader tableHeader;

    public:
        Table(TableHeader& tableHeader) : tableHeader(tableHeader) {;}
        const std::uint8_t& getTableID() const {return tableHeader.getTableID();}
        const Schema& const getSchema() const {return tableHeader.getSchema();}
        const TableHeader& getTableHeader() const {return tableHeader;}
        const std::uint8_t& getNextPageNumber() const {return tableHeader.getNextPageNumber();}
};