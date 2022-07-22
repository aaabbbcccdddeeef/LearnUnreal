// Fill out your copyright notice in the Description page of Project Settings.


#include "QxMaskWidgetStyle.h"


FQxMaskWidgetStyle2::FQxMaskWidgetStyle2()
{
}

FQxMaskWidgetStyle2::~FQxMaskWidgetStyle2()
{
}

const FName FQxMaskWidgetStyle2::TypeName(TEXT("FQxMaskStyleStyle"));

const FQxMaskWidgetStyle2& FQxMaskWidgetStyle2::GetDefault()
{
	static FQxMaskWidgetStyle2 Default;
	return Default;
}

void FQxMaskWidgetStyle2::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them
}
