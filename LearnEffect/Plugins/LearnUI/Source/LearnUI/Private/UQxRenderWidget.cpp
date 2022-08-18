// Fill out your copyright notice in the Description page of Project Settings.


#include "UQxRenderWidget.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Widgets/Images/SImage.h"

TSharedRef<SWidget> UUQxRenderWidget::RebuildWidget()
{
	RenderScene = MakeShareable(new FRenderWidgetPreviewScene());

	USceneCaptureComponent2D* CaptureCom = NewObject<USceneCaptureComponent2D>();
	const FTransform CaptureComponentTransform(
		FRotator(0, 0, 0),
		FVector(-100, 0, 50),
		FVector::OneVector);

	CaptureCom->TextureTarget = RenderTargetTexture;
	RenderScene->AddComponent(CaptureCom, CaptureComponentTransform);

	const FTransform MeshComponentTransform(FTransform::Identity);
	UStaticMeshComponent* MeshComponent =
		NewObject<UStaticMeshComponent>();

	MeshComponent->SetStaticMesh(StaticMeshToRender);
	RenderScene->AddComponent(MeshComponent, MeshComponentTransform);

	CaptureCom->UpdateSceneCaptureContents(RenderScene->GetScene());

	MyImage = SNew(SImage);
	
	return Super::RebuildWidget();
}

void UUQxRenderWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}
