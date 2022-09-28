using UnrealBuildTool;
 
public class QxUpScalerRigister : ModuleRules
{
	public QxUpScalerRigister(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine","Renderer", "QxCustomUpscaler"
		});
 
		PublicIncludePaths.AddRange(new string[] {"QxUpScalerRigister/Public"});
		PrivateIncludePaths.AddRange(new string[] {"QxUpScalerRigister/Private"});
	}
}