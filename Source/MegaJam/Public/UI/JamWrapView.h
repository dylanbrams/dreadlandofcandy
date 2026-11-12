#pragma once

#include "CoreMinimal.h"
#include "Film/JamLedger.h"
#include "Film/JamProgression.h"

/** One row of the ledger as printed. */
struct MEGAJAM_API FJamWrapLine
{
	FString Item;
	FString Note;
	int32 Amount = 0;
};

/** One loot card. */
struct MEGAJAM_API FJamWrapLoot
{
	FString Name;
	FString Flavour;
	FString Stat;        // "+12% Lecture Radius"
	FString Rarity;      // "Rare", or "Director's Personal Collection" on hers
	int32 RarityLevel = 0;
	FString Stamp;       // RESERVED FOR SEQUEL etc. on hers; empty otherwise
	bool bKept = false;      // Fenella keeps it
	bool bEquipped = false;  // the hero's
};

/** One row of the character sheet: before and after the equip. */
struct MEGAJAM_API FJamWrapStat
{
	FString Name;
	float Before = 0.f;
	float After = 0.f;
	FString Unit;        // " uu", " uu/s", "x"
};

/** Everything the wrap panel prints (BUILD_SPEC 3: the wrap card, ledger and loot). Plain data. */
struct MEGAJAM_API FJamWrapView
{
	bool bVisible = false;
	FString SceneTitle;
	int32 Gross = 0;
	TArray<FJamWrapLine> Lines;
	int32 HeroCut = 0;
	FString Footer;
	TArray<FJamWrapLoot> Loot;
	TArray<FJamWrapStat> Stats;
	FString LevelLine;      // "LEVEL 8!  +1 skill point" or empty
	FString ContinueHint;   // "Hold [E] to continue" or empty (the last wrap waits for the Premiere)
};

/** Everything the game rolls at a wrap, and the view of it. Headless: tests run it without a world. */
struct MEGAJAM_API FJamWrapOutcome
{
	FJamLedgerResult Ledger;
	TArray<int32> LootIndices;   // into JamProgression::LootTable(), worst to best
	int32 KeptSlot = INDEX_NONE; // slot in LootIndices she keeps
	int32 EquippedSlot = INDEX_NONE;
};

namespace JamWrap
{
	/** Base gross of each scene in sprinkles, before the notes' multiplier. */
	MEGAJAM_API int32 BaseGross(int32 SceneIndex);
	/** The hero's guaranteed minimum per wrap. */
	constexpr int32 HeroFloor = 40;

	/**
	 * Rolls the wrap: gross from the scene and the notes' multiplier, three loot drops, the ledger. She keeps the
	 * ledger's pick (the best); the hero gets the best of the rest. Deterministic for the seed.
	 */
	MEGAJAM_API FJamWrapOutcome Roll(int32 SceneIndex, float GrossMultiplier, int32 Seed);

	/** The printed view. Before/After are the progression without and with the equipped item. */
	MEGAJAM_API FJamWrapView MakeView(const FJamWrapOutcome& Outcome, const FString& SceneTitle, const FJamStatBlock& Before,
		const FJamStatBlock& After, int32 LevelsGained, int32 Seed);

	/** "+12% Lecture Radius". */
	MEGAJAM_API FString StatText(const FJamLootDef& Loot);
	/** RESERVED FOR SEQUEL / PRODUCER'S PICK / EVIDENCE, by seed. */
	MEGAJAM_API FString Stamp(int32 Seed);
}
