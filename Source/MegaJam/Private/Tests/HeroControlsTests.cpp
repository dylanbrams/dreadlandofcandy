#include "Misc/AutomationTest.h"
#include "Game/JamHero.h"
#include "UI/JamScreens.h"
#include "InputAction.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHeroControlsTest, "Jam.Game.Hero.Controls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FJamHeroControlsTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
    Context.SetCurrentWorld(World);
    World->InitializeActorsForPlay(FURL());
    AJamHero* Hero = World->SpawnActor<AJamHero>();
    Hero->CreateInput();
    auto Bound = [&](const UInputAction* Action, const FKey& Key)
    {
        for (const FEnhancedActionKeyMapping& Mapping : Hero->InputContext->GetMappings())
        {
            if (Mapping.Action == Action && Mapping.Key == Key) { return true; }
        }
        return false;
    };
    TestTrue(TEXT("Q and E turn the camera"), Bound(Hero->CameraTurnAction, EKeys::Q) && Bound(Hero->CameraTurnAction, EKeys::E));
    TestTrue(TEXT("wheel zoom mapping"), Bound(Hero->ZoomAction, EKeys::MouseWheelAxis));
    TestTrue(TEXT("F uses"), Bound(Hero->InteractAction, EKeys::F));
    TestTrue(TEXT("middle mouse signature"), Bound(Hero->SkillActions[0], EKeys::MiddleMouseButton));
    TestTrue(TEXT("left shift signature (no middle button on a trackpad)"), Bound(Hero->SkillActions[0], EKeys::LeftShift));
    TestTrue(TEXT("R ultimate"), Bound(Hero->SkillActions[2], EKeys::R));
    TestTrue(TEXT("X refuses notes"), Bound(Hero->RefuseAction, EKeys::X));
    TestFalse(TEXT("camera drag does not cast signature"), Bound(Hero->SkillActions[0], EKeys::RightMouseButton));
    TestFalse(TEXT("Q no longer casts ultimate"), Bound(Hero->SkillActions[2], EKeys::Q));
    TestTrue(TEXT("gamepad attack buttons use device-specific actions"),
        Bound(Hero->GamepadLectureAction, EKeys::Gamepad_RightTrigger) &&
        Bound(Hero->GamepadSkill2Action, EKeys::Gamepad_RightShoulder) &&
        Bound(Hero->GamepadSkill4Action, EKeys::Gamepad_LeftShoulder));
    Hero->EnableFollowCamera(90.f);
    Hero->AimSource = AJamHero::EAimSource::Mouse;
    Hero->GetCharacterMovement()->Velocity = FVector(100.f, 0.f, 0.f);
    Hero->SetActorRotation(FRotator(0.f, 45.f, 0.f));
    Hero->FaceGamepadAttack();
    TestTrue(TEXT("gamepad cast follows movement, not camera or stale mouse aim"), Hero->GetActorForwardVector().Equals(FVector::ForwardVector, 0.001f));
    TestTrue(TEXT("gamepad cast selects gamepad prompts"), Hero->IsUsingGamepad());
    TestTrue(TEXT("gamepad cast clears stale mouse aim"), Hero->AimSource == AJamHero::EAimSource::Movement);
    Hero->GetCharacterMovement()->Velocity = FVector::ZeroVector;
    Hero->SetActorRotation(FRotator(0.f, -45.f, 0.f));
    Hero->FaceGamepadAttack();
    TestTrue(TEXT("standing gamepad cast preserves facing"), FMath::IsNearlyEqual(Hero->GetActorRotation().Yaw, -45.f, 0.01f));
    Hero->bFollowCamera = false;
    Hero->AimSource = AJamHero::EAimSource::Stick;
    Hero->StickAim = FVector(0.f, 1.f, 0.f);
    Hero->FaceGamepadAttack();
    TestTrue(TEXT("fixed camera retains right-stick aim"), Hero->GetActorForwardVector().Equals(FVector::RightVector, 0.001f));
    Hero->AimSource = AJamHero::EAimSource::Movement;
    Hero->EnableFollowCamera(0.f);
    Hero->OnCameraTurn(FInputActionValue(1.f));
    Hero->TickFollowCamera(0.5f);
    TestEqual(TEXT("keyboard yaw uses elapsed time"), Hero->CameraYaw, 60.f);
    Hero->OnCameraTurn(FInputActionValue(0.f));
    Hero->TickFollowCamera(0.5f);
    TestEqual(TEXT("release stops rotation"), Hero->CameraYaw, 60.f);
    Hero->OnZoom(FInputActionValue(1.f));
    TestEqual(TEXT("wheel in reduces distance"), Hero->CameraArm->TargetArmLength, 780.f);
    Hero->SetFollowFraming(true);
    TestEqual(TEXT("zoom offset persists into a scene"), Hero->CameraArm->TargetArmLength, 1380.f);
    Hero->OnZoom(FInputActionValue(100.f));
    TestEqual(TEXT("near limit"), Hero->CameraArm->TargetArmLength, 600.f);
    Hero->OnZoom(FInputActionValue(-100.f));
    TestEqual(TEXT("far limit"), Hero->CameraArm->TargetArmLength, 2200.f);
    Hero->bCameraPinned = true;
    Hero->OnCameraTurn(FInputActionValue(1.f));
    Hero->TickFollowCamera(1.f);
    TestEqual(TEXT("capture pin wins over controls"), Hero->CameraYaw, 0.f); // scene framing set yaw to zero
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamControlsCardTest, "Jam.UI.Controls.Card",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FJamControlsCardTest::RunTest(const FString&)
{
    FJamMenuSettings S;
    auto HasControls = [](const FJamScreenView& V)
    {
        return V.Options.ContainsByPredicate([](const FJamMenuOption& O) { return O.Action == EJamMenuAction::OpenControls && O.bEnabled; });
    };
    TestTrue(TEXT("title links controls directly"), HasControls(JamScreens::Title(S)));
    TestTrue(TEXT("options links controls"), HasControls(JamScreens::Options(S)));
    TestTrue(TEXT("pause links controls"), HasControls(JamScreens::Pause(S)));
    for (bool Pad : { false, true })
    {
        S.bGamepad = Pad;
        const FJamScreenView V = JamScreens::Controls(S);
        TestEqual(TEXT("read-only card has only Back"), V.Options.Num(), 1);
        TestEqual(TEXT("Back is actionable"), V.Options[0].Action, EJamMenuAction::Back);
        TestEqual(TEXT("six visible instruction lines"), V.Quotes.Num(), 6);
        TestTrue(TEXT("correct device instructions"), V.Quotes[0].Contains(Pad ? TEXT("Left stick") : TEXT("Q / E")));
    }
    return true;
}
#endif
