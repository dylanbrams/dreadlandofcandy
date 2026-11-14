#include "Game/JamNuisance.h"
#include "Components/TextRenderComponent.h"
#include "Game/JamArt.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Game/JamHero.h"
#include "Game/JamGameMode.h"
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
#include "Game/JamSfx.h"

namespace
{
	FLinearColor ColourFor(EJamNuisanceState State, const FLinearColor& Base)
	{
		switch (State)
		{
		case EJamNuisanceState::Flustered: return FLinearColor(1.0f, 0.8f, 0.2f);   // yellow
		case EJamNuisanceState::Pacified:  return FLinearColor(0.75f, 0.75f, 0.78f); // calmed grey
		case EJamNuisanceState::Treated:   return FLinearColor(0.35f, 0.9f, 0.6f);   // mint
		case EJamNuisanceState::Charge:    return FLinearColor(1.0f, 0.12f, 0.08f);  // furious red
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
		case EJamNuisanceState::Charge:    return TEXT("Charge");
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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	BoxMesh = Cube.Object;
	BoxLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("BoxLabel"));
	BoxLabel->SetupAttachment(Capsule);
	BoxLabel->SetHorizontalAlignment(EHTA_Center);
	BoxLabel->SetVerticalAlignment(EVRTA_TextCenter);
	BoxLabel->SetWorldSize(46.f);
	BoxLabel->SetTextRenderColor(FColor(30, 30, 30));
	// Lying on the lid, readable from the game camera (which looks along +X).
	BoxLabel->SetRelativeLocation(FVector(0.f, 0.f, 44.f));
	BoxLabel->SetRelativeRotation(FRotator(90.f, 180.f, 0.f));
	BoxLabel->SetUsingAbsoluteRotation(true);
	BoxLabel->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	TreatArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TreatArrow"));
	TreatArrow->SetupAttachment(Capsule);
	TreatArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TreatArrow->SetCastShadow(false);
	if (Cone.Succeeded()) { TreatArrow->SetStaticMesh(Cone.Object); }
	if (ShapeMaterial.Succeeded()) { TreatArrow->SetMaterial(0, ShapeMaterial.Object); }
	TreatArrow->SetRelativeRotation(FRotator(180.f, 0.f, 0.f));   // point down at the head
	TreatArrow->SetUsingAbsoluteScale(true);                       // same small size on every kind
	TreatArrow->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.36f));
	TreatArrow->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Fx(TEXT("/Game/FX/M_FxUnlit.M_FxUnlit"));
	OverlayBase = Fx.Object;

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
	if (UMaterialInstanceDynamic* Mid = TreatArrow->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.15f, 1.f, 0.55f));
	}
	// The generated model first, so the squash, sit and recoil work from its own scale and height.
	if (!ArtMesh.IsEmpty() && JamArt::Apply(Body, ArtMesh, ArtHeight, -Capsule->GetScaledCapsuleHalfHeight()) && OverlayBase)
	{
		StateOverlay = UMaterialInstanceDynamic::Create(OverlayBase, this);
		Body->SetOverlayMaterial(StateOverlay);
	}
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
	UpdateStateOverlay();

	// Settle into a calm pose in Tick, keeping the imported feet on the floor.
    if (State == EJamNuisanceState::Pacified || State == EJamNuisanceState::Treated)
    {
        RecoilAt = -100.f;
        RecoilVelocity = FVector2D::ZeroVector;
    }
    ApplyBodySquash(1.f);
}

void AJamNuisance::ApplyBodySquash(float Squash)
{
    const float Widen = 1.f + (1.f - Squash) / 3.f;
    Body->SetRelativeScale3D(BodyBaseScale * FVector(Widen, Widen, Squash));
    const UStaticMesh* Mesh = Body->GetStaticMesh();
    const float Bottom = Mesh ? Mesh->GetBoundingBox().Min.Z * BodyBaseScale.Z : 0.f;
    Body->SetRelativeLocation(BodyBaseLocation + FVector(0.f, 0.f, Bottom * (1.f - Squash)));
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
		Fluster(FlusteredSeconds);
	}
}

void AJamNuisance::Fluster(float Seconds)
{
	if (IsPacified() || IsTreated()) { return; }
	if (State == EJamNuisanceState::Charge && Attack.bUnstoppable) { return; }   // the boss keeps coming
	// A skill hit stops a charge, and it does not come straight back.
	if (State == EJamNuisanceState::Charge) { AttackCooldownLeft = FMath::Max(AttackCooldownLeft, Attack.Cooldown * 0.5f); }
	if (State != EJamNuisanceState::Flustered) { SetState(EJamNuisanceState::Flustered); FlusterHold = Seconds; JamSfx::Play(this, TEXT("Flustered")); }
	else { FlusterHold = FMath::Max(FlusterHold, StateTime + Seconds); }
}

void AJamNuisance::Recoil(const FVector& From)
{
	const FVector2D Away = (FVector2D(GetActorLocation()) - FVector2D(From)).GetSafeNormal();
	RecoilVelocity = Away * 900.f;
	RecoilAt = GetWorld()->GetTimeSeconds();
}

void AJamNuisance::UpdateStateOverlay()
{
	if (!StateOverlay) { return; }
	// Wandering shows the model as it is; every other state washes it in its colour.
	const bool bPlain = State == EJamNuisanceState::Wander;
	StateOverlay->SetVectorParameterValue(TEXT("Color"), ColourFor(State, BaseColour));
	float Opacity = bPlain ? 0.f : (State == EJamNuisanceState::Flee ? 0.2f : 0.45f);
	if (State == EJamNuisanceState::Wander || State == EJamNuisanceState::Flee) { Opacity = FMath::Max(Opacity, NoteTintOpacity); }
	StateOverlay->SetScalarParameterValue(TEXT("Opacity"), Opacity);
}

void AJamNuisance::BecomeBox(const FString& Label)
{
	Body->SetOverlayMaterial(nullptr);
	StateOverlay = nullptr;
	Body->EmptyOverrideMaterials();
	Body->SetRelativeRotation(FRotator::ZeroRotator);
	if (BoxMesh) { Body->SetStaticMesh(BoxMesh); }
	BodyBaseScale = FVector(0.8f, 0.8f, 0.8f);
	BodyBaseLocation = FVector(0.f, 0.f, -30.f);
	Body->SetRelativeScale3D(BodyBaseScale);
	Body->SetRelativeLocation(BodyBaseLocation);
	BoxLabel->SetText(FText::FromString(Label));
	BoxLabel->SetHiddenInGame(false);
}

void AJamNuisance::Slow(float Factor, float Seconds)
{
	SlowFactor = FMath::Clamp(Factor, 0.05f, 1.f);
	SlowUntil = GetWorld()->GetTimeSeconds() + Seconds;
}

bool AJamNuisance::Treat()
{
	if (!IsPacified()) { return false; }
	SetState(EJamNuisanceState::Treated);
	JamSfx::Play(this, TEXT("Interact_Complete"));
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
	const float Slowed = GetWorld()->GetTimeSeconds() < SlowUntil ? SlowFactor : 1.f;
	FVector2D Next = FVector2D(Here) + DesiredVelocity * Slowed * DeltaSeconds;
	// Stay inside the set: clamping each axis removes the blocked component, so it slides along walls.
	const float Margin = Capsule->GetScaledCapsuleRadius();
	Next.X = FMath::Clamp(Next.X, SetBounds.Min.X + Margin, SetBounds.Max.X - Margin);
	Next.Y = FMath::Clamp(Next.Y, SetBounds.Min.Y + Margin, SetBounds.Max.Y - Margin);
	const FVector2D Moved = Next - FVector2D(Here);
	float Z = Here.Z;
	if (GroundOffset >= 0.f)
	{
		FHitResult Hit;
		FCollisionQueryParams Params(TEXT("JamNuisanceGround"), false, this);
		if (GetWorld()->LineTraceSingleByObjectType(Hit, FVector(Next, Here.Z + 400.f), FVector(Next, Here.Z - 800.f), FCollisionObjectQueryParams(ECC_WorldStatic), Params))
		{
			Z = Hit.ImpactPoint.Z + GroundOffset * GetActorScale3D().Z;
		}
	}
	SetActorLocation(FVector(Next, Z));
	if (Moved.SizeSquared() > 1.f)
	{
		SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(FMath::Atan2(Moved.Y, Moved.X)), 0.f));
	}
}

void AJamNuisance::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	StateTime += DeltaSeconds;

	// Fixes3 #14: bob a small arrow over anything waiting for a brush or a celery stick.
	const bool bWantsArrow = IsPacified();
	if (TreatArrow->bHiddenInGame == bWantsArrow) { TreatArrow->SetHiddenInGame(!bWantsArrow); }
	if (bWantsArrow)
	{
		const float Top = Capsule->GetScaledCapsuleHalfHeight() + 70.f + FMath::Sin(GetWorld()->GetTimeSeconds() * 5.f) * 12.f;
		TreatArrow->SetWorldLocation(GetActorLocation() + FVector(0.f, 0.f, Top));
	}

	// Recoil: a shove that dies out in a fifth of a second, and a squash that springs back.
	const float SinceRecoil = GetWorld()->GetTimeSeconds() - RecoilAt;
	if (IsPacified() || IsTreated())
    {
        const float Settle = FMath::SmoothStep(0.f, 0.3f, StateTime);
        const float Breathe = FMath::Sin(StateTime * 2.f * PI * 0.7f) * 0.018f * Settle;
        ApplyBodySquash(FMath::Lerp(1.f, 0.55f, Settle) + Breathe);
    }
	else if (SinceRecoil < 0.35f)
	{
		if (!RecoilVelocity.IsNearlyZero()) { Steer(RecoilVelocity, DeltaSeconds); RecoilVelocity *= FMath::Pow(0.0005f, DeltaSeconds); }
		const float Squash = FMath::Sin(PI * SinceRecoil / 0.35f) * 0.3f;
		ApplyBodySquash(1.f - Squash);
	}
	else if (RecoilAt > 0.f)
	{
		RecoilAt = -100.f;
		RecoilVelocity = FVector2D::ZeroVector;
		ApplyBodySquash(1.f);
	}

	float HeroDistance = 0.f;
	const AActor* Hero = FindNearestHero(HeroDistance);
	AttackCooldownLeft = FMath::Max(0.f, AttackCooldownLeft - DeltaSeconds);

	switch (State)
	{
	case EJamNuisanceState::Flustered:
		if (StateTime >= FMath::Max(FlusterHold, FlusteredSeconds)) { FlusterHold = 0.f; SetState(EJamNuisanceState::Wander); }
		break;

	case EJamNuisanceState::Charge:
		TickCharge(Hero, HeroDistance, DeltaSeconds);
		break;

	case EJamNuisanceState::Wander:
	case EJamNuisanceState::Flee:
	{
		// R4b: near enough and ready, it turns and charges (the game mode says whether a charge may start now).
		if (Hero && Attack.bAttacks && AttackCooldownLeft <= 0.f && HeroDistance <= Attack.ChargeRange)
		{
			AJamGameMode* GM = GetWorld()->GetAuthGameMode<AJamGameMode>();
			if (GM && GM->RequestCharge(this, HeroDistance))
			{
				SetState(EJamNuisanceState::Charge);
				AttackCooldownLeft = Attack.Cooldown;
				UE_LOG(LogJam, Log, TEXT("JamNuisance: %s charges from %.0f uu"), *GetName(), HeroDistance);
				break;
			}
		}
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
	// Only with -debuglabels: the jam ships a Development build, where debug draws are visible.
	static const bool bLabels = FParse::Param(FCommandLine::Get(), TEXT("debuglabels"));
	if (bLabels) { DrawDebugString(GetWorld(), FVector(0.f, 0.f, 110.f), NameOf(State), this, ColourFor(State, BaseColour).ToFColor(true), 0.f, true, 1.1f); }
}

namespace JamChargeTuning
{
	constexpr float WindUpSeconds = 0.35f;   // stops and quivers first, so the player can see it coming
}

void AJamNuisance::TickCharge(const AActor* Hero, float HeroDistance, float DeltaSeconds)
{
	using namespace JamChargeTuning;
	if (!Hero) { EndCharge(); return; }
	const FVector2D ToHero = FVector2D(Hero->GetActorLocation()) - FVector2D(GetActorLocation());
	if (StateTime < WindUpSeconds)
	{
		// Wind-up: face him and quiver.
		SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(FMath::Atan2(ToHero.Y, ToHero.X)), 0.f));
		const float Quiver = FMath::Sin(StateTime * 70.f) * 0.08f;
		Body->SetRelativeScale3D(BodyBaseScale * FVector(1.f + Quiver, 1.f - Quiver, 1.f + Quiver));
		return;
	}
	Body->SetRelativeScale3D(BodyBaseScale);
	Steer(ToHero.GetSafeNormal() * Speed * Attack.ChargeSpeedScale, DeltaSeconds);
	if (HeroDistance <= Attack.HitRange + ExtraReach())
	{
		AJamGameMode* GM = GetWorld()->GetAuthGameMode<AJamGameMode>();
		if (GM && GM->HandleNuisanceHit(this)) { ++HitCount; }
		EndCharge();
		return;
	}
	if (StateTime >= WindUpSeconds + Attack.ChargeSeconds) { EndCharge(); }
}

void AJamNuisance::EndCharge()
{
	Body->SetRelativeScale3D(BodyBaseScale);
	AttackCooldownLeft = FMath::Max(AttackCooldownLeft, Attack.Cooldown);
	PickWanderTarget();
	SetState(EJamNuisanceState::Wander);
}
