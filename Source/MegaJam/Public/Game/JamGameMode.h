#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JamGameMode.generated.h"

class AJamHero;
class AJamNuisance;

/**
 * The only place that changes shared game state. Spawns the hero's stage actors and parses game flags.
 * T01: spawns three static test nuisances in front of the hero and counts pacifications.
 *   -autoplay   the hero lectures on its own (T01: twice, then logs JamAutoplay PASS/FAIL)
 */
UCLASS()
class MEGAJAM_API AJamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJamGameMode();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	void SpawnTestNuisances(const AJamHero* Hero);
	void HandlePacified(AActor* Pacified);
	void AutoplayStep();

	UPROPERTY(Transient)
	TArray<TObjectPtr<AJamNuisance>> Nuisances;

	TWeakObjectPtr<AJamHero> AutoplayHero;
	FTimerHandle AutoplayTimer;
	int32 AutoplayCasts = 0;
	int32 PacifiedCount = 0;
};
