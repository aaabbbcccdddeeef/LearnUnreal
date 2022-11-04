#include "QxTestCulling.h"

static TAutoConsoleVariable<float> CVarQxCullingThreshold(
	TEXT("r.QxCulling.Threshold"),
	0.f,
	TEXT("Tolerance level for when montage playback position correction occurs in replays"));

bool FQxVisibilityQuery::Prepare()
{
	return true;
}

bool FQxVisibilityQuery::IsVisible(int32 VisibilityId, const FBoxSphereBounds& Bounds)
{
	const float QxCullingThreshold = CVarQxCullingThreshold.GetValueOnAnyThread();
	if (Bounds.BoxExtent.X >= QxCullingThreshold)
	{
		return true;
	}
	else
	{
		return false;
	}
}

FQxTestCulling::FQxTestCulling()
{
	QxVisibilityQuery = MakeUnique<FQxVisibilityQuery>();	
}

ICustomVisibilityQuery* FQxTestCulling::CreateQuery(const FSceneView& View)
{
	check(QxVisibilityQuery.IsValid());
	return QxVisibilityQuery.Get();
}
