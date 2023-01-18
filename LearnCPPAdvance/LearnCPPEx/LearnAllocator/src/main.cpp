#include <iostream>
#include <cstddef>
#include <vector>

#include "Allocator.h"
#include "Allocator.h"
#include "Benchmark.h"
#include "CAllocator.h"
#include "FreeListAllocator.h"
#include "LinearAllocator.h"
#include "PoolAllocator.h"
#include "StackAllocator.h"

// 测试执行次数
// #define OPERATIONS (10)

int main()
{
    
    const std::size_t A = static_cast<std::size_t>(1e9);
    const size_t B = static_cast<std::size_t>(1e8);
    
    Allocator* cAllocator = new CAllocator();
    Allocator* linearAllocator = new LinearAllocator(A);
    Allocator* stackAllocator = new StackAllocator(A);
    Allocator* poolAllocator = new PoolAllocator(16777216, 4096);
    Allocator* freeListAllocator = new FreeListAllocator(B, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    
    
    const std::vector<std::size_t> ALLOCATION_SIZES {32, 64, 256, 512, 1024, 2048, 4096};
    const std::vector<std::size_t> ALIGNMENTS {8, 8, 8, 8, 8, 8, 8};
    
    Benchmark benchmark;
    
    std::cout << "C" << std::endl;
    benchmark.MultipleAllocation(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(cAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    
    std::cout << "Linear Allocator" << std::endl;
    // linear allocator不支持单独回收内部的一段内存，只能整个reset
    benchmark.MultipleAllocation(linearAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(linearAllocator, ALLOCATION_SIZES, ALIGNMENTS);


    std::cout << "Stack Allocator" << std::endl;
    // 测试header的记录是否正确
    {
        stackAllocator->Init();
        void* testAddress = stackAllocator->Allocate(35, 16);
        stackAllocator->Free(testAddress);
    }
    benchmark.MultipleAllocation(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    std::cout << "RandomAllocate" << std::endl;
    benchmark.RandomAllocation(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(stackAllocator, ALLOCATION_SIZES, ALIGNMENTS);

    std::cout << "Pool allocator" << std::endl;
    benchmark.SingleAllocation(poolAllocator, 4096, 8);
    benchmark.SingleFree(poolAllocator, 4096, 8);


    std::cout << "Freelist Allocator" << std::endl;
    benchmark.MultipleAllocation(freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.MultipleFree(freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomAllocation(freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    benchmark.RandomFree(freeListAllocator, ALLOCATION_SIZES, ALIGNMENTS);
    
    delete cAllocator;
    delete linearAllocator;
    delete stackAllocator;
    delete poolAllocator;
    delete freeListAllocator;
    return 1;
}
