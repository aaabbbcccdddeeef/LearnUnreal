#pragma once

namespace My
{
    struct BlockHeader
    {
        BlockHeader* pNext;
    };

    struct PageHeader
    {
        PageHeader* pNext;
        BlockHeader* Blocks()
        {
            return reinterpret_cast<BlockHeader*>(this + 1);
        }
    };

    class QxAllocator
    {
    public:
        
    };
}
