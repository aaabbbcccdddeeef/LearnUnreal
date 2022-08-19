// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreviewScene.h"
#include "Components/Widget.h"
#include "UObject/Object.h"
#include "Widgets/Images/SImage.h"
#include "QxRenderWidget.generated.h"

class UQxRenderWidget;

class FRenderWidgetPreviewScene : public FPreviewScene
{
public:
	
};


class LEARNUI_API SPriewScene : public  SImage
{
public:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UQxRenderWidget* PreSceneWidgetPtr;
};

/**
 * 
 */
UCLASS()
class LEARNUI_API UQxRenderWidget : public UWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="WorldAsset")
	UTextureRenderTarget2D* RenderTargetTexture;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="WorldAsset")
	UStaticMesh* StaticMeshToRender;

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void SynchronizeProperties() override;
private:
public:
	void UpdateRenderWidgetPreviewScene();
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:

	TSharedPtr<FRenderWidgetPreviewScene> RenderScene;

	TSharedPtr<SImage> MyImage;

	UPROPERTY()
	USceneCaptureComponent2D* CaptureCom = nullptr;

	UPROPERTY()
	UStaticMeshComponent* MeshComponent = nullptr;
};


