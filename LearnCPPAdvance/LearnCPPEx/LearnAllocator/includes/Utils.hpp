#pragma once


namespace Utils
{
    size_t CalculatePadding(const size_t baseAddress, const size_t alignment);

    size_t CalculatePaddingWithHeader(const size_t baseAddress,
                                      const size_t alignment, const size_t headerSize);
}
