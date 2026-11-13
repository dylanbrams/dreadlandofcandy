#include "Misc/AutomationTest.h"
#include "Film/JamDignity.h"
#include "Film/JamShoot.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamDignityFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmDignityWhoAttacks, "Jam.Film.Dignity.WhoAttacks", JamDignityFlags)
bool FJamFilmDignityWhoAttacks::RunTest(const FString&)
{
	TestFalse(TEXT("rats never attack"), JamAttack::For(EJamTargetKind::Rat, false).bAttacks);
	TestFalse(TEXT("even very large rats"), JamAttack::For(EJamTargetKind::Rat, true).bAttacks);
	TestFalse(TEXT("almonds never attack"), JamAttack::For(EJamTargetKind::Almond, false).bAttacks);
	const FJamAttackTuning Goblin = JamAttack::For(EJamTargetKind::Goblin, false);
	const FJamAttackTuning Boss = JamAttack::For(EJamTargetKind::Goblin, true);
	const FJamAttackTuning Bug = JamAttack::For(EJamTargetKind::Bug, false);
	TestTrue(TEXT("goblins attack"), Goblin.bAttacks);
	TestTrue(TEXT("bugs attack"), Bug.bAttacks);
	TestTrue(TEXT("the boss attacks"), Boss.bAttacks);
	TestTrue(TEXT("the boss hits hardest"), Boss.Damage > Goblin.Damage && Goblin.Damage > Bug.Damage);
	TestTrue(TEXT("only the boss cannot be lectured out of a charge"), Boss.bUnstoppable && !Goblin.bUnstoppable && !Bug.bUnstoppable);
	TestTrue(TEXT("the boss knocks furthest"), Boss.Knockback > Goblin.Knockback && Goblin.Knockback > Bug.Knockback);
	for (const FJamAttackTuning& T : { Goblin, Boss, Bug })
	{
		TestTrue(TEXT("a hit staggers"), T.StaggerSeconds > 0.f);
		TestTrue(TEXT("hit range is inside charge range"), T.HitRange < T.ChargeRange);
		TestTrue(TEXT("no single hit breaks him from full"), T.Damage < FJamDignity::Max);
		TestTrue(TEXT("a charge is faster than walking"), T.ChargeSpeedScale > 1.f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmDignityMayCharge, "Jam.Film.Dignity.MayCharge", JamDignityFlags)
bool FJamFilmDignityMayCharge::RunTest(const FString&)
{
	const FJamAttackTuning Goblin = JamAttack::For(EJamTargetKind::Goblin, false);
	const float Near = Goblin.ChargeRange - 10.f;
	TestTrue(TEXT("near, loose, ready: charges"), JamAttack::MayCharge(Goblin, true, 0.f, Near, 0));
	TestFalse(TEXT("too far"), JamAttack::MayCharge(Goblin, true, 0.f, Goblin.ChargeRange + 10.f, 0));
	TestFalse(TEXT("pacified or flustered"), JamAttack::MayCharge(Goblin, false, 0.f, Near, 0));
	TestFalse(TEXT("cooling down"), JamAttack::MayCharge(Goblin, true, 0.5f, Near, 0));
	TestFalse(TEXT("too many already charging"), JamAttack::MayCharge(Goblin, true, 0.f, Near, JamAttack::MaxChargers));
	TestTrue(TEXT("room for one more"), JamAttack::MayCharge(Goblin, true, 0.f, Near, JamAttack::MaxChargers - 1));
	TestFalse(TEXT("rats never"), JamAttack::MayCharge(JamAttack::For(EJamTargetKind::Rat, false), true, 0.f, 1.f, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmDignityMeter, "Jam.Film.Dignity.Meter", JamDignityFlags)
bool FJamFilmDignityMeter::RunTest(const FString&)
{
	FJamDignity D;
	D.Reset();
	TestEqual(TEXT("starts full"), D.GetFraction(), 1.f);
	TestFalse(TEXT("a goblin hit does not break him"), D.Hit(20.f, 0.5f));
	TestEqual(TEXT("drained"), D.GetValue(), 80.f);
	TestTrue(TEXT("staggered"), D.IsStaggered());
	TestFalse(TEXT("grace: a second hit straight away is ignored"), D.Hit(20.f, 0.5f));
	TestEqual(TEXT("still 80"), D.GetValue(), 80.f);
	TestEqual(TEXT("one hit counted"), D.GetHits(), 1);
	D.Tick(0.6f);
	TestFalse(TEXT("stagger over"), D.IsStaggered());
	TestFalse(TEXT("grace still on"), D.CanBeHit());
	D.Tick(JamAttack::HitGraceSeconds);
	TestTrue(TEXT("grace over"), D.CanBeHit());

	// Five goblin hits from full break him, and only the last one says so.
	D.Reset();
	int32 Broke = 0;
	for (int32 i = 0; i < 5; ++i) { Broke += D.Hit(20.f, 0.5f) ? 1 : 0; D.Tick(JamAttack::HitGraceSeconds + 0.01f); }
	TestEqual(TEXT("broken once"), Broke, 1);
	TestTrue(TEXT("broken"), D.IsBroken());
	TestFalse(TEXT("no more hits once broken"), D.Hit(20.f, 0.5f));
	D.Reset();
	TestEqual(TEXT("a retake restores it"), D.GetValue(), FJamDignity::Max);
	TestEqual(TEXT("and the hit count"), D.GetHits(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmDignityFailsTake, "Jam.Film.Dignity.FailsTake", JamDignityFlags)
bool FJamFilmDignityFailsTake::RunTest(const FString&)
{
	// The game mode's path: broken Dignity reports a failure; the retake starts the stage again with a full meter.
	FJamShoot Shoot;
	Shoot.Start(JamScenes::Scene2());
	Shoot.Action();
	TestEqual(TEXT("rolling"), Shoot.GetState(), EJamShootState::Rolling);
	FJamDignity D;
	D.Reset();
	const FJamAttackTuning Boss = JamAttack::For(EJamTargetKind::Goblin, true);
	while (!D.IsBroken())
	{
		if (D.Hit(Boss.Damage, Boss.StaggerSeconds)) { Shoot.ReportFail(); }
		D.Tick(JamAttack::HitGraceSeconds + 0.01f);
	}
	TestEqual(TEXT("the take failed"), Shoot.GetState(), EJamShootState::FailedTake);
	TestTrue(TEXT("retake"), Shoot.Retake());
	D.Reset();
	TestEqual(TEXT("take 2"), Shoot.GetTake(), 2);
	TestFalse(TEXT("full again"), D.IsBroken());
	return true;
}

#undef JamDignityFlags

#endif
