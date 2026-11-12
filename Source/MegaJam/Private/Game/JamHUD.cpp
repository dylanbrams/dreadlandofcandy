#include "Game/JamHUD.h"
#include "Game/JamHero.h"
#include "Game/JamNuisance.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"

void AJamHUD::DrawHUD()
{
	Super::DrawHUD();
	const AJamHero* Hero = PlayerOwner ? Cast<AJamHero>(PlayerOwner->GetPawn()) : nullptr;
	if (!Hero || !Canvas) { return; }

	if (const AJamNuisance* Target = Hero->FindInteractTarget())
	{
		const FVector Screen = Project(Target->GetActorLocation() + FVector(0.f, 0.f, 140.f));
		if (Screen.Z > 0.f)
		{
			const FString Prompt = Hero->IsUsingGamepad() ? TEXT("Hold (A)  Treat") : TEXT("Hold [E]  Treat");
			UFont* Font = GEngine->GetLargeFont();
			float W = 0.f, H = 0.f;
			GetTextSize(Prompt, W, H, Font);
			const float X = Screen.X - W * 0.5f, Y = Screen.Y - H * 0.5f;
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), X - 10.f, Y - 6.f, W + 20.f, H + 12.f);
			DrawText(Prompt, FLinearColor::White, X, Y, Font);
		}
	}
}
