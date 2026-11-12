#include "UI/JamWrapView.h"

int32 JamWrap::BaseGross(int32 SceneIndex)
{
	static const int32 Gross[] = { 1200, 1800, 2600 };
	return Gross[FMath::Clamp(SceneIndex, 0, (int32)UE_ARRAY_COUNT(Gross) - 1)];
}

FString JamWrap::StatText(const FJamLootDef& Loot)
{
	const TCHAR* Stat = TEXT("everything");
	switch (Loot.Stat)
	{
	case EJamLootStat::LectureRadius:   Stat = TEXT("Lecture Radius"); break;
	case EJamLootStat::LectureCooldown: Stat = TEXT("Lecture Cooldown"); break;
	case EJamLootStat::MoveSpeed:       Stat = TEXT("Move Speed"); break;
	case EJamLootStat::Everything:      Stat = TEXT("everything"); break;
	}
	return FString::Printf(TEXT("%+d%% %s"), Loot.Percent, Stat);
}

FString JamWrap::Stamp(int32 Seed)
{
	static const TCHAR* Stamps[] = { TEXT("RESERVED FOR SEQUEL"), TEXT("PRODUCER'S PICK"), TEXT("EVIDENCE") };
	// The first wrap always gets the joke the card is built around.
	return Seed % 100 == 0 ? FString(Stamps[0]) : FString(Stamps[FMath::Abs(Seed * 7 + 3) % UE_ARRAY_COUNT(Stamps)]);
}

FJamWrapOutcome JamWrap::Roll(int32 SceneIndex, float GrossMultiplier, int32 Seed)
{
	FJamWrapOutcome Out;
	Out.LootIndices = JamProgression::RollLoot(Seed, 3);
	const TArray<FJamLootDef>& Table = JamProgression::LootTable();

	FJamLedgerInput In;
	In.Gross = FMath::RoundToInt(BaseGross(SceneIndex) * FMath::Max(0.1f, GrossMultiplier));
	In.Floor = HeroFloor;
	In.Seed = Seed;
	for (int32 Index : Out.LootIndices) { In.LootRarities.Add(Table[Index].Rarity); }
	Out.Ledger = JamLedger::Compute(In);
	Out.KeptSlot = Out.Ledger.KeptLootIndex;

	// The hero gets the best of what she left; later drops are better on ties (RollLoot orders worst to best).
	for (int32 Slot = 0; Slot < Out.LootIndices.Num(); ++Slot)
	{
		if (Slot == Out.KeptSlot) { continue; }
		if (Out.EquippedSlot == INDEX_NONE || Table[Out.LootIndices[Slot]].Rarity >= Table[Out.LootIndices[Out.EquippedSlot]].Rarity)
		{
			Out.EquippedSlot = Slot;
		}
	}
	return Out;
}

FJamWrapView JamWrap::MakeView(const FJamWrapOutcome& Outcome, const FString& SceneTitle, const FJamStatBlock& Before,
	const FJamStatBlock& After, int32 LevelsGained, int32 Seed)
{
	FJamWrapView V;
	V.bVisible = true;
	V.SceneTitle = SceneTitle;
	V.Gross = Outcome.Ledger.Gross;
	for (const FJamLedgerLine& L : Outcome.Ledger.Lines) { V.Lines.Add({ L.Item, L.Note, L.Amount }); }
	V.HeroCut = Outcome.Ledger.HeroCut;
	const TArray<FString>& Footers = JamLedger::Footers();
	V.Footer = Footers.IsValidIndex(Outcome.Ledger.FooterIndex) ? Footers[Outcome.Ledger.FooterIndex] : FString();

	const TArray<FJamLootDef>& Table = JamProgression::LootTable();
	for (int32 Slot = 0; Slot < Outcome.LootIndices.Num(); ++Slot)
	{
		const FJamLootDef& Def = Table[Outcome.LootIndices[Slot]];
		FJamWrapLoot Card;
		Card.Name = Def.Name;
		Card.Flavour = Def.Flavour;
		Card.Stat = StatText(Def);
		Card.bKept = Slot == Outcome.KeptSlot;
		Card.bEquipped = Slot == Outcome.EquippedSlot;
		Card.RarityLevel = Card.bKept ? 4 : Def.Rarity;
		Card.Rarity = JamProgression::RarityName(Def.Rarity, Card.bKept);
		if (Card.bKept) { Card.Stamp = Stamp(Seed); }
		V.Loot.Add(Card);
	}

	V.Stats.Add({ TEXT("Lecture Power"), Before.LecturePower, After.LecturePower, FString() });
	V.Stats.Add({ TEXT("Radius"), Before.Radius, After.Radius, TEXT(" uu") });
	V.Stats.Add({ TEXT("Cooldown Recovery"), Before.CooldownRecovery, After.CooldownRecovery, TEXT("x") });
	V.Stats.Add({ TEXT("Move Speed"), Before.MoveSpeed, After.MoveSpeed, TEXT(" uu/s") });

	if (LevelsGained > 0)
	{
		V.LevelLine = FString::Printf(TEXT("LEVEL UP!  +%d skill point%s"), LevelsGained, LevelsGained == 1 ? TEXT("") : TEXT("s"));
	}
	return V;
}
