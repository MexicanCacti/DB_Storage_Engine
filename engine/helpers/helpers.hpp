#pragma once

#include <vector>
#include <cstddef>
#include <algorithm>
#include <stdexcept>

// Appends all bytes of src to end of dest
void appendBytes(std::vector<std::byte>& dest, const std::vector<std::byte>& src);

// Copies all bytes of src to dest begin + offset
void copyBytes(std::vector<std::byte>& dest, const std::vector<std::byte>& src, std::size_t& offset);

// Copies n bytes of src to dest begin + offset
void copyNBytes(std::vector<std::byte>& dest, const std::vector<std::byte>& src, std::size_t& offset, std::size_t n);

// Copies all bytes starting from src + offset to dest
void copyBytesSrcOffset(std::vector<std::byte>& dest, const std::vector<std::byte>& src, std::size_t& offset);

// Copies n bytes starting from src + offset to dest
void copyNBytesSrcOffset(std::vector<std::byte>& dest, const std::vector<std::byte>& src, std::size_t& offset, std::size_t n);