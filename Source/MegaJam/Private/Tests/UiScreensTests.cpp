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
	TestEqual(TEXT("the game's title"), P.Title, FString(TEXT("THE DREAD LAND OF CANDY")));
	TestEqual(TEXT("theatrical stamp"), P.Stamp, FString(TEXT("THEATRICAL RELEASE")));
	TestEqual(TEXT("credits pair the same way"), JamScreens::Credits(true, FJamMenuSettings()).Stamp, FString(TEXT("DIRECTOR'S CUT")));
	TestEqual(TEXT("as does the title screen"), JamScreens::Title(FJamMenuSettings()).Stamp, JamScreens::EditionStamp(true));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiScreensGeneratedPremiere, "Jam.UI.Screens.GeneratedPremiere", JamUiFlags)
bool FJamUiScreensGeneratedPremiere::RunTest(const FString&)
{
	const FString Good = TEXT("{\"v\":1,\"source\":\"model\",\"title\":\"The Dread Land of Candy: A Study in Shame\",\"tagline\":\"They were big.\\nThey were sorry.\",")
		TEXT("\"reviews\":[{\"quote\":\"I felt seen.\",\"by\":\"a goblin\"},{\"quote\":\"Long.\",\"by\":\"a rat\"},{\"quote\":\"Mine.\",\"by\":\"F. Flicker\"}],\"extra\":1}");
	FJamPremiereTotals T;
	TestTrue(TEXT("a good body reads"), JamScreens::ReadPremiere(Good, T));
	TestEqual(TEXT("title"), T.Title, FString(TEXT("The Dread Land of Candy: A Study in Shame")));
	TestEqual(TEXT("newline collapsed"), T.Tagline, FString(TEXT("They were big. They were sorry.")));
	TestEqual(TEXT("three reviews"), T.Reviews.Num(), 3);
	const FJamScreenView P = JamScreens::Premiere(T, FJamMenuSettings());
	TestEqual(TEXT("generated title shown"), P.Title, FString(TEXT("THE DREAD LAND OF CANDY: A STUDY IN SHAME")));
	TestTrue(TEXT("generated reviews shown"), P.Quotes == T.Reviews);

	FJamPremiereTotals Untouched;
	TestFalse(TEXT("budget falls back"), JamScreens::ReadPremiere(TEXT("{\"v\":1,\"source\":\"budget\"}"), Untouched));
	TestFalse(TEXT("garbage falls back"), JamScreens::ReadPremiere(TEXT("not json"), Untouched));
	TestFalse(TEXT("two reviews fall back"), JamScreens::ReadPremiere(Good.Replace(TEXT(",{\"quote\":\"Mine.\",\"by\":\"F. Flicker\"}"), TEXT("")), Untouched));
	TestFalse(TEXT("wrong version falls back"), JamScreens::ReadPremiere(Good.Replace(TEXT("\"v\":1"), TEXT("\"v\":2")), Untouched));
	TestTrue(TEXT("fallbacks leave it untouched"), Untouched.Title.IsEmpty() && Untouched.Reviews.Num() == 0);
	const FString Long = Good.Replace(TEXT("A Study in Shame"), TEXT("A Study in Shame That Goes On and On and On Forever"));
	FJamPremiereTotals Cut;
	TestTrue(TEXT("long title reads"), JamScreens::ReadPremiere(Long, Cut));
	TestTrue(TEXT("title cut to 48"), Cut.Title.Len() <= 48);
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
	FJamMusicContext Credits;
	Credits.Phase = EJamFilmPhase::Credits;
	TestEqual(TEXT("the theme song plays in the credits"), JamMusic::CueFor(Credits), FName(TEXT("theme_song")));
	TestEqual(TEXT("one take of the song"), JamMusic::AssetPath(TEXT("theme_song"), 2), FString(TEXT("/Game/Audio/Music/M_theme_song_take1.M_theme_song_take1")));
	TestEqual(TEXT("falls back to the credits cue"), JamMusic::Fallback(TEXT("theme_song")), FName(TEXT("credits")));
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
