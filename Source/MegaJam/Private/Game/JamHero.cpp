#include "Game/JamHero.h"
#include "Film/JamProgression.h"
#include "Game/JamPlayerState.h"
#include "Game/JamGameMode.h"
#include "Game/JamHUD.h"
#include "Game/JamNuisance.h"
#include "EngineUtils.h"
#include "InputTriggers.h"
#include "Gas/JamAbilitySystemComponent.h"
#include "MegaJam.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace JamHeroTuning
{
	// BUILD_SPEC 11.6: fixed three-quarter camera, about 1100 uu arm, 50 degree pitch, no player control.
	// Tuned in T01: 1100 put the hero at a third of the screen height and the nuisances off the top.
	constexpr float CameraArmLength = 1700.f;
	// Look ahead of the hero (world space, so it does not swing when the hero turns).
	const FVector CameraLookAhead(300.f, 0.f, 0.f);
	constexpr float CameraPitch = -50.f;
	// Match UJamGA_LectureCone (debug draw only).
	constexpr float WalkSpeed = 450.f;
	constexpr float LectureRange = 800.f;
	constexpr float LectureHalfAngleDeg = 35.f;
	// BUILD_SPEC section 10: hold E 0.6 s, inside 150 uu.
	constexpr float InteractRange = 150.f;
	constexpr float InteractHoldSeconds = 0.6f;
	constexpr float StickAimDeadZone = 0.35f;
}

AJamHero::AJamHero()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = false;   // UpdateFacing() turns the hero
	Move->RotationRate = FRotator(0.f, 720.f, 0.f);
	Move->MaxWalkSpeed = JamHeroTuning::WalkSpeed;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->SetUsingAbsoluteRotation(true);
	CameraArm->SetRelativeRotation(FRotator(JamHeroTuning::CameraPitch, 0.f, 0.f));
	CameraArm->TargetArmLength = JamHeroTuning::CameraArmLength;
	CameraArm->TargetOffset = JamHeroTuning::CameraLookAhead;
	CameraArm->bDoCollisionTest = false;
	CameraArm->bInheritPitch = CameraArm->bInheritYaw = CameraArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->FieldOfView = 55.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Cylinder.Succeeded()) { Body->SetStaticMesh(Cylinder.Object); }
	if (ShapeMaterial.Succeeded()) { Body->SetMaterial(0, ShapeMaterial.Object); }
	Body->SetRelativeScale3D(FVector(0.85f, 0.85f, 1.9f));

	Visor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visor"));
	Visor->SetupAttachment(RootComponent);
	Visor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Cube.Succeeded()) { Visor->SetStaticMesh(Cube.Object); }
	if (ShapeMaterial.Succeeded()) { Visor->SetMaterial(0, ShapeMaterial.Object); }
	Visor->SetRelativeLocation(FVector(45.f, 0.f, 55.f));
	Visor->SetRelativeScale3D(FVector(0.3f, 0.6f, 0.2f));
}

UAbilitySystemComponent* AJamHero::GetAbilitySystemComponent() const
{
	const AJamPlayerState* PS = GetPlayerState<AJamPlayerState>();
	return PS ? PS->GetAbilitySystemComponent() : nullptr;
}

void AJamHero::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (AJamPlayerState* PS = GetPlayerState<AJamPlayerState>())
	{
		PS->InitForAvatar(this);
	}
	// Dark knight: near-black body, steel visor.
	if (UMaterialInstanceDynamic* M = Body->CreateAndSetMaterialInstanceDynamic(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.04f, 0.04f, 0.05f));
	}
	if (UMaterialInstanceDynamic* M = Visor->CreateAndSetMaterialInstanceDynamic(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.5f, 0.5f, 0.55f));
	}
}

bool AJamHero::RequestLecture()
{
	return RequestSkill(EJamSkill::Lecture);
}

bool AJamHero::RequestSkill(EJamSkill Skill)
{
	AJamPlayerState* PS = GetPlayerState<AJamPlayerState>();
	AJamGameMode* GM = GetWorld()->GetAuthGameMode<AJamGameMode>();
	if (!PS) { return false; }
	if (GM && GM->HandleSpendPoint(this, Skill)) { return true; }   // the wrap panel: this button ranks the skill up
	if (GM && GM->IsNotePromptUp()) { return false; }                // CUT!: Q refuses the note, it does not sigh
	const bool bActivated = PS->GetAbilitySystemComponent()->TryActivateAbility(PS->GetSkillHandle(Skill));
	if (bActivated)
	{
		DrawSkillShape(Skill, PS->GetProgression().GetSkillNumbers(Skill));
		UE_LOG(LogJam, Log, TEXT("JamHero: %s cast"), JamProgression::SkillName(Skill));
		if (GM) { GM->NotifySkillCast(this, Skill); }
	}
	else { UE_LOG(LogJam, Verbose, TEXT("JamHero: %s refused"), JamProgression::SkillName(Skill)); }
	return bActivated;
}

void AJamHero::DrawSkillShape(EJamSkill Skill, const FJamSkillRank& N) const
{
	// Debug shapes until the T19 presentation: the cone, the line, the rings.
	const FVector At = GetActorLocation();
	const FVector Fwd = GetActorForwardVector();
	switch (Skill)
	{
	case EJamSkill::Lecture:
		DrawDebugCone(GetWorld(), At, Fwd, N.Radius > 0.f ? N.Radius : JamHeroTuning::LectureRange,
			FMath::DegreesToRadians(JamHeroTuning::LectureHalfAngleDeg), FMath::DegreesToRadians(JamHeroTuning::LectureHalfAngleDeg),
			24, FColor::Orange, false, 1.5f, 0, 4.f);
		break;
	case EJamSkill::FormALine:
		DrawDebugBox(GetWorld(), At + Fwd * N.Length * 0.5f, FVector(N.Length * 0.5f, N.Width * 0.5f, 60.f), GetActorQuat(), FColor::Cyan, false, 1.5f, 0, 4.f);
		break;
	default:
		DrawDebugCircle(GetWorld(), At, N.Radius, 48, Skill == EJamSkill::LastJudgment ? FColor::Red : FColor(150, 120, 255), false, 1.5f, 0, 6.f,
			FVector::ForwardVector, FVector::RightVector, false);
		break;
	}
}

void AJamHero::CreateInput()
{
	if (InputContext) { return; }

	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	LectureAction = NewObject<UInputAction>(this, TEXT("IA_Lecture"));
	LectureAction->ValueType = EInputActionValueType::Boolean;
	AimAction = NewObject<UInputAction>(this, TEXT("IA_Aim"));
	AimAction->ValueType = EInputActionValueType::Axis2D;
	InteractAction = NewObject<UInputAction>(this, TEXT("IA_Interact"));
	InteractAction->ValueType = EInputActionValueType::Boolean;
	UInputTriggerHold* Hold = NewObject<UInputTriggerHold>(InteractAction);
	Hold->HoldTimeThreshold = JamHeroTuning::InteractHoldSeconds;
	Hold->bIsOneShot = true;
	InteractAction->Triggers.Add(Hold);

	RefuseAction = NewObject<UInputAction>(this, TEXT("IA_Refuse"));
	RefuseAction->ValueType = EInputActionValueType::Boolean;
	PauseAction = NewObject<UInputAction>(this, TEXT("IA_Pause"));
	PauseAction->ValueType = EInputActionValueType::Boolean;
	for (int32 i = 0; i < 3; ++i)
	{
		SkillActions[i] = NewObject<UInputAction>(this, *FString::Printf(TEXT("IA_Skill%d"), i + 2));
		SkillActions[i]->ValueType = EInputActionValueType::Boolean;
	}

	InputContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Hero"));

	// Move value: X = right, Y = forward (screen up).
	auto MapMoveKey = [this](const FKey& Key, bool bToY, bool bNegate)
	{
		FEnhancedActionKeyMapping& Mapping = InputContext->MapKey(MoveAction, Key);
		if (bToY)
		{
			UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(InputContext);
			Swizzle->Order = EInputAxisSwizzle::YXZ;
			Mapping.Modifiers.Add(Swizzle);
		}
		if (bNegate) { Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(InputContext)); }
	};
	MapMoveKey(EKeys::W, true, false);
	MapMoveKey(EKeys::S, true, true);
	MapMoveKey(EKeys::D, false, false);
	MapMoveKey(EKeys::A, false, true);
	MapMoveKey(EKeys::Up, true, false);
	MapMoveKey(EKeys::Down, true, true);
	MapMoveKey(EKeys::Right, false, false);
	MapMoveKey(EKeys::Left, false, true);
	InputContext->MapKey(MoveAction, EKeys::Gamepad_Left2D);

	InputContext->MapKey(AimAction, EKeys::Gamepad_Right2D);

	// BUILD_SPEC 3: skill 1 on LMB / RT, interact on E / A. Space also lectures (handy on a laptop).
	InputContext->MapKey(LectureAction, EKeys::LeftMouseButton);
	InputContext->MapKey(LectureAction, EKeys::SpaceBar);
	InputContext->MapKey(LectureAction, EKeys::Gamepad_RightTrigger);
	InputContext->MapKey(InteractAction, EKeys::E);
	InputContext->MapKey(InteractAction, EKeys::Gamepad_FaceButton_Bottom);
	// At the CUT! prompt: hold E / A takes the note, Q / B refuses it (placeholder until the T12 prompt UI).
	InputContext->MapKey(RefuseAction, EKeys::Q);
	InputContext->MapKey(RefuseAction, EKeys::Gamepad_FaceButton_Right);
	InputContext->MapKey(PauseAction, EKeys::Escape);
	// Skills 2 to 4 (BUILD_SPEC 3 controls): RMB / Q / R, RB / LB / Y.
	InputContext->MapKey(SkillActions[0], EKeys::RightMouseButton);
	InputContext->MapKey(SkillActions[0], EKeys::Gamepad_RightShoulder);
	InputContext->MapKey(SkillActions[1], EKeys::Q);
	InputContext->MapKey(SkillActions[1], EKeys::Gamepad_LeftShoulder);
	InputContext->MapKey(SkillActions[2], EKeys::R);
	InputContext->MapKey(SkillActions[2], EKeys::Gamepad_FaceButton_Top);
	InputContext->MapKey(PauseAction, EKeys::Gamepad_Special_Right);
}

void AJamHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	CreateInput();
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AJamHero::OnMove);
		Input->BindAction(LectureAction, ETriggerEvent::Started, this, &AJamHero::OnLecture);
		Input->BindAction(AimAction, ETriggerEvent::Triggered, this, &AJamHero::OnAim);
		Input->BindAction(AimAction, ETriggerEvent::Completed, this, &AJamHero::OnAim);
		Input->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AJamHero::OnInteract);
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &AJamHero::OnInteractPressed);
		Input->BindAction(InteractAction, ETriggerEvent::Canceled, this, &AJamHero::OnInteractReleased);
		Input->BindAction(InteractAction, ETriggerEvent::Completed, this, &AJamHero::OnInteractReleased);
		Input->BindAction(RefuseAction, ETriggerEvent::Started, this, &AJamHero::OnRefuse);
		Input->BindAction(PauseAction, ETriggerEvent::Started, this, &AJamHero::OnPause);
		Input->BindAction(SkillActions[0], ETriggerEvent::Started, this, &AJamHero::OnSkill2);
		Input->BindAction(SkillActions[1], ETriggerEvent::Started, this, &AJamHero::OnSkill3);
		Input->BindAction(SkillActions[2], ETriggerEvent::Started, this, &AJamHero::OnSkill4);
	}
	else
	{
		UE_LOG(LogJam, Error, TEXT("JamHero: input component is not an EnhancedInputComponent"));
	}
}

void AJamHero::PawnClientRestart()
{
	Super::PawnClientRestart();
	CreateInput();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;   // the hero faces the cursor
		if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Sub->AddMappingContext(InputContext, 0);
		}
	}
}

void AJamHero::OnMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	// The camera never turns, so world +X is screen up and +Y is screen right.
	AddMovementInput(FVector::ForwardVector, Axis.Y);
	AddMovementInput(FVector::RightVector, Axis.X);
}

void AJamHero::OnLecture(const FInputActionValue& /*Value*/)
{
	RequestLecture();
}

void AJamHero::OnAim(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	bUsingGamepad = true;
	// Right stick up = screen up = world +X.
	StickAim = FVector(Axis.Y, Axis.X, 0.f);
	if (StickAim.Size() >= JamHeroTuning::StickAimDeadZone) { AimSource = EAimSource::Stick; }
	else if (AimSource == EAimSource::Stick) { AimSource = EAimSource::Movement; }
}

void AJamHero::OnInteract(const FInputActionValue& /*Value*/)
{
	InteractPressedAt = -1.f;
	RequestInteract();
}

void AJamHero::OnInteractPressed(const FInputActionValue& /*Value*/)
{
	InteractPressedAt = GetWorld()->GetTimeSeconds();
}

void AJamHero::OnInteractReleased(const FInputActionValue& /*Value*/)
{
	InteractPressedAt = -1.f;
}

float AJamHero::GetInteractHoldFraction() const
{
	if (InteractPressedAt < 0.f) { return 0.f; }
	return FMath::Clamp((GetWorld()->GetTimeSeconds() - InteractPressedAt) / JamHeroTuning::InteractHoldSeconds, 0.f, 1.f);
}

AJamNuisance* AJamHero::FindInteractTarget() const
{
	AJamNuisance* Best = nullptr;
	// Range is measured from the hero's edge to the nuisance's centre, plus its radius.
	float BestDistance = JamHeroTuning::InteractRange + 40.f;
	for (TActorIterator<AJamNuisance> It(GetWorld()); It; ++It)
	{
		if (!It->IsPacified()) { continue; }
		const float D = FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) - GetCapsuleComponent()->GetScaledCapsuleRadius();
		if (D < BestDistance) { BestDistance = D; Best = *It; }
	}
	return Best;
}

bool AJamHero::RequestInteract()
{
	AJamGameMode* GM = GetWorld()->GetAuthGameMode<AJamGameMode>();
	if (!GM) { return false; }
	if (GM->HandleNoteAnswer(this, true)) { return true; }   // at the CUT! prompt, interact takes the note
	if (GM->HandleDismissWrap(this)) { return true; }   // the wrap panel: interact closes it
	AJamNuisance* Target = FindInteractTarget();
	return Target && GM->HandleInteract(this, Target);
}

bool AJamHero::RequestRefuseNote()
{
	AJamGameMode* GM = GetWorld()->GetAuthGameMode<AJamGameMode>();
	return GM && GM->HandleNoteAnswer(this, false);
}

void AJamHero::OnRefuse(const FInputActionValue& /*Value*/)
{
	RequestRefuseNote();
}

void AJamHero::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateFacing();
}

void AJamHero::UpdateFacing()
{
	FVector Facing = FVector::ZeroVector;
	APlayerController* PC = Cast<APlayerController>(GetController());

	// The mouse takes over as soon as it moves.
	FVector2D Mouse;
	if (PC && PC->GetMousePosition(Mouse.X, Mouse.Y))
	{
		if (LastMousePosition.X >= 0.f && FVector2D::Distance(Mouse, LastMousePosition) > 2.f)
		{
			AimSource = EAimSource::Mouse;
			bUsingGamepad = false;
		}
		LastMousePosition = Mouse;
	}

	if (!AimOverride.IsNearlyZero())
	{
		Facing = AimOverride;
	}
	else if (AimSource == EAimSource::Stick)
	{
		Facing = StickAim.GetSafeNormal2D();
	}
	else if (AimSource == EAimSource::Mouse && PC)
	{
		// Where the cursor ray meets the ground plane under the hero.
		FVector Origin, Direction;
		const float GroundZ = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		if (PC->DeprojectMousePositionToWorld(Origin, Direction) && Direction.Z < -KINDA_SMALL_NUMBER)
		{
			const FVector Hit = Origin + Direction * ((GroundZ - Origin.Z) / Direction.Z);
			Facing = (Hit - GetActorLocation()).GetSafeNormal2D();
		}
	}
	if (Facing.IsNearlyZero())
	{
		Facing = GetCharacterMovement()->Velocity.GetSafeNormal2D();
	}
	if (!Facing.IsNearlyZero())
	{
		SetActorRotation(Facing.Rotation());
	}
}

void AJamHero::ApplyStats(const FJamStatBlock& Stats)
{
	// The walk speed was tuned by hand (450); the stat's base is 600, so loot scales the tuned value.
	const float Base = FJamProgression().GetStats().MoveSpeed;
	GetCharacterMovement()->MaxWalkSpeed = JamHeroTuning::WalkSpeed * (Base > 0.f ? Stats.MoveSpeed / Base : 1.f);
}

void AJamHero::OnPause(const FInputActionValue& /*Value*/)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (AJamHUD* HUD = Cast<AJamHUD>(PC->GetHUD())) { HUD->RequestPause(); }
	}
}

void AJamHero::OnSkill2(const FInputActionValue& /*Value*/) { RequestSkill(EJamSkill::FormALine); }
void AJamHero::OnSkill3(const FInputActionValue& /*Value*/) { RequestSkill(EJamSkill::DisappointedSigh); }
void AJamHero::OnSkill4(const FInputActionValue& /*Value*/) { RequestSkill(EJamSkill::LastJudgment); }
