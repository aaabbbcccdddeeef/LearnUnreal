#include "QxShaderReader.h"

DEFINE_LOG_CATEGORY(QxShaderReader);

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FQxTestGlobal2, "QxTestGlobal2");

#define LOCTEXT_NAMESPACE "FQxShaderReader"

void FQxShaderReader::StartupModule()
{
	UE_LOG(QxShaderReader, Warning, TEXT("QxShaderReader module has been loaded"));

	// 需要保证渲染线程初始化完成，在这个之前
	// InitTestGlobalUniform();
}

void FQxShaderReader::ShutdownModule()
{
	UE_LOG(QxShaderReader, Warning, TEXT("QxShaderReader module has been unloaded"));
}

void FQxShaderReader::InitTestGlobalUniform()
{
	ENQUEUE_RENDER_COMMAND(InitTestGlobalUniform)(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			InitTestGlobalUniform_RenderThread();
		});
}

void FQxShaderReader::InitTestGlobalUniform_RenderThread()
{
	FQxTestGlobal2 TestParam;
	TestParam.TestCount = 23;
	TestParam.TestValue = 789.222;
	
	// CachedZZClipVolumeParams = TUniformBuffer<FZZClippingVolumeParameters>::GetUniformBufferRef()
	CachedZZClipVolumeParams = TUniformBufferRef<FQxTestGlobal2>::CreateUniformBufferImmediate(
		TestParam, UniformBuffer_MultiFrame, EUniformBufferValidation::None);
	
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxShaderReader, QxShaderReader)