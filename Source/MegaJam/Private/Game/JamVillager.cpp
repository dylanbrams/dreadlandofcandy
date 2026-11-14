#include "Game/JamVillager.h"
#include "Game/JamArt.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"

AJamVillager::AJamVillager()
{
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	SetActorEnableCollision(false);
	Art = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Villager"));
	Art->SetupAttachment(RootComponent);
	Art->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Fallback(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	Art->SetStaticMesh(Fallback.Object);
	Art->SetRelativeLocation(FVector(0, 0, 85));
	Art->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.7f));
	RoleProp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TradeProp"));
	RoleProp->SetupAttachment(RootComponent);
	RoleProp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	HatPuff = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HatPuff"));
	HatPuff->SetupAttachment(RootComponent);
	HatPuff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HatPuff->SetStaticMesh(Sphere.Object);
	HatPuff->SetMaterial(0, ShapeMaterial.Object);
	HatPuff->SetHiddenInGame(true);
	HatPuff->SetVisibility(false);
	CylinderMesh = Fallback.Object;
	ShapeMat = ShapeMaterial.Object;
	Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Name"));
	Label->SetupAttachment(RootComponent);
	Label->SetRelativeLocation(FVector(0, 0, 220));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetWorldSize(30.f);
	Label->SetTextRenderColor(FColor(255, 226, 140));
}

void AJamVillager::BeginPlay()
{
	Super::BeginPlay();
	JamArt::Apply(Art, TEXT("SM_Villager"), JamArt::Height(TEXT("SM_Villager")));
}

void AJamVillager::SetName(const FString& Name) { Label->SetText(FText::FromString(Name)); }

void AJamVillager::SetRole(FName Giver)
{
	FString Prop = TEXT("SM_Clapperboard");
	float Height = 45.f, Z = 100.f;
	FVector Offset(45.f, -15.f, 0.f);
	// Fixes3 #11: props held beside the body, not through the head; the baker wears a chef's hat.
	if (Giver == FName(TEXT("GiverMayor"))) { Prop = TEXT("SM_LeafMegaphone"); Height = 40.f; Z = 75.f; Offset = FVector(30.f, -55.f, 0.f); }
	if (Giver == FName(TEXT("GiverBaker")))
	{
		RoleProp->SetStaticMesh(CylinderMesh);
		RoleProp->SetMaterial(0, ShapeMat);
		RoleProp->SetRelativeLocation(FVector(0.f, 0.f, 178.f));
		RoleProp->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.36f));
		HatPuff->SetRelativeLocation(FVector(0.f, 0.f, 202.f));
		HatPuff->SetRelativeScale3D(FVector(0.44f, 0.44f, 0.32f));
		HatPuff->SetHiddenInGame(false);
		HatPuff->SetVisibility(true);
		for (UStaticMeshComponent* C : { RoleProp.Get(), HatPuff.Get() })
		{
			if (UMaterialInstanceDynamic* M = C->CreateAndSetMaterialInstanceDynamic(0)) { M->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.95f, 0.95f, 0.93f)); }
		}
		return;
	}
	else if (Giver == FName(TEXT("GiverGardener"))) { Prop = TEXT("SM_Hat_Straw"); Height = 40.f; Z = 160.f; Offset = FVector::ZeroVector; }
	else if (Giver == FName(TEXT("GiverDentist"))) { Prop = TEXT("SM_HeadMirror"); Height = 30.f; Z = 160.f; Offset = FVector(10.f, 0.f, 0.f); }
	else if (Giver == FName(TEXT("GiverScout"))) { Prop = TEXT("SM_Crook"); Height = 180.f; Z = 0.f; Offset = FVector(10.f, -65.f, 0.f); }
	if (JamArt::Apply(RoleProp, Prop, Height, Z)) { RoleProp->AddLocalOffset(Offset); }
}

void AJamVillager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		Label->SetWorldRotation((Camera->GetCameraLocation() - Label->GetComponentLocation()).Rotation());
	}
}
