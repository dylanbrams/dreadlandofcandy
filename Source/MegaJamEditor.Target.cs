using UnrealBuildTool;

public class MegaJamEditorTarget : TargetRules
{
	public MegaJamEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("MegaJam");
	}
}
