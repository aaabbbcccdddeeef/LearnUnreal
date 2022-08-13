// Fill out your copyright notice in the Description page of Project Settings.


#include "QxRuntimMeshOperate.h"


// Sets default values
AQxRuntimMeshOperate::AQxRuntimMeshOperate()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxRuntimMeshOperate::BeginPlay()
{
	Super::BeginPlay();
	
}

void AQxRuntimMeshOperate::GenerateMeshCom_Test1()
{
	UStaticMeshComponent* MeshCom = NewObject<UStaticMeshComponent>(this,
		UStaticMeshComponent::StaticClass(),
		TEXT("Test Mesh Component"));
	MeshCom->SetFlags(RF_Transient);
	
	UStaticMesh* StaticMesh = NewObject<UStaticMesh>(MeshCom);
	MeshCom->SetStaticMesh(StaticMesh);
	StaticMesh->NeverStream = true;

	TUniquePtr<FStaticMeshRenderData> RenderData = MakeUnique<FStaticMeshRenderData>();
#pragma region InitRenderData
	RenderData->AllocateLODResources(1); //分配NumLOD个LODResource包括响应的FVertexFactory

	
	FStaticMeshLODResources& LODResources = RenderData->LODResources[0];
	FillLODResourcesWithTestData1(LODResources);

	// 设置包围盒
	{
		FBoxSphereBounds TestBounds;
		TestBounds.Origin = FVector::ZeroVector;
		TestBounds.SphereRadius = 100.f;
		RenderData->Bounds = TestBounds;
	}

#pragma endregion

	
	StaticMesh->SetRenderData( MoveTemp(RenderData));

	UMaterial* Material = UMaterial::GetDefaultMaterial(EMaterialDomain::MD_Surface);
	Material->TwoSided = true;

	StaticMesh->SetMaterial(0, Material);

	StaticMesh->InitResources(); // 初始化渲染资源等等
	StaticMesh->CalculateExtendedBounds(); // 根据renderdata 重新计算bounding box
	StaticMesh->GetRenderData()->ScreenSize[0].Default = 1.0f; // 设置LOD0 默认screen size和编辑器中的一致
	
	StaticMesh->CreateBodySetup();

	MeshCom->SetMobility(EComponentMobility::Movable);
	MeshCom->SetupAttachment(this->RootComponent);
	MeshCom->RegisterComponent();
}

void AQxRuntimMeshOperate::FillLODResourcesWithTestData1(FStaticMeshLODResources& OutLODResources)
{
	// build up index array
	TArray<uint32> indices;
	{
		indices.SetNum(3);
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
	}

	// 构建顶点数据
	TArray<FStaticMeshBuildVertex> StaticMeshBuildVertices;
	{
		StaticMeshBuildVertices.SetNum(indices.Num());

		// FStaticMeshBuildVertex封装了一些构建 static mesh 的顶点的数据
		FStaticMeshBuildVertex& vertex0 = StaticMeshBuildVertices[0];
		vertex0.Position = FVector(0, 0, 0);
		vertex0.Color = FColor(255, 0, 0);
		vertex0.UVs[0] = FVector2D(0, 0);
		vertex0.TangentX = FVector(0, 1, 0);
		vertex0.TangentY = FVector(1, 0, 0);
		vertex0.TangentY = FVector(0, 0, 1);
	
		FStaticMeshBuildVertex& vertex1 = StaticMeshBuildVertices[1];
		vertex1.Position = FVector(100, 0, 0);
		vertex1.Color = FColor(255, 0, 0);
		vertex1.UVs[0] = FVector2D(1, 0);
		vertex1.TangentX = FVector(0, 1, 0);
		vertex1.TangentY = FVector(1, 0, 0);
		vertex1.TangentY = FVector(0, 0, 1);

		FStaticMeshBuildVertex& vertex2 = StaticMeshBuildVertices[2];
		vertex2.Position = FVector(0, 100, 0);
		vertex2.Color = FColor(255, 0, 0);
		vertex2.UVs[0] = FVector2D(1, 0);
		vertex2.TangentX = FVector(0, 1, 0);
		vertex2.TangentY = FVector(1, 0, 0);
		vertex2.TangentY = FVector(0, 0, 1);
		
	}

	
	OutLODResources.IndexBuffer.SetIndices(indices, EIndexBufferStride::AutoDetect);
	OutLODResources.bHasDepthOnlyIndices = false;
	OutLODResources.bHasReversedIndices = false;
	OutLODResources.bHasReversedDepthOnlyIndices = false;
	
	OutLODResources.bHasColorVertexData = false;
	OutLODResources.VertexBuffers.PositionVertexBuffer.Init(StaticMeshBuildVertices);

	// 根据是否有顶点色初始化ColorVertexBuffer
	// OutLODResources.VertexBuffers.ColorVertexBuffer.Init()

	// 初始化LODResource 的 法线，切线，贴图坐标buffer
	OutLODResources.VertexBuffers.StaticMeshVertexBuffer.Init(StaticMeshBuildVertices, 1, false);

#pragma region InitResourceSections
	// 使用同一个材质的一组triangle是一个section，
	// FStaticMeshSection定义了材质index，三角面的fisr index和数量已经index buffer的起末,必须初始化
	FStaticMeshLODResources::FStaticMeshSectionArray& Sections = OutLODResources.Sections;
	FStaticMeshSection& MeshSection = Sections.AddDefaulted_GetRef();
	MeshSection.bEnableCollision = false;

	MeshSection.NumTriangles = StaticMeshBuildVertices.Num() / 3;
	MeshSection.FirstIndex = 0;
	MeshSection.MinVertexIndex = 0;
	MeshSection.MaxVertexIndex = StaticMeshBuildVertices.Num() - 1;

#pragma endregion
	
}

// Called every frame
void AQxRuntimMeshOperate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

