// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScreenPass.h"
#include "UObject/Object.h"
#include "QxPPExtSubSystem.generated.h"

/**
 * 这个类用来试验在插件中通过代码实现一些后期效果
 */
UCLASS()
class QXRENDERSERIES_API UQxPPExtSubSystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

private:


	void RenderQxGuassianBlur(FPostOpaqueRenderParameters& InParameters);

	// 通过注册这个函数到回调, 尝试用RDG的方式实现自定义后期，最终失败
	// 原因是这个回调执行在SceneRenderer的GraphBuilder中，通过回调参数无法拿到GraphBuilder，
	// 而如果新创建一个GraphBuilder，一个RDG执行过程中不能够插入新的RDG，所以最终失败
	void RenderQxGuassianBlur_RDG(FPostOpaqueRenderParameters& InParameters);
	
	void RenderQxGuassianOnePass_RDG(FRDGBuilder& GraphBuilder,
		FPostOpaqueRenderParameters& InParameters,
		FRDGTextureRef InRDGTexture,
		FRDGTextureRef TargetTexture,
		bool InIsHorizontal);

	// void RenderQxGuassianOnePass(
	// FPostOpaqueRenderParameters& InParameters,
	// FTextureRHIRef InRDGTexture,
	// FRDGTextureRef TargetTexture,
	// bool InIsHorizontal);

	// 请保证Texture资源的状态在这个函数调用前, 输入的纹理在响应的状态
	void RenderQxGuassianOnePass(FPostOpaqueRenderParameters& InParameters, FTextureRHIRef InTexture,
		FTextureRHIRef TargetTexture, bool InIsHorizontal);
	
	void RegisterRenderCallbacks();

	FDelegateHandle GuassuaDelegateHandle;
};
