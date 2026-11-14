#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamQuestMarker.generated.h"

/** A camera-facing, emissive golden triangle. Anchored markers follow their speaker. */
UCLASS()
class MEGAJAM_API AJamQuestMarker : public AActor
{
	GENERATED_BODY()
public:
	AJamQuestMarker();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void PointAt(const FVector& Ground, AActor* InAnchor = nullptr, float Height = 300.f);
private:
	UPROPERTY() TObjectPtr<class UProceduralMeshComponent> Mesh;
	UPROPERTY() TObjectPtr<class UMaterialInterface> BaseMaterial;
	TWeakObjectPtr<AActor> Anchor;
	FVector Base = FVector::ZeroVector;
	float Lift = 300.f;
	float Age = 0.f;
};
