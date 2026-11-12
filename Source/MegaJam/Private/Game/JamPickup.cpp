#include "Game/JamPickup.h"
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
}

void AJamPickup::BeginPlay()
{
	Super::BeginPlay();
	if (UMaterialInstanceDynamic* M = Body->CreateAndSetMaterialInstanceDynamic(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), Colour);
	}
}
