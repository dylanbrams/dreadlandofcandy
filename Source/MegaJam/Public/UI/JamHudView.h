#pragma once

#include "CoreMinimal.h"
#include "UI/JamWrapView.h"

/** What kind of centre card is up; picks its colours. */
enum class EJamCardKind : uint8
{
	None,
	Slate,    // SCENE n - TAKE n
	Action,   // ACTION!
	Cut,      // CUT! (the note prompt)
	Retake,   // RETAKE! / "Perfect! Again."
	Wrap,     // THAT'S A WRAP!
	Info,     // NOTE TAKEN / REFUSED and other plain cards
};

struct MEGAJAM_API FJamCardStyle
{
	FLinearColor Accent;   // the bar above the title and the title text
	FLinearColor Panel;    // card background
};

/** One slot of the skill bar. */
struct MEGAJAM_API FJamHudSkill
{
	FString Name;
	FString Key;
	int32 Rank = 0;
	int32 MaxRank = 1;
	bool bUnlocked = false;
	float Cooldown = 0.f;      // 1 = just cast, 0 = ready
	bool bSpendable = false;   // a point can go here now (the wrap panel is up)
	FString IconPath;
	FLinearColor ClassColor = FLinearColor(1.f, 0.82f, 0.3f);
	FLinearColor DamageColor = FLinearColor(0.75f, 0.75f, 0.75f);
	float CooldownSeconds = 0.f; // appended to preserve existing sample aggregate initializers
};

/** One speech bubble on screen: text over a screen position (already projected, 1920x1080 design units). */
struct MEGAJAM_API FJamHudBubble
{
	FString Text;
	FVector2D Position = FVector2D::ZeroVector;
	bool bJagged = false;   // lecture bubbles; the faerie's are round
};

/**
 * Everything the HUD widget draws, as plain data. Built from the game state each frame, or from a named sample
 * for `-uiscreen`, so layout can be checked without playing to that moment. The widget holds no game values.
 */
struct MEGAJAM_API FJamHudView
{
	bool bShootVisible = false;   // slate label, REC, objective
	bool bTravelVisible = false;  // skills and progression between Open film sets

	FString SceneLabel;           // "SCENE 1: THE RAT CATCHER"
	int32 Take = 1;
	bool bRolling = false;        // REC blinks
	float FilmSeconds = 0.f;      // the timecode

	FString ObjectiveId;          // scene/take/stage identity, independent of progress
	FString Direction;            // objective without its changing counter
	FString Objective;            // "Pacify the rats   2/3"
	bool bMeterVisible = false;
	float MeterFraction = 0.f;    // 1 = full (time left, or crops still standing)
	FString MeterLabel;           // "41s" or "crops eaten 3/8"
	bool bMeterDanger = false;

	/** R4b: the hero's Dignity, under the slate. Hit = 1 just after a hit, fading to 0 (the bar flashes). */
	bool bDignityVisible = false;
	float DignityFraction = 1.f;
	float DignityHit = 0.f;

	FString Hint;
	/** A4: the hint carries a Rush clock under 5 s; it pulses red. */
	bool bHintDanger = false;
	/** A5: the hero was just hit, a pink screen-edge vignette, 1 fading to 0. */
	float HitVignette = 0.f;

	EJamCardKind CardKind = EJamCardKind::None;
	FString CardTitle;
	FString CardSubtitle;

	bool bPromptVisible = false;
	FString Prompt;               // "Hold [E]  Feed celery"
	FVector2D PromptPosition = FVector2D::ZeroVector;
	float HoldFraction = 0.f;     // interact held, 0 to 1 (fills the prompt and the take tile)

	/** CUT!: Fenella's note, as two choices. Replaces the card while up. */
	bool bNotePrompt = false;
	FString NotePitch;
	float NoteGross = 1.f;        // what taking it does to the box office
	bool bGamepad = false;        // key glyphs
	/** T12b: she is reading the player's own note (the tiles give way to it). */
	bool bReadingNote = false;
	FString ReadingNote;

	/** Run counters on the slate panel, and her mood. */
	int32 NotesTaken = 0;
	int32 NotesRefused = 0;
	bool bSulking = false;

	/** Skill bar and XP bar (BUILD_SPEC 3c). */
	TArray<FJamHudSkill> Skills;
	int32 Level = 0;
	float Xp = 0.f;
	int32 XpEarned = 0;
	int32 XpNeeded = 0;
	/** Fixes3 #26: encore levels past the top level; >0 shows "ENCORE n" and encore EXP. */
	int32 EncoreLevel = 0;
	bool bEncoreXp = false;
	int32 Points = 0;

	/** The wrap panel; replaces the card while up. */
	FJamWrapView Wrap;

	/** A take just failed: a red film-burn over the whole screen, 1 fading to 0. */
	float FailFlash = 0.f;

	TArray<FJamHudBubble> Bubbles;

	/** Fenella's caption, bottom centre (never over the objective). Empty = none. */
	FString Caption;
};

namespace JamHud
{
	/** Film timecode at 24 fps, HH:MM:SS:FF. Negative clamps to zero. */
	MEGAJAM_API FString Timecode(float Seconds);

	MEGAJAM_API FJamCardStyle CardStyle(EJamCardKind Kind);

	/** Fills OutView with a sample for `-uiscreen=<Name>`; false if the name is unknown. Names: SampleNames(). */
	MEGAJAM_API bool Sample(const FString& Name, FJamHudView& OutView);
	MEGAJAM_API TArray<FString> SampleNames();
}
