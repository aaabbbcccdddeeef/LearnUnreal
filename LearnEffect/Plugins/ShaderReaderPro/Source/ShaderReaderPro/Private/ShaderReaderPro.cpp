// Copyright 2020 Jorge CR. All Rights Reserved.


#include "ShaderReaderPro.h"
#include "../Public/MaterialExpressionShaderReader.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#endif
#include "ShaderReaderSettings.h"


#define LOCTEXT_NAMESPACE "FShaderReaderProModule"

DEFINE_LOG_CATEGORY(ShaderReaderLog);

void FShaderReaderProModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
		// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_EDITOR
	FOnShaderFileUpdated OnFileUpdated = FOnShaderFileUpdated::CreateRaw(this, &FShaderReaderProModule::OnFileUpdated);
	FileScanner = new FShaderReaderFileScanner(OnFileUpdated);

	//~ Register the settings class
	RegisterShaderReaderSettings();

	//~ Register the folders inside the settings
	RegisterShaderFolders();
#endif
}

void FShaderReaderProModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if WITH_EDITOR
	if (FileScanner)
	{
		FileScanner->Stop();
	}
#endif
}

#if WITH_EDITOR
FShaderReaderFileScanner* FShaderReaderProModule::FileScanner = nullptr;

FShaderReaderFileScanner* FShaderReaderProModule::GetFileScanner()
{
	return FileScanner;
}

void FShaderReaderProModule::OnFileUpdated(const FString& File)
{
	FString Code;
	TArray<FString> Variables;
	TArray<FCustomDefine> Defines;
	TArray<FString> Includes;

	//~ When the file is updated we send the info to each node, but we parsed only once
	if (UMaterialExpressionShaderReader::ParseFile(File, Code, Variables, Defines, Includes))
	{
		for (TObjectIterator<UMaterialExpressionShaderReader> It; It; ++It)
		{
			if (!It->IsPendingKill() && File == It->GetFileLocation())
			{
				It->UpdateNode(Code, Variables, Defines, Includes);
			}
		}
	}
}

void FShaderReaderProModule::RegisterShaderReaderSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		//~ Get the category container
		TSharedPtr<ISettingsContainer> SettingsContainer = SettingsModule->GetContainer("Project");

		//~ Register new category in container
		TSharedPtr<ISettingsSection> SettingsSection = SettingsModule->RegisterSettings("Project", "Bear Teacher", "Shader Reader",
			LOCTEXT("RuntimeSettingsName", "Shader Reader"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the Shader Reader plugin"),
			GetMutableDefault<UShaderReaderSettings>());

		//~ Register save handler
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FShaderReaderProModule::OnShaderReaderSettingsSaved);
			UE_LOG(ShaderReaderLog, Log, TEXT("Register Shader Reader Settings"));
		}
	}
}

bool FShaderReaderProModule::OnShaderReaderSettingsSaved()
{
	UShaderReaderSettings* Settings = GetMutableDefault<UShaderReaderSettings>();
	if (Settings)
	{
		Settings->SaveConfig();
		UE_LOG(ShaderReaderLog, Log, TEXT("Shader Reader Settings Saved"));
		return true;
	}
	return false;
}

void FShaderReaderProModule::RegisterShaderFolders()
{
	UShaderReaderSettings* Settings = GetMutableDefault<UShaderReaderSettings>();
	if (Settings)
	{
		for (auto& Path : Settings->ShaderFolderPaths)
		{
			FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), Path.Path);
			if (!FPaths::DirectoryExists(ShaderDirectory))
			{
				IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
				PlatformFile.CreateDirectoryTree(*ShaderDirectory);
			}
			AddShaderSourceDirectoryMapping(TEXT("/") + Path.VirtualPath, ShaderDirectory);
		}
	}
}

#endif

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShaderReaderProModule, ShaderReaderPro)