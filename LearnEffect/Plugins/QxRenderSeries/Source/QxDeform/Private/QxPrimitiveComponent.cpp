// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPrimitiveComponent.h"

#include "CommonRenderResources.h"
#include "MeshDrawShaderBindings.h"
#include "MeshMaterialShader.h"

namespace 
{
	static const uint32 VertexCount = 3;
	
	struct FMyVertex
	{
		FVector Position;
	};

class FQxPrimitiveVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FQxVertexFactory)
public:
	FQxPrimitiveVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName) 
			: FVertexFactory(InFeatureLevel)
	{
		bSupportsManualVertexFetch = false;
	}
	
#pragma region Stanadard
	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
	{
		if ( (Parameters.MaterialParameters.MaterialDomain == MD_Surface &&
				Parameters.MaterialParameters.ShadingModels == MSM_Unlit)
			|| Parameters.MaterialParameters.bIsDefaultMaterial)
		{
			return  true;
		}
		return false;
		// return  IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	};

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment)
	{
	};

	// static void ValidateCompiledResult(
	// const FVertexFactoryType* Type,
	// 	EShaderPlatform Platform,
	// 	const FShaderParameterMap& ParameterMap,
	// 	TArray<FString>& OutErrors);
#pragma endregion

	virtual void InitRHI() override
	{
		check(IsInRenderingThread());

		FRHIResourceCreateInfo  CreateInfo;
		CreateInfo.ResourceArray = nullptr;

		StructuredVertexBuffer = RHICreateStructuredBuffer(sizeof(FMyVertex), sizeof(FMyVertex) * Length,
			BUF_UnorderedAccess | BUF_ShaderResource, CreateInfo);
		StructuredVertexBufferUAV = RHICreateUnorderedAccessView(StructuredVertexBuffer, true, false);
		StructuredVertexBufferSRV = RHICreateShaderResourceView(StructuredVertexBuffer);

		// 设置vertex input layout
		//  现在没有用vertex buffer 输入，所以这么写
		SetDeclaration(GEmptyVertexDeclaration.VertexDeclarationRHI);
	};
	virtual void ReleaseRHI() override
	{
		StructuredVertexBuffer->Release();
		StructuredVertexBufferUAV->Release();
		StructuredVertexBufferSRV->Release();
	};

	uint32 Length  = VertexCount;
	FStructuredBufferRHIRef StructuredVertexBuffer;
	FUnorderedAccessViewRHIRef StructuredVertexBufferUAV;
	FShaderResourceViewRHIRef StructuredVertexBufferSRV;
};

	//QxPrimitiveVertexFactory
IMPLEMENT_VERTEX_FACTORY_TYPE(FQxPrimitiveVertexFactory, "/QxShaders/QxPrimitiveVertexFactory.ush",	true, true, true, true, true);

class FQxUserData : public FOneFrameResource
{
public:
	
	~FQxUserData()
	{
		MyBuffer->Release();
	}
	FShaderResourceViewRHIRef MyBuffer;
};
	
class FQxVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	// 这里用inline type layout 和 deform parameter 的 type layout有什么区别?? #TODO
	DECLARE_INLINE_TYPE_LAYOUT(FQxVertexFactoryShaderParameters, NonVirtual);
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		QxStructedBufferParam.Bind(ParameterMap, TEXT("QxStructuredBuffer"));
	}

	/** 
	 * Gets the vertex factory's shader bindings and vertex streams.
	 * View can be null when caching mesh draw commands (only for supported vertex factories)
	 */
	void GetElementShaderBindings(
		const class FSceneInterface* Scene,
		const class FSceneView* View,
		const class FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const class FVertexFactory* VertexFactory,
		const struct FMeshBatchElement& BatchElement,
		class FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const
	{
		FQxUserData* UserData = (FQxUserData*)BatchElement.UserData;
		ShaderBindings.Add(QxStructedBufferParam, UserData->MyBuffer);
	}
	
	
	LAYOUT_FIELD(FShaderResourceParameter, QxStructedBufferParam);
};
	IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FQxPrimitiveVertexFactory, SF_Vertex, FQxVertexFactoryShaderParameters);
	
	
class FQxPrimitiveSceneProxy : public FPrimitiveSceneProxy
{
public:

	//  几乎是固定写法
#pragma region StandardMethods
	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	};
	virtual uint32 GetMemoryFootprint() const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	};

	uint32 GetAllocatedSize() const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }
#pragma endregion

	FQxPrimitiveSceneProxy(UQxPrimitiveComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, QxPrimitiveVertexFactory(GetScene().GetFeatureLevel(), "QxPrimitiveVertexFactory")
	{
		if (InComponent->GetMaterial(0))
		{
			MaterialRenderProxy = InComponent->GetMaterial(0)->GetRenderProxy();
		}
		else
		{
			MaterialRenderProxy = UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();
		}
		QxPrimitiveVertexFactory.Length = VertexCount;
		ENQUEUE_RENDER_COMMAND(InitQxVertexFactory)(
			[this](FRHICommandListImmediate& RHICmdList)
			{
				QxPrimitiveVertexFactory.InitResource();
			}
			);
	}

	~FQxPrimitiveSceneProxy()
	{
		QxPrimitiveVertexFactory.ReleaseResource();
	}

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override
	{
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				FMeshBatch& MeshBatch = Collector.AllocateMesh();
				// 填充Mesh Batch
				MeshBatch.Type = PT_PointList;
				MeshBatch.VertexFactory = &QxPrimitiveVertexFactory;
				MeshBatch.LCI = nullptr;
				MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();

				MeshBatch.bUseForMaterial = true;
				MeshBatch.MaterialRenderProxy = MaterialRenderProxy;
				
				FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
				// 填充batch element

				BatchElement.NumPrimitives = VertexCount;
				BatchElement.IndexBuffer = nullptr;
				BatchElement.BaseVertexIndex = 0;
				BatchElement.FirstIndex = 0;
				BatchElement.bIsInstanceRuns = false;
				BatchElement.NumInstances = 1;
				BatchElement.MinVertexIndex = 0;
				// primtive uniform buffer 必须绑定
				BatchElement.PrimitiveUniformBuffer = GetUniformBuffer();

				// 传buffer
				FQxUserData* UserData = &Collector.AllocateOneFrameResource<FQxUserData>();
				BatchElement.UserData = (void*)UserData;
				UserData->MyBuffer = QxPrimitiveVertexFactory.StructuredVertexBufferSRV;
				
				Collector.AddMesh(ViewIndex, MeshBatch);
			}
		}
	};

	virtual	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance ViewRelevance;
		ViewRelevance.bDrawRelevance = IsShown(View);
		ViewRelevance.bShadowRelevance = IsShadowCast(View);
		ViewRelevance.bDynamicRelevance = true;
		ViewRelevance.bRenderInMainPass = ShouldRenderInMainPass();
		ViewRelevance.bRenderInDepthPass = ShouldRenderInDepthPass();
		ViewRelevance.bRenderCustomDepth = ShouldRenderCustomDepth();
		//#TODO 什么是lightingchannel
		ViewRelevance.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		ViewRelevance.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		// MaterialRelevance.SetPrimitiveViewRelevance(ViewRelevance);

		// 这句为什么在这
		ViewRelevance.bVelocityRelevance = IsMovable() && ViewRelevance.bOpaque && ViewRelevance.bRenderInMainPass;
		return ViewRelevance;
	}
private:
	FQxPrimitiveVertexFactory QxPrimitiveVertexFactory;
	FMaterialRenderProxy* MaterialRenderProxy;
};

}

// Sets default values for this component's properties
UQxPrimitiveComponent::UQxPrimitiveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	// ...
}


// Called when the game starts
void UQxPrimitiveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

FBoxSphereBounds UQxPrimitiveComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBoxSphereBounds testBounds(LocalToWorld.GetLocation(), FVector::OneVector * 1000000.0f, 1000000.0f);
	// return Super::CalcBounds(LocalToWorld);
	return testBounds;
}

FPrimitiveSceneProxy* UQxPrimitiveComponent::CreateSceneProxy()
{
	return new FQxPrimitiveSceneProxy(this);
}


// Called every frame
void UQxPrimitiveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

