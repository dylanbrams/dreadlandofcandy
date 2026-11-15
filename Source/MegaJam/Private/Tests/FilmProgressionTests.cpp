#include "Misc/AutomationTest.h"
#include "Film/JamProgression.h"
#include "World/JamVillageQuests.h"
#include "Math/RandomStream.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamFilmFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace
{
	int32 LootIndex(const TCHAR* Id)
	{
		return JamProgression::LootTable().IndexOfByPredicate([Id](const FJamLootDef& D) { return D.Id == FName(Id); });
	}

	/** points unspent + points spent == levels gained, always. */
	bool Balanced(const FJamProgression& P)
	{
		return P.GetPoints() + P.GetSpent() == P.GetLevel() - JamProgression::StartLevel
			&& P.GetPoints() >= 0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmProgStart, "Jam.Film.Progression.Start", JamFilmFlags)
bool FJamFilmProgStart::RunTest(const FString&)
{
	FJamProgression P;
	TestEqual(TEXT("level one"), P.GetLevel(), 1);
	TestEqual(TEXT("start level constant"), JamProgression::StartLevel, 1);
	TestEqual(TEXT("no points"), P.GetPoints(), 0);
	TestTrue(TEXT("lecture owned"), P.IsUnlocked(EJamSkill::Lecture));
	TestTrue(TEXT("signature owned"), P.IsUnlocked(EJamSkill::FormALine));
	TestEqual(TEXT("basic starts rank one"), P.GetRank(EJamSkill::Lecture), 1);
	TestEqual(TEXT("signature starts rank one"), P.GetRank(EJamSkill::FormALine), 1);
	TestEqual(TEXT("three active slots"), JamProgression::ActiveSkills().Num(), 3);
	TestFalse(TEXT("sigh removed from active roster"), JamProgression::ActiveSkills().Contains(EJamSkill::DisappointedSigh));
	TestFalse(TEXT("sigh not owned"), P.IsUnlocked(EJamSkill::DisappointedSigh));
	TestTrue(TEXT("ultimate owned at start"), P.IsUnlocked(EJamSkill::LastJudgment));
	TestEqual(TEXT("ultimate starts rank one"), P.GetRank(EJamSkill::LastJudgment), 1);
	TestTrue(TEXT("balanced"), Balanced(P));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmProgLevels, "Jam.Film.Progression.LevelsLandOnCards", JamFilmFlags)
bool FJamFilmProgLevels::RunTest(const FString&)
{
	FJamProgression P;
	// Grind far more XP than the game can give: the level must not move.
	for (int32 i = 0; i < 500; ++i) { P.AddXp(EJamXpSource::Treat); }
	TestEqual(TEXT("level held until the card"), P.GetLevel(), 1);
	TestEqual(TEXT("bar full but short"), P.GetXp(), JamProgression::XpForLevel(2) - 1);
	TestEqual(TEXT("no free points"), P.GetPoints(), 0);

	TestEqual(TEXT("scene 1 wrap gives one level"), P.ReachMilestone(EJamMilestone::Scene1Wrap), 1);
	TestEqual(TEXT("level 2"), P.GetLevel(), 2);
	TestEqual(TEXT("one point"), P.GetPoints(), 1);
	TestEqual(TEXT("card is idempotent"), P.ReachMilestone(EJamMilestone::Scene1Wrap), 0);
	TestEqual(TEXT("still one point"), P.GetPoints(), 1);
	TestEqual(TEXT("xp sits on the threshold"), P.GetXp(), JamProgression::XpForLevel(2));
	TestEqual(TEXT("bar restarts"), P.GetLevelProgress(), 0.f);

	P.ReachMilestone(EJamMilestone::Scene2Wrap);
	TestEqual(TEXT("level 3"), P.GetLevel(), 3);
	TestEqual(TEXT("ultimate remains rank one"), P.GetRank(EJamSkill::LastJudgment), 1);
	P.ReachMilestone(EJamMilestone::Scene3FinalNote);
	TestEqual(TEXT("level 4"), P.GetLevel(), 4);
	TestEqual(TEXT("three points across 2 to 4"), P.GetPoints(), 3);
	TestEqual(TEXT("ultimate stays rank one"), P.GetRank(EJamSkill::LastJudgment), 1);

	// Skipping a card still grants every level's point.
	FJamProgression Q;
	TestEqual(TEXT("jump to 4"), Q.ReachMilestone(EJamMilestone::Scene3FinalNote), 3);
	TestEqual(TEXT("three points"), Q.GetPoints(), 3);
	TestTrue(TEXT("balanced"), Balanced(Q));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmProgSpend, "Jam.Film.Progression.PointsNeverExceedLevels", JamFilmFlags)
bool FJamFilmProgSpend::RunTest(const FString&)
{
	FJamProgression P;
	TestFalse(TEXT("no point to spend"), P.SpendPoint(EJamSkill::FormALine));
	P.ReachMilestone(EJamMilestone::Scene1Wrap);
	TestFalse(TEXT("ultimate is never bought"), P.SpendPoint(EJamSkill::LastJudgment));
	TestTrue(TEXT("buy the line"), P.SpendPoint(EJamSkill::FormALine));
	TestEqual(TEXT("signature upgrades to rank two"), P.GetRank(EJamSkill::FormALine), 2);
	TestFalse(TEXT("retired sigh cannot receive points"), P.SpendPoint(EJamSkill::DisappointedSigh));
	TestFalse(TEXT("out of points"), P.SpendPoint(EJamSkill::FormALine));
	TestTrue(TEXT("balanced"), Balanced(P));

	// Random play: whatever is spent, the books balance and ranks stay in range.
	FRandomStream Rng(42);
	for (int32 Run = 0; Run < 100; ++Run)
	{
		FJamProgression R;
		for (int32 Step = 0; Step < 60; ++Step)
		{
			switch (Rng.RandRange(0, 3))
			{
			case 0: R.AddXp((EJamXpSource)Rng.RandRange(0, 2)); break;
			case 1: R.ReachMilestone((EJamMilestone)Rng.RandRange(0, 2)); break;
			default: R.SpendPoint((EJamSkill)Rng.RandRange(0, (int32)EJamSkill::Count - 1)); break;
			}
			if (!Balanced(R)) { AddError(TEXT("books out of balance")); return false; }
			for (int32 S = 0; S < (int32)EJamSkill::Count; ++S)
			{
				if (R.GetRank((EJamSkill)S) > JamProgression::MaxRank((EJamSkill)S)) { AddError(TEXT("rank above max")); return false; }
			}
		}
		TestTrue(TEXT("never more than three skill points"), R.GetSpent() <= 3);
	}

	// Three points: all into the basic; the ultimate stays fixed.
	FJamProgression L;
	L.ReachMilestone(EJamMilestone::Scene3FinalNote);
	while (L.SpendPoint(EJamSkill::Lecture)) {}
	TestEqual(TEXT("rank 4"), L.GetRank(EJamSkill::Lecture), 4);
	TestEqual(TEXT("all points used"), L.GetPoints(), 0);
	TestEqual(TEXT("lecture rank 4 does 96"), JamProgression::RankData(EJamSkill::Lecture, 4).Damage, 96.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmProgStats, "Jam.Film.Progression.StatsTotal", JamFilmFlags)
bool FJamFilmProgStats::RunTest(const FString&)
{
	FJamProgression P;
	FJamStatBlock S = P.GetStats();
	TestEqual(TEXT("base power"), S.LecturePower, 60.f);
	TestEqual(TEXT("base radius"), S.Radius, 800.f);
	TestEqual(TEXT("base recovery"), S.CooldownRecovery, 1.f);
	TestEqual(TEXT("base speed"), S.MoveSpeed, 600.f);

	P.AddLoot(LootIndex(TEXT("Blade")));      // +12% radius
	P.AddLoot(LootIndex(TEXT("Pauldrons")));  // +10% radius
	P.AddLoot(LootIndex(TEXT("Amulet")));     // -5% radius
	P.AddLoot(LootIndex(TEXT("Greaves")));    // +8% speed
	P.AddLoot(LootIndex(TEXT("Helm")));       // -10% cooldown
	S = P.GetStats();
	TestEqual(TEXT("radius 800 x 1.17"), S.Radius, 800.f * 1.17f, 0.01f);
	TestEqual(TEXT("speed 600 x 1.08"), S.MoveSpeed, 648.f, 0.01f);
	TestEqual(TEXT("recovery is 1 / 0.9"), S.CooldownRecovery, 1.f / 0.9f, 0.0001f);
	TestEqual(TEXT("power untouched"), S.LecturePower, 60.f);

	// The skill numbers carry the loot: a bigger cone, a shorter cooldown.
	const FJamSkillRank Cone = P.GetSkillNumbers(EJamSkill::Lecture);
	TestEqual(TEXT("cone radius matches the sheet"), Cone.Radius, S.Radius, 0.01f);
	TestEqual(TEXT("cooldown 1.35"), Cone.Cooldown, 1.35f, 0.001f);

	// The napkin helps every stat, including the cooldown.
	FJamProgression N;
	N.AddLoot(LootIndex(TEXT("Napkin")));
	S = N.GetStats();
	TestEqual(TEXT("napkin power"), S.LecturePower, 60.6f, 0.001f);
	TestEqual(TEXT("napkin radius"), S.Radius, 808.f, 0.01f);
	TestEqual(TEXT("napkin speed"), S.MoveSpeed, 606.f, 0.01f);
	TestTrue(TEXT("napkin shortens the cooldown"), S.CooldownRecovery > 1.f);

	// Ranks change the skill; points change the lecture's totals.
	FJamProgression R;
	R.ReachMilestone(EJamMilestone::Scene1Wrap);
	R.SpendPoint(EJamSkill::Lecture);
	TestEqual(TEXT("rank 2 power at level 2"), R.GetStats().LecturePower, 72.f * JamProgression::LevelPowerMult(2), 0.01f);
	TestEqual(TEXT("rank 2 radius"), R.GetStats().Radius, 850.f);

	TestEqual(TEXT("invalid loot ignored"), (N.AddLoot(999), N.GetLoot().Num()), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmProgTables, "Jam.Film.Progression.Tables", JamFilmFlags)
bool FJamFilmProgTables::RunTest(const FString&)
{
	// Ranks only ever get stronger.
	for (int32 S = 0; S < (int32)EJamSkill::Count; ++S)
	{
		const EJamSkill Skill = (EJamSkill)S;
		for (int32 Rank = 2; Rank <= JamProgression::MaxRank(Skill); ++Rank)
		{
			const FJamSkillRank A = JamProgression::RankData(Skill, Rank - 1), B = JamProgression::RankData(Skill, Rank);
			TestTrue(TEXT("damage does not fall"), B.Damage >= A.Damage);
			TestTrue(TEXT("radius does not fall"), B.Radius >= A.Radius);
			TestTrue(TEXT("status lasts at least as long"), B.StatusSeconds >= A.StatusSeconds);
			TestTrue(TEXT("slow does not weaken"), B.SlowFactor <= A.SlowFactor);
		}
		TestEqual(TEXT("rank 0 is empty"), JamProgression::RankData(Skill, 0).Damage, 0.f);
		TestEqual(TEXT("past max is empty"), JamProgression::RankData(Skill, JamProgression::MaxRank(Skill) + 1).Damage, 0.f);
	}
	TestEqual(TEXT("lecture matches the as-built cone"), JamProgression::RankData(EJamSkill::Lecture, 1).Radius, 800.f);
	TestEqual(TEXT("and 60 Resolve"), JamProgression::RankData(EJamSkill::Lecture, 1).Damage, 60.f);

	const TArray<FJamLootDef>& Loot = JamProgression::LootTable();
	TestEqual(TEXT("twelve items in Jokes.md"), Loot.Num(), 12);
	TSet<FName> Ids;
	for (const FJamLootDef& D : Loot)
	{
		TestFalse(TEXT("unique id"), Ids.Contains(D.Id));
		Ids.Add(D.Id);
		TestFalse(TEXT("named"), D.Name.IsEmpty());
	}

	TestEqual(TEXT("three passives"), JamProgression::Passives().Num(), 3);
	TestEqual(TEXT("level 2 on card 1"), JamProgression::MilestoneLevel(EJamMilestone::Scene1Wrap), 2);
	TestEqual(TEXT("level 4 on card 3"), JamProgression::MilestoneLevel(EJamMilestone::Scene3FinalNote), 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmProgCurve, "Jam.Film.Progression.LevelCurve", JamFilmFlags)
bool FJamFilmProgCurve::RunTest(const FString&)
{
	TestEqual(TEXT("level 1 costs nothing"), JamProgression::XpForLevel(1), 0);
	for (int32 L = 2; L <= JamProgression::MaxLevel; ++L)
	{
		TestTrue(TEXT("curve rises"), JamProgression::XpForLevel(L) > JamProgression::XpForLevel(L - 1));
	}
	TestEqual(TEXT("level 10 threshold"), JamProgression::XpForLevel(10), 1080);

	FJamProgression P;
	P.EnableNumericXp();
	while (P.GetXp() < JamProgression::XpForLevel(10)) { P.AddXp(EJamXpSource::Quest); }
	TestEqual(TEXT("reaches level 10"), P.GetLevel(), 10);
	TestEqual(TEXT("nine points"), P.GetPoints(), 9);
	while (P.SpendPoint(EJamSkill::Lecture)) {}
	TestEqual(TEXT("lecture maxed"), P.GetRank(EJamSkill::Lecture), JamProgression::MaxRank(EJamSkill::Lecture));
	while (P.SpendPoint(EJamSkill::FormALine)) {}
	TestEqual(TEXT("all nine points spendable"), P.GetPoints(), 0);
	TestEqual(TEXT("signature maxed"), P.GetRank(EJamSkill::FormALine), JamProgression::MaxRank(EJamSkill::FormALine));
	TestTrue(TEXT("balanced"), Balanced(P));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmProgLevelPower, "Jam.Film.Progression.LevelPowerAndToughness", JamFilmFlags)
bool FJamFilmProgLevelPower::RunTest(const FString&)
{
	TestEqual(TEXT("level 1 power x1"), JamProgression::LevelPowerMult(1), 1.f);
	TestTrue(TEXT("power rises with level"), JamProgression::LevelPowerMult(10) > JamProgression::LevelPowerMult(5));
	TestEqual(TEXT("level 1 takes full damage"), JamProgression::DignityDamageScale(1), 1.f);
	TestTrue(TEXT("tougher at higher level"), JamProgression::DignityDamageScale(10) < JamProgression::DignityDamageScale(2));
	TestTrue(TEXT("still below one"), JamProgression::DignityDamageScale(2) < 1.f);

	FJamProgression Low, High;
	High.EnableNumericXp();
	while (High.GetLevel() < 8) { High.AddXp(EJamXpSource::Quest); }
	TestTrue(TEXT("stats damage rises with level"), High.GetStats().LecturePower > Low.GetStats().LecturePower);
	TestTrue(TEXT("skill damage rises with level"), High.GetSkillNumbers(EJamSkill::FormALine).Damage > Low.GetSkillNumbers(EJamSkill::FormALine).Damage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmThreatScale, "Jam.Village.ThreatScale", JamFilmFlags)
bool FJamFilmThreatScale::RunTest(const FString&)
{
	TestEqual(TEXT("no jobs, no change"), JamVillageQuests::ThreatScale(0), 1.f);
	TestEqual(TEXT("no jobs, no extra damage"), JamVillageQuests::ThreatDamageScale(0), 1.f);
	TestTrue(TEXT("resolve grows with jobs"), JamVillageQuests::ThreatScale(10) > JamVillageQuests::ThreatScale(3));
	TestTrue(TEXT("damage grows with jobs"), JamVillageQuests::ThreatDamageScale(10) > JamVillageQuests::ThreatDamageScale(3));
	TestEqual(TEXT("ten jobs doubles resolve"), JamVillageQuests::ThreatScale(10), 2.f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmProgLoot, "Jam.Film.Progression.RollLoot", JamFilmFlags)
bool FJamFilmProgLoot::RunTest(const FString&)
{
	const TArray<FJamLootDef>& T = JamProgression::LootTable();
	bool bVaries = false;
	const TArray<int32> Base = JamProgression::RollLoot(0);
	for (int32 Seed = 0; Seed < 200; ++Seed)
	{
		const TArray<int32> Roll = JamProgression::RollLoot(Seed);
		TestEqual(TEXT("three cards"), Roll.Num(), 3);
		TSet<int32> Unique(Roll);
		TestEqual(TEXT("distinct"), Unique.Num(), 3);
		TestTrue(TEXT("sorted by rarity so the best flips last"), T[Roll[0]].Rarity <= T[Roll[1]].Rarity && T[Roll[1]].Rarity <= T[Roll[2]].Rarity);
		TestTrue(TEXT("deterministic"), Roll == JamProgression::RollLoot(Seed));
		bVaries |= Roll != Base;
	}
	TestTrue(TEXT("seeds differ"), bVaries);
	return true;
}

#endif
