#include "Game/JamSfx.h"
#include "Game/JamSettings.h"
#include "MegaJam.h"
#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"

namespace
{
	FJamSfxCue Cue(const TCHAR* Name, float Volume, float Jitter = 0.f, float MinInterval = 0.f)
	{
		FJamSfxCue C;
		C.Event = Name;
		C.Clip = Name;
		C.Volume = Volume;
		C.PitchJitter = Jitter;
		C.MinInterval = MinInterval;
		return C;
	}

	TSet<FName>& MissingClips() { static TSet<FName> Missing; return Missing; }
	TMap<FName, double>& LastPlayed() { static TMap<FName, double> Last; return Last; }

	USoundBase* LoadClip(const FJamSfxCue& C)
	{
		USoundBase* Clip = LoadObject<USoundBase>(nullptr, *JamSfx::AssetPath(C.Clip), nullptr, LOAD_NoWarn | LOAD_Quiet);
		if (!Clip && !MissingClips().Contains(C.Clip))
		{
			MissingClips().Add(C.Clip);
			UE_LOG(LogJam, Warning, TEXT("JamSfx: missing clip %s"), *JamSfx::AssetPath(C.Clip));
		}
		return Clip;
	}

	void PlayNow(UWorld* World, const FJamSfxCue& C)
	{
		if (!World || World->GetNetMode() == NM_DedicatedServer) { return; }
		const double Now = FPlatformTime::Seconds();
		if (C.MinInterval > 0.f)
		{
			if (const double* Last = LastPlayed().Find(C.Event); Last && Now - *Last < C.MinInterval) { return; }
			LastPlayed().Add(C.Event, Now);
		}
		USoundBase* Clip = LoadClip(C);
		if (!Clip) { return; }
		const float Volume = C.Volume * JamSfx::EffectsVolume(World);
		if (Volume <= 0.f) { return; }
		const float Pitch = 1.f + (C.PitchJitter > 0.f ? FMath::FRandRange(-C.PitchJitter, C.PitchJitter) : 0.f);
		UGameplayStatics::PlaySound2D(World, Clip, Volume, Pitch);
		UE_LOG(LogJam, Verbose, TEXT("JamSfx: %s"), *C.Event.ToString());
	}
}

const TArray<FJamSfxCue>& JamSfx::Table()
{
	// Volumes are relative; the mix is loud on purpose. UI and the ambience bed sit under the gags.
	static const TArray<FJamSfxCue> Cues = {
		Cue(TEXT("Sword_Raise"), 1.0f, 0.03f),
		Cue(TEXT("Hero_Inhale"), 1.0f, 0.05f),
		Cue(TEXT("Lecture_Impact"), 1.0f, 0.06f),
		Cue(TEXT("FormLine_Contact"), 1.0f, 0.05f),
		Cue(TEXT("DisappointedSigh"), 1.0f, 0.04f),
		Cue(TEXT("LastJudgment_Contact"), 1.0f),
		Cue(TEXT("Flustered"), 0.7f, 0.15f, 0.25f),
		Cue(TEXT("Pacify"), 0.9f, 0.1f, 0.15f),
		Cue(TEXT("Interact_Complete"), 0.9f, 0.08f),
		Cue(TEXT("Bubble_Pop"), 0.6f, 0.2f, 0.1f),
		Cue(TEXT("Clapper"), 1.0f, 0.02f),
		Cue(TEXT("Cut_Horn"), 1.0f),
		Cue(TEXT("Retake_Rewind"), 1.0f),
		Cue(TEXT("Stage_Complete"), 1.0f),
		Cue(TEXT("Ledger_Deduction"), 0.8f, 0.1f),
		Cue(TEXT("Coin_Trickle"), 0.9f),
		Cue(TEXT("LevelUp"), 1.0f),
		Cue(TEXT("UI_Click"), 0.5f, 0.05f, 0.05f),
		Cue(TEXT("UI_Back"), 0.5f, 0.0f, 0.05f),
		Cue(TEXT("UI_Unavailable"), 0.6f, 0.0f, 0.2f),
		Cue(TEXT("Budget_Warning"), 1.0f),
		Cue(TEXT("Budget_Empty"), 1.0f),
		Cue(TEXT("Backlot_Ambience_Loop"), 0.3f),
		Cue(TEXT("Quest_Accept"), 1.0f),
	};
	return Cues;
}

const FJamSfxCue* JamSfx::Find(FName Event)
{
	return Table().FindByPredicate([Event](const FJamSfxCue& C) { return C.Event == Event; });
}

FString JamSfx::AssetPath(FName Clip)
{
	const FString Name = FString::Printf(TEXT("SFX_%s"), *Clip.ToString());
	return FString::Printf(TEXT("/Game/Audio/SFX/%s.%s"), *Name, *Name);
}

float JamSfx::EffectsVolume(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	const UJamSettings* J = GI ? GI->GetSubsystem<UJamSettings>() : nullptr;
	return J ? J->EffectsVolume : 0.8f;
}

void JamSfx::Play(const UObject* WorldContext, FName Event, float Delay)
{
	const FJamSfxCue* C = Find(Event);
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World) { return; }
	if (!C)
	{
		UE_LOG(LogJam, Warning, TEXT("JamSfx: no table row for %s"), *Event.ToString());
		return;
	}
	if (Delay <= 0.f) { PlayNow(World, *C); return; }
	FTimerHandle Handle;
	const FJamSfxCue Copy = *C;
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(World, [World, Copy]() { PlayNow(World, Copy); }), Delay, false);
}

UAudioComponent* JamSfx::StartLoop(const UObject* WorldContext, FName Event)
{
	const FJamSfxCue* C = Find(Event);
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!C || !World) { return nullptr; }
	USoundBase* Clip = LoadClip(*C);
	if (!Clip) { return nullptr; }
	if (USoundWave* Wave = Cast<USoundWave>(Clip)) { Wave->bLooping = true; }
	UAudioComponent* Loop = UGameplayStatics::CreateSound2D(World, Clip, FMath::Max(C->Volume * EffectsVolume(World), 0.001f), 1.f, 0.f, nullptr, false, true);
	if (Loop) { Loop->Play(); }
	return Loop;
}
