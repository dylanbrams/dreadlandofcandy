#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamVillager.generated.h"

UCLASS()
class MEGAJAM_API AJamVillager : public AActor
{
	GENERATED_BODY()
public:
	AJamVillager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void SetName(const FString& Name);
	void SetRole(FName Giver);
private:
	UPROPERTY() TObjectPtr<class USceneComponent> MotionPivot;
	float MotionPhase = 0.f;
	UPROPERTY() TObjectPtr<class USkeletalMeshComponent> Rig;
	UPROPERTY() TObjectPtr<class UAnimSequence> IdleAnimation;
	UPROPERTY() TObjectPtr<class UAnimSequence> WalkAnimation;
	UPROPERTY() TObjectPtr<class UStaticMeshComponent> Art;
	UPROPERTY() TObjectPtr<class UStaticMeshComponent> RoleProp;
	UPROPERTY() TObjectPtr<class UStaticMeshComponent> HatPuff;
	UPROPERTY() TObjectPtr<class UStaticMesh> CylinderMesh;
	UPROPERTY() TObjectPtr<class UMaterialInterface> ShapeMat;   // Fixes3 #11: top of the baker's chef hat
	UPROPERTY() TObjectPtr<class UTextRenderComponent> Label;
};
