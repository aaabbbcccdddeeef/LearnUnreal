// Fill out your copyright notice in the Description page of Project Settings.


#include "QxTemporalUpscaler.h"

#include "QxTemporalAA.h"
#include "SceneTextureParameters.h"
#include "PostProcess/PostProcessing.h"

TAutoConsoleVariable<float> CVarQxTemporalAACurrentFrameWeight(
    TEXT("r.QxTemporalAACurrentFrameWeight"),
    .04f,
    TEXT("Weight of current frame's contribution to the history.  Low values cause blurriness and ghosting, high values fail to hide jittering."),
    ECVF_Scalability | ECVF_RenderThreadSafe);

namespace 
{
    const int32 GQxTemporalAATileSizeX = 8;
    // class FQxTAAStandaloneCS : public FGlobalShader
    // {
    // public:
    //     class FTAAPassConfigDim : SHADER_PERMUTATION_ENUM_CLASS("TAA_PASS_CONFIG", ETAAPassConfig);
    //
    //     using FPermutationDomain = TShaderPermutationDomain<FTAAPassConfigDim>;
    //     
    //     DECLARE_GLOBAL_SHADER(FQxTAAStandaloneCS);
    //     SHADER_USE_PARAMETER_STRUCT(FQxTAAStandaloneCS, FGlobalShader);
    //     BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
    //         SHADER_PARAMETER(FVector4, ViewportUVToInputBufferUV)
    //
    //         SHADER_PARAMETER_RDG_TEXTURE(Texture2D, EyeAdaptationTexture)
    //
    //         SHADER_PARAMETER(float, CurrentFrameWeight)
    //
    //         SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputSceneColor)
    //         SHADER_PARAMETER_SAMPLER(SamplerState, InputSceneColorSampler)
    //
    //         SHADER_PARAMETER_ARRAY(float, SampleWidhts, [9])
    //         SHADER_PARAMETER_ARRAY(float, PlusWeights, [5])
    //     
    //         // History parameters
    //         SHADER_PARAMETER(FVector4, HistoryBufferSize)
    //         SHADER_PARAMETER(FVector4, HistoryBufferUVMinMax)
    //         SHADER_PARAMETER(FVector4, ScreenPosToHistoryBufferUV)
    //     
    //         // History Resources
    //         SHADER_PARAMETER_RDG_TEXTURE_ARRAY(Texture2D, HistoryBuffer, [FTemporalAAHistory::kRenderTargetCount])
    //         SHADER_PARAMETER_SAMPLER_ARRAY(SamplerState, HistoryBufferSampler,[FTemporalAAHistory::kRenderTargetCount])
    //     
    //         SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneDepthTexture)
    //         SHADER_PARAMETER_SAMPLER(SamplerState, SceneDepthTextureSampler)
    //         SHADER_PARAMETER_RDG_TEXTURE(Texture2D, GBufferVelocityTexture)
    //         SHADER_PARAMETER_SAMPLER(SamplerState, GBufferVelocityTextureSampler)
    //     
    //         SHADER_PARAMETER(FVector4, OutputViewportSize)
    //         SHADER_PARAMETER(FVector4, OutputViewportRect)
    //
    //         SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, ViewUniformBuffer)
    //     
    //         SHADER_PARAMETER_RDG_TEXTURE_UAV_ARRAY(Texture2D, OutComputeTex, [FTemporalAAHistory::kRenderTargetCount])
    //     END_SHADER_PARAMETER_STRUCT()
    //
    //     static void ModifyCompilationEnvironment(const FShaderPermutationParameters& Parameters,
    //         FShaderCompilerEnvironment& OutEnvironment)
    //     {
    //         OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZEX"), GQxTemporalAATileSizeX);
    //         OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZEY"), GQxTemporalAATileSizeX);
    //     }
    //
    //     /** Can be overridden by FShader subclasses to determine whether a specific permutation should be compiled. */
    //     static bool ShouldCompilePermutation(const FShaderPermutationParameters& Parameters)
    //     {
    //         FPermutationDomain PermutationVector(Parameters.PermutationId);
    //         
    //         
    //         return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    //     }
    // };
    // IMPLEMENT_GLOBAL_SHADER(FQxTAAStandaloneCS, "/QxPPShaders/QxTAAStandalone.usf", "MainCS", SF_Compute);

//     
// FTAAOutputs QxAddTemporalAAPass(
//     FRDGBuilder& GraphBuilder,
//     const FViewInfo& View,
//     const FTAAPassParameters& Inputs,
//     const FTemporalAAHistory& InputHistory,
//     FTemporalAAHistory* OutputHistory
//     )
// {
//     check(Inputs.Validate());
//
//     // Number of Render target in TAA History
//     const int32 InputTextureCount = 1;
//
//     // 这个是不是main taa pass
//     const bool bIsMainPass = IsMainTAAConfig(Inputs.Pass);
//
//     // whether to use camera cut shader permutation or not
//     const bool bCameraCut = !InputHistory.IsValid() || View.bCameraCut;
//
//     const FIntPoint OutputExtent = Inputs.GetOutputExtent();
//
//     // Src rectangle
//     const FIntRect SrcRect = Inputs.InputViewRect;
//     const FIntRect DestRect = Inputs.OutputViewRect;
//     const FIntRect PracticableSrcRect = FIntRect::DivideAndRoundUp(SrcRect, Inputs.ResolutionDivisor);
//     const FIntRect PracticableDestRect = FIntRect::DivideAndRoundUp(DestRect, Inputs.ResolutionDivisor);
//
//     const TCHAR* Passname = TEXT("QxTemporalAA");
//
//     FTAAOutputs Outputs;
//
//     TStaticArray<bool, FTemporalAAHistory::kRenderTargetCount> bUseHistoryTexture;
//     TStaticArray<FRDGTextureRef, FTemporalAAHistory::kRenderTargetCount> NewHistoryTexture;
//
//     // 分配需要的History Texture等资源，
//     {
//         EPixelFormat HistoryPixelFormat = EPixelFormat::PF_FloatRGBA;
//
//         FRDGTextureDesc SceneColorDesc = FRDGTextureDesc::Create2D(
//             OutputExtent,
//             HistoryPixelFormat,
//             FClearValueBinding::Black,
//             TexCreate_ShaderResource | TexCreate_UAV
//             );
//         if (Inputs.bOutputRenderTargetable)
//         {
//             SceneColorDesc.Flags |= TexCreate_RenderTargetable;
//         }
//
//         const TCHAR* OutputName = TEXT("QxTemporalAA");
//
//         for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
//         {
//             NewHistoryTexture[i] = GraphBuilder.CreateTexture(
//                 SceneColorDesc,
//                 OutputName,
//                 ERDGTextureFlags::MultiFrame
//                 );
//         }
//
//         NewHistoryTexture[0] = Outputs.SceneColor = NewHistoryTexture[0];
//
//         
//     }
//
//     // 组织shader 参数并dispatch pass
//     {
//         FQxTAAStandaloneCS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxTAAStandaloneCS::FParameters>();
//
//         const FIntPoint InputExtent = Inputs.SceneColorInput->Desc.Extent;
//         const FIntRect InputViewRect = Inputs.InputViewRect;
//         const FIntRect OutputViewRect = Inputs.OutputViewRect;
//         
//         const float ResDivisor = Inputs.ResolutionDivisor;
//         const float ResDivisorInv = 1.f / ResDivisor;
//
//         {
//             float InvSizeX = 1.f / float(InputExtent.X);
//             float InvSizeY = 1.f / float(InputExtent.Y);
//             PassParams->ViewportUVToInputBufferUV = FVector4(
//                 ResDivisorInv * InputViewRect.Width() * InvSizeX,
//                 ResDivisorInv * InputViewRect.Height() * InvSizeY,
//                 ResDivisorInv * InputViewRect.Min.X * InvSizeX,
//                 ResDivisorInv * InputViewRect.Min.Y * InvSizeY
//                 );
//         }
//
//         PassParams->EyeAdaptationTexture = GetEyeAdaptationTexture(GraphBuilder, View);
//         
//         PassParams->OutputViewportSize = FVector4(
//             PracticableDestRect.Width(), PracticableDestRect.Width(),
//             1.f / float(PracticableDestRect.Width()), 1.f / float(PracticableDestRect.Height())
//             );
//         PassParams->OutputViewportRect = FVector4(
//             PracticableDestRect.Min.X, PracticableDestRect.Min.Y,
//             PracticableDestRect.Max.X, PracticableDestRect.Max.Y
//             );
//         PassParams->ViewUniformBuffer = View.ViewUniformBuffer;
//         PassParams->CurrentFrameWeight = CVarQxTemporalAACurrentFrameWeight.GetValueOnAnyThread();
//         
//
//         // RDG_GPU_STAT_SCOPE(GraphBuilder, QXTAA);
//         for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
//         {
//             PassParams->OutComputeTex[i] = GraphBuilder.CreateUAV(NewHistoryTexture[i]);
//         }
//
//
//         FQxTAAStandaloneCS::FPermutationDomain PermutationVector;
//         //#TODO 先只考虑main pass的情况
//         PermutationVector.Set<FQxTAAStandaloneCS::FTAAPassConfigDim>(ETAAPassConfig::Main);
//         
//         TShaderMapRef<FQxTAAStandaloneCS> ComputeShader(View.ShaderMap, PermutationVector);
//         // TShaderMapRef<FQxTAAStandaloneCS> ComputeShader(View.ShaderMap);
//
//         ClearUnusedGraphResources(ComputeShader, PassParams);
//
//         for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
//         {
//             // bUseHistoryTexture[i] = 
//         }
//         
//         FComputeShaderUtils::AddPass(
//             GraphBuilder,
//             RDG_EVENT_NAME("QxTAA %s%s %dx%d -> %dx%d",
//                 Passname, Inputs.bUseFast ? TEXT(" Fast") : TEXT(""),
//                 PracticableSrcRect.Width(), PracticableSrcRect.Height(),
//                 PracticableDestRect.Width(), PracticableDestRect.Height()),
//                 ComputeShader,
//                 PassParams,
//                 FComputeShaderUtils::GetGroupCount(PracticableDestRect.Size(), GQxTemporalAATileSizeX)
//             );
//     }
//     
//     if (!View.bStatePrevViewInfoIsReadOnly)
//     {
//         OutputHistory->SafeRelease();
//         for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
//         {
//             if (bUseHistoryTexture[i])
//             {
//                 GraphBuilder.QueueTextureExtraction(NewHistoryTexture[i], &OutputHistory->RT[i]);
//             }
//         }
//         
//         OutputHistory->ViewportRect = DestRect;
//         OutputHistory->ReferenceBufferSize = OutputExtent * Inputs.ResolutionDivisor;
//     }
//     return Outputs;
// }
//

    
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
    // TAAParameters.bUseFast = GetPostProcessAAQuality() == LowQualityTemporalAA;
    TAAParameters.bUseFast = false;
    
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
    
    const FTAAOutputs TAAOutputs = QxTemporalAA::QxAddTemporalAAPass(
        GraphBuilder,
        View,
        TAAParameters,
        InputHistory,
        &OutputHistory
        );
    
    // FTAAOutputs TAAOutputs;
    // TAAOutputs.SceneColor = PassInputs.SceneDepthTexture;
    
    
    FRDGTextureRef SceneColorTexture = TAAOutputs.SceneColor;
    
    *OutSceneColorTexture = SceneColorTexture;
    *OutSceneColorViewRect = SecondaryViewRect;
    *OutSceneColorHalfResTexture = TAAOutputs.DownsampledSceneColor;
    *OutSceneColorHalfResViewRect = FIntRect::DivideAndRoundUp(SecondaryViewRect, 2);
}
