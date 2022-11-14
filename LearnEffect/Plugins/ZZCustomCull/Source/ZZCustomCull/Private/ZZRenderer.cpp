#include "ZZRenderer.h"

#include "EngineModule.h"
#include "ZZCustomCulling.h"

FZZRenderer::FZZRenderer()
{
    ZZCustomCulling = MakeUnique<FZZCustomCulling>();
    GetRendererModule().RegisterCustomCullingImpl(ZZCustomCulling.Get());
}

FZZRenderer::~FZZRenderer()
{
    GetRendererModule().UnregisterCustomCullingImpl(ZZCustomCulling.Get());
    ZZCustomCulling.Reset();
}
