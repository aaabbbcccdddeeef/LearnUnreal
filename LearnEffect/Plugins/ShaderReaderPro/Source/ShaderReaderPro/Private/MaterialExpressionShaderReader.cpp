// Copyright 2020 Jorge CR. All Rights Reserved.

#include "MaterialExpressionShaderReader.h"

#if WITH_EDITOR
#include "MaterialEditor/Public/MaterialEditingLibrary.h"
#endif
#include <Misc/FileHelper.h>
#include <Async/Async.h>
#include <Materials/MaterialExpressionAdd.h>
#include <ShaderReaderPro.h>
#include "Kismet/KismetStringLibrary.h"

#define LOCTEXT_NAMESPACE "MaterialExpressionShaderReader"

UMaterialExpressionShaderReader::UMaterialExpressionShaderReader(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Custom = nullptr;
	Code = TEXT("return 1;");
	NodeName = TEXT("Shader Reader");
	OutputType = CMOT_Float3;

#if WITH_EDITORONLY_DATA
	MenuCategories.Add(LOCTEXT("BearTeacher", "BearTeacher"));
	//MenuCategories.Add(LOCTEXT("Custom", "Custom"));
#endif

#if WITH_EDITORONLY_DATA
	bCollapsed = false;
#endif // WITH_EDITORONLY_DATA
	Custom = CreateDefaultSubobject<UMaterialExpressionCustom>(TEXT("CustomNode_0"));

	Update();
}



UMaterialExpressionShaderReader::~UMaterialExpressionShaderReader()
{

}

#if WITH_EDITOR
void UMaterialExpressionShaderReader::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property->GetFName().ToString() == "FilePath")
	{
		FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		File.FilePath = File.FilePath.Replace(*ProjectPath, TEXT(""));

		FString FullPath = GetFileLocation();
		if (FPaths::FileExists(FullPath))
		{
			FShaderReaderProModule::GetFileScanner()->RegisterFile(FullPath);
			FShaderReaderProModule::GetFileScanner()->UpdateImmidiatly(FullPath);
		}
	}

	// strip any spaces from input name
	FProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	if (PropertyThatChanged && PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(FCustomInput, InputName))
	{
		for (FCustomInput& Input : Inputs)
		{
			FString InputName = Input.InputName.ToString();
			if (InputName.ReplaceInline(TEXT(" "), TEXT("")) > 0)
			{
				Input.InputName = *InputName;
			}
		}
	}


	if (PropertyChangedEvent.MemberProperty && GraphNode && ContainExpression())
	{
		const FName PropertyName = PropertyChangedEvent.MemberProperty->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UMaterialExpressionCustom, Inputs)) //~ Expression are valid as long the material editor exist or compile manually the mat, this prevent the editor to crash
		{
			GraphNode->ReconstructNode();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR

void UMaterialExpressionShaderReader::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	if (FPaths::FileExists(GetFileLocation()))
	{
		FShaderReaderProModule::GetFileScanner()->RegisterFile(GetFileLocation());
	}
#endif
}

void UMaterialExpressionShaderReader::Update()
{
	if (!Custom)
	{
		Custom = NewObject<UMaterialExpressionCustom>(this, TEXT("CustomTransientNode1"));
	}

	Custom->Code = Code;
	Custom->OutputType = OutputType;
	Custom->Description = NodeName;
	Custom->Inputs = Inputs;
}

#if WITH_EDITOR
int32 UMaterialExpressionShaderReader::Compile(class FMaterialCompiler* Compiler, int32 OutputIndex)
{
	if (!Custom) return 0;
	Update();
	return Custom->Compile(Compiler, OutputIndex);
}

void UMaterialExpressionShaderReader::GetCaption(TArray<FString>& OutCaptions) const
{
	if (!Custom) return;
	Custom->GetCaption(OutCaptions);
	OutCaptions.Add(NodeName);
}

const TArray<FExpressionInput*> UMaterialExpressionShaderReader::GetInputs()
{
	TArray<FExpressionInput*> Result;
	for (int32 i = 0; i < Inputs.Num(); i++)
	{
		Result.Add(&Inputs[i].Input);
	}
	return Result;
}

FExpressionInput* UMaterialExpressionShaderReader::GetInput(int32 InputIndex)
{
	if (InputIndex < Inputs.Num())
	{
		return &Inputs[InputIndex].Input;
	}
	return NULL;
}

FName UMaterialExpressionShaderReader::GetInputName(int32 InputIndex) const
{
	if (!Custom) return NAME_None;

	if (InputIndex < Inputs.Num())
	{
		Custom->GetInputName(InputIndex);
		return Inputs[InputIndex].InputName;
	}
	return NAME_None;
}

uint32 UMaterialExpressionShaderReader::GetInputType(int32 InputIndex)
{
	return MCT_Unknown;
}

uint32 UMaterialExpressionShaderReader::GetOutputType(int32 OutputIndex)
{
	if (!Custom) return 0;
	return Custom->GetOutputType(OutputIndex);
}

bool UMaterialExpressionShaderReader::ParseFile(const FString& File, FString& OutCode, TArray<FString>& OutVariables, TArray<FCustomDefine>& OutDefines, TArray<FString>& OutIncludes)
{
	if (!FPaths::FileExists(File)) return false;

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *File)) return false;

	if (!Lines.Num()) return false;

	OutVariables.Empty();
	int32 CodeStart = -1;
	for (int32 i = 0; i < Lines.Num(); i++)
	{
		CodeStart++;
		if (AddDefines(Lines[i], OutDefines) )
		{
			continue;
		}

		if (AddIncludes(Lines[i], OutIncludes))
		{
			continue;
		}

		if ((Lines[i].Contains("float ") || Lines[i].Contains("float2 ") || Lines[i].Contains("Texture2D ") || Lines[i].Contains("float3 ") || Lines[i].Contains("float4 ")) && Lines[i].Contains(";"))
		{
			FString right;
			Lines[i].Split(" ", nullptr, &right);
			FString Variable;
			right.Split(";", &Variable, nullptr);
			if (!Variable.IsEmpty())
			{
				OutVariables.Add(Variable);
			}
		}
		else
		{
			break;
		}
	}

	OutCode = FString();
	for (int32 i = CodeStart; i < Lines.Num(); i++)
	{
		if (AddDefines(Lines[i], OutDefines))
		{
			continue;
		}

		if (AddIncludes(Lines[i], OutIncludes))
		{
			continue;
		}

		if (i == Lines.Num() - 1)
		{
			OutCode += Lines[i];
		}
		else
		{
			OutCode += Lines[i] + "\n";
		}
	}

	return (OutCode.IsEmpty()) ? false : true;
}

bool UMaterialExpressionShaderReader::AddDefines(const FString& Line, TArray<FCustomDefine>& OutDefines)
{
	if (Line.Contains("#define "))
	{
		FCustomDefine Define;
		FString Right;
		Line.Split(" ", nullptr, &Right);
		Right.Split(" ", &Define.DefineName, &Define.DefineValue);


		OutDefines.Add(Define);
		return true;
	}
	return false;
}

bool UMaterialExpressionShaderReader::AddIncludes(const FString& Line, TArray<FString>& OutIncludes)
{
	if (Line.Contains("#include "))
	{
		FString Include = "";
		Line.Split("#include ", nullptr, &Include);
		FString Value = FString();
		for (auto& ch : Include)
		{
			if (ch != '"')
			{
				Value += ch;
			}
		}

		OutIncludes.Add(Value);
		UE_LOG(LogTemp, Log, TEXT("%s"), *Value);
		return true;
	}
	return false;
}

bool UMaterialExpressionShaderReader::HasDifferentVariables(const TArray<FString>& InVariables)
{
	if (InVariables.Num() == Inputs.Num())
	{
		for (int32 i = 0; i < InVariables.Num(); i++)
		{
			bool bFound = false;

			for (auto& input : Inputs)
			{
				if (InVariables[i] == input.InputName.ToString())
				{
					bFound = true;
				}
			}

			if (!bFound)
			{
				return true;
			}
		}
		return false;
	}
	return true;
}

void UMaterialExpressionShaderReader::UpdateInputs(const TArray<FString>& InVariables)
{
	while (HasDifferentVariables(InVariables))
	{
		for (int32 i = 0; i < InVariables.Num(); i++)
		{
			bool bFound = false;

			for (auto& input : Inputs)
			{
				if (InVariables[i] == input.InputName.ToString())
				{
					bFound = true;
				}
			}

			if (!bFound)
			{
				FCustomInput Input;
				Input.InputName = FName(*InVariables[i]);
				Inputs.Add(Input);
			}
		}

		for (int32 i = 0; i < Inputs.Num(); i++)
		{
			bool bFound = false;

			for (auto& var : InVariables)
			{
				if (var == Inputs[i].InputName.ToString())
				{
					bFound = true;
				}
			}

			if (!bFound)
			{
				Inputs.RemoveAt(i);
			}
		}
	}
}

bool UMaterialExpressionShaderReader::ContainExpression()
{
	if (!Material)
	{
		return false;
	}
	
	for (auto& e : Material->Expressions)
	{
		if (e == this)
		{
			return true;
		}
	}

	return false;
}


FString UMaterialExpressionShaderReader::GetFileLocation()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir())  + File.FilePath;
}

void UMaterialExpressionShaderReader::UpdateNode(const FString& InCode, const TArray<FString>& InVariables, const TArray<FCustomDefine>& InDefines, const TArray<FString>& InIncludes)
{
	if (!ContainExpression()) return;

	bool bCompileMaterial = false;
	if (Code != InCode)
	{
		Code = InCode;
		Custom->Code = Code;
		bCompileMaterial = true;
	}

	if (!Inputs.Num())
	{
		for (auto& var : InVariables)
		{
			FCustomInput Input;
			Input.InputName = FName(*var);
			Inputs.Add(Input);
		}
		Custom->Inputs = Inputs;
		//~ Refresh node
		if (GraphNode && ContainExpression())
		{
			GraphNode->ReconstructNode();
		}
	}


	if (HasDifferentVariables(InVariables))
	{
		UpdateInputs(InVariables);
		Custom->Inputs = Inputs;
		//~ Refresh node
		if (GraphNode && ContainExpression())
		{
			GraphNode->ReconstructNode();
		}
	}

	if (InDefines.Num())
	{
		if (InDefines.Num() != Custom->AdditionalDefines.Num())
		{
			bCompileMaterial = true;
		}
		Custom->AdditionalDefines = InDefines;
	}

	if (InIncludes.Num())
	{
		if (InIncludes.Num() != Custom->IncludeFilePaths.Num())
		{
			bCompileMaterial = true;
		}
		Custom->IncludeFilePaths = InIncludes;
	}


	if (bCompileMaterial)
	{
		UMaterialEditingLibrary::RecompileMaterial(Material);
	}
}

void UMaterialExpressionShaderReader::EditFile()
{
	if (FPaths::FileExists(GetFileLocation()))
	{
		FPlatformProcess::LaunchFileInDefaultExternalApplication(*GetFileLocation());
	}
}

void UMaterialExpressionShaderReader::OpenFileInExplorer()
{
	if (FPaths::FileExists(GetFileLocation()))
	{
		FPlatformProcess::ExploreFolder(*GetFileLocation());
	}
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE