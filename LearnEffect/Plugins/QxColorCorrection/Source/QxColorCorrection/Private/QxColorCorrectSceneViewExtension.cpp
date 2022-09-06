// Fill out your copyright notice in the Description page of Project Settings.


#include "QxColorCorrectSceneViewExtension.h"

#include "QxColorCorrectRegion.h"
#include "QxColorCorrectRegionDatabase.h"
#include "QxColorCorrectSubsystem.h"
#include "PostProcess/PostProcessing.h"

namespace 
{

	bool ViewSupportsRegions(const FSceneView& View)
	{
		return View.Family->EngineShowFlags.PostProcessing &&
			View.Family->EngineShowFlags.PostProcessMaterial;
	}

	FScreenPassTextureViewportParameters GetTextureViewportParameters(
		const FScreenPassTextureViewport& InTexViewport)
	{
		FScreenPassTextureViewportParameters Parameters;

		if (InTexViewport.IsEmpty())
		{
			return  Parameters;	
		}
		
		const FVector2D Extent(InTexViewport.Extent);
		const FVector2D ViewportMin(InTexViewport.Rect.Min.X, InTexViewport.Rect.Min.Y);
		const FVector2D ViewportMax(InTexViewport.Rect.Max.X,
			InTexViewport.Rect.Max.Y);
		const FVector2D ViewportSize = ViewportMax - ViewportMin;

		Parameters.Extent = Extent;
		Parameters.ExtentInverse =
			FVector2D(1.f / Extent.X, 1.f / Extent.Y);

		Parameters.ScreenPosToViewportScale =
			FVector2D(0.5f, -0.5f) * ViewportSize;
		Parameters.ScreenPosToViewportBias =
			(0.5 * ViewportSize) + ViewportMin;

		Parameters.ViewportMin = InTexViewport.Rect.Min;
		Parameters.ViewportMax = InTexViewport.Rect.Max;

		Parameters.ViewportSize = ViewportSize;
		Parameters.ViewportSizeInverse =
			FVector2D(1.f / Parameters.ViewportSize.X,
				1.f / Parameters.ViewportSize.Y);

		Parameters.UVViewportMin = ViewportMin * Parameters.ExtentInverse;
		Parameters.UVViewportMax =
			ViewportMax * Parameters.ExtentInverse;


		Parameters.UVViewportSize =
			Parameters.UVViewportMax - Parameters.UVViewportMin;
		Parameters.UVViewportSizeInverse =
			FVector2D(1.f / Parameters.UVViewportSize.X,
				1.f / Parameters.UVViewportSize.Y);

		Parameters.UVViewportBilinearMin =
			Parameters.UVViewportMin + 0.5f * Parameters.ExtentInverse;
		Parameters.UVViewportMax =
			Parameters.UVViewportMax - 0.5f * Parameters.ExtentInverse;
	}

	void GetPixelSpaceBoundingRect(
		const FSceneView& InView,
		const FVector& InBoxCenter,
		const FVector& InBoxExtents,
		FIntRect& OutViewport,
		float& OutMaxDepth,
		float& OutMinDepth
		)
	{
		OutViewport = FIntRect(INT32_MAX, INT32_MAX,
			-INT32_MAX, -INT32_MAX);
		// 8 corners of the bounding box. To be multiplied by box extent and offset by the center.
		const int NumCorners = 8;
		const FVector Verts[NumCorners] = {
			FVector(1, 1, 1),
			FVector(1, 1,-1),
			FVector(1,-1, 1),
			FVector(1,-1,-1),
			FVector(-1, 1, 1),
			FVector(-1, 1,-1),
			FVector(-1,-1, 1),
			FVector(-1,-1,-1) };

		for (int32 Index = 0; Index < NumCorners; ++Index)
		{
			// project bounding box vertices into screen space
			const FVector WorldVert =
				InBoxCenter + (Verts[Index] * InBoxExtents);
			FVector2D PixelVert;
			FVector4 ScreenSpaceCoordinate  =
				InView.WorldToScreen(WorldVert);

			OutMaxDepth =
				FMath::Max<float>(ScreenSpaceCoordinate.W, OutMaxDepth);
			OutMinDepth =
				FMath::Min<float>(ScreenSpaceCoordinate.W, OutMinDepth);

			if (InView.ScreenToPixel(ScreenSpaceCoordinate, PixelVert))
			{
				// Update screen-space bounding box with transformed vert
				OutViewport.Min.X =
					FMath::Min<int32>(OutViewport.Min.X, PixelVert.X);
				OutViewport.Min.Y =
					FMath::Min<int32>(OutViewport.Min.Y, PixelVert.Y);

				OutViewport.Max.X =
					FMath::Max<int32>(OutViewport.Max.X, PixelVert.X);
				OutViewport.Max.Y =
					FMath::Max<int32>(OutViewport.Max.Y,
						PixelVert.Y);
			}
		}
	}
}

FQxColorCorrectSceneViewExtension::FQxColorCorrectSceneViewExtension(const FAutoRegister& AutoRegister,
                                                                     UQxColorCorrectSubsystem* InWorldSubsystem)
		: FSceneViewExtensionBase(AutoRegister), QxColorCorrectSubsystem(InWorldSubsystem)
{
}

void FQxColorCorrectSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder,
	const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	// FSceneViewExtensionBase::PrePostProcessPass_RenderThread(GraphBuilder, View, Inputs);
	// 这个锁是必要的。actor添加或者删除从场景中。同样的当priority发生变化时
	FScopeLock RegionScopeLock(&QxColorCorrectSubsystem->RegionAccessCriticalSection);

	if (QxColorCorrectSubsystem->Regions.Num() == 0 ||
		!ViewSupportsRegions(View))
	{
		return;
	}

	Inputs.Validate();

	const FSceneViewFamily& ViewFamily = *View.Family;

	const auto FeatureLevel = View.GetFeatureLevel();
	const float ScreenPercentage =
		ViewFamily.GetPrimaryResolutionFractionUpperBound() *
			ViewFamily.SecondaryViewFraction;

	// we need to make sure to take windows and scene scale into acount

	checkSlow(View.bIsViewInfo);
	const FIntRect PrimaryViewRect = static_cast<const FViewInfo&>(View).ViewRect;

	FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture,
		PrimaryViewRect);

	if (!SceneColor.IsValid())
	{
		return;
	}

	{
		// Getting material data for the current view.
		FGlobalShaderMap* GlobalShaderMap =
			GetGlobalShaderMap(GMaxRHIFeatureLevel);

		// 使用sceneColor的texture description来创建我们的back buffer
		FRDGTextureDesc ColorCorrectRegionsOutputDesc = SceneColor.Texture->Desc;
		bool bSampleOpacityFromGBuffer = false;
		if (ColorCorrectRegionsOutputDesc.Format != PF_FloatRGBA)
		{
			bSampleOpacityFromGBuffer = true;
		}
		ColorCorrectRegionsOutputDesc.Format = PF_FloatRGBA;
		FLinearColor ClearColor = FLinearColor::Black;
		ColorCorrectRegionsOutputDesc.ClearValue = FClearValueBinding(ClearColor);

		FRDGTexture* BackBufferRenderTargetTexture =
			GraphBuilder.CreateTexture(ColorCorrectRegionsOutputDesc,
				TEXT("QxBackBufferRenderTargetTexture"));
		FScreenPassRenderTarget BackBufferRenderTarget 
			= FScreenPassRenderTarget(BackBufferRenderTargetTexture,
				SceneColor.ViewRect,
				ERenderTargetLoadAction::EClear);
		FScreenPassRenderTarget SceneColorRenderTarget(SceneColor, ERenderTargetLoadAction::ELoad);
		const FScreenPassTextureViewport SceneColorTextureViewport(SceneColor);

		FRHIBlendState* DefaultBlendState =
			FScreenPassPipelineState::FDefaultBlendState::GetRHI();
		FRHIDepthStencilState* DepthStencilState =
			FScreenPassPipelineState::FDefaultDepthStencilState::GetRHI();

		RDG_EVENT_SCOPE(GraphBuilder, "Qx Color Correct Regions %dx%d",
			SceneColorTextureViewport.Rect.Width(),
			SceneColorTextureViewport.Rect.Height());

		FRHISamplerState* PointClampSampler =
			TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		const FScreenPassTextureViewportParameters SceneTextureViewportParameters =
			GetTextureViewportParameters(SceneColorTextureViewport);
		FScreenPassTextureInput SceneTextureInput;
		{
			SceneTextureInput.Viewport = SceneTextureViewportParameters;
			SceneTextureInput.Texture =
				SceneColorRenderTarget.Texture;
			SceneTextureInput.Sampler = PointClampSampler;
		}

		// 由于我们不是用material proxy，而是global shader,
		// 因此我们需要自行设置scene texture
		// 我们不需要 每个region 都这样做
		FSceneTextureShaderParameters SceneTextures=
			CreateSceneTextureShaderParameters(GraphBuilder,
				View.GetFeatureLevel(),
				ESceneTextureSetupMode::All);

		for (auto  It =
			QxColorCorrectSubsystem->Regions.CreateConstIterator();
			It ; ++It)
		{
			AQxColorCorrectRegion* Region = *It;

			// 我们使用 primitive component ids 来检查
			// 这个region 是否 从camera 隐藏， in cases such ass Display Cluster
			FPrimitiveComponentId FirstComponentId =
				FQxColorCorrectRegionDatabase::GetFirstComponentId(Region);

			bool bNoNeedToRender = Region->IsPendingKill() || Region->IsActorBeingDestroyed();
			bNoNeedToRender = bNoNeedToRender ||
				!Region->Enabled || Region->IsHidden();
#ifdef WITH_EDITOR
			bNoNeedToRender = bNoNeedToRender ||  Region->IsHiddenEd();
#endif
			bNoNeedToRender = bNoNeedToRender ||
				Region->GetWorld() != ViewFamily.Scene->GetWorld() ||
					View.HiddenPrimitives.Contains(FirstComponentId);
			if (bNoNeedToRender)
			{
				continue;
			}

			FVector BoxCenter, BoxExtents;
			Region->GetBounds(BoxCenter, BoxExtents);

			// If bounding box is zero , then we don't need to do  anything
			if (BoxExtents.IsNearlyZero())
			{
				continue;
			}

			FIntRect Viewport;

			float MaxDepth = -BIG_NUMBER;
			float MinDepth = BIG_NUMBER;

			if (Region->Invert)
			{
				// In case of Region inversion we would to render the entire screen
				Viewport = PrimaryViewRect;
			}
			else
			{
				GetPixelSpaceBoundingRect(
					View,
					BoxCenter,
					Viewport,
					MaxDepth,
					MinDepth
					);
				
			}
		}
	}
}

