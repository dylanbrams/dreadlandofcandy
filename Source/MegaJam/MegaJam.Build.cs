using UnrealBuildTool;

public class MegaJam : ModuleRules
{
	public MegaJam(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "GameplayAbilities", "GameplayTags", "GameplayTasks", "EnhancedInput", "UMG", "Slate", "SlateCore", "ProceduralMeshComponent", "HTTP", "Json" });
	}
}
