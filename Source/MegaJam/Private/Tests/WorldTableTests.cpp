#include "Misc/AutomationTest.h"
#include "World/JamWorld.h"
#include "Film/JamShoot.h"
#include "Game/JamBacklot.h"
#include "Game/JamOpenWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

// BUILD_SPEC 3e: every location fits inside the bounds; the village and one film set per scene exist.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamWorldLayoutTest, "Jam.World.Layout", JamFlags)
bool FJamWorldLayoutTest::RunTest(const FString& Parameters)
{
	const FJamWorldDef& W = JamWorld::Def();
	TestTrue(TEXT("bounds are valid"), W.Bounds.bIsValid && W.Bounds.GetSize().X > 0.f);
	TestTrue(TEXT("the village square is at the origin"), W.Bounds.IsInside(FVector2D::ZeroVector));
	for (const FJamLocation& L : W.Locations)
	{
		const FBox2D Extent(L.Position - FVector2D(L.Radius), L.Position + FVector2D(L.Radius));
		TestTrue(FString::Printf(TEXT("%s is inside the bounds"), *L.Id.ToString()), W.Bounds.IsInside(Extent.Min) && W.Bounds.IsInside(Extent.Max));
		TestEqual(FString::Printf(TEXT("%s is unique"), *L.Id.ToString()), W.Locations.FilterByPredicate([&L](const FJamLocation& O) { return O.Id == L.Id; }).Num(), 1);
		if (L.Kind != EJamLocationKind::Village && L.Kind != EJamLocationKind::Landmark)
		{
			TestEqual(FString::Printf(TEXT("%s stands in its own biome"), *L.Id.ToString()), (int32)JamWorld::BiomeAt(L.Position), (int32)L.Biome);
		}
	}
	const TArray<const FJamLocation*> Villages = JamWorld::OfKind(EJamLocationKind::Village);
	TestEqual(TEXT("one village"), Villages.Num(), 1);
	if (Villages.Num() == 1) { TestTrue(TEXT("village at the origin"), Villages[0]->Position.IsNearlyZero()); }

	for (int32 i = 0; i < JamScenes::All().Num(); ++i)
	{
		const FJamLocation* Set = JamWorld::FilmSet(i);
		if (!TestNotNull(FString::Printf(TEXT("film set for scene %d"), i + 1), Set)) { continue; }
		const float Out = Set->Position.Size();
		TestTrue(FString::Printf(TEXT("set %d is 5,000 to 7,000 uu out"), i + 1), Out >= 5000.f && Out <= 7000.f);
		if (JamBacklot::Sets().IsValidIndex(i))
		{
			const FVector2D Floor = JamWorld::FilmSetFloor(*Set).GetSize();
			const FVector2D Backlot = JamBacklot::Sets()[i].Bounds.GetSize();
			TestTrue(FString::Printf(TEXT("set %d is at least the backlot set's size"), i + 1), Floor.X >= Backlot.X && Floor.Y >= Backlot.Y);
		}
	}
	return true;
}

// Each wild biome has at least two camps and a giver; camps hold at least 3, spawn their biome's kinds,
// and overlap neither each other, the village, nor a film set.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamWorldCampsTest, "Jam.World.Camps", JamFlags)
bool FJamWorldCampsTest::RunTest(const FString& Parameters)
{
	const FJamWorldDef& W = JamWorld::Def();
	for (const FJamBiomeDef& B : W.Biomes)
	{
		TestTrue(FString::Printf(TEXT("%s has a giver"), *B.DisplayName), JamWorld::OfKind(EJamLocationKind::Giver, B.Biome).Num() >= 1);
		if (B.Biome == EJamBiome::Village) { continue; }
		TestTrue(FString::Printf(TEXT("%s has two camps"), *B.DisplayName), JamWorld::OfKind(EJamLocationKind::Camp, B.Biome).Num() >= 2);
		TestTrue(FString::Printf(TEXT("%s has nuisance kinds"), *B.DisplayName), B.Kinds.Num() > 0);
	}
	const TArray<const FJamLocation*> Camps = JamWorld::OfKind(EJamLocationKind::Camp);
	const TArray<const FJamLocation*> Sets = JamWorld::OfKind(EJamLocationKind::FilmSet);
	for (const FJamLocation* C : Camps)
	{
		const FString Id = C->Id.ToString();
		TestTrue(FString::Printf(TEXT("%s capacity >= 3"), *Id), C->Capacity >= 3);
		TestTrue(FString::Printf(TEXT("%s spawns something"), *Id), C->Kinds.Num() > 0);
		const FJamBiomeDef* B = JamWorld::Biome(C->Biome);
		for (EJamTargetKind Kind : C->Kinds)
		{
			TestTrue(FString::Printf(TEXT("%s kind belongs to its biome"), *Id), B && B->Kinds.Contains(Kind));
		}
		TestTrue(FString::Printf(TEXT("%s clear of the village"), *Id), C->Position.Size() > W.VillageRadius + C->Radius);
		for (const FJamLocation* O : Camps)
		{
			if (O != C) { TestTrue(FString::Printf(TEXT("%s clear of %s"), *Id, *O->Id.ToString()), FVector2D::Distance(C->Position, O->Position) > C->Radius + O->Radius); }
		}
		for (const FJamLocation* S : Sets)
		{
			// A set's floor is square: its half diagonal bounds it.
			TestTrue(FString::Printf(TEXT("%s clear of %s"), *Id, *S->Id.ToString()), FVector2D::Distance(C->Position, S->Position) > C->Radius + S->Radius * UE_SQRT_2);
		}
	}
	return true;
}

// The fallback scatter is deterministic, keeps clear of every gameplay location, and dresses all three wild biomes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamWorldScatterTest, "Jam.World.Scatter", JamFlags)
bool FJamWorldScatterTest::RunTest(const FString& Parameters)
{
	const TArray<FJamProp> A = JamOpenWorld::Scatter(7);
	const TArray<FJamProp> B = JamOpenWorld::Scatter(7);
	TestEqual(TEXT("same seed, same count"), A.Num(), B.Num());
	TestTrue(TEXT("a few hundred props"), A.Num() >= 100 && A.Num() <= 1200);
	bool bSame = A.Num() == B.Num();
	for (int32 i = 0; bSame && i < A.Num(); ++i) { bSame = A[i].Mesh == B[i].Mesh && A[i].At.Equals(B[i].At); }
	TestTrue(TEXT("same seed, same props"), bSame);

	TSet<EJamBiome> Dressed;
	for (const FJamProp& P : A)
	{
		Dressed.Add(JamWorld::BiomeAt(P.At));
		for (const FJamLocation& L : JamWorld::Def().Locations)
		{
			if (L.Kind == EJamLocationKind::Village || L.Kind == EJamLocationKind::Landmark) { continue; }
			const float R = L.Kind == EJamLocationKind::FilmSet ? L.Radius * UE_SQRT_2 : L.Radius;
			if (FVector2D::Distance(P.At, L.Position) <= R)
			{
				AddError(FString::Printf(TEXT("%s at (%.0f, %.0f) is inside %s"), *P.Mesh, P.At.X, P.At.Y, *L.Id.ToString()));
			}
		}
	}
	TestTrue(TEXT("meadow dressed"), Dressed.Contains(EJamBiome::Meadow));
	TestTrue(TEXT("molars dressed"), Dressed.Contains(EJamBiome::Molars));
	TestTrue(TEXT("orchard dressed"), Dressed.Contains(EJamBiome::Orchard));
	TestTrue(TEXT("fallback map is the template"), JamOpenWorld::IsFallbackMap(TEXT("/Engine/Maps/Templates/Template_Default")) && !JamOpenWorld::IsFallbackMap(TEXT("/Game/World/L_Sugarborough")));
	return true;
}

#endif
