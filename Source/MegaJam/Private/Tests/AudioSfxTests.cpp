#include "Misc/AutomationTest.h"
#include "Game/JamSfx.h"
#include "Sound/SoundBase.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamAudioFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAudioSfxTable, "Jam.Audio.Sfx.Table", JamAudioFlags)
bool FJamAudioSfxTable::RunTest(const FString&)
{
	// The RELEASE_PLAN R1 event list: every one has a row.
	static const TCHAR* Required[] = { TEXT("Sword_Raise"), TEXT("Hero_Inhale"), TEXT("Lecture_Impact"), TEXT("FormLine_Contact"),
		TEXT("DisappointedSigh"), TEXT("LastJudgment_Contact"), TEXT("Flustered"), TEXT("Pacify"), TEXT("Interact_Complete"),
		TEXT("Bubble_Pop"), TEXT("Clapper"), TEXT("Cut_Horn"), TEXT("Retake_Rewind"), TEXT("Stage_Complete"),
		TEXT("Ledger_Deduction"), TEXT("Coin_Trickle"), TEXT("LevelUp"), TEXT("UI_Click"), TEXT("UI_Back"),
		TEXT("UI_Unavailable"), TEXT("Budget_Warning"), TEXT("Budget_Empty"), TEXT("Backlot_Ambience_Loop"), TEXT("Quest_Accept") };
	for (const TCHAR* Event : Required) { TestNotNull(FString::Printf(TEXT("row for %s"), Event), JamSfx::Find(Event)); }

	TSet<FName> Seen;
	for (const FJamSfxCue& C : JamSfx::Table())
	{
		TestFalse(FString::Printf(TEXT("%s once"), *C.Event.ToString()), Seen.Contains(C.Event));
		Seen.Add(C.Event);
		TestTrue(FString::Printf(TEXT("%s volume"), *C.Event.ToString()), C.Volume > 0.f && C.Volume <= 1.5f);
		TestTrue(TEXT("path"), JamSfx::AssetPath(C.Clip).StartsWith(TEXT("/Game/Audio/SFX/SFX_")));
	}
	TestEqual(TEXT("asset path"), JamSfx::AssetPath(TEXT("Clapper")), FString(TEXT("/Game/Audio/SFX/SFX_Clapper.SFX_Clapper")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamAudioSfxAssets, "Jam.Audio.Sfx.Assets", JamAudioFlags)
bool FJamAudioSfxAssets::RunTest(const FString&)
{
	// Every clip the table names is imported (Scripts/import_audio.py).
	for (const FJamSfxCue& C : JamSfx::Table())
	{
		TestNotNull(FString::Printf(TEXT("clip %s"), *C.Clip.ToString()), LoadObject<USoundBase>(nullptr, *JamSfx::AssetPath(C.Clip)));
	}
	return true;
}

#endif
