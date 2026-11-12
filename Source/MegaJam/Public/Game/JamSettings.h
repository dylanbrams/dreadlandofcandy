#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JamSettings.generated.h"

/**
 * Player settings from the Notes menu: Fenella's volume (the master volume, T18 reads it) and the Director's
 * Commentary toggle. Saved to GameUserSettings.ini. Not game state: nothing in the rules reads it.
 */
UCLASS(config = GameUserSettings)
class MEGAJAM_API UJamSettings : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(config)
	float Volume = 0.8f;

	UPROPERTY(config)
	bool bCommentary = false;

	void SetVolume(float InVolume) { Volume = FMath::Clamp(FMath::RoundToFloat(InVolume * 10.f) / 10.f, 0.f, 1.f); SaveConfig(); }
	void SetCommentary(bool bOn) { bCommentary = bOn; SaveConfig(); }
};
