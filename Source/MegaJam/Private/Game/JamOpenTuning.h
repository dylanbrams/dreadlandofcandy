#pragma once

// =====================================================================================================
// TUNING SURFACE (BUILD_SPEC 3e): Open mode's camera, ambient spawns, compass and fallback scatter.
// TUNING may change these numbers; after a change run the build, run-tests.ps1 and both autoplay modes.
// =====================================================================================================

namespace JamOpenTuning
{
	// Flat fallback world (template map only).
	constexpr float TileSize = 2000.f;              // ground tiles, each tinted by the biome at its centre
	constexpr int32 ScatterSeed = 2026;
	constexpr float ScatterSpacing = 900.f;         // one candidate prop per cell of this size
	constexpr float ScatterChance = 0.45f;          // chance a cell gets a prop
	constexpr float ScatterClearance = 300.f;       // kept clear round every location's radius
	constexpr float VillageHouseRing = 1700.f;      // houses face the square from this distance
	constexpr int32 VillageHouses = 10;

	// Where the hero stands at the start of Open mode (village square, a step from the centre).
	constexpr float HeroStartX = 300.f;
	constexpr float HeroStartY = -300.f;
}
