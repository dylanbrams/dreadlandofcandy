#include "Game/JamGameMode.h"
#include "Game/JamBacklot.h"
#include "Game/JamFaerie.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamHUD.h"
#include "Game/JamNuisance.h"
#include "Game/JamPickup.h"
#include "Game/JamPlayerState.h"
#include "Film/JamLines.h"
#include "Film/JamShoot.h"
#include "Gas/JamResolveAttributeSet.h"
#include "MegaJam.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/CommandLine.h"

namespace JamShootPacing
{
	// How long the game lingers in each shoot state before moving on by itself.
	constexpr float SlateSeconds = 2.0f;
	constexpr float ClearedSeconds = 1.2f;
	constexpr float FailedSeconds = 2.5f;
	constexpr float WrapCardSeconds = 4.0f;
}

namespace JamFenellaTuning
{
	constexpr int32 RunSeed = 2026;
	constexpr float WatchRadius = 1500.f;   // she films the nearest loose nuisance within this of the hero
	float CaptionSeconds(const FString& Text) { return FMath::Clamp(2.f + Text.Len() * 0.055f, 3.f, 7.f); }
	/** Story triggers come first in the enum; they queue instead of being dropped. */
	bool IsStory(EJamLineTrigger T) { return (int32)T < (int32)EJamLineTrigger::Idle; }
}

namespace JamAutoplayTuning
{
	/** Per scene, in game time (walking to it included). */
	constexpr float SceneTimeoutSeconds = 180.f;
	// Close in to this before lecturing (cone is 800). Inside the rats' 500 flee radius on purpose, like a
	// player would: the bot then has to chase and corner them, which exercises Flee and the set edges.
	constexpr float LectureDistance = 450.f;
	constexpr float InteractDistance = 110.f;   // walk this close before treating
	constexpr float NoteThinkSeconds = 1.5f;    // reads the CUT! card before answering
}

namespace JamStageTuning
{
	constexpr float CollectRadius = 120.f;
	/** Meter stages: each loose nuisance eats one crop per this many seconds. */
	constexpr float SecondsPerCropPerBug = 15.f;
	/** The hero steps this far onto the next set before its scene starts. */
	constexpr float SetEntryDepth = 250.f;
}

namespace
{
	// BUILD_SPEC section 10.
	struct FKindTuning { float Resolve; float Speed; float FleeRadius; bool bErratic; FLinearColor Colour; const TCHAR* Cast; const TCHAR* Treat; };

	FKindTuning TuningFor(EJamTargetKind Kind)
	{
		switch (Kind)
		{
		case EJamTargetKind::Goblin: return { 150.f, 350.f, 600.f, false, FLinearColor(0.35f, 0.75f, 0.3f), TEXT("Goblins: Themselves, sadly."), TEXT("Brush teeth") };
		case EJamTargetKind::Bug:    return { 60.f, 450.f, 400.f, true, FLinearColor(0.6f, 0.3f, 0.95f), TEXT("Lollypop bugs: Uncredited."), TEXT("") };
		case EJamTargetKind::Almond: return { 1.f, 0.f, 0.f, false, FLinearColor(0.55f, 0.35f, 0.18f), TEXT("Almonds: Nuts."), TEXT("") };
		default:                     return { 100.f, 300.f, 500.f, false, FLinearColor(1.0f, 0.35f, 0.6f), TEXT("Rats: Themselves."), TEXT("Feed celery") };
		}
	}

	/** Spawn spots on a set: rows of three across, starting 700 uu in from the back edge. */
	FVector2D SpotFor(const FBox2D& Set, int32 Index)
	{
		const int32 Row = Index / 3;
		const int32 Col = Index % 3;
		const float HalfY = Set.GetExtent().Y;
		const float Y = Set.GetCenter().Y + (Col - 1) * HalfY * 0.5f + (Row % 2) * HalfY * 0.2f;
		const float X = FMath::Min(Set.Min.X + 700.f + Row * 350.f, Set.Max.X - 150.f);
		return FVector2D(X, Y);
	}

}

AJamGameMode::AJamGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AJamHero::StaticClass();
	PlayerStateClass = AJamPlayerState::StaticClass();
	GameStateClass = AJamGameState::StaticClass();
	HUDClass = AJamHUD::StaticClass();
}

AJamGameState* AJamGameMode::GetJamGameState() const
{
	return GetGameState<AJamGameState>();
}

void AJamGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	UAbilitySystemGlobals::Get().InitGlobalData();
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	AJamHero* NewHero = NewPlayer ? Cast<AJamHero>(NewPlayer->GetPawn()) : nullptr;
	if (!NewHero)
	{
		UE_LOG(LogJam, Error, TEXT("JamGameMode: no hero after spawn"));
		return;
	}
	Hero = NewHero;

	const TCHAR* Cmd = FCommandLine::Get();
	bAutoplay = FParse::Param(Cmd, TEXT("autoplay"));
	if (FParse::Param(Cmd, TEXT("forcefail")))
	{
		ForceFailAfter = 3.f;
		FParse::Value(Cmd, TEXT("forcefail="), ForceFailAfter);
		UE_LOG(LogJam, Log, TEXT("JamGameMode: -forcefail, the first take fails after %.1f s"), ForceFailAfter);
	}
	FString AutoNote;
	if (FParse::Value(Cmd, TEXT("autonote="), AutoNote)) { bAutoNoteTake = !AutoNote.Equals(TEXT("refuse"), ESearchCase::IgnoreCase); }
	int32 SceneArg = 1;
	FParse::Value(Cmd, TEXT("scene="), SceneArg);
	FirstSceneIndex = FMath::Clamp(SceneArg - 1, 0, JamScenes::All().Num() - 1);
	float TimeScale = 1.f;
	if (FParse::Value(Cmd, TEXT("timescale="), TimeScale) && GetWorldSettings())
	{
		GetWorldSettings()->SetTimeDilation(FMath::Clamp(TimeScale, 0.25f, 8.f));
		UE_LOG(LogJam, Log, TEXT("JamGameMode: time scale %.2f"), GetWorldSettings()->TimeDilation);
	}

	// The backlot (T08): built from its table with floor tops just above the template map's floor, so the two
	// never z-fight. The hero starts on the first scene's set mark.
	const float TemplateFloorZ = NewHero->GetActorLocation().Z - NewHero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GroundZ = TemplateFloorZ + 2.f;
	if (AJamBacklot* Backlot = GetWorld()->SpawnActor<AJamBacklot>(AJamBacklot::StaticClass(), FTransform::Identity))
	{
		Backlot->Build(GroundZ);
	}
	const FVector2D Mark = JamBacklot::HeroMark(FirstSceneIndex);
	NewHero->TeleportTo(FVector(Mark, GroundZ + NewHero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.f), FRotator::ZeroRotator);

	Nag.Reset(JamFenellaTuning::RunSeed);
	FActorSpawnParameters FaerieParams;
	FaerieParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Faerie = GetWorld()->SpawnActor<AJamFaerie>(AJamFaerie::StaticClass(), NewHero->GetActorTransform(), FaerieParams);
	if (Faerie) { Faerie->SetFollow(NewHero); }

	StartScene(FirstSceneIndex);
}

void AJamGameMode::StartScene(int32 InSceneIndex)
{
	AJamGameState* GS = GetJamGameState();
	const TArray<FJamSceneDef> Scenes = JamScenes::All();
	if (!GS || !Scenes.IsValidIndex(InSceneIndex) || !JamBacklot::Sets().IsValidIndex(InSceneIndex)) { return; }

	SceneIndex = InSceneIndex;
	FieldBounds = JamBacklot::Sets()[SceneIndex].Bounds;
	bBetweenScenes = false;
	SceneTime = 0.f;
	ClearStageActors();
	GS->SceneNumber = SceneIndex + 1;
	GS->Hint.Reset();
	GS->Shoot.Start(Scenes[SceneIndex]);
	SeenState = EJamShootState::Idle;   // TickShoot notices the Slate and shows its card
	UE_LOG(LogJam, Log, TEXT("JamGameMode: scene %d %s \"%s\" started"), SceneIndex + 1, *Scenes[SceneIndex].Id.ToString(), *Scenes[SceneIndex].Title);
}

void AJamGameMode::ShowCard(EJamCardKind Kind, const FString& Title, const FString& Subtitle, float Seconds)
{
	if (AJamGameState* GS = GetJamGameState())
	{
		GS->CardKind = Kind;
		GS->CardTitle = Title;
		GS->CardSubtitle = Subtitle;
		GS->CardUntil = Seconds < 0.f ? -1.f : GetWorld()->GetTimeSeconds() + Seconds;
	}
}

void AJamGameMode::ClearStageActors()
{
	for (AJamNuisance* N : Nuisances)
	{
		if (N) { FleeCountTotal += N->GetFleeCount(); N->Destroy(); }
	}
	Nuisances.Reset();
	for (AJamPickup* P : Pickups)
	{
		if (P) { P->Destroy(); }
	}
	Pickups.Reset();
	CropsEaten = 0.f;
}

void AJamGameMode::SpawnTargets(const FJamStageDef& Stage, int32 Count, bool bPacified)
{
	const FKindTuning Tuning = TuningFor(Stage.Target);
	const int32 Already = Nuisances.Num();
	for (int32 i = 0; i < Count; ++i)
	{
		const FVector Loc(SpotFor(FieldBounds, Already + i), GroundZ + 70.f);
		AJamNuisance* N = GetWorld()->SpawnActorDeferred<AJamNuisance>(AJamNuisance::StaticClass(), FTransform(Loc),
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!N) { continue; }
		N->SetBounds = FieldBounds;
		N->Speed = Tuning.Speed * Stage.SpeedMultiplier;
		// Boss-sized note targets (Resolve x3 and up) do not flee (BUILD_SPEC 10).
		N->FleeRadius = Stage.ResolveMultiplier >= 3.f ? 0.f : Tuning.FleeRadius;
		N->MaxResolveValue = Tuning.Resolve * Stage.ResolveMultiplier;
		N->BaseColour = Tuning.Colour;
		N->bErratic = Tuning.bErratic;
		N->bStartPacified = bPacified;
		N->FinishSpawning(FTransform(Loc));
		if (Stage.ResolveMultiplier >= 3.f) { N->SetActorScale3D(FVector(1.8f)); }
		N->GetResolveSet()->OnPacified.AddUObject(this, &AJamGameMode::HandlePacified);
		Nuisances.Add(N);
	}
}

void AJamGameMode::PrepareTreatStage(const FJamStageDef& Stage)
{
	// A note can swap in a different target (the very large goblin): fresh, un-pacified, pacify then treat.
	if (!FMath::IsNearlyEqual(Stage.ResolveMultiplier, 1.f))
	{
		ClearStageActors();
		SpawnTargets(Stage, Stage.Count, false);
		return;
	}
	// Otherwise treat the ones the previous stage pacified; a retake un-treats them.
	int32 Ready = 0;
	for (AJamNuisance* N : Nuisances)
	{
		if (!N) { continue; }
		N->ResetToPacified();
		if (N->IsPacified()) { ++Ready; }
	}
	if (Ready < Stage.Count) { SpawnTargets(Stage, Stage.Count - Ready, true); }
}

void AJamGameMode::SpawnPickups(int32 Count)
{
	for (AJamPickup* P : Pickups) { if (P) { P->Destroy(); } }
	Pickups.Reset();
	for (int32 i = 0; i < Count; ++i)
	{
		// Pickups sit between the nuisance rows, so they don't land under a sitting bug.
		const FVector2D Spot = SpotFor(FieldBounds, i + 1) + FVector2D(175.f, 0.f);
		if (AJamPickup* P = GetWorld()->SpawnActor<AJamPickup>(AJamPickup::StaticClass(), FTransform(FVector(Spot, GroundZ + 25.f))))
		{
			Pickups.Add(P);
		}
	}
}

void AJamGameMode::HandlePacified(AActor* Pacified)
{
	AJamGameState* GS = GetJamGameState();
	const FJamStageDef* Stage = GS ? GS->Shoot.GetStage() : nullptr;
	if (Stage && Stage->Verb == EJamStageVerb::Pacify)
	{
		GS->Shoot.ReportProgress(1);
		if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Pacify); }
		Say(EJamLineTrigger::Pacified);
	}
	UE_LOG(LogJam, Log, TEXT("JamGameMode: pacified %s"), Pacified ? *Pacified->GetName() : TEXT("(null)"));
}

bool AJamGameMode::HandleInteract(AJamHero* InHero, AJamNuisance* Target)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || !GS->CanInteract()) { return false; }
	if (!InHero || !Target || !Target->IsPacified()) { return false; }
	// The hero picked the target; check range again here, as a server would.
	if (FVector::Dist2D(InHero->GetActorLocation(), Target->GetActorLocation()) > 250.f) { return false; }
	if (!Target->Treat()) { return false; }
	UE_LOG(LogJam, Log, TEXT("JamGameMode: Treated %s"), *Target->GetName());
	GS->Shoot.ReportProgress(1);
	if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Treat); }
	Say(EJamLineTrigger::Interact);
	return true;
}

bool AJamGameMode::HandleNoteAnswer(AJamHero* InHero, bool bTake)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || !InHero || !GS->IsNotePrompt()) { return false; }
	return GS->Shoot.AnswerNote(bTake);
}

void AJamGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SceneTime += DeltaSeconds;
	if (bBetweenScenes) { TickBetweenScenes(); }
	else { TickShoot(DeltaSeconds); }
	TickFenella(DeltaSeconds);
	if (bAutoplay && !bRunOver && Hero.IsValid())
	{
		TickAutoplay(DeltaSeconds);
	}
}

void AJamGameMode::TickBetweenScenes()
{
	const int32 Next = SceneIndex + 1;
	const AJamHero* H = Hero.Get();
	if (!H || !JamBacklot::Sets().IsValidIndex(Next)) { return; }
	const FBox2D& Set = JamBacklot::Sets()[Next].Bounds;
	const FVector2D Here(H->GetActorLocation());
	if (Set.IsInside(Here) && Here.X > Set.Min.X + JamStageTuning::SetEntryDepth)
	{
		UE_LOG(LogJam, Log, TEXT("JamGameMode: hero reached set %s"), *JamBacklot::Sets()[Next].Id.ToString());
		StartScene(Next);
	}
}

void AJamGameMode::TickShoot(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	FJamShoot& Shoot = GS->Shoot;
	Shoot.Tick(DeltaSeconds);
	if (Shoot.GetState() == EJamShootState::Rolling) { GS->FilmSeconds += DeltaSeconds; }
	TickStage(DeltaSeconds);

	const float Now = GetWorld()->GetTimeSeconds();
	if (Shoot.GetState() != SeenState)
	{
		SeenState = Shoot.GetState();
		StateEnteredAt = Now;
		if (SeenState == EJamShootState::Slate)
		{
			const FJamStageDef* Stage = Shoot.GetStage();
			ShowCard(EJamCardKind::Slate, FString::Printf(TEXT("SCENE %d - TAKE %d"), SceneIndex + 1, Shoot.GetTake()),
				FString::Printf(TEXT("%s\nDirector: F. Flicker. Camera: F. Flicker. %s"), *Shoot.GetScene().Title,
					TuningFor(Stage ? Stage->Target : EJamTargetKind::Rat).Cast),
				JamShootPacing::SlateSeconds);
			// Take 1: she introduces the scene (and, once, the hero). Take 2 already got the failed-take line.
			if (Shoot.GetTake() == 1)
			{
				Say(EJamLineTrigger::SceneStart);
				if (!bSaidLevel7Intro) { bSaidLevel7Intro = true; Say(EJamLineTrigger::Level7Intro); }
			}
			else if (Shoot.GetTake() >= 3)
			{
				Say(JamLines::TriggerForTake(Shoot.GetTake()));
			}
		}
		else if (SeenState == EJamShootState::NotePrompt)
		{
			const FJamLine* Pitch = JamLines::Find(Shoot.GetScene().Note.PitchLineId);
			const FString PitchText = Pitch ? Pitch->Text : FString(TEXT("I have a note."));
			ShowCard(EJamCardKind::Cut, TEXT("CUT!"), PitchText, -1.f);
			UE_LOG(LogJam, Log, TEXT("JamShoot: CUT! note %s pitched"), *Shoot.GetScene().Note.Id.ToString());
			if (SceneIndex == 2)
			{
				if (AJamPlayerState* PS = GetHeroState())
				{
					const int32 Gained = PS->EditProgression().ReachMilestone(EJamMilestone::Scene3FinalNote);
					UE_LOG(LogJam, Log, TEXT("JamGameMode: level %d (+%d) at the final CUT!"), PS->GetProgression().GetLevel(), Gained);
				}
			}
		}
	}

	// Automatic pacing: the slate rolls, cleared stages move on, failed takes go again.
	const float InState = Now - StateEnteredAt;
	switch (Shoot.GetState())
	{
	case EJamShootState::Slate:        if (InState >= JamShootPacing::SlateSeconds)   { Shoot.Action(); } break;
	case EJamShootState::StageCleared: if (InState >= JamShootPacing::ClearedSeconds) { Shoot.Action(); } break;
	case EJamShootState::FailedTake:
		if (InState >= JamShootPacing::FailedSeconds && Shoot.Retake()) { ResetHeroToMark(); }
		break;
	case EJamShootState::Rolling:
		if (ForceFailAfter >= 0.f && !bForceFailDone && InState >= ForceFailAfter)
		{
			bForceFailDone = true;
			bFailForcedByFlag = true;
			UE_LOG(LogJam, Log, TEXT("JamGameMode: -forcefail fails take %d"), Shoot.GetTake());
			Shoot.ReportFail();
		}
		break;
	default: break;
	}

	TArray<FJamShootEvent> Events;
	Shoot.DrainEvents(Events);
	for (const FJamShootEvent& Event : Events)
	{
		HandleShootEvent(Event);
	}
}

void AJamGameMode::TickStage(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	FJamShoot& Shoot = GS->Shoot;
	const FJamStageDef* Stage = Shoot.GetStage();
	if (!Stage || Shoot.GetState() != EJamShootState::Rolling) { return; }

	// Collect: walking over a pickup gathers it.
	if (Stage->Verb == EJamStageVerb::Collect && Hero.IsValid())
	{
		for (int32 i = Pickups.Num() - 1; i >= 0; --i)
		{
			AJamPickup* P = Pickups[i];
			if (P && FVector::Dist2D(P->GetActorLocation(), Hero->GetActorLocation()) < JamStageTuning::CollectRadius)
			{
				P->Destroy();
				Pickups.RemoveAt(i);
				UE_LOG(LogJam, Log, TEXT("JamGameMode: collected a pickup"));
				if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Collect); }
				Shoot.ReportProgress(1);
			}
		}
	}

	// Meter: loose nuisances eat the crops.
	if (Stage->FailMeter > 0)
	{
		int32 Loose = 0;
		for (const AJamNuisance* N : Nuisances) { Loose += (N && !N->IsPacified() && !N->IsTreated()) ? 1 : 0; }
		CropsEaten += DeltaSeconds * Loose / JamStageTuning::SecondsPerCropPerBug;
		while (CropsEaten >= 1.f && Shoot.GetState() == EJamShootState::Rolling)
		{
			CropsEaten -= 1.f;
			Shoot.ReportMeter(1);
		}
	}
}

void AJamGameMode::HandleShootEvent(const FJamShootEvent& Event)
{
	AJamGameState* GS = GetJamGameState();
	const FJamShoot& Shoot = GS->Shoot;
	const FJamStageDef* Stage = Shoot.GetScene().Stages.IsValidIndex(Event.Stage) ? &Shoot.GetScene().Stages[Event.Stage] : nullptr;

	switch (Event.Type)
	{
	case EJamShootEventType::StageStarted:
		UE_LOG(LogJam, Log, TEXT("JamShoot: scene %d stage %d (%s) take %d rolling"), SceneIndex + 1, Event.Stage, Stage ? *Stage->Id.ToString() : TEXT("?"), Event.Take);
		CropsEaten = 0.f;
		if (Stage)
		{
			switch (Stage->Verb)
			{
			case EJamStageVerb::Pacify:
				ClearStageActors();
				SpawnTargets(*Stage, Stage->Count, false);
				// "This time the rats know their lines": no fleeing until the forced retake.
				if (!Shoot.TargetsFlee()) { for (AJamNuisance* N : Nuisances) { if (N) { N->FleeRadius = 0.f; } } }
				break;
			case EJamStageVerb::Interact:
				PrepareTreatStage(*Stage);
				break;
			case EJamStageVerb::Collect:
				SpawnPickups(Stage->Count);
				break;
			}
		}
		ShowCard(EJamCardKind::Action, TEXT("ACTION!"), FString(), 1.2f);   // Fenella's caption carries the line
		break;

	case EJamShootEventType::Progress:
		SinceProgress = 0.f;
		break;

	case EJamShootEventType::StageCleared:
		UE_LOG(LogJam, Log, TEXT("JamShoot: stage %d cleared on take %d"), Event.Stage, Event.Take);
		break;

	case EJamShootEventType::NoteAnswered:
		UE_LOG(LogJam, Log, TEXT("JamShoot: note %s"), Event.Value ? TEXT("taken") : TEXT("refused"));
		ShowCard(EJamCardKind::Info, Event.Value ? TEXT("NOTE TAKEN") : TEXT("NOTE REFUSED"), FString(), 2.f);
		(Event.Value ? GS->RunNotesTaken : GS->RunNotesRefused) += 1;
		Say(Event.Value ? EJamLineTrigger::NoteTaken : EJamLineTrigger::NoteRefused);
		break;

	case EJamShootEventType::TakeFailed:
		UE_LOG(LogJam, Log, TEXT("JamShoot: take %d failed (%s)"), Event.Take, Event.Value ? TEXT("forced") : TEXT("failure"));
		if (Event.Value) { ShowCard(EJamCardKind::Retake, TEXT("Perfect! Again."), FString(), JamShootPacing::FailedSeconds); }
		else { ShowCard(EJamCardKind::Retake, TEXT("RETAKE!"), FailReason(bFailForcedByFlag), JamShootPacing::FailedSeconds); }
		bFailForcedByFlag = false;
		GS->TakeFailedAt = GetWorld()->GetTimeSeconds();
		Say(Event.Value ? EJamLineTrigger::ForcedRetake : EJamLineTrigger::FailedTake);
		break;

	case EJamShootEventType::Wrapped:
	{
		UE_LOG(LogJam, Log, TEXT("JamShoot: scene %d wrapped after %d takes"), SceneIndex + 1, Shoot.GetTakesUsed());
		++ScenesWrapped;
		TakesTotal += Shoot.GetTakesUsed();
		NotesTaken += Shoot.GetNotesTaken();
		NotesRefused += Shoot.GetNotesRefused();
		const bool bLast = !JamScenes::All().IsValidIndex(SceneIndex + 1) || !JamBacklot::Sets().IsValidIndex(SceneIndex + 1);
		ShowCard(EJamCardKind::None, FString(), FString(), 0.f);   // the wrap panel says THAT'S A WRAP
		RollWrap(bLast);
		Say(EJamLineTrigger::Wrap);
		if (bLast)
		{
			FinishRun(true, FString());
		}
		else
		{
			bBetweenScenes = true;
			SceneTime = 0.f;
			GS->Hint = FString::Printf(TEXT("Walk up the street to %s"), *JamBacklot::Sets()[SceneIndex + 1].Sign);
		}
		break;
	}

	default:
		break;
	}
}

void AJamGameMode::FinishRun(bool bPass, const FString& Why)
{
	if (bRunOver) { return; }
	bRunOver = true;
	for (const AJamNuisance* N : Nuisances) { FleeCountTotal += N ? N->GetFleeCount() : 0; }
	if (Hero.IsValid()) { Hero->SetAimOverride(FVector::ZeroVector); }
	if (!bAutoplay) { return; }
	if (bPass)
	{
		// BUILD_SPEC 2: mostly taken = the Director's Cut, mostly refused = the Theatrical Release.
		const TCHAR* Ending = NotesTaken >= NotesRefused ? TEXT("directors_cut") : TEXT("theatrical");
		UE_LOG(LogJam, Log, TEXT("JamAutoplay: PASS scenes=%d first=%d takes=%d notes=%d/%d ending=%s flees=%d lines=%d time=%.1f"),
			ScenesWrapped, FirstSceneIndex + 1, TakesTotal, NotesTaken, NotesRefused, Ending, FleeCountTotal, LinesSpoken, AutoplayTime);
	}
	else
	{
		UE_LOG(LogJam, Log, TEXT("JamAutoplay: FAIL %s"), *Why);
	}
}

void AJamGameMode::TickAutoplay(float DeltaSeconds)
{
	AJamHero* Bot = Hero.Get();
	AutoplayTime += DeltaSeconds;
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	const FJamShoot& Shoot = GS->GetShoot();

	if (SceneTime > JamAutoplayTuning::SceneTimeoutSeconds)
	{
		FinishRun(false, FString::Printf(TEXT("timeout scene=%d between=%d stage=%d take=%d progress=%d"),
			SceneIndex + 1, bBetweenScenes ? 1 : 0, Shoot.GetStageIndex(), Shoot.GetTake(), Shoot.GetProgress()));
		return;
	}

	// Between scenes: walk up the street onto the next set. Line up with the street (Y = 0) first, so the bot
	// never pushes into the wall beside an opening.
	if (bBetweenScenes)
	{
		Bot->SetAimOverride(FVector::ZeroVector);
		const FVector2D Goal = JamBacklot::HeroMark(SceneIndex + 1) + FVector2D(JamStageTuning::SetEntryDepth, 0.f);
		const FVector2D Here(Bot->GetActorLocation());
		FVector2D Dir = Goal - Here;
		if (FMath::Abs(Here.Y) > 60.f && Here.X < Goal.X - 600.f) { Dir = FVector2D(0.3f, -Here.Y); }
		Bot->AddMovementInput(FVector(Dir.GetSafeNormal(), 0.f));
		return;
	}

	if (Shoot.GetState() == EJamShootState::NotePrompt)
	{
		if (GetWorld()->GetTimeSeconds() - StateEnteredAt >= JamAutoplayTuning::NoteThinkSeconds)
		{
			if (bAutoNoteTake) { Bot->RequestInteract(); } else { Bot->RequestRefuseNote(); }
		}
		return;
	}

	const FJamStageDef* Stage = Shoot.GetStage();
	if (!Stage || Shoot.GetState() != EJamShootState::Rolling) { return; }

	// Collect: walk to the nearest pickup.
	if (Stage->Verb == EJamStageVerb::Collect)
	{
		const AJamPickup* Best = nullptr;
		float BestD = TNumericLimits<float>::Max();
		for (const AJamPickup* P : Pickups)
		{
			const float D = P ? FVector::Dist2D(P->GetActorLocation(), Bot->GetActorLocation()) : BestD;
			if (D < BestD) { BestD = D; Best = P; }
		}
		if (Best)
		{
			const FVector To = (Best->GetActorLocation() - Bot->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
			Bot->SetAimOverride(To);
			Bot->AddMovementInput(To.GetSafeNormal());
		}
		return;
	}

	// Pacify: the nearest loose target. Interact: lecture any loose target first (a note's boss), then treat.
	bool bTreating = false;
	AJamNuisance* Target = nullptr;
	float Best = TNumericLimits<float>::Max();
	for (int32 Pass = 0; Pass < 2 && !Target; ++Pass)
	{
		bTreating = Pass == 1;
		if (bTreating && Stage->Verb != EJamStageVerb::Interact) { break; }
		for (AJamNuisance* N : Nuisances)
		{
			if (!N || N->IsTreated() || N->IsPacified() != bTreating) { continue; }
			const float D = FVector::Dist2D(N->GetActorLocation(), Bot->GetActorLocation());
			if (D < Best) { Best = D; Target = N; }
		}
	}
	if (!Target) { return; }

	const FVector ToTarget = (Target->GetActorLocation() - Bot->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
	Bot->SetAimOverride(ToTarget);
	const float Reach = bTreating ? JamAutoplayTuning::InteractDistance + Target->GetSimpleCollisionRadius() * (Target->GetActorScale3D().X - 1.f)
		: JamAutoplayTuning::LectureDistance;
	if (ToTarget.Size() > Reach) { Bot->AddMovementInput(ToTarget.GetSafeNormal()); }
	else if (bTreating) { Bot->RequestInteract(); }
	else { Bot->RequestLecture(); }   // the cooldown refuses the spam
}

// ---- Fenella (T11) -------------------------------------------------------------------------------------------

void AJamGameMode::NotifyLectureCast(AJamHero* InHero)
{
	if (InHero && InHero == Hero.Get()) { Say(EJamLineTrigger::LectureCast); }
}

FJamNagSnapshot AJamGameMode::MakeSnapshot() const
{
	FJamNagSnapshot Snap;
	Snap.Now = GetWorld()->GetTimeSeconds();
	Snap.SecondsSinceProgress = SinceProgress;
	Snap.SecondsStrayed = StrayedFor;
	if (const AJamGameState* GS = GetJamGameState())
	{
		Snap.bSilenced = GS->IsNotePrompt();
		Snap.bSulking = GS->GetShoot().IsSulking();
	}
	return Snap;
}

void AJamGameMode::Say(EJamLineTrigger Trigger)
{
	const FJamLine* Line = Nag.Request(Trigger, MakeSnapshot());
	if (!Line) { return; }
	const AJamGameState* GS = GetJamGameState();
	const bool bBusy = !CaptionQueue.IsEmpty() || (GS && GS->IsCaptionVisible());
	if (JamFenellaTuning::IsStory(Trigger)) { CaptionQueue.Add(Line->Id); }
	else if (!bBusy) { ShowCaption(*Line); }
}

void AJamGameMode::ShowCaption(const FJamLine& Line)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	const float Seconds = JamFenellaTuning::CaptionSeconds(Line.Text);
	GS->CaptionLineId = Line.Id;
	GS->CaptionText = Line.Text;
	GS->CaptionUntil = GetWorld()->GetTimeSeconds() + Seconds;
	if (Faerie) { Faerie->Speak(Seconds); }
	++LinesSpoken;
	UE_LOG(LogJam, Log, TEXT("Fenella: %s \"%s\""), *Line.Id.ToString(), *Line.Text);
}

void AJamGameMode::TickFenella(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	const bool bRolling = !bBetweenScenes && GS->GetShoot().GetState() == EJamShootState::Rolling;

	// Idle and strayed clocks only run while a take rolls.
	SinceProgress = bRolling ? SinceProgress + DeltaSeconds : 0.f;
	const bool bOffSet = Hero.IsValid() && !FieldBounds.IsInside(FVector2D(Hero->GetActorLocation()));
	StrayedFor = (bRolling && bOffSet) ? StrayedFor + DeltaSeconds : 0.f;

	// Story lines wait their turn; ambient lines come from the poll when she has nothing else to say.
	if (!GS->IsCaptionVisible())
	{
		if (!CaptionQueue.IsEmpty())
		{
			const FName Id = CaptionQueue[0];
			CaptionQueue.RemoveAt(0);
			if (const FJamLine* Line = JamLines::Find(Id)) { ShowCaption(*Line); }
		}
		else if (!GS->CaptionLineId.IsNone())
		{
			GS->CaptionLineId = NAME_None;
			GS->CaptionText.Reset();
		}
	}
	NagPollIn -= DeltaSeconds;
	if (NagPollIn <= 0.f)
	{
		NagPollIn = 1.f;
		if (bRolling && CaptionQueue.IsEmpty() && !GS->IsCaptionVisible())
		{
			if (const FJamLine* Line = Nag.Poll(MakeSnapshot())) { ShowCaption(*Line); }
		}
	}

	// She films the nearest loose nuisance, or else the hero.
	if (Faerie && Hero.IsValid())
	{
		AActor* Best = nullptr;
		float BestDist = JamFenellaTuning::WatchRadius;
		for (AJamNuisance* N : Nuisances)
		{
			if (!N || N->IsPacified() || N->IsTreated()) { continue; }
			const float D = FVector::Dist2D(N->GetActorLocation(), Hero->GetActorLocation());
			if (D < BestDist) { BestDist = D; Best = N; }
		}
		Faerie->SetSubject(Best);
		Faerie->SetSulking(GS->GetShoot().IsSulking());
	}
}

// ---- Failure (T13) -------------------------------------------------------------------------------------------

FString AJamGameMode::FailReason(bool bForcedByFlag) const
{
	const AJamGameState* GS = GetJamGameState();
	const FJamStageDef* Stage = GS ? GS->GetShoot().GetStage() : nullptr;
	if (bForcedByFlag || !Stage) { return TEXT("Director's call. Back to one!"); }
	if (Stage->FailMeter > 0 && GS->GetShoot().GetMeter() >= Stage->FailMeter) { return TEXT("The crops are gone. Back to one!"); }
	if (Stage->TimeLimit > 0.f && GS->GetShoot().GetTimeLeft() <= 0.f) { return TEXT("Out of time. Back to one!"); }
	return TEXT("From the top. Back to one!");
}

void AJamGameMode::ResetHeroToMark()
{
	AJamHero* H = Hero.Get();
	if (!H || !JamBacklot::Sets().IsValidIndex(SceneIndex)) { return; }
	const FVector2D Mark = JamBacklot::HeroMark(SceneIndex);
	H->TeleportTo(FVector(Mark, GroundZ + H->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.f), FRotator::ZeroRotator);
	H->GetCharacterMovement()->StopMovementImmediately();
	UE_LOG(LogJam, Log, TEXT("JamGameMode: back to one (take %d)"), GetJamGameState()->GetShoot().GetTake());
}

// ---- Wrap, ledger, loot, progression (T14) ---------------------------------------------------------------------

namespace JamWrapTuning
{
	constexpr float PanelSeconds = 14.f;     // closes itself; interact closes it sooner
	constexpr float RefusedGross = 0.8f;     // each refused note in the scene (BUILD_SPEC 3: refusing lowers the gross)
}

AJamPlayerState* AJamGameMode::GetHeroState() const
{
	return Hero.IsValid() ? Hero->GetPlayerState<AJamPlayerState>() : nullptr;
}

void AJamGameMode::ApplyHeroStats()
{
	AJamPlayerState* PS = GetHeroState();
	if (!PS || !Hero.IsValid()) { return; }
	Hero->ApplyStats(PS->GetProgression().GetStats());
}

void AJamGameMode::RollWrap(bool bLastScene)
{
	AJamGameState* GS = GetJamGameState();
	AJamPlayerState* PS = GetHeroState();
	if (!GS || !PS) { return; }
	const FJamShoot& Shoot = GS->Shoot;
	FJamProgression& Prog = PS->EditProgression();

	const float GrossMultiplier = Shoot.GetGrossMultiplier() * FMath::Pow(JamWrapTuning::RefusedGross, Shoot.GetNotesRefused());
	const int32 Seed = SceneIndex * 100 + (Shoot.GetTakesUsed() - 1);
	const FJamWrapOutcome Out = JamWrap::Roll(SceneIndex, GrossMultiplier, Seed);

	const FJamStatBlock Before = Prog.GetStats();
	if (Out.LootIndices.IsValidIndex(Out.EquippedSlot)) { Prog.AddLoot(Out.LootIndices[Out.EquippedSlot]); }
	int32 Gained = 0;
	if (SceneIndex == 0) { Gained = Prog.ReachMilestone(EJamMilestone::Scene1Wrap); }
	else if (SceneIndex == 1) { Gained = Prog.ReachMilestone(EJamMilestone::Scene2Wrap); }
	PS->Sprinkles += Out.Ledger.HeroCut;
	ApplyHeroStats();

	GS->Wrap = JamWrap::MakeView(Out, Shoot.GetScene().Title, Before, Prog.GetStats(), Gained, Seed);
	GS->WrapUntil = bLastScene ? -1.f : GetWorld()->GetTimeSeconds() + JamWrapTuning::PanelSeconds;

	const FJamLootDef& Equipped = JamProgression::LootTable()[Out.LootIndices[Out.EquippedSlot]];
	UE_LOG(LogJam, Log, TEXT("JamWrap: scene %d gross %d cut %d (sprinkles %d), equipped %s (%s), level %d, radius %.0f -> %.0f"),
		SceneIndex + 1, Out.Ledger.Gross, Out.Ledger.HeroCut, PS->Sprinkles, *Equipped.Name, *JamWrap::StatText(Equipped),
		Prog.GetLevel(), Before.Radius, Prog.GetStats().Radius);
}

bool AJamGameMode::HandleDismissWrap(AJamHero* InHero)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || !InHero || !GS->IsWrapVisible() || GS->WrapUntil < 0.f) { return false; }
	GS->WrapUntil = GetWorld()->GetTimeSeconds();
	return true;
}
