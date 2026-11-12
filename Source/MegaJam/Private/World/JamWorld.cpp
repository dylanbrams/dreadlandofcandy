#include "World/JamWorld.h"

namespace JamWorld
{
	const FJamBiomeDef* Biome(EJamBiome InBiome)
	{
		return Def().Biomes.FindByPredicate([InBiome](const FJamBiomeDef& B) { return B.Biome == InBiome; });
	}

	const FJamLocation* Find(FName Id)
	{
		return Def().Locations.FindByPredicate([Id](const FJamLocation& L) { return L.Id == Id; });
	}

	TArray<const FJamLocation*> OfKind(EJamLocationKind Kind, EJamBiome InBiome)
	{
		TArray<const FJamLocation*> Out;
		for (const FJamLocation& L : Def().Locations) { if (L.Kind == Kind && L.Biome == InBiome) { Out.Add(&L); } }
		return Out;
	}

	TArray<const FJamLocation*> OfKind(EJamLocationKind Kind)
	{
		TArray<const FJamLocation*> Out;
		for (const FJamLocation& L : Def().Locations) { if (L.Kind == Kind) { Out.Add(&L); } }
		return Out;
	}

	const FJamLocation* FilmSet(int32 SceneIndex)
	{
		return Def().Locations.FindByPredicate([SceneIndex](const FJamLocation& L) { return L.Kind == EJamLocationKind::FilmSet && L.SceneIndex == SceneIndex; });
	}

	EJamBiome BiomeAt(const FVector2D& Point)
	{
		const FJamWorldDef& W = Def();
		if (Point.SizeSquared() <= FMath::Square(W.VillageRadius)) { return EJamBiome::Village; }
		const float Bearing = FMath::RadiansToDegrees(FMath::Atan2(Point.Y, Point.X));
		EJamBiome Best = EJamBiome::Village;
		float BestDelta = TNumericLimits<float>::Max();
		for (const FJamBiomeDef& B : W.Biomes)
		{
			if (B.Biome == EJamBiome::Village) { continue; }
			const float Delta = FMath::Abs(FMath::FindDeltaAngleDegrees(Bearing, B.WedgeCentreYaw));
			if (Delta < BestDelta) { BestDelta = Delta; Best = B.Biome; }
		}
		return Best;
	}

	const TCHAR* BiomeName(EJamBiome InBiome)
	{
		const FJamBiomeDef* B = Biome(InBiome);
		return B ? *B->DisplayName : TEXT("");
	}

	FBox2D FilmSetFloor(const FJamLocation& Set)
	{
		return FBox2D(Set.Position - FVector2D(Set.Radius), Set.Position + FVector2D(Set.Radius));
	}
}
