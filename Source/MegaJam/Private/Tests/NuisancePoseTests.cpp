#include "Misc/AutomationTest.h"
#include "Game/JamNuisance.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNuisancePoseTest, "Jam.Game.Nuisance.GroundedPose",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FJamNuisancePoseTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
    Context.SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());
    AJamNuisance* N = World->SpawnActor<AJamNuisance>(FVector(0.f, 0.f, 70.f), FRotator::ZeroRotator);
    N->ArtMesh = TEXT("SM_Goblin");
    N->ArtHeight = 130.f;
    N->DispatchBeginPlay();
    const UStaticMesh* Mesh = N->Body->GetStaticMesh();
    auto Floor = [&]()
    {
        return N->Body->GetComponentTransform().TransformPosition(FVector(0.f, 0.f, Mesh->GetBoundingBox().Min.Z)).Z;
    };
    const float StandingFloor = Floor();
    N->RecoilAt = World->GetTimeSeconds();
    N->SetState(EJamNuisanceState::Pacified);
    N->Tick(0.15f);
    TestTrue(TEXT("settling does not bury the mesh"), FMath::IsNearlyEqual(Floor(), StandingFloor, 0.1f));
    N->Tick(0.25f);
    const float FirstScale = N->Body->GetRelativeScale3D().Z;
    N->Tick(0.2f);
    TestTrue(TEXT("calm pose breathes"), !FMath::IsNearlyEqual(FirstScale, N->Body->GetRelativeScale3D().Z, 0.0001f));
    TestTrue(TEXT("breathing keeps feet planted"), FMath::IsNearlyEqual(Floor(), StandingFloor, 0.1f));
    N->SetActorScale3D(FVector(1.8f));
    N->AddActorWorldOffset(FVector(0.f, 0.f, 56.f));
    N->SetState(EJamNuisanceState::Treated);
    N->Tick(0.4f);
    TestTrue(TEXT("scaled treated goblin stays on floor"), FMath::IsNearlyEqual(Floor(), StandingFloor, 0.1f));
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
    return true;
}
#endif
