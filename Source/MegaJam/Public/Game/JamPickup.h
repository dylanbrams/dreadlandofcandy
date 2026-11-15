#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamPickup.generated.h"

class UStaticMeshComponent;

/** Something to gather on a Collect stage (almonds). No logic: the game mode checks range and collects it. */
UCLASS()
class MEGAJAM_API AJamPickup : public AActor
{
	GENERATED_BODY()

public:
	AJamPickup();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	FLinearColor Colour = FLinearColor(0.55f, 0.35f, 0.18f);   // almond brown
	FString ArtMesh = TEXT("SM_Almond");

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Art;

	FTransform ArtBaseTransform;
	float AnimationTime = 0.f;
	float AnimationPhase = 0.f;
};
