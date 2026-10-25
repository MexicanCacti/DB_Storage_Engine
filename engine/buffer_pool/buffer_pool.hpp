#pragma once
#include "page.hpp"
#include "file_manager.hpp"
#include <list>
#include <unordered_map>

const std::int8_t DEFAULT_BUFFER_FRAME_COUNT = 3;

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
        std::vector<BufferFrame> framePool;
        std::unordered_map<PageID, std::size_t> frameLookup;
        std::list<std::size_t> LRUList;
        FileManager& fileManager;
        std::size_t findUnusedFrame() const;
        std::size_t findVictimFrame() const;
        void evictFrame(std::size_t frameIndex);
    public:
        BufferPool(std::int8_t frameCount);

        //  Get a page, will make room in frame pool if needed (gets victim) then increments pinCount
        Page& fetchPage(const PageID& pageID);

        void unpinPage(const PageID& pageID, bool is_dirty);

        void flushPage(const PageID& pageID);
        void flushAllPages();

        bool isPageInPool(const PageID& pageID) const;
        bool isPagePinned(const PageID& pageID) const;

        std::size_t getPoolCapacity() const {return framePool.size();}
        std::size_t getOccupiedFramesCount() const {return frameLookup.size();}
};