#pragma once
#include "Allocator.h"
#include "StackLinkedList.h"

/**
 * \brief 池类型的allocator
 * 内部维护2个对象来完成内存分配，
 * 1. chunSize倍数的内存大小，
 * 2. 一个栈链表指向所有可用chunk
 * 分配、释放的基本思想是，每次只能申请chunk大小的内存，从freeLink栈顶获得和放入
 */
class PoolAllocator : public  Allocator
{
public:
    PoolAllocator(const size_t totalSize, const size_t chunkSize);
    PoolAllocator(const PoolAllocator& ) = delete;
    ~PoolAllocator() override;
    void* Allocate(const std::size_t size, const std::size_t alignment) override;
    void Free(void* ptr) override;
    void Init() override;

    /**
     * \brief reset 内部的链表指向所有可用chunk
     */
    virtual void Reset();
private:
    struct FreeHeader
    {
    };

    using  Node = StackLinkedList<FreeHeader>::Node;
    StackLinkedList<FreeHeader> m_FreeList;

    void* m_StartPtr = nullptr;
    size_t m_ChunkSize;
};
