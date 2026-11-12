#include "Game/JamOpenWorld.h"
#include "Game/JamOpenTuning.h"
#include "Game/JamArt.h"
#include "World/JamWorld.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace JamOpenWorld
{
	namespace
	{
		const TArray<FString>& Palette(EJamBiome Biome)
		{
			static const TArray<FString> Meadow = { TEXT("SM_CandyCrop"), TEXT("SM_CandyCrop"), TEXT("SM_CandyTree"), TEXT("SM_CandyFence"), TEXT("SM_Boulder") };
			static const TArray<FString> Molars = { TEXT("SM_Molar"), TEXT("SM_Boulder"), TEXT("SM_Boulder") };
			static const TArray<FString> Orchard = { TEXT("SM_CandyTree"), TEXT("SM_CandyTree"), TEXT("SM_Peppermint") };
			static const TArray<FString> None;
			switch (Biome)
			{
			case EJamBiome::Meadow:  return Meadow;
			case EJamBiome::Molars:  return Molars;
			case EJamBiome::Orchard: return Orchard;
			default:                 return None;
			}
		}

		bool IsClear(const FVector2D& At)
		{
			for (const FJamLocation& L : JamWorld::Def().Locations)
			{
				if (L.Kind == EJamLocationKind::Village) { continue; }
				// Film sets are square: clear their half diagonal.
				const float R = (L.Kind == EJamLocationKind::FilmSet ? L.Radius * UE_SQRT_2 : L.Radius) + JamOpenTuning::ScatterClearance;
				if (FVector2D::DistSquared(At, L.Position) < R * R) { return false; }
			}
			return true;
		}
	}

	TArray<FJamProp> Scatter(int32 Seed)
	{
		using namespace JamOpenTuning;
		const FJamWorldDef& W = JamWorld::Def();
		TArray<FJamProp> Out;

		// Houses round the square, fronts to the centre, leaving gaps where the three roads leave.
		for (int32 i = 0; i < VillageHouses; ++i)
		{
			const float Bearing = (i + 0.5f) * 360.f / VillageHouses;
			bool bOnRoad = false;
			for (const FJamBiomeDef& B : W.Biomes)
			{
				if (B.Biome != EJamBiome::Village && FMath::Abs(FMath::FindDeltaAngleDegrees(Bearing, B.WedgeCentreYaw)) < 20.f) { bOnRoad = true; }
			}
			const FVector2D At = FVector2D(FMath::Cos(FMath::DegreesToRadians(Bearing)), FMath::Sin(FMath::DegreesToRadians(Bearing))) * VillageHouseRing;
			if (bOnRoad || !IsClear(At)) { continue; }
			Out.Add({ TEXT("SM_HouseFront"), At, Bearing + 90.f });   // a house front faces +Y at yaw 0
		}

		// Biome props: one candidate per cell, jittered, kept off the village square and every location.
		FRandomStream Rng(Seed);
		for (float X = W.Bounds.Min.X + ScatterSpacing * 0.5f; X < W.Bounds.Max.X; X += ScatterSpacing)
		{
			for (float Y = W.Bounds.Min.Y + ScatterSpacing * 0.5f; Y < W.Bounds.Max.Y; Y += ScatterSpacing)
			{
				const FVector2D At(X + Rng.FRandRange(-0.4f, 0.4f) * ScatterSpacing, Y + Rng.FRandRange(-0.4f, 0.4f) * ScatterSpacing);
				const float Roll = Rng.FRand();
				const float Yaw = Rng.FRandRange(0.f, 360.f);
				const int32 Pick = Rng.RandHelper(1000);
				if (Roll > ScatterChance || !W.Bounds.IsInside(At)) { continue; }
				if (At.Size() < W.VillageRadius + ScatterClearance || !IsClear(At)) { continue; }
				const TArray<FString>& P = Palette(JamWorld::BiomeAt(At));
				if (P.Num() == 0) { continue; }
				Out.Add({ P[Pick % P.Num()], At, Yaw });
			}
		}
		return Out;
	}

	bool IsFallbackMap(const FString& MapName)
	{
		return MapName.Contains(TEXT("Template_Default"));
	}
}

AJamOpenWorld::AJamOpenWorld()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	CubeMesh = Cube.Object;
	CylinderMesh = Cylinder.Object;
	ShapeMaterial = Material.Object;
}

UStaticMeshComponent* AJamOpenWorld::AddShape(UStaticMesh* Mesh, const FVector& Centre, const FVector& Scale, const FLinearColor& Colour, bool bCollide)
{
	if (!Mesh) { return nullptr; }
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	C->SetStaticMesh(Mesh);
	C->SetMobility(EComponentMobility::Static);
	C->SetCollisionProfileName(bCollide ? UCollisionProfile::BlockAll_ProfileName : UCollisionProfile::NoCollision_ProfileName);
	C->SetWorldLocationAndRotation(Centre, FRotator::ZeroRotator);
	C->SetWorldScale3D(Scale);   // the engine's basic shapes are 100 uu across, centred on their origin
	if (ShapeMaterial)
	{
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(ShapeMaterial, C);
		M->SetVectorParameterValue(TEXT("Color"), Colour);
		C->SetMaterial(0, M);
	}
	C->SetupAttachment(RootComponent);
	C->RegisterComponent();
	AddInstanceComponent(C);
	return C;
}

void AJamOpenWorld::AddLabel(const FString& Text, const FVector& At)
{
	UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
	Label->SetText(FText::FromString(Text));
	Label->SetHorizontalAlignment(EHTA_Center);
	Label->SetVerticalAlignment(EVRTA_TextCenter);
	Label->SetWorldSize(160.f);
	Label->SetTextRenderColor(FColor(255, 230, 150));
	Label->SetWorldLocationAndRotation(At, FRotator(60.f, 180.f, 0.f));   // tilted up, readable from above and behind
	Label->SetupAttachment(RootComponent);
	Label->RegisterComponent();
	AddInstanceComponent(Label);
}

void AJamOpenWorld::Build(float GroundZ)
{
	using namespace JamOpenTuning;
	const FJamWorldDef& W = JamWorld::Def();

	// Ground: tiles tinted by the biome at their centre, colliding (the template's own floor is far smaller).
	const float Thickness = 40.f;
	for (float X = W.Bounds.Min.X; X < W.Bounds.Max.X; X += TileSize)
	{
		for (float Y = W.Bounds.Min.Y; Y < W.Bounds.Max.Y; Y += TileSize)
		{
			const FVector2D Centre(X + TileSize * 0.5f, Y + TileSize * 0.5f);
			const FJamBiomeDef* B = JamWorld::Biome(JamWorld::BiomeAt(Centre));
			AddShape(CubeMesh, FVector(Centre, GroundZ - Thickness * 0.5f), FVector(TileSize / 100.f, TileSize / 100.f, Thickness / 100.f), B ? B->Tint : FLinearColor::Gray, true);
		}
	}
	// The village square reads as a disc of its own colour, flush with the tiles.
	if (const FJamBiomeDef* V = JamWorld::Biome(EJamBiome::Village))
	{
		AddShape(CylinderMesh, FVector(0.f, 0.f, GroundZ + 0.5f), FVector(W.VillageRadius / 50.f, W.VillageRadius / 50.f, 0.01f), V->Tint, false);
	}

	for (const FJamLocation& L : W.Locations)
	{
		const FVector At(L.Position, GroundZ);
		switch (L.Kind)
		{
		case EJamLocationKind::FilmSet:
		{
			const FBox2D Floor = JamWorld::FilmSetFloor(L);
			const FLinearColor Colour = JamBacklot::Sets().IsValidIndex(L.SceneIndex) ? JamBacklot::Sets()[L.SceneIndex].Floor : FLinearColor::Gray;
			AddShape(CubeMesh, FVector(Floor.GetCenter(), GroundZ - 9.f), FVector(Floor.GetSize().X / 100.f, Floor.GetSize().Y / 100.f, 0.2f), Colour, true);
			AddLabel(FString::Printf(TEXT("SET %d"), L.SceneIndex + 1), At + FVector(0.f, 0.f, 300.f));
			break;
		}
		case EJamLocationKind::Camp:
			AddShape(CylinderMesh, At + FVector(0.f, 0.f, 1.f), FVector(L.Radius / 50.f, L.Radius / 50.f, 0.02f), FLinearColor(0.35f, 0.25f, 0.2f), false);
			AddLabel(L.Id.ToString(), At + FVector(0.f, 0.f, 250.f));
			break;
		case EJamLocationKind::Giver:
			AddShape(CylinderMesh, At + FVector(0.f, 0.f, 1.f), FVector(L.Radius / 50.f, L.Radius / 50.f, 0.02f), FLinearColor(0.9f, 0.75f, 0.2f), false);
			break;
		default:   // the village is the disc above; landmarks are props (the signpost, below)
			break;
		}
	}

	TArray<FJamProp> Props = JamOpenWorld::Scatter(ScatterSeed);
	if (const FJamLocation* Sign = JamWorld::Find(TEXT("Signpost"))) { Props.Add({ TEXT("SM_Signpost"), Sign->Position, Sign->Yaw }); }
	for (const FJamProp& Prop : Props)
	{
		if (!JamArt::Load(Prop.Mesh)) { continue; }   // -noart: the fallback world stays bare
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
