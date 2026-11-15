#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamComicCard.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UTexture2D;

/**
 * Fixes5 #1: one multilingual comic action card (MANNERS!, BRUSH, VITAMINS...) flying off an attack in the world.
 * Starts flat to the camera so it reads, then tumbles in 3D while it flies outward, pops in and fades out.
 * Cosmetic only: no collision, destroys itself.
 */
UCLASS(NotPlaceable)
class MEGAJAM_API AJamComicCard : public AActor
{
	GENERATED_BODY()

public:
	AJamComicCard();

	/** Call right after spawning. Velocity in uu/s; Delay holds it hidden before it pops. */
	void Launch(UTexture2D* Card, const FVector& Velocity, float Delay, float Life, float Size);

	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Quad;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Mid;

	FVector Start = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	FVector TumbleAxis = FVector::ForwardVector;   // in the card's own frame
	float TumbleRate = 0.f;   // deg/s once it starts tumbling
	float RollRate = 0.f;     // in-plane spin, deg/s, from the start
	float Age = 0.f;
	float Delay = 0.f;
	float Life = 1.2f;
	float Size = 120.f;
};
