#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZZClipperTypes.generated.h"

UENUM(BlueprintType)
enum class EZZClipperType : uint8
{
    Box = 0,
    Sphere = 1
};

// Clipper信息,会encode 到纹素中
USTRUCT(BlueprintType)
struct FZZClipperInfos
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
    FVector Center = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
    FVector BoxExtent = FVector::OneVector;
    UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
    FVector ForwardVector = FVector::ForwardVector;
    UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
    FVector UpVector = FVector::UpVector;

    UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
    EZZClipperType ClipperType = EZZClipperType::Box;

    UPROPERTY(BlueprintReadWrite, Category="MultiOccluder")
    float Radius = 100.f;
};
