// Fill out your copyright notice in the Description page of Project Settings.


#include "QxEnqueRCTest.h"

#include "ClearQuad.h"
#include "Engine/TextureRenderTarget2D.h"


// Sets default values
AQxEnqueRCTest::AQxEnqueRCTest()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AQxEnqueRCTest::BeginPlay()
{
    Super::BeginPlay();
}

// FRHICommandListImmediate& QxGetImmediateCommandList_ForRenderCommand()
// {
//     return  FRHICommandListExecutor::GetImmediateCommandList();
// }

template<typename LAMBDA>
class FQxRenderCommand
{
public:
    FQxRenderCommand(LAMBDA&& InLambda)
        : Lambda(Forward<LAMBDA>(InLambda))
    {
    }

    static ENamedThreads::Type GetDesiredThread()
    {
        return ENamedThreads::GetRenderThread();
    }

    static ESubsequentsMode::Type GetSubsequentsMode()
    {
        // Render Command 不关心依赖
        return ESubsequentsMode::FireAndForget;
    }

    void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionEvent)
    {
        FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
        Lambda(RHICmdList);
    }

    inline TStatId GetStatId()  const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FQxRenderCommand, STATGROUP_RenderThreadCommands);
    }
    
private:
    LAMBDA Lambda;
};

template<typename  LAMBDA>
void QxEnqueRenderCommand(LAMBDA&& Lambda)
{
    if (IsInRenderingThread())
    {
        FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
        Lambda(RHICmdList);
    }
    else
    {
        TGraphTask<FQxRenderCommand<LAMBDA>>::CreateTask().ConstructAndDispatchWhenReady(Forward<LAMBDA>(Lambda));
    }
}

void AQxEnqueRCTest::TestMyEnqueRenderCommand(UTextureRenderTarget2D* InRenderTarget, FLinearColor ClearColor)
{
    if (!InRenderTarget)
    {
        return;
    }

    FTextureRenderTargetResource* TmpTestRenderTarget = InRenderTarget->GameThread_GetRenderTargetResource();
    QxEnqueRenderCommand(
            [TmpTestRenderTarget, ClearColor](FRHICommandListImmediate& RHICmdList)
            {
                FRHIRenderPassInfo RPInfo(TmpTestRenderTarget->GetRenderTargetTexture(), ERenderTargetActions::DontLoad_Store);
                TransitionRenderPassTargets(RHICmdList, RPInfo);
                RHICmdList.BeginRenderPass(RPInfo, TEXT("QxMyClearRT"));
                DrawClearQuad(RHICmdList, ClearColor);
                RHICmdList.EndRenderPass();
                RHICmdList.Transition(FRHITransitionInfo(TmpTestRenderTarget->GetRenderTargetTexture(),
                    ERHIAccess::RTV, ERHIAccess::SRVMask));
            }
        );

    // TGraphTask<>::CreateTask()
}


void AQxEnqueRCTest::TestUseEnqueRenderCommand(UTextureRenderTarget2D* InRenderTarget,FLinearColor ClearColor)
{
    if (!InRenderTarget)
    {
        return;
    }

    FTextureRenderTargetResource* TmpTestRenderTarget = InRenderTarget->GameThread_GetRenderTargetResource();
    ENQUEUE_RENDER_COMMAND(ClearRenderTarget)(
        [TmpTestRenderTarget, ClearColor](FRHICommandListImmediate& RHICmdList)
        {
            FRHIRenderPassInfo RPInfo(TmpTestRenderTarget->GetRenderTargetTexture(), ERenderTargetActions::DontLoad_Store);
            TransitionRenderPassTargets(RHICmdList, RPInfo);
            RHICmdList.BeginRenderPass(RPInfo, TEXT("QxClearRT"));
            DrawClearQuad(RHICmdList, ClearColor);
            RHICmdList.EndRenderPass();

            RHICmdList.Transition(FRHITransitionInfo(TmpTestRenderTarget->GetRenderTargetTexture(),
                ERHIAccess::RTV, ERHIAccess::SRVMask));
        }
        );
}

// Called every frame
void AQxEnqueRCTest::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

