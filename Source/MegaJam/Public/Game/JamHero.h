#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "JamHero.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * The knight. Avatar for the PlayerState's ability system. Fixed three-quarter camera (BUILD_SPEC 11.6),
 * moves relative to the camera, faces where it walks. Input actions and the mapping context are created
 * in C++ at runtime, so there are no input assets to cook.
 */
UCLASS()
class MEGAJAM_API AJamHero : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AJamHero();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Validates and casts the Lecture. Input and the autoplay bot both come through here. */
	bool RequestLecture();

	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

private:
	void CreateInput();
	void OnMove(const FInputActionValue& Value);
	void OnLecture(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	/** Placeholder body and a visor block that shows facing, until the Meshy hero is imported. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Visor;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> InputContext;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> LectureAction;
};
