#include "Benchmark.h"

#include <assert.h>
#include <iostream>

#include "IO.h"


void Benchmark::SingleAllocation(Allocator* allocator, const std::size_t size, const std::size_t alignment)
{
    std::cout << "Benchmark : allocation " << IO::endl;
    std::cout << "\tSize:   \t" << size << IO::endl;
    std::cout << "\tAlignment\t" << alignment << IO::endl;

    StartRound();

    allocator->Init();

    for (size_t i = 0; i < m_nOperations; ++i)
    {
        allocator->Allocate(size, alignment);
    }

    FinishRound();

    const BenchmarkResults& results = buildResults(m_nOperations, m_TimeElapsed, allocator->m_peak);

    PrintResults(results);
}

void Benchmark::SingleFree(Allocator* allocator, const std::size_t size, const std::size_t alignment)
{
    std::cout << "Benchmark : allocation/free " << IO::endl;
    std::cout << "\tSize:   \t" << size << IO::endl;
    std::cout << "\tAlignment\t" << alignment << IO::endl;

   
    void* addresses[OperationNum];

    StartRound();

    size_t operations = 0;

    while (operations < m_nOperations)
    {
        addresses[operations] = allocator->Allocate(size, alignment);
        ++operations;
    }

    while (operations)
    {
        --operations;
        allocator->Free(addresses[operations]);
    }
    
    FinishRound();

    BenchmarkResults results = buildResults(m_nOperations, m_TimeElapsed, allocator->m_peak);

    PrintResults(results);
}

void Benchmark::MultipleAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSize,
    const std::vector<std::size_t>& alignments)
{
    assert(allocationSize.size() == alignments.size() && "allocationSize must equal alignments size");
    
    for (size_t i = 0; i < allocationSize.size(); ++i)
    {
        SingleAllocation(allocator, allocationSize[i], alignments[i]);
    }    
}

void Benchmark::MultipleFree(Allocator* allocator, const std::vector<std::size_t>& allocationSize,
    const std::vector<std::size_t>& alignment)
{
    assert(allocationSize.size() == alignment.size() && "Allocation sizes and Alignments must have same length");

    for (auto i = 0u; i < allocationSize.size(); ++i) {
        SingleFree(allocator, allocationSize[i], alignment[i]);
    }
        
}

void Benchmark::RandomAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes,
    const std::vector<std::size_t>& alignments)
{
    // NOTE: Is this actually initializing the RNG? Jose Fernando Lopez Fernandez 11/07/2018 @ 12:54am (UTC)
    srand(1);

    std::cout << "\tBENCHMARK: ALLOCATION" << IO::endl;

    StartRound();

    std::size_t allocation_size;
    std::size_t alignment;

    allocator->Init();

    auto operations = 0u;

    while (operations < m_nOperations) {
        this->RandomAllocationAttr(allocationSizes, alignments, allocation_size, alignment);
        allocator->Allocate(allocation_size, alignment);
        ++operations;
    }
    
    FinishRound();

    BenchmarkResults results = buildResults(m_nOperations, m_TimeElapsed, allocator->m_peak);
    
    PrintResults(results);    
}

void Benchmark::RandomFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes,
    const std::vector<std::size_t>& alignments)
{
    // NOTE: Is this actually initializing the RNG? Jose Fernando Lopez Fernandez 11/07/2018 @ 1:51am (UTC)
    srand(1);

    std::cout << "\tBENCHMARK: ALLOCATION/FREE" << IO::endl;

    StartRound();

    void* addresses[OperationNum];

    std::size_t allocation_size;
    std::size_t alignment;

    allocator->Init();

    auto operations = 0u;

    while (operations < m_nOperations) {
        this->RandomAllocationAttr(allocationSizes, alignments, allocation_size, alignment);
        addresses[operations] = allocator->Allocate(allocation_size, alignment);
        ++operations;
    }

    while (operations) {
        allocator->Free(addresses[--operations]);
    }

    FinishRound();

    BenchmarkResults results = buildResults(m_nOperations, m_TimeElapsed, allocator->m_peak);

    PrintResults(results);
}

void Benchmark::RandomAllocationAttr(const std::vector<std::size_t>& allocationSizes,
    const std::vector<std::size_t>& alignments, std::size_t& size, std::size_t& alignment)
{
    const int r = rand() % allocationSizes.size();
    size = allocationSizes[r];
    alignment = alignments[r];
}
