// Fill out your copyright notice in the Description page of Project Settings.


#include "QxClippedStaticMeshComponent.h"

#include "MeshDrawShaderBindings.h"
#include "LocalVertexFactory.h"
#include "PhysicalMaterials/PhysicalMaterialMask.h"

class FZZStaticMeshUserData
{
public:
    FRHIShaderResourceView* QxClippingVolumeSB;
};

class FZZVertexFactoryShaderParameters : public FLocalVertexFactoryShaderParametersBase
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
};

IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZZStaticMeshVertexFactory, SF_Vertex, FLocalVertexFactoryShaderParameters);
// IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FZZStaticMeshVertexFactory, SF_Pixel, FZZVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_TYPE_EX(FZZStaticMeshVertexFactory,"/Engine/Private/LocalVertexFactory.ush",true,true,true,true,true,true,true);

// class FZZStaticMeshRenderData
// {
// public:
//     FZZStaticMeshRenderData(
//         UQxClippedStaticMeshComponent* InComponent,
//         ERHIFeatureLevel::Type InFeatureLevel
//         )
//             :Component(InComponent)
//             , FeatureLevel(InFeatureLevel)
//     {
//
//         // Init Vertex factory
//         {
//             
//         }
//     }
//
// private:
//     UQxClippedStaticMeshComponent* Component;
//
//     ERHIFeatureLevel::Type FeatureLevel;
//
//     FStaticMeshLODResources& LODArray;
//
//     // 每一个LOD有一个vertexfactory
//     TIndirectArray<FZZStaticMeshVertexFactory> VertexFactories;
// };

class FZZStaticMeshSceneProxy : public FStaticMeshSceneProxy
{
public:
    FZZStaticMeshSceneProxy(UQxClippedStaticMeshComponent* InComponent,
        bool bFoolLODsShareStaticLighting) :
        FStaticMeshSceneProxy(InComponent, bFoolLODsShareStaticLighting)
        , ComponentPtr(InComponent)
        , StaticMesh(InComponent->GetStaticMesh())
    {
    };

    // void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;

    // 下面是固定写法
    #pragma region StatableParttern
    SIZE_T GetTypeHash() const override
    {
        static size_t UniquePointer;
        return reinterpret_cast<size_t>(&UniquePointer);
    }
    // virtual uint32 GetMemoryFootprint(void) const override
    // {
    //     return sizeof(*this) + GetAllocatedSize();
    // }
    //
    // uint32 GetAllocateSize(void) const
    // {
    //     return FPrimitiveSceneProxy::GetAllocatedSize();
    // }
#pragma endregion

    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
    {
        FPrimitiveViewRelevance Relevance = FStaticMeshSceneProxy::GetViewRelevance(View);
        Relevance.bDynamicRelevance = true; //先只考虑dynamic路径
        return  Relevance;
    }

    virtual  void GetDynamicMeshElements(
        const TArray<const FSceneView*>& Views,
        const FSceneViewFamily& ViewFamily,
        uint32 VisibilityMap,
        FMeshElementCollector& Collector) const override;
private:
    UPROPERTY()
    const UStaticMeshComponent* ComponentPtr;

    UStaticMesh* StaticMesh;

    // FZZStaticMeshRenderData ZZRenderData;

    // 这两个buffer 理论上来说可以引用这个proxy外部的，但现在先每个都创建
    FStructuredBufferRHIRef ZZClippingVolumesSB;

    FShaderResourceViewRHIRef ZZClippingVolumesSRV;
};


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
	const bool bDrawMesh = !bInCollisionView && 
	(	IsRichView(ViewFamily) || HasViewDependentDPG()
		|| EngineShowFlags.Collision
		|| EngineShowFlags.Bounds
		|| bProxyIsSelected 
		|| IsHovered()
		|| bIsLightmapSettingError);

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
								
									if (GetMeshElement(LODIndex, BatchIndex, SectionIndex, SDPG_World, bSectionIsSelected, true, MeshElement))
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

