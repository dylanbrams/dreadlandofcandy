#pragma once

#include "CoreMinimal.h"

/** One sound-effect event (R1): what happened, which clip, how loud. Dylan: "ridiculously over the top". */
struct MEGAJAM_API FJamSfxCue
{
	FName Event;
	FName Clip;              // /Game/Audio/SFX/SFX_<Clip>
	float Volume = 1.f;      // times the Effects slider
	float PitchJitter = 0.f; // +- this, so repeats are not identical
	float MinInterval = 0.f; // seconds; a burst of the same event plays once
};

/**
 * The sound-effect table and its player (R1). Clips are loaded by path from /Game/Audio/SFX (always cooked); a
 * missing clip is logged once and is silence, not a failure. Everything plays 2D at the Effects volume
 * (UJamSettings). The game mode and the HUD call Play() at the moments in the RELEASE_PLAN R1 event list.
 */
namespace JamSfx
{
	MEGAJAM_API const TArray<FJamSfxCue>& Table();
	MEGAJAM_API const FJamSfxCue* Find(FName Event);
	/** "/Game/Audio/SFX/SFX_Clapper.SFX_Clapper". */
	MEGAJAM_API FString AssetPath(FName Clip);

	/** Plays the event's clip now, or after Delay seconds (game time). */
	MEGAJAM_API void Play(const UObject* WorldContext, FName Event, float Delay = 0.f);
	/** A looping clip on a component the caller keeps (ambience). Null if the clip is missing. */
	MEGAJAM_API class UAudioComponent* StartLoop(const UObject* WorldContext, FName Event);
	/** The Effects slider (0..1), or 0.8 without settings. */
	MEGAJAM_API float EffectsVolume(const UObject* WorldContext);
}
