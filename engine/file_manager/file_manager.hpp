#pragma once
#include <unordered_map>
#include "page.hpp"
#include "data_page.hpp"

#include <fstream>
#include <filesystem>
#include <ios>

class FileManager {
    private:
        std::filesystem::path dataDirectory;
        
    public: 
        FileManager(const std::filesystem::path& directory);
        void createFile(const std::string& fileName);
        bool fileExists(const std::string& fileName);

        void readPage(const std::string& fileName, std::int64_t pageNumber, std::vector<std::byte>& pageBytes);
        void writePage(const std::string& fileName, std::int64_t pageNumber, const std::vector<std::byte>& pageBytes);

        std::int64_t getPageCount(std::string& fileName) const;
};