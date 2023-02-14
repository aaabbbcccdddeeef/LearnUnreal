#include "Vector.h"

#include <set>

namespace QxNS
{
    template <typename T>
    bool Vector<T>::Bubble(Rank lo, Rank hi)
    {
        bool bIsSorted = true;
        while (++lo < hi)
        {
            if (m_Elements[lo - 1] > m_Elements[lo])
            {
                bIsSorted = false;
                std::swap(m_Elements[lo - 1], m_Elements[lo]);
            }
        }
        return bIsSorted;
    }

    template <typename T>
    void Vector<T>::BubbleSort(Rank lo, Rank hi)
    {
        while (!Bubble(lo, hi--)) // 逐趟做扫描交换直到有序
        {
        }
    }

    template <typename T>
    void Vector<T>::Merge(Rank lo, Rank mi, Rank hi)
    {
        T* targetEles = m_Elements + lo; // 拷贝输出后的结果 

        int leftLength = mi - lo;
        T* leftTmp = new T[leftLength];
        for (int i = 0; i < leftLength; ++i)
        {
            leftTmp[i] = targetEles[i];
        }

        int rightLength = hi - mi;
        T* right = m_Elements + mi;

        // 归并
        for (Rank iTarget = 0, iLeft = 0, iRight = 0; iLeft < leftLength; )
        {
            targetEles[iTarget++] = (leftTmp[iLeft] <= right[iRight] || rightLength <= iLeft/* 左侧的index超过右侧的长度 */ )   ?
                leftTmp[iLeft++] : right[iRight++]; 
        }
        delete[] leftTmp;
    }

    template <typename T>
    void Vector<T>::MergeSort(Rank lo, Rank hi)
    {
        if (hi - lo < 2)
        {
            return;
        }
        Rank mi = (lo + hi) >> 1;
        MergeSort(lo, mi); // 排序左半部分
        MergeSort(mi, hi); // 排序有半部分
        Merge(lo, mi, hi); // 合并
    }
}
