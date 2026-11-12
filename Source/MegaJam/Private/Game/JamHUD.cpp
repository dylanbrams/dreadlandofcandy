#include "Game/JamHUD.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamNuisance.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

namespace
{
	const FLinearColor Panel(0.f, 0.f, 0.f, 0.6f);
	const FLinearColor RecRed(0.9f, 0.1f, 0.1f);
}

void AJamHUD::DrawTextBox(const FString& Text, float X, float Y, UFont* Font, float Scale, const FLinearColor& Colour, bool bCentred)
{
	float W = 0.f, H = 0.f;
	GetTextSize(Text, W, H, Font, Scale);
	if (bCentred) { X -= W * 0.5f; }
	DrawRect(Panel, X - 12.f, Y - 6.f, W + 24.f, H + 12.f);
	DrawText(Text, Colour, X, Y, Font, Scale);
}

void AJamHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) { return; }
	const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
	const AJamHero* Hero = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr;
	UFont* Font = GEngine->GetLargeFont();
	const float UIScale = Canvas->ClipY / 1080.f;

	if (GS && GS->GetShoot().GetState() != EJamShootState::Idle)
	{
		const FJamShoot& Shoot = GS->GetShoot();

		// Top left: slate label and a blinking REC while rolling.
		const bool bRolling = Shoot.GetState() == EJamShootState::Rolling;
		const FString Label = FString::Printf(TEXT("SCENE 1: %s   TAKE %d"), *Shoot.GetScene().Title.ToUpper(), Shoot.GetTake());
		DrawTextBox(Label, 40.f * UIScale, 36.f * UIScale, Font, 1.4f * UIScale, FLinearColor::White, false);
		if (bRolling && FMath::Fmod(GetWorld()->GetTimeSeconds(), 1.f) < 0.6f)
		{
			DrawTextBox(TEXT("● REC"), 40.f * UIScale, 90.f * UIScale, Font, 1.2f * UIScale, RecRed, false);
		}

		// Top centre: objective and progress (and the countdown, when there is one).
		if (const FJamStageDef* Stage = Shoot.GetStage())
		{
			if (Shoot.GetState() != EJamShootState::Wrap)
			{
				FString Objective = FString::Printf(TEXT("%s   %d/%d"), *Stage->Objective, Shoot.GetProgress(), Stage->Count);
				if (Stage->TimeLimit > 0.f) { Objective += FString::Printf(TEXT("   %.0fs"), FMath::CeilToFloat(Shoot.GetTimeLeft())); }
				DrawTextBox(Objective, Canvas->ClipX * 0.5f, 36.f * UIScale, Font, 1.4f * UIScale, FLinearColor::White, true);
			}
		}

		// Centre: the card.
		if (GS->IsCardVisible())
		{
			const float CY = Canvas->ClipY * 0.38f;
			DrawTextBox(GS->CardTitle, Canvas->ClipX * 0.5f, CY, Font, 3.2f * UIScale, FLinearColor(1.f, 0.85f, 0.3f), true);
			if (!GS->CardSubtitle.IsEmpty())
			{
				TArray<FString> Lines;
				GS->CardSubtitle.ParseIntoArray(Lines, TEXT("\n"));
				float Y = CY + 90.f * UIScale;
				for (const FString& Line : Lines)
				{
					DrawTextBox(Line, Canvas->ClipX * 0.5f, Y, Font, 1.3f * UIScale, FLinearColor::White, true);
					Y += 40.f * UIScale;
				}
			}
		}
	}

	// Interact prompt over the rat the hero would treat, only while treating counts.
	if (Hero && GS && GS->CanInteract())
	{
		if (const AJamNuisance* Target = Hero->FindInteractTarget())
		{
			const FVector Screen = Project(Target->GetActorLocation() + FVector(0.f, 0.f, 140.f));
			if (Screen.Z > 0.f)
			{
				const FString Prompt = Hero->IsUsingGamepad() ? TEXT("Hold (A)  Feed celery") : TEXT("Hold [E]  Feed celery");
				DrawTextBox(Prompt, Screen.X, Screen.Y, Font, 1.2f * UIScale, FLinearColor::White, true);
			}
		}
	}
}
