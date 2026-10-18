#pragma once

/* Page Header:
        {Header: 8 byte reserved for log + 2 byte version # + 2 byte version ID + 2 byte tuple width + 2 byte slot cap + 2 byte alloc slot count + 6 byte pad} H = 24 bytes 
        {8 Byte alloc bit map}
        {8 byte del bit map}
*/

class PageHeader
{
    private:
        std::int64_t logSequenceNumber = 0;
        std::int16_t versionNumber;
        std::int16_t versionID;
        std::int16_t tupleWidth;
        std::int16_t slotCapacity;
        std::int16_t allocatedSlotCount;
        // 8 Byte alloc Bitmap
        // 8 Byte del Bitmap
    public:
        

}