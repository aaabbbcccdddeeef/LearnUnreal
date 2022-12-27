#include "QxTestRenderer.h"

#include "EngineModule.h"
#include "QxRenderSubsystem.h"
#include "QxSSR.h"
#include "QxTestCulling.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "SceneRenderTargetParameters.h"
#include "Engine/TextureRenderTarget2D.h"

FQxTestRenderer::FQxTestRenderer(UQxRenderSubsystem* InQxRenderSubsystem)
	: QxRenderSubsystem(InQxRenderSubsystem)
{
	RenderHandle = GetRendererModule().RegisterPostOpaqueRenderDelegate(
		FPostOpaqueRenderDelegate::CreateRaw(this, &FQxTestRenderer::Render));
	// ThisModule = &FModuleManager::GetModuleChecked<FQyRenderSeriesModule>("QyRenderSeries");

	QxTestCulling = MakeUnique<FQxTestCulling>();
	GetRendererModule().RegisterCustomCullingImpl(QxTestCulling.Get());
}

FQxTestRenderer::~FQxTestRenderer()
{
	GetRendererModule().RemovePostOpaqueRenderDelegate(RenderHandle);
	GetRendererModule().UnregisterCustomCullingImpl(QxTestCulling.Get());
	QxTestCulling.Reset();
	RenderHandle.Reset();

	// ThisModule = nullptr;
}

void FQxTestRenderer::Render(FPostOpaqueRenderParameters& InParameters)
{
	// int test = 100;
	FRDGBuilder& GraphBuilder = *(InParameters.GraphBuilder);

	RDG_EVENT_SCOPE(GraphBuilder, "QxTestRenderer");

	UTextureRenderTarget2D* QxSSRExternal = QxRenderSubsystem->QxSSROutput;
	if (QxSSRExternal == nullptr)
	{
		return;
	}

	// try to resize SSRTex with ColorTexture
	{
		const FIntPoint ColorTexExtent =  InParameters.ColorTexture->Desc.Extent;
		if (FIntPoint(QxSSRExternal->SizeX, QxSSRExternal->SizeY) != ColorTexExtent)
		{
			AsyncTask(ENamedThreads::GameThread, [ColorTexExtent, QxSSRExternal]()
			{
				QxSSRExternal->ResizeTarget(ColorTexExtent.X, ColorTexExtent.Y);
				QxSSRExternal->UpdateResourceImmediate();
			});
		}
	}
	
	// QxRenderSubsystem->QxSSROutput->TextureReference.TextureReferenceRHI
	FTextureRenderTarget2DResource* SSRTexResource =
		static_cast<FTextureRenderTarget2DResource*>(QxSSRExternal->GetRenderTargetResource());
	if (!SSRTexResource)
		return;

	
	
	FRDGTextureRef SSR_RDG_Texture = GraphBuilder.RegisterExternalTexture(
		CreateRenderTarget(SSRTexResource->TextureRHI, TEXT("QxSSROutputTest")));
	FRHICopyTextureInfo CopyTextureInfo;
	CopyTextureInfo.Size = InParameters.ColorTexture->Desc.GetSize();
	// AddCopyTexturePass(GraphBuilder, InParameters.ColorTexture, SSR_RDG_Texture, CopyTextureInfo);

	QxScreenSpaceReflection::RenderQxSSR(InParameters, SSR_RDG_Texture);
}
