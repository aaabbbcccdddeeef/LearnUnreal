// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRenderWidget.h"


#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Widgets/Images/SImage.h"

int32 SPriewScene::OnPaint(const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	
	SImage::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
	                       bParentEnabled);

	if (PreSceneWidgetPtr)
	{
		PreSceneWidgetPtr->UpdateRenderWidgetPreviewScene();
	}

	return  LayerId;
}

TSharedRef<SWidget> UQxRenderWidget::RebuildWidget()
{
	RenderScene = MakeShareable(new FRenderWidgetPreviewScene());

	CaptureCom = NewObject<USceneCaptureComponent2D>();
	const FTransform CaptureComponentTransform(
		FRotator(0, 0, 0),
		FVector(-100, 0, 50),
		FVector::OneVector);

	CaptureCom->TextureTarget = RenderTargetTexture;
	RenderScene->AddComponent(CaptureCom, CaptureComponentTransform);

	const FTransform MeshComponentTransform(FTransform::Identity);
	MeshComponent =
		NewObject<UStaticMeshComponent>();

	MeshComponent->SetStaticMesh(StaticMeshToRender);
	RenderScene->AddComponent(MeshComponent, MeshComponentTransform);

	CaptureCom->UpdateSceneCaptureContents(RenderScene->GetScene());

	MyImage = SNew(SImage);
	FSlateBrush brush;
	brush.SetResourceObject(RenderTargetTexture);
	// MyImage->SetImage(brush);
	
	return Super::RebuildWidget();
}

void UQxRenderWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

void UQxRenderWidget::UpdateRenderWidgetPreviewScene()
{
	// MeshComponent->SetWorldTransform()
	
	CaptureCom->UpdateSceneCaptureContents(
		RenderScene->GetScene());
}

void UQxRenderWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	CaptureCom = nullptr;
	MeshComponent = nullptr;
	RenderScene.Reset();
	MyImage.Reset();
	
	Super::ReleaseSlateResources(bReleaseChildren);
}
