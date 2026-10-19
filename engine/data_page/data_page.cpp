#include "data_page.hpp"

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

    // Check if all the slot states are valid
    // Check if the allocated slot state # = allocatedSlotCount
}