// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPPExtSubSystem.h"

#include "QxRenderUtils.h"
#include "QxShaders.h"
#include "RenderGraphBuilder.h"
#include "ShaderParameterStruct.h"
#include "Renderer/Private/ScreenPass.h"

namespace 
{
	class FQxGuassianBlurPS : public FGlobalShader
	{
		DECLARE_GLOBAL_SHADER(FQxGuassianBlurPS);
		SHADER_USE_PARAMETER_STRUCT(FQxGuassianBlurPS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
			RENDER_TARGET_BINDING_SLOTS()
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, InputSize)
		END_SHADER_PARAMETER_STRUCT()
	public:
		
		static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
		{
			return true;
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxGuassianBlurPS, "/QxShaders/Blurs/QxTwoPassGuassian.usf", "MainPS", SF_Pixel);
}


void UQxPPExtSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RegisterRenderCallbacks();
}

void UQxPPExtSubSystem::Deinitialize()
{
	Super::Deinitialize();

	if (GuassuaDelegateHandle.IsValid())
	{
		GuassuaDelegateHandle.Reset();

		IRendererModule* RendererModule = FModuleManager::Get().GetModulePtr<IRendererModule>("Renderer");
		if (!RendererModule)
		{
			return;
		}
		RendererModule->RemovePostOpaqueRenderDelegate(GuassuaDelegateHandle);
	}
}

void UQxPPExtSubSystem::RenderQxGuassianBlur(FPostOpaqueRenderParameters& InParameters)
{
	
	FRDGBuilder GraphBuilder(*InParameters.RHICmdList);
	RDG_EVENT_SCOPE(GraphBuilder, "QxGuassianPass");

	FScreenPassTexture test;  
	
	// 准备需要的参数
	// InputeTexture 

	//
	// FScreenPassTexture HorizontalResult  = RenderQxGuassianOnePass(
	// 	GraphBuilder,
	// 	InParameters,
	// 	true
	// 	); 
	//
	// FScreenPassTexture GuassianResult = RenderQxGuassianOnePass(
	// 	GraphBuilder,
	// 	InParameters,
	// 	HorizontalResult.Texture,
	// 	false);
	//
	// FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(*InParameters.RHICmdList);
	// SceneContext.SetSceneColor(GuassianResult.Texture);
}

FScreenPassTexture UQxPPExtSubSystem::RenderQxGuassianOnePass(
	FRDGBuilder& GraphBuilder,
	FPostOpaqueRenderParameters& InParameters,
	FRDGTextureRef InRDGTexture,
	bool InIsHorizontal)
{
	FString PassName = FString("QxGuassian_") +
		(InIsHorizontal ?  "Horizontal": "Vertical");
	
	// 准备Shader
	auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FQxScreenPassVS> VertexShader(ShaderMap);
	TShaderMapRef<FQxGuassianBlurPS> PixelShader(ShaderMap);

	// 创建作为RenderTarget的Texture
	// 从RHICmdList得到InputTexture和RenderTarget Texture，参考TUniformBufferRef<FSceneTextureUniformParameters> CreateSceneTextureUniformBuffer
	FSceneRenderTargets& SceneContext  = FSceneRenderTargets::Get(*InParameters.RHICmdList);
	// FRDGTextureRef InputTexture 
	
	// 准备shader 参数
	FQxGuassianBlurPS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxGuassianBlurPS::FParameters>();
	PassParams->InputTexture = InRDGTexture;
	PassParams->RenderTargets[0] = FRenderTargetBinding();
	
	// AddDrawScreenPass(
	// 	GraphBuilder,
	// 	RDG_EVENT_NAME("QxGuassianBlur_%s", (InIsHorizontal ?  "Horizontal": "Vertical")),
	// 	
	// 	);
	// FRHISamplerState* BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	FRHIBlendState* ClearBlendState = TStaticBlendState<>::GetRHI();
	// QxRenderUtils::DrawShaderPass(
	// 	GraphBuilder,
	// 	PassName,
	// 	PassParams,
	// 	VertexShader,
	// 	PixelShader,
	// 	ClearBlendState,
	// 	FIntRect()
	// 	);
	return FScreenPassTexture();
}


void UQxPPExtSubSystem::RegisterRenderCallbacks()
{
	IRendererModule* RendererModule = FModuleManager::Get().GetModulePtr<IRendererModule>("Renderer");
	if (!RendererModule)
	{
		return;
	}

	
	GuassuaDelegateHandle = RendererModule->RegisterPostOpaqueRenderDelegate(
		FPostOpaqueRenderDelegate::CreateUObject(this, &UQxPPExtSubSystem::RenderQxGuassianBlur));
}
