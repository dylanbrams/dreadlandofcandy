#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemInterface.h"
#include "Film/JamDignity.h"
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
	Charge,     // R4b: winds up, then runs at the hero; a hit or a timeout ends it
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
	/** Open mode (R4): keep this height above the ground under it (traced each step); negative = keep Z. */
	float GroundOffset = -1.f;

	/** BUILD_SPEC section 10 (rat): speed 300, flees inside 500 uu. */
	float Speed = 300.f;
	float FleeRadius = 500.f;
	float WanderSpeedScale = 0.4f;
	float FlusteredSeconds = 0.6f;

	// Set by the game mode before FinishSpawning (per kind, BUILD_SPEC 10).
	float MaxResolveValue = 100.f;
	FLinearColor BaseColour = FLinearColor(1.0f, 0.35f, 0.6f);
	/** How much further away a scaled-up nuisance can be treated from (its extra radius over scale 1). */
	float ExtraReach() const
	{
		const float S = GetActorScale3D().X;
		return S > 1.f ? GetSimpleCollisionRadius() * (S - 1.f) / S : 0.f;
	}
	/** A typed note's tint (T12b): wandering and fleeing models wash in BaseColour at this opacity. 0 = plain. */
	float NoteTintOpacity = 0.f;
	/** Changes heading often while wandering (bugs). */
	bool bErratic = false;
	/** The generated model for this kind and its height (T20); empty keeps the placeholder. */
	FString ArtMesh;
	float ArtHeight = 80.f;

	/** R4b: how it hits back (bAttacks false for rats and almonds). Set by the game mode before FinishSpawning. */
	FJamAttackTuning Attack;
	bool IsCharging() const { return State == EJamNuisanceState::Charge; }
	/** Times it connected (autoplay reports it). */
	int32 GetHitCount() const { return HitCount; }

	/** Spawns already sitting (retakes of a treat stage). */
	bool bStartPacified = false;

	/** Form A Line: stop (Flustered) for at least this long. Ignored once pacified. */
	void Fluster(float Seconds);
	/** A skill hit it (T19): a short shove away from the hero and a squash. Cosmetic; the set bounds still hold. */
	void Recoil(const FVector& From);

	/** The budget ran out (T21): the body becomes a grey box with its kind stencilled on top. */
	void BecomeBox(const FString& Label);

	/** The Disappointed Sigh: move at this fraction of speed for a while. */
	void Slow(float Factor, float Seconds);

	/** Treated back to Pacified: a retake of a treat stage. */
	void ResetToPacified();

	/** Called by the game mode only (it validated the request). Returns false if not pacified. */
	bool Treat();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	friend class FJamNuisancePoseTest;
	void ApplyBodySquash(float Squash);
	void SetState(EJamNuisanceState NewState);
	void HandlePacified(AActor* Pacified);
	void HandleResolveChanged(const FOnAttributeChangeData& Data);
	void PickWanderTarget();
	bool TickMunch(float DeltaSeconds);
	const AActor* FindNearestHero(float& OutDistance) const;
	void Steer(const FVector2D& DesiredVelocity, float DeltaSeconds);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UJamAbilitySystemComponent> AbilitySystem;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UTextRenderComponent> BoxLabel;

	/** Fixes3 #14: a small bobbing arrow while it sits waiting to be treated (brushed or fed). */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> TreatArrow;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> BoxMesh;

	/** On the generated model, the state colour is a translucent shell over it (the model has no tint). */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StateOverlay;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> OverlayBase;
	void UpdateStateOverlay();

	UPROPERTY()
	TObjectPtr<UJamResolveAttributeSet> ResolveSet;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	EJamNuisanceState State = EJamNuisanceState::Wander;
	int32 FleeCount = 0;
	int32 HitCount = 0;
	float AttackCooldownLeft = 1.5f;   // a moment's grace after spawning
	void TickCharge(const AActor* Hero, float HeroDistance, float DeltaSeconds);
	void EndCharge();
	float FlusterHold = 0.f;     // this Flustered lasts this long (a hit: FlusteredSeconds; the line: longer)
	float SlowFactor = 1.f;
	FVector2D RecoilVelocity = FVector2D::ZeroVector;
	float RecoilAt = -100.f;
	float SlowUntil = -1.f;
	float StateTime = 0.f;
	FVector2D WanderTarget = FVector2D::ZeroVector;
	// Fixes3 #4: bugs go for food. Between munches they head for the nearest pickup, then stop and chew.
	float NextMunchAt = 0.f;
	float MunchUntil = -1.f;
	bool bHeadingForFood = false;
	float HeadingForFoodSince = 0.f;
	TWeakObjectPtr<AActor> FoodTarget;
	FVector BodyBaseScale = FVector::OneVector;
	FVector BodyBaseLocation = FVector::ZeroVector;
};
