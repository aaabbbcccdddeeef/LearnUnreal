// Copyright 2020 Jorge CR. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "ShaderReaderSettings.generated.h"

USTRUCT(BlueprintType)
struct FShaderPath
{
	GENERATED_BODY()

public:
	/*Virtual path that will be use when #include file*/
	UPROPERTY(EditAnywhere, Category = "Shader Reader")
		FString VirtualPath;
	
	/* Path for the new shader directoy*/
	UPROPERTY(EditAnywhere, Category = "Shader Reader")
		FString Path;

	FShaderPath(FString InVirtualPath, FString InPath)
	{
		VirtualPath = InVirtualPath;
		Path = InPath;
	}

	FShaderPath(){}

};

/**
 * 
 */
 UCLASS(Config = ShaderReaderSettings, defaultconfig)
class SHADERREADERPRO_API UShaderReaderSettings : public UObject
{
	GENERATED_BODY()

public:
	UShaderReaderSettings();

	/* List for all the shader directory to be registered*/
	UPROPERTY(EditAnywhere, Category = "Shader Reader")
		TArray<FShaderPath> ShaderFolderPaths;
};
