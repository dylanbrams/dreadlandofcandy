#include "Game/JamPickup.h"
#include "Game/JamArt.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AJamPickup::AJamPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = Body;
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Sphere.Succeeded()) { Body->SetStaticMesh(Sphere.Object); }
	if (ShapeMaterial.Succeeded()) { Body->SetMaterial(0, ShapeMaterial.Object); }
	Body->SetRelativeScale3D(FVector(0.6f, 0.4f, 0.4f));   // almond-ish
	Art = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Art"));
	Art->SetupAttachment(Body);
	Art->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AJamPickup::BeginPlay()
{
	Super::BeginPlay();
	// The generated almond on the floor (the root floats 25 uu up); the root loses its sphere and its squash.
	if (JamArt::Load(ArtMesh))
	{
		Body->SetStaticMesh(nullptr);
		Body->SetRelativeScale3D(FVector::OneVector);
		JamArt::Apply(Art, ArtMesh, JamArt::Height(ArtMesh), -25.f);
	}
	if (UMaterialInstanceDynamic* M = Body->CreateAndSetMaterialInstanceDynamic(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), Colour);
	}
}
