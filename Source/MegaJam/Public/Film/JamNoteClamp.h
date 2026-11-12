#pragma once

#include "CoreMinimal.h"
#include "Film/JamFilmTypes.h"

/**
 * The safety layer for typed director's notes (BUILD_SPEC 3a, Docs/Design/NoteForm.md, version 1).
 * Turns any response from the note service into a valid note or rejects it, and, when the service is
 * unreachable or rejected, picks the closest authored-style note from keywords in what the player typed.
 * A language model never decides rules: every knob is clamped, every string truncated, unknown fields dropped,
 * the stage must be finishable with the Lecture alone, and the gross is computed here from the knobs.
 * Headless: no world, no JSON module (a small private reader lives in Private/Film/JamMiniJson.*).
 */

enum class EJamTint : uint8 { None, Pink, Mint, Gold, Grey, Purple };
enum class EJamWeather : uint8 { None, Rain, SprinkleSnow, Fog };
enum class EJamMood : uint8 { Cheerful, Epic, Sad };

/** What the world layer needs to know about the stage a note would replace. */
struct MEGAJAM_API FJamNoteContext
{
	FName SceneId;
	/** The authored stage the note replaces. Its verb stays; its numbers and target may change. */
	FJamStageDef BaseStage;
	/** Kinds the set can spawn, and how many of each. */
	TArray<TPair<EJamTargetKind, int32>> Kinds;
	/** The Lecture's damage per hit and cooldown, used to prove the stage is finishable. */
	float LectureDamage = 60.f;
	float LectureCooldown = 1.5f;

	int32 CapacityOf(EJamTargetKind Kind) const;
	/** The "set_capacity" sent to the server: the largest of the kinds. */
	int32 MaxCapacity() const;
};

/** Everything cosmetic or textual that a note carries beyond the replacement stage. */
struct MEGAJAM_API FJamNoteCosmetics
{
	float Scale = 1.f;
	EJamTint Tint = EJamTint::None;
	EJamWeather Weather = EJamWeather::None;
	EJamMood Mood = EJamMood::Cheerful;
	FString Motif;
	FString SceneTitle;
	FString FenellaReaction;
	TArray<FString> LectureBubbles;
	TArray<FString> Replies;
	FString LootName;
	FString LootFlavour;
};

struct MEGAJAM_API FJamClampedNote
{
	/** Replacement stage and gross multiplier; Id is "TypedNote". */
	FJamNoteDef Def;
	FJamNoteCosmetics Cosmetics;
	/** "cache", "model" or "keywords". */
	FString Source;
};

namespace JamNoteClamp
{
	constexpr int32 NoteMaxChars = 80;

	/**
	 * Parses and clamps a service response (the whole body, NoteForm.md "Response").
	 * False means: use the fallback. OutReason says why, for the log.
	 */
	MEGAJAM_API bool FromResponse(const FString& Json, const FJamNoteContext& Ctx, FJamClampedNote& Out, FString& OutReason);

	/** Trims, drops control characters, collapses whitespace and cuts to 80 characters: what the client sends. */
	MEGAJAM_API FString SanitizeNote(const FString& Typed);

	/** Resolve of a kind before multipliers (BUILD_SPEC 10). */
	MEGAJAM_API float BaseResolve(EJamTargetKind Kind);

	/** True if the Lecture alone can finish the stage inside its time limit, on a generous model of play. */
	MEGAJAM_API bool IsFinishable(const FJamStageDef& Stage, const FJamNoteContext& Ctx);

	/** Gross multiplier from the knobs, 1.1 to 2.0. The model never sets it. */
	MEGAJAM_API float GrossFromKnobs(const FJamStageDef& Stage, float Scale, const FJamStageDef& Base);

	/**
	 * The keyword fallback. Looks for words like bigger, more, faster, sad, rain in the typed note.
	 * Returns false if it understands nothing (Fenella then "can't read the handwriting" and the game uses its authored note).
	 * Otherwise Out is valid and finishable, source "keywords", with authored reaction text. Deterministic for the seed.
	 */
	MEGAJAM_API bool FromKeywords(const FString& Typed, const FJamNoteContext& Ctx, int32 Seed, FJamClampedNote& Out);
}
