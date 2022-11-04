#pragma once
#include "CoreMinimal.h"


// 注意：这个类不是线程安全，只能在渲染线程访问
class FQxVisibilityQuery : public ICustomVisibilityQuery
{
public:
	virtual uint32 AddRef() const override
	{
		return  ++NumRefs;
	};
	
	virtual uint32 Release() const override
	{
		return --NumRefs;
	}
	virtual uint32 GetRefCount() const override
	{
		return NumRefs;
	}
	virtual bool Prepare() override;
	virtual bool IsVisible(int32 VisibilityId, const FBoxSphereBounds& Bounds) override;

private:
	mutable  uint32 NumRefs;
};

class FQxTestCulling : public ICustomCulling
{
public:
	FQxTestCulling();

	virtual ICustomVisibilityQuery* CreateQuery(const FSceneView& View) override;

	TUniquePtr<FQxVisibilityQuery> QxVisibilityQuery;
};
