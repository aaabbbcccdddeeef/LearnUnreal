#pragma once
#include "Allocator.h"
#include "SinglyLinkedList.h"


class FreeListAllocator : public Allocator
{
public:
    enum PlacementPolicy
    {
        FIND_FIRST,
        FIND_BEST
    };
public:
    FreeListAllocator(size_t totalSize, PlacementPolicy placePolicy);
    FreeListAllocator(const FreeListAllocator&) = delete;
    ~FreeListAllocator() override;
    void* Allocate(const std::size_t size, const std::size_t alignment) override;
    void Free(void* ptr) override;
    void Init() override;

    virtual  void Reset();
private:
    // 合格header 是但链表记录block size的结构
    struct FreeHeader
    {
        size_t blockSize;
    };

    // 这个header是记录
    struct AllocationHeader
    {
        size_t blockSize;
        char padding;
    };

    using FreeNode =  SinglyLinkedList<FreeHeader>::Node;

    void* m_StartPtr = nullptr;

    PlacementPolicy m_pPolicy;
    SinglyLinkedList<FreeHeader> m_FreeList; //保存所有可以分配的node的链表 

private:
    void Find(size_t size, size_t alignment,
        size_t& outPadding, FreeNode* &outPreviousNode,
        FreeNode* & outFoundNode);
    void FindFirst(size_t size, size_t alignment,
        size_t& outPadding, FreeNode* &outPreviousNode,
        FreeNode* & outFoundNode);
    void FindBest(size_t size, size_t alignment,
        size_t& outPadding, FreeNode* &outPreviousNode,
        FreeNode* & outFoundNode);
private:
    
};
