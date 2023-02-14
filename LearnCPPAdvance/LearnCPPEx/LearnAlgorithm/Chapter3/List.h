#pragma once
#include <functional>

#include "Common/CommonTypes.h"

namespace QxNS
{

    template<typename  T>
    class ListNode
    {
    public:
        //
        T data;
        typedef ListNode<T>*  ListNodePtr;

        ListNodePtr prev; //前驱
        ListNodePtr succ; // 后继

        ListNode() {  }
        ListNode(T inData, ListNodePtr inPrev, ListNodePtr inSucc)
            : data(inData), prev(inPrev), succ(inSucc)
        {
        }

        // 接口
    public:
        ListNodePtr insertAsPrev(const T& e);
        ListNodePtr insertAsSucc(const T& e);
    };

 

    template<typename T>
    class List
    {
    public:
        using ListNodePtr = typename  ListNode<T>::ListNodePtr;
        using ValueType = T;

        List() { Init(); }
        List(const List<T>& other);
        List(const List<T>& other, Rank startIndex, int length);
        List(ListNodePtr startPtr, int length);

        ~List();

        ListNodePtr First() const { return  header->succ; };
        ListNodePtr Last() const { return trailer->prev; }

        bool IsValid(ListNodePtr inPtr)
        {
            return inPtr && (inPtr != header) && (inPtr != trailer);
        }

        ListNodePtr Find(const T& target);
        ListNodePtr Find(const T& target, int size, ListNodePtr inPtr);


        void Sort(ListNodePtr startPtr, int length, SortMethod sortMehtod = SortMethod::SelectSort); // list 区间排序
        void Sort(SortMethod sort_method )
        {
            Sort(First(), m_size, sort_method);
        }
        void Sort()
        {
            Sort(First(), m_size);
        }

        ListNodePtr InsertAsFirst(const T& data);
        ListNodePtr InsertAsLast(const T& data);
        ListNodePtr InsertBefore(ListNodePtr insertPtr, const T& data);
        ListNodePtr InsertAfter(ListNodePtr insertPtr,  const T& data);

        Rank GetSize() const {return m_size;}
        bool IsEmpty() const {return  m_size <= 0;}

        T& operator[](Rank r);


        // void Traverse(std::function<void(T)>);
        template<typename F>
        void Traverse(F functor)
        {
            for (ListNodePtr i = header->succ; i != trailer; i = i->succ)
            {
                functor(i->data);
            }
        }
    protected:
        void Init();
        int Clear();
        void CopyNodes(ListNodePtr startPtr, int length);
        void Mergesort(ListNodePtr startPtr, int length);
        void SelectionSort(ListNodePtr startPtr, int length);
        void InsertionSort(ListNodePtr startPtr, int length);

        ListNodePtr SelectMax(ListNodePtr startPtr, int length);

        // 删除节点返回其数值
        T Remove(ListNodePtr inPtr);

        // inPtr前面的n(n>=1)个前驱中查找<= target的最后一个元素, 目的是在返回的ptr之后插入新的节点
        // 这n个前驱已经有序,
        ListNodePtr Search(const T& target, int n, ListNodePtr inPtr);

    private:
        int m_size = 0;

        ListNodePtr header;
        ListNodePtr trailer;
    };

}


void TestList();
