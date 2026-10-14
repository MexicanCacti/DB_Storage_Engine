#pragma once
#include <array>
#include <cstddef>

struct PageID {
    std::uint64_t tableID;
    std::uint64_t pageNumber;
};

const std::uint64_t dataByteLength = 4096;

/*
    For a 1 page set up... 
    Page 0 = Table meta data... Table Name, ID, Schema Info
    Page 1 = 
                {Header: 8 byte reserved for log + 2 byte version # + 2 byte version ID + 2 byte tuple width + 2 byte slot cap + 2 byte alloc slot count + 6 byte pad} H = 24 bytes 
                {8 Byte alloc bit map}
                {8 byte del bit map}
                {200 x 20 Tuple Data ... 4000 Bytes} ... Max tuple width = 200 so 20 slots
                {56 Byte padding}

    Then to access a tuple slot the following formula is used:
    H + (2 x B(n)[bitmap size = 8]) + i [slot #] x 200 [slot width = 200]

*/

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