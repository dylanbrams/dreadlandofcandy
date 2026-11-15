#include "Game/JamAmbientMotion.h"
#include "Components/StaticMeshComponent.h"
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
	UE_LOG(LogJam, Log, TEXT("JamAmbientMotion: %d rooftops, %d recycled steam puffs"), Sources.Num(), Puffs.Num());
	if (Sources.IsEmpty()) { SetActorTickEnabled(false); }
}

void AJamAmbientMotion::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;
	const APawn* Hero = UGameplayStatics::GetPlayerPawn(this, 0);
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
