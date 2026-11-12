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

	FLinearColor Colour = FLinearColor(0.55f, 0.35f, 0.18f);   // almond brown

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Body;
};
