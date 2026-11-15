#include "Game/JamAmbientMotion.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "MegaJam.h"

AJamAmbientMotion::AJamAmbientMotion()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.f / 30.f;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetActorEnableCollision(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/FX/M_FxUnlit.M_FxUnlit"));
	PuffMesh = Sphere.Object;
	PuffMaterial = Mat.Object;
}

void AJamAmbientMotion::BeginPlay()
{
	Super::BeginPlay();
	if (!PuffMesh || !PuffMaterial) { SetActorTickEnabled(false); return; }
	// Real terrain buildings, discovered by mesh identity (actor editor labels are unavailable in a cook).
	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It && Sources.Num() < 20; ++It)
	{
		UStaticMeshComponent* C = It->GetStaticMeshComponent();
		if (!C || !C->GetStaticMesh()) { continue; }
		const FString Name = C->GetStaticMesh()->GetName();
		if (!(Name.Contains(TEXT("House")) || Name.Contains(TEXT("Cottage")) || Name.Contains(TEXT("TownHall"))
			|| Name.Contains(TEXT("Bakery")) || Name.Contains(TEXT("Dentist")) || Name == TEXT("SM_Farm"))) { continue; }
		const FBoxSphereBounds Bounds = C->Bounds;
		Sources.Add(Bounds.Origin + FVector(0.f, 0.f, Bounds.BoxExtent.Z - 15.f));
		if (Name.Contains(TEXT("Bakery")) && !BakerySign)
		{
			// The bakery faces the village square (+Y); keep these placements
			// relative to its actual world bounds rather than an actor label.
			BakeryFront = Bounds.Origin + FVector(0, Bounds.BoxExtent.Y + 12.f, -Bounds.BoxExtent.Z * 0.15f);
			UPointLightComponent* Oven = NewObject<UPointLightComponent>(this);
			Oven->SetupAttachment(RootComponent);
			Oven->SetMobility(EComponentMobility::Movable);
			Oven->SetLightColor(FLinearColor(1.f, 0.42f, 0.12f));
			Oven->SetIntensity(1800.f);
			Oven->SetAttenuationRadius(420.f);
			Oven->SetCastShadows(false);
			Oven->RegisterComponent();
			Oven->SetWorldLocation(BakeryFront);
			AddInstanceComponent(Oven);
			UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			if (Cube)
			{
				BakerySign = NewObject<UStaticMeshComponent>(this);
				BakerySign->SetupAttachment(RootComponent);
				BakerySign->SetStaticMesh(Cube);
				BakerySign->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				BakerySign->SetCastShadow(false);
				UMaterialInstanceDynamic* SignMat = UMaterialInstanceDynamic::Create(PuffMaterial, this);
				SignMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.45f, 0.16f, 0.06f));
				SignMat->SetScalarParameterValue(TEXT("Opacity"), 1.f);
				BakerySign->SetMaterial(0, SignMat);
				BakerySign->RegisterComponent();
				AddInstanceComponent(BakerySign);
				SignPivot = BakeryFront + FVector(Bounds.BoxExtent.X * 0.65f, 25.f, Bounds.BoxExtent.Z * 0.55f);
				BakerySign->SetWorldScale3D(FVector(0.8f, 0.08f, 0.45f));
			}
			for (int32 j = 0; j < 4; ++j)
			{
				UStaticMeshComponent* Flour = NewObject<UStaticMeshComponent>(this);
				Flour->SetupAttachment(RootComponent);
				Flour->SetStaticMesh(PuffMesh);
				Flour->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Flour->SetCastShadow(false);
				Flour->SetCullDistance(4000.f);
				UMaterialInstanceDynamic* FlourMat = UMaterialInstanceDynamic::Create(PuffMaterial, this);
				FlourMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.f, 0.9f, 0.75f));
				FlourMat->SetScalarParameterValue(TEXT("Opacity"), 0.f);
				Flour->SetMaterial(0, FlourMat);
				Flour->RegisterComponent();
				AddInstanceComponent(Flour);
				FlourPuffs.Add(Flour);
				FlourMaterials.Add(FlourMat);
			}
		}
	}
	for (int32 i = 0; i < Sources.Num() * 5; ++i)
	{
		UStaticMeshComponent* Puff = NewObject<UStaticMeshComponent>(this);
		Puff->SetupAttachment(RootComponent);
		Puff->SetStaticMesh(PuffMesh);
		Puff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Puff->SetCastShadow(false);
		Puff->SetCullDistance(6500.f);
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(PuffMaterial, this);
		Mat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.72f, 0.64f, 0.78f));
		Mat->SetScalarParameterValue(TEXT("Opacity"), 0.f);
		Puff->SetMaterial(0, Mat);
		Puff->RegisterComponent();
		AddInstanceComponent(Puff);
		Puffs.Add(Puff);
		Materials.Add(Mat);
	}
	// A small, fixed pool beside the riverbank. Trace to the terrain so the
	// lights remain close to flowers rather than floating at a guessed height.
	for (int32 i = 0; i < 12; ++i)
	{
		const float Angle = i * 2.399963f;
		const FVector XY(17000.f + FMath::Cos(Angle) * (600.f + 65.f * i),
			-3000.f + FMath::Sin(Angle) * (600.f + 65.f * i), 0.f);
		FHitResult Ground;
		if (!GetWorld()->LineTraceSingleByChannel(Ground, XY + FVector(0, 0, 6000),
			XY - FVector(0, 0, 6000), ECC_Visibility)) { continue; }
		UStaticMeshComponent* Light = NewObject<UStaticMeshComponent>(this);
		Light->SetupAttachment(RootComponent);
		Light->SetStaticMesh(PuffMesh);
		Light->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Light->SetCastShadow(false);
		Light->SetCullDistance(3500.f);
		UMaterialInstanceDynamic* Glow = UMaterialInstanceDynamic::Create(PuffMaterial, this);
		Glow->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.55f, 0.8f, 1.f));
		Glow->SetScalarParameterValue(TEXT("Opacity"), 0.7f);
		Light->SetMaterial(0, Glow);
		Light->RegisterComponent();
		AddInstanceComponent(Light);
		Fireflies.Add(Light);
		FireflyOrigins.Add(Ground.ImpactPoint + FVector(0, 0, 100));
	}
	UE_LOG(LogJam, Log, TEXT("JamAmbientMotion: %d rooftops, %d steam puffs, bakery %s, %d fireflies"),
		Sources.Num(), Puffs.Num(), BakerySign ? TEXT("found") : TEXT("missing"), Fireflies.Num());
	if (Sources.IsEmpty() && Fireflies.IsEmpty()) { SetActorTickEnabled(false); }
}

void AJamAmbientMotion::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;
	const APawn* Hero = UGameplayStatics::GetPlayerPawn(this, 0);
	if (BakerySign)
	{
		const float Swing = 9.f * FMath::Sin(Age * 1.2f);
		BakerySign->SetWorldLocation(SignPivot + FVector(0.f, FMath::Sin(FMath::DegreesToRadians(Swing)) * 22.f, -22.f));
		BakerySign->SetWorldRotation(FRotator(0.f, 0.f, Swing));
	}
	for (int32 i = 0; i < FlourPuffs.Num(); ++i)
	{
		const float Phase = FMath::Fmod(Age + i * 0.23f, 12.f);
		const bool bVisible = Phase < 1.8f && Hero && FVector::DistSquared(BakeryFront, Hero->GetActorLocation()) < FMath::Square(4000.f);
		FlourPuffs[i]->SetVisibility(bVisible);
		if (!bVisible) { continue; }
		const float T = Phase / 1.8f;
		FlourPuffs[i]->SetWorldLocation(BakeryFront + FVector((i - 1.5f) * 28.f, T * 90.f, T * 100.f));
		FlourPuffs[i]->SetWorldScale3D(FVector(FMath::Lerp(0.08f, 0.4f, T)));
		FlourMaterials[i]->SetScalarParameterValue(TEXT("Opacity"), FMath::Sin(T * PI) * 0.32f);
	}
	for (int32 i = 0; i < Fireflies.Num(); ++i)
	{
		const FVector Origin = FireflyOrigins[i];
		const bool bNear = Hero && FVector::DistSquared(Origin, Hero->GetActorLocation()) < FMath::Square(3500.f);
		Fireflies[i]->SetVisibility(bNear);
		if (!bNear) { continue; }
		const float T = Age * 0.65f + i * 1.7f;
		Fireflies[i]->SetWorldLocation(Origin + FVector(55.f * FMath::Cos(T), 55.f * FMath::Sin(T), 25.f * FMath::Sin(T * 1.7f)));
		Fireflies[i]->SetWorldScale3D(FVector(0.035f + 0.012f * (1.f + FMath::Sin(T * 3.f))));
	}
	for (int32 i = 0; i < Puffs.Num(); ++i)
	{
		const FVector Source = Sources[i / 5];
		const bool bNear = !Hero || FVector::DistSquared2D(Source, Hero->GetActorLocation()) < FMath::Square(6000.f);
		Puffs[i]->SetVisibility(bNear);
		if (!bNear) { continue; }
		const float Phase = FMath::Fmod(Age / 5.f + (i % 5) / 5.f + (i / 5) * 0.137f, 1.f);
		const float Curl = Phase * 5.f + i / 5;
		Puffs[i]->SetWorldLocation(Source + FVector(Phase * 170.f + FMath::Sin(Curl) * Phase * 35.f,
			Phase * 55.f + FMath::Cos(Curl) * Phase * 25.f, Phase * 340.f));
		Puffs[i]->SetWorldScale3D(FVector(FMath::Lerp(0.22f, 1.1f, Phase)));
		Materials[i]->SetScalarParameterValue(TEXT("Opacity"), FMath::Sin(Phase * PI) * 0.24f);
	}
}
