#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JamNoteClient.generated.h"

/**
 * The typed-note client (T12b, BUILD_SPEC 3a, Docs/Design/NoteForm.md). One base URL from config
 * ([/Script/MegaJam.JamNoteClient] BaseUrl in DefaultGame.ini) or `-noteurl=<base>`; `-nonotes` turns it off.
 * No keys: the server holds them. Six-second timeout; every failure is silent (the caller falls back).
 * Bodies are handed back as strings: JamNoteClamp parses and clamps them, never this class.
 */
UCLASS(config = Game)
class MEGAJAM_API UJamNoteClient : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** "https://host" (no trailing slash). Empty = offline: the game uses its keyword matcher and authored notes. */
	UPROPERTY(config)
	FString BaseUrl;

	bool IsOnline() const { return !EffectiveUrl.IsEmpty(); }
	const FString& GetSessionId() const { return SessionId; }

	/** Ok is false on timeout, a non-200, or no connection. */
	using FOnBody = TFunction<void(bool bOk, const FString& Body)>;

	/** POST /v1/director/note. */
	void RequestNote(const FString& Scene, const FString& Stage, const TArray<FString>& SetKinds, int32 SetCapacity,
		const FString& Motif, const FString& Note, FOnBody OnDone);

	/** POST /v1/director/premiere. */
	void RequestPremiere(const FString& Motif, const TArray<FString>& Notes, int32 Takes, int32 Taken, int32 Refused,
		bool bDirectorsCut, FOnBody OnDone);

	static constexpr float TimeoutSeconds = 6.f;

private:
	void Post(const FString& Path, const FString& Body, FOnBody OnDone);

	FString EffectiveUrl;
	FString SessionId;
};
