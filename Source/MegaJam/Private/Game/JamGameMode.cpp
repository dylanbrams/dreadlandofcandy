#include "Game/JamGameMode.h"
#include "Game/JamHero.h"
#include "Game/JamNuisance.h"
#include "Game/JamPlayerState.h"
#include "Gas/JamResolveAttributeSet.h"
#include "MegaJam.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "TimerManager.h"

AJamGameMode::AJamGameMode()
{
	DefaultPawnClass = AJamHero::StaticClass();
	PlayerStateClass = AJamPlayerState::StaticClass();
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
	// The camera looks along +X; start the hero facing up-screen, toward the nuisances.
	Hero->SetActorRotation(FRotator::ZeroRotator);
	SpawnTestNuisances(Hero);

	if (FParse::Param(FCommandLine::Get(), TEXT("autoplay")))
	{
		AutoplayHero = Hero;
		GetWorldTimerManager().SetTimer(AutoplayTimer, this, &AJamGameMode::AutoplayStep, 1.0f, true, 1.0f);
	}
}

void AJamGameMode::SpawnTestNuisances(const AJamHero* Hero)
{
	// Three in a row in front of the hero, inside the Lecture cone (800 uu, 35 degrees).
	const FVector Base = Hero->GetActorLocation() + FVector(0.f, 0.f, -26.f);
	const FVector Offsets[] = { FVector(520.f, -260.f, 0.f), FVector(600.f, 0.f, 0.f), FVector(520.f, 260.f, 0.f) };

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	for (const FVector& Offset : Offsets)
	{
		const FVector Loc = Base + Offset;
		const FRotator Facing = (Hero->GetActorLocation() - Loc).GetSafeNormal2D().Rotation();
		if (AJamNuisance* N = GetWorld()->SpawnActor<AJamNuisance>(Loc, Facing, Params))
		{
			N->GetResolveSet()->OnPacified.AddUObject(this, &AJamGameMode::HandlePacified);
			Nuisances.Add(N);
		}
	}
	UE_LOG(LogJam, Log, TEXT("JamGameMode: spawned %d test nuisances"), Nuisances.Num());
}

void AJamGameMode::HandlePacified(AActor* Pacified)
{
	++PacifiedCount;
	UE_LOG(LogJam, Log, TEXT("JamGameMode: pacified %s (%d/%d)"),
		Pacified ? *Pacified->GetName() : TEXT("(null)"), PacifiedCount, Nuisances.Num());
}

void AJamGameMode::AutoplayStep()
{
	AJamHero* Hero = AutoplayHero.Get();
	if (!Hero) { GetWorldTimerManager().ClearTimer(AutoplayTimer); return; }

	// Lecture 60 vs Resolve 100: two casts pacify; the 1.5 s cooldown fits the 2 s spacing.
	if (AutoplayCasts < 2)
	{
		if (Hero->RequestLecture()) { ++AutoplayCasts; }
		// Space the casts 2 s apart.
		GetWorldTimerManager().SetTimer(AutoplayTimer, this, &AJamGameMode::AutoplayStep, 2.0f, false);
		return;
	}

	GetWorldTimerManager().ClearTimer(AutoplayTimer);
	const bool bPass = PacifiedCount == Nuisances.Num() && Nuisances.Num() == 3;
	UE_LOG(LogJam, Log, TEXT("JamAutoplay: %s pacified=%d/%d casts=%d"),
		bPass ? TEXT("PASS") : TEXT("FAIL"), PacifiedCount, Nuisances.Num(), AutoplayCasts);
}
