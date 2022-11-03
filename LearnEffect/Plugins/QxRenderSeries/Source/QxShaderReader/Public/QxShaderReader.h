#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(QxShaderReader, All, All);


BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FQxTestGlobal2,)
	SHADER_PARAMETER(uint32, TestCount)
	SHADER_PARAMETER(float, TestValue)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

class FQxShaderReader : public IModuleInterface
{
	public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

	void InitTestGlobalUniform();

	void InitTestGlobalUniform_RenderThread();

	TUniformBufferRef<FQxTestGlobal2> CachedZZClipVolumeParams;
};