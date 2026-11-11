#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "JamAoePattern.generated.h"

// Shape catalog. Custom uses CustomGridCells (rogue/nethack-style stamped offsets on a tile grid).
UENUM(BlueprintType)
enum class EJamAoeShape : uint8
{
	Circle    UMETA(DisplayName="Circle (point-blank)"),
	Ring      UMETA(DisplayName="Ring (donut)"),
	Cone      UMETA(DisplayName="Cone (forward arc)"),
	Line      UMETA(DisplayName="Line (forward rectangle)"),
	Cross     UMETA(DisplayName="Cross (+ four arms)"),
	Diagonal  UMETA(DisplayName="Diagonal (X four arms)"),
	Plus      UMETA(DisplayName="Plus (+ adjacent only)"),
	Square    UMETA(DisplayName="Square (centered)"),
	Hex       UMETA(DisplayName="Hex ring (6 points)"),
	Spiral    UMETA(DisplayName="Spiral (expanding ring delay)"),
	Custom    UMETA(DisplayName="Custom (grid cells)"),
};

// A safe-spot inside the pattern. LocalOffset is in pattern-local frame (X = forward, Y = right) — same frame
// UJamAoePatternLibrary::TestPoint uses internally. Radius is the gap size; sized so 1-N followers can shelter.
USTRUCT(BlueprintType)
struct FJamAoeHole
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hole")
	FVector2D LocalOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hole", meta=(ClampMin="0"))
	float Radius = 100.f;
};

// Pure-data params. Each field is interpreted by the shape — extras are ignored.
USTRUCT(BlueprintType)
struct FJamAoePatternParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern")
	EJamAoeShape Shape = EJamAoeShape::Circle;

	// Outer radius for Circle/Ring/Hex; half-length for Line/Cross; cone radius.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern", meta=(ClampMin="0"))
	float Radius = 400.f;

	// Inner radius for Ring.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern", meta=(ClampMin="0"))
	float InnerRadius = 200.f;

	// Cone half-angle in degrees.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern", meta=(ClampMin="0", ClampMax="180"))
	float ConeHalfAngleDeg = 30.f;

	// Half-width of Line / arm-thickness of Cross.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern", meta=(ClampMin="0"))
	float Width = 100.f;

	// Side length for Square.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern", meta=(ClampMin="0"))
	float SquareSize = 400.f;

	// Tile size for Custom and grid-aligned shapes (Plus).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern", meta=(ClampMin="1"))
	float TileSize = 100.f;

	// Custom: cell offsets in tile coords. (0,0) = origin tile, (1,0) = one tile forward, etc.
	// Pattern is rotated to face Forward at sample time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern", meta=(EditCondition="Shape == EJamAoeShape::Custom"))
	TArray<FIntPoint> CustomGridCells;

	// Safe-spot holes punched out of the shape. Author-placed; persists through ResolveProceduralHoles.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern|Holes")
	TArray<FJamAoeHole> StaticHoles;

	// At activation time, UJamAoePatternLibrary::ResolveProceduralHoles appends this many random holes
	// inside the shape. 0 = no procedural holes (only StaticHoles apply).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern|Holes", meta=(ClampMin="0"))
	int32 ProceduralHoleCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern|Holes", meta=(ClampMin="0"))
	float ProceduralHoleRadius = 120.f;

	// Minimum center-to-center distance between procedural holes (and from static holes). Larger = more spread.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pattern|Holes", meta=(ClampMin="0"))
	float MinHoleSeparation = 200.f;
};

// Pattern + cosmetic slots. Thin BP shell — author the params, drop in a decal + niagara, done.
UCLASS(BlueprintType)
class MEGAJAM_API UJamAoePatternDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pattern")
	FJamAoePatternParams Params;

	// Yendor's impact VFX / SFX / telegraph-decal slots were dropped in the port; cosmetics will arrive
	// with the Niagara work, driven from an ability event rather than baked into this asset.
};
