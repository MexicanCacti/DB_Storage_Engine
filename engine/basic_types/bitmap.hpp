#pragma once


class Bitmap
{
    private:
        std::vector<std::byte>& pageBytes;
        
        std::size_t bitmapOffset;
        std::size_t bitmapSlotCount;

        std::size_t getStoredBitmapSize() const;
        std::byte& getBitByte(std::size_t index) const;
        bool isValidIndex(std::size_t index) const;

    public:
        Bitmap(std::vector<std::byte>& pageBytes, std::size_t offset, std::size_t slotCount);

        std::int8_t readBit(std::size_t index) const;
        std::int8_t setBit(std::size_t index);
        std::int8_t clearBit(std::size_t index);

        std::size_t findFirstZeroBitIndex(std::size_t index) const;
};