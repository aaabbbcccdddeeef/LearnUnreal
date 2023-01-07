// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxRenderSubsystem.generated.h"

class FQxTestRenderer;
/**
 * 
 */
UCLASS()
class QYRENDERSERIES_API UQxRenderSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(EditAnywhere, Category="QxRenderer")
	UTextureRenderTarget2D* QxSSROutput = nullptr;


	UPROPERTY(Transient, Category="QxRenderer")
	UTextureRenderTarget2D* QxTestRT = nullptr;

private:

	UTextureRenderTarget2D* CreateTestRT();
	
private:
	TSharedPtr<FQxTestRenderer> QxTestRenderer;
};
