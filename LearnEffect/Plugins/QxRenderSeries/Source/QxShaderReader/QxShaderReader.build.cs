using UnrealBuildTool;
 
public class QxShaderReader : ModuleRules
{
	public QxShaderReader(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"Renderer",
			"RenderCore",
			"RHI",
			"Projects",
		});
 
		PrivateIncludePaths.AddRange(new string[] {"QxShaderReader/Private"});
		
		if (Target.bBuildEditor)//we only want this to be included for editor builds but not packaged builds
		{
			PublicDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd",
					"MaterialEditor",
					"Settings",
				}
			);
		}
	}
}