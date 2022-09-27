// Fill out your copyright notice in the Description page of Project Settings.


#include "QxProceduralMeshComponent.h"

#include <stdexcept>

#include "MeshMaterialShader.h"
#include "Components/BrushComponent.h"

static constexpr  uint32 Clipping_Volumn_Max = 16;

static constexpr  int32 QX_THREAD_GROUP_SIIZE = 16;

namespace 
{
	class FQxUpateBufferCS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxUpateBufferCS);

		// 下面这段是RDG shader参数定义的标准格式
		SHADER_USE_PARAMETER_STRUCT(FQxUpateBufferCS, FGlobalShader)
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,) 
			// SHADER_PARAMETER_UAV(RWStructuredBuffer<float4x4>, QxClippingVolumeBuffer)//这里要和hlsl shader中的参数完全对应
			SHADER_PARAMETER_UAV(RWStructuredBuffer<float4>, QxDynamicPointsSB)
			SHADER_PARAMETER(FVector, RotateCenter)
			SHADER_PARAMETER(float, RotateSpeed)
			SHADER_PARAMETER(float, DeltaTime)
			// SHADER_PARAMETER(float, VolumeNum)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}

		static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
		{
			FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
			OutEnvironment.SetDefine(TEXT("THREAD_GROUP_SIZE"), QX_THREAD_GROUP_SIIZE);
		}
	}; 
	IMPLEMENT_GLOBAL_SHADER(FQxUpateBufferCS, "/QxMeshShaders/QxUpateBufferCS.usf", "MainCompute", SF_Compute);
}

#pragma region RenderBuffers
// Base base for qx procedural vertex / index buffer
class FQxProceduralBuffer
{
public:
	FQxProceduralBuffer(uint32 InCapacity)
		: Capacity(InCapacity)
	{  }

	FQxProceduralBuffer()
		: FQxProceduralBuffer(100000)
	{
	}

	virtual void Resize(const uint32 RequestedCapacity) =  0;

	FORCEINLINE uint32 GetCapacity() const {return Capacity;}
protected:
	uint32 Capacity;
};

#pragma endregion

// 保存所有作为use data传入FQProceduralVertexFactoryParameters的数据
struct FQxProceduralMeshBatchElementUserData
{
	FQxProceduralMeshBatchElementUserData()
		: ClppingVolumeNum(0)
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
	FRHIShaderResourceView* QxClippingVolumeSB;
	FRHIShaderResourceView* DynamicPointsSB;
	FVector Tint;
	FVector4 Contrast;
	FVector4 Saturation;
	FMatrix ClipVolumns[Clipping_Volumn_Max];
	uint32 ClppingVolumeNum;
	uint32 bStartClipped;
	FVector ViewRightVector = FVector(0, 1, 0);
	FVector ViewUpVector = FVector(0, 0, 1);
	uint32 UseClippingVolumeSB = 1;
};

class FQxProceduralVertexFactoryParameters : public FVertexFactoryShaderParameters
{
	DECLARE_INLINE_TYPE_LAYOUT(FQxProceduralVertexFactoryParameters, NonVirtual);
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		Tint.Bind(ParameterMap, TEXT("Tint"));
		Contrast.Bind(ParameterMap, TEXT("Contrast"));
		Saturation.Bind(ParameterMap, TEXT("Saturation"));
		QxClippingVolumeSB.Bind(ParameterMap, TEXT("QxClippingVolumeSB"));
		NumClippingVolumes.Bind(ParameterMap, TEXT("NumClippingVolumes"));
		bStartClipped.Bind(ParameterMap, TEXT("bStartClipped"));
		ViewRightVector.Bind(ParameterMap, TEXT("ViewRightVector"));
		ViewUpVector.Bind(ParameterMap, TEXT("ViewUpVector"));
		ClippingVolumes.Bind(ParameterMap, TEXT("ClippingVolumes"));
		DynamicPointsSB.Bind(ParameterMap, TEXT("DynamicPointsSB"));
		UseClippingVolumeSB.Bind(ParameterMap, TEXT("UseClippingVolumeSB"));
	}

	void GetElementShaderBindings(const class FSceneInterface* Scene,
		const FSceneView* View, const FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType, ERHIFeatureLevel::Type FeatureLevel,
	const FVertexFactory* VertexFactory, const FMeshBatchElement& BatchElement,
	class FMeshDrawSingleShaderBindings& ShaderBindings, FVertexInputStreamArray& VertexStreams) const
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
		if (UserData->QxClippingVolumeSB &&
			QxClippingVolumeSB.IsBound())
		{
			ShaderBindings.Add(QxClippingVolumeSB, UserData->QxClippingVolumeSB);
		}
		if (NumClippingVolumes.IsBound())
		{
			ShaderBindings.Add(NumClippingVolumes, UserData->ClppingVolumeNum);
		}
		if (bStartClipped.IsBound())
		{
			ShaderBindings.Add(bStartClipped, UserData->bStartClipped);
		}
		if (ViewRightVector.IsBound())
		{
			ShaderBindings.Add(ViewRightVector, UserData->ViewRightVector);
		}
		if (ViewUpVector.IsBound())
		{
			ShaderBindings.Add(ViewUpVector, UserData->ViewUpVector);
		}
		if (ClippingVolumes.IsBound())
		{
			ShaderBindings.Add(ClippingVolumes, UserData->ClipVolumns);
		}
		if (DynamicPointsSB.IsBound())
		{
			ShaderBindings.Add(DynamicPointsSB, UserData->DynamicPointsSB);
		}
		if (UseClippingVolumeSB.IsBound())
		{
			ShaderBindings.Add(UseClippingVolumeSB, UserData->UseClippingVolumeSB);
		}
	}
	
	LAYOUT_FIELD(FShaderParameter, Tint);
	LAYOUT_FIELD(FShaderParameter, Contrast);
	LAYOUT_FIELD(FShaderParameter, Saturation);
	LAYOUT_FIELD(FShaderResourceParameter, QxClippingVolumeSB);
	LAYOUT_FIELD(FShaderParameter, ClippingVolumes);
	LAYOUT_FIELD(FShaderParameter, NumClippingVolumes);
	LAYOUT_FIELD(FShaderParameter, bStartClipped);
	LAYOUT_FIELD(FShaderParameter, ViewRightVector);
	LAYOUT_FIELD(FShaderParameter, ViewUpVector);
	LAYOUT_FIELD(FShaderParameter, UseClippingVolumeSB);
	LAYOUT_FIELD(FShaderResourceParameter, DynamicPointsSB);
	
};

class FQxProceduralVertexFactoryParametersPS : public FVertexFactoryShaderParameters
{
	DECLARE_INLINE_TYPE_LAYOUT(FQxProceduralVertexFactoryParametersPS, NonVirtual);
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		Tint.Bind(ParameterMap, TEXT("Tint"));
		QxClippingVolumeSB.Bind(ParameterMap, TEXT("QxClippingVolumeSB"));
		DynamicPointsSB.Bind(ParameterMap, TEXT("DynamicPointsSB"));
	}
	
	void GetElementShaderBindings(
		const FSceneInterface* Scene,
		const FSceneView* View,
		const FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory,
		const FMeshBatchElement& BatchElement,
		class FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const
	{
		const FQxProceduralMeshBatchElementUserData* UserData =
			static_cast<const FQxProceduralMeshBatchElementUserData*>(BatchElement.UserData);
		if (Tint.IsBound())
		{
			ShaderBindings.Add(Tint, UserData->Tint);
		}

		if (UserData->QxClippingVolumeSB && QxClippingVolumeSB.IsBound())
		{
			ShaderBindings.Add(QxClippingVolumeSB, UserData->QxClippingVolumeSB);
		}
		if (UserData->DynamicPointsSB && DynamicPointsSB.IsBound())
		{
			ShaderBindings.Add(DynamicPointsSB, UserData->DynamicPointsSB);
		}
	}
	
	LAYOUT_FIELD(FShaderParameter, Tint);
	LAYOUT_FIELD(FShaderResourceParameter, QxClippingVolumeSB);
	LAYOUT_FIELD(FShaderResourceParameter, DynamicPointsSB);
};

class FQxProceduralVertexFactory : public FVertexFactory
{
public:
	DECLARE_VERTEX_FACTORY_TYPE(FQxProceduralVertexFactory)

	~FQxProceduralVertexFactory()
	{
		if (VertexBuffer.IsInitialized())
		{
			VertexBuffer.ReleaseResource();
		}
	}

	class FQxProceduralPointVertexBuffer : public FVertexBuffer
	{
		// TWeakPtr< TArray<FQxProceduralPoint> > PointsDataWPtr;
		TArray<FQxProceduralPoint>* PointsDataWPtr;


		virtual void InitRHI() override
		{
			// check(PointsDataWPtr.IsValid());
			// TSharedPtr<TArray<FQxProceduralPoint>> PointsDataSP = PointsDataWPtr.Pin();
			check(PointsDataWPtr);
			
			FRHIResourceCreateInfo CreateInfo;
			void* Buffer = nullptr;
			VertexBufferRHI = RHICreateAndLockVertexBuffer(
				PointsDataWPtr->Num() * 4 * sizeof(FQxProceduralPoint), // 这里的4是参照点云，故意复制成4个点
				BUF_Static, // #TODO 这里我是希望实时更新的，但现在先用static的 
				CreateInfo,
				Buffer
				);

			uint8* Dest = (uint8*)Buffer;
			for (int32 i = 0; i < PointsDataWPtr->Num(); ++i)
			{
				FQxProceduralPoint& PointData = PointsDataWPtr->operator[](i);
				// FQxProceduralPoint PointData;
				// PointData.Position = FVector(100, 200, 300);
				// PointData.Color = FColor::Green;
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
			// PointsDataWPtr.Reset();
			PointsDataWPtr = nullptr;
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

	// 注意vertex shader 不加过滤直接return true的情况下会有大量的permutation需要编译，
	// 这里尽量约束需要的范围
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
	void Initialized_RenderThread(FQxProceduralRenderData* NewData)
	{
		if (IsInitialized())
		{
			ReleaseResource();	
		}

		// TSharedPtr<TArray<FQxProceduralPoint>> PointsDataSP = MakeShareable(&(NewData->PointsData));
		// VertexBuffer.PointsDataWPtr  = PointsDataSP;
		VertexBuffer.PointsDataWPtr = &NewData->PointsData;
		// VertexBuffer.InitResource();
		
		

		
		InitResource();
	}
	
	virtual void InitRHI() override
	{
		VertexBuffer.InitResource();
		check(VertexBuffer.IsInitialized());

		

		FVertexDeclarationElementList Elements;
		Elements.Add(AccessStreamComponent(
			FVertexStreamComponent(&VertexBuffer, 0,sizeof(FQxProceduralPoint), VET_Float3),
			0));
		Elements.Add(AccessStreamComponent(
			FVertexStreamComponent(&VertexBuffer, 12,sizeof(FQxProceduralPoint), VET_Color),
			1));
		Elements.Add(AccessStreamComponent(
			FVertexStreamComponent(&VertexBuffer, 16,sizeof(FQxProceduralPoint), VET_UInt),
			2));
		
		InitDeclaration(Elements);
	}
	virtual void ReleaseRHI() override
	{
		VertexBuffer.ReleaseRHI();
		
		FVertexFactory::ReleaseRHI();
	}

public:
	FQxProceduralPointVertexBuffer VertexBuffer;

	
};

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FQxProceduralVertexFactory, SF_Vertex, FQxProceduralVertexFactoryParameters);
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FQxProceduralVertexFactory, SF_Pixel, FQxProceduralVertexFactoryParametersPS);
IMPLEMENT_VERTEX_FACTORY_TYPE(FQxProceduralVertexFactory, "/QxMeshShaders/QxProceduralMeshVertexFactory.ush", /* bUsedWithMaterials */ true, /* bSupportsStaticLighting */ false, /* bSupportsDynamicLighting */ true, /* bPrecisePrevWorldPos */ false, /* bSupportsPositionOnly */ true);


class FQxProceduralOneFrameResource : public FOneFrameResource
{
public:
	FQxProceduralMeshBatchElementUserData Payload;
	~FQxProceduralOneFrameResource() {  }
};

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

		
		QxClippingVolumesBuffer.SafeRelease();
		QxClippingVolumesSRV.SafeRelease();
		
		Material =  nullptr;
		if (RenderData != nullptr)
		{
			delete RenderData;
			RenderData =  nullptr;
		}
	}

	class FQxProcedrualIndexBuffer : public FIndexBuffer
	{
	public:
		virtual void InitRHI() override
		{
			FRHIResourceCreateInfo CreatInfo;
			void* Buffer = nullptr;
			const int32 BufferSize = sizeof(int32) * PointsNums *  6;  // 这里x6是因为一个点的位置产生2个三角形,6个index
			IndexBufferRHI = RHICreateAndLockIndexBuffer(
				sizeof(int32),
				BufferSize,
				BUF_Dynamic,
				CreatInfo,
				Buffer
				);

			uint32* Data = (uint32*)Buffer;

			// PointDataIndex 是在数据中的index，IdxInGPU是产出的index buffer的index的key，
			for (uint32 PointDataIndex = 0, IdxInGPU = 0; PointDataIndex < static_cast<uint32>(PointsNums); ++PointDataIndex)
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
			FIndexBuffer::ReleaseResource();
		}
		
		// TWeakPtr< TArray<FQxProceduralPoint> > PointsDataWPtr;
		// 实际用的顶点的数量, FQxProceduralPoint 的index buffer内容和vertex buffer无关，只需要points nums决定输出的长度
		int32 PointsNums;
	};


	FQxProceduralMeshBatchElementUserData BuildQxProceduralUserDataElment(
		const FSceneView* View,
		const FQxProceduralRenderData* InRenderData
		) const
	{
		FQxProceduralMeshBatchElementUserData UserDataElement;

		const bool bUseSprites = InRenderData->PointSize > 0;

		UserDataElement.Contrast = InRenderData->Contrast;
		UserDataElement.Saturation = InRenderData->Saturation;
		UserDataElement.Tint = InRenderData->ColorTint;
		// UserDataElement.bStartClipped = 
		UserDataElement.ViewRightVector = View->GetViewRight();
		UserDataElement.ViewUpVector = View->GetViewUp();
		UserDataElement.bStartClipped = false;
		// UserDataElement.NumClipNums = InRenderData.
		UserDataElement.QxClippingVolumeSB = QxClippingVolumesSRV;
		UserDataElement.ClppingVolumeNum = InRenderData->EffecticeClippingVolumeNum;

		check(QxDynamicPointsSRV.IsValid());
		UserDataElement.DynamicPointsSB = QxDynamicPointsSRV;
		
		return UserDataElement;
	}
	
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
				FQxProceduralMeshBatchElementUserData& UseData =
					Collector.AllocateOneFrameResource<FQxProceduralOneFrameResource>().Payload;
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
				BatchElement.NumPrimitives = RenderData->PointsData.Num() * 2; // 一个点产生2个三角形
				BatchElement.IndexBuffer = &IndexBuffer;

				UseData = BuildQxProceduralUserDataElment(View, RenderData);
				BatchElement.UserData =  &UseData;

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

	void SetDynamicData_RenderThread(FQxProceduralRenderData* NewDynamicData)
	{
		check(IsInRenderingThread());
		check(NewDynamicData);

		// 删除已经存在的dynamic data
		if (RenderData != nullptr)
		{
			delete RenderData;
			RenderData = nullptr;
		}
		RenderData = NewDynamicData;

		// 创建和初始化Index buffer，更合理的做法应该是在ctor中，先这么做
		{
			CreateAndInitIndexBuffer();
		}


		// 创建volume buffer resources
		{
			if (QxClippingVolumesBuffer.IsValid())
			{
				QxClippingVolumesBuffer.SafeRelease();
			}
			if (QxClippingVolumesSRV.IsValid())
			{
				QxClippingVolumesSRV.SafeRelease();
			}
			
			// TResourceArray 是渲染资源的array，一般情况和tarray相同，UMA的情况下不同
			TResourceArray<FMatrix>* ResourceArray = new TResourceArray<FMatrix>(true);
			// ResourceArray->Reserve(RenderData->ClippingVolumes.Num());
			// ResourceArray->Append(RenderData->ClippingVolumes);
			ResourceArray->AddUninitialized(RenderData->ClippingVolumes.Num());
			for (int32 i = 0; i < RenderData->ClippingVolumes.Num(); ++i)
			{
				ResourceArray->operator[](i) = RenderData->ClippingVolumes[i].PackedShaderData;
			}
			
			// 预期回先用compute shader更新这个buffer，再渲染
			FRHIResourceCreateInfo ResourceCI;
			
			ResourceCI.ResourceArray = ResourceArray;
			ResourceCI.DebugName = TEXT("QxClippingVolumesSB");
			
			QxClippingVolumesBuffer = RHICreateStructuredBuffer(
				sizeof(FMatrix),
				sizeof(FMatrix) * RenderData->ClippingVolumes.Num(),
				BUF_ShaderResource | BUF_Dynamic,
				ResourceCI
				);

			QxClippingVolumesSRV = RHICreateShaderResourceView(
				QxClippingVolumesBuffer
				);

		}

		// 创建dynamic points buffers 
		{
			if (QxDynamicPointsSB.IsValid())
			{
				QxDynamicPointsSB.SafeRelease();
			}
			if (QxDynamicPointsSRV.IsValid())
			{
				QxDynamicPointsSRV.SafeRelease();
			}
			if (QxDynamicPointsUAV.IsValid())
			{
				QxDynamicPointsUAV.SafeRelease();
			}

			// TResourceArray 是渲染资源的array，一般情况和tarray相同，UMA的情况下不同
			TResourceArray<FVector>* ResourceArray = new TResourceArray<FVector>(true);
			ResourceArray->Append(RenderData->DynamicPoints);
			
			// 预期回先用compute shader更新这个buffer，再渲染
			FRHIResourceCreateInfo ResourceCI;
			
			ResourceCI.ResourceArray = ResourceArray;
			ResourceCI.DebugName = TEXT("QxDynamicPointsSB");
			
			QxDynamicPointsSB = RHICreateStructuredBuffer(
				sizeof(FVector),
				sizeof(FVector) * RenderData->DynamicPoints.Num(),
				BUF_ShaderResource |  BUF_UnorderedAccess,
				ResourceCI
				);

			QxDynamicPointsSRV = RHICreateShaderResourceView(
				QxDynamicPointsSB
				);

			QxDynamicPointsUAV =
				RHICreateUnorderedAccessView(QxDynamicPointsSB, false, false);
		}

		
		// vertex factory 也包括了vertex buffer
		VertexFactory.Initialized_RenderThread(RenderData);
	}


	void UpdateDynamicPoints_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		float InRotateSpeed)
	{
		check(IsInRenderingThread());
		check(RenderData);
		FRDGBuilder GraphBuilder(RHICmdList);
		
		{
			RDG_EVENT_SCOPE(GraphBuilder, "RDG_UpdateClipVolumes");
			TShaderMapRef<FQxUpateBufferCS> BufferUpdateCS(GetGlobalShaderMap(GetScene().GetFeatureLevel()));

			FQxUpateBufferCS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxUpateBufferCS::FParameters>();
			PassParams->RotateSpeed = InRotateSpeed;
			PassParams->RotateCenter = FVector::ZeroVector;
			PassParams->QxDynamicPointsSB = QxDynamicPointsUAV;
			PassParams->DeltaTime = 1.f/30.f; //#TODO 得到时间传过来

			// int32 ThreadGroupNum = / (QX_THREAD_GROUP_SIIZE );
			FIntVector GroupCount = 
				FComputeShaderUtils::GetGroupCount( RenderData->DynamicPoints.Num(), QX_THREAD_GROUP_SIIZE);
			FComputeShaderUtils::AddPass(
				GraphBuilder,
				RDG_EVENT_NAME("QxUpdateVolumeBufferPass"),
				ERDGPassFlags::Compute,
				BufferUpdateCS,
				PassParams,
				GroupCount
				);
			RHICmdList.Transition(FRHITransitionInfo(QxDynamicPointsUAV, ERHIAccess::Unknown,
				ERHIAccess::SRVGraphics));
		}
		// QxClippingVolumesBuffer
		// GraphBuilder.AddPass(
		// 	RDG_EVENT_NAME("QxUpdateClippingVolumeRDG"),
		// 	
		// 	);
		
		GraphBuilder.Execute();
	}

	
	int32 GetRequiredVertexCount() const
	{
		return  100000;
	}

private:
	// 创建和初始化Index buffer
	void CreateAndInitIndexBuffer()
	{
		IndexBuffer.PointsNums = RenderData->PointsData.Num();
		if (IsInRenderingThread())
		{
			IndexBuffer.InitResource();
		}
		else
		{
			BeginInitResource(&IndexBuffer);
		}
	}
	
private:
	UMaterialInterface* Material;
	FMaterialRelevance MaterialRelevance;

	FQxProcedrualIndexBuffer IndexBuffer;

	FQxProceduralVertexFactory VertexFactory;

	// #TODO 这里改成智能指针
	FQxProceduralRenderData* RenderData = nullptr;

	// clipping volume buffer的gpu buffer
	FStructuredBufferRHIRef QxClippingVolumesBuffer;
	
	FShaderResourceViewRHIRef QxClippingVolumesSRV;

	FStructuredBufferRHIRef QxDynamicPointsSB;

	FShaderResourceViewRHIRef QxDynamicPointsSRV;

	FUnorderedAccessViewRHIRef QxDynamicPointsUAV;
};


AQxClippingVolume::AQxClippingVolume()
{
	bColored = true;
	BrushColor.R = 0;
	BrushColor.G = 128;
	BrushColor.B = 128;
	BrushColor.A = 255;

	GetBrushComponent()->SetMobility(EComponentMobility::Movable);
	
	SetActorScale3D(FVector(50));
}

FQxProceduralClippingVolumeParams::FQxProceduralClippingVolumeParams(const AQxClippingVolume* ClippingVolume)
{
	const FVector Extent = ClippingVolume->GetActorScale3D() * 100;
	PackedShaderData = FMatrix(
		FPlane(ClippingVolume->GetActorLocation(), Mode  == EQxClippingVolumeMode::ClipInside),
		FPlane(ClippingVolume->GetActorForwardVector(), Extent.X),
		FPlane(ClippingVolume->GetActorRightVector(), Extent.Y),
		FPlane(ClippingVolume->GetActorUpVector(), Extent.Z)
		);
}

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

	// GeneratePointsData();
	//
	// MarkRenderDynamicDataDirty();
	UpdateClipVolume_CS();
	
	UE_LOG(LogTemp, Warning, TEXT("FQxProceduralPoint type size = %d"), sizeof(FQxProceduralPoint));
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
	GeneratePointsData();
	MarkRenderDynamicDataDirty();
}

void UQxProceduralMeshComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	if (SceneProxy)
	{
		// auto ProceduralRenderData = MakeUnique<FQxProceduralRenderData>();
		FQxProceduralRenderData* ProceduralRenderData = new FQxProceduralRenderData;
		
		InitRenderData(ProceduralRenderData);

		FQxProceduralMeshProxy* ProceduralMeshProxy = static_cast<FQxProceduralMeshProxy*>(SceneProxy);
		
		ENQUEUE_RENDER_COMMAND(QxUpdateProcedurlRenderData)(
			[ProceduralRenderData, ProceduralMeshProxy](FRHICommandListImmediate& RHICmdList)
			{
				ProceduralMeshProxy->SetDynamicData_RenderThread(ProceduralRenderData);
			}
			);
	}
}

void UQxProceduralMeshComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);
	SendRenderDynamicData_Concurrent();
}

FBoxSphereBounds UQxProceduralMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	// 先给一个极大的bounds #TODO 之后优化
	FBoxSphereBounds TestBounds;
	TestBounds.Origin = FVector::ZeroVector;
	TestBounds.BoxExtent = FVector(HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX);
	TestBounds.SphereRadius = HALF_WORLD_MAX;
	return  TestBounds;	
	// return Super::CalcBounds(LocalToWorld);
}

void UQxProceduralMeshComponent::GeneratePointsData()
{
	// ...
	PointsData.Reset();
	
	// 先参照ray line 的实现生成一系列line的点
	// 这里的位置计算时以local space为准，计算的
	FVector RayOrigin = FVector::ZeroVector;
	FVector RayDirection = FVector(1.f, 0.f, 0.f);
	// PointsData.AddUninitialized(PointsNums);
	PointsData.AddDefaulted(PointsNums);
	PointsData[0].Position = RayOrigin;
	PointsData[0].Color = TestVertexColor;
	
	
	for (int32 i = 1; i < PointsNums; ++i)
	{
		PointsData[i].Position = RayOrigin + i * RayDirection * PointsDistance;
		PointsData[i].Color = TestVertexColor;
	}
}

void UQxProceduralMeshComponent::UpdateClipVolume_CS()
{
	if (SceneProxy)
	{
		FQxProceduralMeshProxy* ProceduralMeshProxy = static_cast<FQxProceduralMeshProxy*>(SceneProxy);
		float lRotateSpeed = RotationSpeed;
		
		ENQUEUE_RENDER_COMMAND(QxUpdateClipVolumes)(
			[ProceduralMeshProxy, lRotateSpeed](FRHICommandListImmediate& RHICmdList)
			{
				ProceduralMeshProxy->UpdateDynamicPoints_RenderThread(RHICmdList, lRotateSpeed);
			}
			);
	}
}

void UQxProceduralMeshComponent::FillClippingVolumes(TArray<FQxProceduralClippingVolumeParams>& OutClippingVolumes)
{
	OutClippingVolumes.Empty();

	// 先用测试数据填

	for (int32 i = 0; i < (16); ++i)
	{
		FQxProceduralClippingVolumeParams tmpParam;

		FVector Extent = FVector::OneVector * 100;
		tmpParam.Mode = EQxClippingVolumeMode::ClipInside; 
		tmpParam.PackedShaderData = FMatrix(
			FPlane(FVector::ZeroVector, 1),
			FPlane(FVector::ForwardVector, Extent.X),
			FPlane(FVector::RightVector, Extent.Y),
			FPlane(FVector::UpVector, Extent.Z)
			);
		OutClippingVolumes.Add(tmpParam);
	}
}

// 这个调用频率并不低，现在先测试，不考虑性能
void UQxProceduralMeshComponent::InitRenderData(FQxProceduralRenderData* OutRenderData)
{
	OutRenderData->ColorTint = FVector(Tint);
	OutRenderData->PointsData = PointsData; // 注意这里是个深拷贝
	OutRenderData->PointSize = PointSize;
	OutRenderData->Contrast = Contrast;
	OutRenderData->Saturation = Saturation;
	FillClippingVolumes(OutRenderData->ClippingVolumes);

	// make test dynamic points
	constexpr  int32 PointsNum = QX_THREAD_GROUP_SIIZE * QX_THREAD_GROUP_SIIZE * QX_THREAD_GROUP_SIIZE;
	OutRenderData->DynamicPoints.AddUninitialized(PointsNum);
	for (int32 i = 0; i < PointsNum; ++i)
	{
		OutRenderData->DynamicPoints[i] = FVector(i * 50.f, 0, 0);
	}
}



