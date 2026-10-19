#pragma once

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <cstdint>

#include "basic_types.hpp"
#include "helpers.hpp"

// This class is solely just to store the raw bytes read from a file
// Will also be used to hold bytes to be written to file later

struct PageID {
    std::int64_t tableID;
    std::int64_t pageNumber;
};

class Page
{
    private:
        PageID pageID;
        std::vector<std::byte> pageBytes;
    public:
        Page();
        Page(const PageID& pageID);
        Page(const PageID& pageID, const std::vector<std::byte>& dataBytes);
        const PageID& getPageID() const {return pageID;}
        const std::int64_t& getPageNumber() const {return pageID.pageNumber;}
        const std::int64_t& getTableID() const {return pageID.tableID;}
        const std::vector<std::byte>& getPageBytesConst() const {return pageBytes;}
        std::vector<std::byte>& getPageBytes() {return pageBytes;}
        const std::size_t& getPageSize() const {return PAGE_BYTE_LENGTH;}
};