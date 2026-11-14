#include "World/JamWorld.h"

// =====================================================================================================
// TUNING SURFACE (BUILD_SPEC 3e). TERRAIN owns the rows (positions, radii, biomes); TUNING may change any
// number here. The structs live in Public/World/JamWorld.h and are GAME's. After a change: build,
// run-tests.ps1 (Jam.World.* guards the rules), and both autoplay modes.
//
// Layout: the village disc at the origin, three biome wedges round it (centre yaw: 0 = +X = north,
// 90 = +Y = east). Locations are placed by bearing and distance from the village square: Polar(yaw, dist).
// =====================================================================================================

namespace JamWorldTuning
{
	constexpr float HalfSize = 12000.f;        // playable square is 24,000 uu
	constexpr float VillageRadius = 2500.f;
	constexpr float CampRadius = 600.f;        // camps are 1,200 uu across

	constexpr float MeadowYaw = 0.f;
	constexpr float MolarsYaw = 120.f;
	constexpr float OrchardYaw = -120.f;
}

namespace JamWorldTableRows   // named, not anonymous: the game target is a unity build
{
	using namespace JamWorldTuning;
	using K = EJamTargetKind;

	FVector2D Polar(float YawDeg, float Distance)
	{
		const float R = FMath::DegreesToRadians(YawDeg);
		return FVector2D(FMath::Cos(R), FMath::Sin(R)) * Distance;
	}

	FJamLocation Loc(const TCHAR* Id, EJamLocationKind Kind, EJamBiome Biome, const FVector2D& At, float Radius, float Yaw = 0.f)
	{
		FJamLocation L;
		L.Id = Id; L.Kind = Kind; L.Biome = Biome; L.Position = At; L.Radius = Radius; L.Yaw = Yaw;
		return L;
	}

	FJamLocation Camp(const TCHAR* Id, EJamBiome Biome, const FVector2D& At, TArray<EJamTargetKind> Kinds, int32 Capacity)
	{
		FJamLocation L = Loc(Id, EJamLocationKind::Camp, Biome, At, CampRadius);
		L.Kinds = MoveTemp(Kinds);
		L.Capacity = Capacity;
		return L;
	}

	FJamLocation Set(const TCHAR* Id, EJamBiome Biome, const FVector2D& At, float HalfSide, int32 SceneIndex)
	{
		// Faces the village, so the hero walks in through the near edge as on the backlot.
		FJamLocation L = Loc(Id, EJamLocationKind::FilmSet, Biome, At, HalfSide, FMath::RadiansToDegrees(FMath::Atan2(-At.Y, -At.X)));
		L.SceneIndex = SceneIndex;
		return L;
	}

	FJamWorldDef Build()
	{
		FJamWorldDef W;
		W.Bounds = FBox2D(FVector2D(-HalfSize, -HalfSize), FVector2D(HalfSize, HalfSize));
		W.VillageRadius = VillageRadius;

		// Biomes. Display names are placeholders until Jokes.md names them. Grade offsets add to JamLook's.
		auto Biome = [](EJamBiome B, const TCHAR* Name, TArray<EJamTargetKind> Kinds, FLinearColor Tint, const TCHAR* Cue, EJamWeather Weather, FJamGradeOffset Grade, float Yaw)
		{
			FJamBiomeDef D;
			D.Biome = B; D.DisplayName = Name; D.Kinds = MoveTemp(Kinds); D.Tint = Tint; D.MusicCue = Cue;
			D.Weather = Weather; D.Grade = Grade; D.WedgeCentreYaw = Yaw;
			return D;
		};
		W.Biomes = {
			Biome(EJamBiome::Village, TEXT("Sugarborough"),        {},                       FLinearColor(0.62f, 0.50f, 0.40f), TEXT("village"), EJamWeather::None, {}, 0.f),
			Biome(EJamBiome::Meadow,  TEXT("The Gumdrop Meadow"),  { K::Rat, K::Almond },    FLinearColor(0.22f, 0.48f, 0.18f), TEXT("feeding_candy"), EJamWeather::None, { 300.f, 0.05f, 0.f, FLinearColor::Black }, MeadowYaw),
			Biome(EJamBiome::Molars,  TEXT("The Molar Hills"),     { K::Goblin },            FLinearColor(0.58f, 0.44f, 0.48f), TEXT("hard_combat"), EJamWeather::Fog, { -400.f, -0.08f, 0.05f, FLinearColor(0.02f, 0.f, 0.03f) }, MolarsYaw),
			Biome(EJamBiome::Orchard, TEXT("The Lollipop Orchard"), { K::Bug, K::Almond },   FLinearColor(0.16f, 0.34f, 0.14f), TEXT("boss_battle"), EJamWeather::None, { 0.f, 0.1f, 0.03f, FLinearColor(0.f, 0.01f, 0.02f) }, OrchardYaw),
		};

		TArray<FJamLocation>& L = W.Locations;
		// The village: the square, two givers by the road, a signpost.
		L.Add(Loc(TEXT("Village"), EJamLocationKind::Village, EJamBiome::Village, FVector2D::ZeroVector, VillageRadius));
		L.Add(Loc(TEXT("Signpost"), EJamLocationKind::Landmark, EJamBiome::Village, Polar(60.f, 500.f), 100.f));
		L.Add(Loc(TEXT("GiverMayor"), EJamLocationKind::Giver, EJamBiome::Village, Polar(180.f, 700.f), 150.f, 0.f));
		L.Add(Loc(TEXT("GiverBaker"), EJamLocationKind::Giver, EJamBiome::Village, Polar(-60.f, 900.f), 150.f, 120.f));
		L.Add(Loc(TEXT("GiverGardener"), EJamLocationKind::Giver, EJamBiome::Village, FVector2D(800.f, 400.f), 150.f, -150.f));
		L.Add(Loc(TEXT("GiverDentist"), EJamLocationKind::Giver, EJamBiome::Village, FVector2D(-800.f, -600.f), 150.f, 35.f));
		L.Add(Loc(TEXT("GiverScout"), EJamLocationKind::Giver, EJamBiome::Village, FVector2D(-400.f, 1000.f), 150.f, -70.f));

		// The film sets (the main questline). Half sides match the backlot sets: 1800, 1400 and 2000 square.
		L.Add(Set(TEXT("SetField"),   EJamBiome::Meadow,  Polar(MeadowYaw, 6000.f),  900.f,  0));
		L.Add(Set(TEXT("SetFort"),    EJamBiome::Molars,  Polar(MolarsYaw, 6500.f),  700.f,  1));
		L.Add(Set(TEXT("SetOrchard"), EJamBiome::Orchard, Polar(OrchardYaw, 6000.f), 1000.f, 2));

		// One giver at each biome's edge, facing the village.
		L.Add(Loc(TEXT("GiverMeadow"),  EJamLocationKind::Giver, EJamBiome::Meadow,  Polar(MeadowYaw - 20.f, 3200.f),  150.f, 180.f + MeadowYaw - 20.f));
		L.Add(Loc(TEXT("GiverMolars"),  EJamLocationKind::Giver, EJamBiome::Molars,  Polar(MolarsYaw - 20.f, 3200.f),  150.f, 180.f + MolarsYaw - 20.f));
		L.Add(Loc(TEXT("GiverOrchard"), EJamLocationKind::Giver, EJamBiome::Orchard, Polar(OrchardYaw + 20.f, 3200.f), 150.f, 180.f + OrchardYaw + 20.f));

		// Camps: three per biome, two near the village road and one far out.
		L.Add(Camp(TEXT("CampMeadowWest"),  EJamBiome::Meadow, Polar(MeadowYaw - 35.f, 4200.f), { K::Rat }, 6));
		L.Add(Camp(TEXT("CampMeadowEast"),  EJamBiome::Meadow, Polar(MeadowYaw + 35.f, 4200.f), { K::Rat, K::Almond }, 6));
		L.Add(Camp(TEXT("CampMeadowFar"),   EJamBiome::Meadow, Polar(MeadowYaw + 10.f, 9800.f), { K::Rat }, 8));

		L.Add(Camp(TEXT("CampMolarsNear"),  EJamBiome::Molars, Polar(MolarsYaw + 30.f, 4300.f), { K::Goblin }, 5));
		L.Add(Camp(TEXT("CampMolarsRidge"), EJamBiome::Molars, Polar(MolarsYaw - 35.f, 8000.f), { K::Goblin }, 6));
		L.Add(Camp(TEXT("CampMolarsFar"),   EJamBiome::Molars, Polar(MolarsYaw + 15.f, 9500.f), { K::Goblin }, 8));

		L.Add(Camp(TEXT("CampOrchardNear"), EJamBiome::Orchard, Polar(OrchardYaw - 30.f, 4300.f), { K::Bug, K::Almond }, 6));
		L.Add(Camp(TEXT("CampOrchardGrove"),EJamBiome::Orchard, Polar(OrchardYaw + 35.f, 8200.f), { K::Bug }, 6));
		L.Add(Camp(TEXT("CampOrchardFar"),  EJamBiome::Orchard, Polar(OrchardYaw - 10.f, 9600.f), { K::Bug, K::Almond }, 8));
		return W;
	}
}

namespace JamWorld
{
	const FJamWorldDef& Def()
	{
		static const FJamWorldDef World = JamWorldTableRows::Build();
		return World;
	}
}
