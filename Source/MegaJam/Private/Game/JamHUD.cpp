#include "Game/JamHUD.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamNuisance.h"
#include "UI/JamHudWidget.h"
#include "UI/JamScreenWidget.h"
#include "UI/JamNoteEditor.h"
#include "Engine/Canvas.h"
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
#include "MegaJam.h"
#include "Blueprint/WidgetLayoutLibrary.h"
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
		if (Widget) { Widget->AddToViewport(0); }
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
				if (GM && H) { GM->HandleTypedNote(H, Note); }
			});
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
	if (Widget) { Widget->ApplyView(View, GetWorld()->GetRealTimeSeconds()); }
	TickMusic();
	TickVoice();

	if (Screen)
	{
		FJamScreenView Sample;
		const bool bSample = !ScreenSampleName.IsEmpty() && JamScreens::Sample(ScreenSampleName, Sample);
		ScreenView = bSample ? Sample : BuildScreen();
		const bool bShown = ScreenView.Screen != EJamScreen::None;
		const ESlateVisibility Vis = bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
		if (Screen->GetVisibility() != Vis) { Screen->SetVisibility(Vis); }
		if (bShown) { Screen->ApplyView(ScreenView, GetWorld()->GetRealTimeSeconds()); }
		// The game HUD hides under any screen (pause too: two captions and a card under the menu was clutter).
		if (Widget)
		{
			const ESlateVisibility HudVis = bShown ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible;
			if (Widget->GetVisibility() != HudVis) { Widget->SetVisibility(HudVis); }
		}
		TickNoteEditor();
		const bool bEditor = NoteEditor && NoteEditor->IsOpen();
		if (!bSample) { UpdateInputMode(bShown && ScreenView.Screen != EJamScreen::Rating ? 1 : (bEditor ? 2 : 0)); }
	}
}

void AJamHUD::DrawHUD()
{
	Super::DrawHUD();
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
	if (NoteEditor->IsOpen() && !bEditorSample && (!GS || !GS->IsNotePrompt() || GS->bReadingNote)) { NoteEditor->Close(); }
	const bool bShow = NoteEditor->IsOpen() && !bPaused;
	const ESlateVisibility Vis = bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (NoteEditor->GetVisibility() != Vis) { NoteEditor->SetVisibility(Vis); }
}

void AJamHUD::BuildView()
{
	View = FJamHudView();
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS || GS->GetShoot().GetState() == EJamShootState::Idle) { return; }
	const FJamShoot& Shoot = GS->GetShoot();
	const EJamShootState State = Shoot.GetState();

	View.bShootVisible = true;
	View.SceneLabel = FString::Printf(TEXT("SCENE %d: %s"), GS->SceneNumber,
		*(GS->SceneTitleOverride.IsEmpty() ? Shoot.GetScene().Title : GS->SceneTitleOverride).ToUpper());
	View.Take = Shoot.GetTake();
	View.bRolling = State == EJamShootState::Rolling;
	View.FilmSeconds = GS->FilmSeconds;
	View.Hint = GS->Hint;

	const FJamStageDef* Stage = Shoot.GetStage();
	if (Stage && State != EJamShootState::Wrap)
	{
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
	if (GS->IsWrapVisible())
	{
		View.Wrap = GS->Wrap;
		View.Wrap.ContinueHint = GS->WrapUntil < 0.f ? FString()
			: (PlayerOwner && Cast<AJamHero>(PlayerOwner->GetPawn()) && Cast<AJamHero>(PlayerOwner->GetPawn())->IsUsingGamepad()
				? TEXT("Hold (A) to continue") : TEXT("Hold [E] to continue"));
	}
	// Skill bar and XP from the hero's progression; cooldowns from the ability system.
	if (const AJamPlayerState* PS = PlayerOwner ? PlayerOwner->GetPlayerState<AJamPlayerState>() : nullptr)
	{
		const FJamProgression& Prog = PS->GetProgression();
		const bool bPad = PlayerOwner && Cast<AJamHero>(PlayerOwner->GetPawn()) && Cast<AJamHero>(PlayerOwner->GetPawn())->IsUsingGamepad();
		static const TCHAR* KeysKb[] = { TEXT("LMB"), TEXT("RMB"), TEXT("Q"), TEXT("R") };
		static const TCHAR* KeysPad[] = { TEXT("RT"), TEXT("RB"), TEXT("LB"), TEXT("Y") };
		const FGameplayTag CooldownTags[] = { JamGameplayTags::Cooldown_Lecture, JamGameplayTags::Cooldown_Line, JamGameplayTags::Cooldown_Sigh, JamGameplayTags::Cooldown_Judgment };
		const bool bWrap = GS->IsWrapVisible();
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		for (int32 i = 0; i < (int32)EJamSkill::Count; ++i)
		{
			const EJamSkill Skill = (EJamSkill)i;
			FJamHudSkill Slot;
			static const TCHAR* Short[] = { TEXT("Lecture"), TEXT("Form A Line"), TEXT("Sigh"), TEXT("Judgment") };
			Slot.Name = Short[i];   // the bar is small; the full names are on the character sheet
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
		View.Points = Prog.GetPoints();
		if (bWrap && View.Points > 0)
		{
			View.Wrap.LevelLine = FString::Printf(TEXT("LEVEL %d!  %d skill point%s: press a skill's button (%s) to rank it up"),
				View.Level, View.Points, View.Points == 1 ? TEXT("") : TEXT("s"), bPad ? TEXT("RT / RB / LB / Y") : TEXT("LMB / RMB / Q / R"));
		}
	}
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
				View.Prompt = FString::Printf(TEXT("%s  %s"), Hero->IsUsingGamepad() ? TEXT("Hold (A)") : TEXT("Hold [E]"), Verb);
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
	if (const UJamSettings* J = Settings()) { S.MusicVolume = J->MusicVolume; S.VoiceVolume = J->VoiceVolume; S.bCommentary = J->bCommentary; }
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
		V.Scroll = InPhase / FMath::Max(1.f, V.Body.Num() * CreditSecondsPerLine);
	}
	if (V.Options.Num() > 0) { V.Selected = FMath::Clamp(Selected, 0, V.Options.Num() - 1); }
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
		PlayerOwner->SetInputMode(FInputModeGameOnly());
	}
	PlayerOwner->bShowMouseCursor = true;   // the hero faces the cursor; menus are clickable
}

void AJamHUD::SetPaused(bool bPause)
{
	bPaused = bPause;
	SubScreen = EJamScreen::None;
	Selected = 0;
	if (PlayerOwner) { PlayerOwner->SetPause(bPause); }
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
	else if (Option.Action == EJamMenuAction::Commentary) { J->SetCommentary(!J->bCommentary); }
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
	case EJamMenuAction::CharacterSheet: ParentSelected = Selected; SubScreen = EJamScreen::Character; Selected = 0; break;
	case EJamMenuAction::WalkOffSet:     ParentSelected = Selected; SubScreen = EJamScreen::QuitConfirm; Selected = 1; break;
	case EJamMenuAction::Fine:
	case EJamMenuAction::Back:           Selected = ParentSelected; SubScreen = EJamScreen::None; break;
	case EJamMenuAction::Resume:      SetPaused(false); break;
	case EJamMenuAction::MusicVolume:
	case EJamMenuAction::VoiceVolume: Adjust(Option, 1); break;
	case EJamMenuAction::Commentary:  Adjust(Option, 1); break;
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
			if (Shown == EJamScreen::Premiere && GM) { GM->RequestCredits(); }
			else if (GM) { GM->RequestBackToTitle(); }
		}
		return;
	}
	const TArray<FJamMenuOption>& Options = ScreenView.Options;
	if (Options.Num() == 0) { return; }
	switch (Input)
	{
	case EJamMenuInput::Up:    Selected = JamScreens::Step(Options, ScreenView.Selected, -1); break;
	case EJamMenuInput::Down:  Selected = JamScreens::Step(Options, ScreenView.Selected, 1); break;
	case EJamMenuInput::Left:  Adjust(Options[ScreenView.Selected], -1); break;
	case EJamMenuInput::Right: Adjust(Options[ScreenView.Selected], 1); break;
	case EJamMenuInput::Hover: if (Options.IsValidIndex(Index) && Options[Index].bEnabled) { Selected = Index; } break;
	case EJamMenuInput::Click:
		if (Options.IsValidIndex(Index) && Options[Index].bEnabled) { Selected = Index; Activate(Options[Index]); }
		break;
	case EJamMenuInput::Confirm: Activate(Options[ScreenView.Selected]); break;
	case EJamMenuInput::Back:
		if (SubScreen != EJamScreen::None) { Activate(FJamMenuOption{ EJamMenuAction::Back }); }
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

	const int32 Take = FMath::RandRange(1, 2);   // Dylan: both takes are fine, play either
	USoundWave* Wave = LoadObject<USoundWave>(nullptr, *JamMusic::AssetPath(Cue, Take));
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

// ---- Voice-over (T23) -------------------------------------------------------------------------------------------

void AJamHUD::TickVoice()
{
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS || !PlayerOwner || !PlayerOwner->IsLocalController()) { return; }
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
