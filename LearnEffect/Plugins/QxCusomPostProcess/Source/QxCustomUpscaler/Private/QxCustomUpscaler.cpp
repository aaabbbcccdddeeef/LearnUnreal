#include "QxCustomUpscaler.h"

#include "SceneViewExtension.h"

DEFINE_LOG_CATEGORY(QxCustomUpscaler);

static TAutoConsoleVariable<int32> CVarQxUpscalerEnable(
	TEXT("r.QxUpscaler.Enable"), 1,
	TEXT("Wheather use qx upscaler "),
	ECVF_Default);

static TAutoConsoleVariable<int> CVarQxAutomationViewIndex(
	TEXT("r.QxRender.Automation.ViewIndex"),
	0,
	TEXT("Select which view to use with NGX DLSS image quality and performance automation. (default = 0). \n"),
	ECVF_RenderThreadSafe);

#define LOCTEXT_NAMESPACE "FQxCustomUpscaler"

class FQxUpscalerExtention : public FSceneViewExtensionBase
{
public:
	FQxUpscalerExtention(const FAutoRegister& AutoRegister)
		: FSceneViewExtensionBase(AutoRegister)
	{
		FSceneViewExtensionIsActiveFunctor IsActiveFunctor;
		IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
		{
			return CVarQxUpscalerEnable.GetValueOnAnyThread();
		};

		IsActiveThisFrameFunctions.Add(IsActiveFunctor);
	}

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) {}
	virtual void SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo) {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) {}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) final {}
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) final
	{
		int32 ViewIndex = CVarQxAutomationViewIndex.GetValueOnRenderThread();

		if (InViewFamily.Views.IsValidIndex(ViewIndex))
		{
			const FSceneView* View = InViewFamily.Views[ViewIndex];
			if (View->bIsGameView )
			{
				const FString AiAgentMarker = FString::Printf(TEXT("{\"camera\":{\"position\":{\"x\": %f, \"y\": %f, \"z\": %f},\"rotation\":{\"pitch\": %f, \"roll\": %f, \"yaw\": %f}}}"),
					View->ViewLocation.X, View->ViewLocation.Y, View->ViewLocation.Z,
					View->ViewRotation.Pitch, View->ViewRotation.Roll, View->ViewRotation.Yaw);

				static FColor ColorMarker = FColor::FromHex("0xA1A5E87");
				RHICmdList.PushEvent(*AiAgentMarker, ColorMarker);
				RHICmdList.PopEvent();
			}
		}
	}
};



void FQxCustomUpscaler::StartupModule()
{
	UE_LOG(QxCustomUpscaler, Warning, TEXT("QxCustomUpscaler module has been loaded"));
}

void FQxCustomUpscaler::ShutdownModule()
{
	UE_LOG(QxCustomUpscaler, Warning, TEXT("QxCustomUpscaler module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQxCustomUpscaler, QxCustomUpscaler)