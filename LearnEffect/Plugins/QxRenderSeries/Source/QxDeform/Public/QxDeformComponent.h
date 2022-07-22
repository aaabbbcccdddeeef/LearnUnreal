// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QxDeformActor.h"
#include "Components/ActorComponent.h"
#include "Components/MeshComponent.h"
#include "QxDeformComponent.generated.h"


USTRUCT()
struct FDeformMeshSection
{
	GENERATED_BODY()
public:

	// 包括这个section mesh信息的mesh
	UPROPERTY()
	UStaticMesh* StaticMesh;

	//  用来进行扭曲的额外矩阵
	UPROPERTY()
	FMatrix DeformTransformMat;

	// local bounding box of section, 主要用来辅助proxy计算bounds
	UPROPERTY()
	FBox SectionLocalBox;

	// 是否显示这个section
	UPROPERTY()
	bool bSectionVisible;

	FDeformMeshSection()
		: SectionLocalBox(ForceInit)
		, bSectionVisible(true)
	{
	}

	// reset this section,clear all mesh info
	void Reset()
	{
		StaticMesh = nullptr;
		SectionLocalBox.Init();
		bSectionVisible = true;
	}
};

UCLASS(ClassGroup=(Rendering), meta=(BlueprintSpawnableComponent), HideCategories=(Object,LOD))
class QXDEFORM_API UQxDeformComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UQxDeformComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	
	void UpdateMeshSectionTransform(int SectionIndex, const FTransform& Transform);
	void FinishTransformsUpdate();
	void CreateMeshSection(int SectionIndex, UStaticMesh* InMesh, const FTransform& Transform);

	// 创建这个component 对应的渲染线程proxy
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	// mesh component这个返回0，需要override
	virtual int32 GetNumMaterials() const override
	{
		// return
		return DeformMeshSections.Num();
	}

	
	//calcBounds定义在SceneComponent中，涉及到物理，渲染等多部分，一般也需要override
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

private:
	void UpateLocalBounds();
	
private:
	UPROPERTY()
	TArray<FDeformMeshSection> DeformMeshSections;

	UPROPERTY()
	FBoxSphereBounds LocalBounds;
	friend class FQxDeformMeshProxy;
};
