#pragma once
#include "ScreenPass.h"

namespace QxRenderUtils
{
	extern  FScreenPassTexture RenderKawaseBlur(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FScreenPassTexture& InputTexture,
		int32 BlurSteps);
}
