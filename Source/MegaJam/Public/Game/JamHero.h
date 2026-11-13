#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Film/JamProgression.h"
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

	/**
	 * A skill button (LMB / RMB / Q / R, or RT / RB / LB / Y). While the wrap panel is up it spends a skill point on
	 * that skill instead (BUILD_SPEC 3c); otherwise it casts. Input and the bot both come through here.
	 */
	bool RequestSkill(EJamSkill Skill);

	/** Asks the game mode to treat the best interact target. Input and the autoplay bot come through here. */
	bool RequestInteract();

	/** At the CUT! prompt: refuse the note. */
	bool RequestRefuseNote();
	/** CUT!: open the note editor to write her note yourself (T, or Y / R, which do nothing else at CUT!). */
	bool RequestWriteNote();

	/** Nearest pacified nuisance inside interact range, or null. The HUD shows the prompt for it. */
	AJamNuisance* FindInteractTarget() const;

	/** Bot aiming: overrides mouse and stick until cleared (zero vector). */
	void SetAimOverride(const FVector& Direction) { AimOverride = Direction.GetSafeNormal2D(); }

	/** True once the player last used a gamepad (for prompt glyphs). */
	bool IsUsingGamepad() const { return bUsingGamepad; }

	/** Pulls the fixed camera back (the -artreview lineup). */
	void SetCameraDistance(float Distance);

	/**
	 * Open mode (R4, BUILD_SPEC 3e): a third-person follow camera. Mouse / right stick turn it, movement is relative
	 * to its yaw, the hero faces where he walks and turns to the camera's yaw when he casts. SetAimOverride also
	 * turns it, so the bot films what it does. Backlot keeps the fixed camera.
	 */
	void EnableFollowCamera(float Yaw);
	bool IsFollowCamera() const { return bFollowCamera; }
	/** Open mode: wide and higher while a scene shoots (the set reads), close and low while walking. */
	void SetFollowFraming(bool bScene, float SceneYaw = 0.f);
	float GetCameraYaw() const { return CameraYaw; }
	/** -camyaw / -campitch pin the follow camera for repeatable screenshots. */
	void PinFollowCamera(float Yaw, float Pitch) { CameraYaw = Yaw; CameraPitchFollow = Pitch; bCameraPinned = true; }

	/** Takes the progression's stats: move speed now; the Lecture reads radius and power itself. */
	void ApplyStats(const struct FJamStatBlock& Stats);

	/** 0 to 1 while interact (E / A) is held toward its 0.6 s threshold; 0 when not held. For the HUD's fill. */
	float GetInteractHoldFraction() const;

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

private:
	void CreateInput();
	void OnMove(const FInputActionValue& Value);
	void OnLecture(const FInputActionValue& Value);
	void OnAim(const FInputActionValue& Value);
	void OnLook(const FInputActionValue& Value);
	void TickFollowCamera(float DeltaSeconds);
	bool bFollowCamera = false;
	bool bCameraPinned = false;
	bool bSceneFraming = false;   // a take: the camera looks along the path into the set and the bot does not swing it
	float CameraYaw = 0.f;
	float CameraPitchFollow = -14.f;
	FVector2D StickLook = FVector2D::ZeroVector;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> LookAction;
	void OnInteract(const FInputActionValue& Value);
	void OnRefuse(const FInputActionValue& Value);
	void OnInteractPressed(const FInputActionValue& Value);
	void OnPause(const FInputActionValue& Value);
	void OnSkill2(const FInputActionValue& Value);
	void OnSkill3(const FInputActionValue& Value);
	void OnSkill4(const FInputActionValue& Value);
	void DrawSkillShape(EJamSkill Skill, const struct FJamSkillRank& Numbers) const;
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

	/** The sword (T19): raised for the Lecture, never swung. Placeholder cubes until SM_Sword is imported. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SwordPivot;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SwordBlade;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> SwordGuard;

	// The generated hero and sword (T20); the shapes above hide when these load.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArtBody;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ArtSword;
	/** The rigged hero (T20): replaces ArtBody when it loads; plays one animation at a time by state. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> ArtRig;
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UAnimSequence>> RigAnims;
	FName RigPlaying;
	float RigOneShotUntil = -1.f;
	/** Plays a one-off (SwordRaise, Sigh) over the idle/walk choice until it ends. */
	void PlayRigOneShot(FName Anim);
	void TickRig();
	FVector ArtBodyScale = FVector::OneVector;
	FVector ArtBodyLocation = FVector::ZeroVector;
	FRotator ArtBodyRotation = FRotator::ZeroRotator;
	float WalkPhase = 0.f;
	float SwordUpAt = -100.f;   // world time of the last Lecture
	void TickSword();

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

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> WriteAction;
	void OnWrite();

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> SkillActions[3];

	enum class EAimSource : uint8 { Movement, Mouse, Stick };
	EAimSource AimSource = EAimSource::Movement;
	FVector AimOverride = FVector::ZeroVector;
	FVector StickAim = FVector::ZeroVector;
	FVector2D LastMousePosition = FVector2D(-1.f);
	bool bUsingGamepad = false;
	/** World time interact was pressed; negative when not held. */
	float InteractPressedAt = -1.f;
};
