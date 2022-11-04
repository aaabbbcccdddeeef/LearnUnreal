using UnrealBuildTool;
 
public class ZZMultiClipper : ModuleRules
{
	public ZZMultiClipper(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "RHI",
			"Renderer", "RenderCore"
		});
 
		PrivateIncludePaths.AddRange(new string[] {"ZZMultiClipper/Private"});

		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
	}
}