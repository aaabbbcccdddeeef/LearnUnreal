// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class QxRenderSeries : ModuleRules
{
	public QxRenderSeries(ReadOnlyTargetRules Target) : base(Target)
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
				// ... add other public dependencies that you statically link with here ...
				"Renderer",
                "RenderCore",
                "RHI",
                "Projects",
                "Slate",
                "UMG",
                "RHI"
            }

            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
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
		
		OptimizeCode = CodeOptimization.Never;
	}
}
