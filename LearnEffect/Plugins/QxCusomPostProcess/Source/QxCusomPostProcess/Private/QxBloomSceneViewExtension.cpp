// Fill out your copyright notice in the Description page of Project Settings.


#include "QxBloomSceneViewExtension.h"

#include <stdexcept>

#include "QxLensFlareAsset.h"
#include "QxPostProcessBloom.h"
#include "QxPostprocessSubsystem.h"
#include "RenderTargetPool.h"
#include "ScreenPass.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PostProcess/PostProcessMaterial.h"

FQxBloomSceneViewExtension::FQxBloomSceneViewExtension(const FAutoRegister& AutoRegister,
	UQxPostprocessSubsystem* InSubsystem)
		: FSceneViewExtensionBase(AutoRegister), QxPostprocessSubsystem(InSubsystem)
{
	check(QxPostprocessSubsystem);

	InitRenderStates();
}

FQxBloomSceneViewExtension::~FQxBloomSceneViewExtension()
{
	QxPostprocessSubsystem = nullptr;
	ClearBlendState = nullptr;
	AdditiveBlendState = nullptr;

	BilinearBorderSampler = nullptr;
	BilinearClampSampler = nullptr;
	BilinearRepeatSampler = nullptr;
}

void FQxBloomSceneViewExtension::InitRenderStates()
{
	if (ClearBlendState)
	{
		return;
	}

	// Blend modes from:
	// '/Engine/Source/Runtime/RenderCore/Private/ClearQuad.cpp'
	// '/Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessMaterial.cpp'
	ClearBlendState = TStaticBlendState<>::GetRHI();
	AdditiveBlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_One>::GetRHI();

	BilinearClampSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	BilinearBorderSampler = TStaticSamplerState<SF_Bilinear, AM_Border, AM_Border, AM_Border>::GetRHI();
	BilinearRepeatSampler = TStaticSamplerState<SF_Bilinear, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
	NearestRepeatSampler = TStaticSamplerState<SF_Point, AM_Wrap, AM_Wrap, AM_Wrap>::GetRHI();
}



void FQxBloomSceneViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId,
                                                               FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
	// FSceneViewExtensionBase::SubscribeToPostProcessingPass(Pass, InOutPassCallbacks, bIsPassEnabled);
	// 当前来看插入到这个pass之后是最合适的
	if (PassId == EPostProcessingPass::MotionBlur)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(
			this, &FQxBloomSceneViewExtension::RenderQxBloom_RenderThread));
	}
}

BEGIN_SHADER_PARAMETER_STRUCT(FQxCopyTextureParameters, )
	RDG_TEXTURE_ACCESS(Input,  ERHIAccess::CopySrc)
	// RDG_TEXTURE_ACCESS(Output, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

FScreenPassTexture FQxBloomSceneViewExtension::RenderQxBloom_RenderThread(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View, const FPostProcessMaterialInputs& PPMaterialInputs)
{
	FScreenPassTexture SceneColor = PPMaterialInputs.GetInput(EPostProcessMaterialInput::SceneColor);
	if (nullptr == QxPostprocessSubsystem->GetBloomSettingAsset() ||
		!QxPostprocessSubsystem->GetBloomSettingAsset()->bEnableQxPPEffect)
	{
		return SceneColor;
	}
	// UE_LOG(LogTemp, Warning, TEXT("tEST Qxbloom"));
	//
	const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);

	// 进行一次downsample 给后面的pass用
	FScreenPassTexture HalResSceneColor;
	{
		FQxDownSampleInputs DownSampleInputs;
		DownSampleInputs.Name = TEXT("QXPostProcessing.SceneColor.HalfRes");
		DownSampleInputs.SceneColor = SceneColor;
		DownSampleInputs.Quality = EQxDownampleQuality::High;
		DownSampleInputs.FormatOverride = PF_FloatRGB;
		HalResSceneColor = QxRenderUtils::AddQxDownSamplePass(
			GraphBuilder,
			ViewInfo,
			DownSampleInputs
			);
	}
	FScreenPassTexture EyeAdaptationTexture = RenderEyeAdaptation(
		GraphBuilder,
		ViewInfo,
		PPMaterialInputs,
		HalResSceneColor
		);

	// 将EyeAdaptationTexture 拷贝到dump中
	if (QxPostprocessSubsystem->QxEyeAdaptationDump != nullptr)
	{
		UTextureRenderTarget2D* dumpRT = QxPostprocessSubsystem->QxEyeAdaptationDump;
		
		FRHITexture2D* dumTexRHI =
			QxPostprocessSubsystem->QxEyeAdaptationDump->Resource->GetTexture2DRHI();
		FQxCopyTextureParameters* Parameters = GraphBuilder.AllocParameters<FQxCopyTextureParameters>();
		Parameters->Input = EyeAdaptationTexture.Texture;
		
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("QxEyeAdaptionDump"),
			Parameters,
			ERDGPassFlags::Copy | ERDGPassFlags::NeverCull,
			[Parameters, dumTexRHI](FRHICommandListImmediate& RHICmdList)
			{
				// FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;
				FRHICopyTextureInfo CopyInfo;
				RHICmdList.Transition(FRHITransitionInfo(dumTexRHI, ERHIAccess::Unknown, ERHIAccess::CopyDest));
				FRHITexture* EyeAdaptationRHI = Parameters->Input.GetTexture()->GetRHI();
				RHICmdList.CopyTexture( EyeAdaptationRHI,
					dumTexRHI, CopyInfo);
			}
			);
	}
	
	FScreenPassTexture BloomTexture = RenderBloomFlare(
		GraphBuilder,
		ViewInfo,
		PPMaterialInputs,
		QxPostprocessSubsystem->GetBloomSettingAsset());
	
	DownSampleTextures.Empty();
	UpSampleTextures.Empty();
	
	
	// #TODO
	// My Tone Mapping
	return BloomTexture;
}


