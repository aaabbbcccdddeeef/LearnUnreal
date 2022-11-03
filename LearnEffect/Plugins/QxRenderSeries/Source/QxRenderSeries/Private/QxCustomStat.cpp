// Fill out your copyright notice in the Description page of Project Settings.


#include "QxCustomStat.h"

// DECLARE_STAT_GROUP(TEXT("QxTestGroup"), STATGROUP_LODZERO, STATCAT_Advanced);
DECLARE_STATS_GROUP(TEXT("QxTestGroup1"), STATGROUP_QxTestGroup, STATCAT_Advanced); //定义命令名称，其中STATGROUP_QxTestGroup是stat命令输入的文字， TEXT内是显示的，STATCAT_Advanced是固定写法
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("qx stat count"), STAT_COUNT, STATGROUP_QxTestGroup);
DECLARE_CYCLE_STAT(TEXT("qx stat time"), STAT_MyTIME, STATGROUP_QxTestGroup); //统计时间, STAT_MyTIME 是宏名和SCOPE_CYCLE_COUNTER关联


TAutoConsoleVariable<int32> CVarQxTest(
TEXT("r.QxRender.TestCar"),
0,
TEXT("Number of passes to render bloom, override bloom asset setting"),
ECVF_RenderThreadSafe
);

// Sets default values
AQxCustomStat::AQxCustomStat()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxCustomStat::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQxCustomStat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SCOPE_CYCLE_COUNTER(STAT_MyTIME);
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("QxCustomStatTick")
	FakeTimeConsumingFunc( TickMaxCounter);

}

void AQxCustomStat::TestQuickState()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_TestQuickState);
	FakeTimeConsumingFunc( TestQuickStateMaxCounter);
}

void AQxCustomStat::FakeTimeConsumingFunc(int32 MaxCounter)
{
	for (int i = 0; i < MaxCounter; i++)
	{
		int a = 1 + i;
	}

	// Async()
	// AsyncTask()
	// AsyncThread()
	// AsyncPool()
}

