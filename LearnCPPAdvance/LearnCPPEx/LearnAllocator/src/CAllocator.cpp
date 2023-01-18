#include "CAllocator.h"

#include <cstdlib>

CAllocator::CAllocator()
    : Allocator(0)
{
}

CAllocator::~CAllocator()
{
    
}

void* CAllocator::Allocate(const std::size_t size, const std::size_t alignment)
{
    return malloc(size);
}

void CAllocator::Free(void* ptr)
{
    free(ptr);
}

void CAllocator::Init()
{
    
}


