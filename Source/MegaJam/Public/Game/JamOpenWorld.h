#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/JamBacklot.h"
#include "JamOpenWorld.generated.h"

class UMaterialInterface;
class UStaticMesh;

namespace JamOpenWorld
{
	/** The fallback world's dressing from the world table: houses round the village square and a seeded scatter
	 *  of each biome's props, clear of every location. Pure and deterministic (tests Jam.World.Scatter). */
	MEGAJAM_API TArray<FJamProp> Scatter(int32 Seed);
	/** True on the engine's template map, where Open mode builds the flat fallback (BUILD_SPEC 3e). */
	MEGAJAM_API bool IsFallbackMap(const FString& MapName);
}

/**
 * Open mode's flat stand-in world (O1), built from JamWorld::Def() alone: biome-tinted ground tiles, a floor per
 * film set, a marker disc per camp and giver, and the scatter. Only spawned on the template map; on TERRAIN's map
 * the game spawns gameplay actors at the table's locations and nothing else.
 */
UCLASS()
class MEGAJAM_API AJamOpenWorld : public AActor
{
	GENERATED_BODY()

public:
	AJamOpenWorld();

	void Build(float GroundZ);

private:
	UStaticMeshComponent* AddShape(UStaticMesh* Mesh, const FVector& Centre, const FVector& Scale, const FLinearColor& Colour, bool bCollide);
	void AddLabel(const FString& Text, const FVector& At);

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CylinderMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMaterial;
};
