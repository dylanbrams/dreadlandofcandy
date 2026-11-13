#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Film/JamCast.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "JamSettings.generated.h"

/**
 * Player settings from the Notes menu: music volume, Fenella's (voice) volume, the Director's Commentary toggle.
 * Saved to GameUserSettings.ini. Not game state: nothing in the rules reads it.
 * Dylan, Nov 12: music and Fenella get separate sliders. OneMore has a fuller settings system; parked.
 */
UCLASS(config = GameUserSettings)
class MEGAJAM_API UJamSettings : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(config)
	float MusicVolume = 0.8f;

	/** Fenella's voice-over (T23 reads it; captions show regardless). */
	UPROPERTY(config)
	float VoiceVolume = 0.8f;

	/** Sound effects (R1, JamSfx). */
	UPROPERTY(config)
	float EffectsVolume = 0.8f;

	UPROPERTY(config)
	bool bCommentary = false;

	/** R6: who plays the hero (JamCast id). -class=<id> on the command line wins until the title's Casting line is used. */
	UPROPERTY(config)
	FString CastId = TEXT("knight");
	bool bCastChosen = false;

	/** Index into JamCast::All(): the title's choice, else -class=, else the saved one, else the knight. */
	int32 CastIndex() const;
	void SetCastIndex(int32 Index);

	static float Step(float V) { return FMath::Clamp(FMath::RoundToFloat(V * 10.f) / 10.f, 0.f, 1.f); }
	void SetMusicVolume(float V) { MusicVolume = Step(V); SaveConfig(); }
	void SetVoiceVolume(float V) { VoiceVolume = Step(V); SaveConfig(); }
	void SetEffectsVolume(float V) { EffectsVolume = Step(V); SaveConfig(); }
	void SetCommentary(bool bOn) { bCommentary = bOn; SaveConfig(); }
};

inline int32 UJamSettings::CastIndex() const
{
	FString Flag;
	if (!bCastChosen && FParse::Value(FCommandLine::Get(), TEXT("-class="), Flag) && JamCast::IndexOf(Flag) != INDEX_NONE) { return JamCast::IndexOf(Flag); }
	return FMath::Max(JamCast::IndexOf(CastId), 0);
}

inline void UJamSettings::SetCastIndex(int32 Index)
{
	CastId = JamCast::Get(Index).Id;
	bCastChosen = true;
	SaveConfig();
}
