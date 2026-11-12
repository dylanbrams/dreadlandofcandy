#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Film/JamShoot.h"
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
	/** Film running time: counts while a take rolls, across the whole film (the HUD timecode). */
	float FilmSeconds = 0.f;
	/** Notes answered over the whole run (the HUD counter; the ending is picked from these). */
	int32 RunNotesTaken = 0;
	EJamFilmPhase Phase = EJamFilmPhase::Title;
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
	/** Waiting for the hero's answer to CUT!. */
	bool IsNotePrompt() const { return Shoot.GetState() == EJamShootState::NotePrompt; }

private:
	friend class AJamGameMode;   // the only writer
	FJamShoot Shoot;
};
