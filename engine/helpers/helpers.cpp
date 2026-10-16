#include "helpers.hpp"

void appendBytes(std::vector<std::byte>& dest, const std::vector<std::byte>& src)
{
    // if the same arg, need to create a temp vector in order to append
    if(&dest == &src)
    {
        std::vector<std::byte> tempSrc(src);
        dest.insert(dest.end(), tempSrc.begin(), tempSrc.end());
        return;
    }

    dest.insert(dest.end(), src.begin(), src.end());
}

void copyBytes(std::vector<std::byte>& dest, const std::vector<std::byte>& src, std::size_t& offset)
{
    if(&dest == &src) throw std::invalid_argument("[copyBytes]: dest & src are the same argument");

    if(offset > dest.size() || src.size() > dest.size() - offset) throw std::out_of_range("[copyBytes]: src copy exceeds dest bounds");
    std::copy(src.begin(), src.end(), dest.begin() + offset);
    offset += src.size();
}

void copyNBytes(std::vector<std::byte>& dest, const std::vector<std::byte>& src, std::size_t& offset, std::size_t n)
{
    if(&dest == &src) throw std::invalid_argument("[copyNBytes]: dest & src are the same argument");

    if(n > src.size()) throw std::out_of_range("[copyNBytes]: n exceeds src size");

    if(offset > dest.size() || n > dest.size() - offset) throw std::out_of_range("[copyNBytes]: src copy exceeds dest bounds");

    std::copy(src.begin(), src.begin() + n, dest.begin() + offset);
    offset += n;
}


void copyBytesSrcOffset(std::vector<std::byte>& dest, const std::vector<std::byte>& src, std::size_t& offset)
{
    if(&dest == &src) throw std::invalid_argument("[copyBytesSrcOffset]: dest & src are the same argument");

    if(offset > src.size()) throw std::out_of_range("[copyBytesSrcOffset]: src offset exceeds src bounds");

    std::size_t copyByteCount = src.size() - offset;

    if(copyByteCount > dest.size()) throw std::out_of_range("[copyBytesSrcOffset]: dest can't fit src");

    std::copy(src.begin() + offset, src.end(), dest.begin());
    offset += copyByteCount;
}

void copyNBytesSrcOffset(std::vector<std::byte>& dest, const std::vector<std::byte>& src, std::size_t& offset, std::size_t n)
{
    if(&dest == &src) throw std::invalid_argument("[copyNBytesSrcOffset]: dest & src are the same argument");

    if(offset > src.size() || n > src.size() - offset) throw std::out_of_range("[copyNBytesSrcOffset]: Not enough bytes in src");

    if(n > dest.size()) throw std::out_of_range("[copyNBytesSrcOffset]: dest cant fit src");

    std::copy(src.begin() + offset, src.begin() + offset + n, dest.begin());

    offset += n;
}