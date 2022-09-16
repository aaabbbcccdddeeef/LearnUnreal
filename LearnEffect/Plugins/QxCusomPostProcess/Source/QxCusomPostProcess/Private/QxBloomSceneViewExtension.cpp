// Fill out your copyright notice in the Description page of Project Settings.


#include "QxBloomSceneViewExtension.h"

#include <stdexcept>

#include "QxLensFlareAsset.h"
#include "QxPostProcessBloom.h"
#include "QxPostprocessSubsystem.h"
#include "RenderTargetPool.h"
#include "ScreenPass.h"
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
		FRHITexture2D* dumTexRHI =
			QxPostprocessSubsystem->QxEyeAdaptationDump->Resource->GetTexture2DRHI();
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("QxEyeAdaptionDump"),
			ERDGPassFlags::Copy | ERDGPassFlags::NeverCull,
			[EyeAdaptationTexture, dumTexRHI](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				RHICmdList.CopyTexture( EyeAdaptationTexture.Texture->GetRHI(),
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


