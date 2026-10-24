#pragma once
#include "basic_types.hpp"
#include "helpers.hpp"
#include <cmath>

// NOTE -> Needs encoder
class Bitmap
{
    private:
        std::vector<std::byte>& pageBytes;
        
        std::size_t bitmapOffset;
        std::int16_t bitmapSlotCount;

        std::size_t getStoredBitmapSize() const;
        std::byte& getBitByte(std::size_t index) const;
        bool isValidIndex(std::size_t index) const;

    public:
        Bitmap(std::vector<std::byte>& pageBytes, std::size_t offset, std::int16_t slotCount);

        bool readBit(std::size_t index) const;
        bool setBit(std::size_t index);
        bool clearBit(std::size_t index);

        std::size_t findFirstZeroBitIndex(std::size_t index) const;
};