#include "PoolAllocator.h"

#include <assert.h>


PoolAllocator::PoolAllocator(const size_t totalSize, const size_t chunkSize)
    : Allocator(totalSize)
{
    assert(chunkSize > 8 && " Chunk size must be bigger than 8");
    assert(totalSize % 8 == 0 && "total size must be multiple of chuncksize");
    m_ChunkSize = chunkSize;
}

PoolAllocator::~PoolAllocator()
{
    free(m_StartPtr);
}

void* PoolAllocator::Allocate(const std::size_t allocationSize, const std::size_t alignment)
{
    assert(allocationSize == m_ChunkSize && " allocation size must be equal to chunk size");

    Node* freeNode  = m_FreeList.Pop();

    assert(freeNode != nullptr && "The pool allocator is full");

    m_used += m_ChunkSize;
    m_peak = m_used;

    return reinterpret_cast<void*>(freeNode);
}

void PoolAllocator::Free(void* ptr)
{
    m_used -= m_ChunkSize;

    m_FreeList.Push((Node*)ptr);
}

void PoolAllocator::Init()
{
    m_StartPtr = malloc(m_totalSize);
    Reset();
}

void PoolAllocator::Reset()
{
    m_used = 0;
    m_peak = 0;

    const int chunkNum = m_totalSize / m_ChunkSize;

    for (int i = 0; i < chunkNum; ++i)
    {
        size_t address = reinterpret_cast<size_t>(m_StartPtr) + i * m_ChunkSize;
        m_FreeList.Push((Node*)address);
    }
}
