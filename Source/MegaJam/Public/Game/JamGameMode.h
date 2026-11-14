#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Film/JamFilmTypes.h"
#include "UI/JamHudView.h"
#include "UI/JamScreens.h"
#include "Film/JamNagScheduler.h"
#include "Film/JamNoteClamp.h"
#include "World/JamVillageQuests.h"
#include "JamGameMode.generated.h"

class AJamHero;
class AJamNuisance;
class AJamPickup;
class AJamGameState;
class AJamFaerie;

/**
 * The only place that changes shared game state. Drives the FJamShoot held by AJamGameState: feeds it
 * world reports (pacified, treated, collected, crops eaten), drains its events, spawns and resets stage
 * actors, shows cards. Scene n plays on backlot set n; after a wrap the hero walks the street to the next
 * set, and stepping onto it starts the next scene.
 *   -autoplay             a bot plays: chases, lectures, treats, collects, answers notes, walks between sets;
 *                         logs JamAutoplay: PASS scenes=N ... ending=<name> after the last wrap, or FAIL <why>
 *   -autonote=take|refuse how the bot answers CUT! (default take); =suggest gives the first suggested note,
 *                         =typed:<words_with_underscores> types its own (T12b)
 *   -noteurl=<base>, -nonotes   the typed-note server (UJamNoteClient); offline uses keywords, then her own note
 *   -scene=<n>            start at scene n (1-based), hero on that set's mark
 *   -timescale=<x>        world time dilation (0.25 to 8), for faster unattended runs
 */
UCLASS()
class MEGAJAM_API AJamGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AJamGameMode();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Tick(float DeltaSeconds) override;

	/** A hero asks to treat a nuisance. Validates stage, range and state, then treats it. */
	bool HandleInteract(AJamHero* Hero, AJamNuisance* Target);
	/** A hero answers CUT!. False unless the note prompt is up. */
	bool HandleNoteAnswer(AJamHero* Hero, bool bTake);

	// The film around the scenes (T16). Menus send these; only the game mode changes the phase.
	void RequestRollCamera();
	void RequestPremiere();
	void RequestCredits();
	void RequestBackToTitle();

	/** Interact while the wrap panel is up closes it (the last one opens the Premiere). True if it did. */
	bool HandleDismissWrap(AJamHero* Hero);
	bool HandleVillageInteract(AJamHero* Hero);
	/** Same target and range rules used by both input and the HUD prompt. */
	bool VillageInteractPrompt(const AJamHero* Hero, AActor*& Anchor, FString& Text) const;
	const FJamVillageQuestLog& GetVillageQuests() const { return VillageQuests; }
	bool AbandonVillageQuest();
	const FJamQuestOfferView& GetQuestOffer() const { return QuestOffer; }
	void ChooseVillageBranch(int32 Choice);
	void RequestVillageBranches();
	void CancelVillageOffer();
	/** Fixes2 #4: the player's own plan for the offered job (QuestBranches.md, "Proposals"); the menu's text editor sends it. */
	void RequestVillageProposal(const FString& Text);
	/** Three ready-made plans for the offered job, for the gamepad editor's suggestion buttons. */
	TArray<FString> ProposalSuggestions() const;

	/** R4b: a nuisance asks to start a charge (shoot rolling, Dignity left, room among the chargers). */
	bool RequestCharge(AJamNuisance* Nuisance, float HeroDistance);
	/** R4b: a charging nuisance reached the hero. Knockback, stagger, Dignity; at zero the take fails. True if it landed. */
	bool HandleNuisanceHit(AJamNuisance* Nuisance);

	/** The hero cast a skill (cosmetic: Fenella may remark on the Lecture; autoplay counts casts). */
	void NotifySkillCast(AJamHero* Hero, EJamSkill Skill);

	/** While the wrap panel is up, a skill button spends a point on that skill. True if it did. */
	bool HandleSpendPoint(AJamHero* Hero, EJamSkill Skill, bool bFromCharacterSheet = false);
	bool IsNotePromptUp() const;

	/**
	 * The player's own note at CUT! (T12b, BUILD_SPEC 3a). She "reads" it while the server answers (6 s at most);
	 * the clamped form, or the keyword matcher, or her own note replaces the next stage. False if not at CUT!.
	 */
	bool HandleTypedNote(AJamHero* Hero, const FString& Typed);

private:
	friend class FJamVillageFlowTest;
	void BuildVillage();
	void BeginRecruitment();
	void ReturnToVillage();
	void TickVillage(float DeltaSeconds);
	void TickAutoplayVillage(float DeltaSeconds);
	void AutoWalkTo(const FVector2D& Goal, float DeltaSeconds);
	bool bAutoVillage = false;
	void OfferVillageQuest(int32 Index);
	void ResolveVillageBranches(int32 Serial, int32 Index, bool bOk, const FString& Body);
	void ResolveVillageProposal(int32 Serial, int32 Index, bool bOk, const FString& Body);
	int32 ProposalSerial = 0;
	/** Spawn the active job's current step at its camp (Fixes2 #5: jobs have steps). */
	void StartVillageStep();
	int32 SpawnedVillageStep = INDEX_NONE;
	FJamQuestOfferView QuestOffer;
	int32 OfferedQuest = INDEX_NONE;
	int32 BranchRequestSerial = 0;
	void UpdateVillageObjective();
	void AcceptVillageQuest(int32 Index);
	void Direct(const FString& Text);
	int32 VillageTarget(const AJamHero* InHero) const;
	void MarkQuest(const FVector2D& At, AActor* Anchor = nullptr, float Height = 350.f);
	FJamVillageQuestLog VillageQuests;
	bool bRecruitmentPending = false;
	bool bVillageExploration = false;
	UPROPERTY(Transient) TObjectPtr<class AJamQuestMarker> QuestMarker;
	UPROPERTY(Transient) TArray<TObjectPtr<class AJamVillager>> Villagers;
	UPROPERTY(Transient) TArray<TObjectPtr<class AJamQuestMarker>> VillagerMarkers;

	AJamGameState* GetJamGameState() const;
	void StartScene(int32 InSceneIndex);
	void TickShoot(float DeltaSeconds);
	void TickStage(float DeltaSeconds);
	void TickBetweenScenes();
	void HandleShootEvent(const FJamShootEvent& Event);
	void ShowCard(EJamCardKind Kind, const FString& Title, const FString& Subtitle, float Seconds);
	void ClearStageActors();
	void SpawnTargets(const FJamStageDef& Stage, int32 Count, bool bPacified, bool bFromFarEdge = false);
	void PrepareTreatStage(const FJamStageDef& Stage);
	void SpawnPickups(int32 Count);
	void HandlePacified(AActor* Pacified);
	void FinishRun(bool bPass, const FString& Why);
	void TickAutoplay(float DeltaSeconds);
	void TickAutoplayAfterRun(float DeltaSeconds);
	/** The bot's fixed policy: Form A Line, then the Sigh, then Lecture ranks. */
	void AutoSpendPoints();
	void SetPhase(EJamFilmPhase Phase);

	// Fenella (T11): the scheduler picks, the game state shows. Story lines queue; ambient lines only fill silence.
	void Say(EJamLineTrigger Trigger);
	/** Queue one authored line by id ("HeroPickup_03"), at most once per session (spent when shown). Direct() clears the queue: call it after. */
	void SayOnce(FName LineId);
	TSet<FName> SaidOnce;

	// Typed notes (T12b).
	void ResolveTypedNote(int32 Serial, const FString& Typed, bool bOk, const FString& Body);
	void ClearNoteCosmetics();
	bool bTypedNoteActive = false;
	FJamNoteCosmetics NoteCosmetics;
	int32 NoteSerial = 0;
	/** The bot's typed note (-autonote=suggest / typed:...). */
	FString AutoTypedNote;
	bool bAutoSuggest = false;
	/** T16a: the generated premiere, asked for at the last wrap (empty title = the authored ending). */
	FJamPremiereTotals GeneratedPremiere;
	void RequestGeneratedPremiere();
	/** The budget runs out (T21): halfway through the finale's last stage. */
	void TickBudget();
	void BudgetOut();

	/** A bubble from a pool over an actor (T19). */
	void Bubble(EJamLineTrigger Pool, AActor* Anchor, const FVector& Offset, float Seconds, bool bJagged);
	void ShowCaption(const FJamLine& Line);
	void TickFenella(float DeltaSeconds);
	FJamNagSnapshot MakeSnapshot() const;

	/** The wrap (T14): rolls the ledger and loot, equips the hero's item, levels up, fills the panel. */
	void RollWrap(bool bLastScene);
	/** XP and milestones go to the hero's PlayerState. */
	class AJamPlayerState* GetHeroState() const;
	/** Pushes the progression's stats onto the hero (move speed). */
	void ApplyHeroStats();

	/** "Back to one": the hero returns to the set's mark for the next take. */
	void ResetHeroToMark();
	/** Why the take just failed, for the RETAKE! card. */
	FString FailReason(bool bForcedByFlag) const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AJamNuisance>> Nuisances;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AJamPickup>> Pickups;

	TWeakObjectPtr<AJamHero> Hero;
	float GroundZ = 0.f;
	/** BUILD_SPEC 3e: Open mode (-open) plays the open world; Backlot (default, -backlot) is the slice, unchanged. */
	bool bOpenMode = false;
	void StartOpenWorld(AJamHero* NewHero, const TCHAR* Cmd);
	/** Where scene i is shot: the backlot set, or in Open mode the world table's film set (R4). */
	int32 NumSets() const;
	FBox2D SceneFloor(int32 Index) const;
	FVector2D SceneMark(int32 Index) const;
	FString SceneSign(int32 Index) const;
	/** Open mode: the ground height under a point (traced down); GroundZ when nothing is hit. */
	float TraceGround(const FVector2D& At) const;
	/** Open mode: walk to the next film set (hint, compass, biome music). */
	void BeginWalkToSet(int32 Next);
	void TickOpenWalk();
	// The bot's way round things on the terrain: if it stops making progress it side-steps for a moment.
	FVector2D BotLastPos = FVector2D::ZeroVector;
	float BotStuckFor = 0.f;
	float BotSideStepFor = 0.f;
	float BotSideSign = 1.f;

	/** Scene (and backlot set) index being shot; FieldBounds is that set's floor. */
	int32 SceneIndex = 0;
	int32 FirstSceneIndex = 0;
	FBox2D FieldBounds = FBox2D(ForceInit);
	/** After a wrap: the next scene starts when the hero steps onto its set. */
	bool bBetweenScenes = false;
	bool bRunOver = false;

	/** The shoot state last seen, and when it was entered (drives the automatic slate/next/retake). */
	EJamShootState SeenState = EJamShootState::Idle;
	float StateEnteredAt = 0.f;
	/** Crops eaten so far, in fractions (meter stages). */
	float CropsEaten = 0.f;
	/** The finale's last stage sends its bugs in waves from the far edge, so one Last Judgment cannot clear it (T15). */
	int32 WaveLeft = 0;
	float NextWaveAt = 0.f;

	// Run totals, for the PASS line.
	int32 ScenesWrapped = 0;
	int32 TakesTotal = 0;
	int32 NotesTaken = 0;
	int32 NotesRefused = 0;
	int32 FleeCountTotal = 0;
	int32 HitsTaken = 0;
	int32 DignityFails = 0;
	/** The take failed because Dignity ran out (the RETAKE! card says so). */
	bool bDignityBroke = false;

	bool bAutoplay = false;
	bool bAutoNoteTake = true;
	/** -forcefail[=seconds]: the first take of the run fails after this long rolling (T13 check). Negative = off. */
	float ForceFailAfter = -1.f;
	bool bForceFailDone = false;
	// Film totals for the Premiere.
	int32 FilmGross = 0;
	int32 DirectorTotal = 0;
	bool bLastWrapped = false;
	float AfterRunTime = 0.f;
	bool bAutoplayEndLogged = false;
	int32 SkillCasts = 0;
	bool bForceBudgetOut = false;
	float NextHumAt = 0.f;
	bool bFailForcedByFlag = false;
	float AutoplayTime = 0.f;
	float SceneTime = 0.f;

	FJamNagScheduler Nag;
	TArray<FName> CaptionQueue;
	float NagPollIn = 1.f;
	float SinceProgress = 0.f;
	float StrayedFor = 0.f;
	int32 LinesSpoken = 0;
	bool bSaidLevel7Intro = false;

	UPROPERTY(Transient)
	TObjectPtr<AJamFaerie> Faerie;
};
