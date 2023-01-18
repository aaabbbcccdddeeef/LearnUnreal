#pragma once


#include <time.h> // timespec
#include <cstddef> // std::size_t
#include <chrono>
#include <iostream>
#include <ratio>
#include <vector>

#include "Allocator.h"
#include "IO.h"

#define OperationNum 100000

class Allocator;

struct BenchmarkResults
{
    std::size_t Operations;
    std::chrono::milliseconds Milliseconds;
    double OperationsPerSec;
    double TimePerOperation;
    std::size_t MemoryPeak;
};

// template<int OperationNum>
// #TODO 改成模板传执行次数
class Benchmark
{
public:
    Benchmark(const size_t nOperations = OperationNum)
        : m_nOperations(nOperations)
    {
        
    }

    // 按同一个size分配多次测试
    void SingleAllocation(Allocator* allocator, const std::size_t size, const std::size_t alignment);
    void SingleFree(Allocator* allocator, const std::size_t size, const std::size_t alignment);

    // 按多个size分配测试
    void MultipleAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSize,
                            const std::vector<std::size_t>& alignments);

    void MultipleFree(Allocator* allocator, const std::vector<std::size_t>& allocationSize,
                      const std::vector<std::size_t>& alignment);

    void RandomAllocation(Allocator* allocator, const std::vector<std::size_t>& allocationSizes,
                          const std::vector<std::size_t>& alignments);

    void RandomFree(Allocator* allocator, const std::vector<std::size_t>& allocationSizes,
                    const std::vector<std::size_t>& alignments);

private:

    void SetStartTime() noexcept
    {
        m_Start = std::chrono::steady_clock::now();
    }
    void SetFinishTime() noexcept
    {
        m_End = std::chrono::steady_clock::now();
    }
    void SetElapsedTime() noexcept
    {
        m_TimeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(m_End - m_Start);
    }
    
    void StartRound()
    {
        SetStartTime();
    }

    void RandomAllocationAttr(const std::vector<std::size_t>& allocationSizes,
        const std::vector<std::size_t>& alignments, std::size_t & size, std::size_t & alignment);
    
    void FinishRound()
    {
        SetFinishTime();
        SetElapsedTime();
    }
    void PrintResults(const BenchmarkResults& results) const
    {
        std::cout << "\tRESULTS:" << IO::endl;
        std::cout << "\t\tOperations:    \t" << results.Operations << IO::endl;
        std::cout << "\t\tTime elapsed: \t" << results.Milliseconds.count() << " ms" << IO::endl;
        std::cout << "\t\tOp per sec:    \t" << results.OperationsPerSec << " ops/ms" << IO::endl;
        std::cout << "\t\tTimer per op:  \t" << results.TimePerOperation << " ms/ops" << IO::endl;
        std::cout << "\t\tMemory peak:   \t" << results.MemoryPeak << " bytes" << IO::endl;

        std::cout << IO::endl;
    }

    
    BenchmarkResults buildResults(size_t nOperations,
        const std::chrono::milliseconds& ellapsedTime, const size_t memoryUsed) const
    {
        BenchmarkResults result;

        result.Operations = nOperations;
        result.Milliseconds = ellapsedTime;
        result.OperationsPerSec = static_cast<double>(result.Operations) / static_cast<double>(result.Milliseconds.count());
        result.TimePerOperation = static_cast<double>(result.Milliseconds.count()) / static_cast<double>(result.Operations);
        result.MemoryPeak = memoryUsed;
        return result;
    }
    
private:
    std::size_t m_nOperations;

    std::chrono::time_point<std::chrono::steady_clock> m_Start;
    std::chrono::time_point<std::chrono::steady_clock> m_End;

    std::chrono::milliseconds m_TimeElapsed;
};

