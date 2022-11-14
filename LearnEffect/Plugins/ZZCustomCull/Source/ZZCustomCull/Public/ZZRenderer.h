#pragma once
#include "CoreMinimal.h"

class FZZCustomCulling;
// 这里类主要通过引擎RenderModule的接口自定义一些渲染逻辑，包括自定义Culling等等
// 属于渲染线程
class FZZRenderer
{
public:
    FZZRenderer();
    ~FZZRenderer();

    TUniquePtr<FZZCustomCulling> ZZCustomCulling;
};
