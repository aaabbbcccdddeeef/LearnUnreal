#pragma once
#include <iostream>


namespace IO
{
    /**
     * \brief  这个函数的主要目的是替代std::endl，输出换行但不执行flush
     */
    template<typename T, typename CharT = std::char_traits<T>>
    std::basic_ostream<T, CharT>&
        endl(std::basic_ostream<T, CharT>& outputStream)
    {
        return outputStream << outputStream.widen('\n');
    }
}
