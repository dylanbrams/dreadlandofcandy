#include "Game/JamConfetti.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AJamConfetti::AJamConfetti()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	SetActorEnableCollision(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/FX/M_FxUnlit.M_FxUnlit"));
	if (Cube.Succeeded()) { CubeMesh = Cube.Object; }
	if (Material.Succeeded()) { FxMaterial = Material.Object; }
}

void AJamConfetti::Burst(int32 Count, float InLife)
{
	Life = FMath::Max(0.5f, InLife);
	static const FLinearColor Colours[Groups] = { FLinearColor(2.4f, 0.5f, 1.1f), FLinearColor(0.5f, 2.2f, 1.4f), FLinearColor(2.4f, 1.7f, 0.3f) };
	TArray<UMaterialInstanceDynamic*> Mids;
	for (int32 g = 0; g < Groups && FxMaterial; ++g)
	{
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(FxMaterial, this);
		Mid->SetVectorParameterValue(TEXT("Color"), Colours[g]);
		Mid->SetScalarParameterValue(TEXT("Opacity"), 1.f);
		Mids.Add(Mid);
		Materials.Add(Mid);
	}
	for (int32 i = 0; i < Count; ++i)
	{
		UStaticMeshComponent* Fleck = NewObject<UStaticMeshComponent>(this);
		Fleck->SetMobility(EComponentMobility::Movable);
		Fleck->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Fleck->SetCastShadow(false);
		Fleck->SetStaticMesh(CubeMesh);
		if (Mids.Num() == Groups) { Fleck->SetMaterial(0, Mids[i % Groups]); }
		Fleck->SetupAttachment(GetRootComponent());
		Fleck->RegisterComponent();
		Flecks.Add(Fleck);
		FPiece P;
		P.Position = GetActorLocation();
		const float Yaw = FMath::FRandRange(0.f, 360.f);
		P.Velocity = FRotator(0.f, Yaw, 0.f).Vector() * FMath::FRandRange(80.f, 260.f) + FVector(0.f, 0.f, FMath::FRandRange(380.f, 620.f));
		P.Rotation = FRotator(FMath::FRandRange(0.f, 360.f), FMath::FRandRange(0.f, 360.f), FMath::FRandRange(0.f, 360.f));
		P.Spin = FRotator(FMath::FRandRange(-540.f, 540.f), FMath::FRandRange(-540.f, 540.f), FMath::FRandRange(-540.f, 540.f));
		P.Size = FMath::FRandRange(0.16f, 0.3f);
		Pieces.Add(P);
		Fleck->SetWorldLocationAndRotation(P.Position, P.Rotation);
		Fleck->SetWorldScale3D(FVector(P.Size, P.Size * 0.5f, P.Size * 0.12f));
	}
}

void AJamConfetti::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;
	if (Age >= Life) { Destroy(); return; }
	const float Fade = FMath::Clamp((Life - Age) / (Life * 0.3f), 0.f, 1.f);
	for (int32 i = 0; i < Pieces.Num(); ++i)
	{
		FPiece& P = Pieces[i];
		P.Velocity.Z -= 900.f * DeltaSeconds;
		P.Velocity *= FMath::Exp(-1.2f * DeltaSeconds);
		P.Position += P.Velocity * DeltaSeconds;
		P.Rotation += P.Spin * DeltaSeconds;
		const float S = P.Size * Fade;
		Flecks[i]->SetWorldLocationAndRotation(P.Position, P.Rotation);
		Flecks[i]->SetWorldScale3D(FVector(S, S * 0.5f, S * 0.12f));
	}
}
