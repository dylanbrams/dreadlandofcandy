#include "Game/JamHUD.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Film/JamCast.h"
#include "Game/JamNuisance.h"
#include "Game/JamQuestMarker.h"
#include "UI/JamHudWidget.h"
#include "UI/JamScreenWidget.h"
#include "UI/JamNoteEditor.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Camera/PlayerCameraManager.h"
#include "Game/JamPlayerState.h"
#include "Gas/JamGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Game/JamGameMode.h"
#include "Game/JamSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "UI/JamMusic.h"
#include "Game/JamSfx.h"
#include "MegaJam.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

AJamHUD::AJamHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void AJamHUD::BeginPlay()
{
	Super::BeginPlay();
	FString Arg;
	if (FParse::Value(FCommandLine::Get(), TEXT("uiscreen="), Arg))
	{
		FJamHudView Probe;
		FJamScreenView ScreenProbe;
		if (JamScreens::Sample(Arg, ScreenProbe))
		{
			ScreenSampleName = Arg;
			UE_LOG(LogJam, Log, TEXT("JamHUD: -uiscreen=%s (sample screen)"), *Arg);
		}
		else if (JamHud::Sample(Arg, Probe))
		{
			SampleName = Arg;
			UE_LOG(LogJam, Log, TEXT("JamHUD: -uiscreen=%s (sample HUD)"), *Arg);
		}
		else
		{
			UE_LOG(LogJam, Log, TEXT("JamHUD: -uiscreen=%s is not a HUD sample (%s)"), *Arg, *FString::Join(JamHud::SampleNames(), TEXT(", ")));
		}
	}
	bNoMusic = FParse::Param(FCommandLine::Get(), TEXT("nomusic"));
	if (PlayerOwner && PlayerOwner->IsLocalController())
	{
		Widget = CreateWidget<UJamHudWidget>(PlayerOwner, UJamHudWidget::StaticClass());
		if (Widget)
        {
            Widget->AddToViewport(0);
            Widget->OnAction.BindLambda([this](EJamHudAction Action)
            {
                if (bPaused || (Screen && Screen->IsVisible()) || (NoteEditor && NoteEditor->IsOpen())) { return; }
                AJamHero* Hero = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr;
                if (!Hero) { return; }
                bool bAccepted = false;
                switch (Action)
                {
                case EJamHudAction::Skill1: bAccepted = Hero->RequestSkill(EJamSkill::Lecture); break;
                case EJamHudAction::Skill2: bAccepted = Hero->RequestSkill(EJamSkill::FormALine); break;
                case EJamHudAction::Ultimate: bAccepted = Hero->RequestSkill(EJamSkill::LastJudgment); break;
                case EJamHudAction::TakeNote:
                case EJamHudAction::Interact:
                case EJamHudAction::Continue: bAccepted = Hero->RequestInteract(); break;
                case EJamHudAction::RefuseNote: bAccepted = Hero->RequestRefuseNote(); break;
                case EJamHudAction::WriteNote: bAccepted = OpenNoteEditor(false); break;
                case EJamHudAction::Character: RequestPause(); SubScreen = EJamScreen::Character; bAccepted = bPaused; break;
                case EJamHudAction::Pause: RequestPause(); bAccepted = bPaused; break;
                }
                JamSfx::Play(this, bAccepted ? TEXT("UI_Click") : TEXT("UI_Unavailable"));
            });
        }
		Screen = CreateWidget<UJamScreenWidget>(PlayerOwner, UJamScreenWidget::StaticClass());
		if (Screen)
		{
			Screen->AddToViewport(10);
			Screen->SetVisibility(ESlateVisibility::Collapsed);
			Screen->OnMenuInput.BindUObject(this, &AJamHUD::HandleMenu);
			Screen->OnDevice.BindLambda([this](bool bGamepad) { bMenuGamepad = bGamepad; });
		}
		NoteEditor = CreateWidget<UJamNoteEditor>(PlayerOwner, UJamNoteEditor::StaticClass());
		if (NoteEditor)
		{
			NoteEditor->AddToViewport(5);
			NoteEditor->SetVisibility(ESlateVisibility::Collapsed);
			NoteEditor->OnSubmit.BindLambda([this](const FString& Note)
			{
				AJamGameMode* GM = AuthGameMode();
				AJamHero* H = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr;
				if (bProposalEditor)
				{
					bProposalEditor = false;
					if (GM) { GM->RequestVillageProposal(Note); }
				}
				else if (GM && H) { GM->HandleTypedNote(H, Note); }
			});
			NoteEditor->OnCancel.BindLambda([this]() { bProposalEditor = false; });
		}
		if (Arg == TEXT("rain")) { WeatherSample = 1; }
		if (Arg == TEXT("snow")) { WeatherSample = 2; }
		if (Arg == TEXT("fog")) { WeatherSample = 3; }
		if (Arg == TEXT("noteeditor") && NoteEditor)
		{
			// Sample: the editor over the running game, for screenshots.
			bEditorSample = true;
			NoteEditor->Open(TEXT("Stronger antagonist. Much stronger."), { TEXT("One enormous goblin"), TEXT("Same scene, but sadder"), TEXT("More. Just... more.") }, false);
		}

	}
}

void AJamHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (SampleName.IsEmpty()) { BuildView(); }
	else { JamHud::Sample(SampleName, View); }
	TickMusic();
	TickVoice();
	TickEffects();

	if (Screen)
	{
		FJamScreenView Sample;
		const bool bSample = !ScreenSampleName.IsEmpty() && JamScreens::Sample(ScreenSampleName, Sample);
		ScreenView = bSample ? Sample : BuildScreen();
		TickNoteEditor();
		const bool bEditor = NoteEditor && NoteEditor->IsOpen();
		// The proposal editor replaces the offer temporarily; Esc returns to its choices.
		const bool bShown = ScreenView.Screen != EJamScreen::None && !(bProposalEditor && bEditor);
		const ESlateVisibility Vis = bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		if (Screen->GetVisibility() != Vis) { Screen->SetVisibility(Vis); }
		if (bShown) { Screen->ApplyView(ScreenView, GetWorld()->GetRealTimeSeconds()); }
		// Resolve every overlay before applying captions, including the note editor.
		// Caption lifetime belongs to the game state (world time stops during pause).
		if (bShown || bEditor) { View.Caption.Reset(); View.Bubbles.Reset(); }
		if (Widget)
		{
			const ESlateVisibility HudVis = (bShown || bEditor) ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible;
			if (Widget->GetVisibility() != HudVis) { Widget->SetVisibility(HudVis); }
		}
		if (!bSample) { UpdateInputMode(bShown && ScreenView.Screen != EJamScreen::Rating ? 1 : (bEditor ? 2 : 0)); }
		// A collapsed screen can take keyboard focus back for a frame or two after it closes, leaving
		// mouse look working but WASD dead until a click. Reassert viewport focus for half a second.
		if (!bSample && InputModeState == 0 && GetWorld()->GetRealTimeSeconds() < RefocusGameUntil) { UWidgetBlueprintLibrary::SetFocusToGameViewport(); }
	}
	if (Widget) { Widget->ApplyView(View, GetWorld()->GetRealTimeSeconds()); }
}

void AJamHUD::DrawHUD()
{
	Super::DrawHUD();
	DrawCompass();
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	uint8 Weather = GS && GS->Phase == EJamFilmPhase::Shooting && !GS->bBudgetOut ? GS->Weather : 0;
	if (WeatherSample > 0) { Weather = WeatherSample; }
	if (!Canvas || Weather == 0) { return; }
	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float T = GetWorld()->GetRealTimeSeconds();
	const float Px = H / 1080.f;
	auto Frac = [](float X) { return X - FMath::FloorToFloat(X); };
	if (Weather == 1)   // rain: slanted streaks
	{
		for (int32 i = 0; i < 220; ++i)
		{
			const float X = Frac(i * 0.6180339f + T * 0.03f) * (W + 200.f) - 100.f;
			const float Y = Frac(i * 0.3711f + T * (1.3f + (i % 5) * 0.12f)) * (H + 60.f) - 30.f;
			DrawLine(X, Y, X - 9.f * Px, Y + 34.f * Px, FLinearColor(0.7f, 0.8f, 1.f, 0.45f), 1.5f * Px);
		}
		DrawRect(FLinearColor(0.1f, 0.15f, 0.3f, 0.18f), 0.f, 0.f, W, H);
	}
	else if (Weather == 2)   // sprinkle snow: candy sprinkles drifting down
	{
		static const FLinearColor Sprinkles[] = { FLinearColor(1.f, 0.4f, 0.7f, 0.9f), FLinearColor(0.4f, 1.f, 0.75f, 0.9f),
			FLinearColor(1.f, 0.85f, 0.3f, 0.9f), FLinearColor(0.6f, 0.5f, 1.f, 0.9f), FLinearColor(1.f, 1.f, 1.f, 0.9f) };
		for (int32 i = 0; i < 160; ++i)
		{
			const float Sway = FMath::Sin(T * 1.3f + i) * 18.f * Px;
			const float X = Frac(i * 0.6180339f) * W + Sway;
			const float Y = Frac(i * 0.4142f + T * (0.07f + (i % 4) * 0.015f)) * (H + 20.f) - 10.f;
			const bool bFlat = (i % 2) == 0;
			DrawRect(Sprinkles[i % 5], X, Y, (bFlat ? 10.f : 4.f) * Px, (bFlat ? 4.f : 10.f) * Px);
		}
	}
	else if (Weather == 3)   // fog: a milky wash, thicker at the top of the screen (further away)
	{
		for (int32 Band = 0; Band < 8; ++Band)
		{
			const float A = 0.32f - Band * 0.03f;
			DrawRect(FLinearColor(0.92f, 0.9f, 0.95f, A), 0.f, Band * H / 8.f, W, H / 8.f + 1.f);
		}
	}
}

bool AJamHUD::OpenNoteEditor(bool bGamepad)
{
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!NoteEditor || !GS || !GS->IsNotePrompt() || GS->bReadingNote || bPaused || NoteEditor->IsOpen()) { return false; }
	FString Pitch = GS->CardSubtitle;
	if (Pitch.StartsWith(TEXT("CUT!"))) { Pitch.RightChopInline(4); Pitch.TrimStartInline(); }
	NoteEditor->Open(Pitch, GS->SuggestedNotes, bGamepad);
	UE_LOG(LogJam, Log, TEXT("JamHUD: note editor open"));
	return true;
}

void AJamHUD::TickNoteEditor()
{
	if (!NoteEditor) { return; }
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	// It closes itself on give or back; it also goes when CUT! does (or a pause covers it).
	const AJamGameMode* GM = AuthGameMode();
	const bool bContextGone = bProposalEditor ? (!GM || !GM->GetQuestOffer().bVisible)
		: (!GS || !GS->IsNotePrompt() || GS->bReadingNote);
	if (NoteEditor->IsOpen() && !bEditorSample && bContextGone) { NoteEditor->Close(); }
	if (!NoteEditor->IsOpen()) { bProposalEditor = false; }
	const bool bShow = NoteEditor->IsOpen() && !bPaused;
	const ESlateVisibility Vis = bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (NoteEditor->GetVisibility() != Vis) { NoteEditor->SetVisibility(Vis); }
}

void AJamHUD::BuildView()
{
	View = FJamHudView();
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS) { return; }
	const bool bTravel = GS->bCompass && GS->Phase == EJamFilmPhase::Shooting;
	if (!bTravel && GS->GetShoot().GetState() == EJamShootState::Idle) { return; }
	View.bTravelVisible = bTravel;
	const FJamShoot& Shoot = GS->GetShoot();
	const EJamShootState State = Shoot.GetState();

	if (!bTravel)
	{
		View.bShootVisible = true;
		View.SceneLabel = FString::Printf(TEXT("SCENE %d: %s"), GS->SceneNumber,
			*(GS->SceneTitleOverride.IsEmpty() ? Shoot.GetScene().Title : GS->SceneTitleOverride).ToUpper());
		View.Take = Shoot.GetTake();
		View.bRolling = State == EJamShootState::Rolling;
		View.FilmSeconds = GS->FilmSeconds;
		View.Hint = GS->Hint;
		if (const AJamHero* H = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr)
		{
			if (H->IsFollowCamera() && !H->IsUsingGamepad()) { View.Hint += TEXT("   Mouse 2 drag / Q-E: camera; wheel: zoom"); }
		}

		View.bDignityVisible = GS->bDignityVisible;
		View.DignityFraction = GS->GetDignity().GetFraction();
		View.DignityHit = GS->GetDignity().IsStaggered() ? 1.f : 0.f;

		const FJamStageDef* Stage = Shoot.GetStage();
		if (Stage && State != EJamShootState::Wrap)
		{
			View.ObjectiveId = FString::Printf(TEXT("%d:%d:%d"), GS->SceneNumber, Shoot.GetTake(), Shoot.GetStageIndex());
			View.Direction = Stage->Objective;
			View.Objective = FString::Printf(TEXT("%s   %d/%d"), *Stage->Objective, Shoot.GetProgress(), Stage->Count);
			if (Stage->TimeLimit > 0.f)
			{
				const float Left = Shoot.GetTimeLeft();
				View.bMeterVisible = true;
				View.MeterFraction = Left / Stage->TimeLimit;
				View.MeterLabel = FString::Printf(TEXT("%.0fs left"), FMath::CeilToFloat(Left));
				View.bMeterDanger = Left <= 10.f;
			}
			else if (Stage->FailMeter > 0)
			{
				View.bMeterVisible = true;
				View.MeterFraction = 1.f - float(Shoot.GetMeter()) / Stage->FailMeter;
				View.MeterLabel = FString::Printf(TEXT("crops eaten %d/%d"), Shoot.GetMeter(), Stage->FailMeter);
				View.bMeterDanger = Shoot.GetMeter() * 10 >= Stage->FailMeter * 6;
			}
		}

		if (GS->IsCaptionVisible()) { View.Caption = GS->CaptionText; }
		// World bubbles, projected; gone when their speaker is.
		const float Now = GetWorld()->GetTimeSeconds();
		for (const AJamGameState::FWorldBubble& B : GS->Bubbles)
		{
			const AActor* Anchor = B.Anchor.Get();
			FVector2D OnScreen;
			if (Anchor && B.Until > Now && UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerOwner, Anchor->GetActorLocation() + B.Offset, OnScreen, false))
			{
				View.Bubbles.Add({ B.Text, OnScreen, B.bJagged });
			}
		}
		View.FailFlash = FMath::Clamp(1.f - (GetWorld()->GetTimeSeconds() - GS->TakeFailedAt) / 0.8f, 0.f, 1.f);
	}
	// Wraps remain readable while the compass already points to the next quest.
	if (GS->IsWrapVisible())
	{
		View.Wrap = GS->Wrap;
		View.Wrap.ContinueHint = GS->WrapUntil < 0.f
			? (GS->bVillageReturnAvailable ? TEXT("FILM COMPLETE!  Return to the village  [F / A / Click]") : TEXT("ADVENTURE COMPLETE!  Watch the Premiere  [F / A / Click]"))
			: (PlayerOwner && Cast<AJamHero>(PlayerOwner->GetPawn()) && Cast<AJamHero>(PlayerOwner->GetPawn())->IsUsingGamepad()
				? TEXT("Hold (A) to continue") : TEXT("Click or hold [F] to continue"));
	}

	// Skill bar and XP from the hero's progression; cooldowns from the ability system.
	if (const AJamPlayerState* PS = PlayerOwner ? PlayerOwner->GetPlayerState<AJamPlayerState>() : nullptr)
	{
		const FJamProgression& Prog = PS->GetProgression();
		const bool bPad = PlayerOwner && Cast<AJamHero>(PlayerOwner->GetPawn()) && Cast<AJamHero>(PlayerOwner->GetPawn())->IsUsingGamepad();
		static const TCHAR* KeysKb[] = { TEXT("Mouse 1"), TEXT("Mouse 3"), TEXT(""), TEXT("R") };
		static const TCHAR* KeysPad[] = { TEXT("RT"), TEXT("RB"), TEXT(""), TEXT("LB") };
		const FGameplayTag CooldownTags[] = { JamGameplayTags::Cooldown_Lecture, JamGameplayTags::Cooldown_Line, JamGameplayTags::Cooldown_Sigh, JamGameplayTags::Cooldown_Judgment };
		const bool bWrap = !bTravel && GS->IsWrapVisible();
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		const AJamHero* CastHero = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr;
		const FJamClassDef& CastDef = JamCast::Get(CastHero ? CastHero->GetCastIndex() : 0);
		for (EJamSkill Skill : JamProgression::ActiveSkills())
		{
			const int32 i = static_cast<int32>(Skill);
			FJamHudSkill Slot;
			static const TCHAR* Short[] = { TEXT("Lecture"), TEXT("Form A Line"), TEXT("Sigh"), TEXT("Judgment") };
			Slot.Name = CastDef.SkillNames.IsValidIndex(i) ? CastDef.SkillNames[i] : Short[i];
			Slot.IconPath = CastDef.SkillIcons.IsValidIndex(i) ? CastDef.SkillIcons[i] : FString();
			Slot.ClassColor = CastDef.PrimaryColor;
			Slot.DamageColor = CastDef.SkillDamageColors.IsValidIndex(i) ? CastDef.SkillDamageColors[i] : FLinearColor(0.75f, 0.75f, 0.75f);
			Slot.Key = bPad ? KeysPad[i] : KeysKb[i];
			Slot.Rank = Prog.GetRank(Skill);
			Slot.MaxRank = JamProgression::MaxRank(Skill);
			Slot.bUnlocked = Prog.IsUnlocked(Skill);
			Slot.bSpendable = bWrap && Prog.CanSpendPoint(Skill);
			if (ASC && Slot.bUnlocked)
			{
				const TArray<TPair<float, float>> Left = ASC->GetActiveEffectsTimeRemainingAndDuration(
					FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CooldownTags[i])));
				for (const TPair<float, float>& L : Left) { if (L.Value > 0.f) { Slot.Cooldown = FMath::Max(Slot.Cooldown, L.Key / L.Value); } }
			}
			View.Skills.Add(Slot);
		}
		View.Level = Prog.GetLevel();
		View.Xp = Prog.GetLevelProgress();
		if (Prog.IsNumericXp())
		{
			View.bEncoreXp = Prog.GetLevel() >= JamProgression::MaxLevel;
			View.EncoreLevel = Prog.GetEncoreLevel();
			View.XpEarned = View.bEncoreXp ? Prog.GetEncoreXp() : Prog.GetXp() - JamProgression::XpForLevel(Prog.GetLevel());
			View.XpNeeded = View.bEncoreXp ? JamProgression::EncoreXpPerLevel
				: JamProgression::XpForLevel(Prog.GetLevel() + 1) - JamProgression::XpForLevel(Prog.GetLevel());
		}
		View.Points = Prog.GetPoints();
		if (bWrap && View.Points > 0)
		{
			View.Wrap.LevelLine = FString::Printf(TEXT("LEVEL %d!  %d skill point%s: press a skill's button (%s) to rank it up"),
				View.Level, View.Points, View.Points == 1 ? TEXT("") : TEXT("s"), bPad ? TEXT("RT / RB / LB") : TEXT("Mouse 1 / Mouse 3 / R"));
		}
	}
	if (bTravel)
	{
		const AJamHero* TravelHero = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr;
		View.bGamepad = TravelHero && TravelHero->IsUsingGamepad();
		View.Hint = GS->Hint;
		if (!GS->VillagePrompt.IsEmpty() && GS->VillagePromptAnchor.IsValid())
		{
			FVector2D OnScreen;
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerOwner, GS->VillagePromptAnchor->GetActorLocation() + FVector(0, 0, 140), OnScreen, false))
			{
				View.bPromptVisible = true;
				View.Prompt = GS->VillagePrompt;
				View.PromptPosition = OnScreen;
				View.HoldFraction = TravelHero ? TravelHero->GetInteractHoldFraction() : 0.f;
			}
		}
		if (GS->IsCaptionVisible()) { View.Caption = GS->CaptionText; }
		return;
	}
	const FJamStageDef* Stage = Shoot.GetStage();
	View.NotesTaken = GS->RunNotesTaken;
	View.NotesRefused = GS->RunNotesRefused;
	View.bSulking = Shoot.IsSulking();

	const AJamHero* Hero = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr;
	View.bGamepad = Hero && Hero->IsUsingGamepad();
	View.HoldFraction = Hero ? Hero->GetInteractHoldFraction() : 0.f;
	if (GS->IsNotePrompt())
	{
		View.bNotePrompt = true;
		View.NotePitch = GS->CardSubtitle;
		View.NoteGross = Shoot.GetScene().Note.GrossMultiplier;
		View.bReadingNote = GS->bReadingNote;
		View.ReadingNote = GS->ReadingNote;
	}

	if (GS->IsCardVisible())
	{
		View.CardKind = GS->CardKind;
		View.CardTitle = GS->CardTitle;
		View.CardSubtitle = GS->CardSubtitle;
	}

	// Interact prompt over the target the hero would treat, only while treating counts.
	if (Hero && GS->CanInteract())
	{
		if (const AJamNuisance* Target = Hero->FindInteractTarget())
		{
			FVector2D OnScreen;
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerOwner, Target->GetActorLocation() + FVector(0.f, 0.f, 140.f), OnScreen, false))
			{
				const TCHAR* Verb = Stage && Stage->Target == EJamTargetKind::Goblin ? TEXT("Brush teeth") : TEXT("Feed celery");
				View.bPromptVisible = true;
				View.Prompt = FString::Printf(TEXT("%s  %s"), Hero->IsUsingGamepad() ? TEXT("Hold (A)") : TEXT("Click / Hold [F]"), Verb);
				View.PromptPosition = OnScreen;
			}
		}
	}
}

// ---- Screens (T16) ----------------------------------------------------------------------------------------------

namespace
{
	constexpr float RatingSeconds = 2.f;
	constexpr float CreditSecondsPerLine = 1.1f;
}

UJamSettings* AJamHUD::Settings() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamSettings>() : nullptr;
}

AJamGameMode* AJamHUD::AuthGameMode() const
{
	return GetWorld()->GetAuthGameMode<AJamGameMode>();
}

FJamMenuSettings AJamHUD::MenuSettings() const
{
	FJamMenuSettings S;
	if (const UJamSettings* J = Settings()) { S.MusicVolume = J->MusicVolume; S.VoiceVolume = J->VoiceVolume; S.EffectsVolume = J->EffectsVolume; S.bCommentary = J->bCommentary; S.CastIndex = J->CastIndex(); }
	S.bGamepad = bMenuGamepad;
	return S;
}

FJamScreenView AJamHUD::BuildScreen() const
{
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS) { return FJamScreenView(); }
	const FJamMenuSettings S = MenuSettings();
	const float InPhase = GetWorld()->GetTimeSeconds() - GS->PhaseSince;
	FJamScreenView V;

	if (const AJamGameMode* GM = AuthGameMode(); GS->Phase == EJamFilmPhase::Shooting && GM && GM->GetQuestOffer().bVisible)
	{
		FJamScreenView Choices = JamScreens::QuestBranches(GM->GetQuestOffer(), S);
		Choices.Selected = FMath::Clamp(Selected, 0, Choices.Options.Num() - 1);
		if (!Choices.Options[Choices.Selected].bEnabled) { Choices.Selected = JamScreens::Step(Choices.Options, Choices.Selected, 1); }
		return Choices;
	}
	if (GS->Phase == EJamFilmPhase::Title)
	{
		if (InPhase < RatingSeconds) { return JamScreens::Rating(); }
		V = SubScreen == EJamScreen::Options ? JamScreens::Options(S)
			: SubScreen == EJamScreen::QuitConfirm ? JamScreens::QuitConfirm(S, false) : JamScreens::Title(S);
	}
	else if (bPaused)
	{
		const AJamPlayerState* PS = PlayerOwner ? PlayerOwner->GetPlayerState<AJamPlayerState>() : nullptr;
		V = SubScreen == EJamScreen::QuitConfirm ? JamScreens::QuitConfirm(S, true)
			: (SubScreen == EJamScreen::Character && PS) ? JamScreens::Character(PS->GetProgression(), S) : JamScreens::Pause(S);
	}
	else if (GS->Phase == EJamFilmPhase::Premiere)
	{
		V = JamScreens::Premiere(GS->Premiere, S);
	}
	else if (GS->Phase == EJamFilmPhase::Credits)
	{
		V = JamScreens::Credits(GS->Premiere.bDirectorsCut, S);
		if (GS->bVillageReturnAvailable) { V.Footer = TEXT("Click / F / (A): return to Sugarborough"); }
		V.Scroll = InPhase / FMath::Max(1.f, V.Body.Num() * CreditSecondsPerLine);
	}
	if (SubScreen == EJamScreen::QuestJournal && bPaused)
	{
		if (const AJamGameMode* GM = AuthGameMode()) { V = JamScreens::Quests(GM->GetVillageQuests(), S); }
	}
	if (SubScreen == EJamScreen::Controls && (GS->Phase == EJamFilmPhase::Title || bPaused)) { V = JamScreens::Controls(S); V.bOpaque = !bPaused; }
	if (V.Options.Num() > 0)
	{
		V.Selected = FMath::Clamp(Selected, 0, V.Options.Num() - 1);
		if (!V.Options[V.Selected].bEnabled) { V.Selected = JamScreens::Step(V.Options, V.Selected, 1); }
	}
	return V;
}

void AJamHUD::UpdateInputMode(int32 Want)
{
	if (!PlayerOwner || Want == InputModeState) { return; }
	InputModeState = Want;
	UWidget* Focus = Want == 1 ? static_cast<UWidget*>(Screen.Get()) : Want == 2 && NoteEditor ? NoteEditor->FocusTarget() : nullptr;
	if (Focus)
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(Focus->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerOwner->SetInputMode(Mode);
	}
	else
	{
		FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(false);
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PlayerOwner->SetInputMode(Mode);
		// UIOnly leaves keyboard focus on the closed menu until the viewport is clicked.
		UWidgetBlueprintLibrary::SetFocusToGameViewport();
		RefocusGameUntil = GetWorld()->GetRealTimeSeconds() + 0.5;
	}
	// Keep HUD controls reachable in both modes. Open camera uses Mouse 2 drag, Q/E and wheel zoom.
	PlayerOwner->bShowMouseCursor = true;
}

void AJamHUD::SetPaused(bool bPause)
{
	bPaused = bPause;
	bLevelUpCharacter = false;
	SubScreen = EJamScreen::None;
	Selected = 0;
	if (PlayerOwner) { PlayerOwner->SetPause(bPause); }
	if (Voice) { Voice->SetPaused(bPause); }
	// Pause is requested from input between HUD ticks; hide the speech panel immediately.
	if (bPause && Widget) { Widget->SetVisibility(ESlateVisibility::Collapsed); }
	UE_LOG(LogJam, Log, TEXT("JamHUD: %s"), bPause ? TEXT("paused") : TEXT("resumed"));
}

void AJamHUD::RequestPause()
{
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS || GS->Phase != EJamFilmPhase::Shooting || bPaused) { return; }
	SetPaused(true);
}

void AJamHUD::Adjust(const FJamMenuOption& Option, int32 Delta)
{
	UJamSettings* J = Settings();
	if (!J) { return; }
	if (Option.Action == EJamMenuAction::MusicVolume) { J->SetMusicVolume(J->MusicVolume + 0.1f * Delta); }
	else if (Option.Action == EJamMenuAction::VoiceVolume) { J->SetVoiceVolume(J->VoiceVolume + 0.1f * Delta); }
	else if (Option.Action == EJamMenuAction::EffectsVolume) { J->SetEffectsVolume(J->EffectsVolume + 0.1f * Delta); }
	else if (Option.Action == EJamMenuAction::Commentary) { J->SetCommentary(!J->bCommentary); }
	else if (Option.Action == EJamMenuAction::Casting)
	{
		J->SetCastIndex(JamCast::Step(J->CastIndex(), Delta));
		if (AJamHero* Hero = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr) { Hero->ApplyCast(J->CastIndex(), true); }
	}
	else { return; }
	JamSfx::Play(this, TEXT("UI_Click"));   // after the change, so the Effects slider previews its new level
}

void AJamHUD::Activate(const FJamMenuOption& Option)
{
	AJamGameMode* GM = AuthGameMode();
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	const bool bTitle = GS && GS->Phase == EJamFilmPhase::Title;
	switch (Option.Action)
	{
	case EJamMenuAction::RollCamera:  if (GM) { GM->RequestRollCamera(); } break;
	case EJamMenuAction::OpenOptions:    ParentSelected = Selected; SubScreen = EJamScreen::Options; Selected = 0; break;
	case EJamMenuAction::OpenControls: ControlsParent = SubScreen; ControlsParentSelected = Selected; SubScreen = EJamScreen::Controls; Selected = 0; break;
	case EJamMenuAction::QuestBranch0: if (GM) { GM->ChooseVillageBranch(0); Selected = 0; } break;
	case EJamMenuAction::QuestBranch1: if (GM) { GM->ChooseVillageBranch(1); Selected = 0; } break;
	case EJamMenuAction::QuestBranch2: if (GM) { GM->ChooseVillageBranch(2); Selected = 0; } break;
	case EJamMenuAction::ProposePlan:
		if (GM && NoteEditor && GM->GetQuestOffer().bVisible && GM->GetQuestOffer().bCanPropose)
		{
			bProposalEditor = true;
			NoteEditor->Open(TEXT("How would you solve it?"), GM->ProposalSuggestions(), bMenuGamepad);
			// Force a new focus assignment even if the previous input mode was also UI.
			InputModeState = -1;
		}
		break;
	case EJamMenuAction::ImproviseQuest: if (GM) { GM->RequestVillageBranches(); Selected = 0; } break;
	case EJamMenuAction::CancelQuestOffer: if (GM) { GM->CancelVillageOffer(); Selected = 0; } break;
	case EJamMenuAction::AbandonQuest: if (GM && SubScreen == EJamScreen::QuestJournal) { GM->AbandonVillageQuest(); Selected = 0; } break;
	case EJamMenuAction::OpenQuests: ParentSelected = Selected; SubScreen = EJamScreen::QuestJournal; Selected = 0; break;
	case EJamMenuAction::CharacterSheet: ParentSelected = Selected; SubScreen = EJamScreen::Character; Selected = 0; break;
	case EJamMenuAction::WalkOffSet:     ParentSelected = Selected; SubScreen = EJamScreen::QuitConfirm; Selected = 1; break;
	case EJamMenuAction::Fine:
	case EJamMenuAction::Back:
		if (SubScreen == EJamScreen::Controls) { Selected = ControlsParentSelected; SubScreen = ControlsParent; }
		else if (SubScreen == EJamScreen::Character && bLevelUpCharacter) { SetPaused(false); }
		else { Selected = ParentSelected; SubScreen = EJamScreen::None; }
		break;
	case EJamMenuAction::RankBasic:
    case EJamMenuAction::RankSignature:
    case EJamMenuAction::RankUltimate:
        if (GM && IsCharacterSheetOpen())
        {
            const EJamSkill Skill = Option.Action == EJamMenuAction::RankBasic ? EJamSkill::Lecture
                : Option.Action == EJamMenuAction::RankSignature ? EJamSkill::FormALine : EJamSkill::LastJudgment;
            AJamHero* Hero = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr;
            GM->HandleSpendPoint(Hero, Skill, true);
        }
        break;
	case EJamMenuAction::Resume:      SetPaused(false); break;
	case EJamMenuAction::MusicVolume:
	case EJamMenuAction::VoiceVolume:
	case EJamMenuAction::EffectsVolume: Adjust(Option, 1); break;
	case EJamMenuAction::Commentary:  Adjust(Option, 1); break;
	case EJamMenuAction::Casting:     Adjust(Option, 1); break;
	case EJamMenuAction::WalkAnyway:
		if (bTitle) { UKismetSystemLibrary::QuitGame(this, PlayerOwner, EQuitPreference::Quit, false); }
		else { SetPaused(false); if (GM) { GM->RequestBackToTitle(); } }
		break;
	default: break;
	}
}

void AJamHUD::HandleMenu(EJamMenuInput Input, int32 Index)
{
	const EJamScreen Shown = ScreenView.Screen;
	AJamGameMode* GM = AuthGameMode();

	// Cards without options: confirm moves on.
	if (Shown == EJamScreen::Premiere || Shown == EJamScreen::Credits)
	{
		if (Input == EJamMenuInput::Confirm || Input == EJamMenuInput::Click)
		{
			JamSfx::Play(this, TEXT("UI_Click"));
			if (Shown == EJamScreen::Premiere && GM) { GM->RequestCredits(); }
			else if (GM) { GM->RequestBackToTitle(); }
		}
		return;
	}
	const TArray<FJamMenuOption>& Options = ScreenView.Options;
	if (Options.Num() == 0) { return; }
	switch (Input)
	{
	case EJamMenuInput::Up:    Selected = JamScreens::Step(Options, ScreenView.Selected, -1); JamSfx::Play(this, TEXT("UI_Click")); break;
	case EJamMenuInput::Down:  Selected = JamScreens::Step(Options, ScreenView.Selected, 1); JamSfx::Play(this, TEXT("UI_Click")); break;
	case EJamMenuInput::Left:  Adjust(Options[ScreenView.Selected], -1); break;
	case EJamMenuInput::Right: Adjust(Options[ScreenView.Selected], 1); break;
	case EJamMenuInput::Hover: if (Options.IsValidIndex(Index) && Options[Index].bEnabled) { Selected = Index; } break;
	case EJamMenuInput::Click:
		if (Options.IsValidIndex(Index) && Options[Index].bEnabled) { Selected = Index; JamSfx::Play(this, TEXT("UI_Click")); Activate(Options[Index]); }
		else { JamSfx::Play(this, TEXT("UI_Unavailable")); }
		break;
	case EJamMenuInput::Confirm:
		JamSfx::Play(this, Options[ScreenView.Selected].bEnabled ? TEXT("UI_Click") : TEXT("UI_Unavailable"));
		if (Options[ScreenView.Selected].bEnabled) { Activate(Options[ScreenView.Selected]); }
		break;
	case EJamMenuInput::Back:
		JamSfx::Play(this, TEXT("UI_Back"));
		if (GM && GM->GetQuestOffer().bVisible) { GM->CancelVillageOffer(); Selected = 0; }
		else if (SubScreen != EJamScreen::None) { Activate(FJamMenuOption{ EJamMenuAction::Back }); }
		else if (bPaused) { SetPaused(false); }
		break;
	}
}

// ---- Music (T18) ------------------------------------------------------------------------------------------------

void AJamHUD::TickMusic()
{
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS || !PlayerOwner || !PlayerOwner->IsLocalController()) { return; }
	FJamMusicContext C;
	C.Phase = GS->Phase;
	C.Scene = GS->SceneNumber;
	C.Stage = GS->GetShoot().GetStageIndex();
	C.bWalking = !GS->Hint.IsEmpty();
	C.bNotePrompt = GS->IsNotePrompt();
	C.bBudgetOut = GS->bBudgetOut && GS->Phase == EJamFilmPhase::Shooting;
	C.Mood = GS->Mood;
	C.WalkCue = GS->WalkMusicCue;
	const FName Cue = bNoMusic ? NAME_None : JamMusic::CueFor(C);
	const UJamSettings* J = Settings();
	const float Volume = 0.7f * (J ? J->MusicVolume : 0.8f);

	if (Cue == MusicCue)
	{
		if (Music) { Music->SetVolumeMultiplier(FMath::Max(Volume, 0.001f)); }
		return;
	}
	MusicCue = Cue;
	// The money running out cuts the music dead; everything else crossfades.
	if (Music) { if (C.bBudgetOut) { Music->Stop(); } else { Music->FadeOut(1.5f, 0.f); } Music = nullptr; }
	if (Cue.IsNone()) { return; }

	int32 Take = FMath::RandRange(1, JamMusic::Takes(Cue));   // Dylan: both takes are fine, play either
	USoundWave* Wave = LoadObject<USoundWave>(nullptr, *JamMusic::AssetPath(Cue, Take), nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!Wave && !JamMusic::Fallback(Cue).IsNone())
	{
		Take = FMath::RandRange(1, JamMusic::Takes(JamMusic::Fallback(Cue)));
		Wave = LoadObject<USoundWave>(nullptr, *JamMusic::AssetPath(JamMusic::Fallback(Cue), Take), nullptr, LOAD_NoWarn | LOAD_Quiet);
	}
	if (!Wave)
	{
		if (!MissingCues.Contains(Cue)) { MissingCues.Add(Cue); UE_LOG(LogJam, Log, TEXT("JamMusic: no asset for %s"), *JamMusic::AssetPath(Cue, Take)); }
		return;
	}
	Wave->bLooping = true;
	Music = UGameplayStatics::CreateSound2D(this, Wave, FMath::Max(Volume, 0.001f), 1.f, 0.f, nullptr, false, true);
	if (Music)
	{
		Music->bIsUISound = true;   // keeps playing under the pause menu
		Music->FadeIn(1.5f, 1.f);
		UE_LOG(LogJam, Log, TEXT("JamMusic: %s take %d"), *Cue.ToString(), Take);
	}
}

// ---- Compass (R4) ------------------------------------------------------------------------------------------------

void AJamHUD::DrawCompass()
{
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	const APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	if (!Canvas || !GS || !GS->bCompass || !Pawn || !PlayerOwner->PlayerCameraManager || GS->Phase != EJamFilmPhase::Shooting) { return; }
	if (Screen && Screen->GetVisibility() == ESlateVisibility::Visible) { return; }
	const float W = Canvas->ClipX;
	const float Px = Canvas->ClipY / 1080.f;
	const float CamYaw = PlayerOwner->PlayerCameraManager->GetCameraRotation().Yaw;
	const FVector To = GS->CompassTarget - Pawn->GetActorLocation();
	const float Rel = FRotator::NormalizeAxis(To.Rotation().Yaw - CamYaw);   // -180..180, 0 = straight ahead

	// The strip: 120 degrees of view across 900 px; beyond that the marker waits at the edge, pointing on.
	const float StripW = 900.f * Px, StripY = 46.f * Px, Cx = W * 0.5f;
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.35f), Cx - StripW * 0.5f, StripY - 3.f * Px, StripW, 6.f * Px);
	for (int32 Tick = -60; Tick <= 60; Tick += 15)
	{
		const float X = Cx + Tick / 60.f * StripW * 0.5f;
		DrawRect(FLinearColor(1.f, 1.f, 1.f, Tick == 0 ? 0.8f : 0.35f), X - 1.f * Px, StripY - (Tick == 0 ? 10.f : 6.f) * Px, 2.f * Px, (Tick == 0 ? 20.f : 12.f) * Px);
	}
	const float Clamped = FMath::Clamp(Rel, -60.f, 60.f);
	const float Mx = Cx + Clamped / 60.f * StripW * 0.5f;
	const float Pulse = 0.55f + 0.45f * (0.5f + 0.5f * FMath::Sin(GetWorld()->GetRealTimeSeconds() * 2.f * PI * 1.5f));
	const FLinearColor Gold(1.f, 0.83f, 0.3f, Pulse);
	const float S = 14.f * Px;
	// Match the world pointer: a red/gold cane on the active compass bearing.
	if (FMath::Abs(Rel) <= 60.f)
	{
		for (int32 Band = 0; Band < 6; ++Band)
		{
			const float Y = StripY - S * 2.f + Band * S / 3.f;
			DrawLine(Mx, Y, Mx, Y + S / 3.f, Band % 2 == 0 ? FLinearColor(1.f, 0.05f, 0.08f, Pulse) : Gold, 5.f * Px);
		}
		for (int32 Segment = 0; Segment < 8; ++Segment)
		{
			const float A = Segment * PI / 8.f, B = (Segment + 1) * PI / 8.f;
			DrawLine(Mx + S * 0.5f - S * 0.5f * FMath::Cos(A), StripY - S * 2.f - S * 0.5f * FMath::Sin(A),
				Mx + S * 0.5f - S * 0.5f * FMath::Cos(B), StripY - S * 2.f - S * 0.5f * FMath::Sin(B),
				Segment % 2 == 0 ? FLinearColor(1.f, 0.05f, 0.08f, Pulse) : Gold, 5.f * Px);
		}
		DrawLine(Mx - S * 0.5f, StripY - S * 0.5f, Mx, StripY, Gold, 3.f * Px);
		DrawLine(Mx + S * 0.5f, StripY - S * 0.5f, Mx, StripY, Gold, 3.f * Px);
	}
	else
	{
		const float Dir = Rel > 0.f ? 1.f : -1.f;
		DrawLine(Mx, StripY - S, Mx + Dir * S * 1.4f, StripY, Gold, 3.f * Px);
		DrawLine(Mx, StripY + S, Mx + Dir * S * 1.4f, StripY, Gold, 3.f * Px);
	}
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font) { return; }
	const FString Label = FString::Printf(TEXT("%s  %d m"), *GS->CompassLabel, FMath::RoundToInt(To.Size2D() / 100.f));
	float Tw = 0.f, Th = 0.f;
	const float Scale = 1.3f * Px;
	GetTextSize(Label, Tw, Th, Font, Scale);
	const float Lx = FMath::Clamp(Mx - Tw * 0.5f, 8.f, W - Tw - 8.f);
	DrawText(Label, FLinearColor::Black, Lx + 2.f * Px, StripY + 34.f * Px + 2.f * Px, Font, Scale);
	DrawText(Label, Gold, Lx, StripY + 34.f * Px, Font, Scale);

}

// ---- Sound effects the HUD sees (R1) ------------------------------------------------------------------------------

void AJamHUD::TickEffects()
{
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS || !PlayerOwner || !PlayerOwner->IsLocalController() || !SampleName.IsEmpty()) { return; }
	if (const AJamPlayerState* PS = PlayerOwner->GetPlayerState<AJamPlayerState>())
	{
		const int32 Level = PS->GetProgression().GetLevel();
		if (SeenLevel > 0 && Level > SeenLevel)
        {
            JamSfx::Play(this, TEXT("LevelUp"), 0.3f);
            if (PS->GetProgression().GetPoints() > 0 && !FParse::Param(FCommandLine::Get(), TEXT("autoplay")))
            {
                RequestPause();
                if (bPaused) { SubScreen = EJamScreen::Character; bLevelUpCharacter = true; }
            }
        }
		SeenLevel = Level;
		// Fixes3 #26: an encore level gets the fanfare but no menu (there is nothing to spend).
		const int32 Encore = PS->GetProgression().GetEncoreLevel();
		if (SeenEncore >= 0 && Encore > SeenEncore)
		{
			JamSfx::Play(this, TEXT("LevelUp"), 0.3f);
			if (AJamGameMode* GM = GetWorld()->GetAuthGameMode<AJamGameMode>())
			{
				GM->Direct(FString::Printf(TEXT("ENCORE LEVEL %d! +%d%% power. The audience demands more, and so, regrettably, do I."),
					Encore, FMath::RoundToInt(100.f * JamProgression::EncorePowerPerLevel * Encore)));
			}
		}
		SeenEncore = Encore;
	}
	const bool bWant = GS->Phase == EJamFilmPhase::Shooting && !GS->bBudgetOut;   // the money going takes the extras with it
	if (bWant && !Ambience) { Ambience = JamSfx::StartLoop(this, TEXT("Backlot_Ambience_Loop")); }
	else if (!bWant && Ambience) { Ambience->FadeOut(1.f, 0.f); Ambience = nullptr; }
	if (Ambience)
	{
		const FJamSfxCue* C = JamSfx::Find(TEXT("Backlot_Ambience_Loop"));
		Ambience->SetVolumeMultiplier(FMath::Max((C ? C->Volume : 0.3f) * JamSfx::EffectsVolume(this), 0.001f));
	}
}

// ---- Voice-over (T23) -------------------------------------------------------------------------------------------

void AJamHUD::TickVoice()
{
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS || !PlayerOwner || !PlayerOwner->IsLocalController()) { return; }
	if (bPaused) { return; } // Keep the current story line and its paused audio intact.
	const FName Line = GS->IsCaptionVisible() ? GS->CaptionLineId : NAME_None;
	if (Line == VoiceLine) { return; }
	VoiceLine = Line;
	if (Voice) { Voice->Stop(); Voice = nullptr; }   // a new line cuts the old one off, like a director would
	if (Line.IsNone()) { return; }
	// Clips are found by convention (BUILD_SPEC 5A); a missing one means the caption alone, which is the default.
	const FString Name = FString::Printf(TEXT("VO_%s"), *Line.ToString());
	USoundBase* Clip = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Audio/VO/%s.%s"), *Name, *Name), nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!Clip) { return; }
	const UJamSettings* J = Settings();
	Voice = UGameplayStatics::CreateSound2D(this, Clip, FMath::Max(J ? J->VoiceVolume : 0.8f, 0.001f), 1.f, 0.f, nullptr, false, true);
	if (Voice) { Voice->bIsUISound = true; Voice->Play(); }
}
