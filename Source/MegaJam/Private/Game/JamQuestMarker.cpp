#include "Game/JamQuestMarker.h"
#include "Game/JamGameState.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AJamQuestMarker::AJamQuestMarker()
{
	PrimaryActorTick.bCanEverTick = true;
	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Cane"));
	SetRootComponent(Pivot);
	SetActorEnableCollision(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/FX/M_FxUnlit.M_FxUnlit"));
	BaseMaterial = Mat.Object;
	// Eight straight bands and twelve short segments form a hooked candy cane.
	for (int32 i = 0; i < 21; ++i)
	{
		UStaticMeshComponent* Band = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Band%d"), i));
		Band->SetupAttachment(Pivot);
		Band->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Band->SetCastShadow(false);
		Band->SetStaticMesh(i == 20 ? Cone.Object : Cylinder.Object);
		if (i < 8)
		{
			Band->SetRelativeLocation(FVector(0, 0, -56.f + i * 18.f));
			Band->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.185f));
		}
		else if (i < 20)
		{
			const float A = (i - 8) * PI / 12.f;
			const float B = (i - 7) * PI / 12.f;
			const FVector Start(28.f - 28.f * FMath::Cos(A), 0, 79.f + 28.f * FMath::Sin(A));
			const FVector End(28.f - 28.f * FMath::Cos(B), 0, 79.f + 28.f * FMath::Sin(B));
			Band->SetRelativeLocation((Start + End) * 0.5f);
			Band->SetRelativeRotation(FRotationMatrix::MakeFromZ(End - Start).Rotator());
			Band->SetRelativeScale3D(FVector(0.18f, 0.18f, ((End - Start).Size() + 3.f) / 100.f));
		}
		else
		{
			Band->SetRelativeLocation(FVector(0, 0, -78.f));
			Band->SetRelativeRotation(FRotator(180.f, 0, 0));
			Band->SetRelativeScale3D(FVector(0.52f, 0.52f, 0.3f));
		}
		Bands.Add(Band);
	}
}

void AJamQuestMarker::BeginPlay()
{
	Super::BeginPlay();
	if (BaseMaterial)
	{
		RedMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		PaleMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		RedMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(2.5f, 0.025f, 0.045f));
		PaleMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.8f, 1.8f, 1.8f));
		RedMaterial->SetScalarParameterValue(TEXT("Opacity"), 1.f);
		PaleMaterial->SetScalarParameterValue(TEXT("Opacity"), 1.f);
		for (int32 i = 0; i < Bands.Num(); ++i) { Bands[i]->SetMaterial(0, i % 2 == 0 && i != 20 ? RedMaterial : PaleMaterial); }
	}
}

FVector AJamQuestMarker::TargetLocation() const
{
	return Anchor.IsValid() ? Anchor->GetActorLocation() : Base;
}

AJamQuestMarker* AJamQuestMarker::FindActive(UWorld* World)
{
	const AJamGameState* GS = World ? World->GetGameState<AJamGameState>() : nullptr;
	if (!GS || !GS->bCompass) { return nullptr; }
	AJamQuestMarker* Best = nullptr;
	float Distance = FMath::Square(200.f);
	for (TActorIterator<AJamQuestMarker> It(World); It; ++It)
	{
		if (It->IsHidden() || It->bFarewell) { continue; }
		const float D = FVector::DistSquared2D(It->TargetLocation(), GS->CompassTarget);
		if (D < Distance || (D == Distance && (!Best || It->GetUniqueID() < Best->GetUniqueID())))
		{
			Best = *It;
			Distance = D;
		}
	}
	return Best;
}

void AJamQuestMarker::PointAt(const FVector& Ground, AActor* InAnchor, float Height)
{
	Base = Ground;
	Anchor = InAnchor;
	Lift = Height;
	SetActorLocation(TargetLocation() + FVector(0, 0, Lift));
	SetActorHiddenInGame(false);
}

AJamQuestMarker* AJamQuestMarker::SpawnFarewell(UWorld* World, const FVector& Where, bool bSpin)
{
	if (!World) { return nullptr; }
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AJamQuestMarker* Ghost = World->SpawnActor<AJamQuestMarker>(AJamQuestMarker::StaticClass(), Where, FRotator::ZeroRotator, Params);
	if (!Ghost) { return nullptr; }
	Ghost->bFarewell = true;
	Ghost->bFarewellSpin = bSpin;
	Ghost->SetActorHiddenInGame(false);
	return Ghost;
}

void AJamQuestMarker::TickFarewell(float DeltaSeconds)
{
	FarewellAge += DeltaSeconds;
	const float Duration = bFarewellSpin ? 0.9f : 0.5f;
	const float T = FMath::Clamp(FarewellAge / Duration, 0.f, 1.f);
	if (T >= 1.f) { Destroy(); return; }
	// Arrival: swell and fade. Turn-in: a fast spin that accelerates, rising a little, shrinking away at the end.
	const float Scale = bFarewellSpin ? 0.85f * (1.f + 0.15f * T) * (1.f - FMath::Square(FMath::Max(0.f, (T - 0.6f) / 0.4f)))
		: 0.85f * (1.f + 1.0f * FMath::InterpEaseOut(0.f, 1.f, T, 2.f));
	const float Opacity = bFarewellSpin ? 1.f - FMath::Max(0.f, (T - 0.7f) / 0.3f) : 1.f - T * T;
	if (RedMaterial) { RedMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity); }
	if (PaleMaterial) { PaleMaterial->SetScalarParameterValue(TEXT("Opacity"), Opacity); }
	SetActorScale3D(FVector(FMath::Max(Scale, 0.01f)));
	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	const float Facing = Camera ? (Camera->GetCameraLocation() - GetActorLocation()).Rotation().Yaw + 90.f : 0.f;
	const float Spin = bFarewellSpin ? 1080.f * T * T : 100.f * FarewellAge;
	SetActorRotation(FRotator(0, Facing + Spin, 0));
	if (bFarewellSpin) { AddActorWorldOffset(FVector(0, 0, 60.f * DeltaSeconds)); }
}

void AJamQuestMarker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bFarewell) { TickFarewell(DeltaSeconds); return; }
	if (IsHidden()) { return; }
	Age += DeltaSeconds;
	const bool bActive = FindActive(GetWorld()) == this;
	if (bActive != bActiveStyle)
	{
		bActiveStyle = bActive;
		if (PaleMaterial) { PaleMaterial->SetVectorParameterValue(TEXT("Color"), bActive ? FLinearColor(1.5f, 0.65f, 0.04f) : FLinearColor(1.8f, 1.8f, 1.8f)); }
	}
	// Readable at distance, but shorter than a character when standing beside Fenella.
	SetActorScale3D(FVector(bActive ? 0.85f : 0.6f));
	SetActorLocation(TargetLocation() + FVector(0, 0, Lift + (bActive ? 26.f * FMath::Sin(Age * 4.f) : 0.f)));
	const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	const float Facing = Camera ? (Camera->GetCameraLocation() - GetActorLocation()).Rotation().Yaw + 90.f : 0.f;
	SetActorRotation(FRotator(0, Facing + (bActive ? Age * 100.f : 0.f), 0));
}
