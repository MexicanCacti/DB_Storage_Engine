#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <cstdint>

#include "helpers.hpp"

/*
    Page Byte setup:
        {Header: 8 byte reserved for log + 2 byte version # + 2 byte version ID + 2 byte tuple width + 2 byte slot cap + 2 byte alloc slot count + 6 byte pad} H = 24 bytes 
        {8 Byte alloc bit map}
        {8 byte del bit map}
        {200 x 20 Tuple Data ... 4000 Bytes} ... Max tuple width = 200 so 20 slots
        {56 Byte padding}

*/

struct PageID {
    std::int64_t tableID;
    std::int64_t pageNumber; 
};

const std::size_t PAGE_BYTE_LENGTH = 4096;

class Page
{
    private:
        PageID pageID;
        std::vector<std::byte> pageBytes;

    public:
        Page(const PageID& pageID) : pageID(pageID) {pageBytes = std::vector<std::byte>(PAGE_BYTE_LENGTH, std::byte{0});}
        Page(const PageID& pageID, const std::vector<std::byte>& dataBytes);
        const PageID& getPageID() const {return pageID;}
        const std::int64_t& getPageNumber() const {return pageID.pageNumber;}
        const std::int64_t& getTableID() const {return pageID.tableID;}
        const std::vector<std::byte>& getPageBytes() const {return pageBytes;}
        const std::size_t& getPageSize() const {return PAGE_BYTE_LENGTH;}
};