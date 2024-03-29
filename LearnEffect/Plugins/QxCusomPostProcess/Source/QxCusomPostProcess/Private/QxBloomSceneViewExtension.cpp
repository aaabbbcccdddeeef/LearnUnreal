﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "QxBloomSceneViewExtension.h"

#include <stdexcept>

#include "PixelShaderUtils.h"
#include "QxBlurs.h"
#include "QxLensFlareAsset.h"
#include "QxPostProcessBloom.h"
#include "QxPostprocessSubsystem.h"
#include "QxRenderPPUtils.h"
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

	if (PassId == EPostProcessingPass::Tonemap)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(
			this, &FQxBloomSceneViewExtension::RenderQxGuassianBlur
			));
	}
}

BEGIN_SHADER_PARAMETER_STRUCT(FQxCopyTextureParameters, )
	RDG_TEXTURE_ACCESS(Input,  ERHIAccess::CopySrc)
	// RDG_TEXTURE_ACCESS(Output, ERHIAccess::CopyDest)
END_SHADER_PARAMETER_STRUCT()

namespace 
{
	// extent 当前纹理占用的大小， Inrect是当前viewport占用的空间
	// 注意，返回的是标准uv空间下的,例如如果视口不变,则一般返回的是[0-1],其它根据放大缩小变化
	FVector2D GetInputViewportSize(const FIntRect& InRect, const FIntPoint& Extent)
	{
		// Based on
		// GetScreenPassTextureViewportParameters()
		// Engine/Source/Runtime/Renderer/Private/ScreenPass.cpp

		FVector2D ExtentInverse = FVector2D(1.f/ Extent.X, 1.f / Extent.Y);

		FVector2D RectMin = FVector2D(InRect.Min);
		FVector2D RectMax = FVector2D(InRect.Max);

		FVector2D Min = RectMin * ExtentInverse;
		FVector2D Max = RectMax * ExtentInverse;
		return (Max - Min);
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
	FScopeLock PostSettingLock(&QxPostprocessSubsystem->PostAssetMutex);
	// UE_LOG(LogTemp, Warning, TEXT("tEST Qxbloom"));
	//
	const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);

#if WITH_EDITOR
	const FScreenPassTextureViewport SceneColorViewport(SceneColor);

	// rect标识占用viewport的大小, extent表示纹理的大小，当拖动修改viewport后这两个会不一致
	if ((SceneColorViewport.Rect.Width() != SceneColorViewport.Extent.X) ||
		(SceneColorViewport.Rect.Height() != SceneColorViewport.Extent.Y))
	{
		FVector2D SceneColorViewportSize = GetInputViewportSize(SceneColorViewport.Rect, SceneColorViewport.Extent);
		const FString PassName("QxRescale");

		FRDGTextureDesc TextureDesc = SceneColor.Texture->Desc;
		TextureDesc.Reset();
		TextureDesc.Extent = SceneColorViewport.Rect.Size();
		TextureDesc.ClearValue = FClearValueBinding(FLinearColor::Transparent);
		FRDGTextureRef RescaledTexture = GraphBuilder.CreateTexture(TextureDesc, TEXT("QxRescaledSceneColor"));

		
		TShaderMapRef<FQxRescaleShader> PixelShader(ViewInfo.ShaderMap);

		FQxRescaleShader::FParameters* PassParams = GraphBuilder.AllocParameters<FQxRescaleShader::FParameters>();
		PassParams->Pass.InputTexture = SceneColor.Texture;
		PassParams->Pass.InputTextureSampler = BilinearClampSampler;
		PassParams->Pass.RenderTargets[0] = FRenderTargetBinding(RescaledTexture, ERenderTargetLoadAction::ENoAction);
		PassParams->InputViewportSize = SceneColorViewportSize;
		
		
		// FPixelShaderUtils::AddFullscreenPass(
		// 	GraphBuilder,
		// 	ViewInfo.ShaderMap,
		// 	RDG_EVENT_NAME("QxRescalePass"),
		// 	PixelShader,
		// 	PassParams,
		// 	SceneColorViewport.Rect
		// 	);
		QxRenderPPUtils::QxDrawScreenPass(
			GraphBuilder,
			TEXT("QxRescalePass"),
			PixelShader,
			PassParams,
			SceneColorViewport.Rect
			);
		

		SceneColor.Texture = RescaledTexture;
		SceneColor.ViewRect = SceneColorViewport.Rect;
	}
#endif
	
	
	// 进行一次downsample 给后面的pass用
	FScreenPassTexture HalResSceneColor;
	{
		FQxDownSampleInputs DownSampleInputs;
		DownSampleInputs.Name = TEXT("QXPostProcessing.SceneColor.HalfRes");
		DownSampleInputs.SceneColor = SceneColor;
		DownSampleInputs.Quality = EQxDownampleQuality::High;
		DownSampleInputs.FormatOverride = PF_FloatRGB;
		HalResSceneColor = QxRenderPPUtils::AddQxDownSamplePass(
			GraphBuilder,
			ViewInfo,
			DownSampleInputs
			);
	}
	FScreenPassTexture EyeAdaptationTexture = RenderEyeAdaptation(
		GraphBuilder,
		ViewInfo,
		SceneColor,
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
		SceneColor,
		QxPostprocessSubsystem->GetBloomSettingAsset());
	
	DownSampleTextures.Empty();
	UpSampleTextures.Empty();


	// 测试用Draw Canvas Pass绘制一些内容
	if (QxPostprocessSubsystem->GetBloomSettingAsset()->bEnableCanvasDraw)
	{
		FScreenPassRenderTarget TmpOutPut(BloomTexture, ERenderTargetLoadAction::ENoAction);
		
		AddDrawCanvasPass(
			GraphBuilder,
			RDG_EVENT_NAME("TestDrawCanvas"),
			ViewInfo,
			TmpOutPut,
			[](FCanvas& DrawCanvas)
			{
				float X,Y,SizeX,SizeY;
				X = Y = 100;
				SizeX = SizeY = 300;
				FLinearColor SelectionColor = FLinearColor::Blue;
				DrawCanvas.DrawTile(X, Y, SizeX, SizeY, 0, 0, 0, 0, SelectionColor);
			}
			);
		//
		// // FScreenPassTexture CanvasOut = SceneColor;
		// DrawCanvasPass(
		// 	GraphBuilder.RHICmdList,
		// 	ViewInfo,
		// 	BloomTexture,
		// 	[](FCanvas& DrawCanvas)
		// 	{
		// 		float X,Y,SizeX,SizeY;
		// 		X = Y = 100;
		// 		SizeX = SizeY = 300;
		// 		FLinearColor SelectionColor = FLinearColor::Blue;
		// 		DrawCanvas.DrawTile(X, Y, SizeX, SizeY, 0, 0, 0, 0, SelectionColor);
		// 	}
		// 	);
	}
	
	// #TODO
	// My Tone Mapping

	// 测试修改view rect 调整view port的位置
	if (QxPostprocessSubsystem->GetBloomSettingAsset()->bEnableCustomViewport)
	{
		const FScreenPassTexture& TargetInputTexture = BloomTexture;
		FIntRect OriginRect = TargetInputTexture.ViewRect;
		FIntRect TestViewRect = FIntRect(
		100, 100,
		500, 500
		);

		

		FRDGTextureDesc TexDesc = TargetInputTexture.Texture->Desc;
		TexDesc.Reset();
		TexDesc.Extent = FIntPoint(400, 400);
		FRDGTextureRef tempTex =  GraphBuilder.CreateTexture(TexDesc, TEXT("TempShrinkTexture"));

		AddCopyTexturePass(
			GraphBuilder,
			TargetInputTexture.Texture,
			tempTex,
			FIntPoint::ZeroValue,
			FIntPoint::ZeroValue,
			TexDesc.Extent
			);

		AddClearRenderTargetPass(
			GraphBuilder,
			TargetInputTexture.Texture,
			FLinearColor::Black
			);
		
		AddDrawTexturePass(
			GraphBuilder,
			ViewInfo,
			tempTex,
			TargetInputTexture.Texture,
			FIntPoint::ZeroValue,
			FIntPoint(100, 100),
			FIntPoint(400, 400)
			);
	}
	
	// FIntRect TestViewRect = FIntRect(
	// 	100, 100,
	// 	500, 500
	// 	);
	// BloomTexture.ViewRect = TestViewRect;
	return BloomTexture;
}


FScreenPassTexture FQxBloomSceneViewExtension::RenderQxGuassianBlur(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessMaterialInputs& PostProcessMaterialInputs)
{
	FScreenPassTexture SceneColor = PostProcessMaterialInputs.GetInput(EPostProcessMaterialInput::SceneColor);
	if (nullptr == QxPostprocessSubsystem->GetBloomSettingAsset() ||
		!QxPostprocessSubsystem->GetBloomSettingAsset()->bEnableQxGuassianBlur)
	{
		return SceneColor;
	}
	FScopeLock PostSettingLock(&QxPostprocessSubsystem->PostAssetMutex);
	const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);

	

	FScreenPassTexture BluredTexture =
		QxRenderPPUtils::RenderQxGuassianBlur(GraphBuilder, ViewInfo, SceneColor, true);
	
	return BluredTexture;
}


