#pragma once
#include "data_page_header.hpp"
    /*
    Page Byte setup:
        {Header = 24 Bytes}
        {Bitmaps = B(n) = 8 * ceil(n / 64)}
        {# of slots x Tuple Width }
        {Byte padding}

    Indexing into a slot:
        H + 2 x B(n) + i X R
        H = header size
        B(n) = bitmap size
        i = ith slot to index
        R = tuple width

    Note: Bitmap not actually saved explicitly
    */

struct RID {
    PageID pageID;
    std::int16_t tupleSlot;
};

// This class interpets the page containing the tuple data & updates the pageBytes.
// This class is not responsible for any I/O btw

class DataPage
{
    private:
        Page& page;

        //std::size_t getAllocationBitmapOffset() const;
        //std::size_t getDeletionBitmapOffset() const;

        //bool getBitmapBit(std::size_t bitmapOffset, std::size_t slotIndex) const;
        //void setBitmapBit(std::size_t bitmapOffset, std::size_t slotIndex, bool value);

    public:
        DataPage(Page& page);

        // Zero-filled w/ header created correctly
        void Initialize(Page& page, std::int16_t tupleWidth);
        void Open(Page& page, std::int16_t expectedTupleWidth);

        const PageID& getPageID() const {return page.getPageID();}
        const Page& getPageConst() const {return page;}
        Page& getPage() {return page;}
        PageHeader getPageHeader() const {return pageHeader;}
        std::int16_t getTupleWidth() const {return pageHeader.tupleWidth;}
        std::int16_t getSlotCapacity() const {return pageHeader.slotCapacity;}
        std::int16_t getAllocatedSlotCount() const {return pageHeader.allocatedSlotCount;}
        std::size_t getSlotOffset(std::size_t slotIndex) const;

        bool isAllocated(std::size_t slotIndex) const;
        bool isDeleted(std::size_t slotIndex) const;
        bool hasFreeSlot() const;

        RID insertTuple(const std::vector<std::byte>& encodedTuple);
        std::vector<std::byte> readTuple(std::size_t slotIndex) const;
        void deleteTuple(std::size_t slotIndex);
};
