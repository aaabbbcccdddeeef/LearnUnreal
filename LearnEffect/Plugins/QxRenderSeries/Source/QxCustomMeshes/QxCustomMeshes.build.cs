using UnrealBuildTool;
 
public class QxCustomMeshes : ModuleRules
{
	public QxCustomMeshes(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Renderer",
				"RenderCore",
				"RHI",
				"Projects",
				// ... add private dependencies that you statically link with here ...	
			}
		);
 
		// PublicIncludePaths.AddRange(new string[] {"QxCustomMeshes/Public"});
		PrivateIncludePaths.AddRange(new string[] {"QxCustomMeshes/Private"});
		
		OptimizeCode = CodeOptimization.Never;
	}
}