#pragma once

#include "CoreMinimal.h"

/**
 * R6, the cast: who plays the hero. All four use the knight's skills; only the knight has the full set of clips
 * (the others have Idle and Walk). Each shouts one of its English battle cries on a skill cast.
 * Plain data, no engine objects: the hero loads /Game/Art/Cast/<Rig>/SK_<Rig>_Rig and /Game/Audio/VO/Cast/VO_Cry_<Id>.
 */
struct MEGAJAM_API FJamClassDef
{
	FString Id;              // "knight": the -class= value and the saved setting
	FString Name;            // "The Knight": the title screen's Casting line
	FString Rig;             // "Knight": /Game/Art/Cast/Knight/SK_Knight_Rig
	FString Blurb;           // one line under the title while this one is cast
	TArray<FString> Cries;   // "K01": /Game/Audio/VO/Cast/VO_Cry_K01
	// Enum-indexed presentation data: 0 basic, 1 signature, 2 unused, 3 ultimate.
	TArray<FString> SkillNames;
	TArray<FString> SkillIcons;
    TArray<FString> SkillAnimations;
    FName WeaponHand = TEXT("hand_r");
    FRotator WeaponGrip = FRotator(0.f, 0.f, -90.f);
    FVector BackOffset = FVector(-34.f, 22.f, 58.f);
    FVector BackAxis = FVector(0.f, -0.65f, -0.76f);
    float BackTwist = 0.f;
	FLinearColor PrimaryColor = FLinearColor(1.f, 0.82f, 0.3f);
	TArray<FLinearColor> SkillDamageColors;
	bool bFullClips = false; // SwordRaise, Sigh, Scrub, ProudStrut exist
};

namespace JamCast
{
	MEGAJAM_API const TArray<FJamClassDef>& All();
	/** Index of the class with this id (case-insensitive), or INDEX_NONE. */
	MEGAJAM_API int32 IndexOf(const FString& Id);
	/** The class at Index, clamped into range (0 = the knight). */
	MEGAJAM_API const FJamClassDef& Get(int32 Index);
	/** Left / right on the Casting line: wraps around. */
	MEGAJAM_API int32 Step(int32 Index, int32 Delta);
	/** The cry for the Nth cast: goes round the list in order, so the same line never plays twice running. */
	MEGAJAM_API FString CryFor(const FJamClassDef& Def, int32 CastCount);
	/** Seconds between two cries, so a skill flurry is one shout, not four. */
	constexpr float CryMinInterval = 3.5f;
}
