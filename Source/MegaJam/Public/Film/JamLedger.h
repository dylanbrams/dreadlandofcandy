#pragma once

#include "CoreMinimal.h"

/**
 * The Production Ledger (BUILD_SPEC 3 "wrap card and ledger", Jokes.md "The Production Ledger").
 * Pure function: gross in, itemized deductions and the hero's cut out. Deterministic for a seed.
 * Rules that stay: the hero's cut never drops below the floor, deductions never exceed gross minus floor,
 * the director never keeps the only loot drop, and the outcome commits once.
 */

struct MEGAJAM_API FJamLedgerInput
{
	int32 Gross = 0;
	/** The hero's guaranteed minimum. Clamped to Gross. */
	int32 Floor = 0;
	/** Seed for the choice of line items; the game uses scene index and take count. */
	int32 Seed = 0;
	/** Rarity of each loot drop (0 Common .. 4 Director's Personal Collection). Only the count and order matter to the rules. */
	TArray<int32> LootRarities;
};

struct MEGAJAM_API FJamLedgerLine
{
	/** Key into the table below; the UI prints Item and Note. */
	FName Id;
	FString Item;
	FString Note;
	int32 Amount = 0;
};

struct MEGAJAM_API FJamLedgerResult
{
	int32 Gross = 0;
	TArray<FJamLedgerLine> Lines;
	int32 DeductionsTotal = 0;
	int32 HeroCut = 0;
	/** Index into LootRarities that the director keeps, or INDEX_NONE (fewer than two drops). */
	int32 KeptLootIndex = INDEX_NONE;
	/** Index into JamLedger::Footers(). */
	int32 FooterIndex = 0;
};

namespace JamLedger
{
	MEGAJAM_API FJamLedgerResult Compute(const FJamLedgerInput& Input);

	MEGAJAM_API const TArray<FString>& Footers();
	/** Number of line items the table offers (for tests). */
	MEGAJAM_API int32 TableSize();
}

/** Commits a ledger outcome once; later calls return the stored result without recomputing. */
class MEGAJAM_API FJamLedgerCommit
{
public:
	/** Returns the committed result. The first call computes it; every later call returns the same one. */
	const FJamLedgerResult& Commit(const FJamLedgerInput& Input);
	bool IsCommitted() const { return bCommitted; }
	void Reset() { bCommitted = false; Result = FJamLedgerResult(); }

private:
	bool bCommitted = false;
	FJamLedgerResult Result;
};
