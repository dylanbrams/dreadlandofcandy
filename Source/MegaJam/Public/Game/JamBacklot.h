#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JamBacklot.generated.h"

class UMaterialInterface;
class UStaticMesh;

/**
 * Sugarborough as a film backlot (BUILD_SPEC 1, "The map"): three flat sets joined by a village street, laid
 * out along +X (up the screen). A C++ table rather than a map asset: the editor MCP cannot place actors or
 * save maps (T08 spike), and a table is testable and cooks with no content. Set i hosts scene i.
 */
struct FJamSetLayout
{
	FName Id;
	FString Sign;            // shown on the board at the set's front edge
	FBox2D Bounds;           // the playable floor inside the walls; nuisances steer inside it
	FLinearColor Floor;
};

/** One piece of set dressing (T20): a generated model standing at a spot, turned to face Yaw (0 = +X). */
struct FJamProp
{
	FString Mesh;
	FVector2D At;
	float Yaw = 0.f;
	float Height = 0.f;      // 0 = the model's game height (JamArt::Height)
};

namespace JamBacklot
{
	/** Street width; every opening in a set wall is this wide, centred on Y = 0. */
	constexpr float StreetWidth = 600.f;
	constexpr float WallHeight = 120.f;
	constexpr float WallThickness = 40.f;

	/** The sets in walking order: the field (Scene 1), Fort Toothache (Scene 2), the orchard (Scene 3). */
	MEGAJAM_API const TArray<FJamSetLayout>& Sets();
	/** The street between set i and set i + 1 (Num() - 1 of them). */
	MEGAJAM_API TArray<FBox2D> Streets();
	/** Where the hero stands when the set's scene starts: just inside its back edge, facing +X. */
	MEGAJAM_API FVector2D HeroMark(int32 SetIndex);
	/** Set dressing: houses along the streets, trees and crops round the sets, film gear in the back corners. */
	MEGAJAM_API const TArray<FJamProp>& Dressing();
}

/** Builds the backlot from the table: floors, low walls with street openings, a sign per set. Static scenery only. */
UCLASS()
class MEGAJAM_API AJamBacklot : public AActor
{
	GENERATED_BODY()

public:
	AJamBacklot();

	/** Builds everything with floor tops at GroundZ. Call once, right after spawning. */
	void Build(float GroundZ);

private:
	void AddBox(const FBox2D& Area, float Bottom, float Top, const FLinearColor& Colour, bool bCollide);
	void AddDressing(float GroundZ);

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> ShapeMaterial;
};
