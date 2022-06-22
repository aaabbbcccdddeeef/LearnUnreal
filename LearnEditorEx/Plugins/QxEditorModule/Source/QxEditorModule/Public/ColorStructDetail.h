// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
// #include "ColorStructDetail.generated.h"

class FColorStructDetail : public IPropertyTypeCustomization
{
public:
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	void OnCheckBoxStateChanged(ECheckBoxState State);

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
private:
	TSharedPtr<IPropertyHandle> ColorHandle;
	TSharedPtr<IPropertyHandle> BoolHandle;
	TSharedPtr<SCheckBox> CheckBox;
};
