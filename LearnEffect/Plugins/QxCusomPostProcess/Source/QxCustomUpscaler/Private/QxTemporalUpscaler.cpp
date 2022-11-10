// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTemporalUpscaler.h"
#include "PostProcess/PostProcessing.h"


FTAAOutputs QxAddTemporalAAPass(
    FRDGBuilder& GraphBuilder,
    const FViewInfo& View,
    const FTAAPassParameters& Inputs,
    const FTemporalAAHistory& InputHistory,
    FTemporalAAHistory* OutputHistory
    )
{
    check(Inputs.Validate());

    // Number of Render target in TAA History
    const int32 InputTextureCount = 1;

    // 这个是不是main taa pass
    const bool bIsMainPass = IsMainTAAConfig(Inputs.Pass);

    // whether to use camera cut shader permutation or not
    const bool bCameraCut = !InputHistory.IsValid() || View.bCameraCut;

    const FIntPoint OutputExtent = Inputs.GetOutputExtent();

    // Src rectangle
    const FIntRect SrcRect = Inputs.InputViewRect;
    const FIntRect DestRect = Inputs.OutputViewRect;
    const FIntRect PracticableSrcRect = FIntRect::DivideAndRoundUp(SrcRect, Inputs.ResolutionDivisor);
    const FIntRect PracticableDestRect = FIntRect::DivideAndRoundUp(DestRect, Inputs.ResolutionDivisor);

    const TCHAR* Passname = TEXT("QxTemporalAA");

    FTAAOutputs Outputs;

    TStaticArray<bool, FTemporalAAHistory::kRenderTargetCount> bUseHistoryTexture;
    TStaticArray<FRDGTextureRef, FTemporalAAHistory::kRenderTargetCount> NewHistoryTexture;

    // 分配需要的History Texture等资源，
    {
        EPixelFormat HistoryPixelFormat = EPixelFormat::PF_FloatRGBA;

        FRDGTextureDesc SceneColorDesc = FRDGTextureDesc::Create2D(
            OutputExtent,
            HistoryPixelFormat,
            FClearValueBinding::Black,
            TexCreate_ShaderResource | TexCreate_UAV
            );
        if (Inputs.bOutputRenderTargetable)
        {
            SceneColorDesc.Flags |= TexCreate_RenderTargetable;
        }

        const TCHAR* OutputName = TEXT("QxTemporalAA");
        
    }

    // 组织shader 参数并dispatch pass
    {
        
    }
    
    if (!View.bStatePrevViewInfoIsReadOnly)
    {
        OutputHistory->SafeRelease();
        for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
        {
            if (bUseHistoryTexture[i])
            {
                GraphBuilder.QueueTextureExtraction(NewHistoryTexture[i], &OutputHistory->RT[i]);
            }
        }
        
        OutputHistory->ViewportRect = DestRect;
        OutputHistory->ReferenceBufferSize = OutputExtent * Inputs.ResolutionDivisor;
    }
}


void FQxTemporalUpscaler::AddPasses(FRDGBuilder& GraphBuilder, const FViewInfo& View, const FPassInputs& PassInputs,
    FRDGTextureRef* OutSceneColorTexture, FIntRect* OutSceneColorViewRect, FRDGTextureRef* OutSceneColorHalfResTexture,
    FIntRect* OutSceneColorHalfResViewRect) const
{
    check(View.AntiAliasingMethod  == EAntiAliasingMethod::AAM_TemporalAA && View.ViewState);

    FTAAPassParameters TAAParameters(View);

    TAAParameters.Pass = View.PrimaryScreenPercentageMethod == EPrimaryScreenPercentageMethod::TemporalUpscale ?
        ETAAPassConfig::MainUpsampling :
        ETAAPassConfig::Main;
    TAAParameters.SetupViewRect(View);
    
    const EPostProcessAAQuality LowQualityTemporalAA = EPostProcessAAQuality::Medium;
    TAAParameters.bUseFast = GetPostProcessAAQuality() == LowQualityTemporalAA;

    const FIntRect SecondaryViewRect = TAAParameters.OutputViewRect;

    // 先不考虑upscale的问题
    const float HistoryUpscalerFactor = 1;

    TAAParameters.DownsampleOverrideFormat = PassInputs.DownsampleOverrideFormat;
    TAAParameters.bDownsample = PassInputs.bAllowDownsampleSceneColor && TAAParameters.bUseFast;
    TAAParameters.SceneDepthTexture = PassInputs.SceneDepthTexture;
    TAAParameters.SceneVelocityTexture = PassInputs.SceneVelocityTexture;
    TAAParameters.SceneColorInput = PassInputs.SceneColorTexture;

    const FTemporalAAHistory& InputHistory = View.PrevViewInfo.TemporalAAHistory;
    FTemporalAAHistory& OutputHistory = View.ViewState->PrevFrameViewInfo.TemporalAAHistory;
    
    const FTAAOutputs TAAOutputs = QxAddTemporalAAPass(
        GraphBuilder,
        View,
        TAAParameters,
        InputHistory,
        &OutputHistory
        );
    
    FRDGTextureRef SceneColorTexture = TAAOutputs.SceneColor;
    
    *OutSceneColorTexture = SceneColorTexture;
    *OutSceneColorViewRect = SecondaryViewRect;
    *OutSceneColorHalfResTexture = TAAOutputs.DownsampledSceneColor;
    *OutSceneColorHalfResViewRect = FIntRect::DivideAndRoundUp(SecondaryViewRect, 2);
}
