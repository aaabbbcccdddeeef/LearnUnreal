#include "StackAllocator.h"

#include "Utils.hpp"

void* StackAllocator::Allocate(
    const std::size_t size,
    const std::size_t alignment)
{
    const size_t curAddress = (size_t)m_pStartPtr + m_Offset;

    size_t padding = Utils::CalculatePaddingWithHeader(curAddress, alignment, sizeof(AllocationHeader));

    if (m_Offset + padding + size > m_totalSize)
    {
        return nullptr;
    }
    m_Offset += padding;

    size_t nextAddress = curAddress + padding;
    size_t headerAddress = nextAddress - sizeof(AllocationHeader);
    AllocationHeader allocationHeader{padding};
    AllocationHeader* headerPtr = (AllocationHeader*)headerAddress;
    (*headerPtr) = allocationHeader;

    m_Offset += size;

    m_used = m_Offset;
    m_peak = std::max(m_peak, m_used);

    return (void*)nextAddress;
}

void StackAllocator::Free(void* ptr)
{
    size_t curAddress = reinterpret_cast<size_t>(ptr);
    size_t headerAddress = curAddress - sizeof(AllocationHeader);
    const AllocationHeader* allocationHeader = reinterpret_cast<AllocationHeader*>(headerAddress);
    
    m_Offset = curAddress - allocationHeader->padding - reinterpret_cast<size_t>(m_pStartPtr);
    m_used = m_Offset;
}

void StackAllocator::Init()
{
    if (m_pStartPtr != nullptr)
    {
        free(m_pStartPtr);
    }
    m_pStartPtr = malloc(m_totalSize);
    m_Offset = 0;
}

void StackAllocator::Reset()
{
    m_Offset = 0;
    m_used  = 0;
    m_peak = 0;
}
