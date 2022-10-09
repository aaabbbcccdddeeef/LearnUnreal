#pragma once

#include "Modules/ModuleManager.h"

// 这个模块主要用来学习UE4 C++核心的部分，多线程、序列化、uobject等等

DECLARE_LOG_CATEGORY_EXTERN(LearnCore, All, All);

class FLearnCore : public IModuleInterface
{
	public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;
};