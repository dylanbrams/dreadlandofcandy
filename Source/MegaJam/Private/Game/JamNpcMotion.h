#pragma once

#include "Game/JamArt.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Animation/AnimSequence.h"

namespace JamNpcMotion
{
	// Attach a rig to the existing cosmetic body so recoil, calm poses and authored offsets stay shared.
	inline bool Load(USkeletalMeshComponent* Rig, UStaticMeshComponent* Body, const FString& Who,
		UAnimSequence*& Idle, UAnimSequence*& Walk)
	{
		if (!JamArt::Enabled() || !Body->GetStaticMesh()) { return false; }
		const FString Base = FString::Printf(TEXT("/Game/Art/NPC/%s"), *Who);
		const FString MeshName = FString::Printf(TEXT("SK_%s_Rig"), *Who);
		USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, *(Base / MeshName + TEXT(".") + MeshName), nullptr, LOAD_NoWarn | LOAD_Quiet);
		Idle = LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("%s/Anims/A_SK_%s_Rig_Idle.A_SK_%s_Rig_Idle"), *Base, *Who, *Who), nullptr, LOAD_NoWarn | LOAD_Quiet);
		Walk = LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("%s/Anims/A_SK_%s_Rig_Walk.A_SK_%s_Rig_Walk"), *Base, *Who, *Who), nullptr, LOAD_NoWarn | LOAD_Quiet);
		if (!Mesh || !Idle || !Walk || Idle->GetSkeleton() != Mesh->GetSkeleton() || Walk->GetSkeleton() != Mesh->GetSkeleton()) { return false; }
		const FBox StaticBox = Body->GetStaticMesh()->GetBoundingBox();
		const FBox RigBox = Mesh->GetImportedBounds().GetBox();
		const float Scale = StaticBox.GetSize().Z / FMath::Max(1.f, RigBox.GetSize().Z);
		Rig->SetSkeletalMesh(Mesh);
		Rig->SetRelativeScale3D(FVector(Scale));
		Rig->SetRelativeLocation(FVector(StaticBox.GetCenter().X - RigBox.GetCenter().X * Scale,
			StaticBox.GetCenter().Y - RigBox.GetCenter().Y * Scale, StaticBox.Min.Z - RigBox.Min.Z * Scale));
		Rig->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		Rig->PlayAnimation(Idle, true);
		Rig->SetHiddenInGame(false);
		Body->SetHiddenInGame(true, false);
		return true;
	}
}
