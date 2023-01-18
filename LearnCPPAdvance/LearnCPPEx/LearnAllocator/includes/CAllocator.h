#pragma once
#include "Allocator.h"

// CAllocator 只是传递分配调用给C的malloc/free
class CAllocator : public Allocator
{
public:
    CAllocator();
    virtual ~CAllocator();

    virtual  void* Allocate(const std::size_t size, const std::size_t alignment) override;
    virtual  void Free(void* ptr) override;
    virtual  void Init() override;
};
