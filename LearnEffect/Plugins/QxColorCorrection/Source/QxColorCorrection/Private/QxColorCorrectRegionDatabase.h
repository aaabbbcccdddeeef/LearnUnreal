#pragma once

class AQxColorCorrectRegion;

class FQxColorCorrectRegionDatabase
{
public:

	static void RemoveCCRData(const AQxColorCorrectRegion* InCCR);

private:
	// 控制FirstPrimitiveIds访问的锁
	static FCriticalSection FirstPrimitiveIdCriticalSection;
	
	/**
	 * @brief 保存每个region 的第一个primtive id,
	 * 渲染线程使用这个判断CCR是否需要隐藏
	 */
	static TMap<const AQxColorCorrectRegion*, FPrimitiveComponentId> FirstPrimitiveIds;
};
