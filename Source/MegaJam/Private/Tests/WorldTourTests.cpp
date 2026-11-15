#include "Misc/AutomationTest.h"
#include "World/JamTour.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace JamTourTestHelp   // named, not anonymous: the game target is a unity build
{
	float DistanceToPath(const FJamTourPath& Path, const FVector2D& P)
	{
		float Best = TNumericLimits<float>::Max();
		for (const FVector2D& S : Path.Samples()) { Best = FMath::Min(Best, FVector2D::Distance(S, P)); }
		return Best;
	}
}

// The flight plan starts in the square, stays inside the world and has no near-duplicate corners.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTourRouteTest, "Jam.Tour.Route", JamFlags)
bool FJamTourRouteTest::RunTest(const FString& Parameters)
{
	const FJamWorldDef& W = JamWorld::Def();
	const TArray<FVector2D> Route = JamTour::Waypoints(W);
	TestTrue(TEXT("a loop has corners"), Route.Num() >= 12);
	if (Route.IsEmpty()) { return false; }
	TestTrue(TEXT("starts in the village square"), Route[0].Size() < 1.f);
	for (int32 I = 0; I < Route.Num(); ++I)
	{
		TestTrue(*FString::Printf(TEXT("corner %d is inside the world"), I), W.Bounds.IsInside(Route[I]));
		TestTrue(*FString::Printf(TEXT("corner %d is not on top of the next"), I), FVector2D::Distance(Route[I], Route[(I + 1) % Route.Num()]) >= 500.f);
	}
	return true;
}

// Every camp, film set and biome giver is flown past, and the village square more than once (it is the showpiece).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTourCoverageTest, "Jam.Tour.Coverage", JamFlags)
bool FJamTourCoverageTest::RunTest(const FString& Parameters)
{
	const FJamWorldDef& W = JamWorld::Def();
	FJamTourPath Path;
	Path.Build(JamTour::Waypoints(W));
	if (!TestTrue(TEXT("the path builds"), Path.IsValid())) { return false; }
	TestTrue(TEXT("a lap is long enough to be worth recording (over 60,000 uu)"), Path.Length() > 60000.f);

	const TArray<FJamTourStop> Stops = JamTour::Stops(W);
	TestTrue(TEXT("the stops include the village and a camp"), Stops.Num() > 10);
	for (const FJamTourStop& S : Stops)
	{
		const float Miss = JamTourTestHelp::DistanceToPath(Path, S.Position);
		TestTrue(*FString::Printf(TEXT("%s is flown past (%.0f uu off the path)"), *S.Id.ToString(), Miss), Miss <= S.Radius + 1500.f);
	}
	return true;
}

// The path is one smooth closed curve: it wraps, no gaps, and it slows for places.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamTourPathTest, "Jam.Tour.Path", JamFlags)
bool FJamTourPathTest::RunTest(const FString& Parameters)
{
	const FJamWorldDef& W = JamWorld::Def();
	FJamTourPath Path;
	Path.Build(JamTour::Waypoints(W));
	if (!TestTrue(TEXT("the path builds"), Path.IsValid())) { return false; }

	TestTrue(TEXT("distance 0 and one lap are the same point"), FVector2D::Distance(Path.PointAt(0.f), Path.PointAt(Path.Length())) < 1.f);
	TestTrue(TEXT("distance wraps past a lap"), FVector2D::Distance(Path.PointAt(500.f), Path.PointAt(Path.Length() + 500.f)) < 1.f);
	TestTrue(TEXT("a negative distance counts back from the end"), FVector2D::Distance(Path.PointAt(-300.f), Path.PointAt(Path.Length() - 300.f)) < 1.f);

	float MaxJump = 0.f;
	for (float D = 0.f; D < Path.Length(); D += 100.f) { MaxJump = FMath::Max(MaxJump, FVector2D::Distance(Path.PointAt(D), Path.PointAt(D + 100.f))); }
	TestTrue(*FString::Printf(TEXT("100 uu along is 100 uu away (worst %.1f)"), MaxJump), MaxJump <= 101.f);

	float MinTurnCos = 1.f;   // the heading never reverses within 400 uu
	for (float D = 0.f; D < Path.Length(); D += 200.f) { MinTurnCos = FMath::Min(MinTurnCos, FVector2D::DotProduct(Path.DirectionAt(D), Path.DirectionAt(D + 400.f))); }
	TestTrue(*FString::Printf(TEXT("no hairpins (worst heading cosine %.2f)"), MinTurnCos), MinTurnCos > 0.f);

	const TArray<FJamTourStop> Stops = JamTour::Stops(W);
	TestTrue(TEXT("full speed in open country"), FMath::IsNearlyEqual(JamTour::SpeedScale(Stops, FVector2D(60000.f, 60000.f)), 1.f));
	TestTrue(TEXT("half speed in the village square"), FMath::IsNearlyEqual(JamTour::SpeedScale(Stops, FVector2D::ZeroVector), 0.5f));
	const FJamLocation* Camp = JamWorld::Find(TEXT("CampMeadowFar"));
	if (TestNotNull(TEXT("a camp"), Camp))
	{
		TestTrue(TEXT("half speed at a camp"), FMath::IsNearlyEqual(JamTour::SpeedScale(Stops, Camp->Position), 0.5f));
		int32 Index = INDEX_NONE;
		TestTrue(TEXT("looks at the camp from 1,000 uu"), JamTour::GazeWeight(Stops, Camp->Position + FVector2D(1000.f, 0.f), Index) > 0.5f && Stops.IsValidIndex(Index) && Stops[Index].Id == Camp->Id);
		TestTrue(TEXT("no glance far from every place"), JamTour::GazeWeight(Stops, FVector2D(60000.f, 60000.f), Index) == 0.f);
	}
	return true;
}

#endif
