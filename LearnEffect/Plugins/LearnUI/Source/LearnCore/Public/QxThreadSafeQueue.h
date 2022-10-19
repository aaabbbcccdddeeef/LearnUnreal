#pragma once
#include <mutex>
#include <queue>

/**
 * @brief 参照cpp concurrency in action 实现的线程安全的队列，用condition variable 实现
 * 使用粗粒度锁实现的线程安全队列
 * 这里直接封装了std queue
 *  改进版本是使用shared_ptr持有数据，将内存分配移出锁的保护范围
 */
template<typename T>
class TQxThreadSafeQueue
{
public:
    TQxThreadSafeQueue() {  }

    void Push(T Data)
    {
        std::lock_guard<std::mutex> Lock(Mut);
        DataQueue.push(std::move(Data));
        DataCond.notify_one();
    }

    void WaitAndPush(T Data)
    {
        std::unique_lock<std::mutex> Lock(Mut);
        
    }

    void WaitAndPop(T& value)
    {
        std::unique_lock<std::mutex> Lock(Mut);
        DataCond.wait(
            Lock,
            [this]
            {
                return !DataQueue.empty();
            }
            );
        value = std::move(DataQueue.front());
        DataQueue.pop();
    }

    std::shared_ptr<T> WaitAndPop()
    {
        std::unique_lock<std::mutex> Lock(Mut);
        DataCond.wait(Lock, [this]
        {
            return !DataQueue.empty();
        });
        std::shared_ptr<T> res = std::make_shared<T>(std::move(DataQueue.front()));
        DataQueue.pop();
    }

    bool TryPop(T& Value)
    {
        std::lock_guard<std::mutex> Lock(Mut);
        if (DataQueue.empty())
        {
            return false;
        }
        Value = std::move(DataQueue.front());
        DataQueue.pop();
        return true;
    }
    
    std::shared_ptr<T> TryPop()
    {
        std::lock_guard<std::mutex> Lock(Mut);
        if (DataQueue.empty())
        {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res =
            std::make_shared<T>(std::move(DataQueue.front()));
        DataQueue.pop();
        return res;
    }

    bool Empty() const
    {
        std::lock_guard<std::mutex> Lock(Mut);
        return DataQueue.empty();
    }
    
private:
    mutable std::mutex Mut;
    std::queue<T> DataQueue;
    std::condition_variable DataCond;
};


template<typename T>
class TQxThreadSafeQueue2
{
private:
    struct TNode
    {
        std::shared_ptr<T> Data;
        std::unique_ptr<TNode> Next;
    };

    std::unique_ptr<TNode> Head;
    TNode* Tail;

public:
    TQxThreadSafeQueue2()
        : Head(new TNode), Tail(Head.get())
    {
    }

    TQxThreadSafeQueue2(const TQxThreadSafeQueue2& Other) = delete;
    TQxThreadSafeQueue2& operator=(const TQxThreadSafeQueue2& Other)=delete;

    std::shared_ptr<T> TryPop()
    {
        if (Head.get() == Tail)
        {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> const Res(Head->Data);
        std::unique_ptr<TNode> OldHead = std::move(Head);
        Head = std::move(OldHead->Next);
        return Res;
    }

    void Push(T NewValue)
    {
        std::shared_ptr<T> NewData =
            std::make_shared<TNode>(std::move(NewData));
        std::unique_ptr<TNode> p(new TNode);
        Tail->Data = NewData;
        TNode* const NewTail = p.get();
        Tail->Next = std::move(p);
        Tail = NewTail;
    }
};