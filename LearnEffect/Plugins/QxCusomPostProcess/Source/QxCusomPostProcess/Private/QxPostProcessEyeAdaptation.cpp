#include "QxPostProcessEyeAdaptation.h"

#include "PixelShaderUtils.h"
#include "QxBloomSceneViewExtension.h"
#include "QxCommonShaders.h"
#include "QxLensFlareAsset.h"
#include "QxPostprocessSubsystem.h"
#include "Interfaces/IPluginManager.h"

#include "RenderGraph.h"
#include "ScreenPass.h"
#include "SystemTextures.h"
#include "PostProcess/PostProcessMaterial.h"


namespace
{
	// 这个作为basic eye adaptation ps/cs的基类，用来放一些公共参数，和模板方法
	class FQxBasicEyeAdaptationShader : public FGlobalShader
	{
	public:
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
			SHADER_PARAMETER_STRUCT(FQxEyeAdaptationParameters, QxEyeAdaptation)
			SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, Color)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, ColorTexture)
			// SHADER_PARAMETER_RDG_TEXTURE(Texture2D, EyeAdaptationTexture)
			SHADER_PARAMETER_SAMPLER(SamplerState, ColorTextureSampler)
		END_SHADER_PARAMETER_STRUCT()

		static const EPixelFormat OutputFormat = PF_A32B32G32R32F;

		static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters,
			FShaderCompilerEnvironment& OutEnvironment)
		{
			FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
			OutEnvironment.SetRenderTargetOutputFormat(0, OutputFormat);
		}

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}

		FQxBasicEyeAdaptationShader() = default;
		FQxBasicEyeAdaptationShader(const CompiledShaderInitializerType& Initializer)
			: FGlobalShader(Initializer)
		{}
	};

	class FQxBasicEyeAdaptationPS : public FQxBasicEyeAdaptationShader
	{
		using Super = FQxBasicEyeAdaptationShader;
	public:
		DECLARE_GLOBAL_SHADER(FQxBasicEyeAdaptationPS);
		SHADER_USE_PARAMETER_STRUCT(FQxBasicEyeAdaptationPS, Super);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_STRUCT_INCLUDE(Super::FParameters, Base)
			RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()
	};
	// "/QxPPShaders/QxNormalDownSample.usf", "MainPS",
	IMPLEMENT_GLOBAL_SHADER(FQxBasicEyeAdaptationPS, "/QxPPShaders/QxEyeAdaption.usf", "QxBasicEyeAdaptation", SF_Pixel);

	
	float GetExposureCompensationFromSetting(const FViewInfo& View)
	{
		const FPostProcessSettings& Settings = View.FinalPostProcessSettings;

		// This scales the average luminance AFTER it gets clamped, affecting the exposure value directly.
		// Setting Bias的值是在2^n的空间中的
		float AutoExposureBias = Settings.AutoExposureBias;

		// 不考虑移动平台
		return FMath::Pow(2.f, AutoExposureBias);
	}

	// LastAverageSceneLuminance 是[0-1]的值,0表示无效
	float GetExposureCompenstationFromCurve(const FViewInfo& View, float LastAverageSceneLuminance)
	{
		const FPostProcessSettings& Settings = View.FinalPostProcessSettings;

		const float LuminanceMax = 1.f; // 不考虑extend 的问题

		float AutoExposureBias = 0.f;
		if (Settings.AutoExposureBiasCurve)
		{
			if (LastAverageSceneLuminance > 0)
			{
				// We need the Log2(0.18) to convert from average luminance to saturation luminance
				// 从0-1空间变换到log2空间
				const float LuminanceEV100 = LuminanceToEV100(LuminanceMax, LastAverageSceneLuminance)
					+ FMath::Log2(1.f / 0.18f);
				AutoExposureBias += Settings.AutoExposureBiasCurve->GetFloatValue(LuminanceEV100);
			}
		}
		return FMath::Pow(2.f, AutoExposureBias);
	}

	// 这个方法中现在为了方便直接用UE4 的post process setting得到auo exposure的设置先
	FQxEyeAdaptationParameters GetQxEyeAdaptationParams(
		const FViewInfo& ViewInfo,
		const UQxBloomFlareAsset* PPSettings,
		float LastAverageSceneLuminance)
	{
		const FPostProcessSettings& Settings = ViewInfo.FinalPostProcessSettings;
		
		const float LuminanceMax = 1.0f;

		// 这2个变量clamp scene color计算得到的平均亮度，我们先计算white point,再得到average grey point
		// 例如whit point是1.0, 中灰就是0.18
		float MinWhitePointLuminance = 1.f;
		float MaxWhitePointLuminance = 1.f;

		float ExposureCompensationSettings = GetExposureCompensationFromSetting(ViewInfo);

		float ExposureCompensationCurve = GetExposureCompenstationFromCurve(ViewInfo, LastAverageSceneLuminance);

		const float HistogramLogMax = Settings.HistogramLogMax;
		const float HistogramLogMin = FMath::Min(Settings.HistogramLogMin, Settings.HistogramLogMax - 1.f);
		const float HistogramLogDelta = HistogramLogMax - HistogramLogMin;
		const float HistogramScale = 1.f / HistogramLogDelta;
		const float HistogramBias = -HistogramLogMin * HistogramScale;
		
		// 中灰的值是0.18
		const float kMiddleGrey = 0.18f; 

		// 调整曝光补偿到中灰
		const float GreyMult = kMiddleGrey;

		MinWhitePointLuminance = Settings.AutoExposureMinBrightness;
		MaxWhitePointLuminance = Settings.AutoExposureMaxBrightness;
		MinWhitePointLuminance = FMath::Min(MinWhitePointLuminance, MaxWhitePointLuminance);

		// const float HistogramLogDelta = 

		// If the white point luminance is 1.0, then the middle grey luminance should be 0.18.
		const float MinAverageLuminance  = MinWhitePointLuminance * kMiddleGrey;
		const float MaxAverageLuminance = MaxWhitePointLuminance * kMiddleGrey;

		// histogram mode时,luminanceMin设置到histogram的下限。basic mode时，用一个极小值
		const float LuminanceMin = (PPSettings->AutoExposureMethod == EQxAutoExposureMethod::AEM_Basic) ?
			0.0001f : FMath::Exp2(HistogramLogMin); 

		const FTextureRHIRef MeterMask = Settings.AutoExposureMeterMask ?
										Settings.AutoExposureMeterMask->Resource->TextureRHI:
										GWhiteTexture->TextureRHI;
		
		FQxEyeAdaptationParameters Parameters;
		Parameters.MinAverageLuminance = MinAverageLuminance;
		Parameters.MaxAverageLuminance = MaxWhitePointLuminance;
		Parameters.ExposureCompensationSettings = ExposureCompensationSettings;
		Parameters.ExposureCompensationCuve = ExposureCompensationCurve;
		Parameters.DeltaWorldTime = ViewInfo.Family->DeltaWorldTime;
		Parameters.HistogramScale = HistogramScale;
		Parameters.HistogramBias = HistogramBias;
		Parameters.LuminanceMin = LuminanceMin;
		Parameters.GreyMult = GreyMult;
		Parameters.LuminanceMax = LuminanceMax;
		Parameters.MeterMaskTexture = MeterMask;
		Parameters.MeterMaskTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		return Parameters;
	}

	// 计算scaled and biased luma for input scene color and puts it in the alpha channel
	class FQxBasicEyeAdaptationSetupPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxBasicEyeAdaptationSetupPS);
		SHADER_USE_PARAMETER_STRUCT(FQxBasicEyeAdaptationSetupPS, FGlobalShader)

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
			SHADER_PARAMETER_STRUCT(FQxEyeAdaptationParameters, QxEyeAdaptation)
			SHADER_PARAMETER_RDG_TEXTURE(Texture2D, ColorTexture)
			SHADER_PARAMETER_SAMPLER(SamplerState, ColorTextureSampler)
			RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
		}
	};
	IMPLEMENT_GLOBAL_SHADER(FQxBasicEyeAdaptationSetupPS, "/QxPPShaders/QxEyeAdaption.usf", "QxBasicEyeAdaptationSetupPS", SF_Pixel);
	
}


FScreenPassTexture FQxBloomSceneViewExtension::RenderEyeAdaptation(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& ViewInfo,
	const FPostProcessMaterialInputs& PostProcessMaterialInput,
	const FScreenPassTexture& InHalfTexture)
{
	RDG_EVENT_SCOPE(GraphBuilder, "QxEyeAdaptation");
	const EQxAutoExposureMethod AutoExposureMethod = QxPostprocessSubsystem->GetBloomSettingAsset()->AutoExposureMethod;
	if (AutoExposureMethod == EQxAutoExposureMethod::AEM_None)
	{
		return PostProcessMaterialInput.GetInput(EPostProcessMaterialInput::SceneColor);
	}
	
	// ResEyeTexture.ViewRect
	// 默认直接用一个white dummy
	FRDGTextureRef EyeAdaptationTexture = GraphBuilder.RegisterExternalTexture(
			GSystemTextures.WhiteDummy
		);
	const FQxEyeAdaptationParameters EyeAdaptationParameters =
		GetQxEyeAdaptationParams(ViewInfo,
			QxPostprocessSubsystem->GetBloomSettingAsset(),
			LastAverageSceneLuminance);
	if (AutoExposureMethod == EQxAutoExposureMethod::AEM_Basic)
	{
		FQxDownSampleChain SceneDownSampleChain;

		const bool bLogLumaInAlpha = true;
		SceneDownSampleChain.Init(
			GraphBuilder,
			ViewInfo,
			EyeAdaptationParameters,
			InHalfTexture,
			EQxDownampleQuality::High,
			bLogLumaInAlpha);

		EyeAdaptationTexture = AddQxBasicEyeAdaptationPass(GraphBuilder,
			ViewInfo,
			EyeAdaptationParameters,
			SceneDownSampleChain.GetLastTexture());
	}

	FScreenPassTexture ResEyeTexture;
	ResEyeTexture.Texture = EyeAdaptationTexture;
	ResEyeTexture.ViewRect = FIntRect(
		0, 0,
		1, 1
		);
	return ResEyeTexture;
}

FRDGTextureRef FQxBloomSceneViewExtension::AddQxBasicEyeAdaptationPass(
	FRDGBuilder& GraphBuilder,
	const FViewInfo& ViewInfo,
	const FQxEyeAdaptationParameters& EyeAdaptationParameters,
	FScreenPassTexture SceneColor)
{
	const FScreenPassTextureViewport SceneColorViewport(SceneColor);
	const bool bUseComputePass = false;
	
	// 创建要输出的texture
	// 现在先不考虑随时间变化的自动曝光，这个texture可以用graphbuilder创建
	FRDGTextureRef ResEyeAdaptationTexture;

	FRDGTextureDesc TexDesc;
	{
		TexDesc.Reset();
		TexDesc.Dimension = ETextureDimension::Texture2D;
		TexDesc.ClearValue = FClearValueBinding::None;
		TexDesc.Flags |= (TexCreate_ShaderResource | TexCreate_RenderTargetable);
		if (bUseComputePass)
		{
			TexDesc.Flags |= TexCreate_UAV  ;
		}
		TexDesc.Format = PF_A32B32G32R32F;
		TexDesc.Extent = FIntPoint(1, 1);;
	}
	ResEyeAdaptationTexture = GraphBuilder.CreateTexture(TexDesc, TEXT("QxEyeAdaptationTexture"));

	
	FQxBasicEyeAdaptationShader::FParameters PassBaseParams;
	PassBaseParams.View = ViewInfo.ViewUniformBuffer;
	PassBaseParams.QxEyeAdaptation = EyeAdaptationParameters;
	PassBaseParams.Color = GetScreenPassTextureViewportParameters(SceneColorViewport);
	PassBaseParams.ColorTexture = SceneColor.Texture;
	PassBaseParams.ColorTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
	// PassBaseParams.EyeAdaptationTexture = 
	
	FQxBasicEyeAdaptationPS::FParameters* PassParams = GraphBuilder.AllocParameters<FQxBasicEyeAdaptationPS::FParameters>();
	PassParams->RenderTargets[0] = FRenderTargetBinding(ResEyeAdaptationTexture, ERenderTargetLoadAction::ENoAction);
	PassParams->Base = PassBaseParams;

	
	TShaderMapRef<FQxBasicEyeAdaptationPS> PixelShader(ViewInfo.ShaderMap);

	const FScreenPassTextureViewport OutputViewport(ResEyeAdaptationTexture);
	AddDrawScreenPass(
		GraphBuilder,
		RDG_EVENT_NAME("QxBasicEyeAdaptation"),
		ViewInfo,
		OutputViewport,
		OutputViewport,
		PixelShader,
		PassParams
		);

	return ResEyeAdaptationTexture;
}


namespace QxRenderUtils
{
	FScreenPassTexture QxRenderUtils::AddBasicEyeAdaptationSetupPass(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& View,
		const FQxEyeAdaptationParameters& EyeAdaptationParameters,
		FScreenPassTexture SceneColor)
	{
		check(SceneColor.IsValid());

		FRDGTextureDesc TexDesc = SceneColor.Texture->Desc;
		TexDesc.Reset();
		// 需要alpha 通道保存 log2 luma
		TexDesc.Format = PF_FloatRGBA;
		// TexDesc.Flags |= GFastVRamConfig.EyeAdaptation;

		FRDGTextureRef OutTexture = GraphBuilder.CreateTexture(TexDesc, TEXT("QxBasicAdaptationSetup"));

		const FScreenPassTextureViewport Viewport(SceneColor);

		FQxBasicEyeAdaptationSetupPS::FParameters* PassParams =
			GraphBuilder.AllocParameters<FQxBasicEyeAdaptationSetupPS::FParameters>();
		PassParams->QxEyeAdaptation = EyeAdaptationParameters;
		PassParams->ColorTexture = SceneColor.Texture;
		PassParams->ColorTextureSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		PassParams->RenderTargets[0] = FRenderTargetBinding(OutTexture,  ERenderTargetLoadAction::ENoAction);
		
		TShaderMapRef<FQxBasicEyeAdaptationSetupPS> PixelShader(View.ShaderMap);
		
		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("QxBasicEyeAdaptationSetup %dx%d", Viewport.Rect.Width(), Viewport.Rect.Width()),
			View,
			Viewport,
			Viewport,
			PixelShader,
			PassParams
			);

		 // OutputTexture;
		return FScreenPassTexture(OutTexture, SceneColor.ViewRect);
	}
}