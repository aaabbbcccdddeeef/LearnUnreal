// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "QxStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew, ClassGroup = (Rendering,Common), HideCategories = "Object", meta = (BlueprintSpawnableComponent))
class QXRENDERSERIES_API UQxStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	friend class FQxStaticMeshSceneProxy;
#pragma region PrimitiveComponentInterface


#pragma endregion


	void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;


	FPrimitiveSceneProxy* CreateSceneProxy() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiplePass")
	UMaterialInterface* SecondPassMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MultiplePass")
	bool bNeedSecondPass = false;

	
};
