// Fill out your copyright notice in the Description page of Project Settings.


#include "QxPPExt.h"

void UQxPPExt::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RegisterRenderCallbacks();
}

void UQxPPExt::Deinitialize()
{
	Super::Deinitialize();

	if (GuassuaDelegateHandle.IsValid())
	{
		GuassuaDelegateHandle.Reset();

		IRendererModule* RendererModule = FModuleManager::Get().GetModulePtr<IRendererModule>("Renderer");
		if (!RendererModule)
		{
			return;
		}
		RendererModule->RemovePostOpaqueRenderDelegate(GuassuaDelegateHandle);
	}
}

void UQxPPExt::RenderQxGuassianBlur(FPostOpaqueRenderParameters& InParameters)
{
	
}

void UQxPPExt::RegisterRenderCallbacks()
{
	IRendererModule* RendererModule = FModuleManager::Get().GetModulePtr<IRendererModule>("Renderer");
	if (!RendererModule)
	{
		return;
	}

	
	GuassuaDelegateHandle = RendererModule->RegisterPostOpaqueRenderDelegate(
		FPostOpaqueRenderDelegate::CreateUObject(this, &UQxPPExt::RenderQxGuassianBlur));
}
