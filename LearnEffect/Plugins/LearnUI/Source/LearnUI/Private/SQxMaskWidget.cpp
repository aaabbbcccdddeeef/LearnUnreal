// Fill out your copyright notice in the Description page of Project Settings.


#include "SQxMaskWidget.h"

#include <stdexcept>

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQxMaskWidget::Construct(const FArguments& InArgs)
{
	/*
	ChildSlot
	[
		// Populate the widget
	];
	*/
	check(InArgs._Style);

	BgColorAndOpacity = InArgs._BgColorAndOpacity;
	Style = const_cast<FQxMaskWidgetStyle2*>(InArgs._Style);

	SetCanTick(false);
}

int32 SQxMaskWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	int ResLayerId = LayerId;
	SQxMaskWidget* MutableThis = const_cast<SQxMaskWidget*>(this);

	const FSlateBrush* CurBgImage = GetBackgroundImage();
	const FSlateBrush* MaskMatBrush = GetMaskBrush();
	// check(CurBgImage && MaskMatBrush);

#if !WITH_EDITOR
	if (MutableThis->bIsMaskeUpdated)
	{
#endif
	//设置材质中的mask/bg等参数	
	if (UMaterialInstanceDynamic* MaskMID = Cast<UMaterialInstanceDynamic>(MaskMatBrush->GetResourceObject()))
	{
		FVector2D GSize = AllottedGeometry.GetLocalSize();

		const TArray<FQxMaskClip>& Clips = Style->MaskClips;
		for (int32 i = 0; i < MAX_MASK_CLIP_COUNT; ++i)
		{
			if (i < Clips.Num())
			{
				FQxMaskClip Clip = Clips[i];
				FVector2D Size = Clips[i].MaskSize;
				FVector2D Pos = Clips[i].MaskPosition;
				FLinearColor paramColor = FLinearColor(Pos.X / GSize.X, Pos.Y / GSize.Y, Size.X / GSize.X, Size.Y / GSize.Y);
				MaskMID->SetVectorParameterValue(
					*FString::Printf(TEXT("MaskUV_%d"), i), paramColor
					);
				if (UTexture2D* Tex = Clip.MaskTex)
				{
					MaskMID->SetTextureParameterValue(
						*FString::Printf(TEXT("MaskTex_%d"), i),
						Tex
						);
				}
			}
		}
		MaskMID->SetTextureParameterValue(
			"BgTex",
			Cast<UTexture>(CurBgImage->GetResourceObject())
			);
	}
#if !WITH_EDITOR
		MutableThis->bIsMaskeUpdated = false;
	}
#endif

	// 	渲染背景
	if (CurBgImage)
	{
		// 背景颜色部分由下面3项决定
		const FLinearColor& bgTint = InWidgetStyle.GetColorAndOpacityTint()
			* BgColorAndOpacity.Get().GetColor(InWidgetStyle) * CurBgImage->GetTint(InWidgetStyle);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			ResLayerId++,
			AllottedGeometry.ToPaintGeometry(),
			CurBgImage,
			ESlateDrawEffect::None,
			bgTint
			);
	}

	// 渲染一组镂空元素
	if (MaskMatBrush)
	{
		FSlateLayoutTransform LayoutTrans(AllottedGeometry.Scale, AllottedGeometry.AbsolutePosition);
		const TArray<FQxMaskClip>& Clips = Style->MaskClips;
		for (int32 i = 0; i < Clips.Num() && i < MAX_MASK_CLIP_COUNT; ++i)
		{
			if (Clips[i].bClipEnable)
			{
				
				// const FGeometry& MaskGeometry = AllottedGeometry.MakeChild()
				const FGeometry& MaskGeometry  = AllottedGeometry.MakeChild(
					Clips[i].MaskPosition,
					Clips[i].MaskSize
					);

				
				FPaintGeometry MaskPaintGeometry(
					Clips[i].MaskPosition,
					Clips[i].MaskSize,
					1.f);
				FSlateDrawElement::MakeBox(
					OutDrawElements,
					ResLayerId++,
					MaskGeometry.ToPaintGeometry(),
					MaskMatBrush,
					ESlateDrawEffect::None,
					InWidgetStyle.GetColorAndOpacityTint()
					);

				//#TODO 输入的处理
			}
		}
		
	}

	return ResLayerId;
}

FVector2D SQxMaskWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	// 期望使用backgournd image的大小
	return GetBackgroundImage()->ImageSize;
}

bool SQxMaskWidget::IsInteractable() const
{
	return  IsEnabled();
}

FReply SQxMaskWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (OnClicked.IsBound())
	{
		return OnClicked.Execute(-1, false);
	}
	return FReply::Handled();
}

FReply SQxMaskWidget::OnTouchGesture(const FGeometry& MyGeometry, const FPointerEvent& GestureEvent)
{
	if (OnClicked.IsBound())
	{
		return OnClicked.Execute(-1, false);
	}
	return FReply::Handled();
}

void SQxMaskWidget::SetStyle(const FQxMaskWidgetStyle2* InStyle)
{
	if (InStyle == nullptr)
	{
		FArguments Defaults;
		Style = const_cast<FQxMaskWidgetStyle2*>(Defaults._Style);
	}
	else
	{
		Style = const_cast<FQxMaskWidgetStyle2*>(InStyle);
	}

	check(Style);

	Invalidate(EInvalidateWidgetReason::Layout);
	bIsMaskeUpdated = true;
}

void SQxMaskWidget::SetBgColorAndOpacity(FLinearColor InBgColorAndOpacity)
{
	// SetAttribute(BgColorAndOpacity, TAttribute<FSlateColor>(InBgColorAndOpacity), EInvalidateWidgetReason::Paint);
	SetBgColorAndOpacity(TAttribute<FSlateColor>(InBgColorAndOpacity));
}

void SQxMaskWidget::SetBgColorAndOpacity(const TAttribute<FSlateColor>& InColorAndOpacity)
{
	SetAttribute(BgColorAndOpacity, InColorAndOpacity, EInvalidateWidgetReason::Paint);
}

void SQxMaskWidget::ReIndexClip()
{
	Style->ReIndexClip();
}

const FSlateBrush* SQxMaskWidget::GetBackgroundImage() const
{
	return BackgroundImage ? BackgroundImage : &Style->BackgroundImage;
}

const FSlateBrush* SQxMaskWidget::GetMaskBrush() const
{
	return &(Style->MaskMatBrush);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
