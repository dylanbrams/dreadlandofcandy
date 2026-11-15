#include "Misc/AutomationTest.h"
#include "Film/JamProgression.h"
#include "Film/JamShoot.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamNumericXpTest, "Jam.Film.Progression.NumericXp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FJamNumericXpTest::RunTest(const FString&)
{
    TestTrue(TEXT("treatment rewards more than disabling"), JamProgression::XpValue(EJamXpSource::Treat, true) > JamProgression::XpValue(EJamXpSource::Pacify, true));
    for (bool TakeNotes : { false, true })
    {
        FJamProgression P;
        P.EnableNumericXp();
        const TArray<FJamSceneDef> Scenes = JamScenes::All();
        for (int32 SceneIndex = 0; SceneIndex < Scenes.Num(); ++SceneIndex)
        {
            const FJamSceneDef& Scene = Scenes[SceneIndex];
            for (int32 StageIndex = 0; StageIndex < Scene.Stages.Num(); ++StageIndex)
            {
                const FJamStageDef& Stage = TakeNotes && StageIndex == Scene.NoteBeforeStage ? Scene.Note.Replacement : Scene.Stages[StageIndex];
                const int32 Takes = Scene.bForcedRetakeOnFirstStage && StageIndex == 0 ? 2 : 1;
                for (int32 Take = 0; Take < Takes; ++Take)
                {
                    for (int32 Target = 0; Target < Stage.Count; ++Target)
                    {
                        if (Stage.Verb == EJamStageVerb::Interact && !FMath::IsNearlyEqual(Stage.ResolveMultiplier, 1.f)) { P.AddXp(EJamXpSource::Pacify); }
                        P.AddXp(Stage.Verb == EJamStageVerb::Pacify ? EJamXpSource::Pacify : Stage.Verb == EJamStageVerb::Interact ? EJamXpSource::Treat : EJamXpSource::Collect);
                    }
                }
            }
            P.CompleteQuest(SceneIndex);
            const int32 After = P.GetXp();
            P.CompleteQuest(SceneIndex);
            TestEqual(TEXT("quest bonus cannot repeat"), P.GetXp(), After);
        }
        TestEqual(TEXT("both authored routes earn 190 XP"), P.GetXp(), 190);
        TestEqual(TEXT("three levels earned"), P.GetLevel(), 4);
        TestEqual(TEXT("ten percent toward the next level"), P.GetLevelProgress(), 0.1f);
        TestEqual(TEXT("three spendable points"), P.GetPoints(), 3);
        TestEqual(TEXT("cards do not grant numeric levels"), P.ReachMilestone(EJamMilestone::Scene3FinalNote), 0);
        const int32 Before = P.GetXp();
        P.CompleteQuest(-1); P.CompleteQuest(3);
        TestEqual(TEXT("invalid quest cannot award XP"), P.GetXp(), Before);
    }
    FJamProgression Grind;
    Grind.EnableNumericXp();
    for (int32 i = 0; i < 1000; ++i) { Grind.AddXp(EJamXpSource::Pacify); }
    TestEqual(TEXT("XP awards cross thresholds without cards"), Grind.GetLevel(), 10);
    TestEqual(TEXT("level capped at ten"), Grind.GetLevel(), JamProgression::MaxLevel);
    TestEqual(TEXT("points match levels"), Grind.GetPoints(), 9);
    TestEqual(TEXT("ultimate remains rank one"), Grind.GetRank(EJamSkill::LastJudgment), 1);
    // Fixes3 #26: XP past level 10 is encore EXP and earns encore levels (+5% power each).
    TestEqual(TEXT("XP keeps counting"), Grind.GetXp(), 2000);
    const int32 Past = 2000 - JamProgression::XpForLevel(10);
    TestEqual(TEXT("encore levels"), Grind.GetEncoreLevel(), Past / JamProgression::EncoreXpPerLevel);
    TestEqual(TEXT("encore EXP into the level"), Grind.GetEncoreXp(), Past % JamProgression::EncoreXpPerLevel);
    TestEqual(TEXT("bar shows encore progress"), Grind.GetLevelProgress(), float(Past % JamProgression::EncoreXpPerLevel) / JamProgression::EncoreXpPerLevel);
    FJamProgression Fresh;
    Fresh.EnableNumericXp();
    TestEqual(TEXT("no encore before level ten"), Fresh.GetEncoreLevel(), 0);
    TestTrue(TEXT("encore levels add power"), Grind.GetStats().LecturePower > Fresh.GetStats().LecturePower * 1.2f);
    return true;
}
#endif
