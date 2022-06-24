// Fill out your copyright notice in the Description page of Project Settings.


#include "QxOccluder.h"

#include "Components/BoxComponent.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FQxOccluderParameters, "QxBoxOccluders");

// 下面部分是渲染线程的不要在游戏线程中改
#pragma region RenderThreadParts
TUniformBufferRef<FQxOccluderParameters> QxOccluderUniiformBuffer;
#pragma endregion



// Sets default values
AQxOccluder::AQxOccluder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Component"));
	SetRootComponent(BoxComponent);
	
}

// Called when the game starts or when spawned
void AQxOccluder::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQxOccluder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UploadBoxOccluderBuffers();
}

void UploadBoxOccluderBuffers_RenderThread(TArray<FBoxOccluder>& InBoxOccluder, FRHICommandListImmediate& RHICmdList)
{
	
	SCOPED_DRAW_EVENT(RHICmdList, QxUploadOccluder); 
	
	// TestStruct testElement;
	// testElement.TestPosition = FVector(1.f, 1.f, 1.f);
	TResourceArray<FVector4> bufferData;
	bufferData.Reset();
	bufferData.Add(InBoxOccluder[0].MinLocation);
	bufferData.SetAllowCPUAccess(true);

	FRHIResourceCreateInfo testCreateInfo;
	testCreateInfo.ResourceArray = &bufferData;
	
	FStructuredBufferRHIRef MinLocationBufferRef;
	MinLocationBufferRef = RHICreateStructuredBuffer(sizeof(FVector4)
			, sizeof(FVector4) * 1, BUF_UnorderedAccess | BUF_ShaderResource,
			testCreateInfo);
	FUnorderedAccessViewRHIRef TestStructureBuffUAV;
	TestStructureBuffUAV = RHICreateUnorderedAccessView(MinLocationBufferRef,
		true, false);

	FQxOccluderParameters ViewUniformBufferParameters;
	QxOccluderUniiformBuffer =
		TUniformBufferRef<FQxOccluderParameters>::CreateUniformBufferImmediate(
			ViewUniformBufferParameters, UniformBuffer_MultiFrame, EUniformBufferValidation::None);
	ViewUniformBufferParameters.MinLocationBuffer = TestStructureBuffUAV;

	// FlushRenderingCommands();
	 // QxOccluderUniiformBuffer.UpdateUniformBufferImmediate()
}

void AQxOccluder::UploadBoxOccluderBuffers()
{
	check(IsInGameThread());
	// 测试，先这么写
	BoxOccluders.Empty();
	FBoxOccluder testOccluder;
	testOccluder.MinLocation = FVector4(1.f, 2.f, 3.f, 1.f);
	testOccluder.MinLocation = FVector4(6, 6, 6, 1);
	BoxOccluders.Push(testOccluder);

	ENQUEUE_RENDER_COMMAND(QxUploadOccluderBuffer)(
		[this, testOccluder](FRHICommandListImmediate& RHICmdList)
		{
			UploadBoxOccluderBuffers_RenderThread(BoxOccluders, RHICmdList);
		}
		);
}

void AQxOccluder::UploadBoxOccluderBuffers_RDG()
{
	
	
}



