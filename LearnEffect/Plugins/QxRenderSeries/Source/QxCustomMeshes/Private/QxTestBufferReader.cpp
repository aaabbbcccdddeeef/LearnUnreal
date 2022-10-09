// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTestBufferReader.h"


void FQxTestRenderData::ReInit(const TArray<FMatrix>& InMatrixs, uint32 InTestNum)
{
    TestMatrixs = InMatrixs;
    TestNum = InTestNum;

    ENQUEUE_RENDER_COMMAND(QxTestUpdateBuffer)(
        [this](FRHICommandListImmediate& RHICmdList)
        {
            ReInit_RenderThread(RHICmdList);
        }
        );
    
}

void FQxTestRenderData::ReInit_RenderThread(FRHICommandListImmediate& RHICmdList)
{
    check(IsInRenderingThread());

    //#TODO buffer已经分配并且够的情况不需要重新分配
    if (TestSB.IsValid())
    {
        TestSB.SafeRelease();
    }
    if (TestSRV.IsValid())
    {
        TestSRV.SafeRelease();
    }

    // TResourceArray 是渲染资源的array，一般情况和tarray相同，UMA的情况下不同
    TResourceArray<FMatrix>* ResourceArray = new TResourceArray<FMatrix>(true);
    // ResourceArray->Reserve(RenderData->ClippingVolumes.Num());
    ResourceArray->Append(TestMatrixs);
			
    // 预期回先用compute shader更新这个buffer，再渲染
    FRHIResourceCreateInfo ResourceCI;
			
    ResourceCI.ResourceArray = ResourceArray;
    ResourceCI.DebugName = TEXT("QxTestSB");
			
    TestSB = RHICreateStructuredBuffer(
        sizeof(FMatrix),
        sizeof(FMatrix) * TestMatrixs.Num(),
        BUF_ShaderResource | BUF_Dynamic,
        ResourceCI
        );

    TestSRV = RHICreateShaderResourceView(
        TestSB
        );
    
}

// Sets default values for this component's properties
UQxTestBufferReader::UQxTestBufferReader()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}

void UQxTestBufferReader::UpdateTestRenderDataAndBuffer()
{
    if (TestRenderData == nullptr)
    {
        TestRenderData = new FQxTestRenderData;
    }

    
    TestRenderData->ReInit(TestMatrix, TestNum);
}




// Called when the game starts
void UQxTestBufferReader::BeginPlay()
{
    Super::BeginPlay();

    // ...
    
}


// Called every frame
void UQxTestBufferReader::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

