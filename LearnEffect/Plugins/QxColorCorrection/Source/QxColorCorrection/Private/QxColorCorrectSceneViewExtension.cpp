// Fill out your copyright notice in the Description page of Project Settings.


#include "QxColorCorrectSceneViewExtension.h"

FQxColorCorrectSceneViewExtension::FQxColorCorrectSceneViewExtension(const FAutoRegister& AutoRegister,
	UQxColorCorrectSubsystem* InWorldSubsystem)
		: FSceneViewExtensionBase(AutoRegister)
{
	
}

void FQxColorCorrectSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder,
	const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	FSceneViewExtensionBase::PrePostProcessPass_RenderThread(GraphBuilder, View, Inputs);
}
