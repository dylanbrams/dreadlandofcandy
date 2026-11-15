#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamQuestMarker.generated.h"

/** Candy cane pointer: the visible marker nearest the compass target is red/gold and animated. */
UCLASS()
class MEGAJAM_API AJamQuestMarker : public AActor
{
	GENERATED_BODY()
public:
	AJamQuestMarker();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	void PointAt(const FVector& Ground, AActor* InAnchor = nullptr, float Height = 300.f);
	static AJamQuestMarker* FindActive(UWorld* World);
	/** Feedback6 C1/C2: a throwaway copy that plays out where the real marker was, then destroys itself.
	 *  bSpin = the turn-in (spins, then shrinks away); otherwise the arrival (scales up and fades). */
	static AJamQuestMarker* SpawnFarewell(UWorld* World, const FVector& Where, bool bSpin);
private:
	bool bFarewell = false;
	bool bFarewellSpin = false;
	float FarewellAge = 0.f;
	void TickFarewell(float DeltaSeconds);
	FVector TargetLocation() const;
	UPROPERTY() TObjectPtr<class USceneComponent> Pivot;
	UPROPERTY() TArray<TObjectPtr<class UStaticMeshComponent>> Bands;
	UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> RedMaterial;
	UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> PaleMaterial;
	UPROPERTY() TObjectPtr<class UMaterialInterface> BaseMaterial;
	TWeakObjectPtr<AActor> Anchor;
	FVector Base = FVector::ZeroVector;
	float Lift = 300.f;
	float Age = 0.f;
	bool bActiveStyle = false;
};
