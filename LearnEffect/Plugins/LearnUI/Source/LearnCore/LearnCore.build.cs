using UnrealBuildTool;
 
public class LearnCore : ModuleRules
{
	public LearnCore(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
 
		PublicIncludePaths.AddRange(new string[] {"LearnCore/Public"});
		PrivateIncludePaths.AddRange(new string[] {"LearnCore/Private"});
	}
}