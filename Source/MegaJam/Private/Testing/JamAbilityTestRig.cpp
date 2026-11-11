#include "Testing/JamAbilityTestRig.h"
#include "MegaJam.h"
#include "Gas/JamAbilitySystemComponent.h"
#include "Gas/JamResolveAttributeSet.h"
#include "Gas/JamGameplayAbility.h"
#include "Gas/JamGameplayTags.h"
#include "AbilitySystemGlobals.h"
#include "Camera/CameraActor.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"

// ---------------------------------------------------------------------------------------------------------
// AJamTestCombatant
// ---------------------------------------------------------------------------------------------------------

AJamTestCombatant::AJamTestCombatant()
{
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(40.f, 88.f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = Capsule;

	AbilitySystem = CreateDefaultSubobject<UJamAbilitySystemComponent>(TEXT("AbilitySystem"));
	// Created as a default subobject, so the ASC registers it automatically.
	ResolveSet = CreateDefaultSubobject<UJamResolveAttributeSet>(TEXT("ResolveSet"));
}

UAbilitySystemComponent* AJamTestCombatant::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

float AJamTestCombatant::GetResolve() const
{
	return ResolveSet ? ResolveSet->GetResolve() : 0.f;
}

void AJamTestCombatant::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UAbilitySystemGlobals::Get().InitGlobalData();
	AbilitySystem->InitAbilityActorInfo(this, this);
	if (TeamTag.IsValid())
	{
		AbilitySystem->AddLooseGameplayTag(TeamTag);
	}
}

// ---------------------------------------------------------------------------------------------------------
// AJamAbilityTestRig
// ---------------------------------------------------------------------------------------------------------

AJamAbilityTestRig::AJamAbilityTestRig()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJamAbilityTestRig::SpawnTarget(const FVector& Offset, bool bExpectedInCone, const TCHAR* Label)
{
	UWorld* World = GetWorld();
	AJamTestCombatant* Target = World->SpawnActorDeferred<AJamTestCombatant>(
		AJamTestCombatant::StaticClass(), FTransform(GetActorLocation() + Offset),
		nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Target->TeamTag = JamGameplayTags::Team_Nuisance;
	Target->FinishSpawning(FTransform(GetActorLocation() + Offset));
	Target->GetResolveSet()->OnPacified.AddUObject(this, &AJamAbilityTestRig::HandlePacified);

	FTargetInfo Info;
	Info.Actor = Target;
	Info.bExpectedInCone = bExpectedInCone;
	Info.Label = Label;
	Targets.Add(Info);
}

void AJamAbilityTestRig::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();

	// The hero stands at the rig and faces +X.
	Hero = World->SpawnActorDeferred<AJamTestCombatant>(
		AJamTestCombatant::StaticClass(), FTransform(GetActorLocation()),
		nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Hero->TeamTag = JamGameplayTags::Team_Hero;
	Hero->FinishSpawning(FTransform(GetActorLocation()));
	LectureHandle = Hero->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(UJamGA_LectureCone::StaticClass(), 1));

	// Inside the cone (800 uu, 35 degrees): five in a fan.
	const float InConeAngles[] = { -30.f, -15.f, 0.f, 15.f, 30.f };
	const float InConeDistances[] = { 450.f, 600.f, 700.f, 550.f, 400.f };
	for (int32 i = 0; i < 5; ++i)
	{
		const FVector Dir = FRotator(0.f, InConeAngles[i], 0.f).Vector();
		SpawnTarget(Dir * InConeDistances[i], true, TEXT("in cone"));
	}
	// Outside: too wide an angle (x2), behind the hero, and in front but beyond the range.
	SpawnTarget(FRotator(0.f,  70.f, 0.f).Vector() * 500.f, false, TEXT("too wide"));
	SpawnTarget(FRotator(0.f, -70.f, 0.f).Vector() * 500.f, false, TEXT("too wide"));
	SpawnTarget(FVector(-400.f, 0.f, 0.f),                   false, TEXT("behind"));
	SpawnTarget(FVector(1100.f, 0.f, 0.f),                   false, TEXT("out of range"));

	// Camera: above and behind the hero, looking down the cone.
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FVector CamPos = GetActorLocation() + FVector(-650.f, 0.f, 750.f);
		const FRotator CamRot = (GetActorLocation() + FVector(450.f, 0.f, 0.f) - CamPos).Rotation();
		if (ACameraActor* Cam = World->SpawnActor<ACameraActor>(CamPos, CamRot, Params))
		{
			PC->SetViewTarget(Cam);
		}
	}
	UE_LOG(LogJam, Log, TEXT("JamAbilityTest: scene ready (%d targets)"), Targets.Num());
}

void AJamAbilityTestRig::CastLecture(const TCHAR* Label, bool bExpectSuccess)
{
	UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent();
	const bool bActivated = ASC->TryActivateAbility(LectureHandle);

	int32 Hit = 0;
	if (const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(LectureHandle))
	{
		if (const UJamGA_AoeAttack* Ability = Cast<UJamGA_AoeAttack>(Spec->GetPrimaryInstance()))
		{
			Hit = bActivated ? Ability->LastHitTargets.Num() : 0;
		}
	}
	UE_LOG(LogJam, Log, TEXT("JamAbilityTest: cast %s -> activated=%d (expected %d), targets hit=%d"),
		Label, bActivated ? 1 : 0, bExpectSuccess ? 1 : 0, Hit);
	if (bActivated != bExpectSuccess) { ++CastFailures; }

	if (bActivated)
	{
		DrawDebugCone(GetWorld(), Hero->GetActorLocation(), Hero->GetActorForwardVector(), 800.f,
			FMath::DegreesToRadians(35.f), FMath::DegreesToRadians(35.f), 24, FColor::Orange, false, 1.2f, 0, 4.f);
	}
}

void AJamAbilityTestRig::HandlePacified(AActor* Pacified)
{
	++PacifiedEvents;
	UE_LOG(LogJam, Log, TEXT("JamAbilityTest: pacified %s"), Pacified ? *Pacified->GetName() : TEXT("(null)"));
}

void AJamAbilityTestRig::Verdict()
{
	int32 Wrong = 0;
	for (const FTargetInfo& T : Targets)
	{
		const AJamTestCombatant* A = T.Actor.Get();
		if (!A) { ++Wrong; continue; }
		const float Expected = T.bExpectedInCone ? 0.f : 100.f;
		if (!FMath::IsNearlyEqual(A->GetResolve(), Expected)) { ++Wrong; }
	}
	const bool bPass = Wrong == 0 && CastFailures == 0 && PacifiedEvents == 5;
	UE_LOG(LogJam, Log, TEXT("JamAbilityTest: %s (wrong targets=%d, cast failures=%d, pacified events=%d, expected 5)"),
		bPass ? TEXT("PASS") : TEXT("FAIL"), Wrong, CastFailures, PacifiedEvents);
}

void AJamAbilityTestRig::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Elapsed += DeltaSeconds;

	UWorld* World = GetWorld();
	for (const FTargetInfo& T : Targets)
	{
		const AJamTestCombatant* A = T.Actor.Get();
		if (!A) { continue; }
		const bool bPacified = A->GetAbilitySystemComponent()->HasMatchingGameplayTag(JamGameplayTags::State_Pacified);
		const FColor Colour = bPacified ? FColor::Cyan : (A->GetResolve() < 100.f ? FColor::Yellow : FColor::Green);
		DrawDebugCapsule(World, A->GetActorLocation(), 88.f, 40.f, FQuat::Identity, Colour, false, -1.f, 0, 3.f);
		DrawDebugString(World, A->GetActorLocation() + FVector(0.f, 0.f, 150.f),
			bPacified ? TEXT("PACIFIED") : *FString::Printf(TEXT("Resolve %.0f"), A->GetResolve()), nullptr, Colour, 0.f, true, 1.4f);
	}
	if (Hero.IsValid())
	{
		DrawDebugCapsule(World, Hero->GetActorLocation(), 88.f, 40.f, FQuat::Identity, FColor::Red, false, -1.f, 0, 4.f);
		DrawDebugString(World, Hero->GetActorLocation() + FVector(0.f, 0.f, 150.f), TEXT("Hero (Lecture)"), nullptr, FColor::Red, 0.f, true, 1.4f);
	}

	// Scripted timeline.
	if (NextStep == 0 && Elapsed >= 1.0f) { CastLecture(TEXT("#1"), true);  bCastOneOk = CastFailures == 0; ++NextStep; }
	else if (NextStep == 1 && Elapsed >= 1.5f) { const int32 Before = CastFailures; CastLecture(TEXT("#2 (on cooldown)"), false); bCastTwoRefused = CastFailures == Before; ++NextStep; }
	else if (NextStep == 2 && Elapsed >= 3.0f) { const int32 Before = CastFailures; CastLecture(TEXT("#3"), true); bCastThreeOk = CastFailures == Before; ++NextStep; }
	else if (NextStep == 3 && Elapsed >= 3.5f) { Verdict(); ++NextStep; }
}

// ---------------------------------------------------------------------------------------------------------
// UJamAbilityTestSubsystem
// ---------------------------------------------------------------------------------------------------------

bool UJamAbilityTestSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return FParse::Param(FCommandLine::Get(), TEXT("abilitytest")) && Super::ShouldCreateSubsystem(Outer);
}

bool UJamAbilityTestSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UJamAbilityTestSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	InWorld.SpawnActor<AJamAbilityTestRig>(FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator, Params);
}
