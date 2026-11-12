#include "Game/JamHero.h"
#include "Game/JamPlayerState.h"
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
	constexpr float LectureRange = 800.f;
	constexpr float LectureHalfAngleDeg = 35.f;
}

AJamHero::AJamHero()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 720.f, 0.f);
	Move->MaxWalkSpeed = 450.f;

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
	AJamPlayerState* PS = GetPlayerState<AJamPlayerState>();
	if (!PS) { return false; }
	const bool bActivated = PS->GetAbilitySystemComponent()->TryActivateAbility(PS->GetLectureHandle());
	if (bActivated)
	{
		DrawDebugCone(GetWorld(), GetActorLocation(), GetActorForwardVector(), JamHeroTuning::LectureRange,
			FMath::DegreesToRadians(JamHeroTuning::LectureHalfAngleDeg), FMath::DegreesToRadians(JamHeroTuning::LectureHalfAngleDeg),
			24, FColor::Orange, false, 1.5f, 0, 4.f);
	}
	UE_LOG(LogJam, Log, TEXT("JamHero: Lecture %s"), bActivated ? TEXT("cast") : TEXT("refused"));
	return bActivated;
}

void AJamHero::CreateInput()
{
	if (InputContext) { return; }

	MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	LectureAction = NewObject<UInputAction>(this, TEXT("IA_Lecture"));
	LectureAction->ValueType = EInputActionValueType::Boolean;

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

	InputContext->MapKey(LectureAction, EKeys::LeftMouseButton);
	InputContext->MapKey(LectureAction, EKeys::SpaceBar);
	InputContext->MapKey(LectureAction, EKeys::Gamepad_FaceButton_Bottom);
}

void AJamHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	CreateInput();
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AJamHero::OnMove);
		Input->BindAction(LectureAction, ETriggerEvent::Started, this, &AJamHero::OnLecture);
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
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
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
