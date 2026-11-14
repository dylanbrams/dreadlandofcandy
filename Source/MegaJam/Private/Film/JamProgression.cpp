#include "Film/JamProgression.h"
#include "Math/RandomStream.h"

namespace JamProgression
{
	int32 XpForLevel(int32 Level)
	{
		switch (Level)
		{
		case 8: return 100;
		case 9: return 250;
		case 10: return 450;
		default: return 0;
		}
	}

	int32 XpValue(EJamXpSource Source, bool bNumeric)
	{
		if (bNumeric)
		{
			switch (Source)
			{
			case EJamXpSource::Pacify: return 2;
			case EJamXpSource::Treat: return 4;
			case EJamXpSource::Collect: return 2;
			case EJamXpSource::Quest: return 40;
			default: return 0;
			}
		}
		switch (Source)
		{
		case EJamXpSource::Pacify: return 8;
		case EJamXpSource::Treat: return 12;
		default: return 6;
		}
	}

	int32 MilestoneLevel(EJamMilestone Milestone)
	{
		return StartLevel + 1 + (int32)Milestone;
	}

	const TArray<EJamSkill>& ActiveSkills()
	{
		static const TArray<EJamSkill> Skills = { EJamSkill::Lecture, EJamSkill::FormALine, EJamSkill::LastJudgment };
		return Skills;
	}

	int32 MaxRank(EJamSkill Skill)
	{
		switch (Skill)
		{
		case EJamSkill::Lecture: return 4;
		case EJamSkill::FormALine: return 3;
		case EJamSkill::DisappointedSigh: return 3;
		default: return 1;
		}
	}

	FJamSkillRank RankData(EJamSkill Skill, int32 Rank)
	{
		FJamSkillRank R;
		if (Rank < 1 || Rank > MaxRank(Skill)) { return R; }
		const int32 I = Rank - 1;
		switch (Skill)
		{
		case EJamSkill::Lecture:
		{
			const float Damage[] = { 60.f, 75.f, 90.f, 110.f };
			const float Radius[] = { 800.f, 850.f, 900.f, 950.f };
			R.Damage = Damage[I]; R.Radius = Radius[I]; R.Cooldown = 1.5f;
			break;
		}
		case EJamSkill::FormALine:
		{
			const float Damage[] = { 25.f, 30.f, 35.f };
			const float Stop[] = { 1.5f, 2.5f, 3.5f };
			R.Damage = Damage[I]; R.Length = 1200.f; R.Width = 150.f; R.StatusSeconds = Stop[I]; R.Cooldown = 4.f;
			break;
		}
		case EJamSkill::DisappointedSigh:
		{
			const float Ring[] = { 400.f, 500.f, 600.f };
			const float Slow[] = { 0.6f, 0.5f, 0.4f };
			R.Damage = 10.f; R.Radius = Ring[I]; R.StatusSeconds = 3.f; R.SlowFactor = Slow[I]; R.Cooldown = 6.f;
			break;
		}
		default:
			R.Damage = 200.f; R.Radius = 1100.f; R.Cooldown = 30.f;
			break;
		}
		return R;
	}

	const TArray<FJamPassive>& Passives()
	{
		static const TArray<FJamPassive> P = {
			{ TEXT("Brooding"), 3, TEXT("Stand near a cliff 15% more meaningfully.") },
			{ TEXT("Cape Maintenance"), 2, TEXT("Your cape is 10% less damp.") },
			{ TEXT("Standing"), 1, TEXT("You may now stand.") },
		};
		return P;
	}

	int32 PassivePointsSpent()
	{
		int32 N = 0;
		for (const FJamPassive& P : Passives()) { N += P.Rank; }
		return N;
	}

	const TArray<FJamLootDef>& LootTable()
	{
		static const TArray<FJamLootDef> T = []()
		{
			struct FRow { const TCHAR* Id; const TCHAR* Name; const TCHAR* Flavour; EJamLootStat Stat; int32 Pct; int32 Rarity; };
			const FRow Rows[] = {
				{ TEXT("Blade"), TEXT("The Unforgiving Blade of the Last Judgment"), TEXT("It has never been swung. It has never needed to be."), EJamLootStat::LectureRadius, 12, 3 },
				{ TEXT("Pauldrons"), TEXT("Pauldrons of I'm Not Angry"), TEXT("Just disappointed."), EJamLootStat::LectureRadius, 10, 2 },
				{ TEXT("Greaves"), TEXT("Greaves of Walking Briskly Toward the Problem"), TEXT("Running would be undignified."), EJamLootStat::MoveSpeed, 8, 2 },
				{ TEXT("Helm"), TEXT("Helm of Respected Queueing"), TEXT("No cutting. No exceptions. No mercy."), EJamLootStat::LectureCooldown, -10, 1 },
				{ TEXT("Doomplate"), TEXT("Doomplate of Reasonable Bedtimes"), TEXT("Forged in the fires of half past eight."), EJamLootStat::LectureCooldown, -12, 2 },
				{ TEXT("Gauntlets"), TEXT("Gauntlets of Firm But Fair"), TEXT("The left one is Firm."), EJamLootStat::LectureRadius, 6, 1 },
				{ TEXT("Toothbrush"), TEXT("Toothbrush of Uncompromising Governance"), TEXT("Soft bristles. Hard truths."), EJamLootStat::LectureCooldown, -8, 1 },
				{ TEXT("Cloak"), TEXT("Cloak of Dramatic Exits"), TEXT("No exits were found."), EJamLootStat::MoveSpeed, 5, 0 },
				{ TEXT("Napkin"), TEXT("The Eternally Slightly Damp Napkin"), TEXT("Nobody knows why. Nobody has asked."), EJamLootStat::Everything, 1, 0 },
				{ TEXT("Amulet"), TEXT("Amulet of the Indoor Voice (Cursed)"), TEXT("Whispers the lecture. The rats lean in."), EJamLootStat::LectureRadius, -5, 0 },
				{ TEXT("Ring"), TEXT("Ring of Having Read the Terms"), TEXT("The only one ever forged."), EJamLootStat::MoveSpeed, 7, 1 },
				{ TEXT("Celery"), TEXT("Celery of Unreasonable Authority"), TEXT("Crisp."), EJamLootStat::LectureRadius, 15, 3 },
			};
			TArray<FJamLootDef> Out;
			for (const FRow& R : Rows)
			{
				FJamLootDef D;
				D.Id = R.Id; D.Name = R.Name; D.Flavour = R.Flavour; D.Stat = R.Stat; D.Percent = R.Pct; D.Rarity = R.Rarity;
				Out.Add(MoveTemp(D));
			}
			return Out;
		}();
		return T;
	}

	TArray<int32> RollLoot(int32 Seed, int32 Count)
	{
		const TArray<FJamLootDef>& T = LootTable();
		Count = FMath::Clamp(Count, 1, T.Num());
		FRandomStream Rng(Seed);
		TArray<int32> Pool;
		for (int32 i = 0; i < T.Num(); ++i) { Pool.Add(i); }
		for (int32 i = Pool.Num() - 1; i > 0; --i) { Pool.Swap(i, Rng.RandRange(0, i)); }
		Pool.SetNum(Count);
		// Order the cards worst to best so the flip reveals the prize last (stable on ties).
		Pool.StableSort([&T](int32 A, int32 B) { return T[A].Rarity < T[B].Rarity; });
		return Pool;
	}

	const TCHAR* RarityName(int32 Rarity, bool bDirectorsPick)
	{
		if (bDirectorsPick) { return TEXT("Director's Personal Collection"); }
		switch (Rarity)
		{
		case 0: return TEXT("Common");
		case 1: return TEXT("Uncommon");
		case 2: return TEXT("Rare");
		default: return TEXT("Legendary");
		}
	}
}

FJamProgression::FJamProgression() {}

int32 FJamProgression::GetSpent() const
{
	int32 Spent = 0;
	for (int32 S = 0; S < (int32)EJamSkill::Count; ++S)
	{
		const EJamSkill Skill = (EJamSkill)S;
		if (Skill == EJamSkill::LastJudgment || Skill == EJamSkill::DisappointedSigh) { continue; }
		// Starting ranks are a grant; only upgrades use points earned during the film.
		Spent += Ranks[S] - 2;
	}
	return Spent;
}

void FJamProgression::GrantLevels(int32 NewLevel)
{
	if (NewLevel <= Level) { return; }
	Points += NewLevel - Level;
	Level = NewLevel;
	if (Level >= JamProgression::MaxLevel) { Ranks[(int32)EJamSkill::LastJudgment] = 1; }
}

int32 FJamProgression::AddXp(EJamXpSource Source)
{
	if (bNumericXp)
	{
		const int32 Before = Xp;
		Xp = FMath::Min(JamProgression::XpForLevel(JamProgression::MaxLevel), Xp + JamProgression::XpValue(Source, true));
		while (Level < JamProgression::MaxLevel && Xp >= JamProgression::XpForLevel(Level + 1)) { GrantLevels(Level + 1); }
		return Xp - Before;
	}
	// Held one short of the next level; only a card raises the level.
	const int32 Cap = Level >= JamProgression::MaxLevel ? INT32_MAX : JamProgression::XpForLevel(Level + 1) - 1;
	const int32 Before = Xp;
	Xp = FMath::Min(Cap, Xp + JamProgression::XpValue(Source));
	return Xp - Before;
}

int32 FJamProgression::ReachMilestone(EJamMilestone Milestone)
{
	if (bNumericXp) { return 0; }
	const int32 Target = JamProgression::MilestoneLevel(Milestone);
	if (Target <= Level) { return 0; }
	const int32 Gained = Target - Level;
	GrantLevels(Target);
	Xp = FMath::Max(Xp, JamProgression::XpForLevel(Level));
	return Gained;
}

int32 FJamProgression::CompleteQuest(int32 SceneIndex)
{
	if (!bNumericXp || SceneIndex < 0 || SceneIndex >= 3 || (CompletedQuests & (1 << SceneIndex))) { return 0; }
	CompletedQuests |= (1 << SceneIndex);
	const int32 Before = Level;
	AddXp(EJamXpSource::Quest);
	return Level - Before;
}

float FJamProgression::GetLevelProgress() const
{
	if (Level >= JamProgression::MaxLevel) { return 1.f; }
	const int32 Lo = JamProgression::XpForLevel(Level);
	const int32 Hi = JamProgression::XpForLevel(Level + 1);
	return FMath::Clamp((float)(Xp - Lo) / (float)(Hi - Lo), 0.f, 1.f);
}

bool FJamProgression::CanSpendPoint(EJamSkill Skill) const
{
	if (Points <= 0 || Skill == EJamSkill::LastJudgment || Skill == EJamSkill::DisappointedSigh || Skill >= EJamSkill::Count) { return false; }
	return Ranks[(int32)Skill] < JamProgression::MaxRank(Skill);
}

bool FJamProgression::SpendPoint(EJamSkill Skill)
{
	if (!CanSpendPoint(Skill)) { return false; }
	++Ranks[(int32)Skill];
	--Points;
	return true;
}

void FJamProgression::AddLoot(int32 LootIndex)
{
	if (JamProgression::LootTable().IsValidIndex(LootIndex)) { Loot.Add(LootIndex); }
}

namespace
{
	/** Multiplier for one stat from all equipped loot. */
	float LootMultiplier(const TArray<int32>& Loot, EJamLootStat Stat)
	{
		int32 Percent = 0;
		for (int32 Index : Loot)
		{
			const FJamLootDef& D = JamProgression::LootTable()[Index];
			if (D.Stat == Stat) { Percent += D.Percent; }
			// "+1% everything" is better for the hero in every stat, so it shortens the cooldown.
			else if (D.Stat == EJamLootStat::Everything) { Percent += Stat == EJamLootStat::LectureCooldown ? -D.Percent : D.Percent; }
		}
		return 1.f + Percent / 100.f;
	}
}

FJamStatBlock FJamProgression::GetStats() const
{
	const FJamSkillRank Lecture = JamProgression::RankData(EJamSkill::Lecture, Ranks[(int32)EJamSkill::Lecture]);
	// Everything-loot (the napkin) is counted once by each stat's multiplier below.
	const float PowerMult = LootMultiplier(Loot, EJamLootStat::Everything);
	const float CooldownMult = FMath::Max(0.1f, LootMultiplier(Loot, EJamLootStat::LectureCooldown));

	FJamStatBlock S;
	S.LecturePower = Lecture.Damage * PowerMult;
	S.Radius = Lecture.Radius * LootMultiplier(Loot, EJamLootStat::LectureRadius);
	S.CooldownRecovery = 1.f / CooldownMult;
	S.MoveSpeed = 600.f * LootMultiplier(Loot, EJamLootStat::MoveSpeed);
	return S;
}

FJamSkillRank FJamProgression::GetSkillNumbers(EJamSkill Skill) const
{
	FJamSkillRank R = JamProgression::RankData(Skill, Ranks[(int32)Skill]);
	const FJamStatBlock S = GetStats();
	const float RadiusMult = LootMultiplier(Loot, EJamLootStat::LectureRadius);
	R.Damage *= LootMultiplier(Loot, EJamLootStat::Everything);
	R.Radius *= RadiusMult;
	R.Length *= RadiusMult;
	R.Cooldown /= S.CooldownRecovery;
	return R;
}

const TCHAR* JamProgression::SkillName(EJamSkill Skill)
{
	switch (Skill)
	{
	case EJamSkill::Lecture:          return TEXT("Lecture");
	case EJamSkill::FormALine:        return TEXT("Form A Line");
	case EJamSkill::DisappointedSigh: return TEXT("Disappointed Sigh");
	case EJamSkill::LastJudgment:     return TEXT("The Last Judgment");
	default:                          return TEXT("?");
	}
}
