#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JamGameMode.generated.h"

class AJamHero;
class AJamNuisance;

/**
 * The only place that changes shared game state. Spawns and resets stage actors, validates requests,
 * parses game flags.
 * T05: one test set (a box in front of the hero) with three rats that wander and flee.
 *   -autoplay   a bot chases, lectures and treats every rat, then logs JamAutoplay PASS/FAIL
 */
UCLASS()
class MEGAJAM_API AJamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJamGameMode();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Tick(float DeltaSeconds) override;

	/** A hero asks to treat a nuisance. Validates range and state, then treats it. */
	bool HandleInteract(AJamHero* Hero, AJamNuisance* Target);

private:
	void SpawnTestSet(const AJamHero* Hero);
	void HandlePacified(AActor* Pacified);
	void TickAutoplay(float DeltaSeconds);

	UPROPERTY(Transient)
	TArray<TObjectPtr<AJamNuisance>> Nuisances;

	FBox2D TestSetBounds = FBox2D(ForceInit);
	float GroundZ = 0.f;

	TWeakObjectPtr<AJamHero> AutoplayHero;
	float AutoplayTime = 0.f;
	bool bAutoplayDone = false;
	int32 PacifiedCount = 0;
	int32 TreatedCount = 0;
};
