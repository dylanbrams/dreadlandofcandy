#pragma once

#include "CoreMinimal.h"
#include "Film/JamFilmTypes.h"

/**
 * The shoot of one scene: takes, stages, the note, the wrap. Plain C++ so tests drive it without a world.
 * The world layer calls the inputs and drains the events each frame; events are data, not delegates.
 *
 *   Slate --Action--> Rolling --progress full--> StageCleared --Action--> Rolling (next stage)
 *                                                               |-------> NotePrompt --AnswerNote--> Rolling
 *                                                               '-------> Wrap (once)
 *   Rolling --ReportFail / timer / meter--> FailedTake --Retake--> Slate (same stage, take + 1)
 */
class MEGAJAM_API FJamShoot
{
public:
	void Start(const FJamSceneDef& InScene);

	// Inputs. Each returns false (and changes nothing) when it does not apply in the current state.
	bool Action();
	bool ReportProgress(int32 Amount = 1);
	bool ReportMeter(int32 Amount = 1);
	bool ReportFail();
	bool AnswerNote(bool bTake);
	bool Retake();
	void Tick(float DeltaSeconds);

	/** Moves the queued events into OutEvents (appends) and clears the queue. */
	void DrainEvents(TArray<FJamShootEvent>& OutEvents);

	// Reads.
	EJamShootState GetState() const { return State; }
	const FJamSceneDef& GetScene() const { return Scene; }
	int32 GetStageIndex() const { return StageIndex; }
	const FJamStageDef* GetStage() const { return Scene.Stages.IsValidIndex(StageIndex) ? &Scene.Stages[StageIndex] : nullptr; }
	int32 GetTake() const { return Take; }
	int32 GetTakesUsed() const { return TakesUsed; }
	int32 GetProgress() const { return Progress; }
	int32 GetMeter() const { return Meter; }
	float GetTimeLeft() const;
	int32 GetNotesTaken() const { return NotesTaken; }
	int32 GetNotesRefused() const { return NotesRefused; }
	bool IsSulking() const { return bSulking; }
	float GetGrossMultiplier() const { return GrossMultiplier; }

	/** Scene 1, take 1 of stage 1: the rats have not learned to run yet ("This time the rats know their lines"). */
	bool TargetsFlee() const;

private:
	void StartStage(int32 Index);
	void Fail(bool bForced);
	void Emit(EJamShootEventType Type, int32 Value = 0, int32 Count = 0);

	FJamSceneDef Scene;
	EJamShootState State = EJamShootState::Idle;
	int32 StageIndex = 0;
	int32 Take = 1;
	int32 TakesUsed = 1;
	int32 Progress = 0;
	int32 Meter = 0;
	float StageTime = 0.f;
	int32 NotesTaken = 0;
	int32 NotesRefused = 0;
	bool bNoteAnswered = false;
	bool bForcedRetakeUsed = false;
	bool bSulking = false;
	float GrossMultiplier = 1.f;
	TArray<FJamShootEvent> Events;
};

/** Authored scenes (BUILD_SPEC 3). Text is in C++: nothing to cook. */
namespace JamScenes
{
	MEGAJAM_API FJamSceneDef Scene1();
	MEGAJAM_API FJamSceneDef Scene2();
	MEGAJAM_API FJamSceneDef Scene3();
	/** Scene 1 to 3 in order. */
	MEGAJAM_API TArray<FJamSceneDef> All();

	/** How many targets of a kind each scene's set spawns at most (the finishable-stage test checks against it). */
	MEGAJAM_API int32 SetCapacity(const FJamSceneDef& Scene, EJamTargetKind Kind);
}
