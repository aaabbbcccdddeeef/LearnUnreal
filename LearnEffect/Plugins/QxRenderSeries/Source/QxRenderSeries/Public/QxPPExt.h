// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QxPPExt.generated.h"

/**
 * 这个类用来试验在插件中通过代码实现一些后期效果
 */
UCLASS()
class QXRENDERSERIES_API UQxPPExt : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

private:
	void RenderQxGuassianBlur(FPostOpaqueRenderParameters& InParameters);
	
	void RegisterRenderCallbacks();

	FDelegateHandle GuassuaDelegateHandle;
};
