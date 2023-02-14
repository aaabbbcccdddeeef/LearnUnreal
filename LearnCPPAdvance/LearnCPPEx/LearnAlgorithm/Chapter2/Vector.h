#pragma once

// #include "Common"
// #include "CommonTypes.h"
#include "Common/CommonTypes.h"
#define DEFAULT_CAPACITY 3

namespace QxNS
{
    template<typename  T>
    class Vector 
    {
    public:

        bool Bubble(Rank lo, Rank hi); // 扫描交换
        void BubbleSort(Rank lo, Rank hi); // 冒泡排序, 冒泡排序是稳定排序


        void Merge(Rank lo, Rank mi, Rank hi); // 一次归并过程
        void MergeSort(Rank lo, Rank hi); // 归并排序

        Rank m_Size;
        int m_Capacity;
        T* m_Elements;
    };


}

