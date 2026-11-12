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
 * Scene 1 plays on set 0 of the backlot (AJamBacklot, T08).
 *   -autoplay   a bot plays the scene: chases, lectures, treats; logs JamAutoplay PASS/FAIL at the wrap,
 *               then walks the street to the next set and logs that too
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
	void TickAutoplayWalk(float DeltaSeconds);

	UPROPERTY(Transient)
	TArray<TObjectPtr<AJamNuisance>> Nuisances;

	TWeakObjectPtr<AJamHero> Hero;
	/** Backlot set the current scene plays on, and its floor inside the walls. */
	int32 CurrentSet = 0;
	FBox2D FieldBounds = FBox2D(ForceInit);
	float GroundZ = 0.f;

	/** The shoot state last seen, and when it was entered (drives the automatic slate/next/retake). */
	EJamShootState SeenState = EJamShootState::Idle;
	float StateEnteredAt = 0.f;

	bool bAutoplay = false;
	bool bAutoplayDone = false;
	bool bAutoplayWalk = false;
	float WalkStartedAt = 0.f;
	float AutoplayTime = 0.f;
	int32 FleeCountTotal = 0;
};
