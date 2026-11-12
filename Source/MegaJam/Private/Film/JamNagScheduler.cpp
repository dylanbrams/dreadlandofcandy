#include "Film/JamNagScheduler.h"

FJamNagScheduler::FJamNagScheduler(int32 InSeed, const FJamNagConfig& InConfig)
	: Config(InConfig)
{
	Reset(InSeed);
}

void FJamNagScheduler::Reset(int32 NewSeed)
{
	Rng.Initialize(NewSeed);
	LastSpokenAt = -1.0e9f;
	for (float& T : LastTriggerAt) { T = -1.0e9f; }
	for (TSet<FName>& U : Used) { U.Reset(); }
	Recent.Reset();
	SpentOnce.Reset();
}

const FJamLine* FJamNagScheduler::Pick(EJamLineTrigger Trigger)
{
	const TArray<const FJamLine*> Pool = JamLines::ForTrigger(Trigger);
	if (Pool.IsEmpty()) { return nullptr; }
	TSet<FName>& UsedSet = Used[(int32)Trigger];

	auto Gather = [&](bool bSkipUsed, bool bSkipRecent)
	{
		TArray<const FJamLine*> Out;
		for (const FJamLine* L : Pool)
		{
			if (bSkipUsed && UsedSet.Contains(L->Id)) { continue; }
			if (bSkipRecent && Recent.Contains(L->Id)) { continue; }
			Out.Add(L);
		}
		return Out;
	};

	TArray<const FJamLine*> Candidates = Gather(true, true);
	if (Candidates.IsEmpty())
	{
		// Pool exhausted: start it again, still avoiding what was just said.
		UsedSet.Reset();
		Candidates = Gather(false, true);
	}
	if (Candidates.IsEmpty())
	{
		// A pool smaller than the window: only avoid the very last line.
		for (const FJamLine* L : Pool)
		{
			if (Recent.IsEmpty() || Recent.Last() != L->Id || Pool.Num() == 1) { Candidates.Add(L); }
		}
	}
	return Candidates[Rng.RandRange(0, Candidates.Num() - 1)];
}

void FJamNagScheduler::Record(const FJamLine* Line, EJamLineTrigger Trigger, float Now)
{
	Used[(int32)Trigger].Add(Line->Id);
	Recent.Add(Line->Id);
	while (Recent.Num() > Config.NoRepeatWindow) { Recent.RemoveAt(0); }
	if (JamLines::IsFenella(Trigger))
	{
		LastSpokenAt = Now;
		LastTriggerAt[(int32)Trigger] = Now;
	}
	if (JamLines::IsOnce(Trigger)) { SpentOnce.Add((uint8)Trigger); }
}

const FJamLine* FJamNagScheduler::PickFromPool(EJamLineTrigger Trigger)
{
	if (JamLines::IsFenella(Trigger)) { return nullptr; }
	const FJamLine* L = Pick(Trigger);
	if (L) { Record(L, Trigger, 0.f); }
	return L;
}

const FJamLine* FJamNagScheduler::Request(EJamLineTrigger Trigger, const FJamNagSnapshot& Snap)
{
	if (!JamLines::IsFenella(Trigger)) { return nullptr; }
	if (JamLines::IsOnce(Trigger) && SpentOnce.Contains((uint8)Trigger)) { return nullptr; }

	if (!JamLines::IsStory(Trigger))
	{
		// Ambient. After a refused note she sulks instead of reacting.
		if (Snap.bSulking && Trigger != EJamLineTrigger::Commentary && Trigger != EJamLineTrigger::Strayed)
		{
			Trigger = EJamLineTrigger::Sulking;
		}
		if (Trigger == EJamLineTrigger::Commentary && !Snap.bCommentary) { return nullptr; }
		if (Snap.bSilenced) { return nullptr; }

		const float Gap = Trigger == EJamLineTrigger::Commentary ? Config.CommentaryGap : Config.GlobalGap;
		if (Snap.Now - LastSpokenAt < Gap) { return nullptr; }
		if (Snap.Now - LastTriggerAt[(int32)Trigger] < Config.TriggerCooldown) { return nullptr; }
		if (Trigger == EJamLineTrigger::LectureCast && Rng.FRand() >= Config.LectureChance) { return nullptr; }
	}

	const FJamLine* Line = Pick(Trigger);
	if (Line) { Record(Line, Trigger, Snap.Now); }
	return Line;
}

const FJamLine* FJamNagScheduler::Poll(const FJamNagSnapshot& Snap)
{
	if (Snap.SecondsStrayed >= Config.StrayedAfter)
	{
		return Request(EJamLineTrigger::Strayed, Snap);
	}
	if (Snap.SecondsSinceProgress >= Config.IdleAfter)
	{
		return Request(EJamLineTrigger::Idle, Snap);
	}
	if (Snap.bCommentary)
	{
		return Request(EJamLineTrigger::Commentary, Snap);
	}
	return nullptr;
}
