#pragma once
#include "Allocator.h"

/**
 * \brief Linear allocator的策略是，初始化时预分配一大段内存, 不支持回收
 * 记录内存起始位置和已分配内存的末尾的offset，每次分配移动offset, O(1)
 * 效率非常高
 * 碎片只有padding
 */
class LinearAllocator : public Allocator
{
public:
    LinearAllocator(size_t totalSize)
        :Allocator(totalSize)
    {
        
    }

    LinearAllocator(const LinearAllocator& ) = delete;

    virtual ~LinearAllocator()
    {
        free(m_StartPtr);
        m_StartPtr = nullptr;
    }
    void* Allocate(const std::size_t size, const std::size_t alignment) override;
    void Free(void* ptr) override;
    void Init() override;


    virtual void Reset();
    
protected:
    void* m_StartPtr = nullptr;
    std::size_t m_Offset = 0;
};
