#include "Game/JamHUD.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamNuisance.h"
#include "UI/JamHudWidget.h"
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
	FString Screen;
	if (FParse::Value(FCommandLine::Get(), TEXT("uiscreen="), Screen))
	{
		FJamHudView Probe;
		if (JamHud::Sample(Screen, Probe))
		{
			SampleName = Screen;
			UE_LOG(LogJam, Log, TEXT("JamHUD: -uiscreen=%s (sample HUD)"), *Screen);
		}
		else
		{
			UE_LOG(LogJam, Log, TEXT("JamHUD: -uiscreen=%s is not a HUD sample (%s)"), *Screen, *FString::Join(JamHud::SampleNames(), TEXT(", ")));
		}
	}
	if (PlayerOwner && PlayerOwner->IsLocalController())
	{
		Widget = CreateWidget<UJamHudWidget>(PlayerOwner, UJamHudWidget::StaticClass());
		if (Widget) { Widget->AddToViewport(0); }
	}
}

void AJamHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (SampleName.IsEmpty()) { BuildView(); }
	else { JamHud::Sample(SampleName, View); }
	if (Widget) { Widget->ApplyView(View, GetWorld()->GetRealTimeSeconds()); }
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
			FVector2D Screen;
			if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PlayerOwner, Target->GetActorLocation() + FVector(0.f, 0.f, 140.f), Screen, false))
			{
				const TCHAR* Verb = Stage && Stage->Target == EJamTargetKind::Goblin ? TEXT("Brush teeth") : TEXT("Feed celery");
				View.bPromptVisible = true;
				View.Prompt = FString::Printf(TEXT("%s  %s"), Hero->IsUsingGamepad() ? TEXT("Hold (A)") : TEXT("Hold [E]"), Verb);
				View.PromptPosition = Screen;
			}
		}
	}
}
