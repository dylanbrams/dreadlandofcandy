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
	uint8 Mood = 0;           // a typed note's mood (T12b): 0 the scene's own, 1 epic, 2 sad
	FName WalkCue;            // Open mode (R4): the biome's cue while walking between sets; None = the street's
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
	/** How many takes a cue has: 2 for the score, 1 for the theme song. */
	MEGAJAM_API int32 Takes(FName Cue);
	/** What plays when a cue's asset is missing (the theme song falls back to the old credits cue); NAME_None = silence. */
	MEGAJAM_API FName Fallback(FName Cue);
	/** "/Game/Audio/Music/M_hard_combat_take2.M_hard_combat_take2". */
	MEGAJAM_API FString AssetPath(FName Cue, int32 Take);
}
