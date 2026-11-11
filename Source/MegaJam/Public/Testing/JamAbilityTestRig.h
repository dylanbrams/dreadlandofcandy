#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "JamAbilityTestRig.generated.h"

class UCapsuleComponent;
class UJamAbilitySystemComponent;
class UJamResolveAttributeSet;

/**
 * Minimal pawn that owns an ASC and a Resolve set. Stands in for both the hero and a nuisance in tests
 * and in the ability test scene. Initialises its ASC in PostInitializeComponents so it works in worlds
 * that never BeginPlay (automation tests).
 */
UCLASS()
class MEGAJAM_API AJamTestCombatant : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AJamTestCombatant();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UJamResolveAttributeSet* GetResolveSet() const { return ResolveSet; }
	float GetResolve() const;

	/** Team tag added to the ASC at spawn. Set before FinishSpawning. */
	FGameplayTag TeamTag;

	virtual void PostInitializeComponents() override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> Capsule;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UJamAbilitySystemComponent> AbilitySystem;

	UPROPERTY()
	TObjectPtr<UJamResolveAttributeSet> ResolveSet;
};

/**
 * Self-running ability test scene. A hero faces a field of nuisances; the rig casts the Lecture cone
 * three times (hit, refused by cooldown, hit again), draws the result, logs a PASS/FAIL verdict.
 * Spawned by UJamAbilityTestSubsystem when the game is run with -abilitytest.
 */
UCLASS()
class MEGAJAM_API AJamAbilityTestRig : public AActor
{
	GENERATED_BODY()

public:
	AJamAbilityTestRig();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	struct FTargetInfo
	{
		TWeakObjectPtr<AJamTestCombatant> Actor;
		bool bExpectedInCone = false;
		FString Label;
	};

	void SpawnTarget(const FVector& Offset, bool bExpectedInCone, const TCHAR* Label);
	void CastLecture(const TCHAR* Label, bool bExpectSuccess);
	void Verdict();
	void HandlePacified(AActor* Pacified);

	TWeakObjectPtr<AJamTestCombatant> Hero;
	TArray<FTargetInfo> Targets;
	FGameplayAbilitySpecHandle LectureHandle;

	float Elapsed = 0.f;
	int32 NextStep = 0;
	int32 PacifiedEvents = 0;
	int32 CastFailures = 0;
	bool bCastOneOk = false;
	bool bCastTwoRefused = false;
	bool bCastThreeOk = false;
};

UCLASS()
class MEGAJAM_API UJamAbilityTestSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};
