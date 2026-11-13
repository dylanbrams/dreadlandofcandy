#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Film/JamShoot.h"
#include "Film/JamDignity.h"
#include "UI/JamHudView.h"
#include "UI/JamWrapView.h"
#include "UI/JamScreens.h"
#include "JamGameState.generated.h"

/**
 * Shared, readable game state: the shoot (scene, take, stage, progress, notes) and the card on screen.
 * Only the game mode writes it; the HUD and other cosmetics read it.
 */
UCLASS()
class MEGAJAM_API AJamGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	const FJamShoot& GetShoot() const { return Shoot; }
	/** R4b: the hero's Dignity this take (the HUD's meter). */
	const FJamDignity& GetDignity() const { return Dignity; }
	/** Show the Dignity meter: a stage with attackers is rolling. */
	bool bDignityVisible = false;

	/** True while an Interact stage is rolling: the only time treating counts. */
	bool CanInteract() const
	{
		const FJamStageDef* Stage = Shoot.GetStage();
		return Shoot.GetState() == EJamShootState::Rolling && Stage && Stage->Verb == EJamStageVerb::Interact;
	}

	/** The big centre card (ACTION!, RETAKE!, ...). Empty title = no card. */
	EJamCardKind CardKind = EJamCardKind::None;
	FString CardTitle;
	FString CardSubtitle;
	/** World time the card disappears; negative = stays. */
	float CardUntil = 0.f;

	bool IsCardVisible() const;

	/** Fenella's caption: the line on screen (a JamLines id, so a client could look the text up). None = quiet. */
	FName CaptionLineId;
	FString CaptionText;
	float CaptionUntil = 0.f;
	bool IsCaptionVisible() const;

	/** 1-based scene being shot (or just wrapped). */
	int32 SceneNumber = 1;
	/** One line under the objective between scenes ("Walk to Set 2"). Empty = none. */
	FString Hint;
	/** Open mode (R4): the compass points at the next film set while the hero walks there. */
	bool bCompass = false;
	FVector CompassTarget = FVector::ZeroVector;
	FString CompassLabel;
	/** Open mode: the music cue of the biome the hero walks through (NAME_None = the street's). */
	FName WalkMusicCue;
	/** Film running time: counts while a take rolls, across the whole film (the HUD timecode). */
	float FilmSeconds = 0.f;
	/** Notes answered over the whole run (the HUD counter; the ending is picked from these). */
	int32 RunNotesTaken = 0;
	EJamFilmPhase Phase = EJamFilmPhase::Title;
	/** The finale ran out of money (T21): grey checker, boxes, [SHOUTING], no music. */
	bool bBudgetOut = false;
	/** World time the phase began (the rating card, the credits scroll). */
	float PhaseSince = 0.f;
	FJamPremiereTotals Premiere;

	/** A speech bubble in the world (T19): a lecture line over the hero, a reply over a nuisance. */
	struct FWorldBubble
	{
		FName LineId;
		FString Text;
		TWeakObjectPtr<AActor> Anchor;
		FVector Offset = FVector(0.f, 0.f, 170.f);
		float Until = 0.f;
		bool bJagged = true;
	};
	TArray<FWorldBubble> Bubbles;

	/** The wrap panel (ledger, loot, character sheet). WrapUntil negative = stays until replaced. */
	FJamWrapView Wrap;
	float WrapUntil = 0.f;
	bool IsWrapVisible() const;

	/** World time the last take failed (the HUD's flash); negative = never. */
	float TakeFailedAt = -100.f;
	int32 RunNotesRefused = 0;
	/** T12b: the player's own note. While she reads it the CUT! panel says so; then its cosmetics hold for the scene. */
	bool bReadingNote = false;
	FString ReadingNote;
	/** The three suggestion buttons at this CUT! (pre-warmed on the server; understood offline too). */
	TArray<FString> SuggestedNotes;
	/** Set by a typed note for the rest of its scene. 0 none, 1 rain, 2 sprinkle snow, 3 fog (EJamWeather). */
	uint8 Weather = 0;
	/** 0 cheerful (the scene's own music), 1 epic, 2 sad (EJamMood). */
	uint8 Mood = 0;
	/** The note renamed the scene (empty = its own title). */
	FString SceneTitleOverride;

	/** Waiting for the hero's answer to CUT!. */
	bool IsNotePrompt() const { return Shoot.GetState() == EJamShootState::NotePrompt; }

private:
	friend class AJamGameMode;   // the only writer
	FJamShoot Shoot;
	FJamDignity Dignity;
};
