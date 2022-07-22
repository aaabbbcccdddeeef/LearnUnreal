// Fill out your copyright notice in the Description page of Project Settings.


#include "QxDeform/Public/QxDeformComponent.h"

#include "PrimitiveSceneProxy.h"
#include "Engine/StaticMesh.h"


/**
 * @brief 封装deform component的渲染部分
 */
class FQxDeformMeshProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FQxDeformMeshProxy(UQxDeformComponent* Component)
		: FPrimitiveSceneProxy(Component)
	{
		
	}

	// 这个函数必须覆盖，下面这个是固定写法
	virtual uint32 GetMemoryFootprint() const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	};

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}
};


// Sets default values for this component's properties
UQxDeformComponent::UQxDeformComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UQxDeformComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UQxDeformComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UQxDeformComponent::UpdateMeshSectionTransform(int SectionIndex, const FTransform& Transform)
{
	
}

void UQxDeformComponent::FinishTransformsUpdate()
{
}

void UQxDeformComponent::CreateMeshSection(int SectionIndex, UStaticMesh* InMesh, const FTransform& Transform)
{
	check(InMesh);
	if (SectionIndex >= DeformMeshSections.Num())
	{
		DeformMeshSections.SetNum(SectionIndex + 1, false);
	}

	FDeformMeshSection& NewSection = DeformMeshSections[SectionIndex];
	NewSection.Reset();

	// 填充section 的数据，假设static mesh只有一个section
	NewSection.StaticMesh = InMesh;
	NewSection.DeformTransformMat = Transform.ToMatrixNoScale().GetTransposed();

	// 用static mesh来计算section local box
	NewSection.StaticMesh->CalculateExtendedBounds();
	NewSection.SectionLocalBox += NewSection.StaticMesh->GetBoundingBox();

	// 把section 的material 设置给当前component, set material有mesh component实现过了
	SetMaterial(0, NewSection.StaticMesh->GetMaterial(0));

	UpateLocalBounds(); //更新bounds
	MarkRenderStateDirty();  // new section需要重建新的proxy
}

FPrimitiveSceneProxy* UQxDeformComponent::CreateSceneProxy()
{
	if (!SceneProxy)
	{
		return new FQxDeformMeshProxy(this);
	}
	else
	{
		return  SceneProxy;
	}
}

FBoxSphereBounds UQxDeformComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));
    Ret.BoxExtent *= BoundsScale;
    Ret.SphereRadius *= BoundsScale;
	return Super::CalcBounds(LocalToWorld);
}

void UQxDeformComponent::UpateLocalBounds()
{
	FBox LocalBox(ForceInit);

	for(const FDeformMeshSection& Section : DeformMeshSections)
	{
		LocalBox += Section.SectionLocalBox;
	}
	
	LocalBounds = LocalBox.IsValid ? FBoxSphereBounds(LocalBox)
		: FBoxSphereBounds(FVector::ZeroVector, FVector::ZeroVector, 0);
	// 更新global bounds
	UpdateBounds();

	// 需要发送到渲染线程
	MarkRenderTransformDirty();
}


