// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QxScriptedTexture.generated.h"


class FWidgetRenderer;
class SVirtualWindow;
/**
 * @brief 参照 https://www.froyok.fr/blog/2020-06-render-target-performances/
 * 主要目的是 得到DrawMaterialToRenderTarget多次调用的高性能版本
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QXRENDERSERIES_API UQxScriptedTexture : public USceneComponent
{
	GENERATED_BODY()

public:
	UQxScriptedTexture(const FObjectInitializer& ObjectInitializer);
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void OnUnregister() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void Render(float DeltaTime);

	void Resize(FIntPoint& NewSize);
	void Init();
private:
	void UpdateSlateWindow();

private:
	// cached window contain the rendering windget
	TSharedPtr<SVirtualWindow> SlateWindow;
	TSharedPtr<FHittestGrid> SlateGrid;

	UPROPERTY(Transient)
	UTextureRenderTarget2D* ScriptedTexture = nullptr;

	UPROPERTY(Transient)
	UUserWidget* RenderingWidget = nullptr;

	
	FWidgetRenderer* WidgetRenderer = nullptr;

	FGeometry SlateGeometry;
};

UCLASS()
class AQxRenderWithSlate : public AActor
{

private:
	GENERATED_BODY()
public:
	AQxRenderWithSlate();

	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;
private:
	UPROPERTY()
	UQxScriptedTexture* ScriptedTexture = nullptr;
};

