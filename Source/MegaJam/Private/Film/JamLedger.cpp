#include "Film/JamLedger.h"
#include "Math/RandomStream.h"

namespace
{
	struct FLedgerRow
	{
		const TCHAR* Id;
		const TCHAR* Item;
		const TCHAR* Note;
	};

	// Jokes.md "The Production Ledger". The first two are always drawn; Miscellaneous is always last and largest.
	const FLedgerRow GFirst[2] = {
		{ TEXT("DirectorsFee"), TEXT("Director's fee"), TEXT("Standard.") },
		{ TEXT("ProducersFee"), TEXT("Producer's fee"), TEXT("Also me.") },
	};
	const FLedgerRow GMisc = { TEXT("Miscellaneous"), TEXT("Miscellaneous"), TEXT("Don't worry about it.") };
	const FLedgerRow GMiddle[] = {
		{ TEXT("Catering"), TEXT("Catering"), TEXT("There was no catering. That costs extra.") },
		{ TEXT("WingWear"), TEXT("Wing wear and tear"), TEXT("They don't flap themselves. They do. Still.") },
		{ TEXT("FaerieDust"), TEXT("Faerie dust (practical effects)"), TEXT("Organic. Locally sourced. From me.") },
		{ TEXT("RatWrangler"), TEXT("Rat wrangler"), TEXT("The rats wrangled themselves. I supervised.") },
		{ TEXT("Celery"), TEXT("Celery, hero-grade"), TEXT("The good celery.") },
		{ TEXT("SwordStorage"), TEXT("Sword storage"), TEXT("It's your sword. You're welcome.") },
		{ TEXT("Continuity"), TEXT("Continuity"), TEXT("You started at level seven. That wasn't free.") },
		{ TEXT("Megaphone"), TEXT("Megaphone"), TEXT("It's a rolled-up leaf. A premium leaf.") },
		{ TEXT("GoblinDental"), TEXT("Goblin dental plan"), TEXT("Somebody has to. You, apparently.") },
		{ TEXT("EmotionalDamages"), TEXT("Emotional damages"), TEXT("Mine. From take three.") },
		{ TEXT("Reshoots"), TEXT("Reshoots"), TEXT("See: you.") },
		{ TEXT("Rounding"), TEXT("Rounding"), TEXT("In my favour. For luck.") },
	};

	FJamLedgerLine MakeLine(const FLedgerRow& Row)
	{
		FJamLedgerLine L;
		L.Id = Row.Id;
		L.Item = Row.Item;
		L.Note = Row.Note;
		return L;
	}
}

namespace JamLedger
{
	const TArray<FString>& Footers()
	{
		static const TArray<FString> Lines = {
			TEXT("plus exposure"),
			TEXT("before tax. I am also the tax."),
			TEXT("non-negotiable. I checked with myself."),
		};
		return Lines;
	}

	int32 TableSize()
	{
		return 2 + UE_ARRAY_COUNT(GMiddle) + 1;
	}

	FJamLedgerResult Compute(const FJamLedgerInput& Input)
	{
		FJamLedgerResult R;
		R.Gross = FMath::Max(0, Input.Gross);
		const int32 Floor = FMath::Clamp(Input.Floor, 0, R.Gross);
		FRandomStream Rng(Input.Seed);

		// The hero lands just above the floor: floor plus about 5% of the gross, never past the gross.
		const int32 Slack = R.Gross - Floor;
		const int32 Bonus = Slack > 0 ? FMath::Min(Slack, FMath::Max(1, R.Gross / 20)) : 0;
		R.HeroCut = Floor + Bonus;
		const int32 Deductions = R.Gross - R.HeroCut;
		R.DeductionsTotal = Deductions;

		if (Deductions > 0)
		{
			// Four to six lines, but never more than there are units to take.
			const int32 Wanted = Rng.RandRange(4, 6);
			const int32 N = FMath::Min(Wanted, Deductions);

			TArray<FJamLedgerLine> Lines;
			if (N == 1)
			{
				Lines.Add(MakeLine(GMisc));
			}
			else
			{
				Lines.Add(MakeLine(GFirst[0]));
				if (N >= 3)
				{
					Lines.Add(MakeLine(GFirst[1]));
				}
				// Middle lines: a seeded shuffle of the pool, no repeats.
				TArray<int32> Pool;
				for (int32 i = 0; i < UE_ARRAY_COUNT(GMiddle); ++i) { Pool.Add(i); }
				for (int32 i = Pool.Num() - 1; i > 0; --i) { Pool.Swap(i, Rng.RandRange(0, i)); }
				for (int32 i = 0; Lines.Num() < N - 1; ++i) { Lines.Add(MakeLine(GMiddle[Pool[i]])); }
				Lines.Add(MakeLine(GMisc));
			}

			// Everyone gets 1; the rest is shared by weight. Miscellaneous weighs more than all the others, so it is largest.
			const int32 Count = Lines.Num();
			TArray<int32> Weights;
			int32 MaxOther = 0;
			for (int32 i = 0; i < Count - 1; ++i)
			{
				Weights.Add(Rng.RandRange(2, 5));
				MaxOther = FMath::Max(MaxOther, Weights.Last());
			}
			Weights.Add(MaxOther + 3);
			int32 TotalWeight = 0;
			for (int32 W : Weights) { TotalWeight += W; }

			int32 Spare = Deductions - Count;
			int32 Given = 0;
			for (int32 i = 0; i < Count; ++i)
			{
				const int32 Share = (int32)((int64)Spare * Weights[i] / TotalWeight);
				Lines[i].Amount = 1 + Share;
				Given += Share;
			}
			Lines.Last().Amount += Spare - Given; // remainder to Miscellaneous
			R.Lines = MoveTemp(Lines);
		}

		// She keeps the highest rarity (first on ties), but never the only drop.
		if (Input.LootRarities.Num() >= 2)
		{
			int32 Best = 0;
			for (int32 i = 1; i < Input.LootRarities.Num(); ++i)
			{
				if (Input.LootRarities[i] > Input.LootRarities[Best]) { Best = i; }
			}
			R.KeptLootIndex = Best;
		}

		R.FooterIndex = Rng.RandRange(0, Footers().Num() - 1);
		return R;
	}
}

const FJamLedgerResult& FJamLedgerCommit::Commit(const FJamLedgerInput& Input)
{
	if (!bCommitted)
	{
		Result = JamLedger::Compute(Input);
		bCommitted = true;
	}
	return Result;
}
