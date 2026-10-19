#include "data_page_header.hpp"

/*
    B(n) = 8 x ceil(n/64)
    H + 2 x B(n) + n x R... H = header size, n = # slot cap, R = tupleWidth
*/

PageHeader::PageHeader(std::int64_t lsn, std::int16_t fv, std::int16_t fid, std::int16_t tw, std::int16_t sCap, int16_t allocSlot)
    : logSequenceNumber(lsn), formatVersion(fv), formatID(fid), tupleWidth(tw), slotCapacity(sCap), allocatedSlotCount(allocSlot)
{

}

PageHeader::PageHeader(std::int16_t tupleWidth)
{
    this->tupleWidth = tupleWidth;
    slotCapacity = calcSlotCapacity(tupleWidth);
    allocatedSlotCount = 0;
}

void PageHeader::setAllocatedSlotCount(std::int16_t allocatedCount)
{
    if(allocatedCount < 0 || allocatedCount > slotCapacity) throw std::invalid_argument("[setAllocatedSlotCount]: bad allocatedCount");
    allocatedSlotCount = allocatedCount;
}

std::size_t PageHeader::calcBitmapSize(std::size_t slotCapacity)
{
    return 8 * std::ceil( (slotCapacity + 63) / 64);
}

    // H + 2 * B(n) + n * R <= 4096, H = header size, R = Tuple Width
            // B(n) = 8 * ceil(n/64), n = # of slots
std::size_t PageHeader::calcSlotCapacity(std::int16_t tupleWidth)
{
    if(tupleWidth <= 0) throw std::invalid_argument("[calcSlotCapacity]: tuple width less than or equal to 0");
    std::size_t potentialCapacity = 1;
    for(potentialCapacity; potentialCapacity <= (PAGE_BYTE_LENGTH - HEADER_BYTE_LENGTH) / tupleWidth ; ++potentialCapacity)
    {
        std::size_t bitMapSize = calcBitmapSize(potentialCapacity);
        std::size_t totalPageSizeWithSlotCapacity = HEADER_BYTE_LENGTH + (2 * bitMapSize) + (potentialCapacity * tupleWidth);
        if(totalPageSizeWithSlotCapacity > PAGE_BYTE_LENGTH) break;
    }

    if(potentialCapacity == 1) throw std::invalid_argument("[calcSlotCapacity]: Tuple cannot fit in page");

    return potentialCapacity - 1;
}

std::int64_t PageHeader::getLogSequenceNumber(const std::vector<std::byte>& headerBytes)
{
    std::size_t offset = 0;
    std::vector<std::byte> logSequenceBytes(getIntTypeSize(IntType::INT64), std::byte{0});
    copyNBytesSrcOffset(logSequenceBytes, headerBytes, offset, getIntTypeSize(IntType::INT64));
    return std::get<std::int64_t>(decodeIntVariant(logSequenceBytes, IntType::INT64));
}

std::int16_t PageHeader::getFormatVersion(const std::vector<std::byte>& headerBytes)
{
    std::size_t offset = 8;
    std::vector<std::byte> formatVersionBytes(getIntTypeSize(IntType::INT16), std::byte{0});
    copyNBytesSrcOffset(formatVersionBytes, headerBytes, offset, getIntTypeSize(IntType::INT16));
    return std::get<std::int16_t>(decodeIntVariant(formatVersionBytes, IntType::INT16));
}

std::int16_t PageHeader::getFormatID(const std::vector<std::byte>& headerBytes)
{
    std::size_t offset = 10;
    std::vector<std::byte> formatIDBytes(getIntTypeSize(IntType::INT16), std::byte{0});
    copyNBytesSrcOffset(formatIDBytes, headerBytes, offset, getIntTypeSize(IntType::INT16));
    return std::get<std::int16_t>(decodeIntVariant(formatIDBytes, IntType::INT16));
}

std::int16_t PageHeader::getTupleWidth(const std::vector<std::byte>& headerBytes)
{
    std::size_t offset = 12;
    std::vector<std::byte> tupleWidthBytes(getIntTypeSize(IntType::INT16), std::byte{0});
    copyNBytesSrcOffset(tupleWidthBytes, headerBytes, offset, getIntTypeSize(IntType::INT16));
    return std::get<std::int16_t>(decodeIntVariant(tupleWidthBytes, IntType::INT16));
}

std::int16_t PageHeader::getSlotCapacity(const std::vector<std::byte>& headerBytes)
{
    std::size_t offset = 14;
    std::vector<std::byte> slotCapacityBytes(getIntTypeSize(IntType::INT16), std::byte{0});
    copyNBytesSrcOffset(slotCapacityBytes, headerBytes, offset, getIntTypeSize(IntType::INT16));
    return std::get<std::int16_t>(decodeIntVariant(slotCapacityBytes, IntType::INT16));
}

std::int16_t PageHeader::getAllocatedSlotCount(const std::vector<std::byte>& headerBytes)
{
    std::size_t offset = 16;
    std::vector<std::byte> allocatedSlotCountBytes(getIntTypeSize(IntType::INT16), std::byte{0});
    copyNBytesSrcOffset(allocatedSlotCountBytes, headerBytes, offset, getIntTypeSize(IntType::INT16));
    return std::get<std::int16_t>(decodeIntVariant(allocatedSlotCountBytes, IntType::INT16));
}

PageHeader PageHeader::decode(const std::vector<std::byte>& headerBytes)
{
    if(headerBytes.size() != HEADER_BYTE_LENGTH) throw std::invalid_argument("[decode]: headerBytes not expected length");

    std::int64_t logSequenceNumber = getLogSequenceNumber(headerBytes);
    std::int16_t formatVersion = getFormatVersion(headerBytes);
    std::int16_t formatID = getFormatID(headerBytes);
    std::int16_t tupleWidth = getTupleWidth(headerBytes);
    std::int16_t slotCapacity = getSlotCapacity(headerBytes);
    std::int16_t allocatedSlotCount = getAllocatedSlotCount(headerBytes);

    return PageHeader(
        logSequenceNumber,
        formatVersion,
        formatID,
        tupleWidth,
        slotCapacity,
        allocatedSlotCount
    );
}

std::vector<std::byte> PageHeader::encode() const
{
    std::vector<std::byte> pageHeaderBytes(HEADER_BYTE_LENGTH, std::byte{0});
    std::size_t offset = 0;
    
    copyBytes(pageHeaderBytes, encodeIntVariant(logSequenceNumber), offset);
    copyBytes(pageHeaderBytes, encodeIntVariant(formatVersion), offset);
    copyBytes(pageHeaderBytes, encodeIntVariant(formatID), offset);
    copyBytes(pageHeaderBytes, encodeIntVariant(tupleWidth), offset);
    copyBytes(pageHeaderBytes, encodeIntVariant(slotCapacity), offset);
    copyBytes(pageHeaderBytes, encodeIntVariant(allocatedSlotCount), offset);

    return pageHeaderBytes;
}