// Copyright 2020 Jorge CR. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ShaderReaderFileScanner.h"

DECLARE_LOG_CATEGORY_EXTERN(ShaderReaderLog, Log, All);

class FShaderReaderProModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#if WITH_EDITOR
private:
	//~ File scanner class module singleton
	static FShaderReaderFileScanner* FileScanner;

public:
	//~ Get Reference to this singleton class
	static FShaderReaderFileScanner* GetFileScanner();

private:
	/**
	 * Triggers when a file has new changes
	 *
	 * @param: File - The file containing the new changes
	*/
	void OnFileUpdated(const FString& File);

	//~ Register the custom shader reader settings into the project settings
	void RegisterShaderReaderSettings();

	//~ Handler for the new config file save
	bool OnShaderReaderSettingsSaved();

	//~ Function to registe the list from the settings
	void RegisterShaderFolders();
#endif
};
