#include "Film/JamShoot.h"

void FJamShoot::Start(const FJamSceneDef& InScene)
{
	Scene = InScene;
	State = EJamShootState::Slate;
	StageIndex = 0;
	Take = 1;
	TakesUsed = 1;
	Progress = 0;
	Meter = 0;
	StageTime = 0.f;
	NotesTaken = 0;
	NotesRefused = 0;
	bNoteAnswered = false;
	bForcedRetakeUsed = false;
	bSulking = false;
	GrossMultiplier = 1.f;
	Events.Reset();
}

void FJamShoot::Emit(EJamShootEventType Type, int32 Value, int32 Count)
{
	FJamShootEvent& E = Events.AddDefaulted_GetRef();
	E.Type = Type;
	E.Stage = StageIndex;
	E.Take = Take;
	E.Value = Value;
	E.Count = Count;
}

void FJamShoot::StartStage(int32 Index)
{
	StageIndex = Index;
	Progress = 0;
	Meter = 0;
	StageTime = 0.f;
	State = EJamShootState::Rolling;
	Emit(EJamShootEventType::StageStarted);
}

bool FJamShoot::Action()
{
	if (State == EJamShootState::Slate)
	{
		StartStage(StageIndex);
		return true;
	}
	if (State == EJamShootState::StageCleared)
	{
		const int32 Next = StageIndex + 1;
		if (!Scene.Stages.IsValidIndex(Next))
		{
			State = EJamShootState::Wrap;
			FJamShootEvent& E = Events.AddDefaulted_GetRef();
			E.Type = EJamShootEventType::Wrapped;
			E.Stage = StageIndex;
			E.Take = TakesUsed;
			return true;
		}
		if (Next == Scene.NoteBeforeStage && !bNoteAnswered)
		{
			State = EJamShootState::NotePrompt;
			FJamShootEvent& E = Events.AddDefaulted_GetRef();
			E.Type = EJamShootEventType::NotePitched;
			E.Stage = Next;
			E.Take = Take;
			return true;
		}
		StartStage(Next);
		return true;
	}
	return false;
}

bool FJamShoot::ReportProgress(int32 Amount)
{
	const FJamStageDef* Stage = GetStage();
	if (State != EJamShootState::Rolling || !Stage || Amount <= 0) { return false; }

	Progress = FMath::Min(Progress + Amount, Stage->Count);
	Emit(EJamShootEventType::Progress, Progress, Stage->Count);
	if (Progress < Stage->Count) { return true; }

	Emit(EJamShootEventType::StageCleared);
	if (Scene.bForcedRetakeOnFirstStage && StageIndex == 0 && Take == 1 && !bForcedRetakeUsed)
	{
		// "Perfect! Again." The stage was cleared, and she makes them do it again anyway. Once.
		bForcedRetakeUsed = true;
		Fail(true);
		return true;
	}
	State = EJamShootState::StageCleared;
	return true;
}

bool FJamShoot::ReportMeter(int32 Amount)
{
	const FJamStageDef* Stage = GetStage();
	if (State != EJamShootState::Rolling || !Stage || Stage->FailMeter <= 0 || Amount <= 0) { return false; }
	Meter += Amount;
	if (Meter >= Stage->FailMeter) { Fail(false); }
	return true;
}

bool FJamShoot::ReportFail()
{
	if (State != EJamShootState::Rolling) { return false; }
	Fail(false);
	return true;
}

void FJamShoot::Fail(bool bForced)
{
	State = EJamShootState::FailedTake;
	Emit(EJamShootEventType::TakeFailed, bForced ? 1 : 0);
}

bool FJamShoot::Retake()
{
	if (State != EJamShootState::FailedTake) { return false; }
	++Take;
	++TakesUsed;
	Progress = 0;
	Meter = 0;
	StageTime = 0.f;
	State = EJamShootState::Slate;
	return true;
}

bool FJamShoot::AnswerNote(bool bTake)
{
	if (State != EJamShootState::NotePrompt) { return false; }
	bNoteAnswered = true;
	const int32 Target = Scene.NoteBeforeStage;
	if (bTake)
	{
		++NotesTaken;
		GrossMultiplier *= Scene.Note.GrossMultiplier;
		if (Scene.Stages.IsValidIndex(Target)) { Scene.Stages[Target] = Scene.Note.Replacement; }
	}
	else
	{
		++NotesRefused;
		bSulking = true;
	}
	Emit(EJamShootEventType::NoteAnswered, bTake ? 1 : 0);
	StartStage(Target);
	return true;
}

void FJamShoot::Tick(float DeltaSeconds)
{
	const FJamStageDef* Stage = GetStage();
	if (State != EJamShootState::Rolling || !Stage) { return; }
	StageTime += DeltaSeconds;
	if (Stage->TimeLimit > 0.f && StageTime >= Stage->TimeLimit) { Fail(false); }
}

float FJamShoot::GetTimeLeft() const
{
	const FJamStageDef* Stage = GetStage();
	return (Stage && Stage->TimeLimit > 0.f) ? FMath::Max(0.f, Stage->TimeLimit - StageTime) : 0.f;
}

bool FJamShoot::TargetsFlee() const
{
	return !(Scene.bForcedRetakeOnFirstStage && StageIndex == 0 && Take == 1);
}

void FJamShoot::DrainEvents(TArray<FJamShootEvent>& OutEvents)
{
	OutEvents.Append(Events);
	Events.Reset();
}

// ---------------------------------------------------------------------------------------------------------

namespace JamScenes
{
	FJamSceneDef Scene1()
	{
		FJamSceneDef S;
		S.Id = TEXT("Scene1");
		S.Title = TEXT("A Balanced Diet");
		S.bForcedRetakeOnFirstStage = true;

		FJamStageDef& Pacify = S.Stages.AddDefaulted_GetRef();
		Pacify.Id = TEXT("S1_PacifyRats");
		Pacify.Verb = EJamStageVerb::Pacify;
		Pacify.Target = EJamTargetKind::Rat;
		Pacify.Count = 3;
		Pacify.Objective = TEXT("Pacify the rats");

		FJamStageDef& Feed = S.Stages.AddDefaulted_GetRef();
		Feed.Id = TEXT("S1_FeedCelery");
		Feed.Verb = EJamStageVerb::Interact;
		Feed.Target = EJamTargetKind::Rat;
		Feed.Count = 3;
		Feed.Objective = TEXT("Feed each rat celery");
		return S;
	}

	int32 SetCapacity(const FJamSceneDef& Scene, EJamTargetKind Kind)
	{
		// The field set spawns three rats (the game mode's Scene 1 spawn points).
		if (Scene.Id == FName(TEXT("Scene1"))) { return Kind == EJamTargetKind::Rat ? 3 : 0; }
		return 0;
	}
}
