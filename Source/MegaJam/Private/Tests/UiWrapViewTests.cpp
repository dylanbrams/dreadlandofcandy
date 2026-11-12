#include "Misc/AutomationTest.h"
#include "UI/JamWrapView.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamUiFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiWrapRoll, "Jam.UI.Wrap.Roll", JamUiFlags)
bool FJamUiWrapRoll::RunTest(const FString&)
{
	const TArray<FJamLootDef>& Table = JamProgression::LootTable();
	for (int32 Seed = 0; Seed < 200; ++Seed)
	{
		const FJamWrapOutcome A = JamWrap::Roll(Seed % 3, 1.f, Seed);
		const FJamWrapOutcome B = JamWrap::Roll(Seed % 3, 1.f, Seed);
		if (!TestEqual(TEXT("three drops"), A.LootIndices.Num(), 3)) { return false; }
		TestTrue(TEXT("deterministic loot"), A.LootIndices == B.LootIndices);
		TestEqual(TEXT("deterministic cut"), A.Ledger.HeroCut, B.Ledger.HeroCut);
		TestTrue(TEXT("she keeps one"), A.LootIndices.IsValidIndex(A.KeptSlot));
		TestTrue(TEXT("the hero gets one"), A.LootIndices.IsValidIndex(A.EquippedSlot));
		TestNotEqual(TEXT("not the same one"), A.KeptSlot, A.EquippedSlot);
		for (int32 Slot = 0; Slot < 3; ++Slot)
		{
			TestTrue(TEXT("she keeps the best"), Table[A.LootIndices[A.KeptSlot]].Rarity >= Table[A.LootIndices[Slot]].Rarity);
		}
		TestTrue(TEXT("cut at least the floor"), A.Ledger.HeroCut >= JamWrap::HeroFloor);
		TestEqual(TEXT("cut and deductions are the gross"), A.Ledger.HeroCut + A.Ledger.DeductionsTotal, A.Ledger.Gross);
	}
	TestTrue(TEXT("taking notes raises the gross"), JamWrap::Roll(1, 1.5f, 7).Ledger.Gross > JamWrap::Roll(1, 1.f, 7).Ledger.Gross);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamUiWrapView, "Jam.UI.Wrap.View", JamUiFlags)
bool FJamUiWrapView::RunTest(const FString&)
{
	const FJamWrapOutcome Out = JamWrap::Roll(0, 1.f, 0);
	FJamProgression P;
	const FJamStatBlock Before = P.GetStats();
	P.AddLoot(Out.LootIndices[Out.EquippedSlot]);
	const FJamWrapView V = JamWrap::MakeView(Out, TEXT("A Balanced Diet"), Before, P.GetStats(), 1, 0);

	TestTrue(TEXT("visible"), V.bVisible);
	TestEqual(TEXT("ledger lines printed"), V.Lines.Num(), Out.Ledger.Lines.Num());
	TestFalse(TEXT("footer"), V.Footer.IsEmpty());
	int32 Kept = 0, Equipped = 0;
	for (const FJamWrapLoot& L : V.Loot)
	{
		Kept += L.bKept ? 1 : 0;
		Equipped += L.bEquipped ? 1 : 0;
		TestEqual(TEXT("only hers is stamped"), !L.Stamp.IsEmpty(), L.bKept);
		TestFalse(TEXT("stat text"), L.Stat.IsEmpty());
	}
	TestEqual(TEXT("one kept"), Kept, 1);
	TestEqual(TEXT("one equipped"), Equipped, 1);
	TestEqual(TEXT("first wrap's stamp is the sequel joke"), JamWrap::Stamp(0), FString(TEXT("RESERVED FOR SEQUEL")));
	TestEqual(TEXT("four stats"), V.Stats.Num(), 4);
	TestFalse(TEXT("level line"), V.LevelLine.IsEmpty());

	// The equipped item changes a real number.
	bool bChanged = false;
	for (const FJamWrapStat& S : V.Stats) { bChanged |= !FMath::IsNearlyEqual(S.Before, S.After); }
	TestTrue(TEXT("a stat changed"), bChanged);
	return true;
}

#endif
