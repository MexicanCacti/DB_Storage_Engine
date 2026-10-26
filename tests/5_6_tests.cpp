#include "buffer_pool.hpp"
#include "file_manager.hpp"
#include "page.hpp"
#include <iostream>
#include <string>
#include <unordered_map>

std::string directoryString = "./storage";
std::string tableName = "students";
std::size_t BUFFER_FRAME_COUNT = 3;

std::size_t cacheHits = 0;

std::unordered_map<std::size_t, std::string> frameToPageName;

void displayAnalytics(std::string fetched, BufferPoolAnalytics analytics, BufferPoolAnalytics previousAnalytics)
{
    std::string evictedPage = "NONE";
    std::string hitOrMiss = "Miss";

    std::list<std::size_t> oldLRUList = previousAnalytics.LRUList;
    std::list<std::size_t> newLRUList = analytics.LRUList;
    if(newLRUList.empty()) return;
    bool fetchedPageExists = false;

    for(auto& [frameID, pageName] : frameToPageName)
    {

        if(pageName == fetched)
        {
            fetchedPageExists = true;
            break;
        }
    }

    if(fetchedPageExists)
    {
        hitOrMiss = "HIT";
        ++cacheHits;
    }
    else
    {
        std::size_t fetchedFrame = newLRUList.back();
        
        if(frameToPageName.find(fetchedFrame) != frameToPageName.end()) evictedPage = frameToPageName.at(fetchedFrame);

        frameToPageName[fetchedFrame] = fetched;
    }

    std::cout << fetched << " | " << hitOrMiss << " | " << evictedPage << " | ";
    for(auto it = newLRUList.rbegin(); it != newLRUList.rend(); ++it)
    {
        std::size_t frame = *it;
        std::cout << frameToPageName.at(frame) << " ";
    }
    std::cout << "\n";
}

int main()
{   


    BufferPoolAnalytics previousAnalytics;
    BufferPoolAnalytics analytics;
    // A, B, C, D
    std::vector<PageID> testPageID = { {0, 0}, {1, 0}, {2, 0}, {3, 0} };

    std::unordered_map<std::int64_t, std::string> tableIDToFileMap;
    tableIDToFileMap[0] = "A";
    tableIDToFileMap[1] = "B";
    tableIDToFileMap[2] = "C";
    tableIDToFileMap[3] = "D";

    std::filesystem::path directoryPath = directoryString;
    FileManager fileManager(directoryPath);
    try{
        fileManager.createFile("A");
        fileManager.createFile("B");
        fileManager.createFile("C");
        fileManager.createFile("D");
    }
    catch(...)
    {
        
    }
    std::vector<std::byte> emptyPage(PAGE_BYTE_LENGTH, std::byte{0});
    std::fstream fileWrite(directoryPath / "A", std::fstream::out | std::fstream::binary);
    fileWrite.write(reinterpret_cast<const char*>(emptyPage.data()), PAGE_BYTE_LENGTH);
    fileWrite.close();


    fileWrite.open(directoryPath / "B", std::fstream::out | std::fstream::binary);
    fileWrite.write(reinterpret_cast<const char*>(emptyPage.data()), PAGE_BYTE_LENGTH);
    fileWrite.close();


    fileWrite.open(directoryPath / "C", std::fstream::out | std::fstream::binary);
    fileWrite.write(reinterpret_cast<const char*>(emptyPage.data()), PAGE_BYTE_LENGTH);
    fileWrite.close();


    fileWrite.open(directoryPath / "D", std::fstream::out | std::fstream::binary);
    fileWrite.write(reinterpret_cast<const char*>(emptyPage.data()), PAGE_BYTE_LENGTH);
    fileWrite.close();


    BufferPool bufferPool(fileManager, tableIDToFileMap , BUFFER_FRAME_COUNT);

    try{
        previousAnalytics = bufferPool.getAnalytics();
        bufferPool.fetchPage(testPageID[0]);
        bufferPool.unpinPage(testPageID[0], false);
        analytics = bufferPool.getAnalytics();
        displayAnalytics("A", analytics, previousAnalytics);


        previousAnalytics = analytics;
        bufferPool.fetchPage(testPageID[1]);
        bufferPool.unpinPage(testPageID[1], false);
        analytics = bufferPool.getAnalytics();
        displayAnalytics("B", analytics, previousAnalytics);

        previousAnalytics = analytics;
        bufferPool.fetchPage(testPageID[2]);
        bufferPool.unpinPage(testPageID[2], false);
        analytics = bufferPool.getAnalytics();
        displayAnalytics("C", analytics, previousAnalytics);

        previousAnalytics = analytics;
        bufferPool.fetchPage(testPageID[0]);
        bufferPool.unpinPage(testPageID[0], false);
        analytics = bufferPool.getAnalytics();
        displayAnalytics("A", analytics, previousAnalytics);

        previousAnalytics = analytics;
        bufferPool.fetchPage(testPageID[3]);
        bufferPool.unpinPage(testPageID[3], false);
        analytics = bufferPool.getAnalytics();
        displayAnalytics("D", analytics, previousAnalytics);

        previousAnalytics = analytics;
        bufferPool.fetchPage(testPageID[1]);
        bufferPool.unpinPage(testPageID[1], false);
        analytics = bufferPool.getAnalytics();
        displayAnalytics("B", analytics, previousAnalytics);

        std::cout << "Disk Reads: " << analytics.readCount << "\n";
        std::cout << "Cache Hits: " << cacheHits << "\n";
        std::cout << "Disk Writes: " << analytics.writeCount << "\n";
    }
    catch(std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }



    return 0;




}