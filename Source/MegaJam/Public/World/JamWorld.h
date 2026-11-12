#pragma once

#include "CoreMinimal.h"
#include "Film/JamFilmTypes.h"
#include "Film/JamNoteClamp.h"

/**
 * The open world as data (BUILD_SPEC 3e, O1). Headless: no UObjects, no world. Everything in Open mode asks
 * JamWorld where things are; the rows live in Private/World/JamWorldTable.cpp (shared with TERRAIN and TUNING).
 * Units are uu; +X is north, the village square is at the origin, z is found by tracing down.
 */

enum class EJamBiome : uint8
{
	Village,
	Meadow,     // the field (Scene 1)
	Molars,     // Fort Toothache's hills (Scene 2)
	Orchard,    // the orchard (Scene 3)
};

enum class EJamLocationKind : uint8
{
	Village,
	Giver,
	Camp,
	FilmSet,
	Landmark,
};

struct MEGAJAM_API FJamLocation
{
	FName Id;
	EJamLocationKind Kind = EJamLocationKind::Landmark;
	EJamBiome Biome = EJamBiome::Village;
	FVector2D Position = FVector2D::ZeroVector;
	float Radius = 0.f;
	float Yaw = 0.f;                         // degrees, 0 = +X
	TArray<EJamTargetKind> Kinds;            // camps: what they spawn
	int32 Capacity = 0;                      // camps: most targets alive at once
	int32 SceneIndex = INDEX_NONE;           // film sets: which of JamScenes::All() they host
};

/** Grade offsets added to JamLook's base grade inside the biome. */
struct MEGAJAM_API FJamGradeOffset
{
	float WhiteTemp = 0.f;       // added to the base white temperature (K)
	float Saturation = 0.f;      // added to the base saturation (1 = unchanged)
	float Contrast = 0.f;
	FLinearColor ShadowTint = FLinearColor::Black;   // added to the shadow offset
};

struct MEGAJAM_API FJamBiomeDef
{
	EJamBiome Biome = EJamBiome::Village;
	FString DisplayName;
	TArray<EJamTargetKind> Kinds;
	FLinearColor Tint = FLinearColor::White;   // floor patches in the flat fallback, map tint
	FName MusicCue;                            // a JamMusic cue
	EJamWeather Weather = EJamWeather::None;
	FJamGradeOffset Grade;
	float WedgeCentreYaw = 0.f;                // degrees; the village has none (it is the centre disc)
};

struct MEGAJAM_API FJamWorldDef
{
	FBox2D Bounds = FBox2D(ForceInit);
	float VillageRadius = 0.f;
	TArray<FJamBiomeDef> Biomes;
	TArray<FJamLocation> Locations;
};

namespace JamWorld
{
	/** The whole world, from JamWorldTable.cpp. Built once. */
	MEGAJAM_API const FJamWorldDef& Def();

	MEGAJAM_API const FJamBiomeDef* Biome(EJamBiome Biome);
	MEGAJAM_API const FJamLocation* Find(FName Id);
	MEGAJAM_API TArray<const FJamLocation*> OfKind(EJamLocationKind Kind, EJamBiome InBiome);
	MEGAJAM_API TArray<const FJamLocation*> OfKind(EJamLocationKind Kind);
	/** The film set hosting scene SceneIndex, or null. */
	MEGAJAM_API const FJamLocation* FilmSet(int32 SceneIndex);
	/** Which biome a point is in: the village disc, else the wedge whose centre yaw is nearest its bearing. */
	MEGAJAM_API EJamBiome BiomeAt(const FVector2D& Point);
	MEGAJAM_API const TCHAR* BiomeName(EJamBiome Biome);
	/** The square floor a film set's scene plays on (same size as its backlot set). */
	MEGAJAM_API FBox2D FilmSetFloor(const FJamLocation& Set);
}
