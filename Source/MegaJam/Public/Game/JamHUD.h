#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/JamHudView.h"
#include "JamHUD.generated.h"

class UJamHudWidget;

/**
 * Owns the HUD widget and feeds it an FJamHudView each frame, built from the game state (read only).
 * `-uiscreen=<sample>` shows one of JamHud::SampleNames() instead, for screenshots of each layer and card.
 */
UCLASS()
class MEGAJAM_API AJamHUD : public AHUD
{
	GENERATED_BODY()

public:
	AJamHUD();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** The view built from the world this frame (or the sample). */
	const FJamHudView& GetView() const { return View; }

private:
	void BuildView();

	UPROPERTY(Transient)
	TObjectPtr<UJamHudWidget> Widget;

	FJamHudView View;
	FString SampleName;
};
