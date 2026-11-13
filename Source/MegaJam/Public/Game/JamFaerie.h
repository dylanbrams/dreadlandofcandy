#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamFaerie.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Fenella Flicker, director and camera operator (BUILD_SPEC 5B). Cosmetic only: hovers at the hero's shoulder,
 * bobs, flaps, points her camcorder at whatever is worth filming. No collision, no skeleton, never touches rules.
 * Placeholder body from basic shapes until SM_Fenella is imported (T20).
 */
UCLASS()
class MEGAJAM_API AJamFaerie : public AActor
{
	GENERATED_BODY()

public:
	AJamFaerie();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Whom she follows. */
	void SetFollow(AActor* InFollow) { Follow = InFollow; }
	/** What she films; null = the hero. */
	void SetSubject(AActor* InSubject) { Subject = InSubject; }
	/** After a refused note: she turns her back and films something else. */
	void SetSulking(bool bInSulking) { bSulking = bInSulking; }
	/** Talking: bobs and flaps faster for a moment. */
	void Speak(float Seconds);

private:
	UStaticMeshComponent* MakePart(const TCHAR* Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale);
	void Tint(UStaticMeshComponent* Part, const FLinearColor& Colour);

	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Root;
	UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> Pivot;   // bob and turn
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Head;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> WingL;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> WingR;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Camcorder;
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Tally;   // the red recording light
	UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Art;     // the generated Fenella (T20)
	/** Her key light on the hero (Dylan, Nov 13: the black knight vanished against the grass). */
	UPROPERTY(VisibleAnywhere) TObjectPtr<class USpotLightComponent> Spot;
	bool bArt = false;
	float ArtBaseZ = 0.f;

	UPROPERTY(Transient) TObjectPtr<UMaterialInterface> ShapeMaterial;

	TWeakObjectPtr<AActor> Follow;
	TWeakObjectPtr<AActor> Subject;
	float Age = 0.f;
	float SpeakUntil = -1.f;
	bool bSulking = false;
};
