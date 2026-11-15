#include "UI/JamHudView.h"
#include "UI/JamUiStyle.h"
#include "Film/JamLines.h"
#include "Film/JamShoot.h"

FString JamHud::Timecode(float Seconds)
{
	const int32 Frames = FMath::FloorToInt(FMath::Max(0.f, Seconds) * 24.f);
	const int32 FF = Frames % 24;
	const int32 Total = Frames / 24;
	return FString::Printf(TEXT("%02d:%02d:%02d:%02d"), Total / 3600, (Total / 60) % 60, Total % 60, FF);
}

FJamCardStyle JamHud::CardStyle(EJamCardKind Kind)
{
	const FLinearColor Dark = JamUi::Surface();
	switch (Kind)
	{
	case EJamCardKind::Slate:  return { JamUi::Ink(), Dark };
	case EJamCardKind::Action: return { JamUi::Mint(), Dark };
	case EJamCardKind::Cut:    return { JamUi::Ruby(), Dark };
	case EJamCardKind::Retake: return { JamUi::Amethyst(), Dark };
	case EJamCardKind::Wrap:   return { JamUi::Frost(), Dark };
	default:                   return { JamUi::Amethyst(), Dark };
	}
}

namespace
{
	void SampleSkills(FJamHudView& V)
	{
		V.Level = 8;
		V.Xp = 0.45f;
		V.Points = 1;
		V.Skills = {
			{ TEXT("Lecture"), TEXT("Mouse 1"), 2, 4, true, 0.f, true },
			{ TEXT("Form A Line"), TEXT("Mouse 3"), 1, 3, true, 0.6f, true },
			{ TEXT("Good Manners"), TEXT("R"), 1, 1, true, 0.f, false } };
		V.Skills[1].CooldownSeconds = 3.f;
		const TCHAR* Icons[] = { TEXT("T_Skill_Lecture"), TEXT("T_Skill_FormALine"), TEXT("T_Skill_LastJudgment") };
		for (int32 i = 0; i < V.Skills.Num(); ++i) { V.Skills[i].IconPath = FString(TEXT("/Game/Art/UI/Icons/")) + Icons[i]; }
	}

	void Rolling(FJamHudView& V)
	{
		SampleSkills(V);
		V.bShootVisible = true;
		V.SceneLabel = FString::Printf(TEXT("SCENE 1: %s"), *JamScenes::Scene1().Title.ToUpper());
		V.Take = 2;
		V.bRolling = true;
		V.FilmSeconds = 83.5f;
		V.Objective = TEXT("Pacify the rats   2/3");
	}

	FString LineOr(FName Id, const TCHAR* Fallback)
	{
		const FJamLine* Line = JamLines::Find(Id);
		return Line ? Line->Text : FString(Fallback);
	}
}

TArray<FString> JamHud::SampleNames()
{
	return { TEXT("hud"), TEXT("slate"), TEXT("action"), TEXT("cut"), TEXT("retake"), TEXT("wrap"), TEXT("meter"), TEXT("dignity"), TEXT("bubbles"), TEXT("caption") };
}

bool JamHud::Sample(const FString& Name, FJamHudView& V)
{
	V = FJamHudView();
	const FString N = Name.ToLower();
	if (N == TEXT("hud"))
	{
		Rolling(V);
		V.bPromptVisible = true;
		V.Prompt = TEXT("Hold [F]  Feed celery");
		V.PromptPosition = FVector2D(1100.f, 560.f);
	}
	else if (N == TEXT("slate"))
	{
		Rolling(V);
		V.bRolling = false;
		V.CardKind = EJamCardKind::Slate;
		V.CardTitle = TEXT("SCENE 1 - TAKE 2");
		V.CardSubtitle = FString::Printf(TEXT("%s\nDirector: F. Flicker. Camera: F. Flicker. Rats: themselves."), *JamScenes::Scene1().Title);
	}
	else if (N == TEXT("action"))
	{
		Rolling(V);
		V.ObjectiveId = TEXT("sample-action");
		V.Direction = TEXT("Pacify the rats");
		V.CardKind = EJamCardKind::Action;
		V.CardTitle = TEXT("ACTION!");
		V.CardSubtitle = TEXT("Places, everyone! And by everyone I mean you.");
	}
	else if (N == TEXT("cut"))
	{
		Rolling(V);
		V.bRolling = false;
		V.SceneLabel = FString::Printf(TEXT("SCENE 2: %s"), *JamScenes::Scene2().Title.ToUpper());
		V.Take = 1;
		V.Objective = TEXT("Pacify the goblins   3/3");
		V.CardKind = EJamCardKind::Cut;
		V.CardTitle = TEXT("CUT!");
		V.CardSubtitle = LineOr(JamScenes::Scene2().Note.PitchLineId, TEXT("I have a note."));
		V.bNotePrompt = true;
		V.NotePitch = V.CardSubtitle;
		V.NoteGross = JamScenes::Scene2().Note.GrossMultiplier;
		V.HoldFraction = 0.4f;
	}
	else if (N == TEXT("retake"))
	{
		Rolling(V);
		V.bRolling = false;
		V.SceneLabel = FString::Printf(TEXT("SCENE 2: %s"), *JamScenes::Scene2().Title.ToUpper());
		V.Objective = TEXT("Brush the goblins' teeth   1/3");
		V.bMeterVisible = true;
		V.MeterFraction = 0.f;
		V.MeterLabel = TEXT("0s left");
		V.bMeterDanger = true;
		V.CardKind = EJamCardKind::Retake;
		V.CardTitle = TEXT("RETAKE!");
		V.CardSubtitle = TEXT("Out of time. Back to one!");
		V.FailFlash = 0.5f;
	}
	else if (N == TEXT("wrap"))
	{
		Rolling(V);
		V.bRolling = false;
		V.Objective.Reset();
		V.CardKind = EJamCardKind::Wrap;
		V.CardTitle = TEXT("THAT'S A WRAP!");
		V.CardSubtitle = TEXT("Everyone gets paid. \"Everyone\" is a term of art.");
		V.Hint = TEXT("Walk up the street to Fort Toothache");
		{
			const FJamWrapOutcome Out = JamWrap::Roll(0, 1.f, 0);
			FJamProgression P;
			const FJamStatBlock Before = P.GetStats();
			P.AddLoot(Out.LootIndices[Out.EquippedSlot]);
			V.Wrap = JamWrap::MakeView(Out, JamScenes::Scene1().Title, Before, P.GetStats(), 1, 0);
			V.Wrap.ContinueHint = TEXT("Hold [F] to continue");
		}
	}
	else if (N == TEXT("meter"))
	{
		Rolling(V);
		V.SceneLabel = FString::Printf(TEXT("SCENE 3: %s"), *JamScenes::Scene3().Title.ToUpper());
		V.Objective = TEXT("Pacify the bugs   4/9");
		V.bMeterVisible = true;
		V.MeterFraction = 0.25f;
		V.MeterLabel = TEXT("crops eaten 6/8");
		V.bMeterDanger = true;
	}
	else if (N == TEXT("dignity"))
	{
		Rolling(V);
		V.SceneLabel = FString::Printf(TEXT("SCENE 2: %s"), *JamScenes::Scene2().Title.ToUpper());
		V.Objective = TEXT("Pacify the goblins   1/3");
		V.bDignityVisible = true;
		V.DignityFraction = 0.2f;
		V.DignityHit = 0.6f;
	}
	else if (N == TEXT("bubbles"))
	{
		Rolling(V);
		V.Bubbles.Add({ TEXT("Rats! You are vermin, and vermin is a choice."), FVector2D(760.f, 430.f), true });
		V.Bubbles.Add({ TEXT("Consider your lifestyle!"), FVector2D(1180.f, 520.f), true });
		V.Bubbles.Add({ TEXT("Ooh, keep that face. That's the poster."), FVector2D(1380.f, 300.f), false });
	}
	else if (N == TEXT("caption"))
	{
		Rolling(V);
		V.Caption = LineOr(TEXT("Sulking_01"), TEXT("Fine."));
		V.bSulking = true;
		V.NotesTaken = 1;
		V.NotesRefused = 1;
	}
	else
	{
		return false;
	}
	return true;
}
