#include "Game/JamQuestMarker.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/ConstructorHelpers.h"

AJamQuestMarker::AJamQuestMarker()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Triangle"));
	SetRootComponent(Mesh);
	SetActorEnableCollision(false);
	Mesh->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/FX/M_FxUnlit.M_FxUnlit"));
	BaseMaterial = Mat.Object;
}

void AJamQuestMarker::BeginPlay()
{
	Super::BeginPlay();
	// A solid triangle and a soft, larger halo share the existing unlit skill-effect material.
	TArray<FVector> Vertices = { { 0, -50, 40 }, { 0, 50, 40 }, { 0, 0, -45 },
		{ -1, -67, 53 }, { -1, 67, 53 }, { -1, 0, -61 } };
	TArray<int32> Triangles = { 3, 4, 5, 0, 1, 2 };
	TArray<FLinearColor> Colours = { FLinearColor::White, FLinearColor::White, FLinearColor::White,
		FLinearColor(1, 1, 1, 0.12f), FLinearColor(1, 1, 1, 0.12f), FLinearColor(1, 1, 1, 0.12f) };
	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, {}, {}, Colours, {}, false);
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		Material->SetVectorParameterValue(TEXT("Color"), FLinearColor(2.8f, 1.8f, 0.08f));
		Material->SetScalarParameterValue(TEXT("Opacity"), 1.f);
		Mesh->SetMaterial(0, Material);
	}
}

void AJamQuestMarker::PointAt(const FVector& Ground, AActor* InAnchor, float Height)
{
	Base = Ground;
	Anchor = InAnchor;
	Lift = Height;
	SetActorLocation((Anchor.IsValid() ? Anchor->GetActorLocation() : Base) + FVector(0, 0, Lift));
	SetActorHiddenInGame(false);
}

void AJamQuestMarker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsHidden()) { return; }
	Age += DeltaSeconds;
	const FVector At = Anchor.IsValid() ? Anchor->GetActorLocation() : Base;
	SetActorLocation(At + FVector(0, 0, Lift + 22.f * FMath::Sin(Age * 4.f)));
	if (const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		SetActorRotation((Camera->GetCameraLocation() - GetActorLocation()).Rotation());
	}
}
