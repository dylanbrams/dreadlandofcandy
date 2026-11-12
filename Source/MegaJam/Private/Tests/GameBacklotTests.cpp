#include "Misc/AutomationTest.h"
#include "Film/JamShoot.h"
#include "Game/JamBacklot.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

// The backlot table is walkable: one set per scene, in order up +X, no overlaps, every street joins the
// openings of the sets on either end, and each hero mark is inside its set.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBacklotLayoutTest, "Jam.Game.Backlot.Layout", JamFlags)
bool FJamBacklotLayoutTest::RunTest(const FString& Parameters)
{
	const TArray<FJamSetLayout>& Sets = JamBacklot::Sets();
	const TArray<FBox2D> Streets = JamBacklot::Streets();
	TestEqual(TEXT("one set per scene"), Sets.Num(), JamScenes::All().Num());
	TestEqual(TEXT("one street between each pair of sets"), Streets.Num(), Sets.Num() - 1);

	for (int32 i = 0; i < Sets.Num(); ++i)
	{
		const FBox2D& B = Sets[i].Bounds;
		TestTrue(FString::Printf(TEXT("set %d is valid"), i), B.bIsValid && B.GetSize().X > 0.f && B.GetSize().Y > 0.f);
		TestTrue(FString::Printf(TEXT("set %d is wide enough for the street opening"), i), B.Min.Y < -JamBacklot::StreetWidth * 0.5f && B.Max.Y > JamBacklot::StreetWidth * 0.5f);
		TestTrue(FString::Printf(TEXT("set %d hero mark is on the set"), i), B.IsInside(JamBacklot::HeroMark(i)));
		if (i > 0)
		{
			TestTrue(FString::Printf(TEXT("set %d is past set %d with room for a street"), i, i - 1), B.Min.X > Sets[i - 1].Bounds.Max.X + 1000.f);
		}
	}
	for (int32 i = 0; i < Streets.Num(); ++i)
	{
		TestEqual(FString::Printf(TEXT("street %d starts at set %d's front edge"), i, i), Streets[i].Min.X, Sets[i].Bounds.Max.X);
		TestEqual(FString::Printf(TEXT("street %d ends at set %d's back edge"), i, i + 1), Streets[i].Max.X, Sets[i + 1].Bounds.Min.X);
	}
	return true;
}

// Each set has room for what its scene spawns: at least 150 x 150 uu of floor per target at the set's capacity.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamBacklotRoomTest, "Jam.Game.Backlot.Room", JamFlags)
bool FJamBacklotRoomTest::RunTest(const FString& Parameters)
{
	const TArray<FJamSceneDef> Scenes = JamScenes::All();
	const TArray<FJamSetLayout>& Sets = JamBacklot::Sets();
	for (int32 i = 0; i < Scenes.Num() && i < Sets.Num(); ++i)
	{
		int32 Most = 0;
		for (const FJamStageDef& Stage : Scenes[i].Stages)
		{
			Most = FMath::Max(Most, JamScenes::SetCapacity(Scenes[i], Stage.Target));
		}
		const FVector2D Size = Sets[i].Bounds.GetSize();
		TestTrue(FString::Printf(TEXT("set %s has room for %d targets"), *Sets[i].Id.ToString(), Most), Size.X * Size.Y >= Most * 150.f * 150.f);
	}
	return true;
}

#undef JamFlags

#endif
