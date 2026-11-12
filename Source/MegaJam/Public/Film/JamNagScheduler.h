#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "Film/JamLines.h"

/**
 * Fenella's nag scheduler (master brief C.7). Plain C++, no world, no audio. It chooses at most one line per
 * request and never changes rules. Deterministic for a seed, so two machines would pick the same line.
 *
 * Ambient triggers (idle, strayed, lecture, pacified, interact, commentary) obey the global gap, the per-trigger
 * cooldown and the silence flag. Story triggers (scene start, CUT!, wrap and so on) are always spoken when asked.
 * No line repeats within the last N spoken, nor until its whole pool has been used.
 */

struct MEGAJAM_API FJamNagConfig
{
	float GlobalGap = 8.f;         // seconds between any two Fenella lines
	float CommentaryGap = 25.f;    // commentary is slower
	float TriggerCooldown = 20.f;  // per ambient trigger
	int32 NoRepeatWindow = 8;      // last N spoken lines are never picked again
	float IdleAfter = 20.f;        // seconds without progress
	float StrayedAfter = 6.f;      // seconds away from the set
	float LectureChance = 0.2f;    // "rare, one in five"
};

struct MEGAJAM_API FJamNagSnapshot
{
	float Now = 0.f;
	float SecondsSinceProgress = 0.f;
	float SecondsStrayed = 0.f;
	/** Dialogue, a menu or the CUT! prompt is up: ambient lines stay quiet. */
	bool bSilenced = false;
	/** A note was refused this scene: ambient reactions come from the sulking pool. */
	bool bSulking = false;
	bool bCommentary = false;
};

class MEGAJAM_API FJamNagScheduler
{
public:
	explicit FJamNagScheduler(int32 InSeed = 1, const FJamNagConfig& InConfig = FJamNagConfig());

	/** An event happened. Returns the line to show, or nullptr if she stays quiet. */
	const FJamLine* Request(EJamLineTrigger Trigger, const FJamNagSnapshot& Snap);

	/** Called each second or so: derives idle, strayed and commentary from the snapshot. */
	const FJamLine* Poll(const FJamNagSnapshot& Snap);

	/** Picks from a non-Fenella pool (bubbles, replies, production notes) with the same no-repeat rules, ungated. */
	const FJamLine* PickFromPool(EJamLineTrigger Trigger);

	/** Forget everything, as at the start of a run. */
	void Reset(int32 NewSeed);

	const FJamNagConfig& GetConfig() const { return Config; }

private:
	const FJamLine* Pick(EJamLineTrigger Trigger);
	void Record(const FJamLine* Line, EJamLineTrigger Trigger, float Now);

	FJamNagConfig Config;
	FRandomStream Rng;
	float LastSpokenAt = -1.0e9f;
	TStaticArray<float, (int32)EJamLineTrigger::Count> LastTriggerAt;
	TStaticArray<TSet<FName>, (int32)EJamLineTrigger::Count> Used;
	TArray<FName> Recent;
	TSet<uint8> SpentOnce;
};
