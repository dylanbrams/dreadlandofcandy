#include "Game/JamNuisance.h"
#include "Game/JamHero.h"
#include "Gas/JamAbilitySystemComponent.h"
#include "Gas/JamResolveAttributeSet.h"
#include "Gas/JamGameplayTags.h"
#include "MegaJam.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	FLinearColor ColourFor(EJamNuisanceState State, const FLinearColor& Base)
	{
		switch (State)
		{
		case EJamNuisanceState::Flustered: return FLinearColor(1.0f, 0.8f, 0.2f);   // yellow
		case EJamNuisanceState::Pacified:  return FLinearColor(0.75f, 0.75f, 0.78f); // calmed grey
		case EJamNuisanceState::Treated:   return FLinearColor(0.35f, 0.9f, 0.6f);   // mint
		default:                           return Base;                              // the kind's own colour
		}
	}

	const TCHAR* NameOf(EJamNuisanceState State)
	{
		switch (State)
		{
		case EJamNuisanceState::Wander:    return TEXT("Wander");
		case EJamNuisanceState::Flee:      return TEXT("Flee");
		case EJamNuisanceState::Flustered: return TEXT("Flustered");
		case EJamNuisanceState::Pacified:  return TEXT("Pacified");
		case EJamNuisanceState::Treated:   return TEXT("Treated");
		}
		return TEXT("?");
	}
}

AJamNuisance::AJamNuisance()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(40.f, 70.f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	// Never block the hero (Dylan: stuck behind pacified rats). Still a Pawn object, so the Lecture's
	// overlap-by-object-type query finds it; nuisances steer by clamping, not by collision.
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = Capsule;

	// Placeholder body until the approved rat mesh is imported (T20).
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	// The shapes default to the grid material; this one has a "Color" parameter for tinting.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(Capsule);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Cylinder.Succeeded()) { Body->SetStaticMesh(Cylinder.Object); }
	if (ShapeMaterial.Succeeded()) { Body->SetMaterial(0, ShapeMaterial.Object); }
	Body->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.4f));

	AbilitySystem = CreateDefaultSubobject<UJamAbilitySystemComponent>(TEXT("AbilitySystem"));
	ResolveSet = CreateDefaultSubobject<UJamResolveAttributeSet>(TEXT("ResolveSet"));
}

UAbilitySystemComponent* AJamNuisance::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

float AJamNuisance::GetResolve() const
{
	return ResolveSet ? ResolveSet->GetResolve() : 0.f;
}

void AJamNuisance::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UAbilitySystemGlobals::Get().InitGlobalData();
	AbilitySystem->InitAbilityActorInfo(this, this);
	AbilitySystem->AddLooseGameplayTag(JamGameplayTags::Team_Nuisance);
	ResolveSet->OnPacified.AddUObject(this, &AJamNuisance::HandlePacified);
	AbilitySystem->GetGameplayAttributeValueChangeDelegate(UJamResolveAttributeSet::GetResolveAttribute())
		.AddUObject(this, &AJamNuisance::HandleResolveChanged);
}

void AJamNuisance::BeginPlay()
{
	Super::BeginPlay();
	BodyBaseScale = Body->GetRelativeScale3D();
	BodyBaseLocation = Body->GetRelativeLocation();
	BodyMaterial = Body->CreateAndSetMaterialInstanceDynamic(0);
	// Max first: PreAttributeChange clamps Resolve to it.
	AbilitySystem->SetNumericAttributeBase(UJamResolveAttributeSet::GetMaxResolveAttribute(), MaxResolveValue);
	AbilitySystem->SetNumericAttributeBase(UJamResolveAttributeSet::GetResolveAttribute(), MaxResolveValue);
	PickWanderTarget();
	SetState(bStartPacified ? EJamNuisanceState::Pacified : EJamNuisanceState::Wander);
}

void AJamNuisance::ResetToPacified()
{
	if (IsTreated()) { SetState(EJamNuisanceState::Pacified); }
}

void AJamNuisance::SetState(EJamNuisanceState NewState)
{
	State = NewState;
	StateTime = 0.f;
	if (BodyMaterial) { BodyMaterial->SetVectorParameterValue(TEXT("Color"), ColourFor(State, BaseColour)); }

	// Pacified sits (squashed); everything else stands.
	const bool bSitting = State == EJamNuisanceState::Pacified || State == EJamNuisanceState::Treated;
	Body->SetRelativeScale3D(bSitting ? BodyBaseScale * FVector(1.15f, 1.15f, 0.55f) : BodyBaseScale);
	Body->SetRelativeLocation(bSitting ? BodyBaseLocation - FVector(0.f, 0.f, 35.f) : BodyBaseLocation);
}

void AJamNuisance::HandlePacified(AActor* /*Pacified*/)
{
	SetState(EJamNuisanceState::Pacified);
}

void AJamNuisance::HandleResolveChanged(const FOnAttributeChangeData& Data)
{
	// A hit that did not pacify: stop briefly. Pacify itself is handled by OnPacified.
	if (Data.NewValue < Data.OldValue && Data.NewValue > 0.f && !IsPacified() && !IsTreated())
	{
		SetState(EJamNuisanceState::Flustered);
	}
}

bool AJamNuisance::Treat()
{
	if (!IsPacified()) { return false; }
	SetState(EJamNuisanceState::Treated);
	return true;
}

void AJamNuisance::PickWanderTarget()
{
	WanderTarget = FVector2D(FMath::FRandRange(SetBounds.Min.X, SetBounds.Max.X), FMath::FRandRange(SetBounds.Min.Y, SetBounds.Max.Y));
}

const AActor* AJamNuisance::FindNearestHero(float& OutDistance) const
{
	const AActor* Nearest = nullptr;
	OutDistance = TNumericLimits<float>::Max();
	for (TActorIterator<AJamHero> It(GetWorld()); It; ++It)
	{
		const float D = FVector::Dist2D(It->GetActorLocation(), GetActorLocation());
		if (D < OutDistance) { OutDistance = D; Nearest = *It; }
	}
	return Nearest;
}

void AJamNuisance::Steer(const FVector2D& DesiredVelocity, float DeltaSeconds)
{
	const FVector Here = GetActorLocation();
	FVector2D Next = FVector2D(Here) + DesiredVelocity * DeltaSeconds;
	// Stay inside the set: clamping each axis removes the blocked component, so it slides along walls.
	const float Margin = Capsule->GetScaledCapsuleRadius();
	Next.X = FMath::Clamp(Next.X, SetBounds.Min.X + Margin, SetBounds.Max.X - Margin);
	Next.Y = FMath::Clamp(Next.Y, SetBounds.Min.Y + Margin, SetBounds.Max.Y - Margin);
	const FVector2D Moved = Next - FVector2D(Here);
	SetActorLocation(FVector(Next, Here.Z));
	if (Moved.SizeSquared() > 1.f)
	{
		SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(FMath::Atan2(Moved.Y, Moved.X)), 0.f));
	}
}

void AJamNuisance::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StateTime += DeltaSeconds;

	float HeroDistance = 0.f;
	const AActor* Hero = FindNearestHero(HeroDistance);

	switch (State)
	{
	case EJamNuisanceState::Flustered:
		if (StateTime >= FlusteredSeconds) { SetState(EJamNuisanceState::Wander); }
		break;

	case EJamNuisanceState::Wander:
	case EJamNuisanceState::Flee:
	{
		const bool bThreatened = Hero && HeroDistance < FleeRadius;
		if (bThreatened != (State == EJamNuisanceState::Flee))
		{
			SetState(bThreatened ? EJamNuisanceState::Flee : EJamNuisanceState::Wander);
			if (bThreatened)
			{
				++FleeCount;
				if (FleeCount == 1) { UE_LOG(LogJam, Log, TEXT("JamNuisance: %s first flees at %.1f s"), *GetName(), GetWorld()->GetTimeSeconds()); }
			}
			if (!bThreatened) { PickWanderTarget(); }
		}
		if (State == EJamNuisanceState::Flee)
		{
			const FVector2D Away = (FVector2D(GetActorLocation()) - FVector2D(Hero->GetActorLocation())).GetSafeNormal();
			Steer(Away * Speed, DeltaSeconds);
		}
		else
		{
			const FVector2D ToTarget = WanderTarget - FVector2D(GetActorLocation());
			// Erratic (bugs): a new heading every half second or so, not only on arrival.
			if (ToTarget.Size() < 40.f || (bErratic && FMath::Fmod(StateTime, 0.5f) < DeltaSeconds)) { PickWanderTarget(); }
			Steer(ToTarget.GetSafeNormal() * Speed * WanderSpeedScale, DeltaSeconds);
		}
		break;
	}

	case EJamNuisanceState::Pacified:
	case EJamNuisanceState::Treated:
		break;
	}

	// Development aid: state label (debug draws are compiled out of Shipping).
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 110.f), NameOf(State), this, ColourFor(State, BaseColour).ToFColor(true), 0.f, true, 1.1f);
}
