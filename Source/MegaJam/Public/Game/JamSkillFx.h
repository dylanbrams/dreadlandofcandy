#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamSkillFx.generated.h"

class UProceduralMeshComponent;
class UMaterialInstanceDynamic;

/** The flat shape a skill covers, for its flash. */
enum class EJamFxShape : uint8 { Fan, Strip, Disc };

/**
 * One skill flash (T19): a flat translucent shape on the ground matching the area the skill really hits, which
 * expands for a moment and fades. Cosmetic only; spawned by the hero when a cast succeeds, destroys itself.
 * Material /Game/FX/M_FxUnlit (Scripts/make_fx_material.py); without it the flash is skipped.
 */
UCLASS()
class MEGAJAM_API AJamSkillFx : public AActor
{
	GENERATED_BODY()

public:
	AJamSkillFx();

	/** Radius: fan and disc radius, strip length. Width: fan half-angle in degrees, strip half-width. */
	static AJamSkillFx* Spawn(UWorld* World, EJamFxShape Shape, const FVector& Origin, const FVector& Forward,
		float Radius, float Width, const FLinearColor& Colour, float Seconds);

	virtual void Tick(float DeltaSeconds) override;

private:
	void Build(EJamFxShape Shape, float Radius, float Width);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> Mesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Material;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> BaseMaterial;

	float Age = 0.f;
	float Duration = 0.4f;
	float PeakOpacity = 0.7f;
};
