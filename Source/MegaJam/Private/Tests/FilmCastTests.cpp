#include "Misc/AutomationTest.h"
#include "Film/JamCast.h"
#include "UI/JamScreens.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamCastFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmCastTable, "Jam.Film.Cast.Table", JamCastFlags)
bool FJamFilmCastTable::RunTest(const FString&)
{
	const TArray<FJamClassDef>& All = JamCast::All();
	TestEqual(TEXT("four in the cast"), All.Num(), 4);
	TestEqual(TEXT("the knight comes first (the default)"), All[0].Id, FString(TEXT("knight")));
	TestTrue(TEXT("only the knight has every clip"), All[0].bFullClips && !All[1].bFullClips && !All[2].bFullClips && !All[3].bFullClips);
	for (int32 i = 0; i < All.Num(); ++i)
	{
		const FJamClassDef& D = All[i];
		TestFalse(TEXT("a name"), D.Name.IsEmpty());
		TestFalse(TEXT("a rig"), D.Rig.IsEmpty());
		TestFalse(TEXT("a blurb"), D.Blurb.IsEmpty());
		TestTrue(TEXT("at least two cries"), D.Cries.Num() >= 2);
		TestEqual(TEXT("ids round-trip"), JamCast::IndexOf(D.Id), i);
	}
	TestEqual(TEXT("-class= ignores case"), JamCast::IndexOf(TEXT("MAGE")), 1);
	TestEqual(TEXT("unknown class"), JamCast::IndexOf(TEXT("bard")), INDEX_NONE);
	TestEqual(TEXT("Get clamps"), JamCast::Get(99).Id, All.Last().Id);
	TestEqual(TEXT("Get clamps below"), JamCast::Get(-3).Id, FString(TEXT("knight")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmCastStepAndCries, "Jam.Film.Cast.StepAndCries", JamCastFlags)
bool FJamFilmCastStepAndCries::RunTest(const FString&)
{
	TestEqual(TEXT("right from the knight"), JamCast::Step(0, 1), 1);
	TestEqual(TEXT("left from the knight wraps"), JamCast::Step(0, -1), 3);
	TestEqual(TEXT("right from the last wraps"), JamCast::Step(3, 1), 0);
	const FJamClassDef& Mage = JamCast::Get(1);
	for (int32 i = 0; i < 6; ++i)
	{
		TestNotEqual(TEXT("never the same cry twice running"), JamCast::CryFor(Mage, i), JamCast::CryFor(Mage, i + 1));
		TestTrue(TEXT("a mage line"), JamCast::CryFor(Mage, i).StartsWith(TEXT("M")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmCastTitleLine, "Jam.Film.Cast.TitleLine", JamCastFlags)
bool FJamFilmCastTitleLine::RunTest(const FString&)
{
	FJamMenuSettings S;
	S.CastIndex = 2;
	const FJamScreenView V = JamScreens::Title(S);
	const FJamMenuOption* Casting = V.Options.FindByPredicate([](const FJamMenuOption& O) { return O.Action == EJamMenuAction::Casting; });
	TestNotNull(TEXT("the title has a Casting line"), Casting);
	if (Casting) { TestTrue(TEXT("it names the archer"), Casting->Value.Contains(TEXT("Archer"))); }
	TestEqual(TEXT("Roll Camera stays first"), V.Options[0].Action, EJamMenuAction::RollCamera);
	return true;
}

#endif
