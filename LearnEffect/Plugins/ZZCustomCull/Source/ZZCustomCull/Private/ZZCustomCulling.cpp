#include "ZZCustomCulling.h"


DECLARE_STATS_GROUP(TEXT("ZZCulling"), STATGROUP_ZZCulling, STATCAT_Advanced); //定义命令名称，其中STATGROUP_QxTestGroup是stat命令输入的文字， TEXT内是显示的，STATCAT_Advanced是固定写法
// DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("ZZQueryCount"), STAT_ZZQueryCount, STATGROUP_ZZCulling);
DECLARE_CYCLE_STAT(TEXT("ZZSumQueryTime"), STAT_ZZSumQueryTime, STATGROUP_ZZCulling); 

static TAutoConsoleVariable<float> CVarZZCullingThreshold(
    TEXT("r.ZZCulling.Threshold"),
    0.f,
    TEXT("zz culling threadshould, 1 indicate full screen, 0.01 means 1 percent of screen"),
    ECVF_RenderThreadSafe);
static TAutoConsoleVariable<bool> CVarZZCullingThreadSafe(
    TEXT("r.ZZCulling.ThreadSafe"),
    true,
    TEXT("Tolerance level for when montage playback position correction occurs in replays"),
    ECVF_RenderThreadSafe);
static TAutoConsoleVariable<bool> CVarUseZZCulling(
    TEXT("r.ZZCulling.Enable"),
    false,
    TEXT("ZZCulling enable "),
    ECVF_RenderThreadSafe);
static TAutoConsoleVariable<int32> CVarCullingmethod(
    TEXT("r.ZZCulling.Cullmethod"),
    1,
    TEXT("zz culling threadshould, 1 indicate full screen, 0.01 means 1 percent of screen"),
    ECVF_RenderThreadSafe);


FZZVisibilityQuery::FZZVisibilityQuery(const FSceneView& InSceneView)
    : SceneView(&InSceneView), ZZCullingThreshold(CVarZZCullingThreshold.GetValueOnAnyThread())
{
    // const float ZZCullingThreshold = ;
    CullMethod = CVarCullingmethod.GetValueOnAnyThread();
}


uint32 FZZVisibilityQuery::Release() const
{
    uint32 NewValue = (uint32)NumRefs.Decrement();
    if (NewValue == 0)
    {
        delete this;
    }
    return NewValue;
}

// #TODO 这里实现的有问题，和projection matrix相关 #TODO 学习UE4 projection matrix的构建和使用
float ZZComputeBoundsScreenSize(const FVector4& Origin, float InRadius, const FSceneView& View)
{
    float OriginViewZ = Dot3(Origin - View.ViewMatrices.GetViewOrigin(),  View.ViewMatrices.GetViewMatrix().GetScaledAxis(EAxis::X));
   
    FVector4 TestPoint(InRadius, 0, OriginViewZ, 1);
    FVector4 Result = View.ViewMatrices.GetProjectionMatrix().TransformFVector4(TestPoint);
    return (Result.X / Result.W) * 2.0f;
}

bool FZZVisibilityQuery::IsVisible(int32 VisibilityId, const FBoxSphereBounds& Bounds)
{
    check(SceneView);

    bool Test1 = FApp::ShouldUseThreadingForPerformance();
    IConsoleVariable* ParrelInitView =  IConsoleManager::Get().FindConsoleVariable(TEXT("r.ParallelInitViews"));
    bool Test2 =  ParrelInitView->GetInt() > 0;

    SCOPE_CYCLE_COUNTER(STAT_ZZSumQueryTime);
    float BoundScreenSize = 0;
    if (CullMethod > 0)
    {
        BoundScreenSize = ComputeBoundsScreenSize(Bounds.Origin, Bounds.SphereRadius, *SceneView);
    }
    else
    {
        BoundScreenSize = ZZComputeBoundsScreenSize(Bounds.Origin, Bounds.SphereRadius, *SceneView);
    }
    
    
    if (BoundScreenSize >= ZZCullingThreshold)
    {
        return true;
    }
    else
    {
        return  false;
    }
}

bool FZZVisibilityQuery::IsThreadsafe()
{
    return CVarZZCullingThreadSafe.GetValueOnAnyThread();
    // return true;
}

FZZCustomCulling::FZZCustomCulling()
{
    
}

ICustomVisibilityQuery* FZZCustomCulling::CreateQuery(const FSceneView& View)
{
    if (CVarUseZZCulling.GetValueOnAnyThread())
    {
        // 这个对象的回收由SceneView调用VisibilityQueryPtr->Release实现
        FZZVisibilityQuery* VisibilityQueryPtr = new FZZVisibilityQuery(View);
        return VisibilityQueryPtr;
    }
    else
    {
        return nullptr;
    }
    
}

