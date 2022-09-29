using UnrealBuildTool;
 
public class QxClippedStaticMesh : ModuleRules
{
	public QxClippedStaticMesh(ReadOnlyTargetRules Target) : base(Target)
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
        PrivateDependencyModuleNames.AddRange(
	        new string[]
	        {
		       
		        // ... add private dependencies that you statically link with here ...	
	        }
        );
 
		// PublicIncludePaths.AddRange(new string[] {"QxClippedStaticMesh/Public"});
		PrivateIncludePaths.AddRange(new string[] {"QxClippedStaticMesh/Private"});
		PrivateIncludePaths.AddRange(new string[]
		{
			EngineDirectory + "/Source/Runtime/Renderer/Private",
			EngineDirectory + "/Source/Runtime/Engine/Private"		
		});
		OptimizeCode = CodeOptimization.Never;
	}
}