#pragma once
#include "QxColorCorrectRegion.h"

class AQxColorCorrectRegion;

class FQxColorCorrectRegionDatabase
{
public:

	
	/**
	 * @brief cleanup associated data when CCR is removed from the scene
	 * @param InCCR 
	 */
	static void RemoveCCRData(const AQxColorCorrectRegion* InCCR);

	/**
	 *	保存first component ids statically in a map with CCR pointer as a key.
	 *	Thread safe
	 */
	static void UpdateCCRDatabaseFirstComponentId(
		AQxColorCorrectRegion* InCCR,
		FPrimitiveComponentId ComponentId);

	/**
	 *
	 * 在渲染线程中用来验证primitive component id
	 * 它在当前view是否隐藏。 thread safe
	 */
	static FPrimitiveComponentId GetFirstComponentId(const AQxColorCorrectRegion* InCCR);
	
private:
	// 控制FirstPrimitiveIds访问的锁
	static FCriticalSection FirstPrimitiveIdCriticalSection;
	
	/**
	 * @brief 保存每个region 的第一个primtive id,
	 * 渲染线程使用这个判断CCR是否需要隐藏
	 */
	static TMap<const AQxColorCorrectRegion*, FPrimitiveComponentId> FirstPrimitiveIds;
};
