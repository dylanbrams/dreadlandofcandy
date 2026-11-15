#include "Game/JamTourPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "MegaJam.h"

AJamTourPawn::AJamTourPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	SetCanBeDamaged(false);
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
	Camera->SetFieldOfView(72.f);
	Camera->bUsePawnControlRotation = false;
}

void AJamTourPawn::Begin(float InSpeed, float InHeight, float StartFraction)
{
	const FJamWorldDef& World = JamWorld::Def();
	Path.Build(JamTour::Waypoints(World));
	Stops = JamTour::Stops(World);
	StopGround.Init(NAN, Stops.Num());
	Speed = FMath::Max(50.f, InSpeed);
	Height = FMath::Max(120.f, InHeight);
	Distance = Path.Length() * FMath::Clamp(StartFraction, 0.f, 1.f);
	UE_LOG(LogJam, Log, TEXT("JamTour: %d waypoints, loop %.0f uu = %.0f s at %.0f uu/s (slower near places), %d places"),
		JamTour::Waypoints(World).Num(), Path.Length(), Path.Length() / Speed, Speed, Stops.Num());
}

float AJamTourPawn::GroundAt(const FVector2D& At)
{
	FHitResult Hit;
	FCollisionQueryParams Params(TEXT("JamTourGround"), false, this);
	if (GetWorld()->LineTraceSingleByChannel(Hit, FVector(At, 50000.f), FVector(At, -50000.f), ECC_WorldStatic, Params))
	{
		LastGround = Hit.ImpactPoint.Z;
	}
	return LastGround;
}

float AJamTourPawn::StopGroundAt(int32 Index)
{
	if (FMath::IsNaN(StopGround[Index])) { StopGround[Index] = GroundAt(Stops[Index].Position); }
	return StopGround[Index];
}

void AJamTourPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Path.IsValid() || DeltaSeconds <= 0.f) { return; }
	DeltaSeconds = FMath::Min(DeltaSeconds, 0.1f);   // a hitch must not teleport the camera
	Age += DeltaSeconds;

	// Along the loop: eased in from a standstill, slower near places.
	const float Ramp = FMath::SmoothStep(0.f, 1.f, Age / 3.f);
	Distance += Speed * Ramp * JamTour::SpeedScale(Stops, Path.PointAt(Distance)) * DeltaSeconds;
	const int32 NowLaps = FMath::FloorToInt(Distance / Path.Length());
	if (NowLaps != Laps) { Laps = NowLaps; UE_LOG(LogJam, Log, TEXT("JamTour: lap %d complete"), Laps); }

	const FVector2D Here = Path.PointAt(Distance);

	// Height: above the highest ground under here and just ahead, rising quickly, settling slowly.
	float Ground = GroundAt(Here);
	for (const float Ahead : { 500.f, 1100.f }) { Ground = FMath::Max(Ground, GroundAt(Path.PointAt(Distance + Ahead))); }
	if (!bStarted) { Z = Ground + Height; }
	const float Target = Ground + Height + Boost;
	Z = Target > Z ? FMath::FInterpTo(Z, Target, DeltaSeconds, 2.5f) : FMath::FInterpTo(Z, Target, DeltaSeconds, 0.7f);
	Z = FMath::Max(Z, GroundAt(Here) + 150.f);
	const FVector Location(Here, Z);

	// Something solid straight ahead (a tall candy prop, a tree): climb until the way is clear.
	FHitResult Hit;
	FCollisionQueryParams Params(TEXT("JamTourAhead"), false, this);
	const bool bBlocked = GetWorld()->SweepSingleByChannel(Hit, Location, Location + View.Vector() * 900.f, FQuat::Identity, ECC_WorldStatic, FCollisionShape::MakeSphere(110.f), Params);
	Boost = bBlocked ? FMath::Min(Boost + 600.f * DeltaSeconds, 3000.f) : FMath::Max(Boost - 200.f * DeltaSeconds, 0.f);

	// Looking: down the loop, glancing at the nearest camp or set while passing it.
	FVector Look(Path.PointAt(Distance + 1800.f), Z - 160.f);
	int32 Stop = INDEX_NONE;
	const float Gaze = JamTour::GazeWeight(Stops, Here, Stop);
	if (Stop != INDEX_NONE)
	{
		const FVector Place(Stops[Stop].Position, StopGroundAt(Stop) + 200.f);
		Look = FMath::Lerp(Look, Place, 0.6f * FMath::SmoothStep(0.f, 1.f, Gaze));
		if (Gaze > 0.5f && Stop != LastLoggedStop) { LastLoggedStop = Stop; UE_LOG(LogJam, Log, TEXT("JamTour: passing %s at t=%.0f s"), *Stops[Stop].Id.ToString(), Age); }
	}
	FRotator Desired = UKismetMathLibrary::FindLookAtRotation(Location, Look);
	Desired.Pitch = FMath::Clamp(Desired.Pitch, -28.f, 8.f);
	Desired.Roll = 0.f;
	View = bStarted ? FMath::RInterpTo(View, Desired, DeltaSeconds, 1.0f) : Desired;
	bStarted = true;

	// A very slow drift so the shot is never mechanically level.
	const FRotator Sway(1.1f * FMath::Sin(Age * 0.31f), 1.6f * FMath::Sin(Age * 0.17f + 1.f), 0.6f * FMath::Sin(Age * 0.23f));
	SetActorLocationAndRotation(Location, View + Sway);
}
