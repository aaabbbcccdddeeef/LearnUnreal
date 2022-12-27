#pragma once
#include "CoreMinimal.h"

class UQxRenderSubsystem;
class FQxTestCulling;
class FQyRenderSeriesModule;

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
