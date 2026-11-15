#include "World/JamVillageQuests.h"
#include "World/JamWorld.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

const TArray<FJamVillageGiverDef>& JamVillageQuests::Givers()
{
    static const TArray<FJamVillageGiverDef> G = {
        { TEXT("GiverMayor"), TEXT("Mayor Bonbon") }, { TEXT("GiverBaker"), TEXT("Baker Crumb") },
        { TEXT("GiverGardener"), TEXT("Gardener Sprig") }, { TEXT("GiverDentist"), TEXT("Dr. Molar") },
        { TEXT("GiverScout"), TEXT("Scout Taffy") }
    };
    return G;
}

float JamVillageQuests::ThreatScale(int32 TotalJobs)
{
    return 1.f + 0.10f * FMath::Max(0, TotalJobs);
}

float JamVillageQuests::ThreatDamageScale(int32 TotalJobs)
{
    return 1.f + 0.06f * FMath::Max(0, TotalJobs);
}

const TArray<FJamVillageQuestDef>& JamVillageQuests::All()
{
    static const TArray<FJamVillageQuestDef> Quests = []
    {
        TArray<FJamVillageQuestDef> Out;
        auto Add = [&](const TCHAR* Id, int32 Giver, const TCHAR* Title, const TCHAR* Request, EJamStageVerb Verb,
            EJamTargetKind Target, int32 Count, int32 Reward, const TCHAR* Objective, const TCHAR* Mesh,
            TArray<FName> Camps, const TCHAR* Prerequisite = TEXT(""))
        {
            FJamVillageQuestDef Q;
            Q.Id = Id; Q.GiverLocation = Givers()[Giver].Location; Q.GiverName = Givers()[Giver].Name;
            Q.Title = Title; Q.Request = Request; Q.Verb = Verb; Q.Target = Target; Q.Count = Count; Q.Reward = Reward;
            Q.Objective = Objective; Q.PickupMesh = Mesh; Q.Destinations = MoveTemp(Camps); Q.Destination = Q.Destinations[0];
            Q.Prerequisite = Prerequisite; Q.MinCount = FMath::Max(2, Count - 1); Q.MaxCount = FMath::Min(8, Count + 1);
            Out.Add(MoveTemp(Q));
        };
        using V = EJamStageVerb; using K = EJamTargetKind;
        Add(TEXT("council_rats"), 0, TEXT("A Quiet Constituency"),
            TEXT("Three rats are holding a public meeting in the meadow. Pacify them. We already have a council."),
            V::Pacify, K::Rat, 3, 150, TEXT("Rats pacified"), TEXT("SM_Almond"), { TEXT("CampMeadowWest") });
        Add(TEXT("actual_nuts"), 1, TEXT("Contains Actual Nuts"),
            TEXT("Bring me five almonds from the meadow. My almond cake is currently a work of fiction."),
            V::Collect, K::Almond, 5, 120, TEXT("Almonds gathered"), TEXT("SM_Almond"), { TEXT("CampMeadowEast") });
        Add(TEXT("celery_actually"), 2, TEXT("Celery, Actually"),
            TEXT("Gather celery from the meadow. It is the only crop here that has not applied for dessert status."),
            V::Collect, K::Almond, 4, 130, TEXT("Celery gathered"), TEXT("SM_Celery"), { TEXT("CampMeadowEast"), TEXT("CampMeadowWest") });
        Add(TEXT("open_wide"), 3, TEXT("Open Wide, Shut Up"),
            TEXT("Pacify the goblins in the hills, then brush their teeth. A stern word is preparation, not dentistry."),
            V::Interact, K::Goblin, 3, 180, TEXT("Goblins brushed"), TEXT("SM_Almond"), { TEXT("CampMolarsNear"), TEXT("CampMolarsRidge") });
        Add(TEXT("uncredited_extras"), 4, TEXT("Uncredited Extras"),
            TEXT("Pacify the orchard bugs. They keep wandering into shots and asking about their motivation."),
            V::Pacify, K::Bug, 4, 160, TEXT("Bugs pacified"), TEXT("SM_Almond"), { TEXT("CampOrchardNear"), TEXT("CampOrchardGrove") });
        Add(TEXT("goblin_representative"), 0, TEXT("A More Representative Goblin"),
            TEXT("Our new representatives have arrived. Pacify the goblins and brush them before the inaugural smile."),
            V::Interact, K::Goblin, 3, 200, TEXT("Goblins brushed"), TEXT("SM_Almond"), { TEXT("CampMolarsNear"), TEXT("CampMolarsFar") }, TEXT("council_rats"));
        Add(TEXT("cake_has_teeth"), 1, TEXT("The Cake Has Teeth"),
            TEXT("Gather fallen candy shards in the orchard. Decorative teeth, hero. The cake is not to bite back."),
            V::Collect, K::Almond, 5, 160, TEXT("Candy shards gathered"), TEXT("SM_LollyShard"), { TEXT("CampOrchardNear"), TEXT("CampOrchardFar") }, TEXT("actual_nuts"));
        Add(TEXT("crop_circles"), 2, TEXT("Crop Circles Are Rude"),
            TEXT("Pacify the bugs flattening my crops. If they want to leave a message, they can write a letter."),
            V::Pacify, K::Bug, 5, 190, TEXT("Bugs pacified"), TEXT("SM_Almond"), { TEXT("CampOrchardGrove"), TEXT("CampOrchardFar") }, TEXT("celery_actually"));
        Add(TEXT("brush_diplomacy"), 3, TEXT("Brushing Up On Diplomacy"),
            TEXT("The rats have agreed to a healthy snack. Pacify them, then feed them celery. Yes, it counts as diplomacy."),
            V::Interact, K::Rat, 4, 180, TEXT("Rats fed celery"), TEXT("SM_Almond"), { TEXT("CampMeadowWest"), TEXT("CampMeadowFar") }, TEXT("open_wide"));
        Add(TEXT("nuts_horizon"), 4, TEXT("Nuts About The Horizon"),
            TEXT("Gather almonds from a distant camp for my next survey. Exploration is mostly finding somewhere to eat."),
            V::Collect, K::Almond, 6, 190, TEXT("Almonds gathered"), TEXT("SM_Almond"), { TEXT("CampMeadowEast"), TEXT("CampOrchardFar") }, TEXT("uncredited_extras"));
        return Out;
    }();
    return Quests;
}

FJamVillageQuestLog::FJamVillageQuestLog(int32 Seed) : Offers(JamVillageQuests::All())
{
    // Keep the introductory pair authored; the rest vary reproducibly between runs.
    FRandomStream Random(Seed);
    for (int32 i = 2; i < Offers.Num(); ++i)
    {
        FJamQuestVariation Variation;
        Variation.Count = Random.RandRange(Offers[i].MinCount, Offers[i].MaxCount);
        Variation.Destination = Offers[i].Destinations[Random.RandHelper(Offers[i].Destinations.Num())];
        ApplyVariation(i, Variation);
    }
    // Fixes2 #5: each giver's repeatable encore, unlocked by their second authored job.
    GiverJobs.SetNumZeroed(JamVillageQuests::Givers().Num());
    for (int32 g = 0; g < JamVillageQuests::Givers().Num(); ++g)
    {
        Offers.Add(JamVillageQuests::Encore(g, 2));
        // Fixes3 #10b: each villager has a different amount of work: 2 authored jobs plus 2-4 encores.
        EncoreQuota.Add(Random.RandRange(2, 4));
    }
}

int32 FJamVillageQuestLog::EncoresLeft(int32 GiverIndex) const
{
    return EncoreQuota.IsValidIndex(GiverIndex) ? EncoreQuota[GiverIndex] : 0;
}

bool FJamVillageQuestLog::ApplyVariation(int32 Index, const FJamQuestVariation& Variation)
{
    if (!Offers.IsValidIndex(Index) || !JamVillageQuests::All().IsValidIndex(Index) || State(Index) == EJamVillageQuestState::Active
        || State(Index) == EJamVillageQuestState::Ready || Finished.Contains(Index)) { return false; }
    FJamVillageQuestDef& Q = Offers[Index];
    if (!Variation.Destination.IsNone())
    {
        const FJamLocation* Destination = JamWorld::Find(Variation.Destination);
        if (!Destination || Destination->Kind != EJamLocationKind::Camp || !Q.Destinations.Contains(Variation.Destination)) { return false; }
    }
    if (!Variation.Destination.IsNone()) { Q.Destination = Variation.Destination; }
    if (Variation.Count != 0)
    {
        Q.Count = FMath::Clamp(Variation.Count, Q.MinCount, Q.MaxCount);
        const FJamVillageQuestDef& Base = JamVillageQuests::All()[Index];
        Q.Reward = FMath::Clamp(FMath::RoundToInt(float(Base.Reward) * Q.Count / Base.Count), 50, 300);
    }
    if (!Variation.Pitch.IsEmpty())
    {
        FString Clean;
        for (TCHAR C : Variation.Pitch) { if (C >= 32 && C != 127) { Clean.AppendChar(C); } }
        Clean = Clean.Left(240).TrimStartAndEnd();
        if (!Clean.IsEmpty()) { Q.Request = Clean; }
    }
    return true;
}

void FJamVillageQuestLog::Emit(EJamQuestEvent Type, int32 Index)
{
    FJamVillageQuestEvent Event { Type, Offers.IsValidIndex(Index) ? Offers[Index].Id : NAME_None,
        Done, Offers.IsValidIndex(Index) ? CurrentStep(Index).Count : 0 };
    OnChanged.Broadcast(Event);
}

bool FJamVillageQuestLog::Recruit()
{
    if (bRecruited) { return false; }
    bRecruited = true; Emit(EJamQuestEvent::Recruited); return true;
}
bool FJamVillageQuestLog::FinishMain()
{
    if (!bRecruited || bMainComplete) { return false; }
    bMainComplete = true; Emit(EJamQuestEvent::MainComplete); return true;
}
EJamVillageQuestState FJamVillageQuestLog::State(int32 Index) const
{
    if (!Offers.IsValidIndex(Index) || !bMainComplete) { return EJamVillageQuestState::Locked; }
    if (Finished.Contains(Index) || Retired.Contains(Index)) { return EJamVillageQuestState::Complete; }
    if (ActiveIndex == Index)
    {
        const bool bLast = StepIndex >= StepCount(Index) - 1;
        return bLast && Done >= CurrentStep(Index).Count ? EJamVillageQuestState::Ready : EJamVillageQuestState::Active;
    }
    if (!Offers[Index].Prerequisite.IsNone())
    {
        const int32 Parent = Offers.IndexOfByPredicate([&](const FJamVillageQuestDef& Q) { return Q.Id == Offers[Index].Prerequisite; });
        if (!Finished.Contains(Parent)) { return EJamVillageQuestState::Locked; }
    }
    return EJamVillageQuestState::Available;
}
int32 FJamVillageQuestLog::ForGiver(FName Location) const
{
    if (ActiveIndex != INDEX_NONE && Offers[ActiveIndex].GiverLocation == Location) { return ActiveIndex; }
    for (int32 i = 0; i < Offers.Num(); ++i) { if (Offers[i].GiverLocation == Location && !Finished.Contains(i) && !Retired.Contains(i)) { return i; } }
    return INDEX_NONE;
}
int32 FJamVillageQuestLog::GiverIndex(int32 Index) const
{
    if (!Offers.IsValidIndex(Index)) { return INDEX_NONE; }
    return JamVillageQuests::Givers().IndexOfByPredicate([&](const FJamVillageGiverDef& G) { return G.Location == Offers[Index].GiverLocation; });
}
bool FJamVillageQuestLog::Accept(int32 Index)
{
    if (ActiveIndex != INDEX_NONE || State(Index) != EJamVillageQuestState::Available) { return false; }
    AcceptedFrom = Offers[Index];
    // The giver's level shapes the job (Fixes2 #5); a player's plan is taken as written.
    if (Offers[Index].Route != FName(TEXT("proposal"))) { Offers[Index] = JamVillageQuests::Scale(Offers[Index], LevelForQuest(Index)); }
    ActiveIndex = Index; Done = 0; StepIndex = 0; Emit(EJamQuestEvent::Accepted, Index); return true;
}
bool FJamVillageQuestLog::Report(EJamStageVerb Verb)
{
    if (ActiveIndex == INDEX_NONE || State(ActiveIndex) != EJamVillageQuestState::Active) { return false; }
    const FJamQuestStep Step = CurrentStep(ActiveIndex);
    if (Step.Verb != Verb) { return false; }
    Done = FMath::Min(Done + 1, Step.Count);
    if (Done >= Step.Count && StepIndex < StepCount(ActiveIndex) - 1)
    {
        ++StepIndex; Done = 0;
        Emit(EJamQuestEvent::StepDone, ActiveIndex);
        return true;
    }
    Emit(Done >= Step.Count ? EJamQuestEvent::Ready : EJamQuestEvent::Progress, ActiveIndex);
    return true;
}
bool FJamVillageQuestLog::TurnIn(int32 Index)
{
    if (State(Index) != EJamVillageQuestState::Ready) { return false; }
    const int32 Giver = GiverIndex(Index);
    const int32 Before = Level(Giver);
    if (GiverJobs.IsValidIndex(Giver)) { ++GiverJobs[Giver]; }
    if (!Offers[Index].bEncore) { Finished.Add(Index); }
    if (Level(Giver) > Before) { Emit(EJamQuestEvent::LevelUp, Index); }
    Emit(EJamQuestEvent::TurnedIn, Index);
    // An encore comes back bigger: the next one is built for the new relationship.
    if (Offers[Index].bEncore)
    {
        if (EncoreQuota.IsValidIndex(Giver)) { EncoreQuota[Giver] = FMath::Max(0, EncoreQuota[Giver] - 1); }
        if (EncoresLeft(Giver) > 0) { Offers[Index] = JamVillageQuests::Encore(Giver, Jobs(Giver)); }
        else { Retired.Add(Index); }
    }
    ActiveIndex = INDEX_NONE; Done = 0; StepIndex = 0; return true;
}

bool FJamVillageQuestLog::Abandon()
{
    if (ActiveIndex == INDEX_NONE) { return false; }
    const int32 Previous = ActiveIndex;
    Offers[Previous] = AcceptedFrom;   // undo the level scaling, so taking it again does not scale twice
    ActiveIndex = INDEX_NONE; Done = 0; StepIndex = 0;
    Emit(EJamQuestEvent::Abandoned, Previous);
    return true;
}

bool FJamVillageQuestLog::ApplyOverridesJson(const FString& Json, FString& Error)
{
    Error.Reset();
    auto Fail = [&](const TCHAR* Why) { Error = Why; return false; };
    if (Json.Len() > 32768) { return Fail(TEXT("Offer JSON exceeds 32K characters.")); }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid()) { return Fail(TEXT("Invalid offer JSON.")); }
    double Version = 0;
    const TArray<TSharedPtr<FJsonValue>>* Overrides = nullptr;
    if (!Root->TryGetNumberField(TEXT("v"), Version) || Version != 1
        || !Root->TryGetArrayField(TEXT("offers"), Overrides) || Overrides->Num() > Offers.Num()) { return Fail(TEXT("Expected v:1 and an offers array within the quest count.")); }
    FJamVillageQuestLog Candidate = *this;
    TSet<FName> Seen;
    for (const TSharedPtr<FJsonValue>& Value : *Overrides)
    {
        const TSharedPtr<FJsonObject>* Object = nullptr;
        FString Id;
        if (!Value->TryGetObject(Object) || !(*Object)->TryGetStringField(TEXT("id"), Id)) { return Fail(TEXT("Each override needs a quest id.")); }
        const FName QuestId(*Id);
        const int32 Index = Offers.IndexOfByPredicate([&](const FJamVillageQuestDef& Q) { return Q.Id == QuestId; });
        if (Index == INDEX_NONE || Seen.Contains(QuestId)) { return Fail(TEXT("Unknown or repeated quest id.")); }
        Seen.Add(QuestId);
        for (const auto& Field : (*Object)->Values)
        {
            if (Field.Key != TEXT("id") && Field.Key != TEXT("destination") && Field.Key != TEXT("count") && Field.Key != TEXT("pitch")) { return Fail(TEXT("Only id, destination, count, and pitch may be overridden.")); }
        }
        FJamQuestVariation Variation;
        FString Destination;
        if ((*Object)->HasField(TEXT("destination")))
        {
            if (!(*Object)->TryGetStringField(TEXT("destination"), Destination)) { return Fail(TEXT("Destination must be a camp id string.")); }
            Variation.Destination = FName(*Destination);
        }
        if ((*Object)->HasField(TEXT("pitch")) && !(*Object)->TryGetStringField(TEXT("pitch"), Variation.Pitch)) { return Fail(TEXT("Pitch must be a string.")); }
        if ((*Object)->HasField(TEXT("count")))
        {
            double Count = 0;
            if (!(*Object)->TryGetNumberField(TEXT("count"), Count) || !FMath::IsFinite(Count) || Count < 1 || Count > MAX_int32 || FMath::FloorToDouble(Count) != Count) { return Fail(TEXT("Count must be a positive integer.")); }
            Variation.Count = int32(Count);
        }
        if (!Candidate.ApplyVariation(Index, Variation)) { return Fail(TEXT("Override changes accepted work or uses a destination outside its recipe.")); }
    }
    Offers = MoveTemp(Candidate.Offers);
    return true;
}

TArray<FJamVillageQuestDef> JamVillageQuests::Routes(int32 Index)
{
    if (!All().IsValidIndex(Index)) { return {}; }
    const FJamVillageQuestDef& Base = All()[Index];
    FJamVillageQuestDef Alternative = Base;
    Alternative.Route = TEXT("alternative");
    if (Base.Verb == EJamStageVerb::Collect)
    {
        Alternative.Verb = EJamStageVerb::Pacify;
        Alternative.Target = EJamTargetKind::Rat;
        Alternative.Objective = TEXT("Rats pacified");
        Alternative.Destinations = { TEXT("CampMeadowWest"), TEXT("CampMeadowFar") };
        Alternative.Request = TEXT("Deal with the rats disrupting the deliveries instead. A firm lecture should settle the dispute.");
    }
    else if (Base.Target == EJamTargetKind::Bug)
    {
        Alternative.Verb = EJamStageVerb::Collect;
        Alternative.Target = EJamTargetKind::Almond;
        Alternative.PickupMesh = TEXT("SM_LollyShard");
        Alternative.Objective = TEXT("Candy shards gathered");
        Alternative.Request = TEXT("Gather fallen candy shards in the orchard instead. The village can put them to use without disturbing the bugs.");
    }
    else
    {
        Alternative.Verb = Base.Verb == EJamStageVerb::Interact ? EJamStageVerb::Pacify : EJamStageVerb::Interact;
        Alternative.Objective = Alternative.Verb == EJamStageVerb::Pacify ? TEXT("Dispute settled by lectures")
            : Base.Target == EJamTargetKind::Goblin ? TEXT("Goblins brushed") : TEXT("Rats fed celery");
        Alternative.Request = Alternative.Verb == EJamStageVerb::Pacify
            ? TEXT("Settle the dispute with a firm lecture instead. Pacify everyone, then report back.")
            : Base.Target == EJamTargetKind::Goblin
                ? TEXT("Give them practical help instead. Pacify the goblins, then brush their teeth before reporting back.")
                : TEXT("Give them practical help instead. Pacify the rats, then feed them celery before reporting back.");
    }
    Alternative.Destination = Alternative.Destinations[0];
    return { Base, Alternative };
}

bool FJamVillageQuestLog::SelectBranch(int32 Index, const FJamQuestBranch& Branch)
{
    if (ActiveIndex != INDEX_NONE || State(Index) != EJamVillageQuestState::Available) { return false; }
    if (!Branch.Plan.IsEmpty())
    {
        FJamQuestPlan Plan; Plan.Label = Branch.Label; Plan.Pitch = Branch.Variation.Pitch; Plan.Steps = Branch.Plan;
        return SelectPlan(Index, Plan);
    }
    if (Offers[Index].bEncore) { return Branch.Route == Offers[Index].Route; }   // one fixed route
    const TArray<FJamVillageQuestDef> Recipes = JamVillageQuests::Routes(Index);
    const FJamVillageQuestDef* Recipe = Recipes.FindByPredicate([&](const FJamVillageQuestDef& Q) { return Q.Route == Branch.Route; });
    if (!Recipe) { return false; }
    const FJamVillageQuestDef Previous = Offers[Index];
    Offers[Index] = *Recipe;
    if (!ApplyVariation(Index, Branch.Variation)) { Offers[Index] = Previous; return false; }
    return true;
}

TArray<FName> FJamVillageQuestLog::CompletedIds() const
{
    TArray<FName> Ids;
    for (int32 i = 0; i < Offers.Num(); ++i) { if (Finished.Contains(i)) { Ids.Add(Offers[i].Id); } }
    return Ids;
}

bool JamVillageQuests::ReadBranches(const FString& Json, int32 Index, TArray<FJamQuestBranch>& Out)
{
    if (!All().IsValidIndex(Index) || Json.Len() > 16384) { return false; }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid()) { return false; }
    double Version = 0;
    FString Source, QuestId;
    const TArray<TSharedPtr<FJsonValue>>* Branches = nullptr;
    if (!Root->TryGetNumberField(TEXT("v"), Version) || Version != 1
        || !Root->TryGetStringField(TEXT("source"), Source) || (Source != TEXT("model") && Source != TEXT("cache"))
        || !Root->TryGetStringField(TEXT("quest_id"), QuestId) || FName(*QuestId) != All()[Index].Id
        || !Root->TryGetArrayField(TEXT("branches"), Branches) || Branches->Num() != 2) { return false; }
    TArray<FJamQuestBranch> Parsed;
    TSet<FName> Seen;
    const TArray<FJamVillageQuestDef> Recipes = Routes(Index);
    auto Clean = [](const FString& Text, int32 Limit)
    {
        FString Result;
        for (TCHAR C : Text) { if (C >= 32 && C != 127 && C != TEXT('<') && C != TEXT('>')) { Result.AppendChar(C); } }
        return Result.Left(Limit).TrimStartAndEnd();
    };
    for (const TSharedPtr<FJsonValue>& Value : *Branches)
    {
        const TSharedPtr<FJsonObject>* Object = nullptr;
        FString Route, Label, Pitch, Destination;
        double Count = 0;
        if (!Value->TryGetObject(Object) || !(*Object)->TryGetStringField(TEXT("route"), Route)
            || !(*Object)->TryGetStringField(TEXT("label"), Label) || !(*Object)->TryGetStringField(TEXT("pitch"), Pitch)
            || !(*Object)->TryGetStringField(TEXT("destination"), Destination) || !(*Object)->TryGetNumberField(TEXT("count"), Count)
            || !FMath::IsFinite(Count) || Count < 1 || Count > MAX_int32 || FMath::FloorToDouble(Count) != Count) { return false; }
        const FName RouteId(*Route), Camp(*Destination);
        const FJamVillageQuestDef* Recipe = Recipes.FindByPredicate([&](const FJamVillageQuestDef& Q) { return Q.Route == RouteId; });
        if (!Recipe || Seen.Contains(RouteId) || !Recipe->Destinations.Contains(Camp)) { return false; }
        Label = Clean(Label, 40); Pitch = Clean(Pitch, 240);
        if (Label.IsEmpty() || Pitch.IsEmpty()) { return false; }
        FJamQuestBranch Branch;
        Branch.Route = RouteId; Branch.Label = Label; Branch.Objective = Recipe->Objective; Branch.Variation.Destination = Camp;
        Branch.Variation.Count = FMath::Clamp(int32(Count), Recipe->MinCount, Recipe->MaxCount);
        Branch.Variation.Pitch = Pitch;
        Parsed.Add(MoveTemp(Branch)); Seen.Add(RouteId);
    }
    Out = MoveTemp(Parsed);
    return true;
}

TArray<FString> FJamVillageQuestLog::CompletedRoutes() const
{
    TArray<FString> History;
    for (int32 i = 0; i < Offers.Num(); ++i)
    {
        if (Finished.Contains(i)) { History.Add(Offers[i].Id.ToString() + TEXT(":") + Offers[i].Route.ToString()); }
    }
    return History;
}

// ---- Steps, relationship levels, encores and player plans (Fixes2 #4, #5) ------------------------------------------

namespace
{
    FString CleanVillageText(const FString& Text, int32 Limit)
    {
        FString Result;
        for (TCHAR C : Text) { if (C >= 32 && C != 127 && C != TEXT('<') && C != TEXT('>')) { Result.AppendChar(C); } }
        return Result.Left(Limit).TrimStartAndEnd();
    }

    FString CampLabel(FName Camp)
    {
        FString S = Camp.ToString();
        S.RemoveFromStart(TEXT("Camp"));
        FString Out;
        for (int32 i = 0; i < S.Len(); ++i)
        {
            if (i > 0 && FChar::IsUpper(S[i])) { Out.AppendChar(TEXT(' ')); }
            Out.AppendChar(S[i]);
        }
        return Out;
    }

    FString StepObjective(const FJamQuestStep& Step)
    {
        switch (Step.Verb)
        {
        case EJamStageVerb::Collect:
            return Step.PickupMesh == TEXT("SM_Celery") ? TEXT("Celery gathered")
                : Step.PickupMesh == TEXT("SM_LollyShard") ? TEXT("Candy shards gathered") : TEXT("Almonds gathered");
        case EJamStageVerb::Interact:
            return Step.Target == EJamTargetKind::Goblin ? TEXT("Goblins brushed") : TEXT("Rats fed celery");
        default:
            return Step.Target == EJamTargetKind::Goblin ? TEXT("Goblins pacified")
                : Step.Target == EJamTargetKind::Bug ? TEXT("Bugs pacified") : TEXT("Rats pacified");
        }
    }

    bool CampAllows(FName Camp, const FJamQuestStep& Step)
    {
        const FJamLocation* L = JamWorld::Find(Camp);
        return L && L->Kind == EJamLocationKind::Camp && L->Kinds.Contains(Step.Verb == EJamStageVerb::Collect ? EJamTargetKind::Almond : Step.Target);
    }

    /** A camp that suits the step other than Avoid, from Avoid's biome first; Avoid if there is none. */
    FName OtherCamp(const FJamQuestStep& Step, FName Avoid)
    {
        const FJamLocation* Near = JamWorld::Find(Avoid);
        for (int32 Pass = 0; Pass < 2; ++Pass)
        {
            for (const FJamLocation* L : JamWorld::OfKind(EJamLocationKind::Camp))
            {
                if (L->Id == Avoid || !CampAllows(L->Id, Step)) { continue; }
                if (Pass == 0 && Near && L->Biome != Near->Biome) { continue; }
                return L->Id;
            }
        }
        return Avoid;
    }

    void SetFirst(FJamVillageQuestDef& Q, const FJamQuestStep& S)
    {
        Q.Verb = S.Verb; Q.Target = S.Target; Q.Count = S.Count; Q.Destination = S.Destination;
        Q.PickupMesh = S.PickupMesh; Q.Objective = S.Objective;
    }

    /** The extra step a job gains at Friend: something different to do, as close to the first as the world allows. */
    FJamQuestStep Complement(const FJamQuestStep& First)
    {
        FJamQuestStep S;
        S.Count = 3;
        if (First.Verb == EJamStageVerb::Collect)
        {
            // Whoever lives at the camp has opinions about the gathering.
            S.Verb = EJamStageVerb::Pacify;
            S.Target = EJamTargetKind::Rat;
            if (const FJamLocation* L = JamWorld::Find(First.Destination))
            {
                for (EJamTargetKind K : L->Kinds) { if (K != EJamTargetKind::Almond) { S.Target = K; break; } }
            }
        }
        else if (First.Verb == EJamStageVerb::Interact && First.Target == EJamTargetKind::Goblin)
        {
            S.Verb = EJamStageVerb::Pacify; S.Target = EJamTargetKind::Goblin;   // word spreads to a second camp of patients
        }
        else
        {
            // Rats get celery restocked; goblins and bugs leave candy shards everywhere.
            S.Verb = EJamStageVerb::Collect; S.Target = EJamTargetKind::Almond;
            S.PickupMesh = First.Target == EJamTargetKind::Rat ? TEXT("SM_Celery") : TEXT("SM_LollyShard");
        }
        S.Objective = StepObjective(S);
        S.Destination = CampAllows(First.Destination, S) && !(S.Verb == First.Verb && S.Target == First.Target)
            ? First.Destination : OtherCamp(S, First.Destination);
        return S;
    }

    const TCHAR* const GLevelNames[] = { TEXT("Stranger"), TEXT("Acquaintance"), TEXT("Friend"), TEXT("Bestie") };

    // Docs/Design/Jokes.md, "Villagers: encores".
    const TCHAR* const GEncoreRequests[] = {
        TEXT("The rats have re-formed the council. With subcommittees. Same job, more of it, and a second item on the agenda."),
        TEXT("Word got round about the cake. Now everyone wants one. I need more of everything, and fewer critics."),
        TEXT("The garden is famous now. Tourists. Tourists bring pests. Pests bring friends."),
        TEXT("My patients have told their friends. Their friends have teeth. Many teeth. Bring a bigger brush."),
        TEXT("My survey was a hit. The sequel needs a bigger map and something worse on it."),
    };
}

FJamQuestStep JamVillageQuests::FirstStep(const FJamVillageQuestDef& Q)
{
    FJamQuestStep S;
    S.Verb = Q.Verb; S.Target = Q.Target; S.Count = Q.Count; S.Destination = Q.Destination;
    S.PickupMesh = Q.PickupMesh; S.Objective = Q.Objective;
    return S;
}

TArray<FJamQuestStep> JamVillageQuests::Steps(const FJamVillageQuestDef& Q)
{
    TArray<FJamQuestStep> Out = { FirstStep(Q) };
    Out.Append(Q.Extra);
    return Out;
}

FString JamVillageQuests::VerbName(const FJamQuestStep& Step)
{
    if (Step.Verb == EJamStageVerb::Collect) { return TEXT("gather"); }
    if (Step.Verb == EJamStageVerb::Interact) { return Step.Target == EJamTargetKind::Goblin ? TEXT("brush") : TEXT("feed"); }
    return TEXT("pacify");
}

FString JamVillageQuests::TargetName(const FJamQuestStep& Step)
{
    if (Step.Verb == EJamStageVerb::Collect)
    {
        return Step.PickupMesh == TEXT("SM_Celery") ? TEXT("celery") : Step.PickupMesh == TEXT("SM_LollyShard") ? TEXT("candy_shard") : TEXT("almond");
    }
    return Step.Target == EJamTargetKind::Goblin ? TEXT("goblin") : Step.Target == EJamTargetKind::Bug ? TEXT("bug") : TEXT("rat");
}

bool JamVillageQuests::MakeStep(const FString& Verb, const FString& Target, int32 Count, FName Destination, FJamQuestStep& Out)
{
    FJamQuestStep S;
    S.Count = Count;
    S.Destination = Destination;
    if (Verb == TEXT("pacify"))
    {
        S.Verb = EJamStageVerb::Pacify;
        if (Target == TEXT("rat")) { S.Target = EJamTargetKind::Rat; }
        else if (Target == TEXT("goblin")) { S.Target = EJamTargetKind::Goblin; }
        else if (Target == TEXT("bug")) { S.Target = EJamTargetKind::Bug; }
        else { return false; }
    }
    else if (Verb == TEXT("feed")) { if (Target != TEXT("rat")) { return false; } S.Verb = EJamStageVerb::Interact; S.Target = EJamTargetKind::Rat; }
    else if (Verb == TEXT("brush")) { if (Target != TEXT("goblin")) { return false; } S.Verb = EJamStageVerb::Interact; S.Target = EJamTargetKind::Goblin; }
    else if (Verb == TEXT("gather"))
    {
        S.Verb = EJamStageVerb::Collect; S.Target = EJamTargetKind::Almond;
        if (Target == TEXT("almond")) { S.PickupMesh = TEXT("SM_Almond"); }
        else if (Target == TEXT("celery")) { S.PickupMesh = TEXT("SM_Celery"); }
        else if (Target == TEXT("candy_shard")) { S.PickupMesh = TEXT("SM_LollyShard"); }
        else { return false; }
    }
    else { return false; }
    S.Objective = StepObjective(S);
    if (!CampAllows(Destination, S)) { return false; }
    Out = S;
    return true;
}

FString JamVillageQuests::StepText(const FJamQuestStep& S)
{
    const FString Where = CampLabel(S.Destination);
    switch (S.Verb)
    {
    case EJamStageVerb::Collect:
        return FString::Printf(TEXT("Gather %d %s at %s"), S.Count,
            S.PickupMesh == TEXT("SM_Celery") ? TEXT("celery") : S.PickupMesh == TEXT("SM_LollyShard") ? TEXT("candy shards") : TEXT("almonds"), *Where);
    case EJamStageVerb::Interact:
        return S.Target == EJamTargetKind::Goblin ? FString::Printf(TEXT("Brush %d goblins at %s"), S.Count, *Where)
            : FString::Printf(TEXT("Feed %d rats celery at %s"), S.Count, *Where);
    default:
        return FString::Printf(TEXT("Pacify %d %s at %s"), S.Count,
            S.Target == EJamTargetKind::Goblin ? TEXT("goblins") : S.Target == EJamTargetKind::Bug ? TEXT("bugs") : TEXT("rats"), *Where);
    }
}

FString JamVillageQuests::LevelName(int32 Level) { return GLevelNames[FMath::Clamp(Level, 0, MaxLevel)]; }

int32 JamVillageQuests::LevelBonus(int32 Level) { return 25 * FMath::Clamp(Level, 0, MaxLevel); }

FJamVillageQuestDef JamVillageQuests::Scale(const FJamVillageQuestDef& Quest, int32 Level)
{
    FJamVillageQuestDef Q = Quest;
    Level = FMath::Clamp(Level, 0, MaxLevel);
    if (Level <= 0 || !Q.Extra.IsEmpty()) { return Q; }   // plans and already-built encores stay as they are
    Q.Count = FMath::Min(Q.Count + 1, 8);
    if (Level >= 2)
    {
        FJamQuestStep Extra = Complement(FirstStep(Q));
        if (Level >= 3)
        {
            // Bestie: one more of everything, and the second step moves to another camp.
            Q.Count = FMath::Min(Q.Count + 1, 8);
            Extra.Count += 1;
            Extra.Destination = OtherCamp(Extra, Extra.Destination);
        }
        Q.Extra.Add(Extra);
    }
    return Q;
}

FJamVillageQuestDef JamVillageQuests::Encore(int32 GiverIndex, int32 Jobs)
{
    if (!Givers().IsValidIndex(GiverIndex)) { return FJamVillageQuestDef(); }
    int32 First = INDEX_NONE, Second = INDEX_NONE;
    for (int32 i = 0; i < All().Num(); ++i)
    {
        if (All()[i].GiverLocation != Givers()[GiverIndex].Location) { continue; }
        if (First == INDEX_NONE) { First = i; } else { Second = i; }
    }
    if (First == INDEX_NONE) { return FJamVillageQuestDef(); }
    const FJamVillageQuestDef& Base = All()[First];
    FJamVillageQuestDef Q = Scale(Base, FMath::Max(2, Jobs));
    // Past Bestie, every encore is one bigger again (capped like everything else).
    const int32 Over = FMath::Max(0, Jobs - MaxLevel);
    Q.Count = FMath::Min(Q.Count + Over, 8);
    for (FJamQuestStep& S : Q.Extra) { S.Count = FMath::Min(S.Count + Over, 8); }
    Q.Id = FName(*FString::Printf(TEXT("%s_encore"), *Base.Id.ToString()));
    Q.Prerequisite = All()[Second != INDEX_NONE ? Second : First].Id;
    Q.Title = TEXT("Encore: ") + Base.Title;
    Q.Request = GEncoreRequests[FMath::Clamp(GiverIndex, 0, int32(UE_ARRAY_COUNT(GEncoreRequests)) - 1)];
    Q.Route = TEXT("encore");
    Q.bEncore = true;
    Q.Reward = FMath::Clamp(Base.Reward + 30 * FMath::Max(2, Jobs), 50, 300);
    Q.Destinations = { Q.Destination };
    Q.MinCount = 1; Q.MaxCount = 8;
    return Q;
}

TArray<FJamQuestStep> JamVillageQuests::ClampPlan(TArray<FJamQuestStep> In, int32 Level)
{
    TArray<FJamQuestStep> Out;
    TArray<FName> Camps;
    for (FJamQuestStep& S : In)
    {
        S.Count = FMath::Clamp(S.Count, 1, 8);
        // Feeding and brushing already start with pacifying: "pacify the rats, then feed them" is one step.
        if (!Out.IsEmpty() && Out.Last().Verb == EJamStageVerb::Pacify && S.Verb == EJamStageVerb::Interact
            && Out.Last().Target == S.Target && Out.Last().Destination == S.Destination)
        {
            S.Count = FMath::Max(S.Count, Out.Last().Count);
            Out.Last() = S;
            continue;
        }
        if (!Camps.Contains(S.Destination))
        {
            if (Camps.Num() >= 2) { continue; }   // at most two camps per plan
            Camps.Add(S.Destination);
        }
        Out.Add(S);
    }
    const int32 MaxSteps = FMath::Clamp(Level, 0, MaxLevel) >= 2 ? 3 : 2;
    if (Out.Num() > MaxSteps) { Out.SetNum(MaxSteps); }
    int32 Total = 0;
    for (int32 i = 0; i < Out.Num(); ++i)
    {
        Out[i].Count = FMath::Min(Out[i].Count, 12 - Total);
        if (Out[i].Count <= 0) { Out.SetNum(i); break; }
        Total += Out[i].Count;
    }
    return Out;
}

bool JamVillageQuests::ReadProposal(const FString& Json, FName QuestId, int32 Level, FJamQuestPlan& Out)
{
    if (Json.Len() > 16384) { return false; }
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid()) { return false; }
    double Version = 0;
    FString Source, Id;
    FJamQuestPlan Plan;
    const TArray<TSharedPtr<FJsonValue>>* Steps = nullptr;
    if (!Root->TryGetNumberField(TEXT("v"), Version) || Version != 1
        || !Root->TryGetStringField(TEXT("source"), Source) || (Source != TEXT("model") && Source != TEXT("cache"))
        || !Root->TryGetStringField(TEXT("quest_id"), Id) || FName(*Id) != QuestId
        || !Root->TryGetStringField(TEXT("label"), Plan.Label) || !Root->TryGetStringField(TEXT("pitch"), Plan.Pitch)
        || !Root->TryGetStringField(TEXT("reaction"), Plan.Reaction)
        || !Root->TryGetArrayField(TEXT("steps"), Steps) || Steps->IsEmpty() || Steps->Num() > 8) { return false; }
    Plan.Label = CleanVillageText(Plan.Label, 40); Plan.Pitch = CleanVillageText(Plan.Pitch, 240); Plan.Reaction = CleanVillageText(Plan.Reaction, 160);
    if (Plan.Label.IsEmpty() || Plan.Pitch.IsEmpty() || Plan.Reaction.IsEmpty()) { return false; }
    TArray<FJamQuestStep> Parsed;
    for (const TSharedPtr<FJsonValue>& Value : *Steps)
    {
        const TSharedPtr<FJsonObject>* Object = nullptr;
        FString Verb, Target, Destination;
        double Count = 0;
        if (!Value.IsValid() || !Value->TryGetObject(Object) || !(*Object)->TryGetStringField(TEXT("verb"), Verb)
            || !(*Object)->TryGetStringField(TEXT("target"), Target) || !(*Object)->TryGetStringField(TEXT("destination"), Destination)
            || !(*Object)->TryGetNumberField(TEXT("count"), Count) || !FMath::IsFinite(Count) || Count < 1 || Count > 1000
            || FMath::FloorToDouble(Count) != Count) { return false; }
        FJamQuestStep Step;
        if (!MakeStep(Verb.ToLower(), Target.ToLower(), int32(Count), FName(*Destination), Step)) { return false; }
        Parsed.Add(MoveTemp(Step));
    }
    Plan.Steps = ClampPlan(MoveTemp(Parsed), Level);
    if (Plan.Steps.IsEmpty()) { return false; }
    Out = MoveTemp(Plan);
    return true;
}

int32 FJamVillageQuestLog::PlanReward(int32 Index, const TArray<FJamQuestStep>& Steps) const
{
    if (!Offers.IsValidIndex(Index)) { return 0; }
    // The client's own formula: the job's reward, scaled by how much work the plan is.
    int32 BaseTotal = 0, Total = 0;
    for (const FJamQuestStep& S : JamVillageQuests::Steps(Offers[Index])) { BaseTotal += S.Count; }
    for (const FJamQuestStep& S : Steps) { Total += S.Count; }
    return FMath::Clamp(FMath::RoundToInt(float(Offers[Index].Reward) * Total / FMath::Max(1, BaseTotal)), 50, 300);
}

int32 FJamVillageQuestLog::StepCount(int32 Index) const
{
    return Offers.IsValidIndex(Index) ? 1 + Offers[Index].Extra.Num() : 0;
}

FJamQuestStep FJamVillageQuestLog::CurrentStep(int32 Index) const
{
    if (!Offers.IsValidIndex(Index)) { return FJamQuestStep(); }
    const TArray<FJamQuestStep> All = JamVillageQuests::Steps(Offers[Index]);
    return All[Index == ActiveIndex ? FMath::Clamp(StepIndex, 0, All.Num() - 1) : 0];
}

int32 FJamVillageQuestLog::Jobs(int32 Giver) const { return GiverJobs.IsValidIndex(Giver) ? GiverJobs[Giver] : 0; }

int32 FJamVillageQuestLog::Level(int32 Giver) const { return FMath::Min(Jobs(Giver), JamVillageQuests::MaxLevel); }

int32 FJamVillageQuestLog::Payout(int32 Index) const
{
    return Offers.IsValidIndex(Index) ? Offers[Index].Reward + JamVillageQuests::LevelBonus(LevelForQuest(Index)) : 0;
}

bool FJamVillageQuestLog::SelectPlan(int32 Index, const FJamQuestPlan& Plan)
{
    if (ActiveIndex != INDEX_NONE || State(Index) != EJamVillageQuestState::Available || Plan.Steps.IsEmpty()) { return false; }
    const TArray<FJamQuestStep> Clamped = JamVillageQuests::ClampPlan(Plan.Steps, LevelForQuest(Index));
    if (Clamped.IsEmpty()) { return false; }
    for (const FJamQuestStep& S : Clamped) { if (!CampAllows(S.Destination, S)) { return false; } }
    FJamVillageQuestDef& Q = Offers[Index];
    Q.Reward = PlanReward(Index, Clamped);
    SetFirst(Q, Clamped[0]);
    Q.Extra.Reset();
    for (int32 i = 1; i < Clamped.Num(); ++i) { Q.Extra.Add(Clamped[i]); }
    Q.Route = TEXT("proposal");
    const FString Pitch = CleanVillageText(Plan.Pitch, 240);
    if (!Pitch.IsEmpty()) { Q.Request = Pitch; }
    return true;
}
