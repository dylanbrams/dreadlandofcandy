#include "Misc/AutomationTest.h"
#include "UI/JamHudView.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamUiFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiHudTimecode, "Jam.UI.Hud.Timecode", JamUiFlags)
bool FJamUiHudTimecode::RunTest(const FString&)
{
	TestEqual(TEXT("zero"), JamHud::Timecode(0.f), FString(TEXT("00:00:00:00")));
	TestEqual(TEXT("negative clamps"), JamHud::Timecode(-3.f), FString(TEXT("00:00:00:00")));
	TestEqual(TEXT("half second is frame 12"), JamHud::Timecode(0.5f), FString(TEXT("00:00:00:12")));
	TestEqual(TEXT("minutes and frames"), JamHud::Timecode(83.5f), FString(TEXT("00:01:23:12")));
	TestEqual(TEXT("hours"), JamHud::Timecode(3661.f), FString(TEXT("01:01:01:00")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiHudSamples, "Jam.UI.Hud.Samples", JamUiFlags)
bool FJamUiHudSamples::RunTest(const FString&)
{
	for (const FString& Name : JamHud::SampleNames())
	{
		FJamHudView V;
		TestTrue(FString::Printf(TEXT("sample %s exists"), *Name), JamHud::Sample(Name, V));
		TestTrue(FString::Printf(TEXT("sample %s shows the shoot"), *Name), V.bShootVisible);
		TestEqual(FString::Printf(TEXT("sample %s: card kind matches a title"), *Name), V.CardKind != EJamCardKind::None, !V.CardTitle.IsEmpty());
	}
	FJamHudView V;
	TestFalse(TEXT("unknown sample"), JamHud::Sample(TEXT("nope"), V));
	TestTrue(TEXT("names ignore case"), JamHud::Sample(TEXT("CUT"), V));
	TestEqual(TEXT("cut sample is a CUT card"), V.CardKind, EJamCardKind::Cut);
	TestFalse(TEXT("cut sample pitches a real line"), V.CardSubtitle.StartsWith(TEXT("I have a note.")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiHudCardStyles, "Jam.UI.Hud.CardStyles", JamUiFlags)
bool FJamUiHudCardStyles::RunTest(const FString&)
{
	// Each card the shoot shows reads differently at a glance.
	const EJamCardKind Kinds[] = { EJamCardKind::Slate, EJamCardKind::Action, EJamCardKind::Cut, EJamCardKind::Retake, EJamCardKind::Wrap };
	for (int32 i = 0; i < UE_ARRAY_COUNT(Kinds); ++i)
	{
		for (int32 j = i + 1; j < UE_ARRAY_COUNT(Kinds); ++j)
		{
			TestFalse(FString::Printf(TEXT("accent %d differs from %d"), i, j), JamHud::CardStyle(Kinds[i]).Accent.Equals(JamHud::CardStyle(Kinds[j]).Accent, 0.05f));
		}
	}
	return true;
}

#endif
