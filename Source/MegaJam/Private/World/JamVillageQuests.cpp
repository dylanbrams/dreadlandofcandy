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
}

bool FJamVillageQuestLog::ApplyVariation(int32 Index, const FJamQuestVariation& Variation)
{
    if (!Offers.IsValidIndex(Index) || State(Index) == EJamVillageQuestState::Active
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
        Done, Offers.IsValidIndex(Index) ? Offers[Index].Count : 0 };
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
    if (Finished.Contains(Index)) { return EJamVillageQuestState::Complete; }
    if (ActiveIndex == Index) { return Done >= Offers[Index].Count ? EJamVillageQuestState::Ready : EJamVillageQuestState::Active; }
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
    for (int32 i = 0; i < Offers.Num(); ++i) { if (Offers[i].GiverLocation == Location && !Finished.Contains(i)) { return i; } }
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
    ActiveIndex = Index; Done = 0; Emit(EJamQuestEvent::Accepted, Index); return true;
}
bool FJamVillageQuestLog::Report(EJamStageVerb Verb)
{
    if (ActiveIndex == INDEX_NONE || State(ActiveIndex) != EJamVillageQuestState::Active || Offers[ActiveIndex].Verb != Verb) { return false; }
    Done = FMath::Min(Done + 1, Offers[ActiveIndex].Count);
    Emit(Done == Offers[ActiveIndex].Count ? EJamQuestEvent::Ready : EJamQuestEvent::Progress, ActiveIndex);
    return true;
}
bool FJamVillageQuestLog::TurnIn(int32 Index)
{
    if (State(Index) != EJamVillageQuestState::Ready) { return false; }
    Finished.Add(Index); Emit(EJamQuestEvent::TurnedIn, Index);
    ActiveIndex = INDEX_NONE; Done = 0; return true;
}

bool FJamVillageQuestLog::Abandon()
{
    if (ActiveIndex == INDEX_NONE) { return false; }
    const int32 Previous = ActiveIndex;
    ActiveIndex = INDEX_NONE; Done = 0;
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
