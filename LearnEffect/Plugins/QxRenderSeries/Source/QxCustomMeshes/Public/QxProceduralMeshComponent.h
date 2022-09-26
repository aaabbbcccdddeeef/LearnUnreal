// Fill out your copyright notice in the Description page of Project Settings.
// 这个组件主要是为了重新练习自定义mesh component的实现,
// 包括自定义的vertex layout/vertex buffer/index buffer/vertex factory
// optional 绑定GS/HL/DS

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QxProceduralMeshComponent.generated.h"


// qxProceduralMeshComponent使用的顶点cpu定义,vertex declaration要和这个一致
// 这个结构参考FLidarPointCloudPoint
// 注意：由于pading/alignment问题，这个结构实际占用20个字节
USTRUCT(BlueprintType)
struct FQxProceduralPoint
{
	GENERATED_BODY()
	FQxProceduralPoint()
	{
		bVisible = 1;
		ClassificationID = 0;
		bSelected = 0;
		bTest = 0;
		Padding1 = 0;
		Padding2 = 0;
		Padding3 = 0;
	}
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QxProcedural")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "QxProcedural")
	FColor Color = FColor::White;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QxProcedural")
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QxProcedural")
	uint8 bVisible : 1;

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QxProcedural")
	uint8 ClassificationID : 5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QxProcedural")
	uint8 bSelected : 1 ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QxProcedural")
	uint8 bTest : 1;
	
	// 下面3个是为了站位，对齐到4字节
	uint8 Padding1 = 0;
	uint8 Padding2 = 0;
	uint8 Padding3 = 0;
};

UENUM(BlueprintType)
enum class EQxClippingVolumeMode : uint8
{
	ClipInside,
	ClipOutside
};



UCLASS()
class AQxClippingVolume : public AVolume
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clipping Volume")
	bool bEnabled = false;

	/** Affects how this volume affects points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clipping Volume")
	EQxClippingVolumeMode Mode = EQxClippingVolumeMode::ClipInside;

	AQxClippingVolume();
};

struct FQxProceduralClippingVolumeParams
{
	EQxClippingVolumeMode Mode;

	FMatrix PackedShaderData;

	FQxProceduralClippingVolumeParams(const AQxClippingVolume* ClippingVolume);
};

// 包括所有需要构建上传到gpu的数据，包括buffers/shader parameters
struct FQxProceduralRenderData
{

	TArray<FQxProceduralPoint> PointsData;

	float PointSize = 1.f;

	FVector ColorTint = FVector(FLinearColor::Green);
	FVector4 Contrast = FVector4(1, 0, 0, 0);
	FVector4 Saturation = FVector4(1, 0, 0, 0);

	TArray<FQxProceduralClippingVolumeParams>  ClippingVolumes;
	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class QXCUSTOMMESHES_API UQxProceduralMeshComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQxProceduralMeshComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual int32 GetNumMaterials() const override;

	virtual FBoxSphereBounds CalcLocalBounds() const override;

	virtual void OnRegister() override;

	virtual void SendRenderDynamicData_Concurrent() override;

	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="QxProcedural")
	TArray<FQxProceduralPoint> PointsData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="QxProcedural")
	FColor TestVertexColor;
	
private:
	void GeneratePointsData();
	
protected:

	// 生成的不同点的间距
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="QxProcedural")
	float PointsDistance = 100.f;
	
	// 用来控制生成的点的多少
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="QxProcedural")
	int32 PointsNums = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="QxProcedural")
	FLinearColor Tint = FLinearColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="QxProcedural")
	float PointSize = 1.f;

	UPROPERTY(interp, EditAnywhere, BlueprintReadWrite, Category = "Color Adjustment", meta = (UIMin = "0.0", UIMax = "2.0", Delta = "0.01", ColorGradingMode = "contrast", ShiftMouseMovePixelPerDelta = "10"))
	FVector4 Contrast;

	UPROPERTY(interp, EditAnywhere, BlueprintReadWrite, Category = "Color Adjustment", meta = (UIMin = "0.0", UIMax = "2.0", Delta = "0.01", ColorGradingMode = "saturation", ShiftMouseMovePixelPerDelta = "10"))
	FVector4 Saturation;
};
