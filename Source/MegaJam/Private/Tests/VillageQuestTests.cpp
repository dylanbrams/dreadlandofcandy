#include "Misc/AutomationTest.h"
#include "World/JamVillageQuests.h"
#include "World/JamWorld.h"
#include "UI/JamScreens.h"
#include "Game/JamGameMode.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamFaerie.h"
#include "Game/JamVillager.h"
#include "Game/JamQuestMarker.h"
#include "Game/JamPickup.h"
#include "Game/JamNuisance.h"
#include "Game/JamPlayerState.h"
#include "Gas/JamGameplayEffects.h"
#include "Gas/JamGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/BoxComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
#define VillageTestFlags (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVillageRulesTest, "Jam.World.Quests.Rules", VillageTestFlags)
bool FJamVillageRulesTest::RunTest(const FString&)
{
	FJamVillageQuestLog Log;
	TestFalse(TEXT("cannot finish main before recruitment"), Log.FinishMain());
	TestFalse(TEXT("villagers wait for main quests"), Log.Accept(0));
	TestTrue(TEXT("recruit once"), Log.Recruit());
	TestFalse(TEXT("cannot recruit twice"), Log.Recruit());
	TestFalse(TEXT("recruitment alone does not unlock villagers"), Log.Accept(1));
	TestTrue(TEXT("finishing main unlocks village"), Log.FinishMain());
	TestFalse(TEXT("main completion is once only"), Log.FinishMain());
	TestFalse(TEXT("invalid quest refused"), Log.Accept(99));
	for (int32 Index : { 1, 0 })
	{
		const FJamVillageQuestDef& Quest = JamVillageQuests::All()[Index];
		TestNotNull(TEXT("giver exists in world"), JamWorld::Find(Quest.GiverLocation));
		TestNotNull(TEXT("destination exists in world"), JamWorld::Find(Quest.Destination));
		TestTrue(TEXT("accept in either order"), Log.Accept(Index));
		TestFalse(TEXT("one quest at a time"), Log.Accept(1 - Index));
		TestFalse(TEXT("wrong objective cannot progress"), Log.Report(Quest.Verb == EJamStageVerb::Collect ? EJamStageVerb::Pacify : EJamStageVerb::Collect));
		TestFalse(TEXT("no early reward"), Log.TurnIn(Index));
		for (int32 n = 0; n < Quest.Count; ++n) { TestTrue(TEXT("objective advances"), Log.Report(Quest.Verb)); }
		TestEqual(TEXT("must return after objectives"), Log.State(Index), EJamVillageQuestState::Ready);
		TestFalse(TEXT("progress capped"), Log.Report(Quest.Verb));
		TestTrue(TEXT("turn in"), Log.TurnIn(Index));
		TestFalse(TEXT("cannot claim reward twice"), Log.TurnIn(Index));
		TestFalse(TEXT("cannot repeat completed quest"), Log.Accept(Index));
	}
	TestEqual(TEXT("both errands completed"), Log.Completed(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVillageFlowTest, "Jam.World.Quests.GameplayFlow", VillageTestFlags)
bool FJamVillageFlowTest::RunTest(const FString&)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AJamGameMode* GM = World->SpawnActor<AJamGameMode>();
	AJamGameState* GS = World->SpawnActor<AJamGameState>();
	GM->GameState = GS;
	World->SetGameState(GS);
	AJamHero* Hero = World->SpawnActor<AJamHero>();
	AJamPlayerState* PS = World->SpawnActor<AJamPlayerState>();
	Hero->SetPlayerState(PS);
	PS->InitForAvatar(Hero);
	PS->EditProgression().EnableNumericXp();
	GM->Hero = Hero;
	GM->bOpenMode = true;
	auto GroundBox = [&](FVector Centre, FVector Extent, ECollisionResponse Response)
	{
		AActor* Actor = World->SpawnActor<AActor>();
		UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
		Actor->SetRootComponent(Box);
		Box->SetBoxExtent(Extent);
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Box->SetCollisionObjectType(ECC_WorldStatic);
		Box->SetCollisionResponseToAllChannels(Response);
		Box->SetWorldLocation(Centre);
		Box->RegisterComponent();
		Actor->AddInstanceComponent(Box);
	};
	GroundBox(FVector(0, 0, -10), FVector(12000, 12000, 10), ECR_Block);
	GroundBox(FVector(0, 0, 500), FVector(1200, 1200, 500), ECR_Overlap);
	TestEqual(TEXT("NPC ground ignores invisible overlapping volumes"), GM->TraceGround(FVector2D::ZeroVector), 0.f);
	GM->Faerie = World->SpawnActor<AJamFaerie>();
	GM->BuildVillage();
	GS->Phase = EJamFilmPhase::Shooting;
	GM->BeginRecruitment();
	TestTrue(TEXT("arrival introduces the reality show"), GS->CaptionText.Contains(TEXT("Heroes of Dread: The Land of Candy")));
	TestEqual(TEXT("remaining introduction beats queue in order"), GM->CaptionQueue.Num(), 4);
	TestEqual(TEXT("two named villagers"), GM->Villagers.Num(), 5);
	TestTrue(TEXT("recruitment has a visible marker"), GM->QuestMarker && !GM->QuestMarker->IsHidden());
	Hero->SetActorLocation(FVector(GM->SceneFloor(0).GetCenter(), 96.f));
	GM->Tick(0.1f);
	TestEqual(TEXT("entering first set before recruitment cannot start it"), GS->GetShoot().GetState(), EJamShootState::Idle);
	TestFalse(TEXT("recruitment requires proximity"), GM->HandleVillageInteract(Hero));
	Hero->SetActorLocation(GM->Faerie->GetActorLocation() + FVector(100, 0, -80));
	TestTrue(TEXT("talk recruits Fenella"), GM->HandleVillageInteract(Hero));
	TestTrue(TEXT("recruiting skips remaining introduction"), GM->CaptionQueue.IsEmpty());
	TestTrue(TEXT("recruitment starts walk to first quest"), GM->bBetweenScenes);
	TestEqual(TEXT("quest marker moves to first clearing"), FVector2D(GM->QuestMarker->GetActorLocation()), GM->SceneFloor(0).GetCenter());
	Hero->SetActorLocation(FVector(GM->SceneFloor(0).GetCenter(), 96.f));
	GM->TickBetweenScenes();
	TestEqual(TEXT("recruited hero starts scene by entering clearing"), GS->GetShoot().GetState(), EJamShootState::Slate);
	// Existing shoot and autoplay tests cover the three main quests. Exercise their final-wrap handoff here.
	GM->VillageQuests.FinishMain();
	GM->bLastWrapped = true;
	GS->Wrap.bVisible = true;
	GS->WrapUntil = -1.f;
	TestTrue(TEXT("final wrap returns to exploration"), GM->HandleDismissWrap(Hero));
	TestTrue(TEXT("village exploration enabled"), GM->bVillageExploration);
	TestFalse(TEXT("wrap dismissed"), GS->IsWrapVisible());
	TestFalse(TEXT("budget gag does not mute village"), GS->bBudgetOut);
	for (AJamQuestMarker* Marker : GM->VillagerMarkers) { TestTrue(TEXT("giver markers unlocked"), Marker && !Marker->IsHidden()); }
	int32 TotalRewards = 0;
	for (int32 Index : { 1, 0, 2, 3, 4, 5, 6, 7, 8, 9 })
	{
		const FJamVillageQuestDef& Quest = GM->VillageQuests.Offer(Index);
		const int32 Giver = GM->VillageQuests.GiverIndex(Index);
		Hero->SetActorLocation(GM->Villagers[Giver]->GetActorLocation() + FVector(90, 0, 96));
		TestTrue(TEXT("talk offers quest routes"), GM->HandleVillageInteract(Hero));
		TestTrue(TEXT("choice screen opens before accepting"), GM->QuestOffer.bVisible);
        if (Index == 1)
        {
            const int32 Stale = GM->BranchRequestSerial;
            GM->CancelVillageOffer();
            GM->ResolveVillageBranches(Stale, Index, true, TEXT("{}"));
            TestFalse(TEXT("late response cannot reopen a cancelled offer"), GM->QuestOffer.bVisible);
            GM->HandleVillageInteract(Hero);
            GM->ResolveVillageBranches(Stale, Index, false, FString());
            TestTrue(TEXT("stale response cannot dismiss a new offer"), GM->QuestOffer.bVisible);
            GM->ResolveVillageBranches(GM->BranchRequestSerial, Index, false, FString());
            TestEqual(TEXT("failed backend keeps both authored routes"), GM->QuestOffer.Branches.Num(), 2);
        }
		GM->ChooseVillageBranch(0);
		TestEqual(TEXT("accepted correct quest"), GM->VillageQuests.Active(), Index);
        if (Index == 1)
        {
            TestTrue(TEXT("can set aside an active errand"), GM->AbandonVillageQuest());
            TestEqual(TEXT("abandon clears pickups"), GM->Pickups.Num(), 0);
            TestEqual(TEXT("abandon returns quest to giver"), GM->VillageQuests.State(Index), EJamVillageQuestState::Available);
            TestTrue(TEXT("can take the errand again"), GM->HandleVillageInteract(Hero));
            GM->ChooseVillageBranch(0);
        }
		const int32 Before = PS->Sprinkles;
		// Fixes2 #5: a giver's second job pays their Acquaintance bonus on top of the reward.
		const int32 Promised = GM->VillageQuests.Payout(Index);
		TestEqual(TEXT("payout is reward plus level bonus"), Promised, Quest.Reward + JamVillageQuests::LevelBonus(GM->VillageQuests.Level(Giver)));
		TotalRewards += Promised;
		TestTrue(TEXT("early talk is allowed"), GM->HandleVillageInteract(Hero));
		TestEqual(TEXT("early talk grants no money"), PS->Sprinkles, Before);
		if (Quest.Verb == EJamStageVerb::Collect)
		{
			TestEqual(TEXT("recipe pickup count spawned"), GM->Pickups.Num(), Quest.Count);
			while (!GM->Pickups.IsEmpty())
			{
				const FVector At = GM->Pickups.Last()->GetActorLocation();
				Hero->SetActorLocation(At + FVector(0, 0, 71.f));
				GM->TickVillage(0.1f);
			}
		}
		else
		{
			TestEqual(TEXT("recipe nuisance count spawned"), GM->Nuisances.Num(), Quest.Count);
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			for (AJamNuisance* Rat : GM->Nuisances)
			{
				FGameplayEffectSpecHandle Hit = ASC->MakeOutgoingSpec(UJamGE_ResolveDamage::StaticClass(), 1.f, ASC->MakeEffectContext());
				Hit.Data->SetSetByCallerMagnitude(JamGameplayTags::SetByCaller_Resolve_Base, 1000.f);
				ASC->ApplyGameplayEffectSpecToTarget(*Hit.Data, Rat->GetAbilitySystemComponent());
				TestTrue(TEXT("target pacified through existing GAS effect"), Rat->IsPacified());
                if (Quest.Verb == EJamStageVerb::Interact)
                {
                    Hero->SetActorLocation(Rat->GetActorLocation() + FVector(90, 0, 26));
                    TestTrue(TEXT("treating advances the errand"), GM->HandleInteract(Hero, Rat));
                    TestFalse(TEXT("same target cannot be treated twice"), GM->HandleInteract(Hero, Rat));
                }
			}
		}
		TestEqual(TEXT("objectives require returning to giver"), GM->VillageQuests.State(Index), EJamVillageQuestState::Ready);
		Hero->SetActorLocation(FVector(5000, 5000, 96));
		TestFalse(TEXT("cannot turn in remotely"), GM->HandleVillageInteract(Hero));
		TestEqual(TEXT("objectives alone pay nothing"), PS->Sprinkles, Before);
		Hero->SetActorLocation(GM->Villagers[Giver]->GetActorLocation() + FVector(90, 0, 96));
		TestTrue(TEXT("turn in at giver"), GM->HandleVillageInteract(Hero));
		TestEqual(TEXT("giver pays promised reward"), PS->Sprinkles, Before + Promised);
		TestFalse(TEXT("quest cannot be turned in again"), GM->VillageQuests.TurnIn(Index));
        TestEqual(TEXT("repeated turn-in cannot duplicate reward"), PS->Sprinkles, Before + Promised);
        const int32 Next = GM->VillageQuests.ForGiver(Quest.GiverLocation);
        TestEqual(TEXT("giver marker reflects follow-up availability"), GM->VillagerMarkers[Giver]->IsHidden(), Next == INDEX_NONE);
	}
	Hero->SetActorLocation(FVector(0, 0, 96));
	GM->HandleVillageInteract(Hero);
	TestEqual(TEXT("Fenella still offers Premiere after errands"), GS->Phase, EJamFilmPhase::Premiere);
	GM->RequestCredits();
	GM->RequestBackToTitle();
	TestEqual(TEXT("credits return to living village"), GS->Phase, EJamFilmPhase::Shooting);
	TestEqual(TEXT("return preserves quest completions"), GM->VillageQuests.Completed(), 10);
	TestEqual(TEXT("return preserves money"), PS->Sprinkles, TotalRewards);
	GM->ClearStageActors();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamVillageVariationTest, "Jam.World.Quests.VariationsAndHooks", VillageTestFlags)
bool FJamVillageVariationTest::RunTest(const FString&)
{
    FJamVillageQuestLog A(42), B(42), C(843);
    bool Changed = false;
    TSet<FName> Ids;
    for (int32 i = 0; i < A.Num(); ++i)
    {
        const FJamVillageQuestDef& Q = A.Offer(i);
        TestFalse(TEXT("stable quest IDs unique"), Ids.Contains(Q.Id)); Ids.Add(Q.Id);
        TestNotNull(TEXT("giver has a world location"), JamWorld::Find(Q.GiverLocation));
        if (!Q.Prerequisite.IsNone())
        {
            const int32 Parent = JamVillageQuests::All().IndexOfByPredicate([&](const FJamVillageQuestDef& P) { return P.Id == Q.Prerequisite; });
            TestTrue(TEXT("prerequisites exist and form a forward chain"), Parent >= 0 && Parent < i);
        }
        TestNotNull(TEXT("destination has a world location"), JamWorld::Find(Q.Destination));
        TestTrue(TEXT("offer within count bounds"), Q.Count >= Q.MinCount && Q.Count <= Q.MaxCount);
        TestEqual(TEXT("same seed same count"), Q.Count, B.Offer(i).Count);
        TestEqual(TEXT("same seed same location"), Q.Destination, B.Offer(i).Destination);
        Changed |= Q.Count != C.Offer(i).Count || Q.Destination != C.Offer(i).Destination;
    }
    TestTrue(TEXT("different seeds vary errands"), Changed);
    FJamQuestVariation Bad; Bad.Destination = TEXT("Village"); Bad.Count = 999;
    const int32 Before = A.Offer(2).Count;
    TestFalse(TEXT("variation cannot move targets into village"), A.ApplyVariation(2, Bad));
    TestEqual(TEXT("rejected variation leaves offer alone"), A.Offer(2).Count, Before);
    FJamQuestVariation Bounded; Bounded.Count = 999; Bounded.Pitch = FString::ChrN(400, TEXT('x'));
    TestTrue(TEXT("valid variation clamped"), A.ApplyVariation(2, Bounded));
    TestEqual(TEXT("count capped at recipe maximum"), A.Offer(2).Count, A.Offer(2).MaxCount);
    TestEqual(TEXT("pitch bounded"), A.Offer(2).Request.Len(), 240);
    TestTrue(TEXT("rewards bounded"), A.Offer(2).Reward <= 300);
    TArray<EJamQuestEvent> Events;
    A.OnChanged.AddLambda([&](const FJamVillageQuestEvent& Event) { Events.Add(Event.Type); });
    A.Recruit(); A.FinishMain();
    TestEqual(TEXT("follow-up locked before its parent"), A.State(5), EJamVillageQuestState::Locked);
    TestFalse(TEXT("cannot accept locked follow-up"), A.Accept(5));
    A.Accept(0);
    TestFalse(TEXT("accepted offer cannot be rewritten"), A.ApplyVariation(0, Bounded));
    for (int32 i = 0; i < A.Offer(0).Count; ++i) { A.Report(EJamStageVerb::Pacify); }
    A.TurnIn(0);
    TestEqual(TEXT("turn-in unlocks follow-up"), A.State(5), EJamVillageQuestState::Available);
    TestEqual(TEXT("giver now offers follow-up"), A.ForGiver(TEXT("GiverMayor")), 5);
    TestEqual(TEXT("first hook recruitment"), Events[0], EJamQuestEvent::Recruited);
    TestTrue(TEXT("ready hook emitted"), Events.Contains(EJamQuestEvent::Ready));
    TestEqual(TEXT("last hook reward claim"), Events.Last(), EJamQuestEvent::TurnedIn);
    const FJamScreenView Journal = JamScreens::Quests(A, FJamMenuSettings());
    TestEqual(TEXT("journal covers all five givers"), Journal.Quotes.Num(), 5);
    TestTrue(TEXT("journal shows completion count"), Journal.Kicker.Contains(TEXT("1 OF 10")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamQuestJsonTest, "Jam.World.Quests.OfferJson", VillageTestFlags)
bool FJamQuestJsonTest::RunTest(const FString&)
{
    FJamVillageQuestLog Log;
    FString Error;
    TestTrue(TEXT("versioned bounded override accepted"), Log.ApplyOverridesJson(TEXT(R"({"v":1,"offers":[{"id":"celery_actually","count":4,"destination":"CampMeadowEast","pitch":"A new delivery, please."}]})"), Error));
    TestEqual(TEXT("override count applied"), Log.Offer(2).Count, 4);
    TestEqual(TEXT("override pitch applied"), Log.Offer(2).Request, FString(TEXT("A new delivery, please.")));
    TestFalse(TEXT("partial batch rejected"), Log.ApplyOverridesJson(TEXT(R"({"v":1,"offers":[{"id":"celery_actually","count":5},{"id":"open_wide","destination":"Village"}]})"), Error));
    TestEqual(TEXT("failed batch rolls back earlier changes"), Log.Offer(2).Count, 4);
    TestFalse(TEXT("wrong schema version rejected"), Log.ApplyOverridesJson(TEXT(R"({"v":2,"offers":[]})"), Error));
    TestFalse(TEXT("generated rewards cannot be injected"), Log.ApplyOverridesJson(TEXT(R"({"v":1,"offers":[{"id":"celery_actually","reward":999999}]})"), Error));
    TestFalse(TEXT("fractional counts rejected"), Log.ApplyOverridesJson(TEXT(R"({"v":1,"offers":[{"id":"celery_actually","count":1.5}]})"), Error));
    TestFalse(TEXT("repeated quest ids rejected"), Log.ApplyOverridesJson(TEXT(R"({"v":1,"offers":[{"id":"celery_actually"},{"id":"celery_actually"}]})"), Error));
    Log.Recruit(); Log.FinishMain(); Log.Accept(2);
    TestFalse(TEXT("active quest is immutable to JSON"), Log.ApplyOverridesJson(TEXT(R"({"v":1,"offers":[{"id":"celery_actually","count":5}]})"), Error));
    TestTrue(TEXT("abandon is available"), Log.Abandon());
    TestFalse(TEXT("cannot abandon twice"), Log.Abandon());
    TestEqual(TEXT("abandon clears progress"), Log.Progress(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamQuestBranchesTest, "Jam.World.Quests.BackendBranches", VillageTestFlags)
bool FJamQuestBranchesTest::RunTest(const FString&)
{
    const FString Good = TEXT(R"({"v":1,"source":"model","quest_id":"council_rats","branches":[{"route":"original","label":"A firm word","pitch":"Pacify the rats and return.","destination":"CampMeadowWest","count":99},{"route":"alternative","label":"Practical help","pitch":"Pacify and feed the rats celery.","destination":"CampMeadowWest","count":3}]})");
    TArray<FJamQuestBranch> Branches;
    TestTrue(TEXT("backend branches parse"), JamVillageQuests::ReadBranches(Good, 0, Branches));
    TestEqual(TEXT("backend count clamped to recipe"), Branches[0].Variation.Count, 4);
    TestFalse(TEXT("foreign destination refused"), JamVillageQuests::ReadBranches(Good.Replace(TEXT("CampMeadowWest"), TEXT("Village")), 0, Branches));
    TestEqual(TEXT("invalid response preserves existing choices"), Branches.Num(), 2);
    TestFalse(TEXT("wrong quest response refused"), JamVillageQuests::ReadBranches(Good, 1, Branches));
    TestFalse(TEXT("refused response falls back"), JamVillageQuests::ReadBranches(Good.Replace(TEXT("model"), TEXT("refused")), 0, Branches));
    FJamVillageQuestLog Log; Log.Recruit(); Log.FinishMain();
    TestTrue(TEXT("select alternate branch"), Log.SelectBranch(0, Branches[1]));
    TestEqual(TEXT("branch changes actual objective"), Log.Offer(0).Verb, EJamStageVerb::Interact);
    TestTrue(TEXT("accept chosen branch"), Log.Accept(0));
    TestFalse(TEXT("accepted branch cannot be changed"), Log.SelectBranch(0, Branches[0]));
    TestFalse(TEXT("pacifying alone cannot complete treatment branch"), Log.Report(EJamStageVerb::Pacify));
    for (int32 i = 0; i < 3; ++i) { Log.Report(EJamStageVerb::Interact); }
    TestTrue(TEXT("alternate branch can be returned for reward"), Log.TurnIn(0));
    TestTrue(TEXT("history records chosen route for future backend context"), Log.CompletedRoutes().Contains(TEXT("council_rats:alternative")));
    TestEqual(TEXT("alternate branch unlocks normal follow-up"), Log.State(5), EJamVillageQuestState::Available);
    FJamQuestOfferView Offer; Offer.bVisible = true; Offer.Branches = Branches;
    Offer.Title = TEXT("A Quiet Constituency"); Offer.bCanImprovise = true;
    const FJamScreenView Screen = JamScreens::QuestBranches(Offer, FJamMenuSettings());
    TestEqual(TEXT("two playable choices plus improvise and cancel"), Screen.Options.Num(), 4);
    TestEqual(TEXT("alternate route has a menu action"), Screen.Options[1].Action, EJamMenuAction::QuestBranch1);
    FJamScreenView Sample;
    TestTrue(TEXT("branch UI can be previewed"), JamScreens::Sample(TEXT("questbranches"), Sample));
    TestTrue(TEXT("journal UI can be previewed"), JamScreens::Sample(TEXT("quests"), Sample));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamQuestLevelsTest, "Jam.World.Quests.RelationshipLevels", VillageTestFlags)
bool FJamQuestLevelsTest::RunTest(const FString&)
{
    FJamVillageQuestLog Log;
    TArray<EJamQuestEvent> Events;
    Log.OnChanged.AddLambda([&](const FJamVillageQuestEvent& Event) { Events.Add(Event.Type); });
    Log.Recruit(); Log.FinishMain();
    const int32 Mayor = 0;
    TestEqual(TEXT("everyone starts a stranger"), Log.Level(Mayor), 0);
    TestEqual(TEXT("level names"), JamVillageQuests::LevelName(0), FString(TEXT("Stranger")));
    TestEqual(TEXT("names cap at Bestie"), JamVillageQuests::LevelName(9), FString(TEXT("Bestie")));
    TestEqual(TEXT("ten authored jobs plus five encores"), Log.Num(), 15);
    TestEqual(TEXT("encores are not authored jobs"), Log.NumAuthored(), 10);

    // A stranger's first job is exactly as authored.
    const int32 FirstCount = Log.Offer(0).Count;
    TestTrue(TEXT("accept first job"), Log.Accept(0));
    TestEqual(TEXT("level 0 leaves the job alone"), Log.Offer(0).Count, FirstCount);
    TestEqual(TEXT("one step"), Log.StepCount(0), 1);
    for (int32 i = 0; i < FirstCount; ++i) { Log.Report(EJamStageVerb::Pacify); }
    TestEqual(TEXT("stranger payout has no bonus"), Log.Payout(0), Log.Offer(0).Reward);
    TestTrue(TEXT("turn in"), Log.TurnIn(0));
    TestEqual(TEXT("one job: acquaintance"), Log.Level(Mayor), 1);
    TestTrue(TEXT("level-up event"), Events.Contains(EJamQuestEvent::LevelUp));

    // Acquaintance: one more target, and abandoning does not stack the bonus.
    const int32 Second = Log.ForGiver(TEXT("GiverMayor"));
    TestEqual(TEXT("mayor's follow-up"), Second, 5);
    const int32 SecondCount = Log.Offer(Second).Count;
    TestTrue(TEXT("accept follow-up"), Log.Accept(Second));
    TestEqual(TEXT("acquaintance adds a target"), Log.Offer(Second).Count, FMath::Min(SecondCount + 1, 8));
    TestTrue(TEXT("abandon"), Log.Abandon());
    TestEqual(TEXT("abandon undoes the scaling"), Log.Offer(Second).Count, SecondCount);
    TestTrue(TEXT("accept again"), Log.Accept(Second));
    TestEqual(TEXT("scaling applies once"), Log.Offer(Second).Count, FMath::Min(SecondCount + 1, 8));
    TestEqual(TEXT("acquaintance bonus paid"), Log.Payout(Second), Log.Offer(Second).Reward + 25);
    for (int32 i = 0; i < Log.Offer(Second).Count; ++i) { Log.Report(EJamStageVerb::Interact); }
    TestTrue(TEXT("turn in follow-up"), Log.TurnIn(Second));
    TestEqual(TEXT("two jobs: friend"), Log.Level(Mayor), 2);

    // Friend: the encore opens, in two steps, and does not count toward the ten.
    const int32 Encore = Log.ForGiver(TEXT("GiverMayor"));
    TestTrue(TEXT("giver offers an encore"), Log.Offer(Encore).bEncore);
    TestEqual(TEXT("encore available"), Log.State(Encore), EJamVillageQuestState::Available);
    TestEqual(TEXT("friend's encore has two steps"), Log.StepCount(Encore), 2);
    const TArray<FJamQuestStep> Steps = JamVillageQuests::Steps(Log.Offer(Encore));
    for (const FJamQuestStep& S : Steps)
    {
        const FJamLocation* Camp = JamWorld::Find(S.Destination);
        TestTrue(TEXT("every step at a real camp"), Camp && Camp->Kind == EJamLocationKind::Camp);
        TestTrue(TEXT("step count in range"), S.Count >= 1 && S.Count <= 8);
    }
    TestTrue(TEXT("steps differ"), Steps[0].Verb != Steps[1].Verb || Steps[0].Target != Steps[1].Target || Steps[0].Destination != Steps[1].Destination);
    TestTrue(TEXT("accept encore"), Log.Accept(Encore));
    TestEqual(TEXT("encore not rescaled on accept"), Log.StepCount(Encore), 2);
    TestFalse(TEXT("second step's verb does nothing yet"), Steps[1].Verb != Steps[0].Verb && Log.Report(Steps[1].Verb));
    for (int32 i = 0; i < Steps[0].Count; ++i) { TestTrue(TEXT("first step advances"), Log.Report(Steps[0].Verb)); }
    TestEqual(TEXT("moved to step two"), Log.Step(), 1);
    TestTrue(TEXT("step event"), Events.Contains(EJamQuestEvent::StepDone));
    TestEqual(TEXT("still active between steps"), Log.State(Encore), EJamVillageQuestState::Active);
    TestEqual(TEXT("current step is the second"), Log.CurrentStep(Encore).Destination, Steps[1].Destination);
    for (int32 i = 0; i < Steps[1].Count; ++i) { TestTrue(TEXT("second step advances"), Log.Report(Steps[1].Verb)); }
    TestEqual(TEXT("ready after the last step"), Log.State(Encore), EJamVillageQuestState::Ready);
    TestEqual(TEXT("friend bonus"), Log.Payout(Encore), Log.Offer(Encore).Reward + 50);
    TestTrue(TEXT("turn in encore"), Log.TurnIn(Encore));
    TestEqual(TEXT("three jobs: bestie"), Log.Level(Mayor), 3);
    TestEqual(TEXT("encores do not count toward the ten"), Log.Completed(), 2);
    TestEqual(TEXT("encore comes back"), Log.State(Encore), EJamVillageQuestState::Available);
    int32 Before = 0, After = 0;
    for (const FJamQuestStep& S : Steps) { Before += S.Count; }
    for (const FJamQuestStep& S : JamVillageQuests::Steps(Log.Offer(Encore))) { After += S.Count; }
    TestTrue(TEXT("the next encore is bigger"), After > Before);
    const FJamScreenView Journal = JamScreens::Quests(Log, FJamMenuSettings());
    TestTrue(TEXT("journal shows the relationship"), Journal.Quotes[0].Contains(TEXT("Bestie")));
    TestTrue(TEXT("journal still counts ten"), Journal.Kicker.Contains(TEXT("OF 10")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FJamQuestProposalTest, "Jam.World.Quests.Proposals", VillageTestFlags)
bool FJamQuestProposalTest::RunTest(const FString&)
{
    const FString Good = TEXT(R"({"v":1,"source":"model","quest_id":"council_rats","label":"Snack diplomacy","pitch":"Calm the rats, then win them over with celery.","reaction":"Bribery! Wonderful.","steps":[{"verb":"gather","target":"celery","count":3,"destination":"CampMeadowEast"},{"verb":"feed","target":"rat","count":3,"destination":"CampMeadowWest"}]})");
    FJamQuestPlan Plan;
    TestTrue(TEXT("contract example parses"), JamVillageQuests::ReadProposal(Good, TEXT("council_rats"), 0, Plan));
    TestEqual(TEXT("two steps"), Plan.Steps.Num(), 2);
    TestEqual(TEXT("gather is collect"), Plan.Steps[0].Verb, EJamStageVerb::Collect);
    TestEqual(TEXT("celery mesh"), Plan.Steps[0].PickupMesh, FString(TEXT("SM_Celery")));
    TestEqual(TEXT("feed is interact on rats"), Plan.Steps[1].Verb, EJamStageVerb::Interact);
    TestEqual(TEXT("reaction kept"), Plan.Reaction, FString(TEXT("Bribery! Wonderful.")));
    TestEqual(TEXT("step text"), JamVillageQuests::StepText(Plan.Steps[1]), FString(TEXT("Feed 3 rats celery at Meadow West")));

    TestFalse(TEXT("cache is fine, refused is not"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("\"model\""), TEXT("\"refused\"")), TEXT("council_rats"), 0, Plan));
    TestTrue(TEXT("cache accepted"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("\"model\""), TEXT("\"cache\"")), TEXT("council_rats"), 0, Plan));
    TestFalse(TEXT("wrong quest"), JamVillageQuests::ReadProposal(Good, TEXT("actual_nuts"), 0, Plan));
    TestFalse(TEXT("unknown verb"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("\"feed\""), TEXT("\"summon\"")), TEXT("council_rats"), 0, Plan));
    TestFalse(TEXT("feeding goblins is not a thing"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("\"target\":\"rat\""), TEXT("\"target\":\"goblin\"")), TEXT("council_rats"), 0, Plan));
    TestFalse(TEXT("new target types refused"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("\"celery\""), TEXT("\"dragon\"")), TEXT("council_rats"), 0, Plan));
    TestFalse(TEXT("gathering where nothing grows"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("CampMeadowEast"), TEXT("CampMeadowWest")), TEXT("council_rats"), 0, Plan));
    TestFalse(TEXT("village is not a camp"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("CampMeadowWest"), TEXT("Village")), TEXT("council_rats"), 0, Plan));
    TestFalse(TEXT("fractional count"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("\"count\":3,\"destination\":\"CampMeadowWest\""), TEXT("\"count\":2.5,\"destination\":\"CampMeadowWest\"")), TEXT("council_rats"), 0, Plan));
    TestFalse(TEXT("empty reaction"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("Bribery! Wonderful."), TEXT("<>")), TEXT("council_rats"), 0, Plan));
    TestFalse(TEXT("not json"), JamVillageQuests::ReadProposal(TEXT("Let's stick to the script"), TEXT("council_rats"), 0, Plan));

    TestTrue(TEXT("big counts clamp"), JamVillageQuests::ReadProposal(Good.Replace(TEXT("\"count\":3"), TEXT("\"count\":50")), TEXT("council_rats"), 0, Plan));
    TestEqual(TEXT("count clamped to 8"), Plan.Steps[0].Count, 8);
    TestEqual(TEXT("total clamped to 12"), Plan.Steps[0].Count + Plan.Steps[1].Count, 12);

    const FString Merge = TEXT(R"({"v":1,"source":"model","quest_id":"council_rats","label":"Calm then feed","pitch":"p","reaction":"r","steps":[{"verb":"pacify","target":"rat","count":3,"destination":"CampMeadowWest"},{"verb":"feed","target":"rat","count":2,"destination":"CampMeadowWest"}]})");
    TestTrue(TEXT("merge parses"), JamVillageQuests::ReadProposal(Merge, TEXT("council_rats"), 0, Plan));
    TestEqual(TEXT("pacify then feed merges"), Plan.Steps.Num(), 1);
    TestEqual(TEXT("merged step feeds them all"), Plan.Steps[0].Count, 3);

    const FString Three = TEXT(R"({"v":1,"source":"model","quest_id":"council_rats","label":"Big","pitch":"p","reaction":"r","steps":[{"verb":"pacify","target":"rat","count":2,"destination":"CampMeadowWest"},{"verb":"gather","target":"almond","count":2,"destination":"CampMeadowEast"},{"verb":"pacify","target":"bug","count":2,"destination":"CampOrchardNear"}]})");
    TestTrue(TEXT("three-step plan parses"), JamVillageQuests::ReadProposal(Three, TEXT("council_rats"), 0, Plan));
    TestEqual(TEXT("strangers get two steps"), Plan.Steps.Num(), 2);
    TestTrue(TEXT("three-step plan parses for a friend"), JamVillageQuests::ReadProposal(Three, TEXT("council_rats"), 2, Plan));
    TestEqual(TEXT("third camp dropped"), Plan.Steps.Num(), 2);

    // Selecting a plan: the client's reward, no level scaling, history says proposal.
    FJamVillageQuestLog Log; Log.Recruit(); Log.FinishMain();
    TestTrue(TEXT("good plan"), JamVillageQuests::ReadProposal(Good, TEXT("council_rats"), 0, Plan));
    FJamQuestBranch Branch; Branch.Route = TEXT("proposal"); Branch.Label = Plan.Label; Branch.Plan = Plan.Steps; Branch.Variation.Pitch = Plan.Pitch;
    const int32 Expected = Log.PlanReward(0, Plan.Steps);
    TestTrue(TEXT("select plan"), Log.SelectBranch(0, Branch));
    TestEqual(TEXT("plan reward from the client formula"), Log.Offer(0).Reward, Expected);
    TestTrue(TEXT("reward bounded"), Expected >= 50 && Expected <= 300);
    TestTrue(TEXT("accept plan"), Log.Accept(0));
    TestEqual(TEXT("plan steps kept"), Log.StepCount(0), 2);
    for (int32 i = 0; i < 3; ++i) { Log.Report(EJamStageVerb::Collect); }
    TestEqual(TEXT("plan's second step"), Log.CurrentStep(0).Verb, EJamStageVerb::Interact);
    for (int32 i = 0; i < 3; ++i) { Log.Report(EJamStageVerb::Interact); }
    TestTrue(TEXT("plan turns in"), Log.TurnIn(0));
    TestTrue(TEXT("history records the proposal"), Log.CompletedRoutes().Contains(TEXT("council_rats:proposal")));
    TestEqual(TEXT("a plan earns the relationship too"), Log.Level(0), 1);

    FJamScreenView Sample;
    TestTrue(TEXT("proposal UI can be previewed"), JamScreens::Sample(TEXT("questproposal"), Sample));
    TestTrue(TEXT("plan is the third route"), Sample.Options.Num() >= 3 && Sample.Options[2].Action == EJamMenuAction::QuestBranch2);
    FJamQuestOfferView Offer; Offer.bVisible = true; Offer.bCanPropose = true; Offer.Branches.SetNum(2);
    const FJamScreenView Screen = JamScreens::QuestBranches(Offer, FJamMenuSettings());
    TestTrue(TEXT("propose button when online"), Screen.Options.ContainsByPredicate([](const FJamMenuOption& O) { return O.Action == EJamMenuAction::ProposePlan && O.bEnabled; }));
    return true;
}
#endif
