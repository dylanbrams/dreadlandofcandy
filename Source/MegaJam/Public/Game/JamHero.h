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
class AJamNuisance;
struct FInputActionValue;

/**
 * The knight. Avatar for the PlayerState's ability system. Fixed three-quarter camera (BUILD_SPEC 11.6),
 * moves relative to the camera. Faces the mouse cursor, or the right stick, or else where it walks
 * (whichever was used last). Input actions and the mapping context are created
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

	/** Asks the game mode to treat the best interact target. Input and the autoplay bot come through here. */
	bool RequestInteract();

	/** At the CUT! prompt: refuse the note. */
	bool RequestRefuseNote();

	/** Nearest pacified nuisance inside interact range, or null. The HUD shows the prompt for it. */
	AJamNuisance* FindInteractTarget() const;

	/** Bot aiming: overrides mouse and stick until cleared (zero vector). */
	void SetAimOverride(const FVector& Direction) { AimOverride = Direction.GetSafeNormal2D(); }

	/** True once the player last used a gamepad (for prompt glyphs). */
	bool IsUsingGamepad() const { return bUsingGamepad; }

	/** Takes the progression's stats: move speed now; the Lecture reads radius and power itself. */
	void ApplyStats(const struct FJamStatBlock& Stats);

	/** 0 to 1 while interact (E / A) is held toward its 0.6 s threshold; 0 when not held. For the HUD's fill. */
	float GetInteractHoldFraction() const;

	virtual void Tick(float DeltaSeconds) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

private:
	void CreateInput();
	void OnMove(const FInputActionValue& Value);
	void OnLecture(const FInputActionValue& Value);
	void OnAim(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnRefuse(const FInputActionValue& Value);
	void OnInteractPressed(const FInputActionValue& Value);
	void OnPause(const FInputActionValue& Value);
	void OnInteractReleased(const FInputActionValue& Value);
	void UpdateFacing();

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

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> RefuseAction;

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> PauseAction;

	enum class EAimSource : uint8 { Movement, Mouse, Stick };
	EAimSource AimSource = EAimSource::Movement;
	FVector AimOverride = FVector::ZeroVector;
	FVector StickAim = FVector::ZeroVector;
	FVector2D LastMousePosition = FVector2D(-1.f);
	bool bUsingGamepad = false;
	/** World time interact was pressed; negative when not held. */
	float InteractPressedAt = -1.f;
};
