#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamConfetti.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UStaticMesh;
class UMaterialInterface;

/**
 * Feedback6 C2: a one-shot confetti burst (pink, mint and gold flecks) that flies up, tumbles and falls.
 * Cosmetic only: no collision, destroys itself.
 */
UCLASS(NotPlaceable)
class MEGAJAM_API AJamConfetti : public AActor
{
	GENERATED_BODY()

public:
	AJamConfetti();

	/** Call right after spawning, at the spot to burst from. */
	void Burst(int32 Count = 36, float InLife = 1.8f);

	virtual void Tick(float DeltaSeconds) override;

private:
	static constexpr int32 Groups = 3;
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Flecks;   // one per piece (the unlit FX material is not flagged for instancing)
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

	UPROPERTY(Transient) TObjectPtr<UStaticMesh> CubeMesh;
	UPROPERTY(Transient) TObjectPtr<UMaterialInterface> FxMaterial;
	struct FPiece { FVector Position; FVector Velocity; FRotator Rotation; FRotator Spin; float Size; };
	TArray<FPiece> Pieces;
	float Age = 0.f;
	float Life = 1.8f;
};
