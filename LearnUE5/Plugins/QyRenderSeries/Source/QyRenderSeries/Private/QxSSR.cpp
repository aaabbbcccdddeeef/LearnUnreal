#include "QxSSR.h"

#include "GlobalShader.h"
#include "PixelShaderUtils.h"
#include "SceneRenderTargetParameters.h"
#include "SceneView.h"
#include "Renderer/Private/SceneRendering.h"

// #include "SceneRenderTargetParameters.h"

namespace 
{
	class FQxSSRPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxSSRPS);
		SHADER_USE_PARAMETER_STRUCT(FQxSSRPS, FGlobalShader);

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}

		static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
		{
			FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		}

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER(FLinearColor, SSRParams)
			// SHADER_PARAMETER_STRUCT_REF(FSceneTextureUniformParameters, SceneTextures)
			SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneTextureUniformParameters, SceneTextures)
			SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, ViewUniformBuffer)
			RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()
	};

	IMPLEMENT_GLOBAL_SHADER(FQxSSRPS, "/QxShaders/QxSSR.usf", "QxSSRPS", SF_Pixel);
}

namespace QxScreenSpaceReflection
{

void RenderQxSSR(FPostOpaqueRenderParameters& InParameters,  FRDGTextureRef ExternalSSROutput)
{
	FRDGBuilder& GraphBuilder = *(InParameters.GraphBuilder);
	const FViewInfo& View = *InParameters.View;
	

	// 分配QxSSR 纹理
	FRDGTextureRef QxSSROuput = nullptr;
	{
		FRDGTextureDesc Desc = FRDGTextureDesc::Create2D(
			GetSceneTextureExtent(),
			PF_FloatRGBA, FClearValueBinding(FLinearColor(0, 0, 0, 0)),
			TexCreate_ShaderResource | TexCreate_RenderTargetable
			);

		QxSSROuput = GraphBuilder.CreateTexture(Desc, TEXT("QxSSROutput"));
	}

	FQxSSRPS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxSSRPS::FParameters>();
	// Setup PassParams
	{
		PassParams->SSRParams = FLinearColor::Red;
		PassParams->ViewUniformBuffer = View.ViewUniformBuffer;
		PassParams->SceneTextures = InParameters.SceneTexturesUniformParams;
		
		PassParams->RenderTargets[0] = FRenderTargetBinding(ExternalSSROutput, ERenderTargetLoadAction::ENoAction);
	}


	TShaderMapRef<FQxSSRPS> PixelShader(View.ShaderMap);

	ClearUnusedGraphResources(PixelShader, PassParams);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("QxSSRPass"),
		PassParams,
		ERDGPassFlags::Raster,
		[PassParams, &View, PixelShader ](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(View.ViewRect.Min.X, View.ViewRect.Min.Y, 0.f,
				View.ViewRect.Max.X, View.ViewRect.Max.Y, 0.f);
			
			FGraphicsPipelineStateInitializer GraphicPSOInit;
			FPixelShaderUtils::InitFullscreenPipelineState(RHICmdList, View.ShaderMap, PixelShader, GraphicPSOInit);
			// 这里这个0x80 抄自RenderScreenSpaceReflections， 为什么 #TODO
			SetGraphicsPipelineState(RHICmdList, GraphicPSOInit, 0x80);
			SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PassParams);
			FPixelShaderUtils::DrawFullscreenTriangle(RHICmdList);
		}
		);
}
}

