#pragma once

#include "CoreMinimal.h"
#include "Film/JamFilmTypes.h"

enum class EJamVillageQuestState : uint8 { Locked, Available, Active, Ready, Complete };
enum class EJamQuestEvent : uint8 { Recruited, MainComplete, Accepted, Progress, Ready, TurnedIn, Abandoned };

struct FJamVillageGiverDef { FName Location; FString Name; };

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
    FOnJamVillageQuestEvent OnChanged;
private:
    void Emit(EJamQuestEvent Event, int32 Index = INDEX_NONE);
    TArray<FJamVillageQuestDef> Offers;
    bool bRecruited = false;
    bool bMainComplete = false;
    int32 ActiveIndex = INDEX_NONE;
    int32 Done = 0;
    TSet<int32> Finished;
};
