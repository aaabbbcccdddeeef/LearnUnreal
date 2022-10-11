// Fill out your copyright notice in the Description page of Project Settings.


#include "QxClippedStaticMeshComponent.h"

#include "MeshDrawShaderBindings.h"
#include "LocalVertexFactory.h"
#include "MeshMaterialShader.h"
#include "PhysicalMaterials/PhysicalMaterialMask.h"
#include "ShaderParameters.h"
#include "TessellationRendering.h"
#include "ZZClipperManager.h"



TAutoConsoleVariable<int32> CVarQxBloomPassAmount(
TEXT("r.ZZRender.TestClippVolmuIndex"),
0,
TEXT("Test Clipping Volume Index"),
ECVF_RenderThreadSafe 
);

class FMaterial;
class FMeshDrawSingleShaderBindings;
class FPrimitiveSceneProxy;
struct FVertexFactoryShaderPermutationParameters;

class FZZStaticMeshUserData
{
public:
    FRHIShaderResourceView* QxClippingVolumeSB = nullptr;
	uint32 EffectVolumeNum = 0;
};

// 这里参照static mesh
static  bool GUseReversedIndexBuffer = true; 

/**
 * Shader parameters for all LocalVertexFactory derived classes.
 */
class FZZLocalVertexFactoryShaderParametersBase : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FZZLocalVertexFactoryShaderParametersBase, NonVirtual);
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		LODParameter.Bind(ParameterMap, TEXT("SpeedTreeLODInfo"));
		bAnySpeedTreeParamIsBound = LODParameter.IsBound() || ParameterMap.ContainsParameterAllocation(TEXT("SpeedTreeData"));
	}

	void GetElementShaderBindingsBase(
		const FSceneInterface* Scene,
		const FSceneView* View,
		const FMeshMaterialShader* Shader, 
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory, 
		const FMeshBatchElement& BatchElement,
		FRHIUniformBuffer* VertexFactoryUniformBuffer,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams
	) const
{
	const auto* LocalVertexFactory = static_cast<const FLocalVertexFactory*>(VertexFactory);
	
	if (LocalVertexFactory->SupportsManualVertexFetch(FeatureLevel) || UseGPUScene(GMaxRHIShaderPlatform, FeatureLevel))
	{
		if (!VertexFactoryUniformBuffer)
		{
			// No batch element override
			VertexFactoryUniformBuffer = LocalVertexFactory->GetUniformBuffer();
		}

		ShaderBindings.Add(Shader->GetUniformBufferParameter<FLocalVertexFactoryUniformShaderParameters>(), VertexFactoryUniformBuffer);
	}

	//@todo - allow FMeshBatch to supply vertex streams (instead of requiring that they come from the vertex factory), and this userdata hack will no longer be needed for override vertex color
	if (BatchElement.bUserDataIsColorVertexBuffer)
	{
		FColorVertexBuffer* OverrideColorVertexBuffer = (FColorVertexBuffer*)BatchElement.UserData;
		check(OverrideColorVertexBuffer);

		if (!LocalVertexFactory->SupportsManualVertexFetch(FeatureLevel))
		{
			LocalVertexFactory->GetColorOverrideStream(OverrideColorVertexBuffer, VertexStreams);
		}	
	}

	
}

	FZZLocalVertexFactoryShaderParametersBase()
		: bAnySpeedTreeParamIsBound(false)
	{
	}

	// SpeedTree LOD parameter
	LAYOUT_FIELD(FShaderParameter, LODParameter);

	// True if LODParameter is bound, which puts us on the slow path in GetElementShaderBindings
	LAYOUT_FIELD(bool, bAnySpeedTreeParamIsBound);
};
IMPLEMENT_TYPE_LAYOUT(FZZLocalVertexFactoryShaderParametersBase);


/** Shader parameter class used by FLocalVertexFactory only - no derived classes. */
class FZZLocalVertexFactoryShaderParameters : public FZZLocalVertexFactoryShaderParametersBase
{
	DECLARE_TYPE_LAYOUT(FZZLocalVertexFactoryShaderParameters, NonVirtual);
public:

	void Bind(const FShaderParameterMap& ParameterMap)
	{
		FZZLocalVertexFactoryShaderParametersBase::Bind(ParameterMap);
		ZZClipingVolumesSB.Bind(ParameterMap, TEXT("ZZClipingVolumesSB"));
		ZZClippingVolumeNum.Bind(ParameterMap, TEXT("ZZClippingVolumeNum"));
	}
	
	void GetElementShaderBindings(
		const FSceneInterface* Scene,
		const FSceneView* View,
		const FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory,
		const FMeshBatchElement& BatchElement,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams
	) const
	{
		// Decode VertexFactoryUserData as VertexFactoryUniformBuffer
		FRHIUniformBuffer* VertexFactoryUniformBuffer = static_cast<FRHIUniformBuffer*>(BatchElement.VertexFactoryUserData);

		FZZLocalVertexFactoryShaderParametersBase::GetElementShaderBindingsBase(
			Scene,
			View,
			Shader,
			InputStreamType,
			FeatureLevel,
			VertexFactory,
			BatchElement,
			VertexFactoryUniformBuffer,
			ShaderBindings,
			VertexStreams);
		const FZZStaticMeshUserData* UserData =
		   static_cast<const FZZStaticMeshUserData*>(BatchElement.UserData);
		// if (ZZClipingVolumesSB.IsBound() && UserData->QxClippingVolumeSB)
		// {
		// 	ShaderBindings.Add(ZZClipingVolumesSB, UserData->QxClippingVolumeSB);
		// }
		// if (ZZClippingVolumeNum.IsBound())
		// {
		// 	ShaderBindings.Add(ZZClippingVolumeNum, UserData->EffectVolumeNum);
		// }


		// UZZClipperSubsystem* ClipperSubsystem = Scene->GetWorld()->GetSubsystem<UZZClipperSubsystem>();
		// UZZClipperSubsystem* ClipperSubsystem = InComponent->GetWorld()->GetSubsystem<UZZClipperSubsystem>();
		// 先不考虑同步问题
		UZZClipperSubsystem* ClipperSubsystem = GEngine->GetEngineSubsystem<UZZClipperSubsystem>();
		
		check(ClipperSubsystem);
		TUniformBufferRef<FZZClippingVolumeParameters> Params =
			ClipperSubsystem->GetClipperRenderData_RenderThread()->CachedZZClipVolumeParams;
				check(Params.IsValid());
				ShaderBindings.Add(Shader->GetUniformBufferParameter<FZZClippingVolumeParameters>(),
					Params);
	}

private:
	LAYOUT_FIELD(FShaderResourceParameter, ZZClipingVolumesSB);
	LAYOUT_FIELD(FShaderParameter, ZZClippingVolumeNum);
};
IMPLEMENT_TYPE_LAYOUT(FZZLocalVertexFactoryShaderParameters);

class FZZVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
    DECLARE_TYPE_LAYOUT(FZZVertexFactoryShaderParameters, NonVirtual);
public:
    void Bind(const FShaderParameterMap& ParameterMap)
    {
        ZZClipingVolumesSB.Bind(ParameterMap, TEXT("ZZClipingVolumesSB"));
    }
    void GetElementShaderBindings(
        const FSceneInterface* Scene,
        const FSceneView* View,
        const FMeshMaterialShader* Shader,
        const EVertexInputStreamType InputStreamType,
        ERHIFeatureLevel::Type FeatureLevel,
        const FVertexFactory* VertexFactory,
        const FMeshBatchElement& BatchElement,
        FMeshDrawSingleShaderBindings& ShaderBindings,
        FVertexInputStreamArray& VertexStreams
        ) const
    {
        const FZZStaticMeshUserData* UserData =
            static_cast<const FZZStaticMeshUserData*>(BatchElement.UserData);
        if (ZZClipingVolumesSB.IsBound() && UserData->QxClippingVolumeSB)
        {
            ShaderBindings.Add(ZZClipingVolumesSB, UserData->QxClippingVolumeSB);
        }
    }

private:
    LAYOUT_FIELD(FShaderResourceParameter, ZZClipingVolumesSB);
};
IMPLEMENT_TYPE_LAYOUT(FZZVertexFactoryShaderParameters);

// 这个vertex factory 不需要做别的事，只需要绑定一个不同factory parameter就行
struct FZZStaticMeshVertexFactory : public FLocalVertexFactory
{
    DECLARE_VERTEX_FACTORY_TYPE(FZZStaticMeshVertexFactory);
public:
    FZZStaticMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
    : FLocalVertexFactory(InFeatureLevel, "FZZStaticMeshVertexFactory")
    {
    	
    }

	using FDataType = FLocalVertexFactory::FDataType;

	// FRHIShaderResourceView* QxClippingVolumeSB = nullptr;
	// uint32 EffectVolumeNum = 0;
};


// IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZZStaticMeshVertexFactory, SF_Vertex, FLocalVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZZStaticMeshVertexFactory, SF_Vertex, FZZLocalVertexFactoryShaderParameters);
// IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZZStaticMeshVertexFactory, SF_Pixel, FZZVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZZStaticMeshVertexFactory, SF_Pixel, FZZLocalVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_TYPE_EX(FZZStaticMeshVertexFactory,"/QxClipMeshShaders/ZZLocalVertexFactory.ush",true,true,true,true,true,true,true);

class FZZStaticMeshRenderData
{
	friend class FZZStaticMeshSceneProxy;
public:
    FZZStaticMeshRenderData(
        UQxClippedStaticMeshComponent* InComponent,
        ERHIFeatureLevel::Type InFeatureLevel
        )
            :Component(InComponent)
            , FeatureLevel(InFeatureLevel)
			, LODModels(InComponent->GetStaticMesh()->GetRenderData()->LODResources)
    {

        InitVertexFactories();

    	
    	// #TODO 用world subystem 时 ClipperSubsystem 为null
    	// UZZClipperSubsystem* ClipperSubsystem = InComponent->GetWorld()->GetSubsystem<UZZClipperSubsystem>();
    	UZZClipperSubsystem* ClipperSubsystem = GEngine->GetEngineSubsystem<UZZClipperSubsystem>();
    	check(ClipperSubsystem);
    	ENQUEUE_RENDER_COMMAND(ZZInitVolumeData)(
    		[this, ClipperSubsystem](FRHICommandListImmediate& RHICmdList)
    		{
    			Init_RenderThread(RHICmdList, ClipperSubsystem);
    		}
    		);
    }

public:
	void Init_RenderThread(FRHICommandListImmediate& RHICmdList, const UZZClipperSubsystem* InClipperSubsystem )
	{
		// InitClippingVolumeBuffers();
		// 初始化buffer引用，通过subsystem 
		GetClippingVolumeBuffers(InClipperSubsystem);

		VolumeRenderData.EffectVolumeNum = static_cast<uint32>(CVarQxBloomPassAmount.GetValueOnRenderThread());
		VolumeRenderData.QxClippingVolumeSB = ZZClippingVolumesSRV;
	}
	
	void ReleaseResources()
	{
		for (int32 LODIndex = 0; LODIndex < VertexFactories.Num(); ++LODIndex)
		{
			VertexFactories[LODIndex].ReleaseResource();
		}
		ZZClippingVolumesSB.SafeRelease();
		ZZClippingVolumesSRV.SafeRelease();
	}

private:
	void InitVertexFactories();

	void InitClippingVolumeBuffers();

	void GetClippingVolumeBuffers(const UZZClipperSubsystem* InClipperSubsystem);
private:
    UQxClippedStaticMeshComponent* Component;

    ERHIFeatureLevel::Type FeatureLevel;

    FStaticMeshLODResourcesArray& LODModels;


    // 每一个LOD有一个vertexfactory
    TIndirectArray<FZZStaticMeshVertexFactory> VertexFactories;

	// FZZStaticMeshRenderData ZZRenderData;
	// FZZStaticMeshVertexFactory ZZVertexFactory;

#pragma region ClppingDataAndBuffers
	// 这两个buffer 理论上来说可以引用这个proxy外部的，但现在先每个都创建
	FStructuredBufferRHIRef ZZClippingVolumesSB;

	FShaderResourceViewRHIRef ZZClippingVolumesSRV;
#pragma endregion

	// 需要传递到mesh batch 的user data的数据
	FZZStaticMeshUserData VolumeRenderData;
};

class FZZStaticMeshSceneProxy : public FStaticMeshSceneProxy
{
public:
    FZZStaticMeshSceneProxy(UQxClippedStaticMeshComponent* InComponent,
        bool bFoolLODsShareStaticLighting) :
        FStaticMeshSceneProxy(InComponent, bFoolLODsShareStaticLighting)
        , ComponentPtr(InComponent)
        , StaticMesh(InComponent->GetStaticMesh())
		, ZZRenderData(InComponent, GetScene().GetFeatureLevel())
    {
    	// Init RenderDatas
    	{
    		
    	}
    };

    // void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;

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

    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
    {
        FPrimitiveViewRelevance Relevance = FStaticMeshSceneProxy::GetViewRelevance(View);
        Relevance.bDynamicRelevance = false; //先只考虑dynamic路径
    	Relevance.bStaticRelevance = true;
        return  Relevance;
    }

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;
	
    virtual  void GetDynamicMeshElements(
        const TArray<const FSceneView*>& Views,
        const FSceneViewFamily& ViewFamily,
        uint32 VisibilityMap,
        FMeshElementCollector& Collector) const override;

	bool GetMeshElementZZ(int32 LODIndex, int32 BatchIndex,
		int32 ElementIndex, uint8 InDepthPriorityGroup,
		bool bUseSelectionOutline, bool bAllowPreCulledIndices,
		FMeshBatch& OutMeshBatch, FMeshElementCollector& Collector) const;

	/** Sets up a FMeshBatch for a specific LOD and element. */
	bool GetMeshElementZZ(
		int32 LODIndex, 
		int32 BatchIndex, 
		int32 SectionIndex, 
		uint8 InDepthPriorityGroup, 
		bool bUseSelectionOutline,
		bool bAllowPreCulledIndices, 
		FMeshBatch& OutMeshBatch) const;

	virtual void DestroyRenderThreadResources() override;
private:
    UPROPERTY()
    const UStaticMeshComponent* ComponentPtr;

    UStaticMesh* StaticMesh;

	FZZStaticMeshRenderData ZZRenderData;

	
};

class FZZStaticMeshOneFrameResource : public FOneFrameResource
{
public:
	FZZStaticMeshUserData Payload;
	~FZZStaticMeshOneFrameResource() {  }
};


/** Sets up a FMeshBatch for a specific LOD and element. */
bool FZZStaticMeshSceneProxy::GetMeshElementZZ(
	int32 LODIndex, 
	int32 BatchIndex, 
	int32 SectionIndex, 
	uint8 InDepthPriorityGroup, 
	bool bUseSelectionOutline,
	bool bAllowPreCulledIndices, 
	FMeshBatch& OutMeshBatch,
	FMeshElementCollector& Collector) const
{
	const ERHIFeatureLevel::Type FeatureLevel = GetScene().GetFeatureLevel();
	const FStaticMeshLODResources& LOD = RenderData->LODResources[LODIndex];
	const FStaticMeshVertexFactories& VFs = RenderData->LODVertexFactories[LODIndex];
	const FStaticMeshSection& Section = LOD.Sections[SectionIndex];
	const FLODInfo& ProxyLODInfo = LODs[LODIndex];

	UMaterialInterface* MaterialInterface = ProxyLODInfo.Sections[SectionIndex].Material;
	const FMaterialRenderProxy* MaterialRenderProxy = MaterialInterface->GetRenderProxy();
	const FMaterial& Material = MaterialRenderProxy->GetIncompleteMaterialWithFallback(FeatureLevel);

	const FVertexFactory* VertexFactory = nullptr;

	FMeshBatchElement& OutMeshBatchElement = OutMeshBatch.Elements[0];

#if WITH_EDITORONLY_DATA
	// If material is hidden, then skip the draw.
	if ((MaterialIndexPreview >= 0) && (MaterialIndexPreview != Section.MaterialIndex))
	{
		return false;
	}
	// If section is hidden, then skip the draw.
	if ((SectionIndexPreview >= 0) && (SectionIndexPreview != SectionIndex))
	{
		return false;
	}

	OutMeshBatch.bUseSelectionOutline = bPerSectionSelection ? bUseSelectionOutline : true;
#endif

	// Has the mesh component overridden the vertex color stream for this mesh LOD?
	if (ProxyLODInfo.OverrideColorVertexBuffer)
	{
		// Make sure the indices are accessing data within the vertex buffer's
		check(Section.MaxVertexIndex < ProxyLODInfo.OverrideColorVertexBuffer->GetNumVertices())

		// Use the instanced colors vertex factory.
		VertexFactory = &VFs.VertexFactoryOverrideColorVertexBuffer;

		OutMeshBatchElement.VertexFactoryUserData = ProxyLODInfo.OverrideColorVFUniformBuffer.GetReference();
		OutMeshBatchElement.UserData = ProxyLODInfo.OverrideColorVertexBuffer;
		OutMeshBatchElement.bUserDataIsColorVertexBuffer = true;
	}
	else
	{
		VertexFactory = &VFs.VertexFactory;
		// Test
		VertexFactory = &ZZRenderData.VertexFactories[LODIndex];
		OutMeshBatchElement.VertexFactoryUserData = VFs.VertexFactory.GetUniformBuffer();
	}

	const bool bWireframe = false;

	// Disable adjacency information when the selection outline is enabled, since tessellation won't be used.
	const bool bRequiresAdjacencyInformation = !bUseSelectionOutline && RequiresAdjacencyInformation(MaterialInterface, VertexFactory->GetType(), FeatureLevel);

	// Two sided material use bIsFrontFace which is wrong with Reversed Indices. AdjacencyInformation use another index buffer.
	const bool bUseReversedIndices = GUseReversedIndexBuffer && IsLocalToWorldDeterminantNegative() && (LOD.bHasReversedIndices != 0) && !bRequiresAdjacencyInformation && !Material.IsTwoSided();

	// No support for stateless dithered LOD transitions for movable meshes
	const bool bDitheredLODTransition = !IsMovable() && Material.IsDitheredLODTransition();

	const uint32 NumPrimitives = SetMeshElementGeometrySource(LODIndex, SectionIndex, bWireframe, bRequiresAdjacencyInformation, bUseReversedIndices, bAllowPreCulledIndices, VertexFactory, OutMeshBatch);

	if(NumPrimitives > 0)
	{
		OutMeshBatch.SegmentIndex = SectionIndex;

		OutMeshBatch.LODIndex = LODIndex;
#if STATICMESH_ENABLE_DEBUG_RENDERING
		OutMeshBatch.VisualizeLODIndex = LODIndex;
		OutMeshBatch.VisualizeHLODIndex = HierarchicalLODIndex;
#endif
		OutMeshBatch.ReverseCulling = IsReversedCullingNeeded(bUseReversedIndices);
		OutMeshBatch.CastShadow = bCastShadow && Section.bCastShadow;
#if RHI_RAYTRACING
		OutMeshBatch.CastRayTracedShadow = OutMeshBatch.CastShadow && bCastDynamicShadow;
#endif
		OutMeshBatch.DepthPriorityGroup = (ESceneDepthPriorityGroup)InDepthPriorityGroup;
		OutMeshBatch.LCI = &ProxyLODInfo;
		OutMeshBatch.MaterialRenderProxy = MaterialRenderProxy;

		OutMeshBatchElement.MinVertexIndex = Section.MinVertexIndex;
		OutMeshBatchElement.MaxVertexIndex = Section.MaxVertexIndex;
#if STATICMESH_ENABLE_DEBUG_RENDERING
		OutMeshBatchElement.VisualizeElementIndex = SectionIndex;
#endif

		SetMeshElementScreenSize(LODIndex, bDitheredLODTransition, OutMeshBatch);

		// Test
		FZZStaticMeshUserData& UserData =
			Collector.AllocateOneFrameResource<FZZStaticMeshOneFrameResource>().Payload;
		UserData.EffectVolumeNum = 2;
		UserData.QxClippingVolumeSB = ZZRenderData.ZZClippingVolumesSRV;
		OutMeshBatchElement.UserData = &UserData;
		
		return true;
	}
	else
	{
		return false;
	}
}

bool FZZStaticMeshSceneProxy::GetMeshElementZZ(int32 LODIndex, int32 BatchIndex, int32 SectionIndex,
	uint8 InDepthPriorityGroup, bool bUseSelectionOutline, bool bAllowPreCulledIndices, FMeshBatch& OutMeshBatch) const
{
	const ERHIFeatureLevel::Type FeatureLevel = GetScene().GetFeatureLevel();
	const FStaticMeshLODResources& LOD = RenderData->LODResources[LODIndex];
	const FStaticMeshVertexFactories& VFs = RenderData->LODVertexFactories[LODIndex];
	const FStaticMeshSection& Section = LOD.Sections[SectionIndex];
	const FLODInfo& ProxyLODInfo = LODs[LODIndex];

	UMaterialInterface* MaterialInterface = ProxyLODInfo.Sections[SectionIndex].Material;
	const FMaterialRenderProxy* MaterialRenderProxy = MaterialInterface->GetRenderProxy();
	const FMaterial& Material = MaterialRenderProxy->GetIncompleteMaterialWithFallback(FeatureLevel);

	const FVertexFactory* VertexFactory = nullptr;

	FMeshBatchElement& OutMeshBatchElement = OutMeshBatch.Elements[0];

#if WITH_EDITORONLY_DATA
	// If material is hidden, then skip the draw.
	if ((MaterialIndexPreview >= 0) && (MaterialIndexPreview != Section.MaterialIndex))
	{
		return false;
	}
	// If section is hidden, then skip the draw.
	if ((SectionIndexPreview >= 0) && (SectionIndexPreview != SectionIndex))
	{
		return false;
	}

	OutMeshBatch.bUseSelectionOutline = bPerSectionSelection ? bUseSelectionOutline : true;
#endif

	// Has the mesh component overridden the vertex color stream for this mesh LOD?
	if (ProxyLODInfo.OverrideColorVertexBuffer)
	{
		// Make sure the indices are accessing data within the vertex buffer's
		check(Section.MaxVertexIndex < ProxyLODInfo.OverrideColorVertexBuffer->GetNumVertices())

		// Use the instanced colors vertex factory.
		VertexFactory = &VFs.VertexFactoryOverrideColorVertexBuffer;

		OutMeshBatchElement.VertexFactoryUserData = ProxyLODInfo.OverrideColorVFUniformBuffer.GetReference();
		OutMeshBatchElement.UserData = ProxyLODInfo.OverrideColorVertexBuffer;
		OutMeshBatchElement.bUserDataIsColorVertexBuffer = true;
	}
	else
	{
		VertexFactory = &VFs.VertexFactory;

		// Test
		VertexFactory = &ZZRenderData.VertexFactories[LODIndex];
		OutMeshBatchElement.VertexFactoryUserData = VFs.VertexFactory.GetUniformBuffer();
	}

	const bool bWireframe = false;

	// Disable adjacency information when the selection outline is enabled, since tessellation won't be used.
	const bool bRequiresAdjacencyInformation = !bUseSelectionOutline && RequiresAdjacencyInformation(MaterialInterface, VertexFactory->GetType(), FeatureLevel);

	// Two sided material use bIsFrontFace which is wrong with Reversed Indices. AdjacencyInformation use another index buffer.
	const bool bUseReversedIndices = GUseReversedIndexBuffer && IsLocalToWorldDeterminantNegative() && (LOD.bHasReversedIndices != 0) && !bRequiresAdjacencyInformation && !Material.IsTwoSided();

	// No support for stateless dithered LOD transitions for movable meshes
	const bool bDitheredLODTransition = !IsMovable() && Material.IsDitheredLODTransition();

	const uint32 NumPrimitives = SetMeshElementGeometrySource(LODIndex, SectionIndex, bWireframe, bRequiresAdjacencyInformation, bUseReversedIndices, bAllowPreCulledIndices, VertexFactory, OutMeshBatch);

	if(NumPrimitives > 0)
	{
		OutMeshBatch.SegmentIndex = SectionIndex;

		OutMeshBatch.LODIndex = LODIndex;
#if STATICMESH_ENABLE_DEBUG_RENDERING
		OutMeshBatch.VisualizeLODIndex = LODIndex;
		OutMeshBatch.VisualizeHLODIndex = HierarchicalLODIndex;
#endif
		OutMeshBatch.ReverseCulling = IsReversedCullingNeeded(bUseReversedIndices);
		OutMeshBatch.CastShadow = bCastShadow && Section.bCastShadow;
#if RHI_RAYTRACING
		OutMeshBatch.CastRayTracedShadow = OutMeshBatch.CastShadow && bCastDynamicShadow;
#endif
		OutMeshBatch.DepthPriorityGroup = (ESceneDepthPriorityGroup)InDepthPriorityGroup;
		OutMeshBatch.LCI = &ProxyLODInfo;
		OutMeshBatch.MaterialRenderProxy = MaterialRenderProxy;

		OutMeshBatchElement.MinVertexIndex = Section.MinVertexIndex;
		OutMeshBatchElement.MaxVertexIndex = Section.MaxVertexIndex;
#if STATICMESH_ENABLE_DEBUG_RENDERING
		OutMeshBatchElement.VisualizeElementIndex = SectionIndex;
#endif

		SetMeshElementScreenSize(LODIndex, bDitheredLODTransition, OutMeshBatch);

		// Test
		// FZZStaticMeshUserData& UserData = VolumeRenderData;
		// UserData.EffectVolumeNum = 2;
		// UserData.QxClippingVolumeSB = ZZRenderData.ZZClippingVolumesSRV;
		OutMeshBatchElement.UserData = &ZZRenderData.VolumeRenderData;
		
		return true;
	}
	else
	{
		return false;
	}
}

void FZZStaticMeshSceneProxy::DestroyRenderThreadResources()
{
	ZZRenderData.ReleaseResources();
	FStaticMeshSceneProxy::DestroyRenderThreadResources();
}

void FZZStaticMeshRenderData::InitVertexFactories()
{
	// Allocate the vertex factories for each LOD
	for (int32 LODIndex = 0; LODIndex < LODModels.Num(); LODIndex++)
	{
		VertexFactories.Add(new FZZStaticMeshVertexFactory(FeatureLevel));
	}

	const int32 LightMapCoordinateIndex = Component->GetStaticMesh()->GetLightMapCoordinateIndex();
	ENQUEUE_RENDER_COMMAND(InstancedStaticMeshRenderData_InitVertexFactories)(
		[this, LightMapCoordinateIndex](FRHICommandListImmediate& RHICmdList)
	{
		for (int32 LODIndex = 0; LODIndex < VertexFactories.Num(); LODIndex++)
		{
			const FStaticMeshLODResources* RenderData = &LODModels[LODIndex];

			FZZStaticMeshVertexFactory::FDataType Data;
			// Assign to the vertex factory for this LOD.
			FZZStaticMeshVertexFactory& VertexFactory = VertexFactories[LODIndex];

			RenderData->VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(&VertexFactory, Data);
			RenderData->VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(&VertexFactory, Data);
			RenderData->VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(&VertexFactory, Data);
			if (LightMapCoordinateIndex < (int32)RenderData->VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords() && LightMapCoordinateIndex >= 0)
			{
				RenderData->VertexBuffers.StaticMeshVertexBuffer.BindLightMapVertexBuffer(&VertexFactory, Data, LightMapCoordinateIndex);
			}
			RenderData->VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(&VertexFactory, Data);

			// check(PerInstanceRenderData);
			// PerInstanceRenderData->InstanceBuffer.BindInstanceVertexBuffer(&VertexFactory, Data);

			VertexFactory.SetData(Data);
			VertexFactory.InitResource();
		}
	});
}

void FZZStaticMeshRenderData::InitClippingVolumeBuffers()
{
	// 先用测试数据
	if (ZZClippingVolumesSB.IsValid())
	{
		ZZClippingVolumesSB.SafeRelease();
	}
	if (ZZClippingVolumesSRV.IsValid())
	{
		ZZClippingVolumesSRV.SafeRelease();
	}

	TArray<FMatrix> TestData;
	{
		TestData.AddUninitialized(100);
		for (int32 i = 0; i < 100; ++i)
		{
			FMatrix TestM = FMatrix(
				FPlane(FVector::OneVector * i, 0),
				FPlane(FVector::ForwardVector, FLT_MAX),
				FPlane(FVector::RightVector, FLT_MAX),
				FPlane(FVector::UpVector, FLT_MAX)
				);
			TestData[i] = TestM;
		}
	}
	
	// TResourceArray 是渲染资源的array，一般情况和tarray相同，UMA的情况下不同
	TResourceArray<FMatrix>* ResourceArray = new TResourceArray<FMatrix>(true);
	// ResourceArray->Reserve(RenderData->ClippingVolumes.Num());
	ResourceArray->Append(TestData);
			
	// 预期回先用compute shader更新这个buffer，再渲染
	FRHIResourceCreateInfo ResourceCI;
			
	ResourceCI.ResourceArray = ResourceArray;
	ResourceCI.DebugName = TEXT("QxClippingVolumesSB");
			
	ZZClippingVolumesSB = RHICreateStructuredBuffer(
		sizeof(FMatrix),
		sizeof(FMatrix) * TestData.Num(),
		BUF_ShaderResource | BUF_Dynamic,
		ResourceCI
		);

	ZZClippingVolumesSRV = RHICreateShaderResourceView(
		ZZClippingVolumesSB
		);
}

void FZZStaticMeshRenderData::GetClippingVolumeBuffers(const UZZClipperSubsystem* InClipperSubsystem)
{
	check(IsInRenderingThread());
	check(InClipperSubsystem);

	FZZCliperRenderData* ClipperRenderData = InClipperSubsystem->GetClipperRenderData_RenderThread();
	check(ClipperRenderData);

	check(ClipperRenderData->ClippingVolumesSB.IsValid());
	check(ClipperRenderData->ClippingVolumesSRV.IsValid());

	ZZClippingVolumesSB = ClipperRenderData->ClippingVolumesSB;
	ZZClippingVolumesSRV = ClipperRenderData->ClippingVolumesSRV;
}

void FZZStaticMeshSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
{
	checkSlow(IsInParallelRenderingThread());
	if (!HasViewDependentDPG())
	{
		// Determine the DPG the primitive should be drawn in.
		uint8 PrimitiveDPG = GetStaticDepthPriorityGroup();
		int32 NumLODs = RenderData->LODResources.Num();
		//Never use the dynamic path in this path, because only unselected elements will use DrawStaticElements
		bool bIsMeshElementSelected = false;
		const auto FeatureLevel = GetScene().GetFeatureLevel();
		const bool IsMobile = IsMobilePlatform(GetScene().GetShaderPlatform());
		const int32 NumRuntimeVirtualTextureTypes = RuntimeVirtualTextureMaterialTypes.Num();

		//check if a LOD is being forced
		if (ForcedLodModel > 0) 
		{
			int32 LODIndex = FMath::Clamp(ForcedLodModel, ClampedMinLOD + 1, NumLODs) - 1;
			const FStaticMeshLODResources& LODModel = RenderData->LODResources[LODIndex];
			// Draw the static mesh elements.
			for(int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
			{
#if WITH_EDITOR
				if( GIsEditor )
				{
					const FLODInfo::FSectionInfo& Section = LODs[LODIndex].Sections[SectionIndex];

					bIsMeshElementSelected = Section.bSelected;
					PDI->SetHitProxy(Section.HitProxy);
				}
#endif // WITH_EDITOR

				const int32 NumBatches = GetNumMeshBatches();
				PDI->ReserveMemoryForMeshes(NumBatches * (1 + NumRuntimeVirtualTextureTypes));

				for (int32 BatchIndex = 0; BatchIndex < NumBatches; BatchIndex++)
				{
					FMeshBatch BaseMeshBatch;

					if (GetMeshElementZZ(LODIndex, BatchIndex, SectionIndex, PrimitiveDPG, bIsMeshElementSelected, true, BaseMeshBatch))
					{
						if (NumRuntimeVirtualTextureTypes > 0)
						{
							// Runtime virtual texture mesh elements.
							FMeshBatch MeshBatch(BaseMeshBatch);
							// SetupMeshBatchForRuntimeVirtualTexture(MeshBatch);
							for (ERuntimeVirtualTextureMaterialType MaterialType : RuntimeVirtualTextureMaterialTypes)
							{
								MeshBatch.RuntimeVirtualTextureMaterialType = (uint32)MaterialType;
								PDI->DrawMesh(MeshBatch, FLT_MAX);
							}
						}
						{
							PDI->DrawMesh(BaseMeshBatch, FLT_MAX);
						}
					}
				}
			}
		} 
		else //no LOD is being forced, submit them all with appropriate cull distances
		{
			for(int32 LODIndex = ClampedMinLOD; LODIndex < NumLODs; LODIndex++)
			{
				const FStaticMeshLODResources& LODModel = RenderData->LODResources[LODIndex];
				float ScreenSize = GetScreenSize(LODIndex);

				bool bUseUnifiedMeshForShadow = false;
				bool bUseUnifiedMeshForDepth = false;

				// 先不处理shadow
				/*
				if (GUseShadowIndexBuffer && LODModel.bHasDepthOnlyIndices)
				{
					const FLODInfo& ProxyLODInfo = LODs[LODIndex];

					// The shadow-only mesh can be used only if all elements cast shadows and use opaque materials with no vertex modification.
					// In some cases (e.g. LPV) we don't want the optimization
					bool bSafeToUseUnifiedMesh = AllowShadowOnlyMesh(FeatureLevel);

					bool bAnySectionUsesDitheredLODTransition = false;
					bool bAllSectionsUseDitheredLODTransition = true;
					bool bIsMovable = IsMovable();
					bool bAllSectionsCastShadow = bCastShadow;

					for (int32 SectionIndex = 0; bSafeToUseUnifiedMesh && SectionIndex < LODModel.Sections.Num(); SectionIndex++)
					{
						const FMaterial& Material = ProxyLODInfo.Sections[SectionIndex].Material->GetRenderProxy()->GetIncompleteMaterialWithFallback(FeatureLevel);
						// no support for stateless dithered LOD transitions for movable meshes
						bAnySectionUsesDitheredLODTransition = bAnySectionUsesDitheredLODTransition || (!bIsMovable && Material.IsDitheredLODTransition());
						bAllSectionsUseDitheredLODTransition = bAllSectionsUseDitheredLODTransition && (!bIsMovable && Material.IsDitheredLODTransition());
						const FStaticMeshSection& Section = LODModel.Sections[SectionIndex];

						bSafeToUseUnifiedMesh =
							!(bAnySectionUsesDitheredLODTransition && !bAllSectionsUseDitheredLODTransition) // can't use a single section if they are not homogeneous
							&& Material.WritesEveryPixel()
							&& !Material.IsTwoSided()
							&& !IsTranslucentBlendMode(Material.GetBlendMode())
							&& !Material.MaterialModifiesMeshPosition_RenderThread()
							&& Material.GetMaterialDomain() == MD_Surface
							&& !Material.IsSky()
							&& !Material.GetShadingModels().HasShadingModel(MSM_SingleLayerWater);

						bAllSectionsCastShadow &= Section.bCastShadow;
					}

					if (bSafeToUseUnifiedMesh)
					{
						bUseUnifiedMeshForShadow = bAllSectionsCastShadow;

						// Depth pass is only used for deferred renderer. The other conditions are meant to match the logic in FDepthPassMeshProcessor::AddMeshBatch.
						bUseUnifiedMeshForDepth = ShouldUseAsOccluder() && GetScene().GetShadingPath() == EShadingPath::Deferred && !IsMovable();

						if (bUseUnifiedMeshForShadow || bUseUnifiedMeshForDepth)
						{
							const int32 NumBatches = GetNumMeshBatches();

							PDI->ReserveMemoryForMeshes(NumBatches);

							for (int32 BatchIndex = 0; BatchIndex < NumBatches; BatchIndex++)
							{
								FMeshBatch MeshBatch;

								if (GetShadowMeshElement(LODIndex, BatchIndex, PrimitiveDPG, MeshBatch, bAllSectionsUseDitheredLODTransition))
								{
									bUseUnifiedMeshForShadow = bAllSectionsCastShadow;

									MeshBatch.CastShadow = bUseUnifiedMeshForShadow;
									MeshBatch.bUseForDepthPass = bUseUnifiedMeshForDepth;
									MeshBatch.bUseAsOccluder = bUseUnifiedMeshForDepth;
									MeshBatch.bUseForMaterial = false;

									PDI->DrawMesh(MeshBatch, ScreenSize);
								}
							}
						}
					}
				} **/

				// Draw the static mesh elements.
				for(int32 SectionIndex = 0;SectionIndex < LODModel.Sections.Num();SectionIndex++)
				{
#if WITH_EDITOR
					if( GIsEditor )
					{
						const FLODInfo::FSectionInfo& Section = LODs[LODIndex].Sections[SectionIndex];

						bIsMeshElementSelected = Section.bSelected;
						PDI->SetHitProxy(Section.HitProxy);
					}
#endif // WITH_EDITOR

					const int32 NumBatches = GetNumMeshBatches();
					PDI->ReserveMemoryForMeshes(NumBatches * (1 + NumRuntimeVirtualTextureTypes));

					for (int32 BatchIndex = 0; BatchIndex < NumBatches; BatchIndex++)
					{
						FMeshBatch BaseMeshBatch;
						if (GetMeshElementZZ(LODIndex, BatchIndex, SectionIndex, PrimitiveDPG, bIsMeshElementSelected, true, BaseMeshBatch))
						{
							if (NumRuntimeVirtualTextureTypes > 0)
							{
								// Runtime virtual texture mesh elements.
								FMeshBatch MeshBatch(BaseMeshBatch);

								for (ERuntimeVirtualTextureMaterialType MaterialType : RuntimeVirtualTextureMaterialTypes)
								{
									MeshBatch.RuntimeVirtualTextureMaterialType = (uint32)MaterialType;
									PDI->DrawMesh(MeshBatch, ScreenSize);
								}
							}

							{
								// Standard mesh elements.
								// If we have submitted an optimized shadow-only mesh, remaining mesh elements must not cast shadows.
								FMeshBatch MeshBatch(BaseMeshBatch);
								MeshBatch.CastShadow &= !bUseUnifiedMeshForShadow;
								MeshBatch.bUseAsOccluder &= !bUseUnifiedMeshForDepth;
								MeshBatch.bUseForDepthPass &= !bUseUnifiedMeshForDepth;
								PDI->DrawMesh(MeshBatch, ScreenSize);
							}
						}
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ZZClipStaticMesh Drawing"));
}



void FZZStaticMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_StaticMeshSceneProxy_GetMeshElements);
	checkSlow(IsInRenderingThread());

	const bool bIsLightmapSettingError = HasStaticLighting() && !HasValidSettingsForStaticLighting();
	const bool bProxyIsSelected = IsSelected();
	const FEngineShowFlags& EngineShowFlags = ViewFamily.EngineShowFlags;

	bool bDrawSimpleCollision = false, bDrawComplexCollision = false;
	const bool bInCollisionView = IsCollisionView(EngineShowFlags, bDrawSimpleCollision, bDrawComplexCollision);
	
	// Skip drawing mesh normally if in a collision view, will rely on collision drawing code below
	// const bool bDrawMesh = !bInCollisionView && 
	// (	IsRichView(ViewFamily) || HasViewDependentDPG()
	// 	|| EngineShowFlags.Collision
	// 	|| EngineShowFlags.Bounds
	// 	|| bProxyIsSelected 
	// 	|| IsHovered()
	// 	|| bIsLightmapSettingError);

	bool Test1 = IsRichView(ViewFamily);
	bool Test2 = HasViewDependentDPG();
	bool Test3 = IsHovered();

	// 对于我自定义的mesh 我希望其显示
	const bool bDrawMesh = true;
	
	// Draw polygon mesh if we are either not in a collision view, or are drawing it as collision.
	if (EngineShowFlags.StaticMeshes && bDrawMesh)
	{
		// how we should draw the collision for this mesh.
		const bool bIsWireframeView = EngineShowFlags.Wireframe;
		const bool bLevelColorationEnabled = EngineShowFlags.LevelColoration;
		const bool bPropertyColorationEnabled = EngineShowFlags.PropertyColoration;
		const ERHIFeatureLevel::Type FeatureLevel = ViewFamily.GetFeatureLevel();

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			const FSceneView* View = Views[ViewIndex];

			if (IsShown(View) && (VisibilityMap & (1 << ViewIndex)))
			{
				FFrozenSceneViewMatricesGuard FrozenMatricesGuard(*const_cast<FSceneView*>(Views[ViewIndex]));

				FLODMask LODMask = GetLODMask(View);

				for (int32 LODIndex = 0; LODIndex < RenderData->LODResources.Num(); LODIndex++)
				{
					if (LODMask.ContainsLOD(LODIndex) && LODIndex >= ClampedMinLOD)
					{
						const FStaticMeshLODResources& LODModel = RenderData->LODResources[LODIndex];
						const FLODInfo& ProxyLODInfo = LODs[LODIndex];

						if (AllowDebugViewmodes() && bIsWireframeView && !EngineShowFlags.Materials
							// If any of the materials are mesh-modifying, we can't use the single merged mesh element of GetWireframeMeshElement()
							&& !ProxyLODInfo.UsesMeshModifyingMaterials())
						{
							FLinearColor ViewWireframeColor( bLevelColorationEnabled ? GetLevelColor() : GetWireframeColor() );
							if ( bPropertyColorationEnabled )
							{
								ViewWireframeColor = GetPropertyColor();
							}

							auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
								GEngine->WireframeMaterial->GetRenderProxy(),
								GetSelectionColor(ViewWireframeColor,!(GIsEditor && EngineShowFlags.Selection) || bProxyIsSelected, IsHovered(), false)
								);

							Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

							const int32 NumBatches = GetNumMeshBatches();

							for (int32 BatchIndex = 0; BatchIndex < NumBatches; BatchIndex++)
							{
								//GetWireframeMeshElement will try SetIndexSource at sectionindex 0
								//and GetMeshElement loops over sections, therefore does not have this issue
								if (LODModel.Sections.Num() > 0)
								{
									FMeshBatch& Mesh = Collector.AllocateMesh();

									if (GetWireframeMeshElement(LODIndex, BatchIndex, WireframeMaterialInstance, SDPG_World, true, Mesh))
									{
										// We implemented our own wireframe
										Mesh.bCanApplyViewModeOverrides = false;
										Collector.AddMesh(ViewIndex, Mesh);
										INC_DWORD_STAT_BY(STAT_StaticMeshTriangles, Mesh.GetNumPrimitives());
									}
								}
							}
						}
						else
						{
							const FLinearColor UtilColor( GetLevelColor() );

							// Draw the static mesh sections.
							for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
							{
								const int32 NumBatches = GetNumMeshBatches();

								for (int32 BatchIndex = 0; BatchIndex < NumBatches; BatchIndex++)
								{
									bool bSectionIsSelected = false;
									FMeshBatch& MeshElement = Collector.AllocateMesh();

									#if WITH_EDITOR
									if (GIsEditor)
									{
										const FLODInfo::FSectionInfo& Section = LODs[LODIndex].Sections[SectionIndex];

										bSectionIsSelected = Section.bSelected || (bIsWireframeView && bProxyIsSelected);
										MeshElement.BatchHitProxyId = Section.HitProxy ? Section.HitProxy->Id : FHitProxyId();
									}
									#endif // WITH_EDITOR
								
									if (GetMeshElementZZ(LODIndex, BatchIndex,
										SectionIndex, SDPG_World, bSectionIsSelected,
										true, MeshElement, Collector))
									{
										bool bDebugMaterialRenderProxySet = false;
									#if STATICMESH_ENABLE_DEBUG_RENDERING

										#if WITH_EDITOR								
										if (bProxyIsSelected && EngineShowFlags.PhysicalMaterialMasks && AllowDebugViewmodes())
										{
											// Override the mesh's material with our material that draws the physical material masks
											UMaterial* PhysMatMaskVisualizationMaterial = GEngine->PhysicalMaterialMaskMaterial;
											check(PhysMatMaskVisualizationMaterial);
											
											FMaterialRenderProxy* PhysMatMaskVisualizationMaterialInstance = nullptr;

											const FLODInfo::FSectionInfo& Section = LODs[LODIndex].Sections[SectionIndex];
											
											if (UMaterialInterface* SectionMaterial = Section.Material)
											{
												if (UPhysicalMaterialMask* PhysicalMaterialMask = SectionMaterial->GetPhysicalMaterialMask())
												{
													if (PhysicalMaterialMask->MaskTexture)
													{
														PhysMatMaskVisualizationMaterialInstance = new FColoredTexturedMaterialRenderProxy(
															PhysMatMaskVisualizationMaterial->GetRenderProxy(),
															FLinearColor::White, NAME_Color, PhysicalMaterialMask->MaskTexture, NAME_LinearColor);
													}

													Collector.RegisterOneFrameMaterialProxy(PhysMatMaskVisualizationMaterialInstance);
													MeshElement.MaterialRenderProxy = PhysMatMaskVisualizationMaterialInstance;

													bDebugMaterialRenderProxySet = true;
												}
											}
										}

										#endif // WITH_EDITOR

										if (!bDebugMaterialRenderProxySet && bProxyIsSelected && EngineShowFlags.VertexColors && AllowDebugViewmodes())
										{
											// Override the mesh's material with our material that draws the vertex colors
											UMaterial* VertexColorVisualizationMaterial = NULL;
											switch( GVertexColorViewMode )
											{
											case EVertexColorViewMode::Color:
												VertexColorVisualizationMaterial = GEngine->VertexColorViewModeMaterial_ColorOnly;
												break;

											case EVertexColorViewMode::Alpha:
												VertexColorVisualizationMaterial = GEngine->VertexColorViewModeMaterial_AlphaAsColor;
												break;

											case EVertexColorViewMode::Red:
												VertexColorVisualizationMaterial = GEngine->VertexColorViewModeMaterial_RedOnly;
												break;

											case EVertexColorViewMode::Green:
												VertexColorVisualizationMaterial = GEngine->VertexColorViewModeMaterial_GreenOnly;
												break;

											case EVertexColorViewMode::Blue:
												VertexColorVisualizationMaterial = GEngine->VertexColorViewModeMaterial_BlueOnly;
												break;
											}
											check( VertexColorVisualizationMaterial != NULL );

											auto VertexColorVisualizationMaterialInstance = new FColoredMaterialRenderProxy(
												VertexColorVisualizationMaterial->GetRenderProxy(),
												GetSelectionColor( FLinearColor::White, bSectionIsSelected, IsHovered() )
												);

											Collector.RegisterOneFrameMaterialProxy(VertexColorVisualizationMaterialInstance);
											MeshElement.MaterialRenderProxy = VertexColorVisualizationMaterialInstance;

											bDebugMaterialRenderProxySet = true;
										}

									#endif // STATICMESH_ENABLE_DEBUG_RENDERING
									#if WITH_EDITOR
										if (!bDebugMaterialRenderProxySet && bSectionIsSelected)
										{
											// Override the mesh's material with our material that draws the collision color
											MeshElement.MaterialRenderProxy = new FOverrideSelectionColorMaterialRenderProxy(
												GEngine->ShadedLevelColorationUnlitMaterial->GetRenderProxy(),
												GetSelectionColor(GEngine->GetSelectedMaterialColor(), bSectionIsSelected, IsHovered())
											);
										}
									#endif
										if (MeshElement.bDitheredLODTransition && LODMask.IsDithered())
										{

										}
										else
										{
											MeshElement.bDitheredLODTransition = false;
										}
								
										MeshElement.bCanApplyViewModeOverrides = true;
										MeshElement.bUseWireframeSelectionColoring = bSectionIsSelected;

										Collector.AddMesh(ViewIndex, MeshElement);
										INC_DWORD_STAT_BY(STAT_StaticMeshTriangles,MeshElement.GetNumPrimitives());
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
}


// Sets default values for this component's properties
UQxClippedStaticMeshComponent::UQxClippedStaticMeshComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UQxClippedStaticMeshComponent::BeginPlay()
{
    Super::BeginPlay();

    // ...
    
}

void UQxClippedStaticMeshComponent::OnRegister()
{
	Super::OnRegister();
	UZZClipperSubsystem* ClipperSubsystem = GEngine->GetEngineSubsystem<UZZClipperSubsystem>();
	// UZZClipperSubsystem* ClipperSubsystem = GetWorld()->GetSubsystem<UZZClipperSubsystem>();
	check(ClipperSubsystem);
	ClipperDelegateHandle = ClipperSubsystem->OnClippingVolumesUpdate.AddLambda(
		[this]()
		{
			this->MarkRenderStateDirty();
		}
		);
}

void UQxClippedStaticMeshComponent::OnUnregister()
{
	Super::OnUnregister();

	if (ClipperDelegateHandle.IsValid())
	{
		GEngine->GetEngineSubsystem<UZZClipperSubsystem>()->OnClippingVolumesUpdate.Remove(ClipperDelegateHandle);
		// GetWorld()->GetSubsystem<UZZClipperSubsystem>()->OnClippingVolumesUpdate.Remove(ClipperDelegateHandle);
		ClipperDelegateHandle.Reset();
	}
}


// Called every frame
void UQxClippedStaticMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

FPrimitiveSceneProxy* UQxClippedStaticMeshComponent::CreateSceneProxy()
{
    if (GetStaticMesh() == nullptr || GetStaticMesh()->GetRenderData() == nullptr)
    {
        return nullptr;
    }

    const FStaticMeshLODResourcesArray& LODResources = GetStaticMesh()->GetRenderData()->LODResources;
    if (LODResources.Num() == 0 || LODResources[FMath::Clamp<int32>(GetStaticMesh()->GetMinLOD().Default, 0, LODResources.Num() - 1)].VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() == 0)
    {
        return nullptr;
    }
    LLM_SCOPE(ELLMTag::StaticMesh);

    //FPrimitiveSceneProxy* Proxy = ::new FStaticMeshSceneProxy(this, false);
    FPrimitiveSceneProxy* proxy = ::new FZZStaticMeshSceneProxy(this, false);
#if STATICMESH_ENABLE_DEBUG_RENDERING
    SendRenderDebugPhysics(proxy);
#endif
    return proxy;
}

void UQxClippedStaticMeshComponent::GenerateClippingVolumes()
{
    FZZPointCloudClippingVolumeParams TestParam;
    
    TestClippingVolumes.Reset();

    for (int32 i = 0; i < (16); ++i)
    {
        FZZPointCloudClippingVolumeParams tmpParam;

        FVector Extent = FVector::OneVector * 100;
        tmpParam.PackedShaderData = FMatrix(
            FPlane(FVector::ZeroVector, 1),
            FPlane(FVector::ForwardVector, Extent.X),
            FPlane(FVector::RightVector, Extent.Y),
            FPlane(FVector::UpVector, Extent.Z)
            );
        TestClippingVolumes.Add(tmpParam);
    }
}

