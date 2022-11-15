#include "ZZCustomCulling.h"

static TAutoConsoleVariable<float> CVarZZCullingThreshold(
    TEXT("r.ZZCulling.Threshold"),
    0.f,
    TEXT("zz culling threadshould"));
static TAutoConsoleVariable<bool> CVarZZCullingThreadSafe(
    TEXT("r.ZZCulling.ThreadSafe"),
    true,
    TEXT("Tolerance level for when montage playback position correction occurs in replays"));
static TAutoConsoleVariable<bool> CVarUseZZCulling(
    TEXT("r.ZZCulling.Enable"),
    false,
    TEXT("ZZCulling enable "));


FZZVisibilityQuery::FZZVisibilityQuery(const FSceneView& InSceneView)
    : SceneView(&InSceneView)
{
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

bool FZZVisibilityQuery::IsVisible(int32 VisibilityId, const FBoxSphereBounds& Bounds)
{
    check(SceneView);


    float BoundScreenSize = ComputeBoundsScreenSize(Bounds.Origin, Bounds.SphereRadius, *SceneView);
    const float ZZCullingThreshold = CVarZZCullingThreshold.GetValueOnAnyThread();
    // if (Bounds.BoxExtent.X >= ZZCullingThreshold)
    // {
    //     return true;
    // }
    // else
    // {
    //     return false;
    // }
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
    // return CVarZZCullingThreadSafe.GetValueOnAnyThread();
    return true;
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

