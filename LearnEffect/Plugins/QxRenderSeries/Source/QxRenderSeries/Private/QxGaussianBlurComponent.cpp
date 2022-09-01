// Fill out your copyright notice in the Description page of Project Settings.


#include "QxGaussianBlurComponent.h"


// Sets default values for this component's properties
UQxGaussianBlurComponent::UQxGaussianBlurComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UQxGaussianBlurComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}

void UQxGaussianBlurComponent::OnRegister()
{
	Super::OnRegister();

#pragma region AddRenderCallbackToRenderModule
	IRendererModule* RendererModule = FModuleManager::Get().GetModulePtr<IRendererModule>("Renderer");
	if (RendererModule)
	{
		// OnGuassianBlurHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this,
		// 	&FQxRenderSeriesModule::PostResolveSceneColor_RenderThread);
		// FPostOpaqueRenderDelegate::CreateUObject(this)
		// RendererModule->RegisterPostOpaqueRenderDelegate()
	}
#pragma endregion
}

void UQxGaussianBlurComponent::OnUnregister()
{
	Super::OnUnregister();

#pragma region RemoveRenderCallbackFromRenderModule

#pragma endregion
}


// Called every frame
void UQxGaussianBlurComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

