#include "Game/JamHUD.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamNuisance.h"
#include "UI/JamHudWidget.h"
#include "UI/JamScreenWidget.h"
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
	}
}

void AJamHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (SampleName.IsEmpty()) { BuildView(); }
	else { JamHud::Sample(SampleName, View); }
	if (Widget) { Widget->ApplyView(View, GetWorld()->GetRealTimeSeconds()); }
	TickMusic();

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
		if (!bSample) { UpdateInputMode(bShown && ScreenView.Screen != EJamScreen::Rating); }
	}
}

void AJamHUD::BuildView()
{
	View = FJamHudView();
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	if (!GS || GS->GetShoot().GetState() == EJamShootState::Idle) { return; }
	const FJamShoot& Shoot = GS->GetShoot();
	const EJamShootState State = Shoot.GetState();

	View.bShootVisible = true;
	View.SceneLabel = FString::Printf(TEXT("SCENE %d: %s"), GS->SceneNumber, *Shoot.GetScene().Title.ToUpper());
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
	if (const UJamSettings* J = Settings()) { S.Volume = J->Volume; S.bCommentary = J->bCommentary; }
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

void AJamHUD::UpdateInputMode(bool bInteractive)
{
	const int32 Want = bInteractive ? 1 : 0;
	if (!PlayerOwner || Want == InputModeState) { return; }
	InputModeState = Want;
	if (bInteractive && Screen)
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(Screen->TakeWidget());
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
	if (Option.Action == EJamMenuAction::Volume) { J->SetVolume(J->Volume + 0.1f * Delta); }
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
	case EJamMenuAction::Volume:      Adjust(Option, 1); break;
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
	const FName Cue = bNoMusic ? NAME_None : JamMusic::CueFor(C);
	const UJamSettings* J = Settings();
	const float Volume = 0.7f * (J ? J->Volume : 0.8f);

	if (Cue == MusicCue)
	{
		if (Music) { Music->SetVolumeMultiplier(FMath::Max(Volume, 0.001f)); }
		return;
	}
	MusicCue = Cue;
	if (Music) { Music->FadeOut(1.5f, 0.f); Music = nullptr; }
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
