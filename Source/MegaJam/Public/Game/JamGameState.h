#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Film/JamShoot.h"
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
	FString CardTitle;
	FString CardSubtitle;
	/** World time the card disappears; negative = stays. */
	float CardUntil = 0.f;

	bool IsCardVisible() const;

private:
	friend class AJamGameMode;   // the only writer
	FJamShoot Shoot;
};
