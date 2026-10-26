#pragma once
#include "page.hpp"
#include "file_manager.hpp"
#include <list>
#include <unordered_map>

const std::int8_t DEFAULT_BUFFER_FRAME_COUNT = 3;

struct BufferPoolAnalytics
{
    std::size_t evictCount;
    std::list<std::size_t> LRUList;
    std::size_t writeCount;
    std::size_t readCount;
};

// Buffer pool should own page!!
struct BufferFrame
{
    bool isOccupied = false;
    Page page;
    std::int16_t pinCount = 0;
    bool dirtyFlag = false;
};

class BufferPool
{
    private:
        std::size_t evictCount = 0;
        std::size_t writeCount = 0;
        std::size_t readCount = 0;

        std::vector<BufferFrame> framePool;
        std::unordered_map<PageID, std::size_t, PageID::PageIDHasher> frameLookup;
        std::list<std::size_t> LRUList;
        std::unordered_map<std::int64_t, std::string>& tableIDToFile;
        FileManager& fileManager;
        std::size_t findUnusedFrame() const;
        std::size_t findVictimFrame() const;
        void evictFrame(std::size_t frameIndex);
        void placeFrame(std::size_t frameIndex, const PageID& pageID);
        void updateLRUFrame(std::size_t frameIndex);
    public:
        BufferPool(FileManager& fileManager, std::unordered_map<std::int64_t, std::string>& tableIDToFile, std::int8_t frameCount);
        //  Get a page, will make room in frame pool if needed (gets victim) then increments pinCount
        Page& fetchPage(const PageID& pageID);

        void unpinPage(const PageID& pageID, bool is_dirty);

        void flushPage(const PageID& pageID);
        void flushAllPages();

        bool isPageInPool(const PageID& pageID) const;
        bool isPagePinned(const PageID& pageID) const;

        std::size_t getPoolCapacity() const {return framePool.size();}
        std::size_t getOccupiedFramesCount() const {return frameLookup.size();}
        BufferPoolAnalytics getAnalytics() const { return BufferPoolAnalytics{ evictCount, LRUList, writeCount, readCount }; }
};