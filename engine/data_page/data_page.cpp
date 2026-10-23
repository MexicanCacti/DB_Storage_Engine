#include "data_page.hpp"

std::size_t DataPage::getDeletionBitmapOffset() const
{
    std::size_t allocationBitmapOffset = getAllocationBitmapOffset();
    std::size_t bitmapSize = PageHeader::calcBitmapSize(getSlotCapacity());
    return allocationBitmapOffset + bitmapSize;
}

Bitmap DataPage::getBitmap(std::size_t type)
{
    std::vector<std::byte>& pageBytes = page.getPageBytes();
    PageHeader decodedPageHeader = PageHeader::decode(pageBytes);
    std::int16_t slotCount = decodedPageHeader.getSlotCapacity();
    if(type == 0) return Bitmap(page.getPageBytes(), getAllocationBitmapOffset(), slotCount);
    else if(type == 1) return Bitmap(page.getPageBytes(), getDeletionBitmapOffset(), slotCount);

    throw std::invalid_argument("[getBitmap]: bad type supplied");
}

DataPage::DataPage(Page& page)
    : page(page)
{

}

void DataPage::writePageHeader()
{
    PageHeader pageHeader = getPageHeader();
    std::vector<std::byte>& pageBytes = page.getPageBytes();
    std::size_t offset = 0;

    copyBytes(pageBytes, pageHeader.encode(), offset);

    if(offset != HEADER_BYTE_LENGTH) throw std::out_of_range("[writePageHeader]: bad write, offset != header byte length");
}

void DataPage::Initialize(std::int16_t tupleWidth)
{
    std::vector<std::byte>& pageBytes = page.getPageBytes();
    PageHeader pageHeader(tupleWidth); // constructor will check if tupleWidth arg is invalid
    std::size_t offset = 0;
    copyBytes(pageBytes, pageHeader.encode(), offset); // will throw error if pageHeader doesn't fit

    if(offset != HEADER_BYTE_LENGTH) throw std::out_of_range("[writePageHeader]: bad write, offset != header byte length");
}

// Verifies that the page DataPage is referencing is valid... expectedTupleWidth comes from table schema (tableHeader)
void DataPage::Open(std::int16_t expectedTupleWidth)
{
    std::vector<std::byte>& headerBytes(HEADER_BYTE_LENGTH, std::byte{0});
    std::vector<std::byte>& pageBytes = page.getPageBytes();

    PageHeader decodedPageHeader = PageHeader::decode(pageBytes);
    PageHeader expectedPageHeader(expectedTupleWidth); // Use to validate: Slot count, format, version, tupleWidth

    if(!PageHeader::similarFormats(decodedPageHeader, expectedPageHeader)) throw std::invalid_argument("[Open]: Decoded page header's format does not match expected format");

    std::int16_t slotCount = decodedPageHeader.getSlotCapacity();

    Bitmap allocationBitmap = getBitmap(0);
    Bitmap deletionBitmap = getBitmap(1);

    std::size_t allocatedCount = 0;
    for(std::size_t index = 0 ; index < slotCount; ++index)
    {
        if(allocationBitmap.readBit(index) == 1) ++allocatedCount;
        if(allocationBitmap.readBit(index) == 0 && deletionBitmap.readBit == 1) throw std::runtime_error("[Open]: invalid bitmap state");
    }

    if(allocatedCount != PageHeader::getAllocatedSlotCount(pageBytes)) throw std::runtime_error("[Open]: invalid bitmap state");
}

/*
        H + 2 x B(n) + i X R
        H = header size
        B(n) = bitmap size
        i = ith slot to index
        R = tuple width
*/
std::size_t DataPage::getSlotOffset(std::size_t slotIndex) const
{
    Bitmap allocationBitmap = getBitmap(0);

    if(!bitmap.readBit(slotIndex))

    std::size_t slotOffset = HEADER_BYTE_LENGTH + 2 * decodedPageHeader.calcBitmapSize(decodedPageHeader.getSlotCapacity()) + slotIndex * decodedPageHeader.tupleWidth;
    if(slotOffset > pageBytes) throw std::invalid_argument
}

bool DataPage::isAllocated(std::size_t slotIndex) const
{
    Bitmap allocationBitmap = getBitmap(0);

    return allocationBitmap.read(slotIndex);
}

bool DataPage::isDeleted(std::size_t slotIndex) const
{
    Bitmap deletionBitmap = getBitmap(0);

    return deletionBitmap.read(slotIndex);
}
bool DataPage::hasFreeSlot() const
{
    Bitmap allocationBitmap = getBitmap(0);
    return allocationBitmap.findFirstZeroBitIndex(0) != -1;
}