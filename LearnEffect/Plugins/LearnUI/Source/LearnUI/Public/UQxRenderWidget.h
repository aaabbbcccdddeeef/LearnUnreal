// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PreviewScene.h"
#include "Components/Widget.h"
#include "UObject/Object.h"
#include "UQxRenderWidget.generated.h"


class FRenderWidgetPreviewScene : public FPreviewScene
{
public:
	
};


class SPriewScene : public  SImage
{
public:
	
};

/**
 * 
 */
UCLASS()
class LEARNUI_API UUQxRenderWidget : public UWidget
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

	TSharedPtr<FRenderWidgetPreviewScene> RenderScene;

	TSharedRef<SImage> MyImage;
};


