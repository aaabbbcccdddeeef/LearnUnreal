#include "Utils.hpp"

namespace Utils
{
    size_t CalculatePadding(const size_t baseAddress, const size_t alignment)
    {
        const size_t multiplier = (baseAddress/alignment) + 1;
        const size_t alignedAddress = multiplier * alignment;
        const size_t padding = alignedAddress - baseAddress;
        return padding;
    }

    size_t CalculatePaddingWithHeader(const size_t baseAddress, const size_t alignment, const size_t headerSize)
    {
        size_t padding = CalculatePadding(baseAddress, alignment);
        size_t neededSpace = headerSize;

        // header 没办法直接放进padding的空间中，计算下一个aligned address that head fit
        if (padding < neededSpace)
        {
            neededSpace -= padding;

            // 为padding添加放header需要的空间, 下面的意思是needSpace能被alignment整除与否用不同个公式计算
            if (neededSpace % alignment > 0)
            {
                padding += alignment * (1 + (neededSpace / alignment));
            }
            else if (neededSpace % alignment == 0)
            {
                padding += alignment * (neededSpace / alignment);
            }
        }

        return padding;
    }
}
