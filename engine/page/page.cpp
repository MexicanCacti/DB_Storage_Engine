#include "page.hpp"

Page::Page(const PageID& pageID, const std::vector<std::byte>& dataBytes)
{
    if(dataBytes.size() != pageByteLength) throw std::invalid_argument("[Page]: dataBytes doesn't match page length");
    pageBytes = std::vector<std::byte>(pageByteLength, std::byte{0});
    std::size_t offset = 0;
    copyBytes(pageBytes, dataBytes, offset);
}