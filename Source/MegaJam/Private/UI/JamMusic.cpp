#include "UI/JamMusic.h"

FName JamMusic::CueFor(const FJamMusicContext& C)
{
	switch (C.Phase)
	{
	case EJamFilmPhase::Title:    return TEXT("quest_accepted");        // a solemn undertaking, for a film about rats
	case EJamFilmPhase::Premiere: return TEXT("emotional_revelation");
	case EJamFilmPhase::Credits:  return TEXT("theme_song");            // R2: the song plays here only, not in play
	default: break;
	}
	if (C.bBudgetOut) { return NAME_None; }   // she hums it instead (T21)
	if (C.bWalking) { return TEXT("open_world_exploration"); }
	if (C.bNotePrompt) { return TEXT("mystery"); }                     // she is thinking. Uh oh.
	if (C.Mood == 2) { return TEXT("emotional_revelation"); }          // "same scene, but sadder"
	if (C.Mood == 1) { return C.Scene >= 3 ? TEXT("final_boss") : TEXT("boss_battle"); }
	switch (C.Scene)
	{
	case 1:  return TEXT("feeding_candy");                             // rats, celery, bonding
	case 2:  return TEXT("hard_combat");                               // goblins, teeth
	default: return C.Stage >= 2 ? TEXT("final_boss") : TEXT("boss_battle");   // the finale, after its CUT!
	}
}

const TArray<FName>& JamMusic::AllCues()
{
	static const TArray<FName> Cues = { TEXT("quest_accepted"), TEXT("feeding_candy"), TEXT("hard_combat"), TEXT("boss_battle"),
		TEXT("final_boss"), TEXT("open_world_exploration"), TEXT("mystery"), TEXT("emotional_revelation"), TEXT("credits"), TEXT("theme_song") };
	return Cues;
}

int32 JamMusic::Takes(FName Cue)
{
	return Cue == FName(TEXT("theme_song")) ? 1 : 2;
}

FName JamMusic::Fallback(FName Cue)
{
	return Cue == FName(TEXT("theme_song")) ? FName(TEXT("credits")) : NAME_None;
}

FString JamMusic::AssetPath(FName Cue, int32 Take)
{
	const FString Name = FString::Printf(TEXT("M_%s_take%d"), *Cue.ToString(), FMath::Clamp(Take, 1, Takes(Cue)));
	return FString::Printf(TEXT("/Game/Audio/Music/%s.%s"), *Name, *Name);
}
