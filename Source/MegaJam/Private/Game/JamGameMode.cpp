#include "Game/JamGameMode.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamHUD.h"
#include "Game/JamNuisance.h"
#include "Game/JamPlayerState.h"
#include "Gas/JamResolveAttributeSet.h"
#include "MegaJam.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"

namespace JamShootPacing
{
	// How long the game lingers in each shoot state before moving on by itself.
	constexpr float SlateSeconds = 2.0f;
	constexpr float ClearedSeconds = 1.2f;
	constexpr float FailedSeconds = 2.5f;
}

namespace JamAutoplayTuning
{
	constexpr float TimeoutSeconds = 90.f;
	// Close in to this before lecturing (cone is 800). Inside the rats' 500 flee radius on purpose, like a
	// player would: the bot then has to chase and corner them, which exercises Flee and the set edges.
	constexpr float LectureDistance = 450.f;
	constexpr float InteractDistance = 110.f;   // walk this close before treating
}

namespace JamRatTuning
{
	// BUILD_SPEC section 10.
	constexpr float Speed = 300.f;
	constexpr float FleeRadius = 500.f;
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
	// The camera looks along +X; start the hero facing up-screen, into the set.
	NewHero->SetActorRotation(FRotator::ZeroRotator);

	// Placeholder field set until the blockout (T08): an 1800 x 1800 box starting just behind the hero.
	const FVector HeroLoc = NewHero->GetActorLocation();
	GroundZ = HeroLoc.Z - NewHero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FieldBounds = FBox2D(FVector2D(HeroLoc.X - 300.f, HeroLoc.Y - 900.f), FVector2D(HeroLoc.X + 1500.f, HeroLoc.Y + 900.f));

	bAutoplay = FParse::Param(FCommandLine::Get(), TEXT("autoplay"));
	StartScene(JamScenes::Scene1());
}

void AJamGameMode::StartScene(const FJamSceneDef& Scene)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	GS->Shoot.Start(Scene);
	SeenState = EJamShootState::Idle;   // TickShoot notices the Slate and shows its card
	UE_LOG(LogJam, Log, TEXT("JamGameMode: scene %s \"%s\" started"), *Scene.Id.ToString(), *Scene.Title);
}

void AJamGameMode::ShowCard(const FString& Title, const FString& Subtitle, float Seconds)
{
	if (AJamGameState* GS = GetJamGameState())
	{
		GS->CardTitle = Title;
		GS->CardSubtitle = Subtitle;
		GS->CardUntil = Seconds < 0.f ? -1.f : GetWorld()->GetTimeSeconds() + Seconds;
	}
}

void AJamGameMode::RespawnTargets(EJamTargetKind Kind, int32 Count)
{
	for (AJamNuisance* N : Nuisances)
	{
		if (N) { FleeCountTotal += N->GetFleeCount(); N->Destroy(); }
	}
	Nuisances.Reset();

	// T06: rats only; other kinds arrive with their scenes.
	const FVector2D Spots[] = { FVector2D(700.f, -450.f), FVector2D(900.f, 0.f), FVector2D(700.f, 450.f) };
	const FVector2D Origin(FieldBounds.Min.X + 300.f, FieldBounds.GetCenter().Y);
	for (int32 i = 0; i < Count && i < UE_ARRAY_COUNT(Spots); ++i)
	{
		const FVector Loc(Origin + Spots[i], GroundZ + 70.f);
		AJamNuisance* N = GetWorld()->SpawnActorDeferred<AJamNuisance>(AJamNuisance::StaticClass(), FTransform(Loc),
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!N) { continue; }
		N->SetBounds = FieldBounds;
		N->Speed = JamRatTuning::Speed;
		N->FleeRadius = JamRatTuning::FleeRadius;
		N->FinishSpawning(FTransform(Loc));
		N->GetResolveSet()->OnPacified.AddUObject(this, &AJamGameMode::HandlePacified);
		Nuisances.Add(N);
	}
}

void AJamGameMode::HandlePacified(AActor* Pacified)
{
	AJamGameState* GS = GetJamGameState();
	const FJamStageDef* Stage = GS ? GS->Shoot.GetStage() : nullptr;
	if (Stage && Stage->Verb == EJamStageVerb::Pacify)
	{
		GS->Shoot.ReportProgress(1);
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
	return true;
}

void AJamGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Development aid: the set's edges.
	if (FieldBounds.bIsValid)
	{
		const FVector Center(FieldBounds.GetCenter(), GroundZ + 2.f);
		const FVector Extent(FieldBounds.GetExtent(), 1.f);
		DrawDebugBox(GetWorld(), Center, Extent, FColor(255, 200, 80), false, -1.f, 0, 6.f);
	}

	TickShoot(DeltaSeconds);
	if (bAutoplay && !bAutoplayDone && Hero.IsValid())
	{
		TickAutoplay(DeltaSeconds);
	}
}

void AJamGameMode::TickShoot(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	FJamShoot& Shoot = GS->Shoot;
	Shoot.Tick(DeltaSeconds);

	const float Now = GetWorld()->GetTimeSeconds();
	if (Shoot.GetState() != SeenState)
	{
		SeenState = Shoot.GetState();
		StateEnteredAt = Now;
		if (SeenState == EJamShootState::Slate)
		{
			ShowCard(FString::Printf(TEXT("SCENE 1 - TAKE %d"), Shoot.GetTake()),
				FString::Printf(TEXT("%s\nDirector: F. Flicker. Camera: F. Flicker. Rats: Themselves."), *Shoot.GetScene().Title),
				JamShootPacing::SlateSeconds);
		}
	}

	// Automatic pacing: the slate rolls, cleared stages move on, failed takes go again.
	const float InState = Now - StateEnteredAt;
	switch (Shoot.GetState())
	{
	case EJamShootState::Slate:        if (InState >= JamShootPacing::SlateSeconds)   { Shoot.Action(); } break;
	case EJamShootState::StageCleared: if (InState >= JamShootPacing::ClearedSeconds) { Shoot.Action(); } break;
	case EJamShootState::FailedTake:   if (InState >= JamShootPacing::FailedSeconds)  { Shoot.Retake(); } break;
	default: break;
	}

	TArray<FJamShootEvent> Events;
	Shoot.DrainEvents(Events);
	for (const FJamShootEvent& Event : Events)
	{
		HandleShootEvent(Event);
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
		UE_LOG(LogJam, Log, TEXT("JamShoot: stage %d (%s) take %d rolling"), Event.Stage, Stage ? *Stage->Id.ToString() : TEXT("?"), Event.Take);
		if (Stage && Stage->Verb == EJamStageVerb::Pacify)
		{
			RespawnTargets(Stage->Target, Stage->Count);
			for (AJamNuisance* N : Nuisances)
			{
				// "This time the rats know their lines": no fleeing until the forced retake.
				if (N) { N->FleeRadius = Shoot.TargetsFlee() ? JamRatTuning::FleeRadius : 0.f; }
			}
		}
		ShowCard(TEXT("ACTION!"), Event.Take == 1 ? TEXT("Places, everyone! And by everyone I mean you.")
			: TEXT("And... ACTION! I love saying that. Ninth time today."), 1.2f);
		break;

	case EJamShootEventType::StageCleared:
		UE_LOG(LogJam, Log, TEXT("JamShoot: stage %d cleared on take %d"), Event.Stage, Event.Take);
		break;

	case EJamShootEventType::TakeFailed:
		UE_LOG(LogJam, Log, TEXT("JamShoot: take %d failed (%s)"), Event.Take, Event.Value ? TEXT("forced") : TEXT("failure"));
		if (Event.Value) { ShowCard(TEXT("Perfect! Again."), TEXT("This time the rats know their lines."), JamShootPacing::FailedSeconds); }
		else { ShowCard(TEXT("RETAKE!"), TEXT("From the top."), JamShootPacing::FailedSeconds); }
		break;

	case EJamShootEventType::Wrapped:
		UE_LOG(LogJam, Log, TEXT("JamShoot: wrapped after %d takes"), Event.Take);
		ShowCard(TEXT("THAT'S A WRAP!"), TEXT("Everyone gets paid. \"Everyone\" is a term of art."), -1.f);
		if (bAutoplay && !bAutoplayDone)
		{
			bAutoplayDone = true;
			int32 Flees = FleeCountTotal;
			for (const AJamNuisance* N : Nuisances) { Flees += N ? N->GetFleeCount() : 0; }
			UE_LOG(LogJam, Log, TEXT("JamAutoplay: PASS scenes=1 takes=%d flees=%d time=%.1f"), Event.Take, Flees, AutoplayTime);
			if (Hero.IsValid()) { Hero->SetAimOverride(FVector::ZeroVector); }
		}
		break;

	default:
		break;
	}
}

void AJamGameMode::TickAutoplay(float DeltaSeconds)
{
	AJamHero* Bot = Hero.Get();
	AutoplayTime += DeltaSeconds;
	if (AutoplayTime > JamAutoplayTuning::TimeoutSeconds)
	{
		bAutoplayDone = true;
		const AJamGameState* GS = GetJamGameState();
		UE_LOG(LogJam, Log, TEXT("JamAutoplay: FAIL timeout stage=%d take=%d progress=%d"),
			GS ? GS->GetShoot().GetStageIndex() : -1, GS ? GS->GetShoot().GetTake() : -1, GS ? GS->GetShoot().GetProgress() : -1);
		return;
	}

	const AJamGameState* GS = GetJamGameState();
	const FJamStageDef* Stage = GS ? GS->GetShoot().GetStage() : nullptr;
	if (!Stage || GS->GetShoot().GetState() != EJamShootState::Rolling) { return; }

	// Target: the nearest rat this stage still needs.
	const bool bTreating = Stage->Verb == EJamStageVerb::Interact;
	AJamNuisance* Target = nullptr;
	float Best = TNumericLimits<float>::Max();
	for (AJamNuisance* N : Nuisances)
	{
		if (!N || N->IsTreated() || N->IsPacified() != bTreating) { continue; }
		const float D = FVector::Dist2D(N->GetActorLocation(), Bot->GetActorLocation());
		if (D < Best) { Best = D; Target = N; }
	}
	if (!Target) { return; }

	const FVector ToTarget = (Target->GetActorLocation() - Bot->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
	Bot->SetAimOverride(ToTarget);
	const float Reach = bTreating ? JamAutoplayTuning::InteractDistance : JamAutoplayTuning::LectureDistance;
	if (ToTarget.Size() > Reach) { Bot->AddMovementInput(ToTarget.GetSafeNormal()); }
	else if (bTreating) { Bot->RequestInteract(); }
	else { Bot->RequestLecture(); }   // the cooldown refuses the spam
}
