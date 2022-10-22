// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxAccelerationTypes.h"
#include "QxOctree.h"
#include "UObject/Object.h"
#include "QxAccelerationBPLib.generated.h"

/**
 * 
 */
UCLASS()
class QXACCELERATE_API UQxAccelerationBPLib : public UObject
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static void BuildKdtree(FQxKdtree& Tree, const TArray<FVector> InPoints);

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static void ClearKdtree(UPARAM(ref) FQxKdtree& Tree);

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static void ValidateKdtree(const FQxKdtree& Tree);

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static void DumpKdtreeToConsole(FQxKdtree& Tree);

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static void CollectFromKdtree(const FQxKdtree& Tree, const FVector Center,
        float Radius, TArray<int32>& OutIndices, TArray<FVector>& OutPoints);


    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static FVector SearchNerestPoint(const FQxKdtree& Tree, const FVector& TargetPoint);


    // 查找距离target point 较近的k get点
    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static void SearchKNNPoint(FQxKdtree& Tree, const FVector& TargetPoint, int32 k, TArray<FVector>& OutPoints);

    // UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    // static void BuildOctree(FQxOctree& OutTree);

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static void BuildTestOctree(FQxOctree& OutTree, const FVector& Origin, float Extent);

    UFUNCTION(BlueprintCallable, Category="QxSpacialDataStructure")
    static  FQxOctree BuiltTestOctree2(const FVector& Origin, float Extent, float InLooseness = 1.f);

    UFUNCTION(BlueprintCallable, Category="QxOctree")
    void AddElement(FQxOctree& InOctree,  UObject* InElement, const FBoxSphereBounds& Bounds);

    UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContextObject") ,Category="QxSpacialDataStructure")
    static void DrawQxOctree(const FQxOctree& InOctree, const UObject* WorldContextObject,FLinearColor InColor,float InLifeTime, float InThickness);
};
