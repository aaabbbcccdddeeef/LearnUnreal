// Fill out your copyright notice in the Description page of Project Settings.


#include "QxMaskWidget.h"
#include "SQxMaskWidget.h"
#include "Components/Widget.h"

#define LOCTEXT_NAMESPACE "QxMask"

void UQxMaskWidget::SetBgColorAndOpacity(FLinearColor InColorAndOpacity)
{
	BgColorAndOpacity = InColorAndOpacity;
	if (QxMask.IsValid())
	{
		QxMask->SetBgColorAndOpacity(BgColorAndOpacity);
	}
}

void UQxMaskWidget::SetBgOpacity(float InOpacity)
{
	BgColorAndOpacity.A = InOpacity;
	if (QxMask.IsValid())
	{
		QxMask->SetBgColorAndOpacity(BgColorAndOpacity);
	}
}

void UQxMaskWidget::SetBgImage(UTexture2D* Tex, bool bMatchSize)
{
	if (!Tex)
	{
		return;
	}

	QxWidgetStyle.BackgroundImage.SetResourceObject(Tex);
	if (bMatchSize)
	{
		QxWidgetStyle.BackgroundImage.ImageSize.X = Tex->GetSizeX();
		QxWidgetStyle.BackgroundImage.ImageSize.Y = Tex->GetSizeY();
	}

	if (QxMask.IsValid())
	{
		QxMask->SetStyle(&QxWidgetStyle);
	}
}

void UQxMaskWidget::SetMaskMaterial(UMaterialInterface* InMaterial)
{
	if (!InMaterial)
	{
		return;
	}

	QxWidgetStyle.MaskMatBrush.SetResourceObject(InMaterial);
	if (QxMask.IsValid())
	{
		QxMask->SetStyle(&QxWidgetStyle);
	}
}

void UQxMaskWidget::SetBrushTintColor(FSlateColor TintColor)
{
	if (QxWidgetStyle.BackgroundImage.TintColor != TintColor)
	{
		QxWidgetStyle.BackgroundImage.TintColor = TintColor;

		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
}

void UQxMaskWidget::SetMaskImage(const int32& ClipIndex, UTexture2D* Tex)
{
	if (Tex && QxWidgetStyle.SetMaskTexByIndex(ClipIndex, Tex))
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
}

void UQxMaskWidget::SetMaskPos(const int32& ClipIndex, const FVector2D& Pos)
{
	if (QxWidgetStyle.SetMaskPos(ClipIndex, Pos))
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
}

void UQxMaskWidget::SetMaskPosXY(const int32& ClipIndex, const float& PosX, const float& PosY)
{
	if (QxWidgetStyle.SetMaskPos(ClipIndex, FVector2D(PosX, PosY)))
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
}

void UQxMaskWidget::SetMaskSize(const int32& ClipIndex, const FVector2D& Size)
{
	if (QxWidgetStyle.SetMaskSize(ClipIndex, Size))
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
}

void UQxMaskWidget::SetMaskSizeXY(const int32& ClipIndex, const float& SizeX, const float& SizeY)
{
	if (QxWidgetStyle.SetMaskSize(ClipIndex, FVector2D(SizeX, SizeY)))
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
}

void UQxMaskWidget::SetMaskPosSize(const int32& ClipIndex, const FVector4& PosSize)
{
	if (QxWidgetStyle.SetMaskPosSize(ClipIndex, PosSize))
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
}

void UQxMaskWidget::EnableMaskClickClip(const int32& ClipIndex, bool Enable)
{
	if (QxWidgetStyle.EnableMaskClickClip(ClipIndex, Enable))
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
}

int32 UQxMaskWidget::AddMaskClickClip(const FVector2D& Position, const FVector2D& Size, UTexture2D* Mask)
{
	int32 Index = QxWidgetStyle.AddMaskClickClip(Position, Size, Mask);
	if (Index > -1)
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
	return Index;
}

bool UQxMaskWidget::RemoveMaskClip(const int32& ClipIndex)
{
	bool Ret = QxWidgetStyle.RemoveMaskClickClip(ClipIndex);
	if (Ret)
	{
		if (QxMask.IsValid())
		{
			QxMask->SetStyle(&QxWidgetStyle);
		}
	}
	return Ret;
}

// 这个方法在Rebuild widget的时候会调用，其他时机??，否则为什么不写在Rebuild widget中
void UQxMaskWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	TAttribute<FSlateColor> ColorAndOpacityBinding = PROPERTY_BINDING(FSlateColor, BgColorAndOpacity);
	if (QxMask.IsValid())
	{
		QxMask->SetStyle(&QxWidgetStyle);
		QxMask->SetBgColorAndOpacity(BgColorAndOpacity);
	}
}

void UQxMaskWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	QxMask.Reset();
}

#if WITH_EDITOR
const FText UQxMaskWidget::GetPaletteCategory()
{
	return LOCTEXT("QxMaskWidget", "QxMaskWidget");
}
#endif


TSharedRef<SWidget> UQxMaskWidget::RebuildWidget()
{
	QxMask = SNew(SQxMaskWidget).Style(&QxWidgetStyle);
	QxMask->ReIndexClip();
	return QxMask.ToSharedRef();
}


#undef LOCTEXT_NAMESPACE