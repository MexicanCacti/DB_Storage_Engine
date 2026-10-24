#include "data_page.hpp"

void DataPage::writePageHeader(const std::vector<std::byte>& encodedPageHeader)
{
    if(encodedPageHeader.size() != HEADER_BYTE_LENGTH) throw std::invalid_argument("[writePageHeader]: bad decodedPageHeader size");
    copyBytes(page.getPageBytes(), encodedPageHeader);
}

PageHeader DataPage::getPageHeader() const
{
    std::vector<std::byte> headerBytes(HEADER_BYTE_LENGTH, std::byte{0});
    copyNBytes(headerBytes, page.getPageBytesConst(), HEADER_BYTE_LENGTH);
    return PageHeader::decode(headerBytes);
}

std::size_t DataPage::getDeletionBitmapOffset() const
{
    std::size_t allocationBitmapOffset = getAllocationBitmapOffset();
    PageHeader decodedPageHeader = getPageHeader();
    
    std::size_t bitmapSize = PageHeader::calcBitmapSize(decodedPageHeader.getSlotCapacity());
    return allocationBitmapOffset + bitmapSize;
}

Bitmap DataPage::getBitmap(std::size_t type) const
{
    PageHeader decodedPageHeader = getPageHeader();
    std::int16_t slotCount = decodedPageHeader.getSlotCapacity();
    if(type == 0) return Bitmap(page.getPageBytes(), getAllocationBitmapOffset(), slotCount);
    else if(type == 1) return Bitmap(page.getPageBytes(), getDeletionBitmapOffset(), slotCount);

    throw std::invalid_argument("[getBitmap]: bad type supplied");
}

// 0 0
std::size_t DataPage::findInsertIndex() const
{
    Bitmap allocationBitmap = getBitmap(0);
    Bitmap deletionBitmap = getBitmap(1);
    PageHeader decodedPageHeader = getPageHeader();
    std::int16_t slotCount = decodedPageHeader.getSlotCapacity();

    for(std::size_t index = 0 ; index < slotCount ; ++index)
    {
        bool allocBit = allocationBitmap.readBit(index);
        bool delBit = deletionBitmap.readBit(index);
        if( (!allocBit && !delBit) ) return index;
    }

    return -1;
}

// 1 0
bool DataPage::isValidReadSlotIndex(std::size_t slotIndex) const
{
    Bitmap allocationBitmap = getBitmap(0);
    Bitmap deletionBitmap = getBitmap(1);

    bool allocBit = allocationBitmap.readBit(slotIndex);
    bool delBit = deletionBitmap.readBit(slotIndex);

    return allocBit && !delBit;
}

DataPage::DataPage(Page& page)
    : page(page)
{
    if(page.getPageBytes().size() != PAGE_BYTE_LENGTH) throw std::invalid_argument("[DataPage]: invalid page byte length");
}

// Check, could be funky
void DataPage::Initialize(std::int16_t tupleWidth)
{
    std::vector<std::byte>& pageBytes = page.getPageBytes();
    if(pageBytes.size() != PAGE_BYTE_LENGTH) throw std::invalid_argument("[Initialize]: invalid page byte length");

    PageHeader pageHeader(tupleWidth); // constructor will check if tupleWidth arg is invalid
    std::size_t offset = 0;
    copyBytes(pageBytes, pageHeader.encode(), offset); // will throw error if pageHeader doesn't fit

    if(offset != HEADER_BYTE_LENGTH) throw std::out_of_range("[writePageHeader]: bad write, offset != header byte length");
    std::vector<std::byte> zeroBytes(PAGE_BYTE_LENGTH - HEADER_BYTE_LENGTH, std::byte{0});
    std::size_t pageStartOffset = 24;
    copyBytes(pageBytes, zeroBytes, pageStartOffset); // zero out non-header

    std::size_t slotCapacity = pageHeader.getSlotCapacity();
}

// Verifies that the page DataPage is referencing is valid... expectedTupleWidth comes from table schema (tableHeader)
void DataPage::Open(std::int16_t expectedTupleWidth)
{
    if(page.getPageBytes().size() != PAGE_BYTE_LENGTH) throw std::invalid_argument("[Initialize]: invalid page byte length");
    PageHeader decodedPageHeader = getPageHeader();
    PageHeader expectedPageHeader(expectedTupleWidth); // Use to validate: Slot count, format, version, tupleWidth

    if(!PageHeader::similarFormats(decodedPageHeader, expectedPageHeader)) throw std::invalid_argument("[Open]: Decoded page header's format does not match expected format");

    std::int16_t slotCount = decodedPageHeader.getSlotCapacity();

    Bitmap allocationBitmap = getBitmap(0);
    Bitmap deletionBitmap = getBitmap(1);

    std::size_t allocatedCount = 0;
    for(std::size_t index = 0 ; index < slotCount; ++index)
    {
        if(allocationBitmap.readBit(index) == 1) ++allocatedCount;
        if(allocationBitmap.readBit(index) == 0 && deletionBitmap.readBit(index) == 1) throw std::runtime_error("[Open]: invalid bitmap state");
    }

    if(allocatedCount != decodedPageHeader.getAllocatedSlotCount()) throw std::runtime_error("[Open]: invalid bitmap state");
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

    PageHeader decodedPageHeader = getPageHeader();
    std::size_t slotCapacity = decodedPageHeader.getSlotCapacity();
    std::size_t tupleWidth = decodedPageHeader.getTupleWidth();

    if(slotIndex >= slotCapacity) throw std::invalid_argument("[getSlotOffset]: invalid slot index");

    std::size_t bitmapSize = PageHeader::calcBitmapSize(slotCapacity);


    std::size_t slotOffset = HEADER_BYTE_LENGTH + 2 * bitmapSize + slotIndex * tupleWidth;

    if(slotOffset > PAGE_BYTE_LENGTH || tupleWidth > PAGE_BYTE_LENGTH - slotOffset) throw std::invalid_argument("[getSlotOffset]: tuple exceeds page limit");

    return slotOffset;
}

bool DataPage::isAllocated(std::size_t slotIndex) const
{
    Bitmap allocationBitmap = getBitmap(0);

    return allocationBitmap.readBit(slotIndex);
}

bool DataPage::isDeleted(std::size_t slotIndex) const
{
    Bitmap deletionBitmap = getBitmap(1);

    return deletionBitmap.readBit(slotIndex);
}

bool DataPage::hasFreeSlot() const
{
    Bitmap allocationBitmap = getBitmap(0);
    Bitmap deletionBitmap = getBitmap(1);
    PageHeader decodedPageHeader = getPageHeader();
    std::int16_t slotCount = decodedPageHeader.getSlotCapacity();

    for(std::size_t index = 0 ; index < slotCount ; ++index)
    {
        bool allocBit = allocationBitmap.readBit(index);
        bool delBit = deletionBitmap.readBit(index);
        if( (!allocBit && !delBit) ) return true;
    }

    return false;
}

RID DataPage::insertTuple(const std::vector<std::byte>& encodedTuple)
{
    PageHeader decodedPageHeader = getPageHeader();

    if(encodedTuple.size() != decodedPageHeader.getTupleWidth()) throw std::invalid_argument("[insertTuple]: encodedTuple not expected size");
    PageID pageID = page.getPageID();
    std::size_t insertIndex = findInsertIndex();
    if(insertIndex == -1) throw std::overflow_error("[insertTuple]: No more space in page for another tuple");

    std::vector<std::byte>& pageBytes = page.getPageBytes();
    std::size_t slotOffset = getSlotOffset(insertIndex);

    copyBytes(pageBytes, encodedTuple, slotOffset);
    
    Bitmap allocationBitmap = getBitmap(0);
    allocationBitmap.setBit(insertIndex);
    
    decodedPageHeader.setAllocatedSlotCount(decodedPageHeader.getAllocatedSlotCount() + 1);
    writePageHeader(decodedPageHeader.encode());

    return RID{pageID, static_cast<int16_t>(insertIndex)};

    // find first empty slot
    // insert ( modify page bytes)
    // increment allocated slot count in header
    // set allocation to be 1
    // craft RID
    // return RID
}

std::vector<std::byte> DataPage::readTuple(std::size_t slotIndex) const
{
    if(!isValidReadSlotIndex(slotIndex)) throw std::invalid_argument("[readTuple]: slot index invalid");
    std::size_t slotOffset = getSlotOffset(slotIndex);

    PageHeader decodedPageHeader = getPageHeader();
    std::vector<std::byte> encodedTuple(decodedPageHeader.getTupleWidth(), std::byte{0});
    copyNBytesSrcOffset(encodedTuple, page.getPageBytesConst(), slotOffset, decodedPageHeader.getTupleWidth());

    return encodedTuple;
    // get allocation bitmap & deletion bitmap
    // check if slot valid
    // get slot offset
    // read bytes into a vector equivalent to how large a tuple should be
    // return those bytes
}

RID DataPage::updateTuple(std::size_t slotIndex, const std::vector<std::byte>& encodedTuple)
{
    if(!isValidReadSlotIndex(slotIndex)) throw std::invalid_argument("[updateTuple]: slot index invalid");
    PageHeader decodedPageHeader = getPageHeader();
    if(encodedTuple.size() != decodedPageHeader.getTupleWidth()) throw std::invalid_argument("[updateTuple]: encodedTuple not expected size");

    std::size_t slotOffset = getSlotOffset(slotIndex);

    copyBytes(page.getPageBytes(), encodedTuple, slotOffset);

    PageID pageID = page.getPageID();
    return RID{pageID, static_cast<int16_t>(slotIndex)};
}

void DataPage::deleteTuple(std::size_t slotIndex) 
{
    if(!isValidReadSlotIndex(slotIndex)) throw std::invalid_argument("[readTuple]: slot index invalid");

    Bitmap deletionBitmap = getBitmap(1);
    deletionBitmap.setBit(slotIndex);

    // check if slot valid
    // set the deletion bit
    // no need to zero out the tuple bytes, logically deleted just write over them if needed
}