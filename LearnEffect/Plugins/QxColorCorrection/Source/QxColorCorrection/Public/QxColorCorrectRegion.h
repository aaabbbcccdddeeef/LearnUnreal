// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QxColorCorrectRegion.generated.h"

class UQxColorCorrectSubsystem;
UENUM(BlueprintType)
enum class EQxColorCorrectRegionsType : uint8
{
	Sphere UMETA(DisplayName = "Sphere"),
	Box UMETA(DisplayName = "Box"),
	Cylinder UMETA(DisplayName = "Cylinder"),
	Cone UMETA(DisplayName = "Cone"),
	MAX
};

UENUM(BlueprintType)
enum class EQxColorCorrectRegionTemperatureType : uint8
{
	LegacyTemperature UMETA(DisplayName = "Temperature(Legacy)"),
	WhiteBalance UMETA(DisplayName = "White Balance"),
	ColorTemperature UMETA(DisplayName = "Color Temperature"),
	MAX
};


/**
 * @brief  一个colorCorrectionRegion的instance。用来聚合所有校色区域
 * QxColorCorrectSubsystem定义这个类的actor示例的添加、删除等等
 * 
 */
UCLASS(Blueprintable)
class QXCOLORCORRECTION_API AQxColorCorrectRegion : public AActor
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AQxColorCorrectRegion();

	void ClearUp();
protected:
#ifdef WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void BeginDestroy() override;

	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

public:
	/**
	 * 我们需要自行管理region 的生命周期，由于EndPlay无法保证被调用
	 * 而BeginDestroy 在GC时调用，太晚了
	 */
	void Cleanup();

	void GetBounds(FVector& InOutOrigin, FVector& InOutBoxExtent) const
	{
		InOutOrigin = BoxOrigin;
		InOutBoxExtent = BoxExtent;
	}
public:

public:
	/** Region type. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	EQxColorCorrectRegionsType Type = EQxColorCorrectRegionsType::Sphere;

	/** Render priority/order. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	int32 Priority = 0;

	/** Color correction intensity. Clamped to 0-1 range. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Color Correction", meta = (UIMin = 0.0, UIMax = 1.0))
	float Intensity = 1.0f;
	
	/** Inner of region. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Color Correction", meta = (UIMin = 0.0, UIMax = 1.0))
	float Inner = 0.5f;

	/// Outer of the region
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Color Correction", meta = (UIMin = 0.0, UIMax = 1.0))
	float Outer = 1.0f;

	/** Falloff. softening the region */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Color Correction", meta = (UIMin = 0.0))
	float Falloff = 1.0f;

	/** invert region */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	bool Invert = false;

	/** type of algorithm to be used to control color temperature or white balance */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	EQxColorCorrectRegionTemperatureType TemperatureType = EQxColorCorrectRegionTemperatureType::ColorTemperature;

	/** Color correction temperature */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	float Temperature = 6500.f;

	/** Color correction settings */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	FColorGradingSettings ColorGradingSettings;

	/** Enable/Disable color correction provide by this region */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	bool Enabled = true;

	/** Enable Stenciling */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="QxColorCorrect")
	bool ExcludeStencil = false;

private:
	UQxColorCorrectSubsystem* QxColorCorrectSubsystem = nullptr;

	FVector BoxOrigin;
	FVector BoxExtent;

	FTransform PreviousFrameTransform;
};
