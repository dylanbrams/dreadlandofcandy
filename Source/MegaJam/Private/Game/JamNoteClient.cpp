#include "Game/JamNoteClient.h"
#include "MegaJam.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "World/JamWorld.h"

void UJamNoteClient::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// A random id per run; no player identity (NoteForm.md).
	SessionId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	EffectiveUrl = BaseUrl;
	FString Arg;
	if (FParse::Value(FCommandLine::Get(), TEXT("noteurl="), Arg)) { EffectiveUrl = Arg; }
	if (FParse::Param(FCommandLine::Get(), TEXT("nonotes"))) { EffectiveUrl.Reset(); }
	EffectiveUrl.RemoveFromEnd(TEXT("/"));
	UE_LOG(LogJam, Log, TEXT("JamNoteClient: %s"), EffectiveUrl.IsEmpty() ? TEXT("offline (keywords and authored notes)") : TEXT("online"));
}

namespace
{
	FString ToJson(const TSharedRef<FJsonObject>& Obj)
	{
		FString Out;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
		FJsonSerializer::Serialize(Obj, Writer);
		return Out;
	}

	TArray<TSharedPtr<FJsonValue>> Strings(const TArray<FString>& In)
	{
		TArray<TSharedPtr<FJsonValue>> Out;
		for (const FString& S : In) { Out.Add(MakeShared<FJsonValueString>(S)); }
		return Out;
	}
}

void UJamNoteClient::RequestNote(const FString& Scene, const FString& Stage, const TArray<FString>& SetKinds, int32 SetCapacity,
	const FString& Motif, const FString& Note, FOnBody OnDone)
{
	const TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField(TEXT("v"), 1);
	Obj->SetStringField(TEXT("session_id"), SessionId);
	Obj->SetStringField(TEXT("scene"), Scene);
	Obj->SetStringField(TEXT("stage"), Stage);
	Obj->SetArrayField(TEXT("set_kinds"), Strings(SetKinds));
	Obj->SetNumberField(TEXT("set_capacity"), SetCapacity);
	Obj->SetStringField(TEXT("motif"), Motif);
	Obj->SetStringField(TEXT("note"), Note);
	Post(TEXT("/v1/director/note"), ToJson(Obj), MoveTemp(OnDone));
}

void UJamNoteClient::RequestPremiere(const FString& Motif, const TArray<FString>& Notes, int32 Takes, int32 Taken, int32 Refused,
	bool bDirectorsCut, FOnBody OnDone)
{
	const TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
	Obj->SetNumberField(TEXT("v"), 1);
	Obj->SetStringField(TEXT("session_id"), SessionId);
	Obj->SetStringField(TEXT("motif"), Motif);
	Obj->SetArrayField(TEXT("notes"), Strings(Notes));
	const TSharedRef<FJsonObject> Stats = MakeShared<FJsonObject>();
	Stats->SetNumberField(TEXT("takes"), Takes);
	Stats->SetNumberField(TEXT("notes_taken"), Taken);
	Stats->SetNumberField(TEXT("notes_refused"), Refused);
	Stats->SetNumberField(TEXT("notes_typed"), Notes.Num());
	Obj->SetObjectField(TEXT("stats"), Stats);
	Obj->SetStringField(TEXT("ending"), bDirectorsCut ? TEXT("directors_cut") : TEXT("theatrical"));
	Post(TEXT("/v1/director/premiere"), ToJson(Obj), MoveTemp(OnDone));
}

void UJamNoteClient::Post(const FString& Path, const FString& Body, FOnBody OnDone)
{
	if (!IsOnline()) { OnDone(false, FString()); return; }
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(EffectiveUrl + Path);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(Body);
	Request->SetTimeout(TimeoutSeconds);
	const double Started = FPlatformTime::Seconds();
	Request->OnProcessRequestComplete().BindLambda(
		[OnDone = MoveTemp(OnDone), Path, Started](FHttpRequestPtr, FHttpResponsePtr Response, bool bConnected)
		{
			const int32 Code = Response.IsValid() ? Response->GetResponseCode() : 0;
			const bool bOk = bConnected && Code == 200;
			UE_LOG(LogJam, Log, TEXT("JamNoteClient: %s -> %d in %.2f s"), *Path, Code, FPlatformTime::Seconds() - Started);
			OnDone(bOk, bOk ? Response->GetContentAsString() : FString());
		});
	Request->ProcessRequest();
}

void UJamNoteClient::RequestQuestBranches(const FJamVillageQuestDef& Quest, const TArray<FJamVillageQuestDef>& Routes,
    const TArray<FName>& Completed, const TArray<FString>& History, const FString& Motif, FOnBody OnDone)
{
    const TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
    Obj->SetNumberField(TEXT("v"), 1);
    Obj->SetStringField(TEXT("session_id"), SessionId);
    Obj->SetStringField(TEXT("quest_id"), Quest.Id.ToString());
    Obj->SetStringField(TEXT("giver"), Quest.GiverName);
    Obj->SetStringField(TEXT("title"), Quest.Title);
    Obj->SetStringField(TEXT("pitch"), Quest.Request);
    Obj->SetStringField(TEXT("motif"), Motif);
    TArray<FString> Ids;
    for (FName Id : Completed) { Ids.Add(Id.ToString()); }
    Obj->SetArrayField(TEXT("completed"), Strings(Ids));
    Obj->SetArrayField(TEXT("history"), Strings(History));
    TArray<TSharedPtr<FJsonValue>> RouteValues;
    for (const FJamVillageQuestDef& Route : Routes)
    {
        const TSharedRef<FJsonObject> R = MakeShared<FJsonObject>();
        R->SetStringField(TEXT("route"), Route.Route.ToString());
        R->SetStringField(TEXT("verb"), Route.Verb == EJamStageVerb::Collect ? TEXT("collect") : Route.Verb == EJamStageVerb::Interact ? TEXT("interact") : TEXT("pacify"));
        R->SetStringField(TEXT("target"), Route.Target == EJamTargetKind::Goblin ? TEXT("goblin") : Route.Target == EJamTargetKind::Bug ? TEXT("bug") : Route.Target == EJamTargetKind::Almond ? TEXT("almond") : TEXT("rat"));
        R->SetStringField(TEXT("objective"), Route.Objective);
        R->SetStringField(TEXT("pitch"), Route.Request);
        R->SetNumberField(TEXT("count"), Route.Count);
        R->SetNumberField(TEXT("min_count"), Route.MinCount);
        R->SetNumberField(TEXT("max_count"), Route.MaxCount);
        TArray<FString> Camps;
        for (FName Camp : Route.Destinations) { Camps.Add(Camp.ToString()); }
        R->SetArrayField(TEXT("destinations"), Strings(Camps));
        RouteValues.Add(MakeShared<FJsonValueObject>(R));
    }
    Obj->SetArrayField(TEXT("routes"), RouteValues);
    Post(TEXT("/v1/director/quest/branches"), ToJson(Obj), MoveTemp(OnDone));
}

void UJamNoteClient::RequestQuestProposal(const FJamVillageQuestDef& Quest, int32 Level, const FString& Proposal,
    const TArray<FName>& Completed, const TArray<FString>& History, const FString& Motif, FOnBody OnDone)
{
    const TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
    Obj->SetNumberField(TEXT("v"), 1);
    Obj->SetStringField(TEXT("session_id"), SessionId);
    Obj->SetStringField(TEXT("quest_id"), Quest.Id.ToString());
    Obj->SetStringField(TEXT("giver"), Quest.GiverName);
    Obj->SetStringField(TEXT("title"), Quest.Title);
    Obj->SetStringField(TEXT("pitch"), Quest.Request);
    Obj->SetStringField(TEXT("motif"), Motif);
    Obj->SetNumberField(TEXT("level"), FMath::Clamp(Level, 0, JamVillageQuests::MaxLevel));
    Obj->SetStringField(TEXT("proposal"), Proposal.Left(160));
    TArray<FString> Ids;
    for (FName Id : Completed) { Ids.Add(Id.ToString()); }
    Obj->SetArrayField(TEXT("completed"), Strings(Ids));
    Obj->SetArrayField(TEXT("history"), Strings(History));

    const FJamQuestStep First = JamVillageQuests::FirstStep(Quest);
    const TSharedRef<FJsonObject> Recipe = MakeShared<FJsonObject>();
    Recipe->SetStringField(TEXT("verb"), JamVillageQuests::VerbName(First));
    Recipe->SetStringField(TEXT("target"), JamVillageQuests::TargetName(First));
    Recipe->SetNumberField(TEXT("count"), First.Count);
    TArray<FString> Destinations;
    for (FName Camp : Quest.Destinations) { Destinations.Add(Camp.ToString()); }
    Recipe->SetArrayField(TEXT("destinations"), Strings(Destinations));
    Obj->SetObjectField(TEXT("recipe"), Recipe);

    // The closed sets, sent every time so the server prompt and validator match the client (contract v1).
    const TSharedRef<FJsonObject> Vocabulary = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> StepValues;
    auto AddVerb = [&StepValues](const TCHAR* Verb, const TArray<FString>& Targets)
    {
        const TSharedRef<FJsonObject> V = MakeShared<FJsonObject>();
        V->SetStringField(TEXT("verb"), Verb);
        V->SetArrayField(TEXT("targets"), Strings(Targets));
        StepValues.Add(MakeShared<FJsonValueObject>(V));
    };
    AddVerb(TEXT("pacify"), { TEXT("rat"), TEXT("goblin"), TEXT("bug") });
    AddVerb(TEXT("feed"), { TEXT("rat") });
    AddVerb(TEXT("brush"), { TEXT("goblin") });
    AddVerb(TEXT("gather"), { TEXT("almond"), TEXT("celery"), TEXT("candy_shard") });
    Vocabulary->SetArrayField(TEXT("steps"), StepValues);
    TArray<TSharedPtr<FJsonValue>> CampValues;
    for (const FJamLocation* Camp : JamWorld::OfKind(EJamLocationKind::Camp))
    {
        TArray<FString> Kinds;
        for (EJamTargetKind K : Camp->Kinds)
        {
            switch (K)
            {
            case EJamTargetKind::Rat: Kinds.Add(TEXT("rat")); break;
            case EJamTargetKind::Goblin: Kinds.Add(TEXT("goblin")); break;
            case EJamTargetKind::Bug: Kinds.Add(TEXT("bug")); break;
            case EJamTargetKind::Almond: Kinds.Add(TEXT("almond")); Kinds.Add(TEXT("celery")); Kinds.Add(TEXT("candy_shard")); break;
            }
        }
        const TSharedRef<FJsonObject> C = MakeShared<FJsonObject>();
        C->SetStringField(TEXT("id"), Camp->Id.ToString());
        C->SetArrayField(TEXT("kinds"), Strings(Kinds));
        CampValues.Add(MakeShared<FJsonValueObject>(C));
    }
    Vocabulary->SetArrayField(TEXT("camps"), CampValues);
    const TSharedRef<FJsonObject> Limits = MakeShared<FJsonObject>();
    Limits->SetNumberField(TEXT("max_steps"), 3);
    Limits->SetNumberField(TEXT("max_count"), 8);
    Limits->SetNumberField(TEXT("max_total"), 12);
    Vocabulary->SetObjectField(TEXT("limits"), Limits);
    Obj->SetObjectField(TEXT("vocabulary"), Vocabulary);
    Post(TEXT("/v1/director/quest/propose"), ToJson(Obj), MoveTemp(OnDone));
}
