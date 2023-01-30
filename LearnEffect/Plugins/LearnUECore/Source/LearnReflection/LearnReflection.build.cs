using UnrealBuildTool;
 
public class LearnReflection : ModuleRules
{
	public LearnReflection(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
 
		PrivateIncludePaths.AddRange(new string[] {"LearnReflection/Private"});
	}
}