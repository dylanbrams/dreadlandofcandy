#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/JamHudView.h"
#include "UI/JamScreens.h"
#include "JamHUD.generated.h"

class UJamHudWidget;
class UJamScreenWidget;
enum class EJamMenuInput : uint8;

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

	/** Esc / Start in play: pause and open the Pause menu. */
	void RequestPause();

	/** The view built from the world this frame (or the sample). */
	const FJamHudView& GetView() const { return View; }

private:
	void BuildView();
	/** The music (T18): one cue per moment, a random take, crossfaded, at the "Fenella" volume. */
	void TickMusic();
	UPROPERTY(Transient)
	TObjectPtr<class UAudioComponent> Music;
	FName MusicCue;
	TSet<FName> MissingCues;
	bool bNoMusic = false;
	// Screens (T16): which one is up, its view, and what its inputs do.
	FJamScreenView BuildScreen() const;
	void HandleMenu(EJamMenuInput Input, int32 Index);
	void Activate(const FJamMenuOption& Option);
	void Adjust(const FJamMenuOption& Option, int32 Delta);
	void SetPaused(bool bPause);
	void UpdateInputMode(bool bInteractive);
	FJamMenuSettings MenuSettings() const;
	class UJamSettings* Settings() const;
	class AJamGameMode* AuthGameMode() const;

	UPROPERTY(Transient)
	TObjectPtr<UJamScreenWidget> Screen;
	FJamScreenView ScreenView;
	EJamScreen SubScreen = EJamScreen::None;   // Options or QuitConfirm over the title or the pause menu
	int32 Selected = 0;
	int32 ParentSelected = 0;                  // the line to return to when a sub-screen closes
	bool bPaused = false;
	bool bMenuGamepad = false;
	int32 InputModeState = -1;                 // 0 game, 1 UI
	FString ScreenSampleName;

	UPROPERTY(Transient)
	TObjectPtr<UJamHudWidget> Widget;

	FJamHudView View;
	FString SampleName;
};
