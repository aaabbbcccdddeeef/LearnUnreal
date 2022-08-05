// Fill out your copyright notice in the Description page of Project Settings.


#include "RayLineComponent.h"
#include <RHICommandList.h>
#include <MeshMaterialShader.h>

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
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), sizeof(int32) * NumIndices
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
	{
		bSupportsManualVertexFetch = false;
		
	}


	/* This is the main method that we're interested in*/
	/* Here we can initialize our RHI resources, so we can decide what would be in the final streams and the vertex declaration*/
	/* In the LocalVertexFactory, 3 vertex declarations are initialized; PositionOnly, PositionAndNormalOnly, and the default, which is the one that will be used in the main rendering*/
	/* PositionOnly is mandatory if you're enabling depth passes, however we can get rid of the PositionAndNormal since we're not interested in shading and we're only supporting unlit materials*/
	virtual void InitRHITest() 
	{

		// Check if this vertex factory has a valid feature level that is supported by the current platform
		check(HasValidFeatureLevel());


		//The vertex declaration element lists (Nothing but an array of FVertexElement)
		FVertexDeclarationElementList Elements; //Used for the Default vertex stream
		FVertexDeclarationElementList PosOnlyElements; // Used for the PositionOnly vertex stream

		if (Data.PositionComponent.VertexBuffer != NULL)
		{
			//We add the position stream component to both elemnt lists
			Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
			PosOnlyElements.Add(AccessStreamComponent(Data.PositionComponent, 0, EVertexInputStreamType::PositionOnly));
		}

		//Initialize the Position Only vertex declaration which will be used in the depth pass
		InitDeclaration(PosOnlyElements, EVertexInputStreamType::PositionOnly);

		//We add all the available texcoords to the default element list, that's all what we'll need for unlit shading
		if (Data.TextureCoordinates.Num())
		{
			const int32 BaseTexCoordAttribute = 4; //这里为啥不是从0开始 #Unkown
			for (int32 CoordinateIndex = 0; CoordinateIndex < Data.TextureCoordinates.Num(); CoordinateIndex++)
			{
				Elements.Add(AccessStreamComponent(
					Data.TextureCoordinates[CoordinateIndex],
					BaseTexCoordAttribute + CoordinateIndex
				));
			}

			for (int32 CoordinateIndex = Data.TextureCoordinates.Num(); CoordinateIndex < MAX_STATIC_TEXCOORDS / 2; CoordinateIndex++)
			{
				Elements.Add(AccessStreamComponent(
					Data.TextureCoordinates[Data.TextureCoordinates.Num() - 1],
					BaseTexCoordAttribute + CoordinateIndex
				));
			}
		}

		check(Streams.Num() > 0);

		InitDeclaration(Elements);
		check(IsValidRef(GetDeclaration()));
	}

	static void InitOrUpdateResource(FRenderResource* Resource)
	{
		if (!Resource->IsInitialized())
		{
			Resource->InitResource();
		}
		else
		{
			Resource->UpdateRHI();
		}
	}

	static void InitRayLineVertexFactory(FRayLineVertexFactory* VertexFactory,
		FRaylineMeshVertexBuffer* InVertexBuffer)
	{
		if (IsInRenderingThread())
		{
			InitOrUpdateResource(InVertexBuffer);
			FLocalVertexFactory::FDataType newData;
			newData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(InVertexBuffer,
				FDynamicMeshVertex, Position, VET_Float3);
			newData.TextureCoordinates.Add(
				FVertexStreamComponent(InVertexBuffer,
					STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate)
					, sizeof(FDynamicMeshVertex), VET_Float2)
			);
			newData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(InVertexBuffer,
				FDynamicMeshVertex, TangentX, VET_PackedNormal);
			newData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(InVertexBuffer,
				FDynamicMeshVertex, TangentZ, VET_PackedNormal);
			VertexFactory->SetData(newData);
		}

		else
		{

			ENQUEUE_RENDER_COMMAND(FRaylineVertexFactoryInit)(
				[VertexFactory, InVertexBuffer](FRHICommandListImmediate& RHICmdList)
				{
					InitOrUpdateResource(InVertexBuffer);

					
					//FLocalVertexFactory::FDataType Data;
					FLocalVertexFactory::FDataType newData;
					{
						newData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(InVertexBuffer,
							FDynamicMeshVertex, Position, VET_Float3);
						//newData.PreSkinPositionComponentSRV = InVertexBuffer->
						newData.TextureCoordinates.Add(
							FVertexStreamComponent(InVertexBuffer,
								STRUCT_OFFSET(FDynamicMeshVertex, TextureCoordinate)
								, sizeof(FDynamicMeshVertex), VET_Float2)
						);
						newData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(InVertexBuffer,
							FDynamicMeshVertex, TangentX, VET_PackedNormal);
						newData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(InVertexBuffer,
							FDynamicMeshVertex, TangentZ, VET_PackedNormal);
					}
					
					
					//StaticMeshData.PositionComponentSRV = PositionComponentSRV;

					//VertexFactory->PositionVertexBuffer
					//InVertexBuffer->bin

					VertexFactory->SetData(newData);
					InitOrUpdateResource(VertexFactory);
				}
				);
		}
	}


	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
	{
		if ((Parameters.MaterialParameters.MaterialDomain == MD_Surface &&
			Parameters.MaterialParameters.ShadingModels == MSM_Unlit) ||
			Parameters.MaterialParameters.bIsDefaultMaterial)
		{
			return true;
		}
		return false;
	}


	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		const bool ContainsManualVertexFetch = OutEnvironment.GetDefinitions().Contains("MANUAL_VERTEX_FETCH");
		if (!ContainsManualVertexFetch)
		{
			OutEnvironment.SetDefine(TEXT("MANUAL_VERTEX_FETCH"), TEXT("0"));
		}

		OutEnvironment.SetDefine(TEXT("DEFORM_MESH"), TEXT("1"));
	}
};

class FRayLineMeshSceneProxy : public FPrimitiveSceneProxy
{
public:

	FRayLineMeshSceneProxy(URayLineComponent* InRayLineCom) : FPrimitiveSceneProxy(InRayLineCom)
		, MaterialRelevance(InRayLineCom->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, VertexFactory(GetScene().GetFeatureLevel(), "FRayLineProxy")
	{
		/*
		//VertexBuffer.InitWithDummyData(&VertexFactory, GetRequiredVertexCount());
		VertexBuffers.InitWithDummyData(&VertexFactory, GetRequiredVertexCount());

		//VertexBuffer.NumVerts = GetRequiredVertexCount();
		IndexBuffer.NumIndices = GetRequiredIndexCount();

		const FColor vertexColor = FColor::White;

		//VertexFactory.Init(&VertexBuffer);
		FRayLineVertexFactory::InitRayLineVertexFactory(&VertexFactory, &VertexBuffer);

		//BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);
		//BeginInitResource(&VertexFactory);

		Material = InRayLineCom->GetMaterial(0);
		if (Material == nullptr)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
		*/

		VertexBuffers.InitWithDummyData(&VertexFactory, GetRequiredVertexCount());

		IndexBuffer.NumIndices = GetRequiredIndexCount();

		// Enqueue initialization of render resource
		BeginInitResource(&IndexBuffer);

		// Grab material
		Material = InRayLineCom->GetMaterial(0);
		if (Material == NULL)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
	}

	~FRayLineMeshSceneProxy()
	{
		VertexBuffers.PositionVertexBuffer.ReleaseResource();
		VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
		VertexBuffers.ColorVertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();

		if (DynamicData != NULL)
		{
			delete DynamicData;
		}
	}

	void BuildMesh(FRayLineDynamicData* NewDynamicData,
		TArray<FDynamicMeshVertex>& OutVertices, TArray<int32>& OutIndices)
	{
		//TArray<FDynamicMeshVertex> Vertices;
		//TArray<int32> Indices;

		for (int32 i = 0; i < NewDynamicData->HitpointsPosition.Num(); i++)
		{
			FDynamicMeshVertex newVert0, newVert1, newVert2, newVert3;
			newVert0.Position = NewDynamicData->HitpointsPosition[i] + FVector(-100, 100, 0);
			newVert1.Position = NewDynamicData->HitpointsPosition[i] + FVector(100, 100, 0);
			newVert2.Position = NewDynamicData->HitpointsPosition[i] + FVector(-100, -100, 0);
			newVert3.Position = NewDynamicData->HitpointsPosition[i] + FVector(100, -100, 0);

			OutVertices.Add(newVert0);
			OutVertices.Add(newVert1);
			OutVertices.Add(newVert2);
			OutVertices.Add(newVert3);

			OutIndices.Add(4 * i);
			OutIndices.Add(4 * i + 1);
			OutIndices.Add(4 * i + 2);
			OutIndices.Add(4 * i + 1);
			OutIndices.Add(4 * i + 3);
			OutIndices.Add(4 * i + 2);
		}

		/*check(Vertices.Num() == GetRequiredVertexCount());
		check(Indices.Num() == GetRequiredIndexCount());

		void* vertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0,
			Vertices.Num() * sizeof(FDynamicMeshVertex), RLM_WriteOnly);
		FMemory::Memcpy(vertexBufferData, &Vertices[0], Vertices.Num() * sizeof(FDynamicMeshVertex));
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);

		void* indexBufferData = RHILockIndexBuffer(IndexBuffer.IndexBufferRHI, 0,
			Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(indexBufferData, &Indices[0], Indices.Num() * sizeof(int32));*/
		//RHIUnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
	}

	void SetDynamicData_RenderThread(FRayLineDynamicData* newDynamicData)
	{
		check(IsInRenderingThread());

		// Free existing data if present
		if (DynamicData)
		{
			delete DynamicData;
			DynamicData = NULL;
		}
		DynamicData = newDynamicData;

		// Build mesh from cable points
		TArray<FDynamicMeshVertex> Vertices;
		TArray<int32> Indices;
		BuildMesh(newDynamicData, Vertices, Indices);


		check(Vertices.Num() == GetRequiredVertexCount());
		check(Indices.Num() == GetRequiredIndexCount());

		for (int i = 0; i < Vertices.Num(); i++)
		{
			const FDynamicMeshVertex& Vertex = Vertices[i];

			VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertex.TangentX.ToFVector(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector());
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertex.TextureCoordinate[0]);
			VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertex.Color;
		}

		{
			auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
			void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
			FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
			RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
		}

		{
			auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
			void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
			FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
			RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
		}

		{
			auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
			void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
			FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
			RHIUnlockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
		}

		{
			auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
			void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
			FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
			RHIUnlockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
		}

		void* IndexBufferData = RHILockIndexBuffer(IndexBuffer.IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, &Indices[0], Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
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

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		FMaterialRenderProxy* MaterialProxy = NULL;
		if (bWireframe)
		{
			MaterialProxy = WireframeMaterialInstance;
		}
		else
		{
			MaterialProxy = Material->GetRenderProxy();
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				// Draw the mesh.
				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;

				bool bHasPrecomputedVolumetricLightmap;
				FMatrix PreviousLocalToWorld;
				int32 SingleCaptureIndex;
				bool bOutputVelocity;
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, DrawsVelocity(), bOutputVelocity);
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = GetRequiredIndexCount() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = GetRequiredVertexCount();
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;
				Collector.AddMesh(ViewIndex, Mesh);
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		// Result.bShadowRelevance = IsShadowCast(View);
		Result.bShadowRelevance = false;
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
	//FRaylineMeshVertexBuffer VertexBuffer;
	FRaylineIndexBuffer IndexBuffer;
	//FRayLineVertexFactory VertexFactory;
	FLocalVertexFactory VertexFactory;

	FStaticMeshVertexBuffers VertexBuffers;

	FMaterialRelevance MaterialRelevance;

	FRayLineDynamicData* DynamicData = nullptr;
};

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

