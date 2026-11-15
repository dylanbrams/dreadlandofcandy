using UnrealBuildTool;

public class MegaJam : ModuleRules
{
	public MegaJam(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// File-based Slate fonts must remain available outside pak containers.
		RuntimeDependencies.Add("$(ProjectDir)/Content/UI/Fonts/*.ttf", StagedFileType.NonUFS);
		RuntimeDependencies.Add("$(ProjectDir)/Content/UI/Fonts/*.txt", StagedFileType.NonUFS);
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "GameplayAbilities", "GameplayTags", "GameplayTasks", "EnhancedInput", "UMG", "Slate", "SlateCore", "ProceduralMeshComponent", "HTTP", "Json", "Niagara" });
	}
}
