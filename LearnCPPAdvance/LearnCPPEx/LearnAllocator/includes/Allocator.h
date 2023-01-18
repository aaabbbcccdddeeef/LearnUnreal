#pragma once
#include <cstddef> // size_t

#include "Benchmark.h"


// https://github.com/mtrebi/memory-allocators
// 这个测试中的allocator参考上面的实现
class Allocator
{
public:
    Allocator(const std::size_t totalSize)
        : m_totalSize(totalSize), m_used(0), m_peak(0)
    {
    }

    virtual ~Allocator()
    {
        m_totalSize = 0;
    }

    virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) = 0;

    virtual void Free(void* ptr) = 0;

    virtual void Init() = 0;

    friend class Benchmark;
protected:
    std::size_t m_totalSize;
    std::size_t m_used;
    std::size_t m_peak;
};
