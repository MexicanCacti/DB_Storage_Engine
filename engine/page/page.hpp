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

    // Operator needed for map comparisons
    bool operator==( const PageID& p2) const
    {
        return tableID == p2.tableID && pageNumber == p2.pageNumber;
    }

    // Needed for unordered_maps using PageID!
    // Probably a bad hash but that could be fixed later!
    struct PageIDHasher {
        std::size_t operator()(const PageID& pageID) const
        {
            std::size_t tableIDHash = std::hash<std::int64_t>{}(pageID.tableID);
            std::size_t pageNumberHash = std::hash<std::int64_t>{}(pageID.pageNumber);
            
            return (tableIDHash << 1) + pageNumberHash + (tableIDHash >> 1);
        }
    };
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
        std::vector<std::byte> getPageBytesCopy() {return pageBytes;}
        const std::size_t& getPageSize() const {return PAGE_BYTE_LENGTH;}
};