#pragma once
#include "ScreenPass.h"


namespace QxRenderUtils
{
	
	/**
	 * @brief 使用kawase blur的方法进行模糊，先进行BlureSteps次downsample再进行BlurSteps次upsample
	 * @param GraphBuilder 
	 * @param ViewInfo 
	 * @param InputTexture 
	 * @param BlurSteps 
	 * @return 
	 */
	FScreenPassTexture RenderKawaseBlur(
		FRDGBuilder& GraphBuilder,
		const FViewInfo& ViewInfo,
		const FScreenPassTexture& InputTexture,
		int32 BlurSteps);
}
