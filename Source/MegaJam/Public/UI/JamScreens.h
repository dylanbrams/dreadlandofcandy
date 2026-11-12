#pragma once

#include "CoreMinimal.h"
#include "Film/JamProgression.h"

/** Where the film is: BUILD_SPEC 3 flow, Title -> the three scenes -> Premiere -> Credits -> Title. */
enum class EJamFilmPhase : uint8 { Title, Shooting, Premiere, Credits };

/** The full-screen menus and cards around the film (BUILD_SPEC 3 flow, T16). */
enum class EJamScreen : uint8
{
	None,
	Rating,       // two seconds before the title
	Title,
	Options,      // "Notes"
	QuitConfirm,
	Pause,
	Character,    // the character sheet and the six passives already spent
	Premiere,
	Credits,
};

/** What choosing a menu line does. */
enum class EJamMenuAction : uint8
{
	None,
	RollCamera,       // start the film
	OpenOptions,
	WalkOffSet,       // opens the quit confirm
	WalkAnyway,       // quit (from the title) or back to the title (from pause)
	Fine,             // close the confirm
	Resume,
	MusicVolume,      // left / right adjusts
	VoiceVolume,      // "Fenella": left / right adjusts
	Commentary,       // toggles
	Back,
	Locked,           // "Budget: Low (locked)", never selectable
	CharacterSheet,
};

struct MEGAJAM_API FJamMenuOption
{
	EJamMenuAction Action = EJamMenuAction::None;
	FString Label;
	FString Value;       // "80%", "On"
	bool bEnabled = true;
};

/** Plain data for one screen; the widget draws it and nothing else. */
struct MEGAJAM_API FJamScreenView
{
	EJamScreen Screen = EJamScreen::None;
	FString Kicker;                   // small line above the title
	FString Title;
	FString Stamp;                    // rubber stamp across the title
	FString Subtitle;
	TArray<TPair<FString, FString>> Rows;   // label, value (Premiere totals)
	TArray<FString> Quotes;           // reviews
	TArray<FString> Body;             // credits lines
	TArray<FJamMenuOption> Options;
	int32 Selected = 0;
	FString Caption;                  // Fenella, under everything
	FString Footer;                   // key hint
	float Scroll = 0.f;               // credits, 0 to 1
	bool bOpaque = true;              // false: the game shows through (pause)
};

/** Totals for the Premiere card. */
struct MEGAJAM_API FJamPremiereTotals
{
	bool bDirectorsCut = true;
	int32 FilmGross = 0;
	int32 Director = 0;
	int32 Hero = 0;
	/** T16a: the film the player's typed notes made (from the note server). Empty title = the authored ending. */
	FString Title;
	FString Tagline;
	TArray<FString> Reviews;   // quote, then " - by"
};

/** Settings a menu shows. */
struct MEGAJAM_API FJamMenuSettings
{
	float MusicVolume = 0.8f;
	float VoiceVolume = 0.8f;
	bool bCommentary = false;
	bool bGamepad = false;
};

namespace JamScreens
{
	MEGAJAM_API FJamScreenView Rating();
	MEGAJAM_API FJamScreenView Title(const FJamMenuSettings& S);
	MEGAJAM_API FJamScreenView Options(const FJamMenuSettings& S);
	MEGAJAM_API FJamScreenView QuitConfirm(const FJamMenuSettings& S, bool bFromPause);
	MEGAJAM_API FJamScreenView Pause(const FJamMenuSettings& S);
	/** Stats, level, points, and the greyed passives panel (BUILD_SPEC 3c). */
	MEGAJAM_API FJamScreenView Character(const FJamProgression& Prog, const FJamMenuSettings& S);
	MEGAJAM_API FJamScreenView Premiere(const FJamPremiereTotals& T, const FJamMenuSettings& S);
	MEGAJAM_API FJamScreenView Credits(bool bDirectorsCut, const FJamMenuSettings& S);

	/**
	 * T16a: reads a /v1/director/premiere body (NoteForm.md) into Out's Title, Tagline and Reviews.
	 * False (Out untouched) unless v is 1, source is "model", and the title, tagline and exactly three reviews
	 * survive cleaning (control characters dropped, whitespace collapsed, cut to 48 / 80 / 70 + 30 characters).
	 */
	MEGAJAM_API bool ReadPremiere(const FString& Json, FJamPremiereTotals& Out);

	/** BUILD_SPEC 2: mostly taken = the Director's Cut (ties go to her). */
	inline bool IsDirectorsCut(int32 NotesTaken, int32 NotesRefused) { return NotesTaken >= NotesRefused; }

	/** Moves the selection, skipping disabled lines; wraps around. */
	MEGAJAM_API int32 Step(const TArray<FJamMenuOption>& Options, int32 Selected, int32 Delta);

	/** The credits text, joke credits then the real ones. In the Theatrical ending she asks to be uncredited. */
	MEGAJAM_API TArray<FString> CreditLines(bool bDirectorsCut);

	/** Sample screens for -uiscreen: rating, title, options, quit, pause, premiere, theatrical, credits. */
	MEGAJAM_API bool Sample(const FString& Name, FJamScreenView& Out);
}
