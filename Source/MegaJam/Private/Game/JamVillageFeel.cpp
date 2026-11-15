// Feedback6 Group C: village quest feedback (arrival pop, turn-in confetti and spin, level-up heart).
// New functions only; JamVillageFlow.cpp calls them at three points.
#include "Game/JamGameMode.h"
#include "Game/JamComicCard.h"
#include "Game/JamConfetti.h"
#include "Game/JamQuestMarker.h"
#include "Game/JamVillager.h"
#include "Game/JamHero.h"
#include "Game/JamSfx.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "MegaJam.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
	/** A heart drawn into a transient texture (pink with a white rim), so no asset import is needed. */
	UTexture2D* HeartTexture()
	{
		static TStrongObjectPtr<UTexture2D> Cached;
		if (Cached.IsValid()) { return Cached.Get(); }
		constexpr int32 Size = 128;
		UTexture2D* Texture = UTexture2D::CreateTransient(Size, Size, PF_B8G8R8A8);
		if (!Texture) { return nullptr; }
		FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
		uint8* Pixels = static_cast<uint8*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
		for (int32 Y = 0; Y < Size; ++Y)
		{
			for (int32 X = 0; X < Size; ++X)
			{
				// Heart curve: (x^2 + y^2 - 1)^3 - x^2 y^3 <= 0, x and y in about -1.3..1.3.
				const float U = (X + 0.5f) / Size * 2.6f - 1.3f;
				const float V = 1.2f - (Y + 0.5f) / Size * 2.6f + 0.05f;
				const float F = FMath::Pow(U * U + V * V - 1.f, 3.f) - U * U * V * V * V;
				const float Inside = FMath::Clamp(-F / 0.05f + 0.5f, 0.f, 1.f);
				const float Rim = FMath::Clamp(1.f - (-F) / 0.07f, 0.f, 1.f);   // near the edge: white
				uint8* Pixel = Pixels + (Y * Size + X) * 4;
				Pixel[0] = (uint8)FMath::Lerp(140.f, 255.f, Rim);   // B
				Pixel[1] = (uint8)FMath::Lerp(70.f, 255.f, Rim);    // G
				Pixel[2] = (uint8)FMath::Lerp(255.f, 255.f, Rim);   // R
				Pixel[3] = (uint8)(Inside * 255.f);
			}
		}
		Mip.BulkData.Unlock();
		Texture->UpdateResource();
		Cached.Reset(Texture);
		return Texture;
	}
}

void AJamGameMode::FeelDemo()
{
	UE_LOG(LogJam, Log, TEXT("JamFeelDemo: hero %d villagers %d"), Hero.IsValid(), Villagers.Num());
	if (!Hero.IsValid() || Villagers.IsEmpty()) { return; }
	for (int32 i = 0; i < Villagers.Num(); ++i) { PlayTurnInFeel(i, true); }
	PlayFinisherFeel();
}

void AJamGameMode::PlayVisitArrivalFeel()
{
	// C1: the marker at the camp pops (scale up, fade) as the step finishes. The arrival chime is Quest_Accept (played by the caller).
	if (QuestMarker && !QuestMarker->IsHidden()) { AJamQuestMarker::SpawnFarewell(GetWorld(), QuestMarker->GetActorLocation(), false); }
}

void AJamGameMode::PlayTurnInFeel(int32 GiverIndex, bool bLevelUp)
{
	if (!Villagers.IsValidIndex(GiverIndex) || !Villagers[GiverIndex]) { return; }
	const FVector Over = Villagers[GiverIndex]->GetActorLocation() + FVector(0.f, 0.f, 230.f);
	// C2: confetti over the giver, and their marker spins before it hides.
	if (AJamConfetti* Confetti = GetWorld()->SpawnActor<AJamConfetti>(AJamConfetti::StaticClass(), Over, FRotator::ZeroRotator))
	{
		Confetti->Burst(bLevelUp ? 54 : 36, 1.8f);
	}
	if (VillagerMarkers.IsValidIndex(GiverIndex) && VillagerMarkers[GiverIndex] && !VillagerMarkers[GiverIndex]->IsHidden())
	{
		AJamQuestMarker::SpawnFarewell(GetWorld(), VillagerMarkers[GiverIndex]->GetActorLocation(), true);
	}
	// C3: a villager level-up sends a heart card flying up over them (the comic card actor, a drawn heart texture).
	if (bLevelUp)
	{
		if (UTexture2D* Heart = HeartTexture())
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AJamComicCard* Card = GetWorld()->SpawnActor<AJamComicCard>(AJamComicCard::StaticClass(), Over - FVector(0.f, 0.f, 60.f), FRotator::ZeroRotator, Params))
			{
				Card->Launch(Heart, FVector(0.f, 0.f, 520.f), 0.15f, 1.8f, 170.f);
			}
		}
		JamSfx::Play(this, TEXT("LevelUp"), 0.1f);
	}
}
