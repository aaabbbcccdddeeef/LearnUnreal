#include "LinearAllocator.h"

#include <assert.h>

#include "Utils.hpp"




void* LinearAllocator::Allocate(const std::size_t size, const std::size_t alignment)
{
    std::size_t padding = 0;
    std::size_t paddedAddress = 0;
    const size_t curAddress = (size_t)m_StartPtr + m_Offset;

    if (alignment != 0 && m_Offset % alignment != 0)
    {
        // 在offset 不是alignment的整数倍时，需要padding
        padding = Utils::CalculatePadding(curAddress, alignment);
    }

    if (m_Offset + padding + size > m_totalSize)
    {
        return nullptr;
    }

    m_Offset += padding;
    const std::size_t nextAddress = curAddress + padding;
    m_Offset += size;

    m_used = m_Offset;
    m_peak = std::max(m_peak, m_used);

    return (void*)nextAddress;
}

void LinearAllocator::Free(void* ptr)
{
    assert(false && "Use Reset() method");
}

void LinearAllocator::Init()
{
    if (m_StartPtr != nullptr)
    {
        free(m_StartPtr);
    }

    m_StartPtr = malloc(m_totalSize);
    m_Offset = 0;
}

void LinearAllocator::Reset()
{
    m_Offset = 0;
    m_used = 0;
    m_peak = 0;
}
