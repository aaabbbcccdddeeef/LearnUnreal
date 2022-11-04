#pragma once
#include "CoreMinimal.h"

class FQxTestCulling;
class FQyRenderSeriesModule;

class FQxTestRenderer
{
public:
	FQxTestRenderer();
	~FQxTestRenderer();

	void Render(FPostOpaqueRenderParameters& InParameters);
private:

	FDelegateHandle RenderHandle;
	FQyRenderSeriesModule* ThisModule;

	TUniquePtr<FQxTestCulling> QxTestCulling;
};
