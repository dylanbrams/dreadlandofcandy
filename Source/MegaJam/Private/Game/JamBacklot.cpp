#include "Game/JamBacklot.h"
#include "Game/JamArt.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace JamBacklot
{
	const TArray<FJamSetLayout>& Sets()
	{
		// Street lengths of 4500 uu are about 7.5 s at the hero's 600 uu/s. BUILD_SPEC asks for about 15 s
		// once the street has its gags (T24); lengthen these then, nothing else depends on them.
		static const TArray<FJamSetLayout> Table = {
			{ TEXT("Field"),   TEXT("SET 1: THE FIELD"),       FBox2D(FVector2D(0.f, -900.f),      FVector2D(1800.f, 900.f)),   FLinearColor(0.20f, 0.45f, 0.16f) },
			{ TEXT("Fort"),    TEXT("SET 2: FORT TOOTHACHE"),  FBox2D(FVector2D(6300.f, -700.f),   FVector2D(7700.f, 700.f)),   FLinearColor(0.55f, 0.42f, 0.45f) },
			{ TEXT("Orchard"), TEXT("SET 3: THE ORCHARD"),     FBox2D(FVector2D(12200.f, -1000.f), FVector2D(14200.f, 1000.f)), FLinearColor(0.16f, 0.32f, 0.12f) },
		};
		return Table;
	}

	TArray<FBox2D> Streets()
	{
		TArray<FBox2D> Out;
		const TArray<FJamSetLayout>& S = Sets();
		for (int32 i = 0; i + 1 < S.Num(); ++i)
		{
			Out.Add(FBox2D(FVector2D(S[i].Bounds.Max.X, -StreetWidth * 0.5f), FVector2D(S[i + 1].Bounds.Min.X, StreetWidth * 0.5f)));
		}
		return Out;
	}

	const TArray<FJamProp>& Dressing()
	{
		static const TArray<FJamProp> Table = []()
		{
			TArray<FJamProp> P;
			const TArray<FJamSetLayout>& S = Sets();
			// Streets: candy houses face the street from both sides, trees behind them. Fronts face +Y after the
			// art's forward fix, so the south row (−Y) faces the street as is and the north row turns round.
			for (const FBox2D& Street : Streets())
			{
				int32 n = 0;
				for (float X = Street.Min.X + 500.f; X < Street.Max.X - 300.f; X += 950.f, ++n)
				{
					const bool bLeft = (n % 2) == 0;
					P.Add({ TEXT("SM_HouseFront"), FVector2D(X, bLeft ? -780.f : 780.f), bLeft ? 0.f : 180.f });
					P.Add({ TEXT("SM_CandyTree"), FVector2D(X + 450.f, bLeft ? 1250.f : -1250.f), 0.f });
				}
				P.Add({ TEXT("SM_Signpost"), FVector2D(Street.Min.X + 150.f, -420.f), 90.f });
				P.Add({ TEXT("SM_Stanchion"), FVector2D(Street.Max.X - 120.f, -390.f), 0.f });
				P.Add({ TEXT("SM_Stanchion"), FVector2D(Street.Max.X - 120.f, 390.f), 0.f });
			}
			// Every set: film gear in the back corners, where the hero starts and the nuisances rarely go.
			for (const FJamSetLayout& Set : S)
			{
				const FBox2D& B = Set.Bounds;
				P.Add({ TEXT("SM_DirectorsChair"), FVector2D(B.Min.X + 120.f, B.Max.Y - 160.f), -60.f });
				P.Add({ TEXT("SM_LightStand"), FVector2D(B.Min.X + 90.f, B.Min.Y + 110.f), 45.f });
				P.Add({ TEXT("SM_LightStand"), FVector2D(B.Max.X - 90.f, B.Max.Y - 110.f), -135.f });
				P.Add({ TEXT("SM_Clapperboard"), FVector2D(B.Min.X + 220.f, B.Max.Y - 90.f), 20.f });
			}
			// The field (Scene 1): candy crops in rows outside both walls, fences along them.
			{
				const FBox2D& B = S[0].Bounds;
				for (float X = B.Min.X + 150.f; X < B.Max.X; X += 260.f)
				{
					for (float Y : { B.Min.Y - 220.f, B.Min.Y - 420.f, B.Max.Y + 220.f, B.Max.Y + 420.f }) { P.Add({ TEXT("SM_CandyCrop"), FVector2D(X, Y), 0.f }); }
				}
				for (float X = B.Min.X + 200.f; X < B.Max.X; X += 420.f)
				{
					P.Add({ TEXT("SM_CandyFence"), FVector2D(X, B.Min.Y - 90.f), 90.f });
					P.Add({ TEXT("SM_CandyFence"), FVector2D(X, B.Max.Y + 90.f), 90.f });
				}
				P.Add({ TEXT("SM_CandyTree"), FVector2D(B.Max.X - 100.f, B.Min.Y - 750.f), 0.f });
				P.Add({ TEXT("SM_Boulder"), FVector2D(B.Min.X + 600.f, B.Max.Y + 750.f), 30.f });
			}
			// Fort Toothache (Scene 2): molar towers at the corners, the gate at the far end.
			{
				const FBox2D& B = S[1].Bounds;
				for (const FVector2D& C : { FVector2D(B.Min.X, B.Min.Y), FVector2D(B.Min.X, B.Max.Y), FVector2D(B.Max.X, B.Min.Y), FVector2D(B.Max.X, B.Max.Y) })
				{
					P.Add({ TEXT("SM_Molar"), C + FVector2D(C.X > B.GetCenter().X ? 150.f : -150.f, C.Y > 0.f ? 150.f : -150.f), 0.f, 380.f });
				}
				// The gate arches over the street at the fort's entrance: the hero walks up to it and through it.
				P.Add({ TEXT("SM_FortGate"), FVector2D(B.Min.X - 80.f, 0.f), 90.f });
				P.Add({ TEXT("SM_Toothbrush"), FVector2D(B.Min.X + 150.f, B.Min.Y + 260.f), 70.f, 120.f });
			}
			// The orchard (Scene 3): candy trees round the outside, crops and peppermints along the inner edges.
			{
				const FBox2D& B = S[2].Bounds;
				for (float X = B.Min.X + 200.f; X < B.Max.X; X += 450.f)
				{
					P.Add({ TEXT("SM_CandyTree"), FVector2D(X, B.Min.Y - 350.f), X * 0.1f });
					P.Add({ TEXT("SM_CandyTree"), FVector2D(X + 200.f, B.Max.Y + 350.f), X * 0.07f });
				}
				for (float X = B.Min.X + 500.f; X < B.Max.X - 200.f; X += 300.f)
				{
					P.Add({ TEXT("SM_CandyCrop"), FVector2D(X, B.Min.Y + 90.f), 0.f });
					P.Add({ TEXT("SM_CandyCrop"), FVector2D(X, B.Max.Y - 90.f), 0.f });
				}
				P.Add({ TEXT("SM_Peppermint"), FVector2D(B.Max.X + 300.f, -500.f), 0.f, 140.f });
				P.Add({ TEXT("SM_Peppermint"), FVector2D(B.Max.X + 300.f, 500.f), 0.f, 140.f });
				P.Add({ TEXT("SM_Boulder"), FVector2D(B.Max.X + 400.f, 0.f), 0.f, 220.f });
			}
			return P;
		}();
		return Table;
	}

	FVector2D HeroMark(int32 SetIndex)
	{
		const TArray<FJamSetLayout>& S = Sets();
		if (!S.IsValidIndex(SetIndex)) { return FVector2D::ZeroVector; }
		return FVector2D(S[SetIndex].Bounds.Min.X + 300.f, S[SetIndex].Bounds.GetCenter().Y);
	}
}

AJamBacklot::AJamBacklot()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	CubeMesh = Cube.Object;
	ShapeMaterial = Material.Object;
}

void AJamBacklot::AddBox(const FBox2D& Area, float Bottom, float Top, const FLinearColor& Colour, bool bCollide)
{
	if (!CubeMesh) { return; }
	UStaticMeshComponent* Box = NewObject<UStaticMeshComponent>(this);
	Box->SetStaticMesh(CubeMesh);
	Box->SetMobility(EComponentMobility::Static);
	Box->SetCollisionProfileName(bCollide ? UCollisionProfile::BlockAll_ProfileName : UCollisionProfile::NoCollision_ProfileName);
	// The engine cube is 100 uu on a side, centred on its origin.
	const FVector2D Size = Area.GetSize();
	Box->SetWorldLocationAndRotation(FVector(Area.GetCenter(), (Bottom + Top) * 0.5f), FRotator::ZeroRotator);
	Box->SetWorldScale3D(FVector(Size.X / 100.f, Size.Y / 100.f, (Top - Bottom) / 100.f));
	if (ShapeMaterial)
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(ShapeMaterial, Box);
		M->SetVectorParameterValue(TEXT("Color"), Colour);
		Box->SetMaterial(0, M);
	}
	Box->SetupAttachment(RootComponent);
	Box->RegisterComponent();
	AddInstanceComponent(Box);
}

void AJamBacklot::Build(float GroundZ)
{
	using namespace JamBacklot;
	const FLinearColor WallColour(0.75f, 0.68f, 0.55f);    // unpainted plywood
	const FLinearColor StreetColour(0.62f, 0.52f, 0.38f);   // packed sugar-dirt
	const float FloorBottom = GroundZ - 20.f;
	const float WallTop = GroundZ + WallHeight;
	const float T = WallThickness;
	const float HalfGap = StreetWidth * 0.5f;

	const TArray<FJamSetLayout>& S = Sets();
	for (int32 i = 0; i < S.Num(); ++i)
	{
		const FBox2D& B = S[i].Bounds;
		AddBox(B, FloorBottom, GroundZ, S[i].Floor, true);

		// Side walls (Y), full length including the corners.
		AddBox(FBox2D(FVector2D(B.Min.X - T, B.Min.Y - T), FVector2D(B.Max.X + T, B.Min.Y)), GroundZ, WallTop, WallColour, true);
		AddBox(FBox2D(FVector2D(B.Min.X - T, B.Max.Y), FVector2D(B.Max.X + T, B.Max.Y + T)), GroundZ, WallTop, WallColour, true);

		// Back (south, -X) and front (north, +X) walls, with a street-wide opening where a street joins.
		const bool bOpenBack = i > 0;
		const bool bOpenFront = i + 1 < S.Num();
		auto EndWall = [&](float X0, float X1, bool bOpen)
		{
			if (!bOpen)
			{
				AddBox(FBox2D(FVector2D(X0, B.Min.Y), FVector2D(X1, B.Max.Y)), GroundZ, WallTop, WallColour, true);
				return;
			}
			AddBox(FBox2D(FVector2D(X0, B.Min.Y), FVector2D(X1, -HalfGap)), GroundZ, WallTop, WallColour, true);
			AddBox(FBox2D(FVector2D(X0, HalfGap), FVector2D(X1, B.Max.Y)), GroundZ, WallTop, WallColour, true);
		};
		EndWall(B.Min.X - T, B.Min.X, bOpenBack);
		EndWall(B.Max.X, B.Max.X + T, bOpenFront);

		// The set's banner over its entrance from the street, tilted to face the fixed camera (which looks along
		// +X, 50 deg down) so the hero walking up the street reads it ahead. The camera never sees behind the
		// hero, so the field (no entrance) carries its banner over its exit instead.
		UTextRenderComponent* Sign = NewObject<UTextRenderComponent>(this);
		Sign->SetText(FText::FromString(S[i].Sign));
		Sign->SetHorizontalAlignment(EHTA_Center);
		Sign->SetVerticalAlignment(EVRTA_TextCenter);
		Sign->SetWorldSize(80.f);
		Sign->SetTextRenderColor(FColor(255, 230, 150));
		Sign->SetWorldLocationAndRotation(FVector(bOpenBack ? B.Min.X : B.Max.X, 0.f, WallTop + 70.f), FRotator(50.f, 180.f, 0.f));
		Sign->SetupAttachment(RootComponent);
		Sign->RegisterComponent();
		AddInstanceComponent(Sign);
	}

	// The land around the backlot: a candy-green ground under everything, hiding the template's checker floor.
	AddBox(FBox2D(FVector2D(-3000.f, -5000.f), FVector2D(17500.f, 5000.f)), FloorBottom - 10.f, GroundZ - 1.f, FLinearColor(0.42f, 0.62f, 0.38f), false);
	AddDressing(GroundZ);

	for (const FBox2D& Street : Streets())
	{
		AddBox(Street, FloorBottom, GroundZ, StreetColour, true);
		AddBox(FBox2D(FVector2D(Street.Min.X, Street.Min.Y - T), FVector2D(Street.Max.X, Street.Min.Y)), GroundZ, WallTop, WallColour, true);
		AddBox(FBox2D(FVector2D(Street.Min.X, Street.Max.Y), FVector2D(Street.Max.X, Street.Max.Y + T)), GroundZ, WallTop, WallColour, true);
	}
}

void AJamBacklot::AddDressing(float GroundZ)
{
	for (const FJamProp& Prop : JamBacklot::Dressing())
	{
		if (!JamArt::Load(Prop.Mesh)) { continue; }   // -noart, or not imported: the backlot stays bare
		USceneComponent* Spot = NewObject<USceneComponent>(this);
		Spot->SetupAttachment(RootComponent);
		Spot->SetWorldLocationAndRotation(FVector(Prop.At, GroundZ), FRotator(0.f, Prop.Yaw, 0.f));
		Spot->RegisterComponent();
		AddInstanceComponent(Spot);
		UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this);
		Mesh->SetupAttachment(Spot);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->RegisterComponent();
		AddInstanceComponent(Mesh);
		JamArt::Apply(Mesh, Prop.Mesh, Prop.Height > 0.f ? Prop.Height : JamArt::Height(Prop.Mesh));
	}
}
