#include "QxTestRenderer.h"

#include "EngineModule.h"
#include "QxRenderSubsystem.h"
#include "QxSSR.h"
#include "QxTestCulling.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "SceneRendering.h"
#include "SceneRenderTargetParameters.h"
#include "Engine/TextureRenderTarget2D.h"


namespace 
{
	struct FQxVertexPosNormalUV
	{
	public:
		FVector3f Position;
		FVector3f Normal;
		FVector2f UV;
	};
	
	// 一个slot传输结构
	class FTestVertexDeclaration : public FRenderResource
	{
	public:
		FVertexDeclarationRHIRef VertexDeclarationRHI;

		virtual  ~FTestVertexDeclaration() {  }

		void InitRHI() override
		{
			FVertexDeclarationElementList Elements;
			uint16 Stride = sizeof(FQxVertexPosNormalUV);
			Elements.Add(
				FVertexElement(
					0, STRUCT_OFFSET(FQxVertexPosNormalUV, Position),
					VET_Float3, 0, Stride));
			Elements.Add(
				FVertexElement(
					0, STRUCT_OFFSET(FQxVertexPosNormalUV, Normal),
					VET_Float3, 1, Stride));
			Elements.Add(
				FVertexElement(
					0, STRUCT_OFFSET(FQxVertexPosNormalUV, UV),
					VET_Float2, 2, Stride));
			VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
		}

		void ReleaseRHI() override
		{
			VertexDeclarationRHI.SafeRelease();
		}
	};

	// 多个slot 传输的layout
	class FTestVertexDeclaration2 : public FRenderResource
	{
	public:
		FVertexDeclarationRHIRef VertexDeclarationRHI;

		virtual  ~FTestVertexDeclaration2() {  }

		void InitRHI() override
		{
			FVertexDeclarationElementList Elements;
			
		}
	};


	class FQxCubeVertexBuffer : public FVertexBuffer
	{
	public:

		void InitRHI() override
		{
			constexpr  int32 VertexNum = 24;
			TResourceArray<FQxVertexPosNormalUV, VERTEXBUFFER_ALIGNMENT> Vertices;
			Vertices.SetNumUninitialized(VertexNum);

			FVector3f CubePositions[VertexNum] =
				{
						{1.00, -1.00, -1.00}, 
						{1.00,  1.00, -1.00}, 
						{1.00,  1.00,  1.00}, 
						{1.00, -1.00,  1.00}, 
						{1.00, -1.00,  1.00}, 
						{1.00,  1.00,  1.00}, 
						{1.00,  1.00, -1.00}, 
						{1.00, -1.00, -1.00}, 
						{1.00,  1.00, -1.00}, 
						{1.00,  1.00,  1.00}, 
						{1.00,  1.00,  1.00},
						{1.00,  1.00, -1.00},
						{1.00, -1.00, -1.00},
						{1.00, -1.00,  1.00},
						{-1.00, -1.00,  1.0},
						{-1.00, -1.00, -1.0},
						{1.00, -1.00,  1.00},
						{1.00,  1.00,  1.00},
						{-1.00,  1.00,  1.0},
						{-1.00, -1.00,  1.0},
						{-1.00, -1.00, -1.0},
						{-1.00,  1.00, -1.0},
						{1.00,  1.00, -1.00},
						{1.00, -1.00, -1.00},
				};
			// #TODO 初始化normals
			FVector3f Normals[VertexNum] =
				{
					 {1.00,  0.00,  0.00}, 
					 {1.00,  0.00,  0.00}, 
					 {1.00,  0.00,  0.00}, 
					 {1.00,  0.00,  0.00}, 
					 {1.00,  0.00,  0.00}, 
					 {1.00,  0.00,  0.00}, 
					 {1.00,  0.00,  0.00}, 
					 {1.00,  0.00,  0.00}, 
					 {0.00,  1.00,  0.00}, 
					 {0.00,  1.00,  0.00}, 
					 {0.00,  1.00,  0.00},
					 {0.00,  1.00,  0.00},
					 {0.00, -1.00,  0.00},
					 {0.00, -1.00,  0.00},
					 {0.00, -1.00,  0.00},
					 {0.00, -1.00,  0.00},
					 {0.00,  0.00,  1.00},
					 {0.00,  0.00,  1.00},
					 {0.00,  0.00,  1.00},
					 {0.00,  0.00,  1.00},
					 {0.00,  0.00, -1.00},
					 {0.00,  0.00, -1.00},
					 {0.00,  0.00, -1.00},
					 {0.00,  0.00, -1.00},
				};

			for (int32 i = 0; i < VertexNum; ++i)
			{
				CubePositions[i] = FVector3f(CubePositions[i].Z, CubePositions[i].X, CubePositions[i].Y);
				Normals[i] = FVector3f(Normals[i].Z, Normals[i].X, Normals[i].Y);
			}
			FVector2f UVs[VertexNum] =
				{
					 {0.00,  1.00},
					 {0.00,  0.00},
					 {1.00,  0.00},
					 {1.00,  1.00},
					 {0.00,  1.00},
					 {0.00,  0.00},
					 {1.00,  0.00},
					 {1.00,  1.00},
					 {0.00,  1.00},
					 {0.00,  0.00},
					 {1.00,  0.00},
					 {1.00,  1.00},
					 {0.00,  1.00},
					 {0.00,  0.00},
					 {1.00,  0.00},
					 {1.00,  1.00},
					 {0.00,  1.00},
					 {0.00,  0.00},
					 {1.00,  0.00},
					 {1.00,  1.00},
					 {0.00,  1.00},
					 {0.00,  0.00},
					 {1.00,  0.00},
					 {1.00,  1.00},
				};
			for (int32 i = 0; i < VertexNum; ++i)
			{
				Vertices[i].Position = CubePositions[i];
				Vertices[i].Normal = Normals[i];
				Vertices[i].UV = UVs[i];
			}
			

			FRHIResourceCreateInfo CreateInfo(TEXT("QxCubeVertices"), &Vertices);
			
			VertexBufferRHI = RHICreateVertexBuffer(
				Vertices.GetResourceDataSize(),
				BUF_Static,
				CreateInfo
				);
		}
	};

	class FQxCubeIndexBuffer : public FIndexBuffer
	{
	public:
		void InitRHI() override
		{
			const uint16 Indices[] =
				{
					0, 1, 2, 2, 3, 0,		// 右面(+X面)
				   4, 5, 6, 6, 7, 4,		// 左面(-X面)
				   8, 9, 10, 10, 11, 8,	// 顶面(+Y面)
				   12, 13, 14, 14, 15, 12,	// 底面(-Y面)
				   16, 17, 18, 18, 19, 16, // 背面(+Z面)
				   20, 21, 22, 22, 23, 20	// 正面(-Z面)
				};

			TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
			uint32 NumIndices = UE_ARRAY_COUNT(Indices);
			IndexBuffer.AddUninitialized(NumIndices);
			FMemory::Memcpy(
				IndexBuffer.GetData(),
				Indices,
				NumIndices * sizeof(uint16));

			FRHIResourceCreateInfo CreateInfo(TEXT("QxCubeIndexBuffer"), &IndexBuffer);
			IndexBufferRHI = RHICreateIndexBuffer(
				sizeof(uint16),
				IndexBuffer.GetResourceDataSize(),
				BUF_Static, CreateInfo);
		}
	};
	
	// 先测试用GlobalResource的方式分配,vb/ib等资源
#pragma region TestGlobalResources
	TGlobalResource<FTestVertexDeclaration> GQxVertexDeclartion_PosNormalUV;
	TGlobalResource<FQxCubeVertexBuffer> GQxCubeVertexBuffer;
	TGlobalResource<FQxCubeIndexBuffer> GQxCubeIndexBuffer;

#pragma endregion
	
	BEGIN_SHADER_PARAMETER_STRUCT(FQxTestParameters,)
	
		SHADER_PARAMETER(FLinearColor, TestColor)
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, ViewUniformBuffer)
		SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneTextureUniformParameters, SceneTextures)
		// RDG_TEXTURE_ACCESS(LightShaftOcclusionTexture, ERHIAccess::SRVGraphics)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
	
	class FQxTestVS: public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxTestVS);
		SHADER_USE_PARAMETER_STRUCT(FQxTestVS, FGlobalShader);

		// using FPermutationDomain = FDownsamplePermutationDomain;

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, ViewUniformBuffer)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			// return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
			return  true;
		}

		static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
		{
			FGlobalShader::ModifyCompilationEnvironment(Parameters,OutEnvironment);
		}
	};

	class FQxTestPS : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FQxTestPS);
		SHADER_USE_PARAMETER_STRUCT(FQxTestPS, FGlobalShader);


		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_STRUCT_INCLUDE(FQxTestParameters, TestParams)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
		{
			// return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
			return true;
		}

		static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
		{
			FGlobalShader::ModifyCompilationEnvironment(Parameters,OutEnvironment);
		}
	};

	IMPLEMENT_GLOBAL_SHADER(FQxTestVS, "/QxShaders/QxTestCube.usf", "MainVS", SF_Vertex);
	IMPLEMENT_GLOBAL_SHADER(FQxTestPS, "/QxShaders/QxTestCube.usf", "MainPS", SF_Pixel);
	
	void RenderTestGeometryWithRDG(
		FPostOpaqueRenderParameters& InParams,
		FTextureRenderTarget2DResource* RenderTargetResource)
	{
		FRDGBuilder& GraphicBuilder = *InParams.GraphBuilder;

		FRDGTextureRef testRT = GraphicBuilder.RegisterExternalTexture(
			CreateRenderTarget(RenderTargetResource->TextureRHI, TEXT("TestRT"))
			);

		FQxTestParameters* TestParameters = GraphicBuilder.AllocParameters<FQxTestParameters>();
		TestParameters->RenderTargets[0] = FRenderTargetBinding(testRT, ERenderTargetLoadAction::ENoAction);
		TestParameters->SceneTextures = InParams.SceneTexturesUniformParams;
		TestParameters->TestColor = FLinearColor::Blue;
		TestParameters->ViewUniformBuffer = InParams.View->ViewUniformBuffer;
		
		//#TODO 这里有些疑惑AddPass只能添加一个参数，对于vs/ps传不同参数应该怎样处理
		FQxTestVS::FParameters* VSParams = GraphicBuilder.AllocParameters<FQxTestVS::FParameters>();
		FQxTestPS::FParameters* PSParams = GraphicBuilder.AllocParameters<FQxTestPS::FParameters>();
		VSParams->ViewUniformBuffer = TestParameters->ViewUniformBuffer;
		PSParams->TestParams = *TestParameters;

		
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(InParams.View->GetFeatureLevel());

		TShaderMapRef<FQxTestVS> vertexShader(GlobalShaderMap);
		TShaderMapRef<FQxTestPS> pixelShader(GlobalShaderMap);

		// GraphicBuilder.AddPass(
		// 	RDG_EVENT_NAME("RenderTestGeometry"),
		// 	
		// 	);
		
		GraphicBuilder.AddPass(
			RDG_EVENT_NAME("RenderTestGeometry"),
			PSParams,
			ERDGPassFlags::Raster,
			[pixelShader, vertexShader,VSParams, PSParams](
				FRHICommandListImmediate& RHICmdListLambda)
			{
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdListLambda.ApplyCachedRenderTargets(GraphicsPSOInit);
				
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				// GraphicsPSOInit.DepthStencilState =
				// 	TStaticDepthStencilState<true, CF_DepthNearOrEqual>::GetRHI();
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false>::GetRHI();
				

				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GQxVertexDeclartion_PosNormalUV.VertexDeclarationRHI;//= GEmptyVertexDeclaration.VertexDeclarationRHI;
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = vertexShader.GetVertexShader(); //VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = pixelShader.GetPixelShader();//PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;

				SetGraphicsPipelineState(RHICmdListLambda, GraphicsPSOInit, 0);

				SetShaderParameters(RHICmdListLambda, pixelShader,
					pixelShader.GetPixelShader(), *PSParams);


				FQxTestVS::FParameters lVSParams;
				lVSParams.ViewUniformBuffer = PSParams->TestParams.ViewUniformBuffer;
				
				SetShaderParameters(RHICmdListLambda,vertexShader,
					vertexShader.GetVertexShader(), lVSParams);


				RHICmdListLambda.SetStreamSource(0, GQxCubeVertexBuffer.VertexBufferRHI, 0);
				RHICmdListLambda.DrawIndexedPrimitive(
					GQxCubeIndexBuffer.IndexBufferRHI,
					0,
					0,
					8,
					0,
					12,
					1
					);
			}
			);
	

	}

	void RenderTestGeometryWithCommandList(
		FRHICommandListImmediate& RHICmdList,
		FTextureRenderTarget2DResource* RenderTargetResource)
	{
		check(IsInRenderingThread());
		SCOPED_DRAW_EVENT(RHICmdList, TestGeometryPass);

		FTexture2DRHIRef testRTRhi  = RenderTargetResource->GetTextureRHI();
		RHICmdList.TransitionResource(ERHIAccess::RTV, testRTRhi);
		FRHIRenderPassInfo PassInfo(testRTRhi, ERenderTargetActions::DontLoad_Store);
		RHICmdList.BeginRenderPass(PassInfo, TEXT("TestGeometryPass"));
		{
			
		}
		RHICmdList.EndRenderPass();
	}
}


FQxTestRenderer::FQxTestRenderer(UQxRenderSubsystem* InQxRenderSubsystem)
	: QxRenderSubsystem(InQxRenderSubsystem)
{
	RenderHandle = GetRendererModule().RegisterPostOpaqueRenderDelegate(
		FPostOpaqueRenderDelegate::CreateRaw(this, &FQxTestRenderer::Render));
	// ThisModule = &FModuleManager::GetModuleChecked<FQyRenderSeriesModule>("QyRenderSeries");

	QxTestCulling = MakeUnique<FQxTestCulling>();
	GetRendererModule().RegisterCustomCullingImpl(QxTestCulling.Get());
}

FQxTestRenderer::~FQxTestRenderer()
{
	GetRendererModule().RemovePostOpaqueRenderDelegate(RenderHandle);
	GetRendererModule().UnregisterCustomCullingImpl(QxTestCulling.Get());
	QxTestCulling.Reset();
	RenderHandle.Reset();

	// ThisModule = nullptr;
}

void FQxTestRenderer::Render(FPostOpaqueRenderParameters& InParameters)
{
	// int test = 100;
	FRDGBuilder& GraphBuilder = *(InParameters.GraphBuilder);

	RDG_EVENT_SCOPE(GraphBuilder, "QxTestRenderer");

	UTextureRenderTarget2D* QxSSRExternal = QxRenderSubsystem->QxSSROutput;
	if (QxSSRExternal == nullptr)
	{
		return;
	}

	// try to resize SSRTex with ColorTexture
	{
		const FIntPoint ColorTexExtent =  InParameters.ColorTexture->Desc.Extent;
		if (FIntPoint(QxSSRExternal->SizeX, QxSSRExternal->SizeY) != ColorTexExtent)
		{
			AsyncTask(ENamedThreads::GameThread, [ColorTexExtent, QxSSRExternal]()
			{
				QxSSRExternal->ResizeTarget(ColorTexExtent.X, ColorTexExtent.Y);
				QxSSRExternal->UpdateResourceImmediate();
			});
		}
	}
	
	// QxRenderSubsystem->QxSSROutput->TextureReference.TextureReferenceRHI
	FTextureRenderTarget2DResource* SSRTexResource =
		static_cast<FTextureRenderTarget2DResource*>(QxSSRExternal->GetRenderTargetResource());
	if (!SSRTexResource)
		return;

	
	
	FRDGTextureRef SSR_RDG_Texture = GraphBuilder.RegisterExternalTexture(
		CreateRenderTarget(SSRTexResource->TextureRHI, TEXT("QxSSROutputTest")));
	FRHICopyTextureInfo CopyTextureInfo;
	CopyTextureInfo.Size = InParameters.ColorTexture->Desc.GetSize();
	// AddCopyTexturePass(GraphBuilder, InParameters.ColorTexture, SSR_RDG_Texture, CopyTextureInfo);

	// QxScreenSpaceReflection::RenderQxSSR(InParameters, SSR_RDG_Texture);

	
	FTextureRenderTarget2DResource* RenderTarget2DResource =
		static_cast<FTextureRenderTarget2DResource*>(QxRenderSubsystem->QxTestRT->GetRenderTargetResource());
	check(RenderTarget2DResource);

	RenderTestGeometryWithRDG(
		InParameters,
		RenderTarget2DResource
		);
	
}
