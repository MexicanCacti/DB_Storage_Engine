#include "buffer_pool.hpp"

void BufferPool::placeFrame(std::size_t frameIndex, const PageID& pageID)
{
    std::string fileName = tableIDToFile.at(pageID.tableID);
    std::int64_t pageNumber = pageID.pageNumber;

    Page bufferPage(pageID);

    fileManager.readPage(fileName, pageNumber, bufferPage.getPageBytes());

    BufferFrame& frame = framePool[frameIndex];
    frame.page = std::move(bufferPage);

    framePool[frameIndex].isOccupied = true;
    framePool[frameIndex].pinCount = 0;
    framePool[frameIndex].dirtyFlag = false;

    frameLookup[pageID] = frameIndex;
}

void BufferPool::updateLRUFrame(std::size_t frameIndex)
{
    LRUList.remove(frameIndex);
    LRUList.push_back(frameIndex);
}

std::size_t BufferPool::findUnusedFrame() const
{
    for(std::size_t frameIndex = 0 ; frameIndex < framePool.size(); ++frameIndex)
    {
        if(!framePool[frameIndex].isOccupied) return frameIndex;
    }

    return framePool.size(); // meaning no unusued frames
}
std::size_t BufferPool::findVictimFrame() const
{
    for(std::size_t frameIndex : LRUList)
    {
        const BufferFrame& frame = framePool[frameIndex];
        if(frame.isOccupied && frame.pinCount == 0) return frameIndex;
    }

    return framePool.size();
}
void BufferPool::evictFrame(std::size_t frameIndex)
{
    BufferFrame& frame = framePool[frameIndex];
    if(!frame.isOccupied) throw std::runtime_error("[evictFrame]: Frame is not occupied");
    if(frame.pinCount > 0) throw std::runtime_error("[evictFrame]: Frame is still pinned");

    const PageID& framePageID = frame.page.getPageID();
    if(frame.dirtyFlag) flushPage(framePageID);

    frameLookup.erase(framePageID);
    LRUList.remove(frameIndex);

    frame.isOccupied = false;
    frame.pinCount = 0;
    frame.dirtyFlag = false;
}

BufferPool::BufferPool(FileManager& fileManager, std::unordered_map<std::int64_t, std::string>& tableIDToFile, std::int8_t frameCount) 
    : fileManager(fileManager), tableIDToFile(tableIDToFile)
{
    if(frameCount <= 0) throw std::invalid_argument("[BufferPool]: Frame Count must be greater than 0");
    framePool.resize(frameCount);
}

Page& BufferPool::fetchPage(const PageID& pageID)
{
    std::size_t frameIndex = framePool.size();
    if(isPageInPool(pageID))
    {
        frameIndex = frameLookup.at(pageID);
    }
    else
    {
        std::string fileName = tableIDToFile.at(pageID.tableID);
        if(!fileManager.fileExists(fileName)) throw std::invalid_argument("[fetchPage]: pageID does not exist in a file");
        
        frameIndex = findUnusedFrame();
        if(frameIndex == framePool.size())
        {
            frameIndex = findVictimFrame();
            if(frameIndex == framePool.size()) throw std::runtime_error("[fetchPage]: buffer pool full & every frame is pinned");
            evictFrame(frameIndex); 
        }
        placeFrame(frameIndex, pageID);
    }

    BufferFrame& frame = framePool[frameIndex];
    frame.pinCount++;
    updateLRUFrame(frameIndex);

    return frame.page;
}

void BufferPool::unpinPage(const PageID& pageID, bool is_dirty)
{
    if(!isPageInPool(pageID)) throw std::runtime_error("[unpinPage]: Suppled page not in frame pool");

    std::size_t frameIndex = frameLookup.at(pageID);
    BufferFrame& frame = framePool[frameIndex];
    if(frame.pinCount == 0) throw std::runtime_error("[unpinPage]: Page in frame's pinCount is 0");
    if(is_dirty) frame.dirtyFlag = true;
    frame.pinCount--;
}

void BufferPool::flushPage(const PageID& pageID)
{
    if(!isPageInPool(pageID)) throw std::runtime_error("[flushPage]: Suppled page not in frame pool");
    std::size_t frameIndex = frameLookup.at(pageID);
    BufferFrame& frame = framePool[frameIndex];
    if(!frame.dirtyFlag) return;
    std::string fileName = tableIDToFile.at(pageID.tableID);
    fileManager.writePage(fileName, pageID.pageNumber, frame.page.getPageBytesConst());
    frame.dirtyFlag = false;
}

void BufferPool::flushAllPages()
{
    for(auto& [pageID, frameIndex] : frameLookup)
    {
        flushPage(pageID);
    }
}

bool BufferPool::isPageInPool(const PageID& pageID) const
{
    return frameLookup.find(pageID) != frameLookup.end();
}

bool BufferPool::isPagePinned(const PageID& pageID) const
{
    if(!isPageInPool(pageID)) return false;
    std::size_t frameIndex = frameLookup.at(pageID);

    const BufferFrame& frame = framePool[frameIndex];
    return frame.pinCount != 0;
}