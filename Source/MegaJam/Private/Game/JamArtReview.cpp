#include "Game/JamArtReview.h"
#include "Game/JamArt.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	UStaticMesh* GCone = nullptr;
	UMaterialInterface* GShape = nullptr;
}

AJamArtReview::AJamArtReview()
{
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cone(TEXT("/Engine/BasicShapes/Cone.Cone"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Shape(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	GCone = Cone.Object;
	GShape = Shape.Object;
}

void AJamArtReview::Build(const FVector& Origin)
{
	SetActorLocation(Origin);
	const TArray<FString>& Names = JamArt::AllNames();
	constexpr int32 Columns = 6;
	constexpr float Spacing = 520.f;
	for (int32 i = 0; i < Names.Num(); ++i)
	{
		const int32 Row = i / Columns, Col = i % Columns;
		// Rows go up the screen (+X); columns across (Y).
		const FVector At((Row) * Spacing, (Col - (Columns - 1) * 0.5f) * Spacing, 0.f);
		UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this);
		Mesh->SetupAttachment(GetRootComponent());
		Mesh->RegisterComponent();
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		USceneComponent* Slot = NewObject<USceneComponent>(this);
		Slot->SetupAttachment(GetRootComponent());
		Slot->RegisterComponent();
		Slot->SetRelativeLocation(At);
		Mesh->AttachToComponent(Slot, FAttachmentTransformRules::KeepRelativeTransform);
		const bool bOk = JamArt::Apply(Mesh, Names[i], JamArt::Height(Names[i]));

		UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
		Label->SetupAttachment(Slot);
		Label->RegisterComponent();
		Label->SetText(FText::FromString(bOk ? Names[i] : Names[i] + TEXT(" (missing)")));
		Label->SetWorldSize(40.f);
		Label->SetHorizontalAlignment(EHTA_Center);
		Label->SetTextRenderColor(bOk ? FColor::White : FColor::Red);
		Label->SetRelativeLocation(FVector(-170.f, 0.f, 5.f));
		Label->SetRelativeRotation(FRotator(90.f, 180.f, 0.f));

		// The forward arrow: a flat cone pointing +X.
		UStaticMeshComponent* Arrow = NewObject<UStaticMeshComponent>(this);
		Arrow->SetupAttachment(Slot);
		Arrow->RegisterComponent();
		Arrow->SetStaticMesh(GCone);
		Arrow->SetMaterial(0, GShape);
		Arrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Arrow->SetRelativeLocation(FVector(-120.f, 120.f, 5.f));
		Arrow->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
		Arrow->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.6f));
		if (UMaterialInstanceDynamic* M = Arrow->CreateAndSetMaterialInstanceDynamic(0)) { M->SetVectorParameterValue(TEXT("Color"), FLinearColor::Red); }
	}
}
