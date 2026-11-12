#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Film/JamFilmTypes.h"
#include "UI/JamHudView.h"
#include "Film/JamNagScheduler.h"
#include "JamGameMode.generated.h"

class AJamHero;
class AJamNuisance;
class AJamPickup;
class AJamGameState;
class AJamFaerie;

/**
 * The only place that changes shared game state. Drives the FJamShoot held by AJamGameState: feeds it
 * world reports (pacified, treated, collected, crops eaten), drains its events, spawns and resets stage
 * actors, shows cards. Scene n plays on backlot set n; after a wrap the hero walks the street to the next
 * set, and stepping onto it starts the next scene.
 *   -autoplay             a bot plays: chases, lectures, treats, collects, answers notes, walks between sets;
 *                         logs JamAutoplay: PASS scenes=N ... ending=<name> after the last wrap, or FAIL <why>
 *   -autonote=take|refuse how the bot answers CUT! (default take)
 *   -scene=<n>            start at scene n (1-based), hero on that set's mark
 *   -timescale=<x>        world time dilation (0.25 to 8), for faster unattended runs
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
	/** A hero answers CUT!. False unless the note prompt is up. */
	bool HandleNoteAnswer(AJamHero* Hero, bool bTake);

	/** Interact while the wrap panel is up closes it. True if it did. */
	bool HandleDismissWrap(AJamHero* Hero);

	/** The hero cast a Lecture (cosmetic: Fenella may remark on it). */
	void NotifyLectureCast(AJamHero* Hero);

private:
	AJamGameState* GetJamGameState() const;
	void StartScene(int32 InSceneIndex);
	void TickShoot(float DeltaSeconds);
	void TickStage(float DeltaSeconds);
	void TickBetweenScenes();
	void HandleShootEvent(const FJamShootEvent& Event);
	void ShowCard(EJamCardKind Kind, const FString& Title, const FString& Subtitle, float Seconds);
	void ClearStageActors();
	void SpawnTargets(const FJamStageDef& Stage, int32 Count, bool bPacified);
	void PrepareTreatStage(const FJamStageDef& Stage);
	void SpawnPickups(int32 Count);
	void HandlePacified(AActor* Pacified);
	void FinishRun(bool bPass, const FString& Why);
	void TickAutoplay(float DeltaSeconds);

	// Fenella (T11): the scheduler picks, the game state shows. Story lines queue; ambient lines only fill silence.
	void Say(EJamLineTrigger Trigger);
	void ShowCaption(const FJamLine& Line);
	void TickFenella(float DeltaSeconds);
	FJamNagSnapshot MakeSnapshot() const;

	/** The wrap (T14): rolls the ledger and loot, equips the hero's item, levels up, fills the panel. */
	void RollWrap(bool bLastScene);
	/** XP and milestones go to the hero's PlayerState. */
	class AJamPlayerState* GetHeroState() const;
	/** Pushes the progression's stats onto the hero (move speed). */
	void ApplyHeroStats();

	/** "Back to one": the hero returns to the set's mark for the next take. */
	void ResetHeroToMark();
	/** Why the take just failed, for the RETAKE! card. */
	FString FailReason(bool bForcedByFlag) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AJamNuisance>> Nuisances;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AJamPickup>> Pickups;

	TWeakObjectPtr<AJamHero> Hero;
	float GroundZ = 0.f;

	/** Scene (and backlot set) index being shot; FieldBounds is that set's floor. */
	int32 SceneIndex = 0;
	int32 FirstSceneIndex = 0;
	FBox2D FieldBounds = FBox2D(ForceInit);
	/** After a wrap: the next scene starts when the hero steps onto its set. */
	bool bBetweenScenes = false;
	bool bRunOver = false;

	/** The shoot state last seen, and when it was entered (drives the automatic slate/next/retake). */
	EJamShootState SeenState = EJamShootState::Idle;
	float StateEnteredAt = 0.f;
	/** Crops eaten so far, in fractions (meter stages). */
	float CropsEaten = 0.f;

	// Run totals, for the PASS line.
	int32 ScenesWrapped = 0;
	int32 TakesTotal = 0;
	int32 NotesTaken = 0;
	int32 NotesRefused = 0;
	int32 FleeCountTotal = 0;

	bool bAutoplay = false;
	bool bAutoNoteTake = true;
	/** -forcefail[=seconds]: the first take of the run fails after this long rolling (T13 check). Negative = off. */
	float ForceFailAfter = -1.f;
	bool bForceFailDone = false;
	bool bFailForcedByFlag = false;
	float AutoplayTime = 0.f;
	float SceneTime = 0.f;

	FJamNagScheduler Nag;
	TArray<FName> CaptionQueue;
	float NagPollIn = 1.f;
	float SinceProgress = 0.f;
	float StrayedFor = 0.f;
	int32 LinesSpoken = 0;
	bool bSaidLevel7Intro = false;

	UPROPERTY(Transient)
	TObjectPtr<AJamFaerie> Faerie;
};
