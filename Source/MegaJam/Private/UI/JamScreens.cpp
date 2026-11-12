#include "UI/JamScreens.h"
#include "Film/JamLines.h"

namespace
{
	const TCHAR* Confirm(const FJamMenuSettings& S) { return S.bGamepad ? TEXT("(A)") : TEXT("[E] / Enter"); }
	const TCHAR* BackKey(const FJamMenuSettings& S) { return S.bGamepad ? TEXT("(B)") : TEXT("Esc"); }

	FString MenuFooter(const FJamMenuSettings& S)
	{
		return FString::Printf(TEXT("%s choose     %s select     %s back"),
			S.bGamepad ? TEXT("D-pad") : TEXT("W / S"), Confirm(S), BackKey(S));
	}

	FJamMenuOption Opt(EJamMenuAction Action, const TCHAR* Label, const FString& Value = FString(), bool bEnabled = true)
	{
		FJamMenuOption O;
		O.Action = Action;
		O.Label = Label;
		O.Value = Value;
		O.bEnabled = bEnabled;
		return O;
	}

	FString FirstLine(EJamLineTrigger Trigger, int32 Index)
	{
		const TArray<const FJamLine*> Lines = JamLines::ForTrigger(Trigger);
		return Lines.IsValidIndex(Index) ? Lines[Index]->Text : FString();
	}

	FString Pct(float V) { return FString::Printf(TEXT("%d%%"), FMath::RoundToInt(FMath::Clamp(V, 0.f, 1.f) * 100.f)); }
}

FJamScreenView JamScreens::Rating()
{
	FJamScreenView V;
	V.Screen = EJamScreen::Rating;
	V.Title = TEXT("S");
	V.Kicker = TEXT("THIS FILM IS RATED");
	V.Subtitle = TEXT("for Sugar. Contains mild peril, firm guidance, and one napkin.");
	return V;
}

FJamScreenView JamScreens::Title(const FJamMenuSettings& S)
{
	FJamScreenView V;
	V.Screen = EJamScreen::Title;
	V.Title = TEXT("THE DREAD LAND OF CANDY");
	V.Stamp = TEXT("DIRECTOR'S CUT");
	V.Subtitle = TEXT("The Definitive Edition of a Game That Never Existed.");
	V.Options = { Opt(EJamMenuAction::RollCamera, TEXT("Roll Camera")), Opt(EJamMenuAction::OpenOptions, TEXT("Notes")),
		Opt(EJamMenuAction::WalkOffSet, TEXT("Walk Off Set")) };
	V.Footer = MenuFooter(S);
	return V;
}

FJamScreenView JamScreens::Options(const FJamMenuSettings& S)
{
	FJamScreenView V;
	V.Screen = EJamScreen::Options;
	V.Kicker = TEXT("NOTES");
	V.Title = TEXT("Production Notes");
	V.Options = {
		Opt(EJamMenuAction::Volume, TEXT("Fenella"), FString::Printf(TEXT("◀  %s  ▶"), *Pct(S.Volume))),
		Opt(EJamMenuAction::Commentary, TEXT("Director's Commentary"), S.bCommentary ? TEXT("On") : TEXT("Off")),
		Opt(EJamMenuAction::Locked, TEXT("Budget"), TEXT("Low (locked)"), false),
		Opt(EJamMenuAction::Back, TEXT("Back")) };
	V.Footer = FString::Printf(TEXT("%s     %s / %s adjust"), *MenuFooter(S), S.bGamepad ? TEXT("D-pad left") : TEXT("A"), S.bGamepad ? TEXT("right") : TEXT("D"));
	return V;
}

FJamScreenView JamScreens::QuitConfirm(const FJamMenuSettings& S, bool bFromPause)
{
	FJamScreenView V;
	V.Screen = EJamScreen::QuitConfirm;
	V.Title = TEXT("Walk off set?");
	V.Subtitle = bFromPause ? TEXT("Your contract has thoughts about that. (Back to the title.)") : TEXT("Your contract has thoughts about that.");
	V.Options = { Opt(EJamMenuAction::WalkAnyway, TEXT("Walk anyway")), Opt(EJamMenuAction::Fine, TEXT("Fine")) };
	V.Selected = 1;   // the safe answer first
	V.Footer = MenuFooter(S);
	V.bOpaque = !bFromPause;
	return V;
}

FJamScreenView JamScreens::Pause(const FJamMenuSettings& S)
{
	FJamScreenView V;
	V.Screen = EJamScreen::Pause;
	V.Kicker = TEXT("PAUSED");
	V.Title = TEXT("Lunch");
	V.Options = {
		Opt(EJamMenuAction::Resume, TEXT("Back to work")),
		Opt(EJamMenuAction::Volume, TEXT("Fenella"), FString::Printf(TEXT("◀  %s  ▶"), *Pct(S.Volume))),
		Opt(EJamMenuAction::Commentary, TEXT("Director's Commentary"), S.bCommentary ? TEXT("On") : TEXT("Off")),
		Opt(EJamMenuAction::Locked, TEXT("Budget"), TEXT("Low (locked)"), false),
		Opt(EJamMenuAction::CharacterSheet, TEXT("Character Sheet")),
		Opt(EJamMenuAction::WalkOffSet, TEXT("Walk Off Set")) };
	V.Caption = FirstLine(EJamLineTrigger::Pause, 0);
	V.Footer = MenuFooter(S);
	V.bOpaque = false;
	return V;
}

FJamScreenView JamScreens::Premiere(const FJamPremiereTotals& T, const FJamMenuSettings& S)
{
	FJamScreenView V;
	V.Screen = EJamScreen::Premiere;
	V.Kicker = TEXT("TONIGHT: THE PREMIERE OF");
	V.Title = T.bDirectorsCut ? TEXT("THE DIRECTOR'S CUT") : TEXT("THE THEATRICAL RELEASE");
	V.Subtitle = T.bDirectorsCut ? TEXT("A bold, uncompromising vision. Hers.")
		: TEXT("Released against the director's wishes. She has asked that her name be removed.");
	V.Rows = {
		{ TEXT("FILM GROSS"), FString::Printf(TEXT("%d sprinkles"), T.FilmGross) },
		{ TEXT("DIRECTOR"), FString::Printf(TEXT("%d sprinkles"), T.Director) },
		{ TEXT("HERO"), FString::Printf(TEXT("%d sprinkles"), T.Hero) },
		{ TEXT("RATS"), TEXT("paid in celery") } };
	V.Quotes = T.bDirectorsCut
		? TArray<FString>{ TEXT("“Long.” — a rat"), TEXT("“I was in it!” — a different rat"),
			TEXT("“I've seen it nine times. I had to. I made it.” — F. Flicker") }
		: TArray<FString>{ TEXT("“Tighter.” — a rat"), TEXT("“Where was the bigger goblin?” — F. Flicker, from the lobby"),
			TEXT("“Fine.” — the hero's agent (the hero has no agent)") };
	// The hero's one word (Jokes.md set piece 12).
	V.Caption = TEXT("Asked for a speech, the hero says: “...Celery.”     Fenella: “Chills. Cut it.”");
	V.Footer = FString::Printf(TEXT("%s roll credits"), Confirm(S));
	return V;
}

TArray<FString> JamScreens::CreditLines(bool bDirectorsCut)
{
	const FString Her = bDirectorsCut ? TEXT("Fenella Flicker") : TEXT("a faerie who wishes to remain nameless");
	const FString Short = bDirectorsCut ? TEXT("F. Flicker") : TEXT("a faerie who wishes to remain nameless");
	return {
		FString::Printf(TEXT("Directed by %s"), *Her),
		FString::Printf(TEXT("Produced by %s"), *Her),
		FString::Printf(TEXT("Executive Producer: %s"), *Short),
		TEXT("Written by: “It's more of a feeling”"),
		TEXT("Starring: The Hero (as “Talent”)"),
		TEXT("Rats: Themselves"),
		TEXT("Second Rat from the Left: Gary"),
		TEXT("Goblins appear courtesy of their dentist"),
		TEXT("Villager"),
		TEXT("Different Villager"),
		TEXT("Villager (Moustache)"),
		TEXT("Catering: none"),
		TEXT("Boom Operator: F. Flicker (see incident report)"),   // she keeps this one even when uncredited
		TEXT("Stunt Double: Cardboard"),
		TEXT("Celery Wrangler: position vacant"),
		TEXT("Best Boy: a good boy"),
		FString::Printf(TEXT("Special Thanks: %s, accepting on her own behalf"), *Her),
		TEXT(""),
		TEXT("No rats were lectured more than necessary in the making of this film."),
		TEXT("Any resemblance to a game that existed is coincidental. This one never did."),
		TEXT(""),
		TEXT("— THE REAL CREDITS —"),
		TEXT("Made by Dylan for the 2026 Epic AI MegaJam"),
		TEXT("Engine: Unreal Engine 5.8"),
		TEXT("Code, scripts and docs: Claude Code"),
		TEXT("Design briefs: ChatGPT"),
		TEXT("3D models: Meshy, Hyper3D Rodin, fal.ai (Hunyuan3D)"),
		TEXT("Pre-jam work, tools and assets are declared on the itch.io page"),
		TEXT(""),
		TEXT("Thank you for watching."),
	};
}

FJamScreenView JamScreens::Credits(bool bDirectorsCut, const FJamMenuSettings& S)
{
	FJamScreenView V;
	V.Screen = EJamScreen::Credits;
	V.Title = bDirectorsCut ? TEXT("THE DIRECTOR'S CUT") : TEXT("THE THEATRICAL RELEASE");
	V.Body = CreditLines(bDirectorsCut);
	// The post-credits pitch shows when the scroll ends.
	V.Caption = TEXT("Sequel. Same rats. Same field. We call it a return to form.");
	V.Footer = FString::Printf(TEXT("%s back to the title"), Confirm(S));
	return V;
}

int32 JamScreens::Step(const TArray<FJamMenuOption>& Options, int32 Selected, int32 Delta)
{
	const int32 N = Options.Num();
	if (N == 0) { return 0; }
	int32 I = FMath::Clamp(Selected, 0, N - 1);
	for (int32 Tries = 0; Tries < N; ++Tries)
	{
		I = (I + Delta + N) % N;
		if (Options[I].bEnabled) { return I; }
	}
	return Selected;
}

bool JamScreens::Sample(const FString& Name, FJamScreenView& Out)
{
	FJamMenuSettings S;
	const FString N = Name.ToLower();
	if (N == TEXT("rating")) { Out = Rating(); }
	else if (N == TEXT("title")) { Out = Title(S); }
	else if (N == TEXT("options")) { Out = Options(S); Out.Selected = 1; }
	else if (N == TEXT("quit")) { Out = QuitConfirm(S, false); }
	else if (N == TEXT("pause")) { Out = Pause(S); }
	else if (N == TEXT("character")) { FJamProgression P; P.ReachMilestone(EJamMilestone::Scene1Wrap); Out = Character(P, S); }
	else if (N == TEXT("premiere") || N == TEXT("theatrical"))
	{
		FJamPremiereTotals T;
		T.bDirectorsCut = N == TEXT("premiere");
		T.FilmGross = 7800;
		T.Director = 7290;
		T.Hero = 510;
		Out = Premiere(T, S);
	}
	else if (N == TEXT("credits")) { Out = Credits(true, S); Out.Scroll = 0.35f; }
	else { return false; }
	return true;
}

FJamScreenView JamScreens::Character(const FJamProgression& Prog, const FJamMenuSettings& S)
{
	FJamScreenView V;
	V.Screen = EJamScreen::Character;
	V.Kicker = TEXT("CHARACTER SHEET");
	V.Title = TEXT("The Hero");
	V.Subtitle = FString::Printf(TEXT("Level %d. %d skill point%s unspent (spend them on a wrap card)."),
		Prog.GetLevel(), Prog.GetPoints(), Prog.GetPoints() == 1 ? TEXT("") : TEXT("s"));
	const FJamStatBlock St = Prog.GetStats();
	V.Rows = {
		{ TEXT("Lecture Power"), FString::Printf(TEXT("%.0f"), St.LecturePower) },
		{ TEXT("Radius"), FString::Printf(TEXT("%.0f uu"), St.Radius) },
		{ TEXT("Cooldown Recovery"), FString::Printf(TEXT("%.2fx"), St.CooldownRecovery) },
		{ TEXT("Move Speed"), FString::Printf(TEXT("%.0f uu/s"), St.MoveSpeed) } };
	// Levels 1 to 6, already spent before the film, cannot be refunded.
	V.Quotes.Add(TEXT("ALREADY SPENT, LEVELS 1 TO 6 (NON-REFUNDABLE):"));
	for (const FJamPassive& P : JamProgression::Passives())
	{
		V.Quotes.Add(FString::Printf(TEXT("%s (rank %d): %s"), *P.Name, P.Rank, *P.Tooltip));
	}
	V.Options = { Opt(EJamMenuAction::Back, TEXT("Back")) };
	V.Footer = FString::Printf(TEXT("%s back"), Confirm(S));
	V.bOpaque = true;
	return V;
}
