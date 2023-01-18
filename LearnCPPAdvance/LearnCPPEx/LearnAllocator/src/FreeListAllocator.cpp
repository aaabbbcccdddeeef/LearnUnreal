#include "FreeListAllocator.h"

#include <assert.h>

#include "Utils.hpp"

FreeListAllocator::FreeListAllocator(size_t totalSize, PlacementPolicy placePolicy)
    : Allocator(totalSize)
    , m_pPolicy(placePolicy)
{
    
}

FreeListAllocator::~FreeListAllocator()
{
    free(m_StartPtr);
    m_StartPtr = nullptr;
}

void* FreeListAllocator::Allocate(const std::size_t size, const std::size_t alignment)
{
    const size_t allocationHeaderSize = sizeof(FreeListAllocator::AllocationHeader);
    const size_t freeHeaderSize = sizeof(FreeListAllocator::FreeHeader);
    assert("Allocation size must be bigger" && size >= sizeof(FreeNode) );
    assert("Alignment must be 8 at least" && alignment >= 8);

    // 从freelist 中查找足够存放要分配的数据的block
    std::size_t padding;
    FreeNode *affectNode, * prevousNode;
    
    Find(size, alignment, padding, prevousNode, affectNode);
    assert(affectNode != nullptr && "Not engouh memory");

    size_t alignmentPadding = padding - allocationHeaderSize;
    size_t requiredSize = size + padding;

    size_t rest = affectNode->data.blockSize - requiredSize;

    if (rest > 0) // 如果还有剩余空间，需要产生一个新的freenode
    {
        FreeNode* newFreeNode = (FreeNode*)((size_t)affectNode + requiredSize);
        newFreeNode->data.blockSize = rest;
        m_FreeList.insert(affectNode, newFreeNode);
    }

    m_FreeList.remove(prevousNode, affectNode);

    size_t headerAddress = reinterpret_cast<size_t>(affectNode) + alignmentPadding;
    size_t dataAddress = headerAddress + allocationHeaderSize;
    AllocationHeader* headerPtr = reinterpret_cast<AllocationHeader*>(headerAddress);
    headerPtr->blockSize = requiredSize;
    headerPtr->padding = static_cast<char>(alignmentPadding);

    m_used += requiredSize;
    m_peak = std::max(m_peak, m_used);

    return reinterpret_cast<void*>(dataAddress);
}

void FreeListAllocator::Free(void* ptr)
{
    size_t curAddress = reinterpret_cast<size_t>(ptr);
    size_t headerAddress = curAddress - sizeof(AllocationHeader);
    AllocationHeader* allocationHeaderPtr = reinterpret_cast<AllocationHeader*>(headerAddress);

    FreeNode* freeNode = reinterpret_cast<FreeNode*>(curAddress);
    freeNode->data.blockSize = allocationHeaderPtr->blockSize + allocationHeaderPtr->padding;
       
}

void FreeListAllocator::Init()
{
    if (m_StartPtr != nullptr)
    {
        free(m_StartPtr);
        m_StartPtr = nullptr;
    }
    m_StartPtr = malloc(m_totalSize);

    Reset();
}

void FreeListAllocator::Reset()
{
    m_used = 0;
    m_peak = 0;
    FreeNode * firstNode = reinterpret_cast<FreeNode*>(m_StartPtr);
    firstNode->data.blockSize = m_totalSize;
    firstNode->next = nullptr;
    m_FreeList.head = nullptr;
    m_FreeList.insert(nullptr, firstNode);
}

void FreeListAllocator::Find(size_t size, size_t alignment,
    size_t& outPadding, FreeNode*& outPreviousNode,
    FreeNode*& outFoundNode)
{
    switch (m_pPolicy)
    {
    case FIND_FIRST:
        FindFirst(size, alignment, outPadding, outPreviousNode, outFoundNode);
        break;
    case FIND_BEST:
        FindBest(size, alignment, outPadding, outPreviousNode, outFoundNode);
        break;
    default: ;
    }
}

// 查找freelist 得到可分配的第一个node
void FreeListAllocator::FindFirst(size_t size, size_t alignment, size_t& outPadding, FreeNode*& outPreviousNode,
    FreeNode*& outFoundNode)
{
    FreeNode* it = m_FreeList.head;
    FreeNode* itPrev = nullptr;

    while (it != nullptr)
    {
        outPadding = Utils::CalculatePaddingWithHeader(reinterpret_cast<size_t>(it),
            alignment, sizeof(FreeListAllocator::AllocationHeader)
            );
        size_t requiredSpace = size + outPadding;
        if (it->data.blockSize >= requiredSpace)
        {
            break;
        }

        itPrev = it;
        it  = it->next;
    }
    outPreviousNode = itPrev;
    outFoundNode = it;
}

void FreeListAllocator::FindBest(size_t size, size_t alignment, size_t& outPadding, FreeNode*& outPreviousNode,
                                 FreeNode*& outFoundNode)
{
    size_t smallsetDiff = std::numeric_limits<std::size_t>::max();
    FreeNode* bestBlock = nullptr;
    FreeNode* it = m_FreeList.head;
    FreeNode* itPrev = nullptr;
    while (it != nullptr)
    {
        outPadding = Utils::CalculatePaddingWithHeader(
            reinterpret_cast<size_t>(m_StartPtr),
            alignment, sizeof(FreeListAllocator::AllocationHeader)
            );
        size_t requiredSize = size + outPadding;
        if (it->data.blockSize >= requiredSize && (it->data.blockSize - requiredSize < smallsetDiff))
        {
            smallsetDiff = it->data.blockSize - requiredSize;
            bestBlock = it;
        }
        itPrev = it;
        it = it->next;
    }
    outPreviousNode = itPrev;
    outFoundNode  = it;
}
