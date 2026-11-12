#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Film/JamFilmTypes.h"
#include "JamGameMode.generated.h"

class AJamHero;
class AJamNuisance;
class AJamGameState;

/**
 * The only place that changes shared game state. Drives the FJamShoot held by AJamGameState: feeds it
 * world reports (pacified, treated), drains its events, spawns and resets stage actors, shows cards.
 * T06: Scene 1 on a placeholder field set (an orange box in front of the hero).
 *   -autoplay   a bot plays the scene: chases, lectures, treats; logs JamAutoplay PASS/FAIL at the wrap
 */
UCLASS()
class MEGAJAM_API AJamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJamGameMode();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Tick(float DeltaSeconds) override;

	/** A hero asks to treat a nuisance. Validates stage, range and state, then treats it. */
	bool HandleInteract(AJamHero* Hero, AJamNuisance* Target);

private:
	AJamGameState* GetJamGameState() const;
	void StartScene(const FJamSceneDef& Scene);
	void TickShoot(float DeltaSeconds);
	void HandleShootEvent(const FJamShootEvent& Event);
	void ShowCard(const FString& Title, const FString& Subtitle, float Seconds);
	void RespawnTargets(EJamTargetKind Kind, int32 Count);
	void HandlePacified(AActor* Pacified);
	void TickAutoplay(float DeltaSeconds);

	UPROPERTY(Transient)
	TArray<TObjectPtr<AJamNuisance>> Nuisances;

	TWeakObjectPtr<AJamHero> Hero;
	FBox2D FieldBounds = FBox2D(ForceInit);
	float GroundZ = 0.f;

	/** The shoot state last seen, and when it was entered (drives the automatic slate/next/retake). */
	EJamShootState SeenState = EJamShootState::Idle;
	float StateEnteredAt = 0.f;

	bool bAutoplay = false;
	bool bAutoplayDone = false;
	float AutoplayTime = 0.f;
	int32 FleeCountTotal = 0;
};
