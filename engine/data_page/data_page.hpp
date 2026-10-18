#pragma once

/*
    Page Byte setup:
        {Header = 24 Bytes}
        {Bitmaps = 16 Bytes total}
        {200 x 20 Tuple Data ... 4000 Bytes} ... Max tuple width = 200 so 20 slots
        {56 Byte padding}

*/

struct RID {
    PageID pageID;
    std::int16_t tupleSlot;
};

class DataPage
{
    private:
        std::vector<std::byte> dataPageBytes;

    public:
}