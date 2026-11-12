#include "Game/JamGameMode.h"
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

namespace JamAutoplayTuning
{
	constexpr float TimeoutSeconds = 60.f;
	// Close in to this before lecturing (cone is 800). Inside the rats' 500 flee radius on purpose, like a
	// player would: the bot then has to chase and corner them, which exercises Flee and the set edges.
	constexpr float LectureDistance = 450.f;
	constexpr float InteractDistance = 110.f;   // walk this close before treating
}

AJamGameMode::AJamGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AJamHero::StaticClass();
	PlayerStateClass = AJamPlayerState::StaticClass();
	HUDClass = AJamHUD::StaticClass();
}

void AJamGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	UAbilitySystemGlobals::Get().InitGlobalData();
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	AJamHero* Hero = NewPlayer ? Cast<AJamHero>(NewPlayer->GetPawn()) : nullptr;
	if (!Hero)
	{
		UE_LOG(LogJam, Error, TEXT("JamGameMode: no hero after spawn"));
		return;
	}
	// The camera looks along +X; start the hero facing up-screen, into the set.
	Hero->SetActorRotation(FRotator::ZeroRotator);
	SpawnTestSet(Hero);

	if (FParse::Param(FCommandLine::Get(), TEXT("autoplay")))
	{
		AutoplayHero = Hero;
	}
}

void AJamGameMode::SpawnTestSet(const AJamHero* Hero)
{
	// Placeholder set until the blockout (T08): a 1800 x 1800 box starting just behind the hero.
	const FVector HeroLoc = Hero->GetActorLocation();
	GroundZ = HeroLoc.Z - Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	TestSetBounds = FBox2D(FVector2D(HeroLoc.X - 300.f, HeroLoc.Y - 900.f), FVector2D(HeroLoc.X + 1500.f, HeroLoc.Y + 900.f));

	const FVector2D Spots[] = { FVector2D(700.f, -450.f), FVector2D(900.f, 0.f), FVector2D(700.f, 450.f) };
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FVector2D& Spot : Spots)
	{
		const FVector Loc(HeroLoc.X + Spot.X, HeroLoc.Y + Spot.Y, GroundZ + 70.f);
		AJamNuisance* N = GetWorld()->SpawnActorDeferred<AJamNuisance>(AJamNuisance::StaticClass(), FTransform(Loc),
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!N) { continue; }
		N->SetBounds = TestSetBounds;
		N->FinishSpawning(FTransform(Loc));
		N->GetResolveSet()->OnPacified.AddUObject(this, &AJamGameMode::HandlePacified);
		Nuisances.Add(N);
	}
	UE_LOG(LogJam, Log, TEXT("JamGameMode: test set with %d rats"), Nuisances.Num());
}

void AJamGameMode::HandlePacified(AActor* Pacified)
{
	++PacifiedCount;
	UE_LOG(LogJam, Log, TEXT("JamGameMode: pacified %s (%d/%d)"),
		Pacified ? *Pacified->GetName() : TEXT("(null)"), PacifiedCount, Nuisances.Num());
}

bool AJamGameMode::HandleInteract(AJamHero* Hero, AJamNuisance* Target)
{
	if (!Hero || !Target || !Target->IsPacified()) { return false; }
	// The hero picked the target; check range again here, as a server would.
	if (FVector::Dist2D(Hero->GetActorLocation(), Target->GetActorLocation()) > 250.f) { return false; }
	if (!Target->Treat()) { return false; }
	++TreatedCount;
	UE_LOG(LogJam, Log, TEXT("JamGameMode: Treated %s (%d/%d)"), *Target->GetName(), TreatedCount, Nuisances.Num());
	return true;
}

void AJamGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Development aid: the set's edges.
	if (TestSetBounds.bIsValid)
	{
		const FVector Center(TestSetBounds.GetCenter(), GroundZ + 2.f);
		const FVector Extent(TestSetBounds.GetExtent(), 1.f);
		DrawDebugBox(GetWorld(), Center, Extent, FColor(255, 200, 80), false, -1.f, 0, 6.f);
	}

	if (AutoplayHero.IsValid() && !bAutoplayDone)
	{
		TickAutoplay(DeltaSeconds);
	}
}

void AJamGameMode::TickAutoplay(float DeltaSeconds)
{
	AJamHero* Hero = AutoplayHero.Get();
	AutoplayTime += DeltaSeconds;

	// Target: the nearest rat that is not treated yet (pacified ones first: they are waiting).
	AJamNuisance* Target = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (AJamNuisance* N : Nuisances)
	{
		if (!N || N->IsTreated()) { continue; }
		const float Score = FVector::Dist2D(N->GetActorLocation(), Hero->GetActorLocation()) - (N->IsPacified() ? 5000.f : 0.f);
		if (Score < BestScore) { BestScore = Score; Target = N; }
	}

	if (!Target)
	{
		bAutoplayDone = true;
		Hero->SetAimOverride(FVector::ZeroVector);
		int32 Flees = 0;
		for (const AJamNuisance* N : Nuisances) { Flees += N ? N->GetFleeCount() : 0; }
		UE_LOG(LogJam, Log, TEXT("JamAutoplay: PASS pacified=%d/%d treated=%d/%d flees=%d time=%.1f"),
			PacifiedCount, Nuisances.Num(), TreatedCount, Nuisances.Num(), Flees, AutoplayTime);
		return;
	}
	if (AutoplayTime > JamAutoplayTuning::TimeoutSeconds)
	{
		bAutoplayDone = true;
		UE_LOG(LogJam, Log, TEXT("JamAutoplay: FAIL timeout pacified=%d/%d treated=%d/%d"),
			PacifiedCount, Nuisances.Num(), TreatedCount, Nuisances.Num());
		return;
	}

	const FVector ToTarget = (Target->GetActorLocation() - Hero->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
	const float Distance = ToTarget.Size();
	Hero->SetAimOverride(ToTarget);

	if (Target->IsPacified())
	{
		if (Distance > JamAutoplayTuning::InteractDistance) { Hero->AddMovementInput(ToTarget.GetSafeNormal()); }
		else { Hero->RequestInteract(); }
	}
	else
	{
		if (Distance > JamAutoplayTuning::LectureDistance) { Hero->AddMovementInput(ToTarget.GetSafeNormal()); }
		else { Hero->RequestLecture(); }   // the cooldown refuses the spam
	}
}
