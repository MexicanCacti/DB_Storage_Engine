#include "bitmap.hpp"

bool Bitmap::isValidIndex(std::size_t index) const
{
    return index < bitmapSlotCount;
}

std::size_t Bitmap::getStoredBitmapSize() const
{
    // B(n) = 8 * ceil(n/64), n = # of slots
    return 8 * std::ceil( (bitmapSlotCount + 63) / 64);
}

std::byte& Bitmap::getBitByte(std::size_t index) const
{
    if(!isValidIndex(index)) throw std::invalid_argument("[getBitByte]: supplied index not valid");

    std::size_t bitByteIndex = bitmapOffset + (index / 8);
    return pageBytes[bitByteIndex];
}

Bitmap::Bitmap(std::vector<std::byte>& pageBytes, std::size_t offset, std::int16_t slotCount)
    : pageBytes(pageBytes), bitmapOffset(offset), bitmapSlotCount(slotCount)
{
    
    if(bitmapOffset + getStoredBitmapSize() > pageBytes.size()) throw std::invalid_argument("[Bitmap]: bitmap region exceeds page size");
}

bool Bitmap::readBit(std::size_t index) const
{
    std::byte& bitByte = getBitByte(index);
    std::size_t relativeIndex = index % 8;

    // 0x01 = 00000001
    // So we are shifting so that the bit we want is LSB, then zero'ing out rest of byte... this returns either 1 or 0
    return static_cast<bool>((bitByte >> relativeIndex) & std::byte{0x01});

}

bool Bitmap::setBit(std::size_t index)
{
    std::byte& bitByte = getBitByte(index);
    std::size_t relativeIndex = index % 8;

    bool prevVal = readBit(index);
    // With the shift, the only bit set to 1 will be the position that we want to change
    // So then ORing will set it to 1 always
    bitByte |= (std::byte{0x01} << relativeIndex);

    return prevVal;

}

bool Bitmap::clearBit(std::size_t index)
{
    std::byte& bitByte = getBitByte(index);
    std::size_t relativeIndex = index % 8;

    bool prevVal = readBit(index);

    // With the shift, the only bit set to 1 will be the position that we want to change
    // Then inverting it will make it so only the posiiton we want to change will be 0
    // So then anding will cause that bit position to be set to 0 while the rest preserved
    bitByte &= ~(std::byte{0x01} << relativeIndex);

    return prevVal;
}
        
std::size_t Bitmap::findFirstZeroBitIndex(std::size_t index) const
{
    if(!isValidIndex(index)) throw std::invalid_argument("[findFirstZeroBitIndex]: supplied index not valid");

    std::vector<std::byte> bitmapBytes(bitmapSlotCount, std::byte{0});
    std::size_t bitmapSize = getStoredBitmapSize();
    std::size_t offset = bitmapOffset;
    copyNBytesSrcOffset(bitmapBytes, pageBytes, offset, bitmapSize);

    std::size_t indexesChecked = 0;
    while(indexesChecked < bitmapSlotCount)
    {
        if(readBit(index) == 0) return index;
        index = (index + 1) % bitmapSize;
        ++indexesChecked;
    }

    return -1;
}