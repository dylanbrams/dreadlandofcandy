#include "Misc/AutomationTest.h"
#include "UI/JamScreens.h"
#include "UI/JamMusic.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamUiFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiScreensMenus, "Jam.UI.Screens.Menus", JamUiFlags)
bool FJamUiScreensMenus::RunTest(const FString&)
{
	FJamMenuSettings S;
	const FJamScreenView Title = JamScreens::Title(S);
	TestEqual(TEXT("title starts on Roll Camera"), Title.Options[Title.Selected].Action, EJamMenuAction::RollCamera);

	const FJamScreenView Options = JamScreens::Options(S);
	int32 Locked = INDEX_NONE;
	for (int32 i = 0; i < Options.Options.Num(); ++i) { if (Options.Options[i].Action == EJamMenuAction::Locked) { Locked = i; } }
	TestTrue(TEXT("the budget line exists"), Locked != INDEX_NONE);
	TestFalse(TEXT("and is greyed out"), Options.Options[Locked].bEnabled);
	// Stepping never lands on the locked line, both ways, from every line.
	for (int32 i = 0; i < Options.Options.Num(); ++i)
	{
		TestNotEqual(TEXT("down skips the budget"), JamScreens::Step(Options.Options, i, 1), Locked);
		TestNotEqual(TEXT("up skips the budget"), JamScreens::Step(Options.Options, i, -1), Locked);
	}
	TestEqual(TEXT("wraps from the top"), JamScreens::Step(Title.Options, 0, -1), Title.Options.Num() - 1);

	const FJamScreenView Quit = JamScreens::QuitConfirm(S, false);
	TestEqual(TEXT("quit confirm starts on Fine"), Quit.Options[Quit.Selected].Action, EJamMenuAction::Fine);
	TestFalse(TEXT("pause caption is Fenella's line"), JamScreens::Pause(S).Caption.IsEmpty());

	FJamMenuSettings Pad;
	Pad.bGamepad = true;
	TestTrue(TEXT("gamepad glyphs"), JamScreens::Title(Pad).Footer.Contains(TEXT("(A)")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiScreensEndings, "Jam.UI.Screens.Endings", JamUiFlags)
bool FJamUiScreensEndings::RunTest(const FString&)
{
	TestTrue(TEXT("ties go to the director"), JamScreens::IsDirectorsCut(1, 1));
	TestFalse(TEXT("mostly refused is theatrical"), JamScreens::IsDirectorsCut(0, 2));

	FJamPremiereTotals T;
	T.bDirectorsCut = false;
	const FJamScreenView P = JamScreens::Premiere(T, FJamMenuSettings());
	TestEqual(TEXT("theatrical title"), P.Title, FString(TEXT("THE THEATRICAL RELEASE")));
	TestEqual(TEXT("four totals rows, rats last"), P.Rows.Num() > 0 ? P.Rows.Last().Key : FString(), FString(TEXT("RATS")));
	TestEqual(TEXT("three reviews"), P.Quotes.Num(), 3);

	// Theatrical: her name is gone from every credit except the Boom Operator's.
	for (const FString& Line : JamScreens::CreditLines(false))
	{
		if (Line.Contains(TEXT("Flicker"))) { TestTrue(FString::Printf(TEXT("only the boom operator keeps her name: %s"), *Line), Line.StartsWith(TEXT("Boom Operator"))); }
	}
	TestTrue(TEXT("director's cut credits her"), JamScreens::CreditLines(true)[0].Contains(TEXT("Fenella Flicker")));
	bool bReal = false;
	for (const FString& Line : JamScreens::CreditLines(true)) { bReal |= Line.Contains(TEXT("Unreal Engine")); }
	TestTrue(TEXT("real credits follow"), bReal);

	for (const TCHAR* Name : { TEXT("rating"), TEXT("title"), TEXT("options"), TEXT("quit"), TEXT("pause"), TEXT("premiere"), TEXT("theatrical"), TEXT("credits") })
	{
		FJamScreenView V;
		TestTrue(FString::Printf(TEXT("sample %s"), Name), JamScreens::Sample(Name, V));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiMusicCues, "Jam.UI.Music.Cues", JamUiFlags)
bool FJamUiMusicCues::RunTest(const FString&)
{
	FJamMusicContext C;
	TestEqual(TEXT("title"), JamMusic::CueFor(C), FName(TEXT("quest_accepted")));
	C.Phase = EJamFilmPhase::Shooting;
	TSet<FName> Scenes;
	for (int32 Scene = 1; Scene <= 3; ++Scene) { C.Scene = Scene; Scenes.Add(JamMusic::CueFor(C)); }
	TestEqual(TEXT("each scene has its own music"), Scenes.Num(), 3);
	C.Stage = 2;
	TestEqual(TEXT("the finale changes after its CUT!"), JamMusic::CueFor(C), FName(TEXT("final_boss")));
	C.bWalking = true;
	TestEqual(TEXT("the street"), JamMusic::CueFor(C), FName(TEXT("open_world_exploration")));
	for (const FName Cue : JamMusic::AllCues())
	{
		TestTrue(FString::Printf(TEXT("%s path"), *Cue.ToString()), JamMusic::AssetPath(Cue, 2).StartsWith(TEXT("/Game/Audio/Music/M_")));
	}
	// Every cue the plan can produce is in the import list.
	for (const EJamFilmPhase Phase : { EJamFilmPhase::Title, EJamFilmPhase::Shooting, EJamFilmPhase::Premiere, EJamFilmPhase::Credits })
	{
		for (int32 Scene = 1; Scene <= 3; ++Scene)
		{
			for (int32 Stage = 0; Stage < 3; ++Stage)
			{
				for (int32 Flags = 0; Flags < 4; ++Flags)
				{
					FJamMusicContext X;
					X.Phase = Phase; X.Scene = Scene; X.Stage = Stage; X.bWalking = (Flags & 1) != 0; X.bNotePrompt = (Flags & 2) != 0;
					TestTrue(TEXT("cue is imported"), JamMusic::AllCues().Contains(JamMusic::CueFor(X)));
				}
			}
		}
	}
	return true;
}

#endif
