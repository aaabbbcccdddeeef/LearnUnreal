#pragma once
#include "RendererInterface.h"


namespace QxScreenSpaceReflection
{

	void RenderQxSSR(FPostOpaqueRenderParameters& InParameters, FRDGTextureRef ExternalSSROutput);
}
