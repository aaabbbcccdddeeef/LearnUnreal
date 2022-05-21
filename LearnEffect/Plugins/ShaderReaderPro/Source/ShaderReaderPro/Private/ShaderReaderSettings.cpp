// Copyright 2020 Jorge CR. All Rights Reserved.



#include "ShaderReaderSettings.h"

UShaderReaderSettings::UShaderReaderSettings()
{
	//~ Default value is the shader folder
	ShaderFolderPaths.Add(FShaderPath("Shaders","Shaders"));
}
