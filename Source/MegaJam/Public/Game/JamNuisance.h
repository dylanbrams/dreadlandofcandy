#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "JamNuisance.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UJamAbilitySystemComponent;
class UJamResolveAttributeSet;
struct FOnAttributeChangeData;

UENUM()
enum class EJamNuisanceState : uint8
{
	Wander,     // drifts between random points in its set
	Flee,       // runs from a hero inside FleeRadius
	Flustered,  // brief stop after a hit that did not pacify
	Pacified,   // sits, waits for treatment
	Treated,    // done
};

/**
 * Something in the candy village that needs a stern word. Has Resolve instead of health; at zero it is
 * Pacified (tag, once) and sits until a hero treats it. Steers without a navmesh: it stays inside its set's
 * bounds (a flat box) and slides along the edges. Spawned and reset only by the game mode.
 */
UCLASS()
class MEGAJAM_API AJamNuisance : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AJamNuisance();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UJamResolveAttributeSet* GetResolveSet() const { return ResolveSet; }
	float GetResolve() const;

	EJamNuisanceState GetState() const { return State; }
	bool IsPacified() const { return State == EJamNuisanceState::Pacified; }
	bool IsTreated() const { return State == EJamNuisanceState::Treated; }

	/** Times it started fleeing (autoplay reports it, to prove Flee happened). */
	int32 GetFleeCount() const { return FleeCount; }

	/** The set it lives in (world XY). Set by the game mode at spawn. */
	FBox2D SetBounds = FBox2D(FVector2D(-1000.f), FVector2D(1000.f));

	/** BUILD_SPEC section 10 (rat): speed 300, flees inside 500 uu. */
	float Speed = 300.f;
	float FleeRadius = 500.f;
	float WanderSpeedScale = 0.4f;
	float FlusteredSeconds = 0.6f;

	/** Called by the game mode only (it validated the request). Returns false if not pacified. */
	bool Treat();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void SetState(EJamNuisanceState NewState);
	void HandlePacified(AActor* Pacified);
	void HandleResolveChanged(const FOnAttributeChangeData& Data);
	void PickWanderTarget();
	const AActor* FindNearestHero(float& OutDistance) const;
	void Steer(const FVector2D& DesiredVelocity, float DeltaSeconds);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UJamAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<UJamResolveAttributeSet> ResolveSet;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	EJamNuisanceState State = EJamNuisanceState::Wander;
	int32 FleeCount = 0;
	float StateTime = 0.f;
	FVector2D WanderTarget = FVector2D::ZeroVector;
	FVector BodyBaseScale = FVector::OneVector;
	FVector BodyBaseLocation = FVector::ZeroVector;
};
