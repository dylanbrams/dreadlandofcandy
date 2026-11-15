#include "Misc/AutomationTest.h"
#include "UI/JamHudWidget.h"
#include "UI/JamComicGeometry.h"
#include "UI/JamScreenWidget.h"
#include "UI/JamScreens.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Input/HittestGrid.h"
#include "InputCoreTypes.h"
#include "Rendering/DrawElements.h"
#include "Types/PaintArgs.h"
#include "Widgets/SWindow.h"
#include "Widgets/SNullWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamHudInputTest, "Jam.UI.Hud.MouseControls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FJamHudInputTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
    Context.SetCurrentWorld(World);
    UJamHudWidget* Hud = NewObject<UJamHudWidget>(World);
    Hud->Initialize();
    const TSharedRef<SWidget> SlateHud = Hud->TakeWidget();
    const TSharedRef<SWindow> Window = SNew(SWindow).ClientSize(FVector2D(1920.f, 1080.f));
    Window->SetContent(SlateHud);
    FHittestGrid Grid;
    Grid.SetHittestArea(FVector2D::ZeroVector, FVector2D(1920.f, 1080.f));
    const FGeometry Geometry = FGeometry::MakeRoot(FVector2D(1920.f, 1080.f), FSlateLayoutTransform());
    auto Paint = [&]()
    {
        Grid.Clear();
        SlateHud->SlatePrepass(1.f);
        FSlateWindowElementList Elements(Window);
        const FPaintArgs Args(&Window.Get(), Grid, FVector2D::ZeroVector, 10.0, 0.016f);
        SlateHud->Paint(Args, Geometry, FSlateRect(0.f, 0.f, 1920.f, 1080.f), Elements, 0, FWidgetStyle(), true);
    };
    int32 Calls = 0;
    EJamHudAction LastAction = EJamHudAction::Pause;
    Hud->OnAction.BindLambda([&](EJamHudAction Action) { ++Calls; LastAction = Action; });
    auto Click = [&](UWidget* Control, EJamHudAction Expected, const TCHAR* Label)
    {
        const FGeometry& ControlGeometry = Control->GetCachedGeometry();
        TestTrue(FString::Printf(TEXT("%s has layout"), Label), ControlGeometry.GetLocalSize().SizeSquared() > 1.f);
        const FVector2D Position = ControlGeometry.LocalToAbsolute(ControlGeometry.GetLocalSize() * 0.5f);
        bool bInHitPath = false;
        for (const FWidgetAndPointer& Hit : Grid.GetBubblePath(Position, 0.f, true))
        {
            bInHitPath |= Hit.Widget == Control->GetCachedWidget();
        }
        TestTrue(FString::Printf(TEXT("%s is mouse reachable through ancestors"), Label), bInHitPath);
        const int32 Before = Calls;
        const FPointerEvent Primary(0, Position, Position, TSet<FKey>{ EKeys::LeftMouseButton }, EKeys::LeftMouseButton, 0.f, FModifierKeysState());
        TestTrue(FString::Printf(TEXT("%s handles primary click"), Label), Hud->NativeOnMouseButtonDown(Geometry, Primary).IsEventHandled());
        TestEqual(FString::Printf(TEXT("%s dispatches once"), Label), Calls, Before + 1);
        TestEqual(FString::Printf(TEXT("%s dispatches correct action"), Label), LastAction, Expected);
        const FPointerEvent Secondary(0, Position, Position, TSet<FKey>{ EKeys::RightMouseButton }, EKeys::RightMouseButton, 0.f, FModifierKeysState());
        TestTrue(TEXT("secondary click cannot cast through a control"), Hud->NativeOnMouseButtonDown(Geometry, Secondary).IsEventHandled());
        TestEqual(TEXT("secondary click does not activate primary action"), Calls, Before + 1);
    };
    FJamHudView View;
    JamHud::Sample(TEXT("hud"), View);
    Hud->ApplyView(View, 10.f);
    TestEqual(TEXT("sample cooldown has readable seconds"), Hud->SkillCooldowns[1]->GetText().ToString(), FString(TEXT("3s")));
    View.Skills[1].CooldownSeconds = 2.1f;
    Hud->ApplyView(View, 10.f);
    TestEqual(TEXT("remaining cooldown rounds up"), Hud->SkillCooldowns[1]->GetText().ToString(), FString(TEXT("3s")));
    View.Skills[1].CooldownSeconds = 0.f;
    Hud->ApplyView(View, 10.f);
    TestEqual(TEXT("ready skill hides time overlay"), Hud->SkillCooldowns[1]->GetVisibility(), ESlateVisibility::Collapsed);
    View.Skills[1].CooldownSeconds = 3.f;
    View.Skills[1].bUnlocked = false;
    Hud->ApplyView(View, 10.f);
    TestEqual(TEXT("locked skill hides time overlay"), Hud->SkillCooldowns[1]->GetVisibility(), ESlateVisibility::Collapsed);
    View.Skills[1].bUnlocked = true;
    Hud->ApplyView(View, 10.f);
    // Diagnose the MID parameters as well as the GAS read: replacing an icon
    // must retain the material and continue updating its radial parameter.
    UMaterialInstanceDynamic* OriginalCooldown = Hud->SkillIconMaterials[1];
    TestNotNull(TEXT("cooldown material is loadable"), OriginalCooldown);
    if (OriginalCooldown)
    {
        float Ratio = -1.f;
        TestTrue(TEXT("Cooldown scalar exists"), OriginalCooldown->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Cooldown")), Ratio));
        TestEqual(TEXT("sample cooldown reaches the material"), Ratio, 0.6f);
        View.Skills[1].IconPath = TEXT("/Game/Art/UI/Skills/T_Skill_K02");
        Hud->ApplyView(View, 10.f);
        TestEqual(TEXT("class icon load retains the MID"), Hud->SkillIconMaterials[1].Get(), OriginalCooldown);
        UTexture* IconTexture = nullptr;
        TestTrue(TEXT("icon texture parameter exists"), OriginalCooldown->GetTextureParameterValue(FMaterialParameterInfo(TEXT("IconTexture")), IconTexture));
        TestTrue(TEXT("class icon texture actually loaded"), IconTexture && IconTexture->GetName() == TEXT("T_Skill_K02"));
        View.Skills[1].Cooldown = 0.25f;
        Hud->ApplyView(View, 10.f);
        OriginalCooldown->GetScalarParameterValue(FMaterialParameterInfo(TEXT("Cooldown")), Ratio);
        TestEqual(TEXT("cooldown updates after icon replacement"), Ratio, 0.25f);
        AddInfo(FString::Printf(TEXT("Cooldown MID retained through K02 icon load; scalar %.2f"), Ratio));
    }
    Paint();
    Click(Hud->SkillTiles[0], EJamHudAction::Skill1, TEXT("basic tile"));
    Click(Hud->SkillTiles[1], EJamHudAction::Skill2, TEXT("signature tile"));
    Click(Hud->SkillTiles[2], EJamHudAction::Ultimate, TEXT("ultimate tile uses enum slot 3"));
    Click(Hud->PromptPanel, EJamHudAction::Interact, TEXT("interact prompt"));
    Click(Hud->PauseControl, EJamHudAction::Pause, TEXT("pause"));
    const FVector2D Empty(1800.f, 500.f);
    const FPointerEvent WorldClick(0, Empty, Empty, TSet<FKey>{ EKeys::LeftMouseButton }, EKeys::LeftMouseButton, 0.f, FModifierKeysState());
    TestFalse(TEXT("empty space forwards gameplay click"), Hud->NativeOnMouseButtonDown(Geometry, WorldClick).IsEventHandled());
    View.bShootVisible = false;
    View.bTravelVisible = true;
    View.bPromptVisible = false;
    Hud->ApplyView(View, 11.f);
    Paint();
    Click(Hud->SkillTiles[0], EJamHudAction::Skill1, TEXT("travel basic tile"));
    Click(Hud->SkillTiles[1], EJamHudAction::Skill2, TEXT("travel signature tile"));
    Click(Hud->SkillTiles[2], EJamHudAction::Ultimate, TEXT("travel ultimate tile"));
    Click(Hud->LevelText, EJamHudAction::Character, TEXT("travel character sheet"));
    JamHud::Sample(TEXT("cut"), View);
    Hud->ApplyView(View, 12.f);
    Paint();
    Click(Hud->ChoiceTiles[0], EJamHudAction::TakeNote, TEXT("take note"));
    Click(Hud->ChoiceTiles[1], EJamHudAction::RefuseNote, TEXT("refuse note"));
    Click(Hud->ChoiceTiles[2], EJamHudAction::WriteNote, TEXT("write direction"));
    View.bReadingNote = true;
    Hud->ApplyView(View, 13.f);
    TestEqual(TEXT("note choices hide while request pending"), Hud->ChoiceTiles[0]->GetVisibility(), ESlateVisibility::Collapsed);
    JamHud::Sample(TEXT("wrap"), View);
    Hud->ApplyView(View, 14.f);
    Hud->ApplyView(View, 15.f);
    Paint();
    Click(Hud->WrapContinueButton, EJamHudAction::Continue, TEXT("continue button"));
    TestTrue(TEXT("continue has a large click target"), Hud->WrapContinueButton->GetCachedGeometry().GetLocalSize().Y >= 70.f);
    Click(Hud->WrapContinue, EJamHudAction::Continue, TEXT("continue label"));
    Window->SetContent(SNullWidget::NullWidget);
    Hud->ReleaseSlateResources(true);
    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamCharacterSpendingTest, "Jam.UI.Character.SkillChoices",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FJamCharacterSpendingTest::RunTest(const FString&)
{
    FJamProgression Progression;
    const FJamMenuSettings Settings;
    FJamScreenView Sheet = JamScreens::Character(Progression, Settings);
    TestEqual(TEXT("three active skills plus back"), Sheet.Options.Num(), 4);
    TestFalse(TEXT("no point means basic disabled"), Sheet.Options[0].bEnabled);
    Progression.ReachMilestone(EJamMilestone::Scene1Wrap);
    Sheet = JamScreens::Character(Progression, Settings);
    TestTrue(TEXT("basic can rank up"), Sheet.Options[0].bEnabled);
    TestTrue(TEXT("signature can rank up"), Sheet.Options[1].bEnabled);
    TestFalse(TEXT("ultimate remains rank one"), Sheet.Options[2].bEnabled);
    TestEqual(TEXT("ultimate menu action"), Sheet.Options[2].Action, EJamMenuAction::RankUltimate);
    TestTrue(TEXT("choose signature"), Progression.SpendPoint(EJamSkill::FormALine));
    TestEqual(TEXT("signature raised"), Progression.GetRank(EJamSkill::FormALine), 3);
    Sheet = JamScreens::Character(Progression, Settings);
    TestFalse(TEXT("maxed signature disabled"), Sheet.Options[1].bEnabled);
    TestFalse(TEXT("cannot spend ultimate"), Progression.SpendPoint(EJamSkill::LastJudgment));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamComicGeometryTest, "Jam.UI.ComicBorder.Geometry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FJamComicGeometryTest::RunTest(const FString&)
{
    for (const FVector2f Size : { FVector2f(1000, 70), FVector2f(448, 72), FVector2f(660, 74), FVector2f(400, 200) })
    {
        const TArray<FVector2f> Points = JamComicGeometry::Outline(Size);
        const TArray<int32> Triangles = JamComicGeometry::Triangulate(Points);
        TestTrue(TEXT("serrated polygon triangulates"), Triangles.Num() >= 3);
        double PolygonArea = 0, TriangleArea = 0;
        for (int32 i = 0; i < Points.Num(); ++i) { PolygonArea += JamComicGeometry::Cross(Points[i], Points[(i + 1) % Points.Num()]) * 0.5; }
        for (int32 i = 0; i + 2 < Triangles.Num(); i += 3)
        {
            const float Area = JamComicGeometry::Cross(Points[Triangles[i + 1]] - Points[Triangles[i]], Points[Triangles[i + 2]] - Points[Triangles[i]]) * 0.5f;
            TestTrue(TEXT("triangles have consistent winding"), Area >= 0.f);
            TriangleArea += Area;
        }
        TestTrue(TEXT("fill covers polygon once, without overlapping alpha"), FMath::Abs(TriangleArea - PolygonArea) < PolygonArea * 0.0001);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamQuestScreenInputTest, "Jam.UI.Quests.DeliberateChoice",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FJamQuestScreenInputTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
    Context.SetCurrentWorld(World);
    UJamScreenWidget* Widget = NewObject<UJamScreenWidget>(World);
    Widget->Initialize();
    const TSharedRef<SWidget> Slate = Widget->TakeWidget();
    FJamScreenView View; JamScreens::Sample(TEXT("questbranches"), View);
    Widget->ApplyView(View, 1.f);
    const FGeometry Geometry = FGeometry::MakeRoot(FVector2D(1920, 1080), FSlateLayoutTransform());
    int32 Calls = 0;
    EJamMenuInput Last = EJamMenuInput::Hover;
    Widget->OnMenuInput.BindLambda([&](EJamMenuInput Input, int32) { ++Calls; Last = Input; });
    const FKeyEvent Held(EKeys::F, FModifierKeysState(), 0, true, 0, 0);
    TestTrue(TEXT("held talk key consumed"), Widget->NativeOnKeyDown(Geometry, Held).IsEventHandled());
    TestEqual(TEXT("held talk key cannot choose a branch"), Calls, 0);
    const FVector2D Position(100, 100);
    const FPointerEvent CameraDrag(0, Position, Position, TSet<FKey>{ EKeys::RightMouseButton }, EKeys::RightMouseButton, 0, FModifierKeysState());
    Widget->NativeOnMouseButtonDown(Geometry, CameraDrag);
    TestEqual(TEXT("camera mouse button cannot choose a branch"), Calls, 0);
    const FKeyEvent Press(EKeys::F, FModifierKeysState(), 0, false, 0, 0);
    Widget->NativeOnKeyDown(Geometry, Press);
    TestEqual(TEXT("fresh key press chooses once"), Calls, 1);
    TestEqual(TEXT("fresh key press confirms"), Last, EJamMenuInput::Confirm);
    Widget->ReleaseSlateResources(true);
    GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    return true;
}
#endif
