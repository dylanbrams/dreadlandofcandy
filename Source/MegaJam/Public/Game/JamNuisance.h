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

/**
 * Something in the candy village that needs a stern word. Has Resolve instead of health; at zero it is
 * Pacified (tag, once) and turns pale. Spawned and reset only by the game mode.
 * T01: static placeholder (basic-shape body), no movement or states yet.
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
	bool IsPacified() const;

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

private:
	void HandlePacified(AActor* Pacified);

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
};
