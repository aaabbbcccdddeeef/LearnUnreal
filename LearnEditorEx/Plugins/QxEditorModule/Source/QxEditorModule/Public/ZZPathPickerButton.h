// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "UObject/Object.h"
#include "ZZPathPickerButton.generated.h"

class SZZPathPickerButton;
/**
 * 这个widget 只是给编辑器用的
 */
UCLASS()
class QXEDITORMODULE_API UZZPathPickerButton : public UWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="ZZMeshTool")
	FString GetPickedPath() const;
	
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
protected:
	TSharedPtr<SZZPathPickerButton> PathPickerButton;
};
