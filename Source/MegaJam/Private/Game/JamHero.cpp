#include "Game/JamHero.h"
#include "Game/JamGameState.h"
#include "Gas/JamResolveAttributeSet.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "Film/JamProgression.h"
#include "Game/JamPlayerState.h"
#include "Game/JamGameMode.h"
#include "Game/JamHUD.h"
#include "Game/JamSkillFx.h"
#include "Game/JamAttackParticles.h"
#include "Game/JamArt.h"
#include "Game/JamSettings.h"
#include "Film/JamCast.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
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
	constexpr float WalkSpeed = 600.f;
	constexpr float LectureRange = 800.f;
	constexpr float LectureHalfAngleDeg = 35.f;
	// BUILD_SPEC section 10: hold E 0.6 s, inside 150 uu.
	constexpr float InteractRange = 150.f;
	constexpr float InteractHoldSeconds = 0.6f;
	constexpr float StickAimDeadZone = 0.35f;
	// Open mode's follow camera (BUILD_SPEC 3e: about 500 uu, 15 degrees down; a little further so Fenella fits).
	constexpr float FollowArmLength = 900.f;          // Dylan, Nov 13: farther back, slightly lower
	constexpr float FollowSceneArmLength = 1500.f;
	constexpr float FollowScenePitch = -32.f;
	constexpr float FollowWalkPitch = -14.f;
	constexpr float FollowPitchMin = -55.f;
	constexpr float FollowPitchMax = 5.f;
	const FVector FollowTargetOffset(0.f, 0.f, 110.f);
	constexpr float MouseYawSpeed = 0.6f;        // degrees per mouse count
	constexpr float StickYawSpeed = 160.f;       // degrees per second at full stick
	constexpr float BotTurnSpeed = 4.f;          // how fast the camera swings behind the bot's aim
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

	// The sword hangs at his right hand, point down and forward; the Lecture raises it straight up.
	SwordPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SwordPivot"));
	SwordPivot->SetupAttachment(RootComponent);
	SwordPivot->SetRelativeLocation(FVector(10.f, 58.f, 5.f));
	SwordBlade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordBlade"));
	SwordBlade->SetupAttachment(SwordPivot);
	SwordGuard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordGuard"));
	SwordGuard->SetupAttachment(SwordPivot);
	for (UStaticMeshComponent* Part : { SwordBlade.Get(), SwordGuard.Get() })
	{
		Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Cube.Succeeded()) { Part->SetStaticMesh(Cube.Object); }
		if (ShapeMaterial.Succeeded()) { Part->SetMaterial(0, ShapeMaterial.Object); }
	}
	SwordBlade->SetRelativeLocation(FVector(0.f, 0.f, 75.f));
	SwordBlade->SetRelativeScale3D(FVector(0.05f, 0.12f, 1.3f));
	SwordGuard->SetRelativeLocation(FVector(0.f, 0.f, 8.f));
	SwordGuard->SetRelativeScale3D(FVector(0.07f, 0.42f, 0.06f));

	ArtBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArtBody"));
	ArtBody->SetupAttachment(RootComponent);
	ArtBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArtRig = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ArtRig"));
	ArtRig->SetupAttachment(RootComponent);
	ArtRig->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArtSword = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArtSword"));
	ArtSword->SetupAttachment(SwordPivot);
	ArtSword->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArtShield = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArtShield"));
	ArtShield->SetupAttachment(ArtRig, TEXT("hand_l"));
	ArtShield->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArtShield->SetHiddenInGame(true);
    ArtBrush = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArtBrush"));
    ArtBrush->SetupAttachment(GetRootComponent());
    ArtBrush->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ArtBrush->SetHiddenInGame(true);
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
	// The generated knight and his sword, standing on the capsule's floor; the placeholder shapes hide.
	const float Floor = -GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	if (JamArt::Apply(ArtBody, TEXT("SM_Hero"), JamArt::Height(TEXT("SM_Hero")), Floor))
	{
		ArtBodyScale = ArtBody->GetRelativeScale3D();
		ArtBodyLocation = ArtBody->GetRelativeLocation();
		ArtBodyRotation = ArtBody->GetRelativeRotation();
		Body->SetHiddenInGame(true);
		Visor->SetHiddenInGame(true);
	}
	// The sword's grip sits at the pivot (the hand); the blade runs along the pivot's +Z, which the Lecture raises.
	if (JamArt::Apply(ArtSword, TEXT("SM_Sword"), 150.f, -20.f, 90.f))
	{
		SwordBlade->SetHiddenInGame(true);
		SwordGuard->SetHiddenInGame(true);
	}
	// The rigged cast member (R6), if imported: replaces the static knight.
	const UJamSettings* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamSettings>() : nullptr;
	ApplyCast(Settings ? Settings->CastIndex() : 0);
	if (UMaterialInstanceDynamic* M = SwordBlade->CreateAndSetMaterialInstanceDynamic(0)) { M->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.85f, 0.88f, 0.95f)); }
	if (UMaterialInstanceDynamic* M = SwordGuard->CreateAndSetMaterialInstanceDynamic(0)) { M->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.85f, 0.6f, 0.15f)); }
}

void AJamHero::ApplyCast(int32 Index, bool bAnnounce)
{
	CastIndex = FMath::Clamp(Index, 0, JamCast::All().Num() - 1);
	const FJamClassDef& Def = JamCast::Get(CastIndex);
	USkeletalMesh* Rig = JamArt::LoadCastRig(Def.Rig);
	const bool bCast = Rig != nullptr;
	if (!Rig) { Rig = JamArt::LoadHeroRig(); }   // not imported: the old knight
	if (!Rig) { return; }
	// The sword's fitted size in world terms, taken before the rig's scale changes (the FBX bones carry their own scale).
	const FVector SwordWorldScale = ArtSword->GetComponentScale();
	const float Floor = -GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	ArtRig->SetSkeletalMesh(Rig);
	ArtShield->SetHiddenInGame(true);
	if (CastIndex == 0)
	{
		if (UStaticMesh* Shield = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Art/Props/Shield/SM_KnightShield.SM_KnightShield"), nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			ArtShield->SetStaticMesh(Shield);
			ArtShield->SetHiddenInGame(false);
		}
	}
	const FBox Box = Rig->GetImportedBounds().GetBox();
	// Fitted to the capsule like the static one, facing +X (the mannequin faces +Y).
	const float Scale = JamArt::Height(TEXT("SM_Hero")) / FMath::Max(Box.GetSize().Z, 1.f);
	ArtRig->SetRelativeScale3D(FVector(Scale));
	ArtRig->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	ArtRig->SetRelativeLocation(FVector(0.f, 0.f, Floor - Box.Min.Z * Scale));
	ArtRig->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	RigAnims.Reset();
	RigPlaying = NAME_None;
	RigOneShotUntil = -1.f;
	for (const TCHAR* Name : { TEXT("Idle"), TEXT("Walk"), TEXT("SwordRaise"), TEXT("Scrub"), TEXT("Sigh"), TEXT("ProudStrut"), TEXT("FormALine"), TEXT("LastJudgment") })
	{
        FString Clip = Name;
        int32 Slot = -1;
        if (Clip == TEXT("SwordRaise")) { Slot = 0; }
        else if (Clip == TEXT("FormALine")) { Slot = 1; }
        else if (Clip == TEXT("LastJudgment")) { Slot = 3; }
        if (bCast && Def.SkillAnimations.IsValidIndex(Slot) && !Def.SkillAnimations[Slot].IsEmpty()) { Clip = Def.SkillAnimations[Slot]; }
        UAnimSequence* Anim = bCast ? JamArt::LoadCastAnim(Def.Rig, Clip) : JamArt::LoadHeroAnim(Clip);
		if (Anim) { RigAnims.Add(FName(Name), Anim); }
	}
	ArtRig->SetHiddenInGame(false);
	ArtBody->SetHiddenInGame(true);
	Body->SetHiddenInGame(true);
	Visor->SetHiddenInGame(true);
	// The sword goes into the right hand and follows the animation (the Lecture's raise is in the knight's SwordRaise clip).
	// The grip (0, 0, -90) in the hand bone's frame: point up when the hand is raised; checked close up (-closecam).
	ArtSword->AttachToComponent(ArtRig, FAttachmentTransformRules::KeepRelativeTransform, TEXT("hand_r"));
	ArtSword->SetWorldScale3D(SwordWorldScale);

    static const TCHAR* Weapons[] = { TEXT(""), TEXT("SM_MageStaff"), TEXT("SM_ArcherBow"), TEXT("SM_BarbarianRollingPin") };
    if (CastIndex == 0)
    {
        if (UStaticMesh* Weapon = JamArt::Load(TEXT("SM_Sword")))
        {
            ArtSword->EmptyOverrideMaterials();
            ArtSword->SetStaticMesh(Weapon);
            ArtSword->SetWorldScale3D(FVector(150.f / FMath::Max(1.f, Weapon->GetBoundingBox().GetSize().Z)));
        }
    }
    else
    {
        const FString WeaponName = Weapons[CastIndex];
        const FString WeaponPath = FString::Printf(TEXT("/Game/Art/Cast/Weapons/%s.%s"), *WeaponName, *WeaponName);
        if (UStaticMesh* Weapon = LoadObject<UStaticMesh>(nullptr, *WeaponPath))
        {
            ArtSword->EmptyOverrideMaterials();
            ArtSword->SetStaticMesh(Weapon);
            const float Height = CastIndex == 1 ? 180.f : (CastIndex == 2 ? 140.f : 105.f);
            ArtSword->SetWorldScale3D(FVector(Height / FMath::Max(1.f, Weapon->GetBoundingBox().GetSize().Z)));
        }
    }

	ArtSword->SetRelativeLocation(FVector::ZeroVector);
	ArtSword->SetRelativeRotation(FRotator(0.f, 0.f, -90.f));
    EquippedWeaponScale = ArtSword->GetComponentScale();
    WeaponDrawUntil = -1.f;
    BrushUntil = -1.f;
    BrushTarget.Reset();
    bWeaponDrawn = true;
    UpdateWeaponCarry(true);
	TickRig();
	UE_LOG(LogJam, Log, TEXT("JamHero: cast %s (%s), %d animations, bounds %s, scale %.3f"), *Def.Id, bCast ? TEXT("cast rig") : TEXT("old hero rig"),
		RigAnims.Num(), *Box.GetSize().ToString(), Scale);
	if (bAnnounce) { LastCryAt = -100.0; PlayCry(EJamSkill::Lecture, false); }
}

void AJamHero::PlayCry(EJamSkill Skill, bool bAttack)
{
	const double Now = FPlatformTime::Seconds();
	AJamPlayerState* PS = GetPlayerState<AJamPlayerState>();
	const uint8 SkillBit = uint8(1u << static_cast<uint8>(Skill));
	const bool bFirst = bAttack && PS && !(PS->HeardFirstAttackMask & SkillBit);
	if (bFirst) { PS->HeardFirstAttackMask |= SkillBit; }
	if (!bFirst && Now - LastCryAt < JamCast::CryMinInterval) { return; }
	const FJamClassDef& Def = JamCast::Get(CastIndex);
	const FString Id = bAttack && Def.SkillCries.IsValidIndex(static_cast<int32>(Skill))
		? Def.SkillCries[static_cast<int32>(Skill)] : JamCast::CryFor(Def, CryCount);
	if (Id.IsEmpty()) { return; }
	FString Name = TEXT("VO_Cry_") + Id;
	USoundBase* Clip = nullptr;
	if (bAttack && !bFirst)
	{
		static const TCHAR* Languages[] = { TEXT("de"), TEXT("fr"), TEXT("ja"), TEXT("ko"), TEXT("zh_CN"), TEXT("pt_BR"), TEXT("es_ES"), TEXT("it"), TEXT("pl"), TEXT("ru"), TEXT("tr"), TEXT("uk"), TEXT("cs"), TEXT("th") };
		Name += TEXT("_") + FString(Languages[FMath::RandRange(0, UE_ARRAY_COUNT(Languages) - 1)]);
		Clip = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Audio/VO/Cast/International/%s.%s"), *Name, *Name), nullptr, LOAD_NoWarn | LOAD_Quiet);
	}
	if (!Clip)
	{
		Name = TEXT("VO_Cry_") + Id;
		Clip = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Audio/VO/Cast/%s.%s"), *Name, *Name), nullptr, LOAD_NoWarn | LOAD_Quiet);
	}
	if (!Clip) { UE_LOG(LogJam, Warning, TEXT("JamHero: missing cry %s"), *Name); return; }
	const UJamSettings* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamSettings>() : nullptr;
	const float Volume = Settings ? Settings->VoiceVolume : 0.8f;
	LastCryAt = Now;
	++CryCount;
	if (Volume <= 0.f) { return; }
	UGameplayStatics::PlaySound2D(this, Clip, Volume);
	UE_LOG(LogJam, Log, TEXT("JamHero: cry %s"), *Name);
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
	if (IsStaggered()) { return false; }                             // R4b: knocked off his stride
	if (GM && GM->IsNotePromptUp())                                  // CUT!: Q refuses the note, it does not sigh
	{
		return false;
	}
	// Gamepad attacks follow the camera; mouse attacks retain the cursor aim set by UpdateFacing.
	if (bFollowCamera && AimOverride.IsNearlyZero() && AimSource != EAimSource::Mouse) { SetActorRotation(FRotator(0.f, CameraYaw, 0.f)); }
	const bool bActivated = PS->GetAbilitySystemComponent()->TryActivateAbility(PS->GetSkillHandle(Skill));
	if (bActivated)
	{
		DrawSkillShape(Skill, PS->GetProgression().GetSkillNumbers(Skill));
		UE_LOG(LogJam, Log, TEXT("JamHero: %s cast"), JamProgression::SkillName(Skill));
		PlayCry(Skill, true);
		if (GM) { GM->NotifySkillCast(this, Skill); }
	}
	else { UE_LOG(LogJam, Verbose, TEXT("JamHero: %s refused"), JamProgression::SkillName(Skill)); }
	return bActivated;
}

void AJamHero::DrawSkillShape(EJamSkill Skill, const FJamSkillRank& N) const
{
	// The flash on the ground (T19); the debug shapes stay as the fallback when the FX material is missing.
	const FVector At = GetActorLocation();
	const FVector Fwd = GetActorForwardVector();
	const FVector Floor = At - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	AJamAttackParticles::Spawn(const_cast<AJamHero*>(this), Skill, N, Floor);
	AJamSkillFx* Fx = nullptr;
	switch (Skill)
	{
	case EJamSkill::Lecture:
		const_cast<AJamHero*>(this)->SwordUpAt = GetWorld()->GetTimeSeconds();
		const_cast<AJamHero*>(this)->PlayRigOneShot(TEXT("SwordRaise"));
		Fx = AJamSkillFx::Spawn(GetWorld(), EJamFxShape::Fan, Floor, Fwd, N.Radius > 0.f ? N.Radius : JamHeroTuning::LectureRange,
			JamHeroTuning::LectureHalfAngleDeg, FLinearColor(1.f, 0.32f, 0.05f), 0.45f);
		break;
	case EJamSkill::FormALine:
		const_cast<AJamHero*>(this)->PlayRigOneShot(TEXT("FormALine"));
		Fx = AJamSkillFx::Spawn(GetWorld(), EJamFxShape::Strip, Floor, Fwd, N.Length, N.Width * 0.5f, FLinearColor(0.3f, 0.85f, 1.f), 0.5f);
		break;
	case EJamSkill::DisappointedSigh:
		const_cast<AJamHero*>(this)->PlayRigOneShot(TEXT("Sigh"));
		Fx = AJamSkillFx::Spawn(GetWorld(), EJamFxShape::Disc, Floor, Fwd, N.Radius, 0.f, FLinearColor(0.65f, 0.5f, 1.f), 0.8f);
		break;
	default:
		const_cast<AJamHero*>(this)->PlayRigOneShot(TEXT("LastJudgment"));
		Fx = AJamSkillFx::Spawn(GetWorld(), EJamFxShape::Disc, Floor, Fwd, N.Radius, 0.f, FLinearColor(1.f, 0.15f, 0.1f), 1.0f);
		break;
	}
	if (Fx) { return; }
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
	WriteAction = NewObject<UInputAction>(this, TEXT("IA_WriteNote"));
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
	LookAction = NewObject<UInputAction>(this, TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;
	InputContext->MapKey(LookAction, EKeys::Mouse2D);
	CameraTurnAction = NewObject<UInputAction>(this, TEXT("IA_CameraTurn"));
	CameraTurnAction->ValueType = EInputActionValueType::Axis1D;
	InputContext->MapKey(CameraTurnAction, EKeys::Q).Modifiers.Add(NewObject<UInputModifierNegate>(InputContext));
	InputContext->MapKey(CameraTurnAction, EKeys::E);
	ZoomAction = NewObject<UInputAction>(this, TEXT("IA_Zoom"));
	ZoomAction->ValueType = EInputActionValueType::Axis1D;
	InputContext->MapKey(ZoomAction, EKeys::MouseWheelAxis);

	// Nov 14: primary / RT, interact on F / A. Space also lectures (handy on a laptop).
	InputContext->MapKey(LectureAction, EKeys::LeftMouseButton);
	InputContext->MapKey(LectureAction, EKeys::SpaceBar);
	InputContext->MapKey(LectureAction, EKeys::Gamepad_RightTrigger);
	InputContext->MapKey(InteractAction, EKeys::F);
	InputContext->MapKey(InteractAction, EKeys::Gamepad_FaceButton_Bottom);
	// At CUT!: hold F / A takes the note, X / B refuses it.
	InputContext->MapKey(RefuseAction, EKeys::X);
	InputContext->MapKey(RefuseAction, EKeys::Gamepad_FaceButton_Right);
	InputContext->MapKey(PauseAction, EKeys::Escape);
	InputContext->MapKey(WriteAction, EKeys::T);
	// Three active slots: Mouse 1 / Mouse 3 / R, RT / RB / LB.
	InputContext->MapKey(SkillActions[0], EKeys::MiddleMouseButton);
	InputContext->MapKey(SkillActions[0], EKeys::Gamepad_RightShoulder);
	InputContext->MapKey(SkillActions[2], EKeys::Gamepad_LeftShoulder);
	InputContext->MapKey(SkillActions[2], EKeys::R);
	InputContext->MapKey(WriteAction, EKeys::Gamepad_FaceButton_Top);
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
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AJamHero::OnLook);
		Input->BindAction(CameraTurnAction, ETriggerEvent::Triggered, this, &AJamHero::OnCameraTurn);
		Input->BindAction(CameraTurnAction, ETriggerEvent::Completed, this, &AJamHero::OnCameraTurn);
		Input->BindAction(CameraTurnAction, ETriggerEvent::Canceled, this, &AJamHero::OnCameraTurn);
		Input->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &AJamHero::OnZoom);
		Input->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AJamHero::OnInteract);
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &AJamHero::OnInteractPressed);
		Input->BindAction(InteractAction, ETriggerEvent::Canceled, this, &AJamHero::OnInteractReleased);
		Input->BindAction(InteractAction, ETriggerEvent::Completed, this, &AJamHero::OnInteractReleased);
		Input->BindAction(RefuseAction, ETriggerEvent::Started, this, &AJamHero::OnRefuse);
		Input->BindAction(PauseAction, ETriggerEvent::Started, this, &AJamHero::OnPause);
		Input->BindAction(WriteAction, ETriggerEvent::Started, this, &AJamHero::OnWrite);
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
		PC->bShowMouseCursor = true;   // Both modes keep HUD controls reachable
		if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Sub->AddMappingContext(InputContext, 0);
		}
	}
}

void AJamHero::OnMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (bFollowCamera)
	{
		const FRotator Yaw(0.f, CameraYaw, 0.f);
		AddMovementInput(Yaw.RotateVector(FVector::ForwardVector), Axis.Y);
		AddMovementInput(Yaw.RotateVector(FVector::RightVector), Axis.X);
		return;
	}
	// The camera never turns, so world +X is screen up and +Y is screen right.
	AddMovementInput(FVector::ForwardVector, Axis.Y);
	AddMovementInput(FVector::RightVector, Axis.X);
}

void AJamHero::OnLook(const FInputActionValue& Value)
{
	if (!bFollowCamera || bCameraPinned) { return; }
	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !PC->IsInputKeyDown(EKeys::RightMouseButton)) { return; }
	const FVector2D Axis = Value.Get<FVector2D>();   // mouse counts this frame
	bUsingGamepad = false;
	CameraYaw = FRotator::NormalizeAxis(CameraYaw + Axis.X * JamHeroTuning::MouseYawSpeed);
	CameraPitchFollow = FMath::Clamp(CameraPitchFollow + Axis.Y * JamHeroTuning::MouseYawSpeed * 0.6f,
		JamHeroTuning::FollowPitchMin, JamHeroTuning::FollowPitchMax);
}

void AJamHero::OnCameraTurn(const FInputActionValue& Value)
{
	CameraTurn = Value.Get<float>();
	if (!FMath::IsNearlyZero(CameraTurn)) { bUsingGamepad = false; }
}

void AJamHero::OnZoom(const FInputActionValue& Value)
{
	if (!bFollowCamera || bCameraPinned) { return; }
	const float Base = bSceneFraming ? JamHeroTuning::FollowSceneArmLength : JamHeroTuning::FollowArmLength;
	const float Distance = FMath::Clamp(Base + CameraZoomOffset - Value.Get<float>() * 120.f, 600.f, 2200.f);
	CameraZoomOffset = Distance - Base;
	CameraArm->TargetArmLength = Distance;
	bUsingGamepad = false;
}

void AJamHero::EnableFollowCamera(float Yaw)
{
	bFollowCamera = true;
	CameraYaw = Yaw;
	CameraArm->TargetArmLength = JamHeroTuning::FollowArmLength;
	CameraArm->TargetOffset = JamHeroTuning::FollowTargetOffset;
	CameraArm->bDoCollisionTest = true;
	CameraArm->ProbeChannel = ECC_Camera;
	CameraArm->bEnableCameraLag = true;
	CameraArm->CameraLagSpeed = 10.f;
	CameraArm->SetWorldRotation(FRotator(CameraPitchFollow, CameraYaw, 0.f));
	if (APlayerController* PC = Cast<APlayerController>(GetController())) { PC->bShowMouseCursor = true; }
}

void AJamHero::SetFollowFraming(bool bScene, float SceneYaw)
{
	if (!bFollowCamera) { return; }
	bSceneFraming = bScene;
	if (bScene && !bCameraPinned) { CameraYaw = SceneYaw; }
	CameraArm->TargetArmLength = FMath::Clamp((bScene ? JamHeroTuning::FollowSceneArmLength : JamHeroTuning::FollowArmLength) + CameraZoomOffset, 600.f, 2200.f);
	if (!bCameraPinned) { CameraPitchFollow = bScene ? JamHeroTuning::FollowScenePitch : JamHeroTuning::FollowWalkPitch; }
}

void AJamHero::TickFollowCamera(float DeltaSeconds)
{
	if (!bFollowCamera) { return; }
	if (!bCameraPinned) { CameraYaw = FRotator::NormalizeAxis(CameraYaw + CameraTurn * 120.f * DeltaSeconds); }
	if (!bCameraPinned && StickLook.Size() >= JamHeroTuning::StickAimDeadZone)
	{
		CameraYaw = FRotator::NormalizeAxis(CameraYaw + StickLook.X * JamHeroTuning::StickYawSpeed * DeltaSeconds);
		CameraPitchFollow = FMath::Clamp(CameraPitchFollow + StickLook.Y * JamHeroTuning::StickYawSpeed * 0.5f * DeltaSeconds,
			JamHeroTuning::FollowPitchMin, JamHeroTuning::FollowPitchMax);
	}
	// The bot's aim swings the camera round behind it, so a screenshot shows what it is doing.
	if (!AimOverride.IsNearlyZero() && !bCameraPinned && !bSceneFraming)
	{
		CameraYaw = FMath::FixedTurn(CameraYaw, AimOverride.Rotation().Yaw, JamHeroTuning::BotTurnSpeed * 45.f * DeltaSeconds);
	}
	CameraArm->SetWorldRotation(FRotator(CameraPitchFollow, CameraYaw, 0.f));
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
	if (bFollowCamera) { StickLook = Axis; StickAim = FVector::ZeroVector; AimSource = EAimSource::Movement; return; }   // the stick looks instead
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
    const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
    if (GS && GS->CanInteract()) { BeginBrushFeedback(FindInteractTarget(), 0.8f); }
}

void AJamHero::OnInteractReleased(const FInputActionValue& /*Value*/)
{
    InteractPressedAt = -1.f;
    if (BrushTarget.IsValid() && BrushTarget->IsPacified()) { BrushUntil = -1.f; }
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
		const float D = FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) - GetCapsuleComponent()->GetScaledCapsuleRadius() - It->ExtraReach();
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
	if (GM->HandleVillageInteract(this)) { return true; }
	AJamNuisance* Target = FindInteractTarget();
    if (Target && GM->HandleInteract(this, Target))
    {
        BeginBrushFeedback(Target, 0.9f);
        return true;
    }
    return false;
}

bool AJamHero::RequestWriteNote()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	AJamHUD* HUD = PC ? Cast<AJamHUD>(PC->GetHUD()) : nullptr;
	return HUD && HUD->OpenNoteEditor(bUsingGamepad);
}

void AJamHero::OnWrite()
{
	RequestWriteNote();
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
	TickFollowCamera(DeltaSeconds);
	TickSword();
	TickRig();
    TickPresentationReview();
}

void AJamHero::PlayRigOneShot(FName Anim)
{
	const TObjectPtr<UAnimSequence>* Found = RigAnims.Find(Anim);
	if (!Found || !ArtRig->GetSkeletalMeshAsset()) { return; }
    BrushUntil = -1.f;
    BrushTarget.Reset();
    ArtBrush->SetHiddenInGame(true);
	ArtRig->PlayAnimation(*Found, false);
    const float Rate = (*Found)->GetName().Contains(TEXT("SwordRaise")) ? 1.f : FMath::Max(1.f, (*Found)->GetPlayLength() / 1.6f);
    ArtRig->SetPlayRate(Rate);
    WeaponDrawUntil = GetWorld()->GetTimeSeconds() + 2.f;
	RigPlaying = Anim;
	RigOneShotUntil = GetWorld()->GetTimeSeconds() + FMath::Min((*Found)->GetPlayLength(), 1.6f);
}

void AJamHero::TickRig()
{
    UpdateBrushing();
    UpdateWeaponCarry();
    UpdateShieldCarry();
	if (ArtRig->GetSkeletalMeshAsset())
	{
		// R4b: a hit tips him back for a moment, then he stands straight again, unmoved.
		const float SinceHit = GetWorld()->GetTimeSeconds() - HitAt;
		const float Tip = SinceHit < 0.45f ? FMath::Sin(PI * SinceHit / 0.45f) * 18.f : 0.f;
		ArtRig->SetRelativeRotation(FQuat(FRotator(Tip, 0.f, 0.f)) * FQuat(FRotator(0.f, -90.f, 0.f)));
	}
	if (!ArtRig->GetSkeletalMeshAsset() || GetWorld()->GetTimeSeconds() < RigOneShotUntil) { return; }
	// Holding interact: he scrubs (teeth) or feeds (celery); otherwise walk or stand.
	FName Want = GetVelocity().Size2D() > 60.f ? FName(TEXT("Walk")) : FName(TEXT("Idle"));
	if (BrushTarget.IsValid() && GetWorld()->GetTimeSeconds() < BrushUntil && RigAnims.Contains(TEXT("Scrub"))) { Want = TEXT("Scrub"); }
	// Fixes3 #15: the stride follows ground speed so the feet keep up instead of sliding.
	const float WalkRate = FMath::Clamp(2.1f * GetVelocity().Size2D() / FMath::Max(1.f, JamHeroTuning::WalkSpeed), 1.2f, 3.f);
	if (Want == RigPlaying)
	{
		if (Want == FName(TEXT("Walk"))) { ArtRig->SetPlayRate(WalkRate); }
		return;
	}
	if (const TObjectPtr<UAnimSequence>* Found = RigAnims.Find(Want))
	{
		ArtRig->PlayAnimation(*Found, true);
		ArtRig->SetPlayRate(Want == FName(TEXT("Walk")) ? WalkRate : 1.f);
		RigPlaying = Want;
	}
}

void AJamHero::UpdateShieldCarry()
{
    if (CastIndex != 0 || !ArtShield->GetStaticMesh() || !ArtRig->GetSkeletalMeshAsset()) { return; }
    const FVector Hand = ArtRig->GetSocketLocation(TEXT("hand_l"));
    const FVector Elbow = ArtRig->GetSocketLocation(TEXT("lowerarm_l"));
    const FVector AlongArm = (Elbow - Hand).GetSafeNormal();
    const FVector Up = AlongArm.IsNearlyZero() ? FVector::UpVector : AlongArm;
    const FVector Outward = -GetActorRightVector();
    const FVector Size = ArtShield->GetStaticMesh()->GetBoundingBox().GetSize();
    const float Scale = 76.f / FMath::Max(1.f, Size.Z);
    // glTF/FBX importers may use X or Y as the thin/front axis; preserve Z-up.
    const FQuat SourceFacing = Size.X <= Size.Y ? FQuat::Identity : FQuat(FRotator(0, -90.f, 0));
    const FQuat Facing = FRotationMatrix::MakeFromXZ(Outward, Up).ToQuat() * SourceFacing;
    ArtShield->SetWorldTransform(FTransform(Facing, Hand + Up * 14.f + Outward * 9.f, FVector(Scale)));
}

void AJamHero::UpdateWeaponCarry(bool bForce)
{
    if (!ArtRig->GetSkeletalMeshAsset() || !ArtSword->GetStaticMesh()) { return; }
    const float Now = GetWorld()->GetTimeSeconds();
    const bool bBrushing = BrushTarget.IsValid() && Now < BrushUntil;
    bool bDraw = !bBrushing && (Now < RigOneShotUntil ||
        (Now < WeaponDrawUntil && GetVelocity().Size2D() < 60.f));
    const AJamGameState* GS = GetWorld()->GetGameState<AJamGameState>();
    if (PresentationReviewMode == -2 && GS && GS->GetShoot().GetState() != EJamShootState::Rolling && Now >= RigOneShotUntil) { bDraw = false; }
    if (PresentationReviewMode == -1) { bDraw = false; }
    if (!bForce && bDraw == bWeaponDrawn) { return; }
    bWeaponDrawn = bDraw;
    const FJamClassDef& Def = JamCast::Get(CastIndex);
    if (bDraw)
    {
        ArtSword->AttachToComponent(ArtRig, FAttachmentTransformRules::KeepWorldTransform, Def.WeaponHand);
        ArtSword->SetRelativeRotation(Def.WeaponGrip);
        const FTransform Hand = ArtRig->GetSocketTransform(Def.WeaponHand);
        // Keep the prop snug to the hand socket. A larger forward offset made
        // the oversized class props read as detached from the character.
        ArtSword->SetWorldLocation(Hand.GetLocation() + Hand.GetRotation().RotateVector(FVector(2.f, 0.f, 0.f)));
        ArtSword->SetWorldScale3D(EquippedWeaponScale);
    }
    else
    {
        const FName Spine = ArtRig->DoesSocketExist(TEXT("spine_03")) ? FName(TEXT("spine_03")) : FName(TEXT("pelvis"));
        ArtSword->AttachToComponent(ArtRig, FAttachmentTransformRules::KeepWorldTransform, Spine);
        const FQuat Align = FQuat::FindBetweenNormals(FVector::UpVector, Def.BackAxis.GetSafeNormal());
        const FQuat Turn(FVector::UpVector, FMath::DegreesToRadians(Def.BackTwist));
        ArtSword->SetWorldTransform(FTransform(GetActorQuat() * Align * Turn,
            GetActorTransform().TransformPosition(Def.BackOffset), EquippedWeaponScale));
    }
    UE_LOG(LogJam, Log, TEXT("JamWeapon: %s %s bone=%s"), *Def.Id, bDraw ? TEXT("hand") : TEXT("back"),
        *ArtSword->GetAttachSocketName().ToString());
}

FVector AJamHero::BrushingMouth(const AJamNuisance* Target) const
{
    if (const UStaticMeshComponent* BodyMesh = Target->FindComponentByClass<UStaticMeshComponent>())
    {
        const FBox Box = BodyMesh->Bounds.GetBox();
        const FVector Forward = Target->GetActorForwardVector();
        const FVector Extent = Box.GetExtent();
        const float Depth = FMath::Abs(Forward.X) * Extent.X + FMath::Abs(Forward.Y) * Extent.Y;
        return Box.GetCenter() + FVector(0.f, 0.f, Extent.Z * 0.52f) + Forward * Depth * 0.7f;
    }
    return Target->GetActorLocation();
}

void AJamHero::BeginBrushFeedback(AJamNuisance* Target, float Seconds)
{
    if (!Target || !Target->ArtMesh.Contains(TEXT("Goblin"))) { return; }
    BrushTarget = Target;
    BrushUntil = GetWorld()->GetTimeSeconds() + Seconds;
    RigOneShotUntil = -1.f;
    WeaponDrawUntil = -1.f;
    RigPlaying = NAME_None;
    if (!ArtBrush->GetStaticMesh()) { ArtBrush->SetStaticMesh(JamArt::Load(TEXT("SM_Toothbrush"))); }
    UpdateWeaponCarry();
    UpdateBrushing();
    UE_LOG(LogJam, Log, TEXT("BrushTool: %s oversized toothbrush -> %s"), *JamCast::Get(CastIndex).Id, *Target->GetName());
}

void AJamHero::UpdateBrushing()
{
    const float Now = GetWorld()->GetTimeSeconds();
    AJamNuisance* Target = BrushTarget.Get();
    const bool bShow = Target && Now < BrushUntil && ArtBrush->GetStaticMesh() && ArtRig->GetSkeletalMeshAsset();
    ArtBrush->SetHiddenInGame(!bShow);
    if (!bShow) { return; }
    SetActorRotation(FRotator(0.f, (Target->GetActorLocation() - GetActorLocation()).Rotation().Yaw, 0.f));
    const FTransform Hand = ArtRig->GetSocketTransform(TEXT("hand_r"));
    const FVector Grip = Hand.GetLocation() + Hand.GetRotation().RotateVector(FVector(7.f, 0.f, 0.f));
    const FVector Mouth = BrushingMouth(Target);
    const FVector Contact = Mouth + Target->GetActorRightVector() * FMath::Sin(Now * 18.f) * 7.f;
    const FVector Toward = Contact - Grip;
    const FBox Box = ArtBrush->GetStaticMesh()->GetBoundingBox();
    const float Tall = FMath::Max(1.f, Box.GetSize().Z);
    const float Length = FMath::Clamp(Toward.Size() / 0.78f, 130.f, 240.f);
    const float Scale = Length / Tall;
    const float Tip = Box.Min.Z + Tall * 0.88f;
    const float GripZ = FMath::Clamp(Tip - Toward.Size() / Scale, Box.Min.Z + Tall * 0.05f, Box.Min.Z + Tall * 0.65f);
    const FVector LocalGrip(Box.GetCenter().X, Box.GetCenter().Y, GripZ);
    const FQuat Turn = FQuat::FindBetweenNormals(FVector::UpVector, Toward.GetSafeNormal());
    ArtBrush->SetWorldTransform(FTransform(Turn, Grip - Turn.RotateVector(LocalGrip * Scale), FVector(Scale)));
    if (Now >= NextBrushFoamAt)
    {
        AJamAttackParticles::SpawnBrushFoam(this, Target, Mouth);
        NextBrushFoamAt = Now + 0.55f;
    }
}

void AJamHero::TickPresentationReview()
{
#if !UE_BUILD_SHIPPING
    const float Now = GetWorld()->GetTimeSeconds();
    if (!bPresentationReviewParsed)
    {
        bPresentationReviewParsed = true;
        bPresentationReviewAll = FParse::Param(FCommandLine::Get(), TEXT("classreview"));
        FParse::Value(FCommandLine::Get(), TEXT("castreview="), PresentationReviewMode);
        FParse::Value(FCommandLine::Get(), TEXT("reviewage="), PresentationReviewAge);
        PresentationReviewSince = Now;
        if (bPresentationReviewAll || PresentationReviewMode != -2)
        {
            GetCharacterMovement()->DisableMovement();
            SetActorLocation(FVector(0.f, 0.f, 10000.f));
            SetActorRotation(FRotator::ZeroRotator);
            AActor* ReviewFloor = GetWorld()->SpawnActor<AActor>();
            UStaticMeshComponent* FloorMesh = NewObject<UStaticMeshComponent>(ReviewFloor);
            ReviewFloor->SetRootComponent(FloorMesh);
            ReviewFloor->AddInstanceComponent(FloorMesh);
            FloorMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
            FloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            FloorMesh->SetCollisionResponseToAllChannels(ECR_Block);
            FloorMesh->RegisterComponent();
            ReviewFloor->SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 10.f));
            ReviewFloor->SetActorScale3D(FVector(20.f, 20.f, 0.2f));
            ReviewFloor->SetLifeSpan(60.f);
        }
    }
    if (bPresentationReviewAll)
    {
        const int32 Step = FMath::FloorToInt((Now - PresentationReviewSince) / 2.f);
        if (Step >= 17)
        {
            UE_LOG(LogJam, Log, TEXT("ClassReview: SUCCESS 12 attacks, four back carries and brushing"));
            if (APlayerController* PC = Cast<APlayerController>(GetController())) { PC->ConsoleCommand(TEXT("quit")); }
            return;
        }
        if (Step != PresentationReviewStep)
        {
            PresentationReviewStep = Step;
            for (TActorIterator<AJamAttackParticles> It(GetWorld()); It; ++It) { It->Destroy(); }
            ApplyCast(Step < 16 ? Step / 4 : 0, false);
            const int32 Modes[] = { -1, 0, 1, 3 };
            PresentationReviewMode = Step < 16 ? Modes[Step % 4] : 4;
            bPresentationReviewStarted = false;
            bPresentationReviewCaptured = false;
            PresentationReviewStepSince = Now;
        }
    }
    if (PresentationReviewMode == -2) { return; }
    CameraArm->TargetArmLength = 520.f;
    CameraArm->TargetOffset = FVector::ZeroVector;
    CameraArm->bDoCollisionTest = false;
    CameraArm->bEnableCameraLag = false;
    CameraArm->SetWorldRotation(FRotator(-12.f, (PresentationReviewMode == -1 || PresentationReviewMode == 4) ? 30.f : 150.f, 0.f));
    for (TActorIterator<AJamNuisance> It(GetWorld()); It; ++It) { It->Fluster(60.f); }
    if (!bPresentationReviewStarted)
    {
        bPresentationReviewStarted = true;
        PresentationReviewStepSince = Now;
        if (PresentationReviewMode == -1)
        {
            if (const auto* Walk = RigAnims.Find(TEXT("Walk"))) { ArtRig->PlayAnimation(*Walk, true); }
        }
        else if (PresentationReviewMode == 4)
        {
            for (TActorIterator<AJamNuisance> It(GetWorld()); It; ++It)
            {
                if (It->ArtMesh.Contains(TEXT("Goblin")))
                {
                    It->GetResolveSet()->SetResolve(0.f);
                    It->GetResolveSet()->OnPacified.Broadcast(*It);
                    It->SetActorLocation(GetActorLocation() + GetActorForwardVector() * 180.f);
                    It->SetActorRotation((GetActorLocation() - It->GetActorLocation()).Rotation());
                    BeginBrushFeedback(*It, 20.f);
                    break;
                }
            }
        }
        else
        {
            FJamSkillRank Rank;
            Rank.Radius = 400.f; Rank.Length = 700.f; Rank.Width = 180.f;
            DrawSkillShape(static_cast<EJamSkill>(PresentationReviewMode), Rank);
        }
    }
    if (PresentationReviewMode != 4)
    {
        RigOneShotUntil = Now + 1.f;
        WeaponDrawUntil = Now + 1.f;
        ArtRig->SetPlayRate(0.f);
        ArtRig->SetPosition(PresentationReviewAge, false);
        UpdateWeaponCarry();
    }
    if (bPresentationReviewAll && !bPresentationReviewCaptured && Now - PresentationReviewStepSince >= 1.2f)
    {
        bPresentationReviewCaptured = true;
        const FString Mode = PresentationReviewMode == -1 ? TEXT("back") : PresentationReviewMode == 4 ? TEXT("brush") : FString::FromInt(PresentationReviewMode);
        const FString Path = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Screenshots/WindowsEditor"),
            FString::Printf(TEXT("classreview_%s_%s.png"), *JamCast::Get(CastIndex).Id, *Mode));
        FScreenshotRequest::RequestScreenshot(Path, true, false);
        const float Floor = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        UE_LOG(LogJam, Log, TEXT("ClassReview: class=%s mode=%s weapon_min_z=%.2f floor=%.2f"),
            *JamCast::Get(CastIndex).Id, *Mode, ArtSword->Bounds.GetBox().Min.Z, Floor);
    }
#endif
}

namespace JamSwordTuning
{
	constexpr float RestPitch = -135.f;   // point down and forward
	constexpr float UpPitch = -4.f;       // straight up, a touch forward
	constexpr float RaiseSeconds = 0.12f; // the whole wind-up stays under 0.4 s (BUILD_SPEC 6)
	constexpr float HoldSeconds = 0.45f;
	constexpr float LowerSeconds = 0.3f;
	constexpr float InhaleSeconds = 0.25f;
	constexpr float InhaleAmount = 0.1f;
}

void AJamHero::TickSword()
{
	using namespace JamSwordTuning;
	const float T = GetWorld()->GetTimeSeconds() - SwordUpAt;
	float Up = 0.f;
	if (T < RaiseSeconds) { Up = FMath::InterpEaseOut(0.f, 1.f, FMath::Clamp(T / RaiseSeconds, 0.f, 1.f), 3.f); }
	else if (T < RaiseSeconds + HoldSeconds) { Up = 1.f; }
	else if (T < RaiseSeconds + HoldSeconds + LowerSeconds) { Up = 1.f - FMath::InterpEaseInOut(0.f, 1.f, (T - RaiseSeconds - HoldSeconds) / LowerSeconds, 2.f); }
	SwordPivot->SetRelativeRotation(FRotator(FMath::Lerp(RestPitch, UpPitch, Up), 0.f, 0.f));
	// The inhale before the speech.
	const float Breath = T < InhaleSeconds ? FMath::Sin(PI * T / InhaleSeconds) * InhaleAmount : 0.f;
	Body->SetRelativeScale3D(FVector(0.85f * (1.f + Breath), 0.85f * (1.f + Breath), 1.9f));
	if (ArtBody->GetStaticMesh())
	{
		ArtBody->SetRelativeScale3D(ArtBodyScale * FVector(1.f + Breath, 1.f + Breath, 1.f));
		// No rig yet (BUILD_SPEC 6 fallback): a static knight that waddles. Bounce twice per stride, rock once.
		const float Speed = GetVelocity().Size2D();
		const float Walk = FMath::Clamp(Speed / 300.f, 0.f, 1.f);
		WalkPhase += GetWorld()->GetDeltaSeconds() * (4.f + Speed / 80.f) * Walk;
		const float Bounce = FMath::Abs(FMath::Sin(WalkPhase)) * 9.f * Walk;
		const float Rock = FMath::Sin(WalkPhase) * 5.f * Walk;
		ArtBody->SetRelativeLocation(ArtBodyLocation + FVector(0.f, 0.f, Bounce));
		ArtBody->SetRelativeRotation(ArtBodyRotation + FRotator(0.f, 0.f, Rock));
	}
}

void AJamHero::UpdateFacing()
{
	FVector Facing = FVector::ZeroVector;
	APlayerController* PC = Cast<APlayerController>(GetController());

	// Mouse movement aims unless it is being used to drag the follow camera.
	FVector2D Mouse;
	if (PC && PC->GetMousePosition(Mouse.X, Mouse.Y))
	{
		if ((!bFollowCamera || !PC->IsInputKeyDown(EKeys::RightMouseButton)) && LastMousePosition.X >= 0.f && FVector2D::Distance(Mouse, LastMousePosition) > 2.f)
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

void AJamHero::BeginPlay()
{
	Super::BeginPlay();
	// -closecam=<uu>: the camera close on the hero (checking the rig and the sword).
	float Close = 0.f;
	if (FParse::Value(FCommandLine::Get(), TEXT("closecam="), Close) && Close > 0.f) { CameraArm->TargetArmLength = Close; CameraArm->TargetOffset = FVector::ZeroVector; }
}

void AJamHero::SetCameraDistance(float Distance)
{
	CameraArm->TargetArmLength = Distance;
	CameraArm->TargetOffset = FVector(Distance * 0.35f, 0.f, 0.f);
}

// ---- R4b: hit back ----------------------------------------------------------------------------------------------

void AJamHero::TakeHit(const FVector& From, float Knockback, float StaggerSeconds)
{
	const float Now = GetWorld()->GetTimeSeconds();
	StaggerUntil = FMath::Max(StaggerUntil, Now + StaggerSeconds);
	HitAt = Now;
	FVector Away = GetActorLocation() - From;
	Away.Z = 0.f;
	Away = Away.GetSafeNormal();
	if (Away.IsNearlyZero()) { Away = -GetActorForwardVector(); }
	GetCharacterMovement()->StopMovementImmediately();
	LaunchCharacter(Away * Knockback + FVector(0.f, 0.f, 260.f), true, true);
}

bool AJamHero::IsStaggered() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < StaggerUntil;
}

void AJamHero::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	if (IsStaggered()) { return; }
	Super::AddMovementInput(WorldDirection, ScaleValue, bForce);
}
