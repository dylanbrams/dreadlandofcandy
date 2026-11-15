#pragma once

#include "CoreMinimal.h"
#include "Game/JamGameMode.h"
#include "JamTourGameMode.generated.h"

/**
 * Stock footage for trailers: the real open world (village, villagers, terrain, look and ambience) with no HUD,
 * no hero and no menus, and a camera that slowly flies a loop through all of it (JamTour). Nothing to play.
 * Choose it with the game option, not a flag:
 *   <Map>?game=/Script/MegaJam.JamTourGameMode        (Scripts\stock-footage.ps1 does this)
 * Flags: -tourspeed=<uu/s, default 650>  -tourheight=<uu above ground, default 380>  -tourstart=<0..1 of the loop>
 *        -tourtime=<seconds, then quit; default runs until Esc>
 * The music is the HUD's, so the capture has none; lay your own track over it.
 */
UCLASS()
class MEGAJAM_API AJamTourGameMode : public AJamGameMode
{
	GENERATED_BODY()

public:
	AJamTourGameMode();
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	float QuitAfter = -1.f;
	float Elapsed = 0.f;
};
