#include "Misc/AutomationTest.h"
#include "Film/JamNagScheduler.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamFilmFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace
{
	FJamNagSnapshot At(float Now)
	{
		FJamNagSnapshot S;
		S.Now = Now;
		return S;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmNagTable, "Jam.Film.Nag.Table", JamFilmFlags)
bool FJamFilmNagTable::RunTest(const FString&)
{
	TSet<FName> Ids;
	for (const FJamLine& L : JamLines::All())
	{
		TestFalse(TEXT("ids are unique"), Ids.Contains(L.Id));
		Ids.Add(L.Id);
		TestFalse(TEXT("text is not empty"), L.Text.IsEmpty());
		TestTrue(TEXT("Find round-trips"), JamLines::Find(L.Id) == &L);
	}
	for (int32 i = 0; i < (int32)EJamLineTrigger::Count; ++i)
	{
		const EJamLineTrigger T = (EJamLineTrigger)i;
		const int32 N = JamLines::ForTrigger(T).Num();
		TestTrue(*FString::Printf(TEXT("%s has lines"), JamLines::TriggerName(T)), N >= 1);
		// One-shots and the two-line story beats may be short; everything repeatable needs depth.
		const bool bShortOk = JamLines::IsOnce(T) || T == EJamLineTrigger::ForcedRetake || T == EJamLineTrigger::NarrationCut
			|| T == EJamLineTrigger::Premiere;
		if (!bShortOk)
		{
			TestTrue(*FString::Printf(TEXT("%s has 3+ lines"), JamLines::TriggerName(T)), N >= 3);
		}
	}
	TestEqual(TEXT("deep ambient pools"), JamLines::ForTrigger(EJamLineTrigger::Idle).Num(), 8);
	TestEqual(TEXT("take 1 and 2 use scene start"), JamLines::TriggerForTake(2), EJamLineTrigger::SceneStart);
	TestEqual(TEXT("take 3"), JamLines::TriggerForTake(3), EJamLineTrigger::Take3);
	TestEqual(TEXT("take 9 caps at 6"), JamLines::TriggerForTake(9), EJamLineTrigger::Take6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmNagGap, "Jam.Film.Nag.GlobalGapAndCooldown", JamFilmFlags)
bool FJamFilmNagGap::RunTest(const FString&)
{
	FJamNagScheduler Nag(1);
	TestNotNull(TEXT("first pacified line"), Nag.Request(EJamLineTrigger::Pacified, At(100.f)));
	TestNull(TEXT("inside the global gap"), Nag.Request(EJamLineTrigger::Interact, At(103.f)));
	TestNull(TEXT("same trigger inside its cooldown"), Nag.Request(EJamLineTrigger::Pacified, At(110.f)));
	TestNotNull(TEXT("another trigger after the gap"), Nag.Request(EJamLineTrigger::Interact, At(109.f)));
	TestNotNull(TEXT("same trigger after the cooldown"), Nag.Request(EJamLineTrigger::Pacified, At(121.f)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmNagSilence, "Jam.Film.Nag.SilentAndStory", JamFilmFlags)
bool FJamFilmNagSilence::RunTest(const FString&)
{
	FJamNagScheduler Nag(2);
	FJamNagSnapshot Quiet = At(100.f);
	Quiet.bSilenced = true;
	TestNull(TEXT("ambient is silent during a prompt"), Nag.Request(EJamLineTrigger::Idle, Quiet));
	TestNotNull(TEXT("the CUT! pitch is the prompt's own line"), Nag.Request(EJamLineTrigger::CutPitch, Quiet));
	// Story lines ignore the gap too: back-to-back is fine.
	TestNotNull(TEXT("note answer right after"), Nag.Request(EJamLineTrigger::NoteRefused, At(100.5f)));
	// The story line still resets the gap for ambient chatter.
	TestNull(TEXT("ambient waits after a story line"), Nag.Request(EJamLineTrigger::Pacified, At(103.f)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmNagNoRepeat, "Jam.Film.Nag.NoRepeat", JamFilmFlags)
bool FJamFilmNagNoRepeat::RunTest(const FString&)
{
	// Pacified has five lines: five requests give five different ones, and the sixth starts the pool again.
	FJamNagScheduler Nag(3);
	TSet<FName> Seen;
	float Now = 100.f;
	for (int32 i = 0; i < 5; ++i)
	{
		const FJamLine* L = Nag.Request(EJamLineTrigger::Pacified, At(Now));
		if (TestNotNull(TEXT("line"), L)) { TestFalse(TEXT("no repeat until exhausted"), Seen.Contains(L->Id)); Seen.Add(L->Id); }
		Now += 30.f;
	}
	TestEqual(TEXT("all five used"), Seen.Num(), 5);
	const FJamLine* Sixth = Nag.Request(EJamLineTrigger::Pacified, At(Now));
	TestNotNull(TEXT("pool restarts"), Sixth);

	// Over a long run no line repeats inside the window, in any pool.
	FJamNagScheduler Long(4);
	TArray<FName> History;
	const EJamLineTrigger Triggers[] = { EJamLineTrigger::Idle, EJamLineTrigger::Pacified, EJamLineTrigger::Interact, EJamLineTrigger::Strayed };
	for (int32 i = 0; i < 200; ++i)
	{
		const FJamLine* L = Long.Request(Triggers[i % 4], At(1000.f + i * 100.f));
		if (!L) { continue; }
		for (int32 b = FMath::Max(0, History.Num() - Long.GetConfig().NoRepeatWindow); b < History.Num(); ++b)
		{
			TestNotEqual(TEXT("not inside the window"), History[b], L->Id);
		}
		History.Add(L->Id);
	}
	TestTrue(TEXT("plenty spoken"), History.Num() > 100);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmNagDeterministic, "Jam.Film.Nag.Deterministic", JamFilmFlags)
bool FJamFilmNagDeterministic::RunTest(const FString&)
{
	auto Run = [](int32 Seed)
	{
		FJamNagScheduler Nag(Seed);
		TArray<FName> Out;
		for (int32 i = 0; i < 60; ++i)
		{
			const EJamLineTrigger T = (EJamLineTrigger)((int32)EJamLineTrigger::Idle + i % 5);
			if (const FJamLine* L = Nag.Request(T, At(100.f + i * 12.f))) { Out.Add(L->Id); }
			else { Out.Add(NAME_None); }
		}
		return Out;
	};
	TestTrue(TEXT("same seed, same lines"), Run(11) == Run(11));
	TestTrue(TEXT("different seed, different lines"), Run(11) != Run(12));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmNagPools, "Jam.Film.Nag.SulkCommentaryOnce", JamFilmFlags)
bool FJamFilmNagPools::RunTest(const FString&)
{
	{
		FJamNagScheduler Nag(5);
		FJamNagSnapshot S = At(100.f);
		S.bSulking = true;
		const FJamLine* L = Nag.Request(EJamLineTrigger::Pacified, S);
		if (TestNotNull(TEXT("sulk line"), L)) { TestEqual(TEXT("from the sulking pool"), L->Trigger, EJamLineTrigger::Sulking); }
	}
	{
		FJamNagScheduler Nag(6);
		TestNull(TEXT("commentary off"), Nag.Request(EJamLineTrigger::Commentary, At(100.f)));
		FJamNagSnapshot S = At(100.f);
		S.bCommentary = true;
		TestNotNull(TEXT("commentary on"), Nag.Request(EJamLineTrigger::Commentary, S));
		S.Now = 110.f;
		TestNull(TEXT("commentary is slower than the global gap"), Nag.Request(EJamLineTrigger::Commentary, S));
		S.Now = 130.f;
		TestNotNull(TEXT("commentary after its gap"), Nag.Request(EJamLineTrigger::Commentary, S));
	}
	{
		FJamNagScheduler Nag(7);
		TestNotNull(TEXT("level seven intro"), Nag.Request(EJamLineTrigger::Level7Intro, At(1.f)));
		TestNull(TEXT("only once"), Nag.Request(EJamLineTrigger::Level7Intro, At(500.f)));
		TestNotNull(TEXT("take three"), Nag.Request(EJamLineTrigger::Take3, At(600.f)));
		TestNull(TEXT("take three only once"), Nag.Request(EJamLineTrigger::Take3, At(900.f)));
		Nag.Reset(7);
		TestNotNull(TEXT("a new run says it again"), Nag.Request(EJamLineTrigger::Level7Intro, At(1.f)));
	}
	{
		FJamNagScheduler Nag(8);
		TestNull(TEXT("non-Fenella pools are not requested"), Nag.Request(EJamLineTrigger::BubbleDefault, At(100.f)));
		TestNotNull(TEXT("but can be picked"), Nag.PickFromPool(EJamLineTrigger::BubbleDefault));
		TestNull(TEXT("Fenella lines are not picked ungated"), Nag.PickFromPool(EJamLineTrigger::Idle));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmNagPoll, "Jam.Film.Nag.PollThresholds", JamFilmFlags)
bool FJamFilmNagPoll::RunTest(const FString&)
{
	FJamNagScheduler Nag(9);
	FJamNagSnapshot S = At(100.f);
	S.SecondsSinceProgress = 19.f;
	TestNull(TEXT("not idle yet"), Nag.Poll(S));
	S.SecondsSinceProgress = 21.f;
	const FJamLine* Idle = Nag.Poll(S);
	if (TestNotNull(TEXT("idle"), Idle)) { TestEqual(TEXT("idle pool"), Idle->Trigger, EJamLineTrigger::Idle); }

	S.Now = 200.f;
	S.SecondsStrayed = 7.f;
	const FJamLine* Strayed = Nag.Poll(S);
	if (TestNotNull(TEXT("strayed"), Strayed)) { TestEqual(TEXT("strayed wins over idle"), Strayed->Trigger, EJamLineTrigger::Strayed); }

	S.Now = 300.f;
	S.SecondsStrayed = 3.f;
	S.bSilenced = true;
	TestNull(TEXT("silent during a prompt"), Nag.Poll(S));
	return true;
}

#endif
