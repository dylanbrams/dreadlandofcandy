#include "Game/JamLook.h"
#include "MegaJam.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
	APostProcessVolume* FindGrade(UWorld* World)
	{
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			if (It->Tags.Contains(TEXT("JamGrade"))) { return *It; }
		}
		return nullptr;
	}
}

void JamLook::Apply(UWorld* World)
{
	if (!World || FParse::Param(FCommandLine::Get(), TEXT("nograde"))) { return; }

	// The sun: a late-afternoon warm key from behind the camera's left, so shadows fall up and right, away from
	// the hero's face. A brighter sky fills the shadows so the black knight reads against the green.
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		if (UDirectionalLightComponent* Sun = Cast<UDirectionalLightComponent>(It->GetLightComponent()))
		{
			Sun->SetMobility(EComponentMobility::Movable);
			It->SetActorRotation(FRotator(-48.f, 35.f, 0.f));
			Sun->SetLightColor(FLinearColor(1.f, 0.9f, 0.78f));
			Sun->SetIntensity(Sun->Intensity * 0.9f);
		}
	}
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		if (USkyLightComponent* Sky = It->GetLightComponent())
		{
			Sky->SetMobility(EComponentMobility::Movable);
			Sky->SetIntensity(Sky->Intensity * 1.4f);
			Sky->SetLightColor(FLinearColor(1.f, 0.9f, 1.f));   // a hint of pink in the fill
			Sky->RecaptureSky();
		}
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APostProcessVolume* PPV = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform::Identity, Params);
	if (!PPV) { return; }
	PPV->Tags.Add(TEXT("JamGrade"));
	PPV->bUnbound = true;
	PPV->Priority = 10.f;
	FPostProcessSettings& S = PPV->Settings;
	S.bOverride_WhiteTemp = true;            S.WhiteTemp = 6700.f;          // above 6500 reads warmer
	S.bOverride_ColorSaturation = true;      S.ColorSaturation = FVector4(1.05f, 1.05f, 1.05f, 1.05f);
	S.bOverride_ColorContrast = true;        S.ColorContrast = FVector4(1.1f, 1.1f, 1.1f, 1.1f);
	S.bOverride_ColorOffsetShadows = true;   S.ColorOffsetShadows = FVector4(0.012f, 0.f, 0.02f, 0.f);   // violet shadows
	S.bOverride_ColorGainHighlights = true;  S.ColorGainHighlights = FVector4(1.03f, 1.f, 0.96f, 1.f);  // warm highlights
	S.bOverride_AutoExposureBias = true;    S.AutoExposureBias = -0.2f;     // the candy-green floor was glaring
	S.bOverride_BloomIntensity = true;       S.BloomIntensity = 0.9f;
	S.bOverride_VignetteIntensity = true;    S.VignetteIntensity = 0.5f;
	S.bOverride_FilmGrainIntensity = true;   S.FilmGrainIntensity = 0.12f;   // it is a film
	UE_LOG(LogJam, Log, TEXT("JamLook: grade and lights applied"));
}

void JamLook::BudgetOut(UWorld* World)
{
	if (APostProcessVolume* PPV = World ? FindGrade(World) : nullptr)
	{
		FPostProcessSettings& S = PPV->Settings;
		S.ColorSaturation = FVector4(0.55f, 0.55f, 0.55f, 0.55f);
		S.WhiteTemp = 6500.f;
		S.ColorOffsetShadows = FVector4(0.f, 0.f, 0.f, 0.f);
		S.ColorGainHighlights = FVector4(1.f, 1.f, 1.f, 1.f);
		S.VignetteIntensity = 0.f;
		S.FilmGrainIntensity = 0.f;
		S.BloomIntensity = 0.3f;
	}
}
