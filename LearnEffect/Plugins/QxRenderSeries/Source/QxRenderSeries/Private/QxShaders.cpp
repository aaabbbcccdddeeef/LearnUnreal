// Fill out your copyright notice in the Description page of Project Settings.


#include "QxShaders.h"


IMPLEMENT_SHADER_TYPE(, FQxShaderTestVS, TEXT("/QxShaders/QxTest.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FQxShaderTestPS, TEXT("/QxShaders/QxTest.usf"), TEXT("MainPS"), SF_Pixel);


void FQxShaderTestPS::SetTestColor(FRHICommandList& RHICmdList, const FLinearColor& InTestColor)
{
	FRHIPixelShader* ps = RHICmdList.GetBoundPixelShader();
	SetShaderValue(RHICmdList, ps, TestColor, InTestColor);

}

void FQxShaderTestPS::SetTestTexture(FRHICommandList& RHICmdList, FTextureReferenceRHIRef InTextureRHI)
{
	FRHIPixelShader* ps = RHICmdList.GetBoundPixelShader();

	FRHISamplerState* testSamplerState = 
		TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	SetTextureParameter(RHICmdList, ps, TestTexture, TestTextureSampler, testSamplerState, InTextureRHI);

}
