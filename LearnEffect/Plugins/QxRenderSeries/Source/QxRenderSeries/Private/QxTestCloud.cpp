// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestCloud.h"
#include <Engine/TextureRenderTarget2D.h>
#include <Shader.h>

#define LOCTEXT_NAMESPACE "QxTestCloud"

//~ Begin Define My Volume Computer Shader
class FQxVolumeCSShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FQxVolumeCSShader, Global)
public:
	FQxVolumeCSShader()
	{

	}

	FQxVolumeCSShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		CloudOutputSurface.Bind(Initializer.ParameterMap, TEXT("RWOutputSurface"));
		TestStructureBufferSurface.Bind(Initializer.ParameterMap, TEXT("TestStructBuffer"));

	}

	//----------------------------------------------------//
	static bool ShouldCache(EShaderPlatform PlateForm)
	{
		return IsFeatureLevelSupported(PlateForm, ERHIFeatureLevel::SM5);
	}
	//----------------------------------------------------//
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	//----------------------------------------------------//
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		//Define micro here
		//OutEnvironment.SetDefine(TEXT("TEST_MICRO"), 1);
	}

	void SetSurface(FRHICommandList& RHICmdList,
		FUnorderedAccessViewRHIRef& OutputSurfaceUAV,
		FUnorderedAccessViewRHIRef& TestStructureUAV)
	{
		// 设置uavs
		//GetComputeShader();
		FRHIComputeShader* computeShaderRHI = RHICmdList.GetBoundComputeShader();

		if (CloudOutputSurface.IsBound())
		{
			RHICmdList.SetUAVParameter(computeShaderRHI, CloudOutputSurface.GetBaseIndex(),
				OutputSurfaceUAV);
		}
		if (TestStructureBufferSurface.IsBound())
		{
			RHICmdList.SetUAVParameter(computeShaderRHI, TestStructureBufferSurface.GetBaseIndex(),
				TestStructureUAV);
		}
	}

	void UnBindBuffers(FRHICommandList& RHICmdList)
	{
		FRHIComputeShader* computeShaderRHI = RHICmdList.GetBoundComputeShader();
		if (CloudOutputSurface.IsBound())
		{
			RHICmdList.SetUAVParameter(computeShaderRHI, CloudOutputSurface.GetBaseIndex(),
				FUnorderedAccessViewRHIRef());
		}
		if (TestStructureBufferSurface.IsBound())
		{
			RHICmdList.SetUAVParameter(computeShaderRHI, TestStructureBufferSurface.GetBaseIndex(),
				FUnorderedAccessViewRHIRef());
		}
	}

private:
	LAYOUT_FIELD(FShaderResourceParameter, CloudOutputSurface);
	LAYOUT_FIELD(FShaderResourceParameter, TestStructureBufferSurface);
};

IMPLEMENT_SHADER_TYPE(, FQxVolumeCSShader, TEXT("/QxShaders/QxVolumeCloudSky.usf"), TEXT("MainCS"), SF_Compute);

//~ end Define My Volume Computer Shader


// Sets default values
AQxTestCloud::AQxTestCloud()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AQxTestCloud::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQxTestCloud::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AQxTestCloud::RenderCloud_RenderThread(
	FRHICommandListImmediate& RHICmdList, 
	ERHIFeatureLevel::Type FeatureLevel, 
	FRHITexture* InRenderTarget, 
	int32 sizeX, int32 sizeY)
{
	check(IsInRenderingThread());
	check(InRenderTarget != nullptr);

	TShaderMapRef<FQxVolumeCSShader> qxVolumeCSShader(GetGlobalShaderMap(FeatureLevel));
	RHICmdList.SetComputeShader(qxVolumeCSShader.GetComputeShader());

	// Init 
	if (RenderCloud_RenderThreadInit == false ||
		TextureRHI.IsValid() == false ||
		TextureUAV.IsValid() == false)
	{
		FRHIResourceCreateInfo rhiCreateInfo;
		TextureRHI = RHICreateTexture2D(sizeX, sizeY, PF_FloatRGBA, 1, 1,
			ETextureCreateFlags::TexCreate_ShaderResource | TexCreate_UAV, rhiCreateInfo);
		TextureUAV = RHICreateUnorderedAccessView(TextureRHI);

		TestStruct testElement;
		testElement.TestPosition = FVector(1.f, 1.f, 1.f);
		TResourceArray<TestStruct> bufferData;
		bufferData.Reset();
		bufferData.Add(testElement);
		bufferData.SetAllowCPUAccess(true);

		FRHIResourceCreateInfo testCreateInfo;
		testCreateInfo.ResourceArray = &bufferData;

		TestStructureBuff = RHICreateStructuredBuffer(sizeof(TestStruct)
			, sizeof(TestStruct) * 1, BUF_UnorderedAccess | BUF_ShaderResource,
			testCreateInfo);
		TestStructureBuffUAV = RHICreateUnorderedAccessView(TestStructureBuff, true, false);

		RenderCloud_RenderThreadInit = true;
	}

	qxVolumeCSShader->SetSurface(RHICmdList, TextureUAV, TestStructureBuffUAV);
	
	DispatchComputeShader(RHICmdList, qxVolumeCSShader, sizeX / 32, sizeY / 32, 1);
	qxVolumeCSShader->UnBindBuffers(RHICmdList);

	FRHICopyTextureInfo copyTexInfo;
	copyTexInfo.Size = FIntVector(sizeX, sizeY, 0);

	TArray<FVector> data;
	data.Reset();
	FVector testEle(1.f, 1.f, 1.f);
	data.Add(testEle);

	// 注意：从GPU读回内容相当于1个pass，而且可能造成同步导致效率较低 
	FVector* srcptr = (FVector*)RHILockStructuredBuffer(TestStructureBuff.GetReference(),
		0, sizeof(FVector), EResourceLockMode::RLM_ReadOnly);
	FMemory::Memcpy(data.GetData(), srcptr, sizeof(TestStruct));
	RHIUnlockStructuredBuffer(TestStructureBuff.GetReference());
}

void AQxTestCloud::RenderCloud(UTextureRenderTarget2D* InRenderTarget)
{
	UWorld* world = GetWorld();
	ERHIFeatureLevel::Type featureLevel = world->Scene->GetFeatureLevel();
	checkf(featureLevel == ERHIFeatureLevel::SM5, TEXT("Only support SM5"));

	FTextureReferenceRHIRef	rtRHI = InRenderTarget->TextureReference.TextureReferenceRHI;
	checkf(rtRHI != nullptr, TEXT("cannot get render target texture"));

	FRHITexture* renderTargetTextureRef = rtRHI->GetReferencedTexture();
	int32 sizeX = InRenderTarget->SizeX;
	int32 sizeY = InRenderTarget->SizeY;

	ENQUEUE_RENDER_COMMAND(QxRenderTestCloud)(
		[featureLevel, renderTargetTextureRef, sizeX, sizeY, this](FRHICommandListImmediate& RHICmdList)
		{
			RenderCloud_RenderThread(
				RHICmdList,
				featureLevel,
				renderTargetTextureRef,
				sizeX,
				sizeY
			);
		}
		);
}

#undef LOCTEXT_NAMESPACE