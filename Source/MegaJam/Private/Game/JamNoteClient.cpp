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
