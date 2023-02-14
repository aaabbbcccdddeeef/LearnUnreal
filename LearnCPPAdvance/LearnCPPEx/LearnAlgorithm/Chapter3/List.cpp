#include "List.h"

#include <cstdlib>
#include <stdexcept>

#include "Common/QxLogger.h"

namespace QxNS
{
    template <typename T>
    typename ListNode<T>::ListNodePtr ListNode<T>::insertAsPrev(const T& e)
    {
        ListNodePtr newNodePtr = new ListNode<T>(e, prev, this);
        // newNodePtr->succ = this;
        
        prev->succ = newNodePtr;
        this->prev = newNodePtr;
        return  newNodePtr;
    }

    template <typename T>
    typename ListNode<T>::ListNodePtr ListNode<T>::insertAsSucc(const T& e)
    {
        ListNodePtr newNode = new ListNode<T>(e, this, this->succ);
        succ->prev = newNode;
        this->succ = newNode;
        return newNode;
    }

    template <typename T>
    List<T>::List(const List<T>& other)
    {
        CopyNodes(other.fi)
    }

    template <typename T>
    List<T>::~List()
    {
        Clear();
        delete header;
        delete trailer;
    }


    template <typename T>
    typename List<T>::ListNodePtr List<T>::Find(const T& target)
    {
    }

    template <typename T>
    typename List<T>::ListNodePtr List<T>::Find(const T& target, int size, ListNodePtr inPtr)
    {
    }

    template <typename T>
    void List<T>::Sort(ListNodePtr startPtr, int length, SortMethod sortMehtod)
    {
        switch (sortMehtod)
        {
        case SortMethod::SelectSort:
            SelectionSort(startPtr, length);
            break;
        case SortMethod::InsertionSort:
            InsertionSort(startPtr, length);
            break;
        case SortMethod::MergetSort:
            Mergesort(startPtr, length);
            break;
        default:
            throw std::runtime_error("no implemented");
            ;
        }
    }

    template <typename T>
    typename List<T>::ListNodePtr List<T>::InsertAsFirst(const T& data)
    {
        ++m_size;
        return header->insertAsSucc(data);
    }

    template <typename T>
    typename List<T>::ListNodePtr List<T>::InsertAsLast(const T& data)
    {
        ++m_size;
        return trailer->insertAsPrev(data);
    }

    template <typename T>
    typename List<T>::ListNodePtr List<T>::InsertBefore(ListNodePtr insertPtr, const T& data)
    {
        ++m_size;
        return insertPtr->insertAsPrev(data);
    }

    template <typename T>
    typename List<T>::ListNodePtr List<T>::InsertAfter(ListNodePtr insertPtr, const T& data)
    {
        ++m_size;
        return insertPtr->insertAsSucc(data);
    }

    template <typename T>
    T& List<T>::operator[](Rank r)
    {
        ListNodePtr targetPtr = header;
        for (int i = 0; i <= r; ++i)
        {
            targetPtr = targetPtr->succ;
        }
        return  targetPtr->data;
    }

    // template <typename T>
    // void List<T>::Traverse(std::function<void(T)> functor)
    // {
    //     for (ListNodePtr i = header->succ; i != trailer; i = i->succ)
    //     {
    //         functor<T>(i->data);
    //     }
    // }

    // template <typename T>
    // template <typename F>
    // void List<T>::Traverse(F functor)
    // {
    //     for (ListNodePtr i = header->succ; i != trailer; i = i->succ)
    //     {
    //         functor(i->data);
    //     }
    // }

    template <typename T>
    void List<T>::Init()
    {
        header = new ListNode<T>();
        trailer = new ListNode<T>();

        header->succ = trailer;
        header->prev = nullptr;

        trailer->prev = header;
        trailer->succ = nullptr;

        m_size = 0;
    }

    template <typename T>
    int List<T>::Clear()
    {
        int oldSize = m_size;
        while (m_size > 0)
        {
            Remove(header->succ);
        }
        return oldSize;
    }

    template <typename T>
    void List<T>::CopyNodes(ListNodePtr startPtr, int length)
    {
        Init();
        ListNodePtr curPtr = startPtr;
        for (int i = 0; i < length; ++i)
        {
            InsertAsLast(curPtr->data);
            curPtr = curPtr->succ;
        }
    }

    template <typename T>
    void List<T>::Mergesort(ListNodePtr startPtr, int length)
    {
    }

    template <typename T>
    void List<T>::SelectionSort(ListNodePtr startPtr, int length)
    {
        ListNodePtr head = startPtr->prev;
        ListNodePtr tail = startPtr;
        for (int i = 0; i < length; ++i)
        {
            tail = tail->succ;
        }

        int n = length;
        while (n > 1)
        {
            ListNodePtr maxPtr = SelectMax(head->succ, n);
            // 拿掉最大的节点，取其元素创建新节点插入tail前
            InsertBefore(tail, Remove(maxPtr));

            tail = tail->prev;
            n--;
        }
    }

    template <typename T>
    void List<T>::InsertionSort(ListNodePtr startPtr, int length)
    {
        ListNodePtr curPtr = startPtr;
        for (int i = 0; i < length; ++i)
        {
            InsertAfter(Search(curPtr->data, i, curPtr), curPtr->data);
            ListNodePtr nodeToFree = curPtr;
            curPtr = curPtr->succ;
            Remove(nodeToFree);
        }
    }

    template <typename T>
    typename List<T>::ListNodePtr List<T>::SelectMax(ListNodePtr startPtr, int length)
    {
        ListNodePtr maxPtr = startPtr;
        // T maxValue = startPtr->data;
        ListNodePtr curPtr = startPtr;
        for (int i = 0; i < length; ++i)
        {
            if (curPtr->data >= maxPtr->data)
            {
                maxPtr = curPtr;
            }
            curPtr = curPtr->succ;
        }

        return maxPtr;
    }

    template <typename T>
    T List<T>::Remove(ListNodePtr inPtr)
    {
        T e = inPtr->data;
        inPtr->prev->succ = inPtr->succ;
        inPtr->succ->prev = inPtr->prev;
        delete inPtr;
        m_size--;
        return  e;
    }

    template <typename T>
    typename List<T>::ListNodePtr List<T>::Search(const T& target, int n, ListNodePtr inPtr)
    {
        printf("searching for"); MyPrint(target); printf("\n");
        
        // 从后往前找，inPtr前一个是
        ListNodePtr curPtr = inPtr->prev; 
        for (int i = n - 1;
            i >= 0 &&
            target < curPtr->data; 
            --i, (curPtr = curPtr->prev))
        {
        }
        return curPtr;
    }
}

namespace 
{
    using namespace QxNS;

    template<typename T>
    void GenerateRandomList(QxNS::List<T>& outList, int n)
    {
        using ListPtr = typename  QxNS::List<T>::ListNodePtr;
        // ListPtr p =
        //   ( rand() % 2 ) ?
        //   outList.InsertAsLast ( rand() % ( T ) n * 2 ) :
        //   outList.InsertAsFirst ( rand() % ( T ) n * 2 );
        // for ( int i = 1; i < n; i++ )
        //     p = rand() % 2 ?
        //         outList.InsertAfter ( p, rand() % ( T ) n * 2 ) :
        //         outList.InsertBefore ( p, rand() % ( T ) n * 2 );
        // ListPtr startPtr = outList.InsertAsLast( rand() * 100);
        for (int i = 0; i < n; ++i)
        {
            outList.InsertAsLast( std::rand() % 100);
        }
    }


    void Test1()
    {
        std::vector<int> testVec;
        
    }
}

void TestList()
{
    using namespace QxNS;
    List<int> testList;
    GenerateRandomList(testList, 5);
    // testList.InsertAfter()
    
     // QxLogger::Print<QxNS::List<int>>(testList);
    QxLogger::Print(testList);

    // testList.Sort(SortMethod::InsertionSort);
    testList.Sort(SortMethod::SelectSort);
    QxLogger::Print(testList);
}
