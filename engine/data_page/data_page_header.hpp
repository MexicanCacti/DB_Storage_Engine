#pragma once
#include "basic_types.hpp"
#include "cmath"
#include "basic_types_encode_decode.hpp"

/* Page Header:
        {Header: 8 byte reserved for log + 2 byte version # + 2 byte version ID + 2 byte tuple width + 2 byte slot cap + 2 byte alloc slot count + 6 byte pad} H = 24 bytes 
*/

class PageHeader
{
    private:
        std::int64_t logSequenceNumber = 0;
        std::int16_t formatVersion = FORMAT_VERSION;
        std::int16_t formatID = FORMAT_ID;
        std::int16_t tupleWidth;
        std::int16_t slotCapacity;
        std::int16_t allocatedSlotCount;
        PageHeader(std::int64_t lsn, std::int16_t fv, std::int16_t fid, std::int16_t tw, std::int16_t sCap, int16_t allocSlot);
    public:
        // Can publicly create a blank PageHeader
        PageHeader(std::int16_t tupleWidth);
        const std::int64_t& getLogSequenceNumber() const {return logSequenceNumber;}
        const std::int16_t& getFormatVersion() const {return formatVersion;}
        const std::int16_t& getFormatID() const {return formatID;}
        const std::int16_t& getTupleWidth() const {return tupleWidth;}
        const std::int16_t& getSlotCapacity() const {return slotCapacity;}
        const std::int16_t& getAllocatedSlotCount() const {return allocatedSlotCount;}
        void setAllocatedSlotCount(std::int16_t allocatedCount);
        // B(n) = 8 * ceil(n/64), n = # of slots
        static std::size_t calcBitmapSize(std::size_t slotCapacity);
        // H + 2 * B(n) + n * R <= 4096, H = header size, R = Tuple Width
        static std::size_t calcSlotCapacity(std::int16_t tupleWidth);

        // Static getter so that these values are retrievable without reconstructing a PageHeader every time
        static std::int64_t getLogSequenceNumber(const std::vector<std::byte>& headerBytes);
        static std::int16_t getFormatVersion(const std::vector<std::byte>& headerBytes);
        static std::int16_t getFormatID(const std::vector<std::byte>& headerBytes);
        static std::int16_t getTupleWidth(const std::vector<std::byte>& headerBytes);
        static std::int16_t getSlotCapacity(const std::vector<std::byte>& headerBytes);
        static std::int16_t getAllocatedSlotCount(const std::vector<std::byte>& headerBytes);

        static PageHeader decode(const std::vector<std::byte>& headerBytes);
        std::vector<std::byte> encode() const;

        static bool similarFormats(const PageHeader& p1, const PageHeader& p2);
};