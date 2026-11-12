#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UStaticMeshComponent;

/**
 * The generated art (T20), loaded by name from /Game/Art/Meshes/<Name>/StaticMeshes/<Name> (always cooked).
 * Everything falls back to the basic-shape placeholders when a mesh is missing or the game runs with -noart,
 * so mechanics tests and autoplay can run on the light version.
 */
namespace JamArt
{
	/** False with -noart. */
	MEGAJAM_API bool Enabled();

	/** The mesh, or null (missing, or art disabled). Cached. */
	MEGAJAM_API UStaticMesh* Load(const FString& Name);

	/**
	 * Puts the named mesh on Comp, scaled uniformly to Height (uu), standing at local Z = GroundZ, centred in XY and
	 * turned by its forward fix plus Yaw. Clears override materials so the model's own show. False = left alone.
	 */
	MEGAJAM_API bool Apply(UStaticMeshComponent* Comp, const FString& Name, float Height, float GroundZ = 0.f, float Yaw = 0.f);

	/** Every mesh the game uses (the import list and the -artreview lineup). */
	MEGAJAM_API const TArray<FString>& AllNames();

	/** How tall the model stands in the game (uu). */
	MEGAJAM_API float Height(const FString& Name);

	/** Degrees that turn the model to face +X (filled in from the -artreview lineup). */
	MEGAJAM_API float ForwardFix(const FString& Name);
}
