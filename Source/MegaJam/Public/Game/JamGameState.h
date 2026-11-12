#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Film/JamShoot.h"
#include "UI/JamHudView.h"
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

	/** 1-based scene being shot (or just wrapped). */
	int32 SceneNumber = 1;
	/** One line under the objective between scenes ("Walk to Set 2"). Empty = none. */
	FString Hint;
	/** Film running time: counts while a take rolls, across the whole film (the HUD timecode). */
	float FilmSeconds = 0.f;
	/** Waiting for the hero's answer to CUT!. */
	bool IsNotePrompt() const { return Shoot.GetState() == EJamShootState::NotePrompt; }

private:
	friend class AJamGameMode;   // the only writer
	FJamShoot Shoot;
};
