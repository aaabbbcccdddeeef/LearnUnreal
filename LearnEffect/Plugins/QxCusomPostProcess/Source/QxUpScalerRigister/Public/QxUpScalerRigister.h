#pragma once

#include "FQxUpScalerExtention.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(QxUpScalerRigister, All, All);

class FQxUpScalerRigister : public IModuleInterface
{
	public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FQxUpscalerExtention, ESPMode::ThreadSafe> QxUpscalerExtention;
};