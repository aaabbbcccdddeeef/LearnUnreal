// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class QxColorCorrection : ModuleRules
{
	public QxColorCorrection(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"RenderCore",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"MovieSceneCapture",
				"RenderCore",
				"RHI",
				"Slate",
				"SlateCore",
				"Renderer",
				"Projects",
				"RenderCore",
			}
			);
		
		PrivateIncludePaths.AddRange(new string[]
		{
			EngineDirectory + "/Source/Runtime/Renderer/Private"		
		});
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.Add("UnrealEd");
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
