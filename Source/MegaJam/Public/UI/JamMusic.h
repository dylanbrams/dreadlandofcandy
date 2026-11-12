#pragma once

#include "CoreMinimal.h"
#include "UI/JamScreens.h"

/** Where the film is, as far as the music cares. */
struct MEGAJAM_API FJamMusicContext
{
	EJamFilmPhase Phase = EJamFilmPhase::Title;
	int32 Scene = 1;          // 1-based
	int32 Stage = 0;
	bool bWalking = false;    // between scenes, on the street
	bool bNotePrompt = false; // CUT!
	bool bBudgetOut = false;  // the finale ran out of money: no music at all
};

/**
 * The music plan (T18). Dylan's tracks come in two takes each and either is fine, so each cue picks a take at
 * random when it starts. Assets are /Game/Audio/Music/M_<cue>_take<1|2>, loaded by path (always cooked);
 * a missing one means silence, not a failure.
 */
namespace JamMusic
{
	/** The cue for a moment; NAME_None = silence. */
	MEGAJAM_API FName CueFor(const FJamMusicContext& Context);
	/** Every cue the game uses (the import list). */
	MEGAJAM_API const TArray<FName>& AllCues();
	/** "/Game/Audio/Music/M_hard_combat_take2.M_hard_combat_take2". */
	MEGAJAM_API FString AssetPath(FName Cue, int32 Take);
}
