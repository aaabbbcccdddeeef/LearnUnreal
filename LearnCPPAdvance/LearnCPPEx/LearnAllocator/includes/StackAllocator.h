#pragma once
#include "Allocator.h"

/**
 * \brief StackAllocator和linearAllocator相似，
 * 也是预分配一块内存池，通过offset指针指示已分配内存末尾，
 * 但可以实现内存回收，实现上是在分配内存时在前面额外插入一个header结构，存储当前offset到返回的分配地址之间的padding大小
 */
class StackAllocator : public Allocator
{
public:
    StackAllocator(size_t totalSize)
        : Allocator(totalSize)
    {
    }

    StackAllocator(const StackAllocator&) = delete; 

    ~StackAllocator() override
    {
        
    }
    void* Allocate(const std::size_t size, const std::size_t alignment) override;
    void Free(void* ptr) override;
    void Init() override;


    virtual void Reset();
protected:
    void* m_pStartPtr = nullptr;
    size_t m_Offset = 0;

private:
    struct AllocationHeader
    {
        char padding;
    };
};
