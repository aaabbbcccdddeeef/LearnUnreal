#include "QxTestRenderer.h"

#include "EngineModule.h"
#include "QxTestCulling.h"

FQxTestRenderer::FQxTestRenderer(): ThisModule(nullptr)
{
	RenderHandle = GetRendererModule().RegisterPostOpaqueRenderDelegate(
		FPostOpaqueRenderDelegate::CreateRaw(this, &FQxTestRenderer::Render));
	ThisModule = &FModuleManager::GetModuleChecked<FQyRenderSeriesModule>("QyRenderSeries");

	QxTestCulling = MakeUnique<FQxTestCulling>();
	GetRendererModule().RegisterCustomCullingImpl(QxTestCulling.Get());
}

FQxTestRenderer::~FQxTestRenderer()
{
	GetRendererModule().RemovePostOpaqueRenderDelegate(RenderHandle);
	QxTestCulling.Reset();
	RenderHandle.Reset();

	ThisModule = nullptr;
}

void FQxTestRenderer::Render(FPostOpaqueRenderParameters& InParameters)
{
	int test = 100;
}
