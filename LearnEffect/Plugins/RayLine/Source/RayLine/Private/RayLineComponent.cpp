// Fill out your copyright notice in the Description page of Project Settings.


#include "RayLineComponent.h"
#include <RHICommandList.h>

class FRaylineMeshVertexBuffer : public FVertexBuffer
{
public:
	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo createInfo;
		VertexBufferRHI = RHICreateVertexBuffer(NumVerts * sizeof(FDynamicMeshVertex),
			BUF_Dynamic, createInfo);
	}

	int32 NumVerts;
};

class FRaylineIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo createInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), sizeof(NumIndices) * NumIndices
			, BUF_Dynamic, createInfo);
	}

	int32 NumIndices;
};

struct FRayLineDynamicData
{
	TArray<FVector> HitpointsPosition;

};

class FRayLineVertexFactory : public FLocalVertexFactory
{
public:
	FRayLineVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
		: FLocalVertexFactory(InFeatureLevel, "FRayLineVertexFactory")
	{}

	void Init(const FRaylineMeshVertexBuffer* InVertexBuffer)
	{

	}


};

class FRayLineMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	FRayLineMeshSceneProxy(URayLineComponent* InRayLineCom);

	~FRayLineMeshSceneProxy()
	{
		VertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

	void BuildMesh(FRayLineDynamicData* NewDynamicData)
	{
		TArray<FDynamicMeshVertex> Vertices;
		TArray<int32> Indices;

		for (int32 i = 0; i < NewDynamicData->HitpointsPosition.Num(); i++)
		{
			FDynamicMeshVertex newVert0, newVert1, newVert2, newVert3;
			newVert0.Position = NewDynamicData->HitpointsPosition[i] + FVector(-100, 100, 0);
			newVert1.Position = NewDynamicData->HitpointsPosition[i] + FVector(100, 100, 0);
			newVert2.Position = NewDynamicData->HitpointsPosition[i] + FVector(-100, -100, 0);
			newVert3.Position = NewDynamicData->HitpointsPosition[i] + FVector(100, -100, 0);

			Vertices.Add(newVert0);
			Vertices.Add(newVert1);
			Vertices.Add(newVert2);
			Vertices.Add(newVert3);

			Indices.Add(4 * i);
			Indices.Add(4 * i + 1);
			Indices.Add(4 * i + 2);
			Indices.Add(4 * i + 1);
			Indices.Add(4 * i + 3);
			Indices.Add(4 * i + 2);
		}

		check(Vertices.Num() == GetRequiredVertexCount());
		check(Vertices.Num() == GetRequiredIndexCount());

		void* vertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0,
			Vertices.Num() * sizeof(FDynamicMeshVertex), RLM_WriteOnly);
		FMemory::Memcpy(vertexBufferData, &Vertices[0], Vertices.Num() * sizeof(FDynamicMeshVertex));
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);

		void* indexBufferData = RHILockIndexBuffer(IndexBuffer.IndexBufferRHI, 0,
			Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(indexBufferData, &Indices[0], Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
	}

	void SetDynamicData_RenderThread(FRayLineDynamicData* newDynamicData)
	{
		check(IsInRenderingThread());

		BuildMesh(newDynamicData);
	}


	int32 GetRequiredVertexCount() const
	{
		return 40;
	}

	int32 GetRequiredIndexCount() const
	{
		return 60;
	}

	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily, 
		uint32 VisibilityMap, class FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_FRayLineMeshSceneProxy_GetDynamicMeshElements);

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
		auto wireframeMI = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0.f, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(wireframeMI);

		FMaterialRenderProxy* MaterialProxy = nullptr;
		if (bWireframe)
		{
			MaterialProxy = wireframeMI;
		}
		else
		{
			MaterialProxy = Material->GetRenderProxy();
		}

		for (int32 viewIndex = 0; viewIndex < Views.Num(); viewIndex++)
		{
			if (VisibilityMap & (1 << viewIndex))
			{
				const FSceneView* view = Views[viewIndex];

				// Draw the mesh
				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = GetRequiredIndexCount() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = GetRequiredVertexCount();

				bool bHasPrecomputedVolumetricLightmap;
				FMatrix PreviousLocalToWorld;
				int32 SingleCaptureIndex;
				bool bOutputVelocity;
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);


				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer
					= Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld,
					GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap,
					DrawsVelocity(), bOutputVelocity);
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;

				Collector.AddMesh(viewIndex, Mesh);
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual uint32 GetMemoryFootprint(void) const override
	{
		return sizeof(*this) + GetAllocatedSize();
	}

	uint32 GetAllocateSize(void) const
	{
		return FPrimitiveSceneProxy::GetAllocatedSize();
	}

private:

	UMaterialInterface* Material;
	FRaylineMeshVertexBuffer VertexBuffer;
	FRaylineIndexBuffer IndexBuffer;
	FRayLineVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;
};

FRayLineMeshSceneProxy::FRayLineMeshSceneProxy(URayLineComponent* InRayLineCom) 
	: FPrimitiveSceneProxy(InRayLineCom)
	, MaterialRelevance(InRayLineCom->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	, VertexFactory(GetScene().GetFeatureLevel())
{
	VertexBuffer.NumVerts = GetRequiredVertexCount();
	IndexBuffer.NumIndices = GetRequiredIndexCount();

	const FColor vertexColor = FColor::White;

	VertexFactory.Init(&VertexBuffer);

	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	Material = InRayLineCom->GetMaterial(0);
	if (Material == nullptr)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}

URayLineComponent::URayLineComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;


}

FPrimitiveSceneProxy* URayLineComponent::CreateSceneProxy()
{
	return new FRayLineMeshSceneProxy(this);
}

int32 URayLineComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds URayLineComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds newBounds;
	newBounds.Origin = FVector::ZeroVector;
	newBounds.BoxExtent = FVector(HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	newBounds.SphereRadius = FMath::Sqrt(3.0f * FMath::Square(HALF_WORLD_MAX));
	return newBounds;
}

void URayLineComponent::OnRegister()
{
	Super::OnRegister();
	
	RayLineHitPoints.Reset();

	FVector rayDirection = FVector(1.f, 0.f, 0.f);
	FVector rayOrigin = FVector::ZeroVector;
	const int32 hitPointNum = 10;
	const float secondLength = 50.f;

	RayLineHitPoints.AddUninitialized(hitPointNum);
	RayLineHitPoints[0].HitPosition = rayOrigin;
	RayLineHitPoints[0].HitNextDirection = rayDirection;

	float t = DebugSec;
	for (int32 i = 1; i < hitPointNum; i++)
	{
		RayLineHitPoints[i].HitPosition = rayDirection * t + rayOrigin;
		 
		t += DebugSec;
	}

	MarkRenderDynamicDataDirty();
}

void URayLineComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RayLineHitPoints.Reset();

	FVector rayDirection = FVector(1.f, 0.f, 0.f);
	FVector rayOrigin = FVector::ZeroVector;
	const int32 hitPointNum = 10;
	const float secondLength = 50.f;

	RayLineHitPoints.AddUninitialized(hitPointNum);
	RayLineHitPoints[0].HitPosition = rayOrigin;
	RayLineHitPoints[0].HitNextDirection = rayDirection;

	float t = DebugSec;
	for (int32 i = 1; i < hitPointNum; i++)
	{
		RayLineHitPoints[i].HitPosition = rayDirection * t + rayOrigin;

		t += DebugSec;
	}

	MarkRenderDynamicDataDirty();

	UpdateComponentToWorld();

}

void URayLineComponent::SendRenderDynamicData_Concurrent()
{
	if (SceneProxy)
	{
		FRayLineDynamicData* newDynamicData = new FRayLineDynamicData;
		newDynamicData->HitpointsPosition.AddUninitialized(RayLineHitPoints.Num());
		for (int32 i = 0; i < RayLineHitPoints.Num(); i++)
		{
			newDynamicData->HitpointsPosition[i] = RayLineHitPoints[i].HitPosition;
		}

		/*FSendRayLineComponentDynamicData,
			FRayLineMeshSceneProxy*, CableSceneProxy,
			(FRayLineMeshSceneProxy*)SceneProxy,
			FRayLineDynamicData*, NewDynamicData, NewDynamicData,
		{
			CableSceneProxy->
		}

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			
		);*/
		FRayLineMeshSceneProxy* raylineSceneProxy = (FRayLineMeshSceneProxy*)SceneProxy;

		ENQUEUE_RENDER_COMMAND(FSendRayLineDynamicData)(
			[raylineSceneProxy, newDynamicData](FRHICommandListImmediate& RHICmdList)
			{
				raylineSceneProxy->SetDynamicData_RenderThread(newDynamicData);
			}
			);
	}
}

void URayLineComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);
	SendRenderDynamicData_Concurrent();
}

