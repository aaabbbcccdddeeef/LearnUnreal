#include "QxColorCorrectRegionDatabase.h"

void FQxColorCorrectRegionDatabase::RemoveCCRData(const AQxColorCorrectRegion* InCCR)
{
	if (FirstPrimitiveIds.Contains(InCCR))
	{
		FScopeLock RegionScopeLock(&FirstPrimitiveIdCriticalSection);
		FirstPrimitiveIds.Remove(InCCR);
	}
}
