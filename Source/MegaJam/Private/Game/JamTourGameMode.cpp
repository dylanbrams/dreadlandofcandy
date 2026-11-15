#include "Game/JamTourGameMode.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "Game/JamFaerie.h"
#include "Game/JamHero.h"
#include "Game/JamTourPawn.h"
#include "InputCoreTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CommandLine.h"
#include "MegaJam.h"

AJamTourGameMode::AJamTourGameMode()
{
	HUDClass = AHUD::StaticClass();   // the plain engine HUD draws nothing: no widgets, no canvas, no music
}

void AJamTourGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);   // spawns the hero and builds the village, the look and the ambience exactly as in play
	AJamHero* Player = NewPlayer ? Cast<AJamHero>(NewPlayer->GetPawn()) : nullptr;
	if (!Player) { UE_LOG(LogJam, Error, TEXT("JamTour: no hero to hide, no tour")); return; }

	// The hero and her faerie are not in the shot, and must not catch the camera's traces.
	Player->SetActorHiddenInGame(true);
	Player->SetActorEnableCollision(false);
	for (TActorIterator<AJamFaerie> It(GetWorld()); It; ++It) { It->SetActorHiddenInGame(true); }

	const TCHAR* Cmd = FCommandLine::Get();
	float Speed = 650.f, Height = 380.f, Start = 0.f;
	FParse::Value(Cmd, TEXT("tourspeed="), Speed);
	FParse::Value(Cmd, TEXT("tourheight="), Height);
	FParse::Value(Cmd, TEXT("tourstart="), Start);
	if (FParse::Value(Cmd, TEXT("tourtime="), QuitAfter)) { UE_LOG(LogJam, Log, TEXT("JamTour: quitting after %.0f s"), QuitAfter); }

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AJamTourPawn* Camera = GetWorld()->SpawnActor<AJamTourPawn>(AJamTourPawn::StaticClass(), Player->GetActorTransform(), Params);
	if (!Camera) { UE_LOG(LogJam, Error, TEXT("JamTour: could not spawn the camera")); return; }
	Camera->Begin(Speed, Height, Start);
	NewPlayer->Possess(Camera);
	NewPlayer->SetInputMode(FInputModeGameOnly());
	NewPlayer->bShowMouseCursor = false;
}

void AJamTourGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Elapsed += DeltaSeconds;
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	const bool bEscape = PC && PC->WasInputKeyJustPressed(EKeys::Escape);
	if (bEscape || (QuitAfter > 0.f && Elapsed >= QuitAfter))
	{
		UE_LOG(LogJam, Log, TEXT("JamTour: done after %.0f s"), Elapsed);
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}
