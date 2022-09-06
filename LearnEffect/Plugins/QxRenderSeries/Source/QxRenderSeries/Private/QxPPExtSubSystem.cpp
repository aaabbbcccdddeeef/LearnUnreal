// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPPExtSubSystem.h"

#include "QxRenderUtils.h"
#include "QxShaders.h"
#include "RenderGraphBuilder.h"
#include "RenderTargetPool.h"
#include "ShaderParameterStruct.h"
#include "Renderer/Private/ScreenPass.h"

namespace 
{
	// RDG 参数版本的shader
	class FQxGuassianBlurPS_RDG : public FGlobalShader
	{
		DECLARE_GLOBAL_SHADER(FQxGuassianBlurPS_RDG);
		SHADER_USE_PARAMETER_STRUCT(FQxGuassianBlurPS_RDG, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
			RENDER_TARGET_BINDING_SLOTS()
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, InputViewportSize)
			SHADER_PARAMETER(int32, IsHorizontal)
		END_SHADER_PARAMETER_STRUCT()
	public:
		
		static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
		{
			return true;
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxGuassianBlurPS_RDG, "/QxShaders/Blurs/QxTwoPassGuassian.usf", "MainPS", SF_Pixel);

	
	class FQxGuassianBlurePS : public FGlobalShader
	{
		// 注意：下面这种方式定义shader参数是和RDG无关的，任意shader都能用，只是RDG相关的shader变量需要用RDG资源传入
		DECLARE_GLOBAL_SHADER(FQxGuassianBlurePS);
		SHADER_USE_PARAMETER_STRUCT(FQxGuassianBlurePS, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_TEXTURE(Texture2D, InputTexture)
			// RENDER_TARGET_BINDING_SLOTS()
			SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			SHADER_PARAMETER(FVector2D, InputViewportSize)
			SHADER_PARAMETER(int32, IsHorizontal)
		END_SHADER_PARAMETER_STRUCT()
	public:
		
		static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
		{
			return true;
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxGuassianBlurePS, "/QxShaders/Blurs/QxTwoPassGuassian.usf", "MainPS", SF_Pixel);
	// class FQxGuassianBlurePS : public FGlobalShader
	// {
	// 	DECLARE_GLOBAL_SHADER(FQxGuassianBlurePS);
	// public:
	// 	FQxGuassianBlurePS() {  }
	// 	FQxGuassianBlurePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	// 		: FGlobalShader(Initializer)
	// 	{
	// 		InputTexture.Bind(Initializer.ParameterMap, TEXT("InputTexture"));
	// 		InputTextureSampler.Bind(Initializer.ParameterMap, TEXT("InputTextureSampler"));
	// 		InputViewportSize.Bind(Initializer.ParameterMap, TEXT("InputViewportSize"));
	// 		IsHorizontal.Bind(Initializer.ParameterMap, TEXT("IsHorizontal"));
	// 	}
	//
	// 	// void 
	// 	
	// 	static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
	// 	{
	// 		return true;
	// 	}
	// private:
	//
	// 	LAYOUT_FIELD(FShaderResourceParameter, InputTexture);
	// 	LAYOUT_FIELD(FShaderParameter, InputTextureSampler);
	// 	LAYOUT_FIELD(FShaderParameter, InputViewportSize);
	// 	LAYOUT_FIELD(FShaderParameter, IsHorizontal);
	// };
	// IMPLEMENT_GLOBAL_SHADER(FQxGuassianBlurePS, "/QxShaders/Blurs/QxTwoPassGuassian.usf", "MainPS", SF_Pixel);
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
	if (!bRenderGuassianBlur)
	{
		return;	
	}
	// QUICK_SCOPE_CYCLE_COUNTER(STAT_ShaderPlugin_PixelShader); // Used to gather CPU profiling data for the UE4 session frontend
	SCOPED_DRAW_EVENT(*InParameters.RHICmdList, QxQuassianBlur); // Used to profile GPU activity and add metadata to be consumed by for example RenderDoc

	
	// 准备需要作为输入和输出的纹理
	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(*InParameters.RHICmdList);
	// FRDGTextureRef IntermidateTex = GraphBuilder.CreateTexture(Desc, TEXT("Guassian Intermedietate"));

	FPooledRenderTargetDesc newTexDesc = SceneContext.GetSceneColor()->GetDesc();
	newTexDesc.Reset();
	newTexDesc.ClearValue = FClearValueBinding(FLinearColor::Black);
		
	// FPooledRenderTargetDesc Desc = FPooledRenderTargetDesc::Create2DDesc(PaddedFrequencySize, PixelFormat,
	// FClearValueBinding::None, TexCreate_None, TexCreate_RenderTargetable | TexCreate_ShaderResource, false);

	// Temp buffer used at intermediate buffer when transforming the world space kernel 
	TRefCountPtr<IPooledRenderTarget> TmpRT;
	GRenderTargetPool.FindFreeElement(*InParameters.RHICmdList,
		newTexDesc,
		TmpRT,
		TEXT("Guassian Horinzontal result"));

	
	// 渲染高斯 horizontal pass
	// RenderTarget是新分配的，input texture是scene color;
	{
		RenderQxGuassianOnePass(
			InParameters,
			SceneContext.GetSceneColorTexture(),
			TmpRT->GetTargetableRHI(),
			true
			);
	}

	// 渲染高斯 vertical pass
	// render target是scene color, input texture是上面的render target
	{
		RenderQxGuassianOnePass(
			InParameters,
			TmpRT->GetShaderResourceRHI(),
			SceneContext.GetSceneColor()->GetRenderTargetItem().GetRHI(ERenderTargetTexture::Targetable),
			false
			);
	}
}

void UQxPPExtSubSystem::RenderQxGuassianBlur_RDG(FPostOpaqueRenderParameters& InParameters)
{
	FRDGBuilder GraphBuilder(*InParameters.RHICmdList);
	{
		RDG_EVENT_SCOPE(GraphBuilder, "QxGuassianPass");

		// 准备需要作为输入和输出的纹理
		FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(*InParameters.RHICmdList);
		FRDGTextureRef SceneColorTex = GraphBuilder.RegisterExternalTexture(
				SceneContext.GetSceneColor()
			);
		// Build Texture
		FRDGTextureDesc Desc = SceneColorTex->Desc;
		Desc.Reset();
		Desc.ClearValue = FClearValueBinding(FLinearColor::Black);
		// FRDGTextureRef IntermidateTex = GraphBuilder.CreateTexture(Desc, TEXT("Guassian Intermedietate"));
		
		FPooledRenderTargetDesc newTexDesc = SceneContext.GetSceneColor()->GetDesc();
		newTexDesc.Reset();
		newTexDesc.ClearValue = FClearValueBinding(FLinearColor::Black);
		
		// FPooledRenderTargetDesc Desc = FPooledRenderTargetDesc::Create2DDesc(PaddedFrequencySize, PixelFormat,
		// FClearValueBinding::None, TexCreate_None, TexCreate_RenderTargetable | TexCreate_ShaderResource, false);

		// Temp buffer used at intermediate buffer when transforming the world space kernel 
		TRefCountPtr<IPooledRenderTarget> TmpRT;
		GRenderTargetPool.FindFreeElement(*InParameters.RHICmdList,
			newTexDesc,
			TmpRT,
			TEXT("FFT Tmp Kernel Buffer"));
		FRDGTextureRef IntermidateTex = GraphBuilder.RegisterExternalTexture(
			TmpRT	
			);
		
		
		// GRenderTargetPool.FindFreeElement()
		
		//
		RenderQxGuassianOnePass_RDG(
			GraphBuilder,
			InParameters,
			SceneColorTex,
			IntermidateTex,
			true
			); 
		RenderQxGuassianOnePass_RDG(
			GraphBuilder,
			InParameters,
			IntermidateTex,
			SceneColorTex,
			false);
	}
	// FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(*InParameters.RHICmdList);
	// SceneContext.SetSceneColor(GuassianResult.Texture);

	GraphBuilder.Execute();
}

void UQxPPExtSubSystem::RenderQxGuassianOnePass_RDG(
	FRDGBuilder& GraphBuilder,
	FPostOpaqueRenderParameters& InParameters,
	FRDGTextureRef InRDGTexture,
	FRDGTextureRef TargetTexture,
	bool InIsHorizontal)
{
	FString PassName = FString("QxGuassian_") +
		(InIsHorizontal ?  "Horizontal": "Vertical");
	
	// 准备Shader
	auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FQxScreenPassVS> VertexShader(ShaderMap);
	TShaderMapRef<FQxGuassianBlurPS_RDG> PixelShader(ShaderMap);

	// 创建作为RenderTarget的Texture
	// 从RHICmdList得到InputTexture和RenderTarget Texture，参考TUniformBufferRef<FSceneTextureUniformParameters> CreateSceneTextureUniformBuffer
	FSceneRenderTargets& SceneContext  = FSceneRenderTargets::Get(*InParameters.RHICmdList);
	// FRDGTextureRef InputTexture 
	
	// 准备shader 参数
	FQxGuassianBlurPS_RDG::FParameters* PassParams = GraphBuilder.AllocParameters<FQxGuassianBlurPS_RDG::FParameters>();
	PassParams->InputTexture = InRDGTexture;
	PassParams->RenderTargets[0] = FRenderTargetBinding(TargetTexture, ERenderTargetLoadAction::ENoAction);
	PassParams->InputTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PassParams->InputViewportSize = InParameters.ViewportRect.Size();
	PassParams->IsHorizontal = InIsHorizontal;
	
	// AddDrawScreenPass(
	// 	GraphBuilder,
	// 	RDG_EVENT_NAME("QxGuassianBlur_%s", (InIsHorizontal ?  "Horizontal": "Vertical")),
	// 	
	// 	);
	// FRHISamplerState* BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	FRHIBlendState* ClearBlendState = TStaticBlendState<>::GetRHI();
	QxRenderUtils::DrawShaderPass(
		GraphBuilder,
		PassName,
		PassParams,
		VertexShader,
		PixelShader,
		ClearBlendState,
		InParameters.ViewportRect
		);
	return ;
}

void UQxPPExtSubSystem::RenderQxGuassianOnePass(FPostOpaqueRenderParameters& InParameters, FTextureRHIRef InTexture,
	FTextureRHIRef TargetTexture, bool InIsHorizontal)
{
	// 验证Texture的状态
#pragma region ValidateTextureStates

#pragma endregion
	
	FRHICommandListImmediate& RHICmdList = *InParameters.RHICmdList;

	// #TODO 这里Rendertarget action或许有更好的选择，先这么做
	FRHIRenderPassInfo GuassianRenderPassInfo(TargetTexture,
		ERenderTargetActions::Clear_DontStore);
	RHICmdList.BeginRenderPass(GuassianRenderPassInfo, InIsHorizontal ? TEXT("QxGuassianBlur_Horizontal") : TEXT("QxGuassianBlur_Vertical"));
	

	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FQxScreenPassVS> VertexShader(GlobalShaderMap);
	TShaderMapRef<FQxGuassianBlurePS> PixelShader(GlobalShaderMap);

	FRHIBlendState* BlendState =  TStaticBlendState<>::GetRHI();
	const FScreenPassPipelineState PipelineState(VertexShader, PixelShader, BlendState);
	const FIntRect& Viewport = InParameters.ViewportRect;
	RHICmdList.SetViewport(Viewport.Min.X, Viewport.Min.Y, 0.f,
					Viewport.Max.X, Viewport.Max.Y, 1.f);
	SetScreenPassPipelineState(RHICmdList, PipelineState);


	FQxGuassianBlurePS::FParameters PassParams;
	PassParams.InputTexture = InTexture;
	PassParams.InputTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PassParams.InputViewportSize = InParameters.ViewportRect.Size();
	PassParams.IsHorizontal = InIsHorizontal;
	
	SetShaderParameters(RHICmdList,
		PixelShader,
		PixelShader.GetPixelShader(),
		PassParams
		);
				
	DrawRectangle(
		RHICmdList,
		0.f, 0.f,
		Viewport.Width(), Viewport.Height(),
		Viewport.Min.X, Viewport.Min.Y,
		Viewport.Width(), Viewport.Height(),
		Viewport.Size(),
		Viewport.Size(),
		PipelineState.VertexShader,
		EDrawRectangleFlags::EDRF_Default
		);

	RHICmdList.EndRenderPass();
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
