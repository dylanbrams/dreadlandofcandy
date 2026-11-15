#include "Game/JamComicCard.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/ConstructorHelpers.h"

AJamComicCard::AJamComicCard()
{
	PrimaryActorTick.bCanEverTick = true;
	Quad = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Quad"));
	RootComponent = Quad;
	Quad->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Quad->SetCastShadow(false);
	Quad->SetGenerateOverlapEvents(false);
	Quad->SetMobility(EComponentMobility::Movable);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/FX/M_ComicCard.M_ComicCard"));
	if (Plane.Succeeded()) { Quad->SetStaticMesh(Plane.Object); }
	if (Material.Succeeded()) { Quad->SetMaterial(0, Material.Object); }
	SetActorEnableCollision(false);
}

void AJamComicCard::Launch(UTexture2D* Card, const FVector& InVelocity, float InDelay, float InLife, float InSize)
{
	Start = GetActorLocation();
	Velocity = InVelocity;
	Delay = InDelay;
	Life = FMath::Max(0.2f, InLife);
	Size = InSize;
	// Mostly a pitch/yaw flip (out of the screen plane), a little roll: a real 3D tumble.
	TumbleAxis = FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-0.3f, 0.3f)).GetSafeNormal(UE_SMALL_NUMBER, FVector::ForwardVector);
	TumbleRate = FMath::FRandRange(140.f, 260.f) * (FMath::RandBool() ? 1.f : -1.f);
	RollRate = FMath::FRandRange(-30.f, 30.f);
	if (Quad)
	{
		Mid = Quad->CreateAndSetMaterialInstanceDynamic(0);
		if (Mid && Card) { Mid->SetTextureParameterValue(TEXT("Card"), Card); }
	}
	SetActorHiddenInGame(true);
	SetActorScale3D(FVector(0.01f));
}

void AJamComicCard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;
	const float T = Age - Delay;
	if (T < 0.f) { return; }
	if (T >= Life) { Destroy(); return; }
	if (IsHidden()) { SetActorHiddenInGame(false); }

	// Flight: outward and up, slowing, with a touch of gravity at the end.
	const float Ease = (1.f - FMath::Exp(-2.2f * T)) / 2.2f;   // integral of a decaying speed
	SetActorLocation(Start + Velocity * Ease + FVector(0.f, 0.f, -60.f * T * T));

	// Face the camera (plane's +Z toward it, texture upright), then roll and tumble on top.
	const APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0);
	FQuat Face = GetActorQuat();
	if (Cam)
	{
		const FVector ToCam = (Cam->GetCameraLocation() - GetActorLocation()).GetSafeNormal();
		// The plane's U runs along its X: point X at camera right so the lettering reads left to right.
		const FVector CamRight = Cam->GetCameraRotation().Quaternion().GetRightVector();
		Face = FRotationMatrix::MakeFromZX(ToCam, CamRight).ToQuat();
	}
	// Readable for the first quarter second, then the tumble ramps in.
	const float TumbleTime = FMath::Max(0.f, T - 0.25f);
	const float TumbleAngle = TumbleRate * TumbleTime * FMath::Clamp(TumbleTime / 0.3f, 0.f, 1.f);
	const FQuat Roll(FVector::UpVector, FMath::DegreesToRadians(RollRate * T));
	const FQuat Tumble(TumbleAxis, FMath::DegreesToRadians(TumbleAngle));
	SetActorRotation(Face * Roll * Tumble);

	// Pop in with an overshoot, fade over the last third.
	const float Pop = T < 0.12f ? FMath::Lerp(0.3f, 1.15f, T / 0.12f) : FMath::Lerp(1.15f, 1.f, FMath::Clamp((T - 0.12f) / 0.1f, 0.f, 1.f));
	SetActorScale3D(FVector(Size / 100.f * Pop));
	if (Mid) { Mid->SetScalarParameterValue(TEXT("Fade"), FMath::Clamp((Life - T) / (Life * 0.35f), 0.f, 1.f)); }
}
