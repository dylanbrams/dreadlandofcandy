#pragma once

#include "CoreMinimal.h"
#include "Film/JamFilmTypes.h"

enum class EJamVillageQuestState : uint8 { Locked, Available, Active, Ready, Complete };
enum class EJamQuestEvent : uint8 { Recruited, MainComplete, Accepted, Progress, Ready, TurnedIn, Abandoned, StepDone, LevelUp };

struct FJamVillageGiverDef { FName Location; FString Name; };

/**
 * One objective of a village job (Fixes2 #4, #5). Authored jobs have one; relationship levels and player plans add
 * more. Interact = feed (rats) or brush (goblins), and includes pacifying first. Collect = gather pickups (PickupMesh).
 */
struct FJamQuestStep
{
    EJamStageVerb Verb = EJamStageVerb::Pacify;
    EJamTargetKind Target = EJamTargetKind::Rat;
    int32 Count = 1;
    FName Destination;
    FString PickupMesh = TEXT("SM_Almond");
    FString Objective = TEXT("Rats pacified");
};

struct FJamVillageQuestDef
{
    FName GiverLocation;
    FName Destination;
    FString GiverName;
    FString Title;
    FString Request;
    EJamStageVerb Verb = EJamStageVerb::Pacify;
    int32 Count = 1;
    int32 Reward = 100;
    FName Id;
    FName Prerequisite;
    EJamTargetKind Target = EJamTargetKind::Rat;
    FString PickupMesh = TEXT("SM_Almond");
    FString Objective = TEXT("Rats pacified");
    TArray<FName> Destinations;
    int32 MinCount = 1;
    int32 MaxCount = 8;
    FName Route = TEXT("original");
    /** Steps after the first (the first is Verb/Target/Count/Destination above). */
    TArray<FJamQuestStep> Extra;
    /** A giver's repeatable job after their two authored ones; never counts toward Completed(). */
    bool bEncore = false;
};

/** A player's proposal as the backend rewrote it, validated and clamped (QuestBranches.md, "Proposals"). */
struct FJamQuestPlan
{
    FString Label;
    FString Pitch;
    FString Reaction;
    TArray<FJamQuestStep> Steps;
};

/** Narrow hook for authored tools or a future generated-quest service. Gameplay stays within a recipe. */
struct FJamQuestVariation
{
    FName Destination;
    int32 Count = 0; // zero keeps the recipe's count
    FString Pitch; // optional cosmetic description; bounded and cleaned before use
};

struct FJamQuestBranch
{
    FName Route;
    FString Label;
    FString Objective;
    FJamQuestVariation Variation;
    /** Non-empty: a player plan (route "proposal"); the steps replace the recipe's. */
    TArray<FJamQuestStep> Plan;
    /** Shown reward when set (plans, encores); otherwise the screen scales the base reward by count. */
    int32 Reward = 0;
};

struct FJamQuestOfferView
{
    bool bVisible = false;
    bool bReading = false;
    bool bCanImprovise = false;
    FString Title;
    FString Giver;
    FString Request;
    FString Status;
    int32 BaseCount = 1;
    int32 BaseReward = 100;
    TArray<FJamQuestBranch> Branches;
    /** The giver's relationship (Fixes2 #5): 0 Stranger .. 3 Bestie, and what it does to this job. */
    int32 Level = 0;
    FString LevelNote;
    int32 LevelBonus = 0;
    /** Player proposal (Fixes2 #4): offered while online, once per offer. */
    bool bCanPropose = false;
    bool bProposing = false;
};

struct FJamVillageQuestEvent
{
    EJamQuestEvent Type;
    FName Quest;
    int32 Progress = 0;
    int32 Required = 0;
};
DECLARE_MULTICAST_DELEGATE_OneParam(FOnJamVillageQuestEvent, const FJamVillageQuestEvent&);

namespace JamVillageQuests
{
    MEGAJAM_API const TArray<FJamVillageGiverDef>& Givers();
    MEGAJAM_API const TArray<FJamVillageQuestDef>& All();
    MEGAJAM_API TArray<FJamVillageQuestDef> Routes(int32 QuestIndex);
    MEGAJAM_API bool ReadBranches(const FString& Json, int32 QuestIndex, TArray<FJamQuestBranch>& Out);

    /** Every step of a job, in order: its own first, then Extra. */
    MEGAJAM_API TArray<FJamQuestStep> Steps(const FJamVillageQuestDef& Quest);
    MEGAJAM_API FJamQuestStep FirstStep(const FJamVillageQuestDef& Quest);
    /** "Pacify 3 rats at Meadow West". */
    MEGAJAM_API FString StepText(const FJamQuestStep& Step);
    /** Relationship names: Stranger, Acquaintance, Friend, Bestie (capped at 3). */
    constexpr int32 MaxLevel = 3;
    MEGAJAM_API FString LevelName(int32 Level);
    /** Bonus sprinkles a giver adds at this level. */
    MEGAJAM_API int32 LevelBonus(int32 Level);
    /** What the level does to a job: level 1 one more target, 2 an extra step, 3 another target and a second camp. */
    MEGAJAM_API FJamVillageQuestDef Scale(const FJamVillageQuestDef& Quest, int32 Level);
    /** A giver's repeatable encore after Jobs paid jobs (built at least at Friend); bigger every time. */
    MEGAJAM_API FJamVillageQuestDef Encore(int32 GiverIndex, int32 Jobs);
    /** Validate and clamp a /quest/propose reply (contract v1). False = authored routes only. */
    MEGAJAM_API bool ReadProposal(const FString& Json, FName QuestId, int32 Level, FJamQuestPlan& Out);
    /** Clamp plan steps: count 1-8, total 12, steps by level, pacify+feed/brush of the same target merged. */
    MEGAJAM_API TArray<FJamQuestStep> ClampPlan(TArray<FJamQuestStep> Steps, int32 Level);
    /** Contract names: "pacify" "feed" "brush" "gather"; "rat" "goblin" "bug" "almond" "celery" "candy_shard". */
    MEGAJAM_API FString VerbName(const FJamQuestStep& Step);
    MEGAJAM_API FString TargetName(const FJamQuestStep& Step);
    MEGAJAM_API bool MakeStep(const FString& Verb, const FString& Target, int32 Count, FName Destination, FJamQuestStep& Out);
    /** Monsters get tougher with village jobs paid in total: Resolve x(1 + 0.10 * jobs). */
    MEGAJAM_API float ThreatScale(int32 TotalJobs);
    /** Their attack damage grows further: x(1 + 0.06 * jobs), on top of ThreatScale's effect on Resolve. */
    MEGAJAM_API float ThreatDamageScale(int32 TotalJobs);
}

/** Session quest rules. Rewards require objectives AND a return to the giver. Offers are stable once accepted. */
class MEGAJAM_API FJamVillageQuestLog
{
public:
    explicit FJamVillageQuestLog(int32 Seed = 2026);
    bool Recruit();
    bool IsRecruited() const { return bRecruited; }
    bool FinishMain();
    bool IsMainComplete() const { return bMainComplete; }
    EJamVillageQuestState State(int32 Index) const;
    bool Accept(int32 Index);
    bool Report(EJamStageVerb Verb);
    bool TurnIn(int32 Index);
    bool Abandon();
    int32 Active() const { return ActiveIndex; }
    int32 Progress() const { return Done; }
    int32 Completed() const { return Finished.Num(); }
    int32 Num() const { return Offers.Num(); }
    const FJamVillageQuestDef& Offer(int32 Index) const { return Offers[Index]; }
    int32 ForGiver(FName Location) const;
    int32 GiverIndex(int32 QuestIndex) const;
    /** Rejects foreign destinations; clamps counts and recalculates rewards. Never rewrites accepted work. */
    bool ApplyVariation(int32 Index, const FJamQuestVariation& Variation);
    /** Versioned JSON offer hook. Transactional: any invalid override preserves every existing offer. */
    bool ApplyOverridesJson(const FString& Json, FString& Error);
    bool SelectBranch(int32 Index, const FJamQuestBranch& Branch);
    TArray<FName> CompletedIds() const;
    TArray<FString> CompletedRoutes() const;
    /** Steps (Fixes2 #4, #5): the active job's current step; any other job's first. */
    int32 Step() const { return StepIndex; }
    /** Defeat on a dangerous errand: the current step's progress goes back to 0. */
    void RestartStep() { if (ActiveIndex != INDEX_NONE) { Done = 0; } }
    int32 StepCount(int32 Index) const;
    FJamQuestStep CurrentStep(int32 Index) const;
    /** Relationship with a giver (index into Givers()): jobs paid so far, capped at MaxLevel. */
    int32 Level(int32 GiverIndex) const;
    int32 LevelForQuest(int32 Index) const { return Level(GiverIndex(Index)); }
    /** Jobs paid by a giver, uncapped (encores keep counting). */
    int32 Jobs(int32 GiverIndex) const;
    /** Jobs paid across every giver. */
    int32 TotalJobs() const
    {
        int32 N = 0;
        for (int32 i = 0; i < JamVillageQuests::Givers().Num(); ++i) { N += Jobs(i); }
        return N;
    }
    /** What turning this job in pays: its reward plus the giver's level bonus. */
    int32 Payout(int32 Index) const;
    /** Fixes3 #10b: encores a giver still has (2-4 each, seeded); at 0 they are done and give their afterglow. */
    int32 EncoresLeft(int32 GiverIndex) const;
    /** Authored jobs only (10); encores are extra. */
    int32 NumAuthored() const { return JamVillageQuests::All().Num(); }
    /** Start a player plan instead of a recipe route. */
    bool SelectPlan(int32 Index, const FJamQuestPlan& Plan);
    /** What SelectPlan would set as the job's reward (before the level bonus). */
    int32 PlanReward(int32 Index, const TArray<FJamQuestStep>& Steps) const;
    FOnJamVillageQuestEvent OnChanged;
private:
    void Emit(EJamQuestEvent Event, int32 Index = INDEX_NONE);
    TArray<FJamVillageQuestDef> Offers;
    bool bRecruited = false;
    bool bMainComplete = false;
    int32 ActiveIndex = INDEX_NONE;
    int32 Done = 0;
    int32 StepIndex = 0;
    TSet<int32> Finished;
    TArray<int32> GiverJobs;
    TArray<int32> EncoreQuota;   // per giver, encores left
    TSet<int32> Retired;         // encores whose giver has run out of work
    FJamVillageQuestDef AcceptedFrom;
};
