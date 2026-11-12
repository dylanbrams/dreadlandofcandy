#pragma once

#include "CoreMinimal.h"

/**
 * Every authored line (BUILD_SPEC 5A). Text lives in C++, from Docs/Design/Jokes.md; nothing to cook or load.
 * Line ids are "<Trigger>_<nn>" in table order (e.g. "Idle_03"), so voice-over is found by convention:
 * /Game/Audio/VO/VO_<LineId>. Do not reorder a pool once clips exist; append.
 */

enum class EJamLineTrigger : uint8
{
	// Fenella, event triggers ("story": always spoken when asked)
	SceneStart,
	Level7Intro,
	FailedTake,
	Take3,
	Take4,
	Take5,
	Take6,
	CutPitch,
	NoteTaken,
	NoteRefused,
	ForcedRetake,   // "Perfect! Again."
	NarrationCut,   // "CUT. Nobody reads these."
	Wrap,
	BudgetOut,
	Premiere,
	Pause,
	// Fenella, ambient triggers (gated by gap, cooldown and silence)
	Idle,
	Strayed,
	LectureCast,
	Pacified,
	Interact,
	Sulking,        // replaces the four above for the rest of a scene after a refused note
	Commentary,     // only while the Director's Commentary toggle is on
	// Other authored pools, picked without gating
	BubbleDefault,
	BubbleSadder,
	BubbleBudget,
	NuisanceReply,
	ProductionNote,
	Count
};

struct MEGAJAM_API FJamLine
{
	FName Id;
	EJamLineTrigger Trigger = EJamLineTrigger::Idle;
	FString Text;
};

namespace JamLines
{
	/** The whole table, in id order. */
	MEGAJAM_API const TArray<FJamLine>& All();
	/** Lines of one trigger, in table order. */
	MEGAJAM_API TArray<const FJamLine*> ForTrigger(EJamLineTrigger Trigger);
	MEGAJAM_API const FJamLine* Find(FName Id);
	MEGAJAM_API const TCHAR* TriggerName(EJamLineTrigger Trigger);

	/** Fenella speaks these (everything up to and including Commentary). */
	MEGAJAM_API bool IsFenella(EJamLineTrigger Trigger);
	/** Always spoken when asked; skips gap, cooldown and silence. */
	MEGAJAM_API bool IsStory(EJamLineTrigger Trigger);
	/** Said at most once per run. */
	MEGAJAM_API bool IsOnce(EJamLineTrigger Trigger);
	/** The trigger for the start of take N: SceneStart for 1 and 2, Take3..Take6 after, Take6 beyond. */
	MEGAJAM_API EJamLineTrigger TriggerForTake(int32 Take);
}
