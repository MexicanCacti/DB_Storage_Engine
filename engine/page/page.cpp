#include "page.hpp"

Page::Page(const PageID& pageID, const std::vector<std::byte>& dataBytes)
{
    if(dataBytes.size() != PAGE_BYTE_LENGTH) throw std::invalid_argument("[Page]: dataBytes doesn't match page length");
    pageBytes = std::vector<std::byte>(PAGE_BYTE_LENGTH, std::byte{0});
    std::size_t offset = 0;
    copyBytes(pageBytes, dataBytes, offset);
}