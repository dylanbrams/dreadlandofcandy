#pragma once

#include "CoreMinimal.h"

/**
 * Levels, points, skill ranks, stats and loot (BUILD_SPEC 3c). Headless; lives on the PlayerState.
 *
 * The hero starts at level 7; basic/signature ranks are 2 and the ultimate stays rank 1.
 * Open earns levels directly from numeric XP, one skill point per level. Quest bonuses are once-only.
 * Legacy Backlot holds ordinary XP below the next threshold and advances on its three cards.
 */

enum class EJamSkill : uint8
{
	Lecture,          // slot 1, cone, owned at start, ranks 1 to 4
	FormALine,        // slot 2, line, bought with a point, ranks 1 to 3
	DisappointedSigh, // slot 3, ring around the hero, bought with a point, ranks 1 to 3
	LastJudgment,     // slot 4, big ring, free at level 10, rank 1
	Count
};

enum class EJamXpSource : uint8 { Pacify, Treat, Collect, Quest };

enum class EJamMilestone : uint8 { Scene1Wrap, Scene2Wrap, Scene3FinalNote };

enum class EJamLootStat : uint8 { LectureRadius, LectureCooldown, MoveSpeed, Everything };

/** What a skill does at one rank, before stats. Unused fields are 0. */
struct MEGAJAM_API FJamSkillRank
{
	float Damage = 0.f;        // Resolve
	float Radius = 0.f;        // cone length, or ring radius (uu)
	float Length = 0.f;        // line length (uu)
	float Width = 0.f;         // line width (uu)
	float StatusSeconds = 0.f; // Flustered stop (line) or slow (sigh)
	float SlowFactor = 1.f;    // move speed multiplier while slowed (sigh)
	float Cooldown = 0.f;      // seconds
};

/** The four numbers on the character sheet, totals for the Lecture at its current rank. */
struct MEGAJAM_API FJamStatBlock
{
	float LecturePower = 0.f;      // Resolve per hit
	float Radius = 0.f;            // uu
	float CooldownRecovery = 1.f;  // 1.0 = normal; 1.25 = cooldown 20% shorter
	float MoveSpeed = 0.f;         // uu/s
};

struct MEGAJAM_API FJamLootDef
{
	FName Id;
	FString Name;
	FString Flavour;
	EJamLootStat Stat = EJamLootStat::LectureRadius;
	/** Signed percent of the stat: +12 radius, -10 cooldown, -5 radius for the cursed amulet. */
	int32 Percent = 0;
	/** 0 Common, 1 Uncommon, 2 Rare, 3 Legendary. The one she keeps is shown as Director's Personal Collection. */
	int32 Rarity = 0;
};

struct MEGAJAM_API FJamPassive
{
	FString Name;
	int32 Rank = 0;
	FString Tooltip;
};

namespace JamProgression
{
	constexpr int32 StartLevel = 7;
	constexpr int32 MaxLevel = 10;
	/** Fixes3 #26: past MaxLevel XP keeps counting as encore EXP; each EncoreXpPerLevel is an encore level (+5% power). */
	constexpr int32 EncoreXpPerLevel = 200;
	constexpr float EncorePowerPerLevel = 0.05f;

	/** XP at which the given level (8 to 10) is reached; 0 for the start level. */
	MEGAJAM_API int32 XpForLevel(int32 Level);
	MEGAJAM_API int32 XpValue(EJamXpSource Source, bool bNumeric = false);
	MEGAJAM_API int32 MilestoneLevel(EJamMilestone Milestone);

	/** The jam uses basic, signature and ultimate; the old Sigh enum remains for asset compatibility. */
	MEGAJAM_API const TArray<EJamSkill>& ActiveSkills();
	MEGAJAM_API int32 MaxRank(EJamSkill Skill);
	/** Numbers for a skill at a rank (1-based); rank 0 returns zeros. */
	MEGAJAM_API FJamSkillRank RankData(EJamSkill Skill, int32 Rank);

	/** The six points already spent (levels 1 to 6). */
	MEGAJAM_API const TArray<FJamPassive>& Passives();
	MEGAJAM_API int32 PassivePointsSpent();

	MEGAJAM_API const TArray<FJamLootDef>& LootTable();
	/** Three (or Count) distinct drops, deterministic for the seed, always including the highest rarity on offer. */
	MEGAJAM_API TArray<int32> RollLoot(int32 Seed, int32 Count = 3);
	MEGAJAM_API const TCHAR* RarityName(int32 Rarity, bool bDirectorsPick);
	/** "Lecture", "Form A Line", "Disappointed Sigh", "The Last Judgment". */
	MEGAJAM_API const TCHAR* SkillName(EJamSkill Skill);
}

class MEGAJAM_API FJamProgression
{
public:
	FJamProgression();

	int32 GetLevel() const { return Level; }
	int32 GetXp() const { return Xp; }
	/** Encore levels earned past MaxLevel (numeric XP only). */
	int32 GetEncoreLevel() const;
	/** Encore EXP into the current encore level (0 to EncoreXpPerLevel - 1). */
	int32 GetEncoreXp() const;
	/** Open earns levels directly from XP; the legacy card progression remains for Backlot. */
	void EnableNumericXp() { bNumericXp = true; }
	bool IsNumericXp() const { return bNumericXp; }
	/** A once-only bonus per authored quest. Returns levels gained. */
	int32 CompleteQuest(int32 SceneIndex);
	/** Unspent skill points. */
	int32 GetPoints() const { return Points; }
	int32 GetRank(EJamSkill Skill) const { return Ranks[(int32)Skill]; }
	bool IsUnlocked(EJamSkill Skill) const { return Ranks[(int32)Skill] > 0; }
	/** Points spent on skills, not counting the six passives. */
	int32 GetSpent() const;

	/** Awards XP; Open levels immediately, legacy Backlot holds below its next card. Returns the XP actually added. */
	int32 AddXp(EJamXpSource Source);
	/** Legacy Backlot card: raises the level to the milestone's; no-op in numeric mode. Returns the levels gained (0 if already reached). */
	int32 ReachMilestone(EJamMilestone Milestone);

	/** XP bar: 0 to 1 within the current level; 1 at the top level. */
	float GetLevelProgress() const;

	/** Spends one point: buys rank 1 of a skill or raises its rank. False (nothing changes) if not allowed. */
	bool SpendPoint(EJamSkill Skill);
	bool CanSpendPoint(EJamSkill Skill) const;

	/** Loot indices into JamProgression::LootTable(). All equipped items stack. */
	void AddLoot(int32 LootIndex);
	const TArray<int32>& GetLoot() const { return Loot; }

	FJamStatBlock GetStats() const;
	/** A skill at its current rank with loot applied (power to Damage, radius to Radius and Length, recovery to Cooldown). */
	FJamSkillRank GetSkillNumbers(EJamSkill Skill) const;

private:
	void GrantLevels(int32 NewLevel);

	int32 Xp = 0;
	int32 Level = JamProgression::StartLevel;
	int32 Points = 0;
	bool bNumericXp = false;
	uint8 CompletedQuests = 0;
	int32 Ranks[(int32)EJamSkill::Count] = { 2, 2, 0, 1 };
	TArray<int32> Loot;
};
