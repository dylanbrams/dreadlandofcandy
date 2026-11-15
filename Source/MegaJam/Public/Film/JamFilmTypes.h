#pragma once

#include "CoreMinimal.h"

/**
 * Headless film rules (BUILD_SPEC 5A): plain structs, no UObjects, no world. Tests run them directly.
 */

enum class EJamStageVerb : uint8
{
	Pacify,     // bring targets' Resolve to zero
	Interact,   // treat targets (feed, brush) with E
	Collect,    // pick things up
	Visit,      // village errands only (Fixes5 #4): walk to a place
};

enum class EJamTargetKind : uint8
{
	Rat,
	Goblin,
	Bug,
	Almond,
};

struct MEGAJAM_API FJamStageDef
{
	FName Id;
	EJamStageVerb Verb = EJamStageVerb::Pacify;
	EJamTargetKind Target = EJamTargetKind::Rat;
	int32 Count = 1;
	float ResolveMultiplier = 1.f;
	float SpeedMultiplier = 1.f;
	/** Seconds; 0 = no countdown. */
	float TimeLimit = 0.f;
	/** Damage meter that fails the take when it reaches this; 0 = none. */
	int32 FailMeter = 0;
	/** Shown on the HUD, followed by the progress count. */
	FString Objective;

	bool operator==(const FJamStageDef& Other) const
	{
		return Id == Other.Id && Verb == Other.Verb && Target == Other.Target && Count == Other.Count
			&& ResolveMultiplier == Other.ResolveMultiplier && SpeedMultiplier == Other.SpeedMultiplier
			&& TimeLimit == Other.TimeLimit && FailMeter == Other.FailMeter && Objective == Other.Objective;
	}
};

/** Fenella's rewrite of one stage, pitched with CUT! at a transition. */
struct MEGAJAM_API FJamNoteDef
{
	FName Id;
	FName PitchLineId;
	FJamStageDef Replacement;
	float GrossMultiplier = 1.f;
	FName BubblePool;
};

struct MEGAJAM_API FJamSceneDef
{
	FName Id;
	FString Title;
	TArray<FJamStageDef> Stages;
	/** The note is pitched before this stage index starts; INDEX_NONE = no note in this scene. */
	int32 NoteBeforeStage = INDEX_NONE;
	FJamNoteDef Note;
	/** Scene 1's "Perfect. Again.": clearing stage 0 on take 1 forces a retake, once. */
	bool bForcedRetakeOnFirstStage = false;
};

enum class EJamShootState : uint8
{
	Idle,         // no scene started
	Slate,        // take about to roll; Action() rolls it
	Rolling,      // a stage is being played
	StageCleared, // Action() moves on (next stage, the note, or the wrap)
	NotePrompt,   // waiting for AnswerNote()
	FailedTake,   // Retake() resets the stage
	Wrap,         // scene done; nothing changes any more
};

enum class EJamShootEventType : uint8
{
	StageStarted,   // Stage, Take
	Progress,       // Stage, Value = progress, Count
	StageCleared,   // Stage
	NotePitched,    // Stage = the stage the note would replace
	NoteAnswered,   // Value = 1 taken, 0 refused
	TakeFailed,     // Stage, Take, Value = 1 if forced ("Perfect. Again."), 0 for a real failure
	Wrapped,        // Take = takes used in total
};

struct MEGAJAM_API FJamShootEvent
{
	EJamShootEventType Type = EJamShootEventType::StageStarted;
	int32 Stage = INDEX_NONE;
	int32 Take = 0;
	int32 Value = 0;
	int32 Count = 0;
};
