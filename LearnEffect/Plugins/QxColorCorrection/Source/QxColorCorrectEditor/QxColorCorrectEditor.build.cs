using UnrealBuildTool;
 
public class QxColorCorrectEditor : ModuleRules
{
	public QxColorCorrectEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"RenderCore",
			"UnrealEd",
			"ColorCorrectRegions",
			"Slate",
			"SlateCore",
		});
 
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Projects",
				"UnrealEd",
				"Slate",
				"SlateCore",
				"QxColorCorrection",
				"PlacementMode",
			}
		);
		// PublicIncludePaths.AddRange(new string[] {"QxColorCorrectEditor/Public"});
		// PrivateIncludePaths.AddRange(new string[] {"QxColorCorrectEditor/Private"});
	}
}