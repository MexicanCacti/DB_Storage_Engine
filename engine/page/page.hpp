#pragma once
#include <array>
#include <cstddef>

struct PageID {
    std::uint64_t tableID;
    std::uint64_t pageNumber;
};

const std::uint64_t dataByteLength = 4096;

class Page {
    private:
        PageID pageID;
        std::array<std::byte, dataByteLength> dataBytes{};
        Page() = default;
    public:
        Page(const PageID& pageID) : pageID(pageID) {;}
        Page(const PageID& pageID, const std::array<std::byte, dataByteLength>& dataBytes) : pageID(pageID), dataBytes(dataBytes) {;}
        const PageID& getPageID() const {return pageID;}
        const std::uint64_t& getPageNumber() const {return pageID.pageNumber;}
        const std::uint64_t& getTableID() const {return pageID.tableID;}
        const std::array<std::byte, dataByteLength>& getData() const {return dataBytes;}
};