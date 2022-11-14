#include "QxTemporalAA.h"

#include "PixelShaderUtils.h"

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
        DECLARE_GLOBAL_SHADER(FQxTAACS);
        SHADER_USE_PARAMETER_STRUCT(FQxTAACS, FGlobalShader);
        BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
            SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HistoryTexture)
            SHADER_PARAMETER_SAMPLER(SamlerState, HistoryTextureSampler)
        END_SHADER_PARAMETER_STRUCT()
        static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
        {
            return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
        }
    };
    IMPLEMENT_GLOBAL_SHADER(FQxTAACS, "/QxAAShaders/QxTAA.usf", "MainCS", SF_Compute);
    
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
        FIntRect DestRect = Inputs.OutputViewRect;
        FIntRect SrcRect = Inputs.InputViewRect;
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
                TexCreate_ShaderResource |  TexCreate_UAV | TexCreate_RenderTargetable
                );
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


        // Pixel Shader 版本的实现
#pragma region PixelVersionImple
        {
            // FQxTAAPS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxTAAPS::FParameters>();
            // PassParams->HistoryTexture = Inputs.SceneColorInput;
            // PassParams->HistoryTextureSampler = TStaticSamplerState<SF_Point>::GetRHI();
            // PassParams->RenderTargets[0] = FRenderTargetBinding(Outputs.SceneColor, ERenderTargetLoadAction::ENoAction);
            //
            // TShaderMapRef<FQxTAAPS> PixelShader(View.ShaderMap);
            // const FScreenPassTextureViewport InputViewport(SrcRect);
            // const FScreenPassTextureViewport OutputViewport(DestRect);
            // AddDrawScreenPass(
            //     GraphBuilder,
            //     RDG_EVENT_NAME("QxTAAPass"),
            //     View,
            //     OutputViewport,
            //     InputViewport,
            //     PixelShader,
            //     PassParams
            //     );
        }
#pragma endregion

        // Compute Shader 版本实现
        {
            
        }
        
        {
            for (int32 i = 0; i < FTemporalAAHistory::kRenderTargetCount; ++i)
            {
                bUseHistoryTexture[i] = InputHistory.RT[i].IsValid();
            }
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
