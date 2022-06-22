// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "UObject/Object.h"
// #include "QxAssetDetail.generated.h"

/**
 * 
 */
class FQxAssetDetail : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	void CutomizeProperty(IDetailLayoutBuilder& DetailBuilder);
	void CutomizeProperty2(IDetailLayoutBuilder& DetailBuilder);

	void GetPropertyAndAddToCategory(IDetailLayoutBuilder& DetailBuilder);
	void Test1(IDetailLayoutBuilder& DetailBuilder);
};


