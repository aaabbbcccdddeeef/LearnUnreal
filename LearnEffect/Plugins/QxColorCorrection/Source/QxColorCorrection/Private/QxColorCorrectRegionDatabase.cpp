#include "QxColorCorrectRegionDatabase.h"

#include <stdexcept>

FCriticalSection FQxColorCorrectRegionDatabase::FirstPrimitiveIdCriticalSection;
TMap<const AQxColorCorrectRegion*, FPrimitiveComponentId> FQxColorCorrectRegionDatabase::FirstPrimitiveIds;
void FQxColorCorrectRegionDatabase::RemoveCCRData(const AQxColorCorrectRegion* InCCR)
{
	if (FirstPrimitiveIds.Contains(InCCR))
	{
		FScopeLock RegionScopeLock(&FirstPrimitiveIdCriticalSection);
		FirstPrimitiveIds.Remove(InCCR);
	}
}

void FQxColorCorrectRegionDatabase::UpdateCCRDatabaseFirstComponentId(AQxColorCorrectRegion* InCCR,
	FPrimitiveComponentId ComponentId)
{
	if (!FirstPrimitiveIds.Contains(InCCR))
	{
		FScopeLock RegionScopeLock(&FirstPrimitiveIdCriticalSection);

		FirstPrimitiveIds.Add(InCCR, ComponentId);
	}
	else if (!(FirstPrimitiveIds[InCCR] == ComponentId))
	{
		FScopeLock RegionScopeLock(&FirstPrimitiveIdCriticalSection);
		FirstPrimitiveIds[InCCR] = ComponentId;
	}
}

FPrimitiveComponentId FQxColorCorrectRegionDatabase::GetFirstComponentId(const AQxColorCorrectRegion* InCCR)
{
	FScopeLock RegionScopeLock(&FirstPrimitiveIdCriticalSection);
	if (FirstPrimitiveIds.Contains(InCCR))
	{
		return FirstPrimitiveIds[InCCR];
	}
	return FPrimitiveComponentId();
}
