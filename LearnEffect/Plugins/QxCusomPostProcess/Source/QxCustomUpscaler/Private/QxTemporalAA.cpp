#include "QxTemporalAA.h"

#include "PixelShaderUtils.h"

TAutoConsoleVariable<float> CVarQxCurrentFrameWeight(
    TEXT("r.QxTAAWeight"),
    .04f,
    TEXT("Weight of current frame's contribution to the history.  Low values cause blurriness and ghosting, high values fail to hide jittering."),
    ECVF_Scalability | ECVF_RenderThreadSafe);

namespace QxTemporalAA
{
    class FQxTAAPS : public  FGlobalShader
    {
    public:
        DECLARE_GLOBAL_SHADER(FQxTAAPS);
        SHADER_USE_PARAMETER_STRUCT(FQxTAAPS, FGlobalShader);
        BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
            SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistoryTexture)
            SHADER_PARAMETER_SAMPLER(SamlerState, HistoryTextureSampler)
            RENDER_TARGET_BINDING_SLOTS()
        END_SHADER_PARAMETER_STRUCT()
        static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
        {
            return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
        }
    };
    IMPLEMENT_GLOBAL_SHADER(FQxTAAPS, "/QxAAShaders/QxTAA.usf", "MainPS", SF_Pixel);

    class FQxTAACS : public FGlobalShader
    {
    public:
        static constexpr int32 GroupSize = 8;
        DECLARE_GLOBAL_SHADER(FQxTAACS);
        SHADER_USE_PARAMETER_STRUCT(FQxTAACS, FGlobalShader);
        BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
            SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColorTexture)
            SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistoryTexture)
            SHADER_PARAMETER_SAMPLER(SamlerState, HistoryTextureSampler)
            SHADER_PARAMETER_RDG_TEXTURE_UAV(Texture2D, OutputComputeTex)
            SHADER_PARAMETER(FVector4, OutputViewportSize)
            SHADER_PARAMETER(FVector4, ScreenPosToHistoryTextureUV)
            SHADER_PARAMETER(FVector4, HistoryBufferSize)
            SHADER_PARAMETER(FVector4, HistoryBufferUVMinMax)
            SHADER_PARAMETER(FVector4, InputSceneColorSize)
            SHADER_PARAMETER(FVector4, ViewportUVToInputBufferUV)

            SHADER_PARAMETER(FIntPoint, InputMinPixelCoord)
            SHADER_PARAMETER(FIntPoint, InputMaxPixelCoord)
            SHADER_PARAMETER(FVector2D, ScreenPosAbsMax)
            SHADER_PARAMETER(float, CurrentFrameWeight)
        END_SHADER_PARAMETER_STRUCT()
        static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
        {
            return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
        }

        static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
        {
            OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZEX"), GroupSize);
            OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZEY"), GroupSize);
        }
        
    };
    IMPLEMENT_GLOBAL_SHADER(FQxTAACS, "/QxAAShaders/QxTAACS.usf", "MainCS", SF_Compute);
    
    // 尝试自定义TAA
    FTAAOutputs QxAddTemporalAAPass(FRDGBuilder& GraphBuilder, const FViewInfo& View, const FTAAPassParameters& Inputs,
        const FTemporalAAHistory& InputHistory, FTemporalAAHistory* OutputHistory)
    {
        // check(Inputs.Validate());

        const bool isMainPassConfig = Inputs.Pass == ETAAPassConfig::Main;
        // 现在只支持main pass 不支持up sample
        check(isMainPassConfig);
        check(Inputs.bDownsample == false);

        RDG_EVENT_SCOPE(GraphBuilder, "QxTAA");
        const FIntRect DestRect = Inputs.OutputViewRect;
        const FIntRect SrcRect = Inputs.InputViewRect;
        const FIntRect PracticableSrcRect = FIntRect::DivideAndRoundUp(SrcRect, Inputs.ResolutionDivisor);
        const FIntRect PracticableDestRect = FIntRect::DivideAndRoundUp(DestRect, Inputs.ResolutionDivisor);
        
        const FIntPoint OutputExtent = Inputs.SceneColorInput->Desc.Extent; //Inputs.GetOutputExtent();
        
        FTAAOutputs Outputs;

        TStaticArray<FRDGTextureRef, FTemporalAAHistory::kRenderTargetCount> NewHistoryTexture;
        TStaticArray<bool, FTemporalAAHistory::kRenderTargetCount> bUseHistoryTexture;

        // 创建新的HistoryTextures
        {
            EPixelFormat HistoryPixelFormat = PF_FloatRGBA;
            FRDGTextureDesc SceneColorDesc = FRDGTextureDesc::Create2D(
                OutputExtent,
                HistoryPixelFormat,
                FClearValueBinding::Black,
                // 这里不需要这么多 #TODO
                TexCreate_ShaderResource |  TexCreate_UAV 
                );
            if (true)
            {
                SceneColorDesc.Flags |= TexCreate_RenderTargetable;
            }
            const TCHAR* OutputName = TEXT("QxTemporalAA");
            for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
            {
                NewHistoryTexture[i] = GraphBuilder.CreateTexture(
                    SceneColorDesc,
                    OutputName,
                    ERDGTextureFlags::MultiFrame
                    );
            }
            NewHistoryTexture[0] = Outputs.SceneColor = NewHistoryTexture[0];
            
        }

        

        bool bUseComputeShader = true;
        // Compute Shader 版本实现
        if (bUseComputeShader)
        {
            FIntPoint ReferenceViewportOffset = InputHistory.ViewportRect.Min;
            FIntPoint ReferenceViewportSize = InputHistory.ViewportRect.Size();
            FIntPoint ReferenceBufferSize = InputHistory.ReferenceBufferSize;
            float InvReferenceBufferSizeX = 1.f / float(InputHistory.ReferenceBufferSize.X);
            float InvReferenceBufferSizeY = 1.f / float(InputHistory.ReferenceBufferSize.Y);
            FIntPoint ViewportOffset = ReferenceViewportOffset / Inputs.ResolutionDivisor;
            FIntPoint ViewportSize = FIntPoint::DivideAndRoundUp(ReferenceViewportSize, Inputs.ResolutionDivisor);
            FIntPoint BufferSize = ReferenceBufferSize / Inputs.ResolutionDivisor;
            float InvBufferSizeX = 1.f / float(BufferSize.X);
            float InvBufferSizeY = 1.f / float(BufferSize.Y);
            const float ResDivisor = Inputs.ResolutionDivisor;
            const float ResDivisorInv =  1.f  / ResDivisor;
            const FIntRect InputViewRect = Inputs.InputViewRect;
            const FIntRect OutputViewRect = Inputs.OutputViewRect;

            const FIntPoint SceneColorExtent = Inputs.SceneColorInput->Desc.Extent;
            
            
            FRDGTextureRef BlackDummy = GraphBuilder.RegisterExternalTexture(GSystemTextures.BlackDummy);
            const TCHAR* PassName = TEXT("QxTAAPass");
            FQxTAACS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxTAACS::FParameters>();
            PassParams->SceneColorTexture = Inputs.SceneColorInput;

            
#pragma region SetupHistoryTextureParam
            // #TODO History texture换成数组
            // for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
            // {
            //     if (InputHistory.RT[i].IsValid())
            //     {
            //         PassParams->HistoryTexture 
            //     }
            // }
            if (InputHistory.RT[0].IsValid())
            {
                PassParams->HistoryTexture = GraphBuilder.RegisterExternalTexture(InputHistory.RT[0]);
            }
            else
            {
                PassParams->HistoryTexture = BlackDummy;
            }
            PassParams->HistoryTextureSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
#pragma endregion
            PassParams->ScreenPosToHistoryTextureUV = FVector4(
                0.5f * ReferenceViewportSize.X   * InvReferenceBufferSizeX,
                -0.5 * ReferenceViewportSize.Y * InvReferenceBufferSizeY,
                (ReferenceViewportSize.X * 0.5f + ReferenceViewportOffset.X) * InvReferenceBufferSizeX,
                (ReferenceViewportSize.Y * 0.5f + ReferenceViewportOffset.Y) * InvReferenceBufferSizeY
                );
            
            PassParams->OutputViewportSize = FVector4(
                PracticableDestRect.Width(), PracticableDestRect.Height(),
                1.f/float(PracticableDestRect.Width()), 1.f/float(PracticableDestRect.Height())
                );
            PassParams->HistoryBufferSize = FVector4(
                BufferSize.X, BufferSize.Y,
                InvBufferSizeX, InvBufferSizeY
                );
            PassParams->HistoryBufferUVMinMax = FVector4(
                (ViewportOffset.X + 0.5f) * InvBufferSizeX,
                (ViewportOffset.Y + 0.5f) * InvBufferSizeY,
                (ViewportOffset.X + ViewportSize.X  - 0.5f) * InvBufferSizeX,
                (ViewportOffset.Y + ViewportSize.Y - 0.5f) * InvBufferSizeY
                );

            // UE4 中常用的Screen space是左下角-1，-1右上角1,1的,空间实际采样，只到角落的像素的中心，所以这里减去
            PassParams->ScreenPosAbsMax = FVector2D(1.f - 1.f/float(ViewportSize.X),
                1 - 1.f/float(ViewportSize.Y));
            
            PassParams->CurrentFrameWeight = CVarQxCurrentFrameWeight.GetValueOnAnyThread();

            PassParams->InputSceneColorSize = FVector4(
                SceneColorExtent.X, SceneColorExtent.Y,
                1.f/float(SceneColorExtent.X), 1.f/float(SceneColorExtent.Y)
                );
            PassParams->InputMaxPixelCoord = PracticableSrcRect.Min;
            PassParams->InputMaxPixelCoord = PracticableSrcRect.Max - FIntPoint(1, 1);
            {
                const float InvSizeX = 1.f/ float(SceneColorExtent.X);
                const float InvSizeY = 1.f/ float(SceneColorExtent.Y);
                PassParams->ViewportUVToInputBufferUV = FVector4(
                    ResDivisorInv * InputViewRect.Width() * InvSizeX,
                    ResDivisorInv * InputViewRect.Height() * InvSizeY,
                    ResDivisorInv * InputViewRect.Min.X * InvSizeX,
                    ResDivisorInv * InputViewRect.Min.Y * InvSizeY
                    );
            }

            // Create UAVs and setup parameter
            {
                for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
                {
                    PassParams->OutputComputeTex = GraphBuilder.CreateUAV(Outputs.SceneColor);
                }
            }

            TShaderMapRef<FQxTAACS> ComputeShader(View.ShaderMap);
            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("QxTAA %s%s %dx%d -> %dx%d",
                    PassName, Inputs.bUseFast ? TEXT("Fast") : TEXT(""),
                    PracticableSrcRect.Width(), PracticableSrcRect.Height(),
                    PracticableDestRect.Width(), PracticableDestRect.Height()),
                ComputeShader,
                PassParams,
                FComputeShaderUtils::GetGroupCount(PracticableDestRect.Size(), FQxTAACS::GroupSize)
                );
        }
        else
        {        // Pixel Shader 版本的实现
            #pragma region PixelVersionImple
            FQxTAAPS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxTAAPS::FParameters>();
            PassParams->HistoryTexture = Inputs.SceneColorInput;
            PassParams->HistoryTextureSampler = TStaticSamplerState<SF_Point>::GetRHI();
            PassParams->RenderTargets[0] = FRenderTargetBinding(Outputs.SceneColor, ERenderTargetLoadAction::ENoAction);
            
            TShaderMapRef<FQxTAAPS> PixelShader(View.ShaderMap);
            const FScreenPassTextureViewport InputViewport(SrcRect);
            const FScreenPassTextureViewport OutputViewport(DestRect);
            AddDrawScreenPass(
                GraphBuilder,
                RDG_EVENT_NAME("QxTAAPass"),
                View,
                OutputViewport,
                InputViewport,
                PixelShader,
                PassParams
                );
            #pragma endregion
        }
        

        {
            for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
            {
                bUseHistoryTexture[i] = InputHistory.RT[i].IsValid();
            }
            bUseHistoryTexture[0] = true;
        }
        
        // 设置输出的OutputHistory
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
        return Outputs;
    }
}
