#include "Game/JamNuisance.h"
#include "Gas/JamAbilitySystemComponent.h"
#include "Gas/JamResolveAttributeSet.h"
#include "Gas/JamGameplayTags.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor NuisanceColour(1.0f, 0.35f, 0.6f);   // candy pink
	const FLinearColor PacifiedColour(0.75f, 0.75f, 0.78f); // calmed, grey
}

AJamNuisance::AJamNuisance()
{
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->InitCapsuleSize(40.f, 70.f);
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
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

bool AJamNuisance::IsPacified() const
{
	return AbilitySystem->HasMatchingGameplayTag(JamGameplayTags::State_Pacified);
}

void AJamNuisance::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UAbilitySystemGlobals::Get().InitGlobalData();
	AbilitySystem->InitAbilityActorInfo(this, this);
	AbilitySystem->AddLooseGameplayTag(JamGameplayTags::Team_Nuisance);
	ResolveSet->OnPacified.AddUObject(this, &AJamNuisance::HandlePacified);
}

void AJamNuisance::BeginPlay()
{
	Super::BeginPlay();
	BodyMaterial = Body->CreateAndSetMaterialInstanceDynamic(0);
	if (BodyMaterial) { BodyMaterial->SetVectorParameterValue(TEXT("Color"), NuisanceColour); }
}

void AJamNuisance::HandlePacified(AActor* /*Pacified*/)
{
	if (BodyMaterial) { BodyMaterial->SetVectorParameterValue(TEXT("Color"), PacifiedColour); }
}
