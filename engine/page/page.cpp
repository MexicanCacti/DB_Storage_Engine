#include "page.hpp"

 Page::Page()
 {
    pageID = {-1, -1};
    pageBytes = std::vector<std::byte>(PAGE_BYTE_LENGTH, std::byte{0});
 }

Page::Page(const PageID& pageID)
    : pageID(pageID) 
{
    pageBytes = std::vector<std::byte>(PAGE_BYTE_LENGTH, std::byte{0});
}

Page::Page(const PageID& pageID, const std::vector<std::byte>& dataBytes)
    : pageID(pageID), pageBytes(dataBytes)
{
    if(dataBytes.size() != PAGE_BYTE_LENGTH) throw std::invalid_argument("[Page]: dataBytes doesn't match page length");
}