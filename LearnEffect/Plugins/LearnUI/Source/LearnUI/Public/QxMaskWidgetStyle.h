// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"

#include "QxMaskWidgetStyle.generated.h"

static const uint8 MAX_MASK_CLIP_COUNT = 3;

// 辅助定义mask clip的形状
USTRUCT(BlueprintType)
struct FQxMaskClip
{
	GENERATED_BODY()

public:
	FQxMaskClip() {  }

	FQxMaskClip(const int32& Index, const FVector2D& Pos, const FVector2D& Size, UTexture2D* Mask)
	{
		ClipIndex = Index;
		MaskPosition = Pos;
		MaskSize = Size;
		MaskTex = Mask;
	}

	virtual ~FQxMaskClip(){}
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaskClip")
	UTexture2D* MaskTex = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaskClip")
	FVector2D MaskPosition = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaskClip")
	FVector2D MaskSize = FVector2D(32.f, 32.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MaskClip")
	bool bClipEnable = false;

	
	int32 ClipIndex = -1;
};


/**
 * 这里叫2主要是出了重命名的造成的编译问题
 */
USTRUCT(BlueprintType)
struct LEARNUI_API FQxMaskWidgetStyle2 : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FQxMaskWidgetStyle2();
	virtual ~FQxMaskWidgetStyle2();

#pragma region TemplateMethods
	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;


	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FQxMaskWidgetStyle2& GetDefault();
#pragma endregion

public:
	bool RemoveMaskClickClip(int32 ClipIndex);
	int32 AddMaskClickClip(const FVector2D& Position, const FVector2D& Size, UTexture2D* Mask);
	bool EnableMaskClickClip(int32 ClipIndex, bool bEnable);
	bool SetMaskPosSize(int32 ClipIndex, const FVector4& Vector4);
	bool SetMaskSize(int32 ClipIndex, const FVector2D& Size);
	bool SetMaskPos(int32 ClipIndex, const FVector2D& Pos);
	bool SetMaskTexByIndex(int32 ClipIndex, UTexture2D* Tex);
	void ReIndexClip();

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Appearance")
	TArray<FQxMaskClip> MaskClips;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Appearance")
	FSlateBrush BackgroundImage;

	// 或许更合理的做法是代码加载固定位置的材质，这里是实现效果用的材质，不是纹理
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
	FSlateBrush MaskMatBrush;
	
	FQxMaskWidgetStyle2& SetBackgroundImage(const FSlateBrush& InBgImage)
	{
		BackgroundImage = InBgImage;
		return *this;
	}

	
};

/**
 *  这个类的主要用途是实现FStyle的序列化，在编辑器中创建和编辑内部的WidgetStyle资源
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UQxMaskWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FQxMaskWidgetStyle2 WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast<const struct FSlateWidgetStyle*>(&WidgetStyle);
	}
};
