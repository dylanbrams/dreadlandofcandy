#include "Game/JamFaerie.h"
#include "Game/JamArt.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Game/JamHero.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace JamFaerieTuning
{
	// From the hero: behind (-X is down the screen), to the right, up at head height and over.
	const FVector ShoulderOffset(-90.f, 130.f, 170.f);
	// Open mode (R4): further out, relative to the follow camera's yaw (forward, right, up), so she frames the shot.
	const FVector OpenOffset(-60.f, 320.f, 230.f);
	constexpr float SpotLumens = 250.f;   // Dylan, Nov 13: still too bright at 500
	constexpr float SpotRadius = 2500.f;
	constexpr float FollowSpeed = 4.f;
	constexpr float TurnSpeed = 6.f;
	constexpr float BobHeight = 14.f;
	constexpr float BobRate = 2.2f;
	constexpr float FlapRate = 22.f;
	constexpr float FlapDegrees = 35.f;
}

AJamFaerie::AJamFaerie()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorEnableCollision(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	ShapeMaterial = Mat.Object;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	Pivot->SetupAttachment(Root);
	// Placeholder body reads small at the 1700 uu game camera; scale the whole figure.
	Pivot->SetRelativeScale3D(FVector(1.6f));

	// Basic shapes are 100 uu across. She is small: about 60 uu tall, a head over a cone dress.
	Body = MakePart(TEXT("Body"), Cone.Object, FVector(0.f, 0.f, -10.f), FVector(0.32f, 0.32f, 0.42f));
	Head = MakePart(TEXT("Head"), Sphere.Object, FVector(0.f, 0.f, 26.f), FVector(0.24f));
	WingL = MakePart(TEXT("WingL"), Cube.Object, FVector(-10.f, -22.f, 14.f), FVector(0.03f, 0.42f, 0.26f));
	WingR = MakePart(TEXT("WingR"), Cube.Object, FVector(-10.f, 22.f, 14.f), FVector(0.03f, 0.42f, 0.26f));
	Camcorder = MakePart(TEXT("Camcorder"), Cube.Object, FVector(18.f, 0.f, 6.f), FVector(0.26f, 0.12f, 0.14f));
	Tally = MakePart(TEXT("Tally"), Sphere.Object, FVector(26.f, 0.f, 15.f), FVector(0.04f));
	Art = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Art"));
	Art->SetupAttachment(Root);
	Art->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Spot = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spot"));
	Spot->SetupAttachment(Root);
	Spot->SetUsingAbsoluteRotation(true);
	Spot->SetMobility(EComponentMobility::Movable);
	Spot->IntensityUnits = ELightUnits::Lumens;
	Spot->Intensity = JamFaerieTuning::SpotLumens;
	Spot->AttenuationRadius = JamFaerieTuning::SpotRadius;
	Spot->InnerConeAngle = 6.f;
	Spot->OuterConeAngle = 16.f;
	Spot->SetLightColor(FLinearColor(1.f, 0.93f, 0.82f));
	Spot->CastShadows = false;   // a fill on the hero, not a second shadow
}

UStaticMeshComponent* AJamFaerie::MakePart(const TCHAR* Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale)
{
	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Part->SetupAttachment(Pivot);
	Part->SetStaticMesh(Mesh);
	Part->SetRelativeLocation(Location);
	Part->SetRelativeScale3D(Scale);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetGenerateOverlapEvents(false);
	Part->SetCastShadow(true);
	if (ShapeMaterial) { Part->SetMaterial(0, ShapeMaterial); }
	return Part;
}

void AJamFaerie::Tint(UStaticMeshComponent* Part, const FLinearColor& Colour)
{
	if (UMaterialInstanceDynamic* M = Part->CreateAndSetMaterialInstanceDynamic(0))
	{
		M->SetVectorParameterValue(TEXT("Color"), Colour);
	}
}

void AJamFaerie::BeginPlay()
{
	Super::BeginPlay();
	Tint(Body, FLinearColor(0.55f, 0.3f, 0.85f));     // violet dress
	Tint(Head, FLinearColor(1.f, 0.82f, 0.7f));
	Tint(WingL, FLinearColor(0.7f, 0.95f, 1.f));      // dragonfly wings
	Tint(WingR, FLinearColor(0.7f, 0.95f, 1.f));
	Tint(Camcorder, FLinearColor(0.05f, 0.05f, 0.06f));
	Tint(Tally, FLinearColor(1.f, 0.05f, 0.05f));
	// The generated Fenella replaces the shapes; she still bobs (Art follows the pivot's height in Tick).
	if (JamArt::Apply(Art, TEXT("SM_Fenella"), JamArt::Height(TEXT("SM_Fenella")), -45.f))
	{
		bArt = true;
		ArtBaseZ = Art->GetRelativeLocation().Z;
		for (UStaticMeshComponent* Part : { Body.Get(), Head.Get(), WingL.Get(), WingR.Get(), Camcorder.Get(), Tally.Get() }) { Part->SetHiddenInGame(true); }
	}
}

void AJamFaerie::Speak(float Seconds)
{
	SpeakUntil = Age + Seconds;
}

void AJamFaerie::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	using namespace JamFaerieTuning;
	Age += DeltaSeconds;
	const bool bSpeaking = Age < SpeakUntil;

	if (const AActor* F = Follow.Get())
	{
		const AJamHero* Hero = Cast<AJamHero>(F);
		const FVector Goal = F->GetActorLocation() + (Hero && Hero->IsFollowCamera()
			? FRotator(0.f, Hero->GetCameraYaw(), 0.f).RotateVector(OpenOffset) : ShoulderOffset);
		const FVector Here = GetActorLocation();
		// Snap if far (a teleport, a new scene); otherwise drift after the hero.
		SetActorLocation(FVector::Dist(Here, Goal) > 2000.f ? Goal : FMath::VInterpTo(Here, Goal, DeltaSeconds, FollowSpeed));

		// The spotlight stays on the hero's chest whatever she is watching.
		const FVector Chest = F->GetActorLocation() + FVector(0.f, 0.f, 30.f);
		if (!(Chest - Spot->GetComponentLocation()).IsNearlyZero()) { Spot->SetWorldRotation((Chest - Spot->GetComponentLocation()).Rotation()); }
		const AActor* Watch = Subject.IsValid() ? Subject.Get() : F;
		FVector ToSubject = Watch->GetActorLocation() - GetActorLocation();
		ToSubject.Z = 0.f;
		if (bSulking) { ToSubject = -(F->GetActorLocation() - GetActorLocation()).GetSafeNormal2D(); }   // the bush takes notes
		if (!ToSubject.IsNearlyZero())
		{
			const FRotator Want = ToSubject.Rotation();
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), Want, DeltaSeconds, TurnSpeed));
		}
	}

	const float Rate = bSpeaking ? BobRate * 2.f : BobRate;
	Pivot->SetRelativeLocation(FVector(0.f, 0.f, FMath::Sin(Age * Rate * UE_TWO_PI * 0.5f) * BobHeight));
	if (bArt)
	{
		Art->SetRelativeLocation(FVector(Art->GetRelativeLocation().X, Art->GetRelativeLocation().Y, ArtBaseZ + Pivot->GetRelativeLocation().Z));
	}
	const float Flap = FMath::Sin(Age * (bSpeaking ? FlapRate * 1.5f : FlapRate)) * FlapDegrees;
	WingL->SetRelativeRotation(FRotator(0.f, 0.f, Flap));
	WingR->SetRelativeRotation(FRotator(0.f, 0.f, -Flap));
	// The tally light blinks while she films.
	Tally->SetVisibility(FMath::Fmod(Age, 1.f) < 0.6f);
}
