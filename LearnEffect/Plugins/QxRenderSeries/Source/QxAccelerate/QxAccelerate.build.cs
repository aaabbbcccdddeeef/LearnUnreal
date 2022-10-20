using UnrealBuildTool;
 
public class QxAccelerate : ModuleRules
{
	public QxAccelerate(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
 
		PrivateIncludePaths.AddRange(new string[] {"QxAccelerate/Private"});
	}
}