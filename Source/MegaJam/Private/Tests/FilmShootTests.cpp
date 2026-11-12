#include "Misc/AutomationTest.h"
#include "Film/JamShoot.h"
#include "Film/JamNoteClamp.h"
#include "Film/JamLines.h"

#if WITH_DEV_AUTOMATION_TESTS

#define JamFilmFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

namespace
{
	FJamStageDef MakeStage(const TCHAR* Id, int32 Count, float TimeLimit = 0.f, int32 FailMeter = 0)
	{
		FJamStageDef S;
		S.Id = Id;
		S.Count = Count;
		S.TimeLimit = TimeLimit;
		S.FailMeter = FailMeter;
		S.Objective = Id;
		return S;
	}

	/** Three stages; the note (if bWithNote) is pitched before stage 1 and replaces it. */
	FJamSceneDef MakeScene(bool bWithNote)
	{
		FJamSceneDef S;
		S.Id = TEXT("TestScene");
		S.Stages = { MakeStage(TEXT("A"), 2), MakeStage(TEXT("B"), 3, 10.f), MakeStage(TEXT("C"), 1) };
		if (bWithNote)
		{
			S.NoteBeforeStage = 1;
			S.Note.Id = TEXT("BiggerB");
			S.Note.Replacement = MakeStage(TEXT("B_Note"), 1);
			S.Note.Replacement.ResolveMultiplier = 4.8f;
			S.Note.GrossMultiplier = 1.5f;
		}
		return S;
	}

	TArray<FJamShootEvent> Drain(FJamShoot& Shoot)
	{
		TArray<FJamShootEvent> Out;
		Shoot.DrainEvents(Out);
		return Out;
	}

	int32 CountOf(const TArray<FJamShootEvent>& Events, EJamShootEventType Type)
	{
		return Events.FilterByPredicate([Type](const FJamShootEvent& E) { return E.Type == Type; }).Num();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmShootHappyPath, "Jam.Film.Shoot.HappyPath", JamFilmFlags)
bool FJamFilmShootHappyPath::RunTest(const FString&)
{
	FJamShoot Shoot;
	Shoot.Start(MakeScene(false));
	TestEqual(TEXT("starts on the slate"), Shoot.GetState(), EJamShootState::Slate);
	TestFalse(TEXT("progress ignored before ACTION"), Shoot.ReportProgress());

	TestTrue(TEXT("action rolls"), Shoot.Action());
	TestEqual(TEXT("stage 0 started"), CountOf(Drain(Shoot), EJamShootEventType::StageStarted), 1);
	Shoot.ReportProgress(2);
	TestEqual(TEXT("stage 0 cleared"), Shoot.GetState(), EJamShootState::StageCleared);
	TestFalse(TEXT("progress ignored once cleared"), Shoot.ReportProgress());

	Shoot.Action();
	TestEqual(TEXT("on stage 1"), Shoot.GetStageIndex(), 1);
	Shoot.ReportProgress(3);
	Shoot.Action();
	Shoot.ReportProgress(1);
	Shoot.Action();
	TestEqual(TEXT("wrapped"), Shoot.GetState(), EJamShootState::Wrap);

	// Once-only wrap: nothing after it does anything.
	TestFalse(TEXT("action after wrap"), Shoot.Action());
	TestFalse(TEXT("retake after wrap"), Shoot.Retake());
	TestFalse(TEXT("fail after wrap"), Shoot.ReportFail());
	const TArray<FJamShootEvent> Events = Drain(Shoot);
	TestEqual(TEXT("exactly one Wrapped"), CountOf(Events, EJamShootEventType::Wrapped), 1);
	TestEqual(TEXT("one take used"), Shoot.GetTakesUsed(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmShootNoteReplacesNextStage, "Jam.Film.Shoot.NoteReplacesOnlyNextStage", JamFilmFlags)
bool FJamFilmShootNoteReplacesNextStage::RunTest(const FString&)
{
	// C.10 A: a CUT! at a transition replaces only the next stage; earlier and later stages are unchanged.
	const FJamSceneDef Def = MakeScene(true);
	FJamShoot Shoot;
	Shoot.Start(Def);
	Shoot.Action();
	Shoot.ReportProgress(2);
	Shoot.Action();
	TestEqual(TEXT("note prompt at the transition"), Shoot.GetState(), EJamShootState::NotePrompt);
	TestEqual(TEXT("NotePitched names stage 1"), Drain(Shoot).Last().Stage, 1);

	TestTrue(TEXT("note taken"), Shoot.AnswerNote(true));
	TestTrue(TEXT("stage 0 unchanged"), Shoot.GetScene().Stages[0] == Def.Stages[0]);
	TestTrue(TEXT("stage 1 replaced"), Shoot.GetScene().Stages[1] == Def.Note.Replacement);
	TestTrue(TEXT("stage 2 unchanged"), Shoot.GetScene().Stages[2] == Def.Stages[2]);
	TestEqual(TEXT("rolling stage 1"), Shoot.GetState(), EJamShootState::Rolling);
	TestEqual(TEXT("on stage 1"), Shoot.GetStageIndex(), 1);
	TestEqual(TEXT("NotesTaken"), Shoot.GetNotesTaken(), 1);
	TestEqual(TEXT("gross multiplier"), Shoot.GetGrossMultiplier(), 1.5f);
	TestFalse(TEXT("not sulking"), Shoot.IsSulking());

	// The note is pitched once: after a failure and retake of stage 1 there is no second prompt.
	Shoot.ReportFail();
	Shoot.Retake();
	Shoot.Action();
	TestEqual(TEXT("retake rolls stage 1 again"), Shoot.GetStageIndex(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmShootNoteRefused, "Jam.Film.Shoot.NoteRefused", JamFilmFlags)
bool FJamFilmShootNoteRefused::RunTest(const FString&)
{
	const FJamSceneDef Def = MakeScene(true);
	FJamShoot Shoot;
	Shoot.Start(Def);
	Shoot.Action();
	Shoot.ReportProgress(2);
	Shoot.Action();
	TestTrue(TEXT("note refused"), Shoot.AnswerNote(false));
	TestTrue(TEXT("stage 1 kept"), Shoot.GetScene().Stages[1] == Def.Stages[1]);
	TestEqual(TEXT("NotesRefused"), Shoot.GetNotesRefused(), 1);
	TestEqual(TEXT("NotesTaken"), Shoot.GetNotesTaken(), 0);
	TestTrue(TEXT("sulking"), Shoot.IsSulking());
	TestEqual(TEXT("gross multiplier unchanged"), Shoot.GetGrossMultiplier(), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmShootCutOutsideTransition, "Jam.Film.Shoot.CutRefusedOutsideTransition", JamFilmFlags)
bool FJamFilmShootCutOutsideTransition::RunTest(const FString&)
{
	// C.10 B: answering a note is refused anywhere but the note prompt.
	const FJamSceneDef Def = MakeScene(true);
	FJamShoot Shoot;
	Shoot.Start(Def);
	TestFalse(TEXT("on the slate"), Shoot.AnswerNote(true));
	Shoot.Action();
	TestFalse(TEXT("while rolling"), Shoot.AnswerNote(true));
	Shoot.ReportProgress(1);
	Shoot.ReportFail();
	TestFalse(TEXT("after a failed take"), Shoot.AnswerNote(true));
	TestEqual(TEXT("no notes counted"), Shoot.GetNotesTaken() + Shoot.GetNotesRefused(), 0);
	TestTrue(TEXT("stage 1 untouched"), Shoot.GetScene().Stages[1] == Def.Stages[1]);

	// A scene without a note never prompts.
	FJamShoot NoNote;
	NoNote.Start(MakeScene(false));
	NoNote.Action();
	NoNote.ReportProgress(2);
	NoNote.Action();
	TestEqual(TEXT("no prompt without a note"), NoNote.GetState(), EJamShootState::Rolling);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmShootRetakeResets, "Jam.Film.Shoot.RetakeResets", JamFilmFlags)
bool FJamFilmShootRetakeResets::RunTest(const FString&)
{
	// C.10 C: RETAKE! resets the stage's counters, increments the take, and cannot be applied twice.
	FJamShoot Shoot;
	Shoot.Start(MakeScene(false));
	Shoot.Action();
	Shoot.ReportProgress(2);
	Shoot.Action();               // stage 1, needs 3
	Shoot.ReportProgress(2);
	TestTrue(TEXT("fail"), Shoot.ReportFail());
	TestFalse(TEXT("cannot fail twice"), Shoot.ReportFail());
	TestEqual(TEXT("failed"), Shoot.GetState(), EJamShootState::FailedTake);

	TestTrue(TEXT("retake"), Shoot.Retake());
	TestFalse(TEXT("retake does not double-apply"), Shoot.Retake());
	TestEqual(TEXT("take 2"), Shoot.GetTake(), 2);
	TestEqual(TEXT("progress reset"), Shoot.GetProgress(), 0);
	TestEqual(TEXT("same stage"), Shoot.GetStageIndex(), 1);
	TestEqual(TEXT("back on the slate"), Shoot.GetState(), EJamShootState::Slate);

	const TArray<FJamShootEvent> Events = Drain(Shoot);
	TestEqual(TEXT("one TakeFailed"), CountOf(Events, EJamShootEventType::TakeFailed), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmShootTimerAndMeter, "Jam.Film.Shoot.TimerAndMeter", JamFilmFlags)
bool FJamFilmShootTimerAndMeter::RunTest(const FString&)
{
	FJamSceneDef Def;
	Def.Id = TEXT("Timed");
	Def.Stages = { MakeStage(TEXT("Timed"), 3, 5.f), MakeStage(TEXT("Metered"), 3, 0.f, 4) };

	FJamShoot Shoot;
	Shoot.Start(Def);
	Shoot.Tick(10.f);
	TestEqual(TEXT("timer does not run on the slate"), Shoot.GetState(), EJamShootState::Slate);
	Shoot.Action();
	Shoot.Tick(3.f);
	TestEqual(TEXT("time left"), Shoot.GetTimeLeft(), 2.f);
	Shoot.Tick(3.f);
	TestEqual(TEXT("countdown fails the take"), Shoot.GetState(), EJamShootState::FailedTake);
	Shoot.Tick(3.f);
	TestEqual(TEXT("one failure only"), CountOf(Drain(Shoot), EJamShootEventType::TakeFailed), 1);

	Shoot.Retake();
	Shoot.Action();
	TestEqual(TEXT("timer restarts"), Shoot.GetTimeLeft(), 5.f);
	Shoot.ReportProgress(3);
	Shoot.Action();
	TestFalse(TEXT("meter needs a meter stage"), Shoot.ReportMeter(0));
	Shoot.ReportMeter(3);
	TestEqual(TEXT("still rolling at 3/4"), Shoot.GetState(), EJamShootState::Rolling);
	Shoot.ReportMeter(1);
	TestEqual(TEXT("meter fails the take"), Shoot.GetState(), EJamShootState::FailedTake);
	Shoot.Retake();
	TestEqual(TEXT("meter reset"), Shoot.GetMeter(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmShootForcedRetake, "Jam.Film.Shoot.ForcedRetakeOnce", JamFilmFlags)
bool FJamFilmShootForcedRetake::RunTest(const FString&)
{
	// Scene 1: "Perfect! Again." Clearing stage 0 on take 1 forces take 2, where the rats flee. Once only.
	FJamShoot Shoot;
	Shoot.Start(JamScenes::Scene1());
	Shoot.Action();
	TestFalse(TEXT("rats do not flee on take 1"), Shoot.TargetsFlee());
	Shoot.ReportProgress(3);
	TestEqual(TEXT("forced into a failed take"), Shoot.GetState(), EJamShootState::FailedTake);
	TArray<FJamShootEvent> Events = Drain(Shoot);
	TestEqual(TEXT("cleared, then failed"), CountOf(Events, EJamShootEventType::StageCleared), 1);
	TestTrue(TEXT("the failure is marked forced"), Events.Last().Type == EJamShootEventType::TakeFailed && Events.Last().Value == 1);

	Shoot.Retake();
	Shoot.Action();
	TestTrue(TEXT("rats flee on take 2"), Shoot.TargetsFlee());
	Shoot.ReportProgress(3);
	TestEqual(TEXT("take 2 clears normally"), Shoot.GetState(), EJamShootState::StageCleared);
	Shoot.Action();
	Shoot.ReportProgress(3);
	Shoot.Action();
	TestEqual(TEXT("scene 1 wraps"), Shoot.GetState(), EJamShootState::Wrap);
	TestEqual(TEXT("two takes used"), Shoot.GetTakesUsed(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmScenesFinishable, "Jam.Film.Scenes.Finishable", JamFilmFlags)
bool FJamFilmScenesFinishable::RunTest(const FString&)
{
	// Every stage, and every authored note's replacement, asks for no more targets than its set spawns and
	// can be finished with the Lecture alone inside its clock.
	const TArray<FJamSceneDef> Scenes = JamScenes::All();
	TestEqual(TEXT("three scenes"), Scenes.Num(), 3);
	for (const FJamSceneDef& Scene : Scenes)
	{
		const FString Name = Scene.Id.ToString();
		TestTrue(*FString::Printf(TEXT("%s has stages"), *Name), Scene.Stages.Num() > 0);
		TestFalse(*FString::Printf(TEXT("%s has a title"), *Name), Scene.Title.IsEmpty());
		for (int32 i = 0; i < Scene.Stages.Num(); ++i)
		{
			const FJamStageDef& Stage = Scene.Stages[i];
			const FJamNoteContext Ctx = JamNoteClamp::MakeContext(Scene, i);
			TestTrue(*FString::Printf(TEXT("%s: count %d fits the set"), *Stage.Id.ToString(), Stage.Count),
				Stage.Count > 0 && Stage.Count <= JamScenes::SetCapacity(Scene, Stage.Target));
			TestFalse(*FString::Printf(TEXT("%s has an objective"), *Stage.Id.ToString()), Stage.Objective.IsEmpty());
			TestTrue(*FString::Printf(TEXT("%s is finishable"), *Stage.Id.ToString()), JamNoteClamp::IsFinishable(Stage, Ctx));
		}
		if (Scene.NoteBeforeStage != INDEX_NONE)
		{
			TestTrue(*FString::Printf(TEXT("%s: note sits at a transition"), *Name), Scene.NoteBeforeStage > 0 && Scene.NoteBeforeStage < Scene.Stages.Num());
			const FJamStageDef& R = Scene.Note.Replacement;
			TestTrue(*FString::Printf(TEXT("%s: note stage fits the set"), *Name), R.Count > 0 && R.Count <= JamScenes::SetCapacity(Scene, R.Target));
			TestTrue(*FString::Printf(TEXT("%s: note stage is finishable"), *Name), JamNoteClamp::IsFinishable(R, JamNoteClamp::MakeContext(Scene, Scene.NoteBeforeStage)));
			TestTrue(*FString::Printf(TEXT("%s: the pitch line exists"), *Name), JamLines::Find(Scene.Note.PitchLineId) != nullptr);
			TestTrue(*FString::Printf(TEXT("%s: taking pays more"), *Name), Scene.Note.GrossMultiplier > 1.f);
			TestFalse(*FString::Printf(TEXT("%s: note has an objective"), *Name), R.Objective.IsEmpty());
		}
	}
	TestTrue(TEXT("only scene 1 forces a retake"), Scenes[0].bForcedRetakeOnFirstStage && !Scenes[1].bForcedRetakeOnFirstStage && !Scenes[2].bForcedRetakeOnFirstStage);
	TestEqual(TEXT("scene 1 has no note"), Scenes[0].NoteBeforeStage, (int32)INDEX_NONE);
	TestNotEqual(TEXT("scene 2 has a note"), Scenes[1].NoteBeforeStage, (int32)INDEX_NONE);
	TestNotEqual(TEXT("scene 3 has a note"), Scenes[2].NoteBeforeStage, (int32)INDEX_NONE);
	TestTrue(TEXT("scene 2 fails by countdown"), Scenes[1].Stages.ContainsByPredicate([](const FJamStageDef& S) { return S.TimeLimit > 0.f; }));
	TestTrue(TEXT("scene 3 fails by meter"), Scenes[2].Stages.ContainsByPredicate([](const FJamStageDef& S) { return S.FailMeter == 10; }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamFilmScenesPlaythrough, "Jam.Film.Scenes.Playthrough", JamFilmFlags)
bool FJamFilmScenesPlaythrough::RunTest(const FString&)
{
	// Drive every scene to its wrap with each answer to the note: it always ends, exactly once.
	for (const FJamSceneDef& Scene : JamScenes::All())
	{
		for (int32 Answer = 0; Answer < 2; ++Answer)
		{
			FJamShoot Shoot;
			Shoot.Start(Scene);
			int32 Wraps = 0, Steps = 0;
			while (Shoot.GetState() != EJamShootState::Wrap && Steps++ < 100)
			{
				switch (Shoot.GetState())
				{
				case EJamShootState::Slate:
				case EJamShootState::StageCleared: Shoot.Action(); break;
				case EJamShootState::Rolling: Shoot.ReportProgress(Shoot.GetStage()->Count); break;
				case EJamShootState::NotePrompt: Shoot.AnswerNote(Answer == 1); break;
				case EJamShootState::FailedTake: Shoot.Retake(); break;
				default: break;
				}
				TArray<FJamShootEvent> Events;
				Shoot.DrainEvents(Events);
				for (const FJamShootEvent& E : Events) { Wraps += E.Type == EJamShootEventType::Wrapped ? 1 : 0; }
			}
			TestEqual(*FString::Printf(TEXT("%s (answer %d) reaches the wrap"), *Scene.Id.ToString(), Answer), Shoot.GetState(), EJamShootState::Wrap);
			TestEqual(TEXT("wrapped once"), Wraps, 1);
			TestEqual(TEXT("notes answered"), Shoot.GetNotesTaken() + Shoot.GetNotesRefused(), Scene.NoteBeforeStage == INDEX_NONE ? 0 : 1);
		}
	}
	return true;
}

#endif
