#include "Game/JamSkillFx.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"

namespace JamFxTuning
{
	constexpr float GrowSeconds = 0.12f;   // from 70% to full size: the lecture "lands"
	constexpr float StartScale = 0.7f;
	constexpr float LiftZ = 4.f;           // above the floor, under the actors' feet
}

AJamSkillFx::AJamSkillFx()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorEnableCollision(false);
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Mat(TEXT("/Game/FX/M_FxUnlit.M_FxUnlit"));
	BaseMaterial = Mat.Object;
}

AJamSkillFx* AJamSkillFx::Spawn(UWorld* World, EJamFxShape Shape, const FVector& Origin, const FVector& Forward,
	float Radius, float Width, const FLinearColor& Colour, float Seconds)
{
	if (!World) { return nullptr; }
	const FRotator Facing(0.f, Forward.Rotation().Yaw, 0.f);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AJamSkillFx* Fx = World->SpawnActor<AJamSkillFx>(AJamSkillFx::StaticClass(), FTransform(Facing, Origin), P);
	if (!Fx) { return nullptr; }
	if (!Fx->BaseMaterial) { Fx->Destroy(); return nullptr; }
	Fx->Duration = FMath::Max(0.15f, Seconds);
	Fx->Material = UMaterialInstanceDynamic::Create(Fx->BaseMaterial, Fx);
	Fx->Material->SetVectorParameterValue(TEXT("Color"), Colour);
	Fx->Material->SetScalarParameterValue(TEXT("Opacity"), 0.f);
	Fx->Build(Shape, Radius, Width);
	Fx->Mesh->SetMaterial(0, Fx->Material);
	Fx->SetActorScale3D(FVector(JamFxTuning::StartScale));
	return Fx;
}

void AJamSkillFx::Build(EJamFxShape Shape, float Radius, float Width)
{
	TArray<FVector> V;
	TArray<int32> T;
	TArray<FLinearColor> C;
	const float Z = JamFxTuning::LiftZ;
	// Faint at the hero, strong at the edge: reads as a wave going out.
	const FLinearColor Inner(1.f, 1.f, 1.f, 0.25f), Outer(1.f, 1.f, 1.f, 1.f);

	auto Tri = [&T](int32 A, int32 B, int32 D) { T.Add(A); T.Add(B); T.Add(D); };
	if (Shape == EJamFxShape::Strip)
	{
		// Two quads, near half and far half, so the alpha ramps along the line.
		const float W = Width;
		V = { FVector(0, -W, Z), FVector(0, W, Z), FVector(Radius * 0.5f, -W, Z), FVector(Radius * 0.5f, W, Z), FVector(Radius, -W, Z), FVector(Radius, W, Z) };
		C = { Inner, Inner, FLinearColor(1, 1, 1, 0.6f), FLinearColor(1, 1, 1, 0.6f), Outer, Outer };
		Tri(0, 2, 1); Tri(1, 2, 3); Tri(2, 4, 3); Tri(3, 4, 5);
	}
	else
	{
		const bool bFan = Shape == EJamFxShape::Fan;
		const float Half = bFan ? FMath::DegreesToRadians(Width) : PI;
		const int32 Segments = bFan ? 20 : 48;
		V.Add(FVector(0, 0, Z));
		C.Add(Inner);
		for (int32 i = 0; i <= Segments; ++i)
		{
			const float A = -Half + 2.f * Half * i / Segments;
			V.Add(FVector(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius, Z));
			C.Add(Outer);
		}
		for (int32 i = 1; i <= Segments; ++i) { Tri(0, i + 1, i); }
	}
	TArray<FVector> Normals;
	Normals.Init(FVector::UpVector, V.Num());
	Mesh->CreateMeshSection_LinearColor(0, V, T, Normals, TArray<FVector2D>(), C, TArray<FProcMeshTangent>(), false);
}

void AJamSkillFx::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;
	const float Grow = FMath::Clamp(Age / JamFxTuning::GrowSeconds, 0.f, 1.f);
	SetActorScale3D(FVector(FMath::Lerp(JamFxTuning::StartScale, 1.f, FMath::InterpEaseOut(0.f, 1.f, Grow, 2.f))));
	const float Fade = FMath::Clamp(1.f - Age / Duration, 0.f, 1.f);
	if (Material) { Material->SetScalarParameterValue(TEXT("Opacity"), PeakOpacity * Fade * Fade); }
	if (Age >= Duration) { Destroy(); }
}
