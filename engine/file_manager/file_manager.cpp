#include "file_manager.hpp"


FileManager::FileManager(const std::filesystem::path& directory) : dataDirectory(directory)
{
    std::filesystem::create_directories(dataDirectory);
}

void FileManager::createFile(const std::string& fileName)
{
    std::filesystem::path file(fileName);
    std::filesystem::path filePath = dataDirectory / file;

    if(fileExists(fileName)) throw std::invalid_argument("[createFile]: File already exists");

    std::fstream fileCreate;
    fileCreate.open(filePath, std::fstream::out | std::fstream::binary);

    if(!fileExists(fileName)) throw std::runtime_error("[createFile]: Failed to create file");
}

bool FileManager::fileExists(const std::string& fileName)
{
    std::filesystem::path file(fileName);
    std::filesystem::path filePath = dataDirectory / file;
    return std::filesystem::is_regular_file(filePath);
}

void FileManager::readPage(const std::string& fileName, std::int64_t pageNumber, std::vector<std::byte>& pageBytes)
{
    if(pageNumber < 0) throw std::invalid_argument("[readPage]: Invalid page number");
    if(pageBytes.size() != PAGE_BYTE_LENGTH) throw std::invalid_argument("[readPage]: Invalid page bytes size");
    if(!fileExists(fileName)) throw std::invalid_argument("[readPage]: File does not exist");

    std::filesystem::path file(fileName);
    std::filesystem::path filePath = dataDirectory / file;

    std::fstream fileInput;
    fileInput.open(filePath, std::fstream::in | std::fstream::binary);

    if(!fileInput.is_open()) throw std::runtime_error("[readPage]: Failed to open file");

    std::size_t readOffset = pageNumber * PAGE_BYTE_LENGTH;
    std::size_t fileSize = std::filesystem::file_size(filePath);
    std::size_t pageCount = fileSize / PAGE_BYTE_LENGTH;

    if(pageNumber >= pageCount) throw std::out_of_range("[readPage]: Page does not exist");

    if(readOffset > fileSize || fileSize - readOffset < PAGE_BYTE_LENGTH) throw std::out_of_range("[readPage]: Page does not exist");

    // put input stream to start of page
    fileInput.seekg(readOffset, std::fstream::beg);

    if(!fileInput) throw std::runtime_error("[readPage]: Failed to seek");

    std::vector<std::byte> readBuffer(PAGE_BYTE_LENGTH, std::byte{0});

    fileInput.read(reinterpret_cast<char*>(readBuffer.data()), PAGE_BYTE_LENGTH);
    if(!fileInput) throw std::runtime_error("[readPage]: Failed to read page");

    fileInput.close();

    copyBytes(pageBytes, readBuffer);


}

void FileManager::writePage(const std::string& fileName, std::int64_t pageNumber, const std::vector<std::byte>& pageBytes)
{
    if(pageNumber < 0) throw std::invalid_argument("[writePage]: Invalid page number");
    if(pageBytes.size() != PAGE_BYTE_LENGTH) throw std::invalid_argument("[writePage]: Invalid page bytes size");
    if(!fileExists(fileName)) throw std::invalid_argument("[writePage]: File does not exist");

    std::filesystem::path file(fileName);
    std::filesystem::path filePath = dataDirectory / file;

    std::fstream fileOutput;
    fileOutput.open(filePath, std::fstream::out | std::fstream::in | std::fstream::binary);
    if(!fileOutput.is_open()) throw std::runtime_error("[writePage]: Failed to open file");

    std::size_t writeOffset = pageNumber * PAGE_BYTE_LENGTH;
    fileOutput.seekp(writeOffset, std::fstream::beg);

    if(!fileOutput) throw std::runtime_error("[writePage]: Failed to seek");

    fileOutput.write(reinterpret_cast<const char*>(pageBytes.data()), PAGE_BYTE_LENGTH);
    if(!fileOutput) throw std::runtime_error("[writePage]: Failed to write page");
    fileOutput.close();
}

std::int64_t FileManager::getPageCount(std::string& fileName) const
{
    std::filesystem::path file(fileName);
    std::filesystem::path filePath = dataDirectory / file;

    std::size_t fileSize = std::filesystem::file_size(filePath);

    if(fileSize % PAGE_BYTE_LENGTH != 0) throw std::runtime_error("[getPageCount]: Invalid file size");

    return static_cast<std::int64_t>(fileSize / PAGE_BYTE_LENGTH);
}