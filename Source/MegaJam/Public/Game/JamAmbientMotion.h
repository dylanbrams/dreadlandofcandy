#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamAmbientMotion.generated.h"

/** Small, bounded pool of recycled rooftop steam puffs. No collision or gameplay state. */
UCLASS()
class MEGAJAM_API AJamAmbientMotion : public AActor
{
	GENERATED_BODY()
public:
	AJamAmbientMotion();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
private:
	UPROPERTY() TObjectPtr<class UStaticMesh> PuffMesh;
	UPROPERTY() TObjectPtr<class UMaterialInterface> PuffMaterial;
	UPROPERTY() TArray<TObjectPtr<class UStaticMeshComponent>> Puffs;
	UPROPERTY() TArray<TObjectPtr<class UMaterialInstanceDynamic>> Materials;
	TArray<FVector> Sources;
	UPROPERTY() TArray<TObjectPtr<class UStaticMeshComponent>> Fireflies;
	TArray<FVector> FireflyOrigins;
	UPROPERTY() TObjectPtr<class UStaticMeshComponent> BakerySign;
	UPROPERTY() TArray<TObjectPtr<class UStaticMeshComponent>> FlourPuffs;
	UPROPERTY() TArray<TObjectPtr<class UMaterialInstanceDynamic>> FlourMaterials;
	FVector BakeryFront = FVector::ZeroVector;
	FVector SignPivot = FVector::ZeroVector;
	float Age = 0.f;
};
