using UnrealBuildTool;
 
public class QxCustomUpscaler : ModuleRules
{
	public QxCustomUpscaler(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add other public dependencies that you statically link with here ...
				"Core", "CoreUObject", "Engine",
				"RHI", "Renderer", "RenderCore", "Projects"
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
 
		// PublicIncludePaths.AddRange(new string[] {"QxCustomUpscaler/Public"});
		PrivateIncludePaths.AddRange(new string[] {"QxCustomUpscaler/Private"});
		
		OptimizeCode = CodeOptimization.Never;
	}
}