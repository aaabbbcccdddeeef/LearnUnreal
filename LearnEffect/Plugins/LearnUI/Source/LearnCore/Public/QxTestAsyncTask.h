#pragma once

/**
 * @brief 当线程被销毁时，会调用Abandon函数
 * 继承FNonAbandonableTask的话这时候就不会丢弃而等待执行完成
 */
class FQxTestAsyncTask : public FNonAbandonableTask
{
    friend class FAsyncTask<FQxTestAsyncTask>;
    friend class FAutoDeleteAsyncTask<FQxTestAsyncTask>;
public:
    FQxTestAsyncTask(int32 Target)
        : TargetCounter(Target)
    {
    }

    void DoWork();

    FORCEINLINE TStatId GetStatId()  const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(MySyncTask, STATGROUP_ThreadPoolAsyncTasks);
    }
    
    const int32 TargetCounter;
    
};
