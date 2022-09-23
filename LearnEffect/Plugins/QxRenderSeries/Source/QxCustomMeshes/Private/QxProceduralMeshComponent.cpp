// Fill out your copyright notice in the Description page of Project Settings.


#include "QxProceduralMeshComponent.h"

#include "MeshMaterialShader.h"

static uint32 Clipping_Volumn_Max = 16;

// 保存所有作为use data传入FQProceduralVertexFactoryParameters的数据
struct FQxProceduralMeshBatchElementUserData
{
	FQxProceduralMeshBatchElementUserData()
		: NumClipNums(0)
		, bStartClipped(false)
	{
		for (int32 i = 0; i < Clipping_Volumn_Max; ++i)
		{
			ClipVolumns[i] = FMatrix(
				FPlane(FVector::ZeroVector, 0),
				FPlane(FVector::ForwardVector, FLT_MAX),
				FPlane(FVector::RightVector, FLT_MAX),
				FPlane(FVector::UpVector, FLT_MAX)
				);
		}
	}
	FRHIShaderResourceView* ClippingVolumeBuffer;
	FVector Tint;
	FVector4 Contrast;
	FVector4 Saturation;
	FMatrix ClipVolumns[Clipping_Volumn_Max];
	uint32 NumClipNums;
	uint32 bStartClipped;
};

class FQProceduralVertexFactoryParameters : public FVertexFactoryShaderParameters
{
	DECLARE_INLINE_TYPE_LAYOUT(FQProceduralVertexFactoryParameters, NonVirtual);
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		Tint.Bind(ParameterMap, TEXT("Tint"));
		Contrast.Bind(ParameterMap, TEXT("Contrast"));
		Saturation.Bind(ParameterMap, TEXT("Saturation"));
		ClippingVolumeBuffer.Bind(ParameterMap, TEXT("ClippingVolumeBuffer"));
		NumClippoingVolumes.Bind(ParameterMap, TEXT("NumClippoingVolumes"));
		bStartClipped.Bind(ParameterMap, TEXT("bStartClipped"));
	}

	void GetElementShaderBindings(const class FSceneInterface* Scene, const FSceneView* View, const FMeshMaterialShader* Shader, const EVertexInputStreamType InputStreamType, ERHIFeatureLevel::Type FeatureLevel,
	const FVertexFactory* VertexFactory, const FMeshBatchElement& BatchElement, class FMeshDrawSingleShaderBindings& ShaderBindings, FVertexInputStreamArray& VertexStreams) const
	{
		const FQxProceduralMeshBatchElementUserData* UserData =
			static_cast<const FQxProceduralMeshBatchElementUserData*>(BatchElement.UserData);
		if (Saturation.IsBound())
		{
			ShaderBindings.Add(Saturation, UserData->Saturation);
		}
		if (Contrast.IsBound())
		{
			ShaderBindings.Add(Contrast, UserData->Contrast);
		}
		if (Tint.IsBound())
		{
			ShaderBindings.Add(Tint, UserData->Tint);
		}
		if (UserData->ClippingVolumeBuffer &&
			ClippingVolumeBuffer.IsBound())
		{
			ShaderBindings.Add(ClippingVolumeBuffer, UserData->ClippingVolumeBuffer);
		}
		if (NumClippoingVolumes.IsBound())
		{
			ShaderBindings.Add(NumClippoingVolumes, UserData.)
		}
	}
	
	LAYOUT_FIELD(FShaderParameter, Tint);
	LAYOUT_FIELD(FShaderParameter, Contrast);
	LAYOUT_FIELD(FShaderParameter, Saturation);
	LAYOUT_FIELD(FShaderResourceParameter, ClippingVolumeBuffer);
	LAYOUT_FIELD(FShaderParameter, NumClippoingVolumes);
	LAYOUT_FIELD(FShaderParameter, bStartClipped);
};


class FQxProceduralVertexFactory : public FVertexFactory
{
public:
	DECLARE_VERTEX_FACTORY_TYPE(FQxProceduralVertexFactory)

	class FQxProceduralPointVertexBuffer : public FVertexBuffer
	{
		TWeakPtr< TArray<FQxProceduralPoint> > PointsDataWPtr;

		virtual void InitRHI() override
		{
			check(PointsDataWPtr.IsValid());
			TSharedPtr<TArray<FQxProceduralPoint>> PointsDataSP = PointsDataWPtr.Pin();
			
			FRHIResourceCreateInfo CreateInfo;
			void* Buffer = nullptr;
			VertexBufferRHI = RHICreateAndLockVertexBuffer(
				PointsDataSP->Num() * 4 * sizeof(FQxProceduralPoint), // 这里的4是参照点云，故意复制成4个点
				BUF_Static, // #TODO 这里我是希望实时更新的，但现在先用static的 
				CreateInfo,
				Buffer
				);

			uint8* Dest = (uint8*)Buffer;
			for (int32 i = 0; i < PointsDataSP->Num(); ++i)
			{
				FQxProceduralPoint& PointData = PointsDataSP.Get()->operator[](i);
				FMemory::Memcpy(Dest, &PointData, sizeof(FQxProceduralPoint));
				Dest += sizeof(FQxProceduralPoint);
				FMemory::Memcpy(Dest, &PointData, sizeof(FQxProceduralPoint));
				Dest += sizeof(FQxProceduralPoint);
				FMemory::Memcpy(Dest, &PointData, sizeof(FQxProceduralPoint));
				Dest += sizeof(FQxProceduralPoint);
				FMemory::Memcpy(Dest, &PointData, sizeof(FQxProceduralPoint));
				Dest += sizeof(FQxProceduralPoint);
			}

			RHIUnlockVertexBuffer(VertexBufferRHI);
			Buffer = nullptr;
		}

		virtual void ReleaseResource() override
		{
			PointsDataWPtr.Reset();
			FVertexBuffer::ReleaseResource();
		}

		virtual FString GetFriendlyName() const override
		{
			return TEXT("QxProceduralPointVertexBuffer");
		};
		friend FQxProceduralVertexFactory;
	};

	FQxProceduralVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
		: FVertexFactory(InFeatureLevel)
	{
		bSupportsManualVertexFetch = false;
	}

	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
	{
		if ((Parameters.MaterialParameters.MaterialDomain == MD_Surface
			&& Parameters.MaterialParameters.ShadingModels == MSM_Unlit) 
			|| Parameters.MaterialParameters.bIsDefaultMaterial)
		{
			return true;
		}
		return false;
	}

	// 主要是初始化vertex buffer的数据
	void Initialized(FQxProceduralUpdateData* NewData)
	{
		if (IsInitialized())
		{
			ReleaseResource();	
		}

		TSharedPtr<TArray<FQxProceduralPoint>> PointsDataSP = MakeShareable(&(NewData->PointsData));
		VertexBuffer.PointsDataWPtr  = PointsDataSP;
		InitResource();
	}
	
	virtual void InitRHI() override
	{
		VertexBuffer.InitResource();

		FVertexDeclarationElementList Elements;
		Elements.Add(AccessStreamComponent(
			FVertexStreamComponent(&VertexBuffer, 0,sizeof(FQxProceduralPointVertexBuffer), VET_Float3),
			0));
		Elements.Add(AccessStreamComponent(
			FVertexStreamComponent(&VertexBuffer, 12,sizeof(FQxProceduralPointVertexBuffer), VET_Color),
			2));
		Elements.Add(AccessStreamComponent(
			FVertexStreamComponent(&VertexBuffer, 16,sizeof(FQxProceduralPointVertexBuffer), VET_UInt),
			3));
		
		InitDeclaration(Elements);
	}
	virtual void ReleaseRHI() override
	{
		FVertexFactory::ReleaseRHI();
		VertexBuffer.ReleaseRHI();
	}

public:
	FQxProceduralPointVertexBuffer VertexBuffer;
};

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FQxProceduralVertexFactory, SF_Vertex, FQProceduralVertexFactoryParameters);
IMPLEMENT_VERTEX_FACTORY_TYPE(FQxProceduralVertexFactory, "/QxShaders/QxProceduralMeshVertexFactory.ush", /* bUsedWithMaterials */ true, /* bSupportsStaticLighting */ false, /* bSupportsDynamicLighting */ true, /* bPrecisePrevWorldPos */ false, /* bSupportsPositionOnly */ true);

class FQxProceduralMeshProxy : public FPrimitiveSceneProxy
{
public:
	FQxProceduralMeshProxy(UQxProceduralMeshComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, MaterialRelevance(InComponent->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, VertexFactory(GetScene().GetFeatureLevel())
	{
		Material = InComponent->GetMaterial(0);
		if (Material == nullptr)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
	}

	~FQxProceduralMeshProxy()
	{
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
		if (DynamicData != nullptr)
		{
			delete DynamicData;
			DynamicData =  nullptr;
		}
	}

	class FQxProcedrualIndexBuffer : public FIndexBuffer
	{
	public:
		virtual void InitRHI() override
		{
			check(PointsDataWPtr.IsValid());
			TSharedPtr<TArray<FQxProceduralPoint>> PointsDataSP = PointsDataWPtr.Pin();
			
			FRHIResourceCreateInfo CreatInfo;
			void* Buffer = nullptr;
			const int32 BufferSize = sizeof(int32) * PointsDataSP->Num() * PointsDataSP->GetTypeSize() * 6;
			IndexBufferRHI = RHICreateAndLockIndexBuffer(
				sizeof(int32),
				BufferSize, // 这里x6是因为一个点的位置产生2个三角形,6个index
				BUF_Dynamic,
				CreatInfo,
				Buffer
				);

			uint32* Data = (uint32*)Buffer;

			// PointDataIndex 是在数据中的index，IdxInGPU是产出的index buffer的index的key，
			for (uint32 PointDataIndex = 0, IdxInGPU = 0; PointDataIndex < static_cast<uint32>(PointsDataSP->Num()); ++PointDataIndex)
			{
				const uint32 v = PointDataIndex *  4;

				// Full quad
				Data[IdxInGPU++] = v;
				Data[IdxInGPU++] = v + 1;
				Data[IdxInGPU++] = v + 2;
				Data[IdxInGPU++] = v;
				Data[IdxInGPU++] = v + 2;
				Data[IdxInGPU++] = v + 3;
				// // Points
				// Data[PointOffset + i] = v;
			}

			RHIUnlockIndexBuffer(IndexBufferRHI);
			Buffer = nullptr;
		}

		virtual FString GetFriendlyName() const override { return TEXT("QxProcedrualIndexBuffer"); }

		virtual void ReleaseResource() override
		{
			PointsDataWPtr.Reset();
			FIndexBuffer::ReleaseResource();
		}
		
		TWeakPtr< TArray<FQxProceduralPoint> > PointsDataWPtr;
	};
	
	
	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override
	{
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
		{
			const FSceneView* View = Views[ViewIndex];

			if (IsShown(View) &&
				(VisibilityMap & (1 << ViewIndex)))
			{
				FMeshBatch& MeshBatch = Collector.AllocateMesh();

				// 设置Mesh Batch
				MeshBatch.Type = PT_TriangleList;
				MeshBatch.LODIndex = 0;
				MeshBatch.VertexFactory = &VertexFactory;
				MeshBatch.bWireframe = false;
				MeshBatch.MaterialRenderProxy = Material->GetRenderProxy();
				MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
				MeshBatch.DepthPriorityGroup = SDPG_World;

				// 设置Mesh Batch Element
				FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
				BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();
				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = DynamicData->PointsData.Num() * 2; // 一个点产生2个三角形
				

				Collector.AddMesh(ViewIndex, MeshBatch);
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bStaticRelevance = false;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	// 下面是固定写法
#pragma region StatableParttern
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
	virtual uint32 GetMemoryFootprint(void) const override
	{
		return sizeof(*this) + GetAllocatedSize();
	}

	uint32 GetAllocateSize(void) const
	{
		return FPrimitiveSceneProxy::GetAllocatedSize();
	}
#pragma endregion

	void SetDynamicData_RenderThread(FQxProceduralUpdateData* NewDynamicData)
	{
		check(IsInRenderingThread());
		check(NewDynamicData);

		// 删除已经存在的dynamic data
		if (DynamicData != nullptr)
		{
			delete DynamicData;
			DynamicData = nullptr;
		}

		VertexFactory.Initialized(NewDynamicData);
	}
private:
	UMaterialInterface* Material;
	FMaterialRelevance MaterialRelevance;

	FQxProcedrualIndexBuffer IndexBuffer;

	FQxProceduralVertexFactory VertexFactory;

	// #TODO 这里改成智能指针
	FQxProceduralUpdateData* DynamicData = nullptr;
};


// Sets default values for this component's properties
UQxProceduralMeshComponent::UQxProceduralMeshComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UQxProceduralMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UQxProceduralMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                               FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FPrimitiveSceneProxy* UQxProceduralMeshComponent::CreateSceneProxy()
{
	// return Super::CreateSceneProxy();
	return new FQxProceduralMeshProxy(this);
}

int32 UQxProceduralMeshComponent::GetNumMaterials() const
{
	// return Super::GetNumMaterials();
	return 1;
}

FBoxSphereBounds UQxProceduralMeshComponent::CalcLocalBounds() const
{
	return Super::CalcLocalBounds();
}

void UQxProceduralMeshComponent::OnRegister()
{
	Super::OnRegister();
}

void UQxProceduralMeshComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();
}

void UQxProceduralMeshComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);
}

