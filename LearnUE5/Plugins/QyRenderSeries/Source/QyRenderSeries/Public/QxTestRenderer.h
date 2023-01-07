#pragma once
#include "CoreMinimal.h"

class UQxRenderSubsystem;
class FQxTestCulling;
class FQyRenderSeriesModule;

// 注意这个类的对象生命周期一定在QxRenderSubsystem内
class FQxTestRenderer
{
public:
	FQxTestRenderer(UQxRenderSubsystem* InQxRenderSubsystem);
	~FQxTestRenderer();

	void Render(FPostOpaqueRenderParameters& InParameters);
private:

	FDelegateHandle RenderHandle;
	// FQyRenderSeriesModule* ThisModule;

	TUniquePtr<FQxTestCulling> QxTestCulling;

	UQxRenderSubsystem* QxRenderSubsystem;
};
