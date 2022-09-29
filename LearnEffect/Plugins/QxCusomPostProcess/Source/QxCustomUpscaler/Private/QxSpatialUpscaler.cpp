// Fill out your copyright notice in the Description page of Project Settings.


#include "QxSpatialUpscaler.h"

#include "ScreenPass.h"


// FScreenPassTexture FQxSpatialUpscaler::AddPasses(FRDGBuilder& GraphBuilder,
//                                                  const FViewInfo& View,
//                                                  const FInputs& PassInputs) const
// {
//     UE_LOG(LogTemp, Warning, TEXT("Test"));
//     FScreenPassTexture FinalOut = PassInputs.SceneColor;
//     return MoveTemp(FinalOut);
// }

FScreenPassTexture FQxSpatialUpscaler::AddPasses(FRDGBuilder& GraphBuilder, const FViewInfo& View,
    const FInputs& PassInputs) const
{
    UE_LOG(LogTemp, Warning, TEXT("Test"));

	
    // Data->UpscaleTexture.Texture = GraphBuilder.CreateTexture(Data->FSROutputTextureDesc, TEXT("FFX-FSR-Output"), ERDGTextureFlags::MultiFrame);
    // Data->UpscaleTexture.ViewRect = View.UnscaledViewRect;

    FScreenPassTexture FinalOut = FScreenPassTexture(PassInputs.SceneColor);
    
    {
        
        FRDGTextureDesc TexDesc = PassInputs.SceneColor.Texture->Desc;
        TexDesc.Reset();
        TexDesc.Extent = View.UnscaledViewRect.Max;
        // TexDesc.Format = PassInputs.SceneColor.Texture->Desc.Format;
        TexDesc.ClearValue = FClearValueBinding::Black;
        TexDesc.Flags = TexCreate_ShaderResource | TexCreate_UAV | TexCreate_RenderTargetable;
    
        FRDGTextureRef TestTex = GraphBuilder.CreateTexture(TexDesc, TEXT("TempTexture"),
            ERDGTextureFlags::MultiFrame);
    
    
        AddCopyTexturePass(
            GraphBuilder,
            PassInputs.SceneColor.Texture,
            TestTex,
            FIntPoint::ZeroValue,
            FIntPoint::ZeroValue,
            PassInputs.SceneColor.ViewRect.Size()
            );
        FinalOut.Texture = TestTex;
        FinalOut.ViewRect = View.UnscaledViewRect;
    }


    
    
    return MoveTemp(FinalOut);
}

ISpatialUpscaler* FQxSpatialUpscaler::Fork_GameThread(const FSceneViewFamily& ViewFamily) const
{
    // 	// the object we return here will get deleted by UE4 when the scene view tears down, so we need to instantiate a new one every frame.
    // 没帧都需要创建up scaler
    return new FQxSpatialUpscaler();
}
