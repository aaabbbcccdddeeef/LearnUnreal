// Fill out your copyright notice in the Description page of Project Settings.


#include "QxMaskWidgetStyle.h"


FQxMaskWidgetStyle2::FQxMaskWidgetStyle2()
{
}

FQxMaskWidgetStyle2::~FQxMaskWidgetStyle2()
{
}

const FName FQxMaskWidgetStyle2::TypeName(TEXT("FQxMaskWidgetStyle2"));

const FQxMaskWidgetStyle2& FQxMaskWidgetStyle2::GetDefault()
{
	static FQxMaskWidgetStyle2 Default;
	return Default;
}

bool FQxMaskWidgetStyle2::RemoveMaskClickClip(int32 ClipIndex)
{
	if (ClipIndex >= 0 && ClipIndex < MaskClips.Num())
	{
		MaskClips.RemoveAt(ClipIndex);
		return  true;
	}
	else
	{
		UE_LOG(LogInit, Error, TEXT("ERROR: FMaskWidgetStyle::RemoveMaskClickClip invalid ClipIndex = %d, MaskClips.Num() = %d"), ClipIndex, MaskClips.Num());
	}
	return  false;
}

int32 FQxMaskWidgetStyle2::AddMaskClickClip(const FVector2D& Position, const FVector2D& Size, UTexture2D* Mask)
{
	int32 Count = MaskClips.Num();
	if (Count < MAX_MASK_CLIP_COUNT)
	{
		MaskClips.Add(FQxMaskClip(Count, Position, Size, Mask));
		return Count;
	}
	return  -1;
}

bool FQxMaskWidgetStyle2::EnableMaskClickClip(int32 ClipIndex, bool bEnable)
{
	if (ClipIndex < MaskClips.Num())
	{
		MaskClips[ClipIndex].bClipEnable = bEnable;
		return true;
	}
	return false;
}

bool FQxMaskWidgetStyle2::SetMaskPosSize(int32 ClipIndex, const FVector4& PosSize)
{
	if (ClipIndex < MaskClips.Num())
	{
		MaskClips[ClipIndex].MaskPosition = FVector2D(PosSize.X, PosSize.Y);
		MaskClips[ClipIndex].MaskSize = FVector2D(PosSize.Z, PosSize.W);
		return  true;
	}
	return false;
}

bool FQxMaskWidgetStyle2::SetMaskSize(int32 ClipIndex, const FVector2D& Size)
{
	if (ClipIndex < MaskClips.Num())
	{
		MaskClips[ClipIndex].MaskSize = Size;
		return  true;
	}
	return false;
}

bool FQxMaskWidgetStyle2::SetMaskPos(int32 ClipIndex, const FVector2D& Pos)
{
	if (ClipIndex < MaskClips.Num())
	{
		MaskClips[ClipIndex].MaskPosition = Pos;
		return  true;
	}
	return false;
}

bool FQxMaskWidgetStyle2::SetMaskTexByIndex(int32 ClipIndex, UTexture2D* Tex)
{
	if (!Tex)
	{
		return false;
	}
	if (ClipIndex >= 0 && ClipIndex < MaskClips.Num())
	{
		MaskClips[ClipIndex].MaskTex = Tex;
		return true;
	}
	return false;
}

void FQxMaskWidgetStyle2::ReIndexClip()
{
	for (int32 i = 0; i < MaskClips.Num(); ++i)
	{
		MaskClips[i].ClipIndex = i;
	}
}

void FQxMaskWidgetStyle2::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	OutBrushes.Add(&BackgroundImage);
	OutBrushes.Add(&MaskMatBrush);
	// Add any brush resources here so that Slate can correctly atlas and reference them
}
