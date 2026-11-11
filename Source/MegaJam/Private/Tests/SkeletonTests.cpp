#include "Misc/AutomationTest.h"
#include "AgentHooks.h"

#if WITH_DEV_AUTOMATION_TESTS

// Proves the test runner finds the game's tests. Game tests go under "Jam." too.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamSkeletonModuleTest, "Jam.Skeleton.ModuleLoaded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FJamSkeletonModuleTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("MegaJam module is loaded"), FModuleManager::Get().IsModuleLoaded(TEXT("MegaJam")));
	TestNotNull(TEXT("Agent hooks class exists"), UAgentHooksSubsystem::StaticClass());
	return true;
}

#endif
