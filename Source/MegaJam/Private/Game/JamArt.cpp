#include "Game/JamArt.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

bool JamArt::Enabled()
{
	static const bool bEnabled = !FParse::Param(FCommandLine::Get(), TEXT("noart"));
	return bEnabled;
}

UStaticMesh* JamArt::Load(const FString& Name)
{
	if (!Enabled() || Name.IsEmpty()) { return nullptr; }
	static TMap<FString, TWeakObjectPtr<UStaticMesh>> Cache;
	if (const TWeakObjectPtr<UStaticMesh>* Hit = Cache.Find(Name)) { if (Hit->IsValid()) { return Hit->Get(); } }
	const FString Path = FString::Printf(TEXT("/Game/Art/Meshes/%s/StaticMeshes/%s.%s"), *Name, *Name, *Name);
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
	Cache.Add(Name, Mesh);
	return Mesh;
}

bool JamArt::Apply(UStaticMeshComponent* Comp, const FString& Name, float Height, float GroundZ, float Yaw)
{
	UStaticMesh* Mesh = Comp ? Load(Name) : nullptr;
	if (!Mesh) { return false; }
	const FBox Box = Mesh->GetBoundingBox();
	const float Tall = FMath::Max(Box.GetSize().Z, 1.f);
	const float Scale = Height / Tall;
	Comp->EmptyOverrideMaterials();
	Comp->SetStaticMesh(Mesh);
	const FRotator Turn(0.f, ForwardFix(Name) + Yaw, 0.f);
	const FVector Centre = Turn.RotateVector(FVector(Box.GetCenter().X, Box.GetCenter().Y, 0.f) * Scale);
	Comp->SetRelativeRotation(Turn);
	Comp->SetRelativeScale3D(FVector(Scale));
	Comp->SetRelativeLocation(FVector(-Centre.X, -Centre.Y, GroundZ - Box.Min.Z * Scale));
	return true;
}

const TArray<FString>& JamArt::AllNames()
{
	static const TArray<FString> Names = {
		TEXT("SM_Hero"), TEXT("SM_Sword"), TEXT("SM_Fenella"), TEXT("SM_Rat"), TEXT("SM_Goblin"), TEXT("SM_LollyBug"), TEXT("SM_Almond"),
		TEXT("SM_Villager"), TEXT("SM_Celery"), TEXT("SM_Toothbrush"), TEXT("SM_HouseFront"), TEXT("SM_CandyTree"), TEXT("SM_CandyCrop"),
		TEXT("SM_CandyFence"), TEXT("SM_Boulder"), TEXT("SM_Signpost"), TEXT("SM_FortGate"), TEXT("SM_Molar"), TEXT("SM_DirectorsChair"),
		TEXT("SM_LightStand"), TEXT("SM_Clapperboard"), TEXT("SM_Stanchion"), TEXT("SM_Table"), TEXT("SM_CardboardBox"), TEXT("SM_Peppermint"),
		TEXT("SM_LollyShard"), TEXT("SM_Crook"), TEXT("SM_LeafMegaphone"), TEXT("SM_Hat_Straw"), TEXT("SM_HeadMirror") };
	return Names;
}

float JamArt::ForwardFix(const FString& Name)
{
	// Filled in from the -artreview lineup: how far each model is turned from facing +X.
	// The generators' glTF fronts arrive facing +Y (rat, goblin, villager in the lineup); -90 turns them to +X.
	static const TMap<FString, float> Fix = {};
	const float* F = Fix.Find(Name);
	return F ? *F : -90.f;
}

float JamArt::Height(const FString& Name)
{
	// The hero's capsule is 192 uu tall; everything else is sized against him.
	static const TMap<FString, float> Heights = {
		{ TEXT("SM_Hero"), 190.f }, { TEXT("SM_Sword"), 160.f }, { TEXT("SM_Fenella"), 90.f }, { TEXT("SM_Rat"), 80.f },
		{ TEXT("SM_Goblin"), 130.f }, { TEXT("SM_LollyBug"), 75.f }, { TEXT("SM_Almond"), 45.f }, { TEXT("SM_Villager"), 170.f },
		{ TEXT("SM_Celery"), 60.f }, { TEXT("SM_Toothbrush"), 50.f }, { TEXT("SM_HouseFront"), 650.f }, { TEXT("SM_CandyTree"), 450.f },
		{ TEXT("SM_CandyCrop"), 110.f }, { TEXT("SM_CandyFence"), 110.f }, { TEXT("SM_Boulder"), 160.f }, { TEXT("SM_Signpost"), 260.f },
		{ TEXT("SM_FortGate"), 650.f }, { TEXT("SM_Molar"), 220.f }, { TEXT("SM_DirectorsChair"), 140.f }, { TEXT("SM_LightStand"), 320.f },
		{ TEXT("SM_Clapperboard"), 50.f }, { TEXT("SM_Stanchion"), 100.f }, { TEXT("SM_Table"), 90.f }, { TEXT("SM_CardboardBox"), 70.f },
		{ TEXT("SM_Peppermint"), 60.f }, { TEXT("SM_LollyShard"), 60.f }, { TEXT("SM_Crook"), 180.f }, { TEXT("SM_LeafMegaphone"), 50.f },
		{ TEXT("SM_Hat_Straw"), 30.f }, { TEXT("SM_HeadMirror"), 30.f } };
	const float* H = Heights.Find(Name);
	return H ? *H : 100.f;
}

USkeletalMesh* JamArt::LoadHeroRig()
{
	if (!Enabled()) { return nullptr; }
	return LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Art/Hero/SK_Hero_Rig.SK_Hero_Rig"), nullptr, LOAD_NoWarn | LOAD_Quiet);
}

UAnimSequence* JamArt::LoadHeroAnim(const FString& Name)
{
	if (!Enabled()) { return nullptr; }
	const FString Asset = TEXT("A_SK_Hero_Rig_") + Name;
	return LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("/Game/Art/Hero/Anims/%s.%s"), *Asset, *Asset), nullptr, LOAD_NoWarn | LOAD_Quiet);
}
