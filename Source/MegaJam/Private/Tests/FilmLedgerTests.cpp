#include "Misc/AutomationTest.h"
#include "Film/JamLedger.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamFilmFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace
{
	FJamLedgerInput In(int32 Gross, int32 Floor, int32 Seed, std::initializer_list<int32> Loot = { 0, 1, 3 })
	{
		FJamLedgerInput I;
		I.Gross = Gross;
		I.Floor = Floor;
		I.Seed = Seed;
		I.LootRarities = Loot;
		return I;
	}

	int32 Sum(const FJamLedgerResult& R)
	{
		int32 T = 0;
		for (const FJamLedgerLine& L : R.Lines) { T += L.Amount; }
		return T;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmLedgerFloor, "Jam.Film.Ledger.Floor", JamFilmFlags)
bool FJamFilmLedgerFloor::RunTest(const FString&)
{
	for (int32 Gross : { 0, 1, 2, 3, 7, 40, 100, 250, 1000, 99999 })
	{
		for (int32 Floor : { 0, 1, 10, 30, 100, 5000 })
		{
			for (int32 Seed = 0; Seed < 25; ++Seed)
			{
				const FJamLedgerResult R = JamLedger::Compute(In(Gross, Floor, Seed));
				const int32 EffFloor = FMath::Min(Floor, Gross);
				TestTrue(TEXT("hero cut at least the floor"), R.HeroCut >= EffFloor);
				TestTrue(TEXT("deductions never exceed gross minus floor"), R.DeductionsTotal <= Gross - EffFloor);
				TestEqual(TEXT("lines add up to the total"), Sum(R), R.DeductionsTotal);
				TestEqual(TEXT("everything is accounted for"), R.HeroCut + R.DeductionsTotal, Gross);
				for (const FJamLedgerLine& L : R.Lines) { TestTrue(TEXT("no empty line"), L.Amount >= 1); }
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmLedgerShape, "Jam.Film.Ledger.Shape", JamFilmFlags)
bool FJamFilmLedgerShape::RunTest(const FString&)
{
	for (int32 Seed = 0; Seed < 200; ++Seed)
	{
		const FJamLedgerResult R = JamLedger::Compute(In(1000, 100, Seed));
		TestTrue(TEXT("four to six lines"), R.Lines.Num() >= 4 && R.Lines.Num() <= 6);
		TestEqual(TEXT("first is the director"), R.Lines[0].Id, FName(TEXT("DirectorsFee")));
		TestEqual(TEXT("second is the producer"), R.Lines[1].Id, FName(TEXT("ProducersFee")));
		TestEqual(TEXT("last is miscellaneous"), R.Lines.Last().Id, FName(TEXT("Miscellaneous")));
		int32 Largest = 0;
		TSet<FName> Seen;
		for (const FJamLedgerLine& L : R.Lines)
		{
			Largest = FMath::Max(Largest, L.Amount);
			TestFalse(TEXT("no repeated line"), Seen.Contains(L.Id));
			Seen.Add(L.Id);
			TestFalse(TEXT("every line has a note"), L.Note.IsEmpty());
		}
		TestEqual(TEXT("miscellaneous is the largest"), R.Lines.Last().Amount, Largest);
		TestTrue(TEXT("hero lands just above the floor"), R.HeroCut >= 100 && R.HeroCut <= 100 + 1000 / 20);
		TestTrue(TEXT("footer in range"), R.FooterIndex >= 0 && R.FooterIndex < JamLedger::Footers().Num());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmLedgerLoot, "Jam.Film.Ledger.Loot", JamFilmFlags)
bool FJamFilmLedgerLoot::RunTest(const FString&)
{
	TestEqual(TEXT("she keeps the best"), JamLedger::Compute(In(500, 50, 1, { 0, 2, 4, 1 })).KeptLootIndex, 2);
	TestEqual(TEXT("first of equal bests"), JamLedger::Compute(In(500, 50, 1, { 3, 3, 1 })).KeptLootIndex, 0);
	TestEqual(TEXT("never the only drop"), JamLedger::Compute(In(500, 50, 1, { 4 })).KeptLootIndex, (int32)INDEX_NONE);
	TestEqual(TEXT("no drops, nothing kept"), JamLedger::Compute(In(500, 50, 1, {})).KeptLootIndex, (int32)INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmLedgerDeterministic, "Jam.Film.Ledger.Deterministic", JamFilmFlags)
bool FJamFilmLedgerDeterministic::RunTest(const FString&)
{
	bool bVaries = false;
	const FJamLedgerResult Base = JamLedger::Compute(In(800, 80, 7));
	for (int32 i = 0; i < 5; ++i)
	{
		const FJamLedgerResult Again = JamLedger::Compute(In(800, 80, 7));
		TestEqual(TEXT("same lines"), Again.Lines.Num(), Base.Lines.Num());
		for (int32 j = 0; j < Base.Lines.Num() && j < Again.Lines.Num(); ++j)
		{
			TestEqual(TEXT("same id"), Again.Lines[j].Id, Base.Lines[j].Id);
			TestEqual(TEXT("same amount"), Again.Lines[j].Amount, Base.Lines[j].Amount);
		}
		TestEqual(TEXT("same hero cut"), Again.HeroCut, Base.HeroCut);
		TestEqual(TEXT("same footer"), Again.FooterIndex, Base.FooterIndex);
	}
	for (int32 Seed = 0; Seed < 20 && !bVaries; ++Seed)
	{
		bVaries = JamLedger::Compute(In(800, 80, Seed)).Lines.Num() != Base.Lines.Num()
			|| JamLedger::Compute(In(800, 80, Seed)).Lines[2].Id != Base.Lines[2].Id;
	}
	TestTrue(TEXT("different seeds draw different items"), bVaries);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmLedgerIdempotent, "Jam.Film.Ledger.Idempotent", JamFilmFlags)
bool FJamFilmLedgerIdempotent::RunTest(const FString&)
{
	FJamLedgerCommit Commit;
	TestFalse(TEXT("not committed yet"), Commit.IsCommitted());
	const FJamLedgerResult First = Commit.Commit(In(600, 60, 3));
	TestTrue(TEXT("committed"), Commit.IsCommitted());
	// A second commit, even with different input, returns the first outcome unchanged.
	const FJamLedgerResult Second = Commit.Commit(In(9999, 0, 99));
	TestEqual(TEXT("gross unchanged"), Second.Gross, First.Gross);
	TestEqual(TEXT("hero cut unchanged"), Second.HeroCut, First.HeroCut);
	TestEqual(TEXT("lines unchanged"), Second.Lines.Num(), First.Lines.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmLedgerTable, "Jam.Film.Ledger.Table", JamFilmFlags)
bool FJamFilmLedgerTable::RunTest(const FString&)
{
	TestEqual(TEXT("fifteen line items in Jokes.md"), JamLedger::TableSize(), 15);
	TestEqual(TEXT("three footers"), JamLedger::Footers().Num(), 3);
	return true;
}

#endif
