#pragma once
#include "CoreMinimal.h"

/**
 * @brief 实现自定义Query，#TODO 先只实现单线程的版本
 * RefCount 部分实现参考FRenderAssetUpdate
 */
class FZZVisibilityQuery : public ICustomVisibilityQuery
{
public:
    FZZVisibilityQuery(const FSceneView& InSceneView);


    virtual uint32 AddRef() const override
    {
        return (uint32)NumRefs.Increment();
    };
    virtual uint32 Release() const override;;
    virtual uint32 GetRefCount() const override
    {
        return (uint32)NumRefs.GetValue();
    };
    virtual bool Prepare() override
    {
        return true;
    };
    virtual bool IsVisible(int32 VisibilityId, const FBoxSphereBounds& Bounds) override;
    virtual bool IsThreadsafe() override;;

private:
    // Ref Counting
    mutable  FThreadSafeCounter NumRefs  = 0;

    const FSceneView* const SceneView ;
};


class FZZCustomCulling : public ICustomCulling
{
public:
    FZZCustomCulling();
    virtual ICustomVisibilityQuery* CreateQuery(const FSceneView& View) override;

    // TUniquePtr<FZZVisibilityQuery> ZZVisibilityQuery = nullptr;
};
