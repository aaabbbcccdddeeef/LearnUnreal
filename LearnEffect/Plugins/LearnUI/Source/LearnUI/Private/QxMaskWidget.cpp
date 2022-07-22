// Fill out your copyright notice in the Description page of Project Settings.


#include "QxMaskWidget.h"

#include <stdexcept>

#include "SQxMaskWidget.h"

#define LOCTEXT_NAMESPACE "QxMask"

void UQxMaskWidget::SetBgColorAndOpacity(FLinearColor InColorAndOpacity)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetBgOpacity(float InOpacity)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetImage(UTexture2D* Tex, bool bMatchSize)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetBrushTintColor(FSlateColor TintColor)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetMaskImage(const int32& ClipIndex, UTexture2D* Tex)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetMaskPos(const int32& ClipIndex, const FVector2D& Pos)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetMaskPosXY(const int32& ClipIndex, const float& PosX, const float& PosY)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetMaskSize(const int32& ClipIndex, const FVector2D& Size)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetMaskSizeXY(const int32& ClipIndex, const float& SizeX, const float& SizeY)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SetMaskPosSize(const int32& ClipIndex, const FVector4& PosSize)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::EnableMaskClickClip(const int32& ClipIndex, bool Enable)
{
	throw std::logic_error("Not implemented");
}

int32 UQxMaskWidget::AddMaskClickClip(const FVector2D& Position, const FVector2D& Size, UTexture2D* Mask)
{
	throw std::logic_error("Not implemented");
}

bool UQxMaskWidget::RemoveMaskClip(const int32& ClipIndex)
{
	throw std::logic_error("Not implemented");
}

void UQxMaskWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

void UQxMaskWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
}

#if WITH_EDITOR
const FText UQxMaskWidget::GetPaletteCategory()
{
	return LOCTEXT("QxMaskWidget", "QxMaskWidget");
}
#endif


TSharedRef<SWidget> UQxMaskWidget::RebuildWidget()
{
	QxMask = SNew(SQxMaskWidget);
	return QxMask.ToSharedRef();
}


#undef LOCTEXT_NAMESPACE