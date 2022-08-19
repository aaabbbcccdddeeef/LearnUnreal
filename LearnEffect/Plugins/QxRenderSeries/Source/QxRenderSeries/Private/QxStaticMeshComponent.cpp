// Fill out your copyright notice in the Description page of Project Settings.


#include "QxStaticMeshComponent.h"


class FQxStaticMeshSceneProxy : public FStaticMeshSceneProxy
{
public:
	FQxStaticMeshSceneProxy(UQxStaticMeshComponent* Component,
		bool bFoolLODsShareStaticLighting) :FStaticMeshSceneProxy(Component, bFoolLODsShareStaticLighting)
	{
		ComponentPtr = Component;
	};

	void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override;

private:
	UPROPERTY()
	const UStaticMeshComponent* ComponentPtr;
};


void FQxStaticMeshSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
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
			for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
			{
#if WITH_EDITOR
				if (GIsEditor)
				{
					const FLODInfo::FSectionInfo& Section = LODs[LODIndex].Sections[SectionIndex];

					bIsMeshElementSelected = Section.bSelected;
					PDI->SetHitProxy(Section.HitProxy);
				}
#endif // WITH_EDITOR

				const int32 NumBatches = GetNumMeshBatches();
				// 先不考虑virtual texture
				//PDI->ReserveMemoryForMeshes(NumBatches * (1 + NumRuntimeVirtualTextureTypes));
				PDI->ReserveMemoryForMeshes(NumBatches);

				for (int32 BatchIndex = 0; BatchIndex < NumBatches; BatchIndex++)
				{
					FMeshBatch BaseMeshBatch;

					if (GetMeshElement(LODIndex, BatchIndex, SectionIndex, PrimitiveDPG, bIsMeshElementSelected, true, BaseMeshBatch))
					{
						PDI->DrawMesh(BaseMeshBatch, FLT_MAX);

						// 下面是自己添加的
						const FLODInfo&  proxyLODInfo =  LODs[LODIndex];
						UMaterialInterface* MaterialInterface = proxyLODInfo.Sections[SectionIndex].Material;

						const UQxStaticMeshComponent* qxStaticMeshComponent = dynamic_cast<const UQxStaticMeshComponent*>(ComponentPtr);
						if (MaterialInterface == qxStaticMeshComponent->SecondPassMaterial
							|| qxStaticMeshComponent->SecondPassMaterial == nullptr)
						{
							continue;;
						}

						if (qxStaticMeshComponent->bNeedSecondPass)
						{
							BaseMeshBatch.MaterialRenderProxy =
								qxStaticMeshComponent->SecondPassMaterial->GetRenderProxy();

							// 设置反转剔除
							BaseMeshBatch.ReverseCulling = true;
							PDI->DrawMesh(BaseMeshBatch, FLT_MAX);
						}
					}
				}
			}
		}
		else //no LOD is being forced, submit them all with appropriate cull distances
		{
			for (int32 LODIndex = ClampedMinLOD; LODIndex < NumLODs; LODIndex++)
			{
				const FStaticMeshLODResources& LODModel = RenderData->LODResources[LODIndex];
				float ScreenSize = GetScreenSize(LODIndex);

				bool bUseUnifiedMeshForShadow = false;
				bool bUseUnifiedMeshForDepth = false;

				// 先不考虑 shadow 的问题
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
				}

				*/


				// Draw the static mesh elements.
				for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
				{
#if WITH_EDITOR
					if (GIsEditor)
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
						if (GetMeshElement(LODIndex, BatchIndex, SectionIndex, PrimitiveDPG, bIsMeshElementSelected, true, BaseMeshBatch))
						{

							{
								// Standard mesh elements.
								// If we have submitted an optimized shadow-only mesh, remaining mesh elements must not cast shadows.
								FMeshBatch MeshBatch(BaseMeshBatch);
								MeshBatch.CastShadow &= !bUseUnifiedMeshForShadow;
								MeshBatch.bUseAsOccluder &= !bUseUnifiedMeshForDepth;
								MeshBatch.bUseForDepthPass &= !bUseUnifiedMeshForDepth;
								PDI->DrawMesh(MeshBatch, ScreenSize);
							}


							// 下面是自己添加的
							{
								const FLODInfo& proxyLODInfo = LODs[LODIndex];
								UMaterialInterface* MaterialInterface = proxyLODInfo.Sections[SectionIndex].Material;

								const UQxStaticMeshComponent* qxStaticMeshComponent = dynamic_cast<const UQxStaticMeshComponent*>(ComponentPtr);
								if (MaterialInterface == qxStaticMeshComponent->SecondPassMaterial
									|| qxStaticMeshComponent->SecondPassMaterial == nullptr)
								{
									continue;;
								}

								if (qxStaticMeshComponent->bNeedSecondPass)
								{
									BaseMeshBatch.MaterialRenderProxy =
										qxStaticMeshComponent->SecondPassMaterial->GetRenderProxy();

									// 设置反转剔除
									BaseMeshBatch.ReverseCulling = true;
									PDI->DrawMesh(BaseMeshBatch, FLT_MAX);
								}
							}
						}
					}
				}
			}
		}
	}
}

void UQxStaticMeshComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials /*= false*/) const
{
	//Super::GetUsedMaterials(OutMaterials, false);

	if (GetStaticMesh() && GetStaticMesh()->RenderData)
	{
		TMap<int32, UMaterialInterface*> materialMap;

		for (int32 LODIndex = 0; LODIndex < GetStaticMesh()->RenderData->LODResources.Num(); LODIndex++)
		{
			FStaticMeshLODResources& lodResources = GetStaticMesh()->RenderData->LODResources[LODIndex];
			int32 materialNum = 0;
			
			for (int32 SectionIndex = 0; SectionIndex < lodResources.Sections.Num(); SectionIndex++)
			{
				// Get the material for each element at the current lod index
				int32 MaterialIndex = lodResources.Sections[SectionIndex].MaterialIndex;
				if (!materialMap.Contains(MaterialIndex))
				{
					materialMap.Add(MaterialIndex, GetMaterial(MaterialIndex));
					materialNum++;
				}
			}

			if (bNeedSecondPass)
			{
				bool needAddMaterial = true;
				for (auto& element: materialMap)
				{
					// 如果已经有了，不用再添加
					if (element.Value == SecondPassMaterial)
					{
						needAddMaterial = false;
					}
				}
				if (needAddMaterial)
				{
					materialMap.Add(materialNum, SecondPassMaterial);
				}
			}

			if (materialMap.Num() > 0)
			{
				//We need to output the material in the correct order (follow the material index)
				//So we sort the map with the material index
				materialMap.KeySort([](int32 A, int32 B) {
					return A < B; // sort keys in order
					});

				//Preadd all the material item in the array
				OutMaterials.AddZeroed(materialMap.Num());
				//Set the value in the correct order
				int32 MaterialIndex = 0;
				for (auto Kvp : materialMap)
				{
					OutMaterials[MaterialIndex++] = Kvp.Value;
				}
			}
		}
	}
}

FPrimitiveSceneProxy* UQxStaticMeshComponent::CreateSceneProxy()
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
	FPrimitiveSceneProxy* proxy = ::new FQxStaticMeshSceneProxy(this, false);
#if STATICMESH_ENABLE_DEBUG_RENDERING
	SendRenderDebugPhysics(proxy);
#endif
	return proxy;
}
