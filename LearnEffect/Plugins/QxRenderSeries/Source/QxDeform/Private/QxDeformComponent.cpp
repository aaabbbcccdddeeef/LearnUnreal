// Fill out your copyright notice in the Description page of Project Settings.


#include "QxDeform/Public/QxDeformComponent.h"

#include <stdexcept>

#include "MeshMaterialShader.h"
#include "PrimitiveSceneProxy.h"
#include "Engine/StaticMesh.h"


/**
 * @brief
 * 这里继承LocalVertexFactory是因为这个类有可以复用的部分，可以尝试从VertexrFactory完全自定义实现
 */
struct FQxDeformMeshVertexFactory : public FLocalVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FQxDeformMeshVertexFactory)
public:
	FQxDeformMeshVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
		: FLocalVertexFactory(InFeatureLevel, "FQxDeformMeshVertexFactory")
	{
		// 这里的manual vertex fetch 是做什么用的 #TODO
		bSupportsManualVertexFetch = false;
	}

	// 这里可以定义不同permutation 是否需要编译，
	// 例如下面自定surface和unit是编译
	// 注意：我们一般还需要对默认材质编译，否则fall back到默认材质时会crash
	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
	{
		if ( (Parameters.MaterialParameters.MaterialDomain == MD_Surface &&
				Parameters.MaterialParameters.ShadingModels == MSM_Unlit)
			|| Parameters.MaterialParameters.bIsDefaultMaterial)
		{
			return  true;
		}
		return false;
	}

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters,
		FShaderCompilerEnvironment& OutEnvironment)
	{
		const bool ContainsManualVertexFetch = OutEnvironment.GetDefinitions().Contains("MANUAL_VERTEX_FETCH");
		if (!ContainsManualVertexFetch)
		{
			OutEnvironment.SetDefine(TEXT("MANUAL_VERTEX_FETCH"), TEXT("0"));
		}

		// 这里我们自定义一个QXDEFORM_MESH的宏
		OutEnvironment.SetDefine(TEXT("QXDEFORM_MESH"), TEXT("1"));
	}

	// 注意： VerterFactory默认定义了3个vertex stream, 3个vertex declaration
	// 这里要初始化这6个成员, 但一般PosNormal可以不处理，需要处理4个
	// 关键实现，关于InitRHI和InitDynamicRHI有些疑问，
	virtual void InitRHI() override
	{
		check(HasValidFeatureLevel());

		FVertexDeclarationElementList Elements;
		FVertexDeclarationElementList PosOnlyElements;

		if (Data.PositionComponent.VertexBuffer != nullptr)
		{
			// AccessStreamComponent是根据传入的vertex stream 构造FVertexElement的方法
			// 注意AccessStreamComponent 有一个side effect即添加vertex stream到当前vertex factory中
			Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
			PosOnlyElements.Add(AccessStreamComponent(Data.PositionComponent, 0, EVertexInputStreamType::PositionOnly));
		}

		// 初始化Pos only 的declaration
		InitDeclaration(PosOnlyElements, EVertexInputStreamType::PositionOnly);

		
		if (Data.TextureCoordinates.Num())
		{
			// 这里为什么不是从0开始, 这里应该是因为是steam index 是4，查看renderdoc
			const int32 BaseTexCoordAttribute = 4; // 这里不太理解，之后用Renderdoc 看一下 #TODO

			for (int32 CoordIndex = 0; CoordIndex < Data.TextureCoordinates.Num(); ++CoordIndex)
			{
				Elements.Add(AccessStreamComponent(
					Data.TextureCoordinates[CoordIndex],
					BaseTexCoordAttribute + CoordIndex
				));
			}

			for (int32 CoordIndex = Data.TextureCoordinates.Num(); CoordIndex < MAX_STATIC_TEXCOORDS / 2; ++CoordIndex)
			{
				Elements.Add(
						AccessStreamComponent(
							Data.TextureCoordinates[Data.TextureCoordinates.Num() - 1],
							BaseTexCoordAttribute + CoordIndex
							)
					);
			}
		}

		check(Streams.Num() > 0);

		// 初始化正常的declaration
		InitDeclaration(Elements);
		check(IsValidRef(GetDeclaration()));
	};

	inline  void SetTransformIndex(uint16 Index)
	{
		TransformIndex = Index;
	}

	inline  void SetSceneProxy(FQxDeformMeshProxy* InQxDeformMeshProxy)
	{
		QxDeformProxy = InQxDeformMeshProxy;
	}
	
private:
	
	uint16 TransformIndex;

	FQxDeformMeshProxy* QxDeformProxy;

	friend class FQxDeformVertexFactoryShaderParameters;
};

/**
 * @brief
 * 渲染一个section需要下面这些成员
 * 1. vertexdata 包括index buffer和verter factory(定义vertex strams和vertex decalrations/layout)
 * 2. material
 * 3. other data: visiblity, maximum vertex index
 */
class FQxDeformMeshSectionProxy
{
public:
	FQxDeformMeshSectionProxy(ERHIFeatureLevel::Type InFeatureLevel)
		: Material(nullptr)
		, VertexFactory(InFeatureLevel)
		, bSectionVisible(true)
	{
	}

public:

	UMaterialInterface* Material;

	FRawStaticIndexBuffer IndexBuffer;

	FQxDeformMeshVertexFactory VertexFactory;

	bool bSectionVisible;

	uint32 MaxVertexIndex;
	
};

// 定义一个初始化RenderRosource的工具方法
static void InitOrUpdateResouce(FRenderResource* InResource)
{
	if (InResource->IsInitialized())
	{
		InResource->UpdateRHI();
	}
	else
	{
		InResource->InitResource();
	}
}

// 定义一个初始化vertex factory data的方法
static void InitVertexFactoryData(FQxDeformMeshVertexFactory* QxVertexFactory, FStaticMeshVertexBuffers* VertexBuffers)
{
	ENQUEUE_RENDER_COMMAND(StaticMeshVertexFactoryInit)(
		[QxVertexFactory, VertexBuffers](FRHICommandListImmediate& RHICmdList)
		{
			InitOrUpdateResouce(&VertexBuffers->PositionVertexBuffer);
			InitOrUpdateResouce(&VertexBuffers->StaticMeshVertexBuffer);

			// 利用vertex buffer构造FDataType中的 vertex stream, 并将其设置给vertex factory
			FLocalVertexFactory::FDataType Data;
			VertexBuffers->PositionVertexBuffer.BindPositionVertexBuffer(QxVertexFactory, Data);
			VertexBuffers->StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(QxVertexFactory, Data);
			QxVertexFactory->SetData(Data);

			InitOrUpdateResouce(QxVertexFactory);
		}
		);
}


/**
 * @brief 封装deform component的渲染部分
 */
class FQxDeformMeshProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override // 这个什么时候调用
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FQxDeformMeshProxy(UQxDeformComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{
		const uint16 NumSections = Component->DeformMeshSections.Num();

		// 用sections 数据初始化section proxy
		DeformTransforms_Proxy.AddZeroed(NumSections);
		Sections.AddZeroed(NumSections);

		for (uint16 SectionIndex = 0; SectionIndex < NumSections; ++SectionIndex)
		{
			const FDeformMeshSection& SrcSection = Component->DeformMeshSections[SectionIndex];
			{
				// 创建section proxy
				FQxDeformMeshSectionProxy* NewSection = new FQxDeformMeshSectionProxy(GetScene().GetFeatureLevel());

				// 从static mesh获取需要的数据
				auto& LODResource = SrcSection.StaticMesh->GetRenderData()->LODResources[0];

				FQxDeformMeshVertexFactory* VertexFactory = &NewSection->VertexFactory;

				InitVertexFactoryData(VertexFactory, &(LODResource.VertexBuffers));

				VertexFactory->SetTransformIndex(SectionIndex);
				VertexFactory->SetSceneProxy(this);

				// 用static mesh 的index buffer初始化section proxy的index buffer;
				{
					TArray<uint32> tmpIndices;
					LODResource.IndexBuffer.GetCopy(tmpIndices);
					NewSection->IndexBuffer.AppendIndices(tmpIndices.GetData(), tmpIndices.Num());

					// 初始化Index buffer
					BeginInitResource(&NewSection->IndexBuffer);
				}

				DeformTransforms_Proxy[SectionIndex] = SrcSection.DeformTransformMat;

				NewSection->MaxVertexIndex = LODResource.VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

				NewSection->Material = Component->GetMaterial(SectionIndex);

				if (NewSection->Material == nullptr)
				{
					NewSection->Material = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
				}

				NewSection->bSectionVisible = SrcSection.bSectionVisible;

				Sections[SectionIndex] = NewSection;
			}
			
		}

		// 创建 deform transform 用的structed buffer相关
		if (NumSections > 0)
		{
			TResourceArray<FMatrix>* ResourceArray = new TResourceArray<FMatrix>(true);
			FRHIResourceCreateInfo RHIResourceCI;
			ResourceArray->Append(DeformTransforms_Proxy);

			RHIResourceCI.ResourceArray = ResourceArray;
			// 这里设置的名字会在Renderdoc 中显示
			RHIResourceCI.DebugName = TEXT("QxDeformMesh_TransformsSB");

			if (DeformTransformsSB.IsValid())
			{
				DeformTransformsSB.SafeRelease();
			}
			if (DeformTransformsSRV.IsValid())
			{
				DeformTransformsSRV.SafeRelease();
			}
			DeformTransformsSB = RHICreateStructuredBuffer(
				sizeof(FMatrix),
				NumSections * sizeof(FMatrix),
				BUF_ShaderResource ,
				RHIResourceCI
				);

			bDeformTransformDirty = false;

			FString test = FString(DeformTransformsSB->GetDebugName());
			// 创建structed buffer的srv以便绑定到vertex factory
			DeformTransformsSRV = RHICreateShaderResourceView(DeformTransformsSB);
		}
	}

	virtual ~FQxDeformMeshProxy()
	{
		for (FQxDeformMeshSectionProxy* SectionProxy : Sections)
		{
			if (SectionProxy)
			{
				SectionProxy->VertexFactory.ReleaseResource();
				SectionProxy->IndexBuffer.ReleaseResource();
				delete SectionProxy;
			}
		}

		// DeformTransformsSB.SafeRelease();
		// DeformTransformsSRV.SafeRelease();
	}
public:
	
	void UpdataDeformTransformSB_RenderThread()
	{
		check(IsInRenderingThread());

		if (bDeformTransformDirty && DeformTransformsSB)
		{
			void* DeformTranformSB_Data = RHILockStructuredBuffer(
				DeformTransformsSB,
				0,
				DeformTransforms_Proxy.Num() * sizeof(FMatrix),
				RLM_WriteOnly
				);
			TArray<FMatrix> tmpArray = DeformTransforms_Proxy;
			FMemory::Memcpy(DeformTranformSB_Data, DeformTransforms_Proxy.GetData(),
				DeformTransforms_Proxy.Num() * sizeof(FMatrix));
			RHIUnlockStructuredBuffer(DeformTransformsSB);
			uint32 test = DeformTransformsSB->GetSize();
			uint32 testStride = DeformTransformsSB->GetStride();
			bDeformTransformDirty = false;
		}
	};

	void UpdateDeformTransform_RenderThread(int32 SectionIndex, FMatrix Transform)
	{
		check(IsInRenderingThread());

		if (SectionIndex < Sections.Num() &&
			Sections[SectionIndex] != nullptr)
		{
			DeformTransforms_Proxy[SectionIndex] = Transform;
			// mark transform diry
			bDeformTransformDirty = true;
		}
	}

	void SetSectionVisiblity_RenderThread(int32 SectionIndex, bool bNewVisiblity)
	{
		if (SectionIndex < Sections.Num() &&
			Sections[SectionIndex] != nullptr)
		{
			Sections[SectionIndex]->bSectionVisible = bNewVisiblity;
		}
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest; //#TODO
	};

	FShaderResourceViewRHIRef& GetDeformTransofrmsSRV()
	{
		return DeformTransformsSRV;
	}

	// 根据传入的views和visiblity map, 输入mesh batch到collector
	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override
	{
		// 设置wire frame material
		const bool bWireFrame = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireFrameMaterialInstance = nullptr;
		if (bWireFrame)
		{
			WireFrameMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
				FLinearColor(0, 0.5f, 1.f)
				);

			// 注册到collector由collector 负责回收
			Collector.RegisterOneFrameMaterialProxy(WireFrameMaterialInstance);
		}

		for (const FQxDeformMeshSectionProxy* SectionProxy : Sections)
		{
			if (SectionProxy && SectionProxy->bSectionVisible)
			{
				FMaterialRenderProxy* CurMaterialProxy = bWireFrame ? WireFrameMaterialInstance :
					SectionProxy->Material->GetRenderProxy();

				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
				{
					// 当前index view 是否可见
					if (VisibilityMap & (1 << ViewIndex))
					{
						const FSceneView* View = Views[ViewIndex];
						// 分配一个mesh batch并得到引用
						FMeshBatch& MeshBatch = Collector.AllocateMesh();
						FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
	
						// 填充这个batch elment 用section render data
						BatchElement.IndexBuffer = &SectionProxy->IndexBuffer;
						MeshBatch.bWireframe = bWireFrame;
						MeshBatch.VertexFactory = &SectionProxy->VertexFactory;
						MeshBatch.MaterialRenderProxy = CurMaterialProxy;

						// 设置local vertex factory中的uniform buffer的参数信息
						bool bHasProcomputedVolumetricLightmap;
						FMatrix PreviousLocalToWorld;
						int32 SingleCaptureIndex;
						bool bOutputVelocity;
						GetScene().GetPrimitiveUniformShaderParameters_RenderThread(
							GetPrimitiveSceneInfo(),
							bHasProcomputedVolumetricLightmap,
							PreviousLocalToWorld,
							SingleCaptureIndex,
							bOutputVelocity
							);

						// 分配一个临时的primitive uniform buffer, fill it with the data and set it in the batch element
						FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer =
							Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
						DynamicPrimitiveUniformBuffer.Set(
							GetLocalToWorld(), PreviousLocalToWorld,
							GetBounds(), GetLocalBounds(),
							true, bHasProcomputedVolumetricLightmap,
							DrawsVelocity(), bOutputVelocity
							);
						BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
						BatchElement.PrimitiveIdMode = PrimID_DynamicPrimitiveShaderData;

						// addtional data
						BatchElement.FirstIndex = 0;
						BatchElement.NumPrimitives = SectionProxy->IndexBuffer.GetNumIndices() /3;
						BatchElement.MinVertexIndex = 0;
						BatchElement.MaxVertexIndex = SectionProxy->MaxVertexIndex;
						MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative(); // 这里不太懂 #TODO
						MeshBatch.Type = PT_TriangleList;
						MeshBatch.DepthPriorityGroup = SDPG_World;
						MeshBatch.bCanApplyViewModeOverrides = false;

						// add the batch to the collector
						Collector.AddMesh(ViewIndex, MeshBatch);
					}
				}
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
		MaterialRelevance.SetPrimitiveViewRelevance(ViewRelevance);

		// 这句为什么在这
		ViewRelevance.bVelocityRelevance = IsMovable() && ViewRelevance.bOpaque && ViewRelevance.bRenderInMainPass;
		return ViewRelevance;
	}
	
	// 这个函数必须覆盖，下面这个是固定写法, 具体什么事件调用还不知道#TODO
	virtual uint32 GetMemoryFootprint() const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	};

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

private:
	FMaterialRelevance MaterialRelevance;

	TArray<FQxDeformMeshSectionProxy*> Sections;

	// 所有section 的deform transform
	TArray<FMatrix> DeformTransforms_Proxy;

	// 这个structed buffer 包括所有section 的deform transform
	FStructuredBufferRHIRef DeformTransformsSB;

	// structed buffer的srv， 将会绑定到vertex factory shader
	FShaderResourceViewRHIRef DeformTransformsSRV;

	bool bDeformTransformDirty;
};



// 自定义vertex shader parameters
class FQxDeformVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
	DECLARE_TYPE_LAYOUT(FQxDeformVertexFactoryShaderParameters, NonVirtual);
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		TransformIndex.Bind(ParameterMap, TEXT("QxDFTransformIndex"), SPF_Optional);
		TransformsSRV.Bind(ParameterMap, TEXT("QxDFTransforms"), SPF_Optional);
	}

	void GetElementShaderBindings(
		const FSceneInterface* Scene,
		const FSceneView* SceneView,
		const FMeshMaterialShader* Shader,
		const EVertexInputStreamType InputStreamType,
		ERHIFeatureLevel::Type FeatureLevel,
		const FVertexFactory* VertexFactory,
		const FMeshBatchElement& BatchElement,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams
		) const
	{
		if (BatchElement.bUserDataIsColorVertexBuffer)
		{
			const auto* LocalVertexFactory = static_cast<const FLocalVertexFactory*>(VertexFactory);
			FColorVertexBuffer* OverrideColorVertexBuffer = (FColorVertexBuffer*)(BatchElement.UserData);

			check(OverrideColorVertexBuffer);

			if (!LocalVertexFactory->SupportsManualVertexFetch(FeatureLevel))
			{
				LocalVertexFactory->GetColorOverrideStream(OverrideColorVertexBuffer, VertexStreams);
			}
		}

		const FQxDeformMeshVertexFactory* DeformMeshVertexFactory = (FQxDeformMeshVertexFactory*)(VertexFactory);

		const int32 Index = DeformMeshVertexFactory->TransformIndex;

		ShaderBindings.Add(TransformIndex, Index);
		ShaderBindings.Add(TransformsSRV, DeformMeshVertexFactory->QxDeformProxy->GetDeformTransofrmsSRV());
	}

private:
	LAYOUT_FIELD(FShaderParameter, TransformIndex);
	LAYOUT_FIELD(FShaderResourceParameter, TransformsSRV);
};

IMPLEMENT_TYPE_LAYOUT(FQxDeformVertexFactoryShaderParameters);


IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FQxDeformMeshVertexFactory, SF_Vertex, FQxDeformVertexFactoryShaderParameters);

IMPLEMENT_VERTEX_FACTORY_TYPE(FQxDeformMeshVertexFactory, "/QxShaders/QxLocalVertexFactory.ush",
	true, true, true, true, true);

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
	if (SectionIndex < DeformMeshSections.Num())
	{
		
		const FMatrix TransformMatrix = Transform.ToMatrixWithScale().GetTransposed();
		DeformMeshSections[SectionIndex].DeformTransformMat = TransformMatrix;

		DeformMeshSections[SectionIndex].SectionLocalBox +=
			DeformMeshSections[SectionIndex].StaticMesh->GetBoundingBox().TransformBy(Transform);

		if (SceneProxy)
		{
			// 注意UE4默认编译没有RTTI，dynamic cast不能用
			// FQxDeformMeshProxy* DeformMeshProxy = dynamic_cast<FQxDeformMeshProxy*>(SceneProxy);
			FQxDeformMeshProxy* DeformMeshProxy = (FQxDeformMeshProxy*)(SceneProxy);
			ENQUEUE_RENDER_COMMAND(FDeformTransformUpdate)(
				[DeformMeshProxy, SectionIndex, TransformMatrix](FRHICommandListImmediate& RHICmdList)
				{
					DeformMeshProxy->UpdateDeformTransform_RenderThread(SectionIndex, TransformMatrix);
				}
				);
		}

		UpdateLocalBounds();
		MarkRenderTransformDirty();
	}

}

void UQxDeformComponent::FinishTransformsUpdate()
{
	if (SceneProxy)
	{
		FQxDeformMeshProxy* DeformMeshProxy = (FQxDeformMeshProxy*)(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FDeformMeshAllTransformSBData)(
			[DeformMeshProxy](FRHICommandListImmediate& RHICmdList)
			{
				DeformMeshProxy->UpdataDeformTransformSB_RenderThread();
			}
			);
	}
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
	NewSection.DeformTransformMat = Transform.ToMatrixNoScale().GetTransposed(); // #TODO 这里为什么转置

	// 用static mesh来计算section local box
	NewSection.StaticMesh->CalculateExtendedBounds();
	NewSection.SectionLocalBox += NewSection.StaticMesh->GetBoundingBox();

	// 把section 的material 设置给当前component, set material有mesh component实现过了
	SetMaterial(0, NewSection.StaticMesh->GetMaterial(0));

	UpdateLocalBounds(); //更新bounds
	MarkRenderStateDirty();  // new section需要重建新的proxy
}

void UQxDeformComponent::SetDeformSection(int32 SectinoIndex, const FDeformMeshSection& Section)
{
	if (SectinoIndex >= DeformMeshSections.Num())
	{
		DeformMeshSections.SetNum(SectinoIndex + 1, false);	
	}

	DeformMeshSections[SectinoIndex] = Section;

	UpdateLocalBounds();
	MarkRenderStateDirty();
}

FPrimitiveSceneProxy* UQxDeformComponent::CreateSceneProxy()
{
	return new FQxDeformMeshProxy(this);
	// 这里不创建新的proxy 会不会有问题 #TODO
	if (!SceneProxy)
	{
		return new FQxDeformMeshProxy(this);
	}
	else
	{
		return  SceneProxy;
	}
}

int32 UQxDeformComponent::GetNumMaterials() const
{
	return DeformMeshSections.Num();
}

FBoxSphereBounds UQxDeformComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));
    Ret.BoxExtent *= BoundsScale;
    Ret.SphereRadius *= BoundsScale;
	return Super::CalcBounds(LocalToWorld);
}

void UQxDeformComponent::ClearMeshSection(int32 SectionIndex)
{
	if (SectionIndex < DeformMeshSections.Num())
	{
		DeformMeshSections[SectionIndex].Reset();
		UpdateLocalBounds();
		MarkRenderStateDirty();
	}
}

void UQxDeformComponent::ClearAllMeshSections()
{
	DeformMeshSections.Empty();
	UpdateLocalBounds();
	MarkRenderStateDirty();
}

bool UQxDeformComponent::IsMeshSectionVisible(int32 SectionIndex) const
{
	return (SectionIndex < DeformMeshSections.Num()) ? DeformMeshSections[SectionIndex].bSectionVisible : false;
}

void UQxDeformComponent::SetMeshSectionVisible(int32 SectionIndex, bool bNewVisibility)
{
	if (SectionIndex < DeformMeshSections.Num())
	{
		DeformMeshSections[SectionIndex].bSectionVisible = bNewVisibility;

		if (SceneProxy)
		{
			FQxDeformMeshProxy* DeformMeshSceneProxy = (FQxDeformMeshProxy*)SceneProxy;
			ENQUEUE_RENDER_COMMAND(FDeformSectionVisiblityUpdate)(
				[DeformMeshSceneProxy, SectionIndex, bNewVisibility](FRHICommandListImmediate& RHICmdList)
				{
					DeformMeshSceneProxy->SetSectionVisiblity_RenderThread(SectionIndex, bNewVisibility);
				}
				);
			
		}
	}
}

int32 UQxDeformComponent::GetNumSections() const
{
	return DeformMeshSections.Num();
}

FDeformMeshSection* UQxDeformComponent::GetDeformMeshSection(int32 SectionIndex)
{
	if (SectionIndex < DeformMeshSections.Num())
	{
		return   &DeformMeshSections[SectionIndex];
	}
	else
	{
		return nullptr;
	}
}

void UQxDeformComponent::UpdateLocalBounds()
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


