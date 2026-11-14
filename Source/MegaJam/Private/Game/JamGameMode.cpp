#include "Game/JamGameMode.h"
#include "Game/JamNoteClient.h"
#include "Game/JamLook.h"
#include "Film/JamNoteClamp.h"
#include "Game/JamBacklot.h"
#include "Game/JamOpenWorld.h"
#include "Game/JamOpenTuning.h"
#include "World/JamWorld.h"
#include "Game/JamArtReview.h"
#include "Game/JamArt.h"
#include "Game/JamFaerie.h"
#include "Game/JamQuestMarker.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamHUD.h"
#include "Game/JamNuisance.h"
#include "Game/JamPickup.h"
#include "Game/JamPlayerState.h"
#include "Game/JamSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Film/JamLines.h"
#include "Film/JamShoot.h"
#include "Gas/JamResolveAttributeSet.h"
#include "MegaJam.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/CommandLine.h"
#include "Game/JamSfx.h"
#include "Misc/FileHelper.h"
#include "Sound/SoundBase.h"

namespace JamShootPacing
{
	// How long the game lingers in each shoot state before moving on by itself.
	constexpr float SlateSeconds = 2.0f;
	constexpr float ClearedSeconds = 1.2f;
	constexpr float FailedSeconds = 2.5f;
	constexpr float WrapCardSeconds = 4.0f;
}

namespace JamFenellaTuning
{
	constexpr int32 RunSeed = 2026;
	constexpr float WatchRadius = 1500.f;   // she films the nearest loose nuisance within this of the hero
	float CaptionSeconds(const FString& Text) { return FMath::Clamp(2.f + Text.Len() * 0.055f, 3.f, 7.f); }
	/** Story triggers come first in the enum; they queue instead of being dropped. */
	bool IsStory(EJamLineTrigger T) { return (int32)T < (int32)EJamLineTrigger::Idle; }
}

namespace JamAutoplayTuning
{
	/** Per scene, in game time (walking to it included). */
	constexpr float SceneTimeoutSeconds = 180.f;
	// Close in to this before lecturing (cone is 800). Inside the rats' 500 flee radius on purpose, like a
	// player would: the bot then has to chase and corner them, which exercises Flee and the set edges.
	constexpr float LectureDistance = 450.f;
	constexpr float InteractDistance = 110.f;   // walk this close before treating
	constexpr float NoteThinkSeconds = 1.5f;    // reads the CUT! card before answering
}

namespace JamNoteSuggestions
{
	/** The three buttons per CUT! (BACKEND pre-warms these; the keyword matcher understands each offline too). */
	TArray<FString> For(FName SceneId)
	{
		if (SceneId == FName(TEXT("Scene2"))) { return { TEXT("One enormous goblin"), TEXT("Same scene, but sadder"), TEXT("More. Just... more.") }; }
		if (SceneId == FName(TEXT("Scene3"))) { return { TEXT("Add a ticking clock"), TEXT("Make it rain"), TEXT("Everything is gold now") }; }
		return { TEXT("Bigger"), TEXT("Sadder"), TEXT("Make it rain") };
	}

	FLinearColor TintColour(EJamTint Tint)
	{
		switch (Tint)
		{
		case EJamTint::Pink:   return FLinearColor(1.f, 0.35f, 0.7f);
		case EJamTint::Mint:   return FLinearColor(0.35f, 1.f, 0.7f);
		case EJamTint::Gold:   return FLinearColor(1.f, 0.78f, 0.15f);
		case EJamTint::Grey:   return FLinearColor(0.45f, 0.45f, 0.5f);
		case EJamTint::Purple: return FLinearColor(0.6f, 0.3f, 1.f);
		default:               return FLinearColor::White;
		}
	}

	/** Seconds she "reads" a note when nothing has to be fetched (offline). */
	constexpr float OfflineReadSeconds = 1.2f;
}

namespace JamStageTuning
{
	constexpr float CollectRadius = 120.f;
	/** Meter stages: each loose nuisance eats one crop per this many seconds. */
	constexpr float SecondsPerCropPerBug = 15.f;
	/** The hero steps this far onto the next set before its scene starts. */
	constexpr float SetEntryDepth = 250.f;
	/** The finale's last stage: one more bug over the far wall every this many seconds. */
	constexpr float WaveSeconds = 2.5f;
}

namespace
{
	// BUILD_SPEC section 10.
	struct FKindTuning { float Resolve; float Speed; float FleeRadius; bool bErratic; FLinearColor Colour; const TCHAR* Cast; const TCHAR* Treat; };

	FKindTuning TuningFor(EJamTargetKind Kind)
	{
		switch (Kind)
		{
		case EJamTargetKind::Goblin: return { 150.f, 350.f, 600.f, false, FLinearColor(0.35f, 0.75f, 0.3f), TEXT("Goblins: Themselves, sadly."), TEXT("Brush teeth") };
		case EJamTargetKind::Bug:    return { 60.f, 450.f, 400.f, true, FLinearColor(0.6f, 0.3f, 0.95f), TEXT("Lollypop bugs: Uncredited."), TEXT("") };
		case EJamTargetKind::Almond: return { 1.f, 0.f, 0.f, false, FLinearColor(0.55f, 0.35f, 0.18f), TEXT("Almonds: Nuts."), TEXT("") };
		default:                     return { 100.f, 300.f, 500.f, false, FLinearColor(1.0f, 0.35f, 0.6f), TEXT("Rats: Themselves."), TEXT("Feed celery") };
		}
	}

	/** Spawn spots on a set: rows of three across, starting 700 uu in from the back edge. */
	/** The finale's last stage (taken note or not) sends its bugs in waves. */
	bool IsWaveStage(const FJamShoot& Shoot)
	{
		const TArray<FJamSceneDef> All = JamScenes::All();
		return All.Num() > 0 && Shoot.GetScene().Id == All.Last().Id
			&& Shoot.GetStageIndex() == Shoot.GetScene().Stages.Num() - 1;
	}

	FVector2D SpotFor(const FBox2D& Set, int32 Index)
	{
		const int32 Row = Index / 3;
		const int32 Col = Index % 3;
		const float HalfY = Set.GetExtent().Y;
		const float Y = Set.GetCenter().Y + (Col - 1) * HalfY * 0.5f + (Row % 2) * HalfY * 0.2f;
		const float X = FMath::Min(Set.Min.X + 700.f + Row * 350.f, Set.Max.X - 150.f);
		return FVector2D(X, Y);
	}

}

AJamGameMode::AJamGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AJamHero::StaticClass();
	PlayerStateClass = AJamPlayerState::StaticClass();
	GameStateClass = AJamGameState::StaticClass();
	HUDClass = AJamHUD::StaticClass();
}

AJamGameState* AJamGameMode::GetJamGameState() const
{
	return GetGameState<AJamGameState>();
}

void AJamGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	UAbilitySystemGlobals::Get().InitGlobalData();
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	AJamHero* NewHero = NewPlayer ? Cast<AJamHero>(NewPlayer->GetPawn()) : nullptr;
	if (!NewHero)
	{
		UE_LOG(LogJam, Error, TEXT("JamGameMode: no hero after spawn"));
		return;
	}
	Hero = NewHero;

	const TCHAR* Cmd = FCommandLine::Get();
	bAutoplay = FParse::Param(Cmd, TEXT("autoplay"));
	bAutoVillage = bAutoplay && FParse::Param(Cmd, TEXT("autovillage"));
	bForceBudgetOut = FParse::Param(Cmd, TEXT("budgetout"));   // the beat as soon as the finale's last stage rolls
	if (FParse::Param(Cmd, TEXT("forcefail")))
	{
		ForceFailAfter = 3.f;
		FParse::Value(Cmd, TEXT("forcefail="), ForceFailAfter);
		UE_LOG(LogJam, Log, TEXT("JamGameMode: -forcefail, the first take fails after %.1f s"), ForceFailAfter);
	}
	FString AutoNote;
	if (FParse::Value(Cmd, TEXT("autonote="), AutoNote))
	{
		bAutoNoteTake = !AutoNote.Equals(TEXT("refuse"), ESearchCase::IgnoreCase);
		bAutoSuggest = AutoNote.Equals(TEXT("suggest"), ESearchCase::IgnoreCase);
		if (AutoNote.StartsWith(TEXT("typed:"))) { AutoTypedNote = AutoNote.RightChop(6).Replace(TEXT("_"), TEXT(" ")); }
	}
	int32 SceneArg = 1;
	FParse::Value(Cmd, TEXT("scene="), SceneArg);
	FirstSceneIndex = FMath::Clamp(SceneArg - 1, 0, JamScenes::All().Num() - 1);
	float TimeScale = 1.f;
	if (FParse::Value(Cmd, TEXT("timescale="), TimeScale) && GetWorldSettings())
	{
		GetWorldSettings()->SetTimeDilation(FMath::Clamp(TimeScale, 0.25f, 8.f));
		UE_LOG(LogJam, Log, TEXT("JamGameMode: time scale %.2f"), GetWorldSettings()->TimeDilation);
	}

	// The backlot (T08): built from its table with floor tops just above the template map's floor, so the two
	// never z-fight. The hero starts on the first scene's set mark.
	const float TemplateFloorZ = NewHero->GetActorLocation().Z - NewHero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GroundZ = TemplateFloorZ + 2.f;
	bOpenMode = !FParse::Param(Cmd, TEXT("backlot"));   // Dylan: Open is now the default; -backlot keeps the fallback.
	if (bOpenMode)
	{
		StartOpenWorld(NewHero, Cmd);
		return;
	}
	if (AJamBacklot* Backlot = GetWorld()->SpawnActor<AJamBacklot>(AJamBacklot::StaticClass(), FTransform::Identity))
	{
		Backlot->Build(GroundZ);
	}
	JamLook::Apply(GetWorld());
	const FVector2D Mark = JamBacklot::HeroMark(FirstSceneIndex);
	NewHero->TeleportTo(FVector(Mark, GroundZ + NewHero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.f), FRotator::ZeroRotator);

	Nag.Reset(JamFenellaTuning::RunSeed);
	FActorSpawnParameters FaerieParams;
	FaerieParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Faerie = GetWorld()->SpawnActor<AJamFaerie>(AJamFaerie::StaticClass(), NewHero->GetActorTransform(), FaerieParams);
	if (Faerie) { Faerie->SetFollow(NewHero); }

	// -artreview (T20): every generated mesh in a lineup for checking, no scene.
	FString ReviewArg;
	if (FParse::Param(Cmd, TEXT("artreview")) || FParse::Value(Cmd, TEXT("artreview="), ReviewArg))
	{
		if (AJamArtReview* Review = GetWorld()->SpawnActor<AJamArtReview>(AJamArtReview::StaticClass(), FTransform::Identity))
		{
			Review->Build(FVector(JamBacklot::HeroMark(FirstSceneIndex) + FVector2D(700.f, 0.f), GroundZ));
		}
		NewHero->SetCameraDistance(ReviewArg.Equals(TEXT("close")) ? 2300.f : 4600.f);   // close: the first row, the cast
		SetPhase(EJamFilmPhase::Shooting);
		UE_LOG(LogJam, Log, TEXT("JamGameMode: -artreview lineup"));
		return;
	}

	// Straight to the shoot when a flag asks for a scene or a screen sample; otherwise the title (T16).
	FString Dummy;
	const bool bSkipTitle = FParse::Value(Cmd, TEXT("scene="), Dummy) || FParse::Value(Cmd, TEXT("uiscreen="), Dummy) || FParse::Param(Cmd, TEXT("notitle"));
	if (bSkipTitle) { RequestRollCamera(); }
	else { SetPhase(EJamFilmPhase::Title); }
}

void AJamGameMode::StartScene(int32 InSceneIndex)
{
	AJamGameState* GS = GetJamGameState();
	const TArray<FJamSceneDef> Scenes = JamScenes::All();
	if (!GS || !Scenes.IsValidIndex(InSceneIndex) || InSceneIndex >= NumSets()) { return; }

	SceneIndex = InSceneIndex;
	FieldBounds = SceneFloor(SceneIndex);
	if (bOpenMode)
	{
		GroundZ = TraceGround(FieldBounds.GetCenter());
		GS->bCompass = false;
		GS->WalkMusicCue = NAME_None;
		JamSfx::Play(this, TEXT("Quest_Accept"));
		// The camera stands on the path from the village, looking in: the open side of every set.
		const FVector2D Inward = FieldBounds.GetCenter() - SceneMark(SceneIndex);
		if (Hero.IsValid()) { Hero->SetFollowFraming(true, FMath::RadiansToDegrees(FMath::Atan2(Inward.Y, Inward.X))); }
	}
	bBetweenScenes = false;
	SceneTime = 0.f;
	ClearStageActors();
	ClearNoteCosmetics();
	GS->SceneNumber = SceneIndex + 1;
	GS->Hint.Reset();
	GS->Shoot.Start(Scenes[SceneIndex]);
	SeenState = EJamShootState::Idle;   // TickShoot notices the Slate and shows its card
	UE_LOG(LogJam, Log, TEXT("JamGameMode: scene %d %s \"%s\" started"), SceneIndex + 1, *Scenes[SceneIndex].Id.ToString(), *Scenes[SceneIndex].Title);
}

void AJamGameMode::ShowCard(EJamCardKind Kind, const FString& Title, const FString& Subtitle, float Seconds)
{
	if (AJamGameState* GS = GetJamGameState())
	{
		GS->CardKind = Kind;
		GS->CardTitle = Title;
		GS->CardSubtitle = Subtitle;
		GS->CardUntil = Seconds < 0.f ? -1.f : GetWorld()->GetTimeSeconds() + Seconds;
	}
}

void AJamGameMode::ClearStageActors()
{
	for (AJamNuisance* N : Nuisances)
	{
		if (N) { FleeCountTotal += N->GetFleeCount(); N->Destroy(); }
	}
	Nuisances.Reset();
	for (AJamPickup* P : Pickups)
	{
		if (P) { P->Destroy(); }
	}
	Pickups.Reset();
	CropsEaten = 0.f;
	WaveLeft = 0;
}

void AJamGameMode::SpawnTargets(const FJamStageDef& Stage, int32 Count, bool bPacified, bool bFromFarEdge)
{
	const FKindTuning Tuning = TuningFor(Stage.Target);
	const int32 Already = Nuisances.Num();
	for (int32 i = 0; i < Count; ++i)
	{
		// Waves come in over the far wall (up the screen), spread across it.
		const FVector2D Spot = bFromFarEdge
			? FVector2D(FieldBounds.Max.X - 150.f, FMath::Lerp(FieldBounds.Min.Y + 200.f, FieldBounds.Max.Y - 200.f, FMath::FRand()))
			: SpotFor(FieldBounds, Already + i);
		const FVector Loc(Spot, (bOpenMode ? TraceGround(Spot) : GroundZ) + 70.f);
		AJamNuisance* N = GetWorld()->SpawnActorDeferred<AJamNuisance>(AJamNuisance::StaticClass(), FTransform(Loc),
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!N) { continue; }
		N->SetBounds = FieldBounds;
		if (bOpenMode) { N->GroundOffset = 70.f; }
		N->Speed = Tuning.Speed * Stage.SpeedMultiplier;
		// Boss-sized note targets (Resolve x3 and up) do not flee (BUILD_SPEC 10).
		N->FleeRadius = Stage.ResolveMultiplier >= 3.f ? 0.f : Tuning.FleeRadius;
		N->MaxResolveValue = Tuning.Resolve * Stage.ResolveMultiplier;
		N->BaseColour = Tuning.Colour;
		N->bErratic = Tuning.bErratic;
		N->bStartPacified = bPacified;
		N->Attack = JamAttack::For(Stage.Target, Stage.ResolveMultiplier >= 3.f);
		if (bTypedNoteActive && Stage.Id == FName(TEXT("TypedNote")) && NoteCosmetics.Tint != EJamTint::None)
		{
			N->BaseColour = JamNoteSuggestions::TintColour(NoteCosmetics.Tint);
			N->NoteTintOpacity = 0.55f;
		}
		switch (Stage.Target)
		{
		case EJamTargetKind::Goblin: N->ArtMesh = TEXT("SM_Goblin"); break;
		case EJamTargetKind::Bug:    N->ArtMesh = TEXT("SM_LollyBug"); break;
		default:                     N->ArtMesh = TEXT("SM_Rat"); break;
		}
		N->ArtHeight = JamArt::Height(N->ArtMesh);
		N->FinishSpawning(FTransform(Loc));
		if (bTypedNoteActive && Stage.Id == FName(TEXT("TypedNote")))
		{
			if (!FMath::IsNearlyEqual(NoteCosmetics.Scale, 1.f)) { N->SetActorScale3D(FVector(NoteCosmetics.Scale)); }
		}
		else if (Stage.ResolveMultiplier >= 3.f) { N->SetActorScale3D(FVector(1.8f)); }
		N->AddActorWorldOffset(FVector(0.f, 0.f, 70.f * (N->GetActorScale3D().Z - 1.f)));
		N->GetResolveSet()->OnPacified.AddUObject(this, &AJamGameMode::HandlePacified);
		Nuisances.Add(N);
	}
}

void AJamGameMode::PrepareTreatStage(const FJamStageDef& Stage)
{
	// A note can swap in a different target (the very large goblin): fresh, un-pacified, pacify then treat.
	if (!FMath::IsNearlyEqual(Stage.ResolveMultiplier, 1.f))
	{
		ClearStageActors();
		SpawnTargets(Stage, Stage.Count, false);
		return;
	}
	// Otherwise treat the ones the previous stage pacified; a retake un-treats them.
	int32 Ready = 0;
	for (AJamNuisance* N : Nuisances)
	{
		if (!N) { continue; }
		N->ResetToPacified();
		if (N->IsPacified()) { ++Ready; }
	}
	if (Ready < Stage.Count) { SpawnTargets(Stage, Stage.Count - Ready, true); }
}

void AJamGameMode::SpawnPickups(int32 Count)
{
	for (AJamPickup* P : Pickups) { if (P) { P->Destroy(); } }
	Pickups.Reset();
	for (int32 i = 0; i < Count; ++i)
	{
		// Pickups sit between the nuisance rows, so they don't land under a sitting bug.
		const FVector2D Spot = SpotFor(FieldBounds, i + 1) + FVector2D(175.f, 0.f);
		if (AJamPickup* P = GetWorld()->SpawnActor<AJamPickup>(AJamPickup::StaticClass(), FTransform(FVector(Spot, (bOpenMode ? TraceGround(Spot) : GroundZ) + 25.f))))
		{
			Pickups.Add(P);
		}
	}
}

void AJamGameMode::HandlePacified(AActor* Pacified)
{
	AJamGameState* GS = GetJamGameState();
	JamSfx::Play(this, TEXT("Pacify"));
	const FJamStageDef* Stage = GS ? GS->Shoot.GetStage() : nullptr;
	if (bVillageExploration)
	{
		const bool bOwned = Nuisances.Contains(Cast<AJamNuisance>(Pacified));
		if (bOwned && VillageQuests.Active() != INDEX_NONE && VillageQuests.State(VillageQuests.Active()) == EJamVillageQuestState::Active)
		{
			if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Pacify); }
			VillageQuests.Report(EJamStageVerb::Pacify);
			UpdateVillageObjective();
			if (VillageQuests.State(VillageQuests.Active()) == EJamVillageQuestState::Ready)
			{
				Direct(TEXT("Objectives complete! Return to ") + VillageQuests.Offer(VillageQuests.Active()).GiverName + TEXT(" to collect your reward."));
				SayOnce(SaidOnce.Contains(TEXT("HeroPickup_01")) ? TEXT("HeroPickup_02") : TEXT("HeroPickup_01"));
			}
		}
		return;
	}
	if (GS && GS->Phase == EJamFilmPhase::Shooting && !bBetweenScenes && Stage && GS->Shoot.GetState() == EJamShootState::Rolling)
	{
		if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Pacify); }
	}
	if (Stage && Stage->Verb == EJamStageVerb::Pacify)
	{
		GS->Shoot.ReportProgress(1);
		Say(EJamLineTrigger::Pacified);
		// Sometimes the pacified one answers back.
		if (FMath::FRand() < 0.8f) { Bubble(EJamLineTrigger::NuisanceReply, Pacified, FVector(0.f, 0.f, 130.f), 1.8f, false); }
	}
	UE_LOG(LogJam, Log, TEXT("JamGameMode: pacified %s"), Pacified ? *Pacified->GetName() : TEXT("(null)"));
}

bool AJamGameMode::HandleInteract(AJamHero* InHero, AJamNuisance* Target)
{
	AJamGameState* GS = GetJamGameState();
	const int32 Side = VillageQuests.Active();
	if (bVillageExploration && Side != INDEX_NONE && VillageQuests.State(Side) == EJamVillageQuestState::Active
		&& VillageQuests.CurrentStep(Side).Verb == EJamStageVerb::Interact)
	{
		if (!InHero || InHero != Hero.Get() || !Target || !Nuisances.Contains(Target) || !Target->IsPacified()) { return false; }
		if (FVector::Dist(InHero->GetActorLocation(), Target->GetActorLocation()) > 250.f + Target->ExtraReach() || !Target->Treat()) { return false; }
		const EJamTargetKind Treated = VillageQuests.CurrentStep(Side).Target;
		VillageQuests.Report(EJamStageVerb::Interact);
		if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Treat); }
		UpdateVillageObjective();
		if (VillageQuests.State(Side) == EJamVillageQuestState::Ready)
		{
			Direct(TEXT("Everyone treated! Return to the quest giver and collect your payment."));
			SayOnce(Treated == EJamTargetKind::Goblin ? TEXT("HeroPickup_04") : TEXT("HeroPickup_05"));
		}
		return true;
	}
	if (!GS || !GS->CanInteract()) { return false; }
	if (!InHero || !Target || !Target->IsPacified()) { return false; }
	// The hero picked the target; check range again here, as a server would.
	if (FVector::Dist2D(InHero->GetActorLocation(), Target->GetActorLocation()) > 250.f + Target->ExtraReach()) { return false; }
	if (!Target->Treat()) { return false; }
	UE_LOG(LogJam, Log, TEXT("JamGameMode: Treated %s"), *Target->GetName());
	GS->Shoot.ReportProgress(1);
	if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Treat); }
	Say(EJamLineTrigger::Interact);
	return true;
}

bool AJamGameMode::HandleNoteAnswer(AJamHero* InHero, bool bTake)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || !InHero || !GS->IsNotePrompt() || GS->bReadingNote) { return false; }
	return GS->Shoot.AnswerNote(bTake);
}

void AJamGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SceneTime += DeltaSeconds;
	if (bRecruitmentPending || bVillageExploration) { TickVillage(DeltaSeconds); }
	else
	{
		if (bBetweenScenes) { TickBetweenScenes(); }
		else { TickShoot(DeltaSeconds); }
		TickBudget();
		if (bOpenMode) { TickVillage(DeltaSeconds); }
	}
	TickFenella(DeltaSeconds);
	if (bAutoplay && Hero.IsValid())
	{
		if (!bRunOver) { TickAutoplay(DeltaSeconds); }
		else { TickAutoplayAfterRun(DeltaSeconds); }
	}
}

void AJamGameMode::TickBetweenScenes()
{
	const int32 Next = SceneIndex + 1;
	const AJamHero* H = Hero.Get();
	if (!H || Next < 0 || Next >= NumSets()) { return; }
	if (GetJamGameState() && GetJamGameState()->IsWrapVisible()) { return; }
	const FBox2D Set = SceneFloor(Next);
	const FVector2D Here(H->GetActorLocation());
	if (bOpenMode)
	{
		TickOpenWalk();
		// Open: any step onto the floor, a little way in from its edge.
		if (Set.ExpandBy(-JamStageTuning::SetEntryDepth).IsInside(Here))
		{
			UE_LOG(LogJam, Log, TEXT("JamGameMode: hero reached film set %s"), *SceneSign(Next));
			StartScene(Next);
		}
		return;
	}
	if (Set.IsInside(Here) && Here.X > Set.Min.X + JamStageTuning::SetEntryDepth)
	{
		UE_LOG(LogJam, Log, TEXT("JamGameMode: hero reached set %s"), *JamBacklot::Sets()[Next].Id.ToString());
		StartScene(Next);
	}
}

void AJamGameMode::TickShoot(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	FJamShoot& Shoot = GS->Shoot;
	Shoot.Tick(DeltaSeconds);
	GS->Dignity.Tick(DeltaSeconds);
	if (Shoot.GetState() == EJamShootState::Rolling) { GS->FilmSeconds += DeltaSeconds; }
	{
		// R4b: the Dignity meter shows while something on the set can still hit back.
		bool bThreat = false;
		for (const AJamNuisance* N : Nuisances) { bThreat |= N && N->Attack.bAttacks && !N->IsPacified() && !N->IsTreated(); }
		const FJamStageDef* Stage = Shoot.GetStage();
		bThreat |= Stage && Stage->Verb == EJamStageVerb::Pacify && JamAttack::For(Stage->Target, false).bAttacks;
		const EJamShootState St = Shoot.GetState();
		GS->bDignityVisible = bThreat && (St == EJamShootState::Rolling || St == EJamShootState::FailedTake);
	}
	TickStage(DeltaSeconds);

	const float Now = GetWorld()->GetTimeSeconds();
	if (Shoot.GetState() != SeenState)
	{
		SeenState = Shoot.GetState();
		StateEnteredAt = Now;
		if (SeenState == EJamShootState::Slate)
		{
			JamSfx::Play(this, TEXT("Clapper"));
			const FJamStageDef* Stage = Shoot.GetStage();
			ShowCard(EJamCardKind::Slate, FString::Printf(TEXT("SCENE %d - TAKE %d"), SceneIndex + 1, Shoot.GetTake()),
				FString::Printf(TEXT("%s\nDirector: F. Flicker. Camera: F. Flicker. %s"), *Shoot.GetScene().Title,
					TuningFor(Stage ? Stage->Target : EJamTargetKind::Rat).Cast),
				JamShootPacing::SlateSeconds);
			// Take 1: she introduces the scene (and, once, the hero). Take 2 already got the failed-take line.
			if (Shoot.GetTake() == 1)
			{
				Say(EJamLineTrigger::SceneStart);
				if (!bSaidLevel7Intro) { bSaidLevel7Intro = true; Say(EJamLineTrigger::Level7Intro); }
			}
			else if (Shoot.GetTake() >= 3)
			{
				Say(JamLines::TriggerForTake(Shoot.GetTake()));
			}
		}
		else if (SeenState == EJamShootState::NotePrompt)
		{
			const FJamLine* Pitch = JamLines::Find(Shoot.GetScene().Note.PitchLineId);
			const FString PitchText = Pitch ? Pitch->Text : FString(TEXT("I have a note."));
			ShowCard(EJamCardKind::Cut, TEXT("CUT!"), PitchText, -1.f);
			JamSfx::Play(this, TEXT("Cut_Horn"));
			GS->SuggestedNotes = JamNoteSuggestions::For(Shoot.GetScene().Id);
			UE_LOG(LogJam, Log, TEXT("JamShoot: CUT! note %s pitched"), *Shoot.GetScene().Note.Id.ToString());
			if (!bOpenMode && SceneIndex == 2)
			{
				if (AJamPlayerState* PS = GetHeroState())
				{
					const int32 Gained = PS->EditProgression().ReachMilestone(EJamMilestone::Scene3FinalNote);
					UE_LOG(LogJam, Log, TEXT("JamGameMode: level %d (+%d) at the final CUT!"), PS->GetProgression().GetLevel(), Gained);
				}
			}
		}
	}

	// Automatic pacing: the slate rolls, cleared stages move on, failed takes go again.
	const float InState = Now - StateEnteredAt;
	switch (Shoot.GetState())
	{
	case EJamShootState::Slate:        if (InState >= JamShootPacing::SlateSeconds)   { Shoot.Action(); } break;
	case EJamShootState::StageCleared: if (InState >= JamShootPacing::ClearedSeconds) { Shoot.Action(); } break;
	case EJamShootState::FailedTake:
		if (InState >= JamShootPacing::FailedSeconds && Shoot.Retake()) { ResetHeroToMark(); }
		break;
	case EJamShootState::Rolling:
		if (ForceFailAfter >= 0.f && !bForceFailDone && InState >= ForceFailAfter)
		{
			bForceFailDone = true;
			bFailForcedByFlag = true;
			UE_LOG(LogJam, Log, TEXT("JamGameMode: -forcefail fails take %d"), Shoot.GetTake());
			Shoot.ReportFail();
		}
		break;
	default: break;
	}

	TArray<FJamShootEvent> Events;
	Shoot.DrainEvents(Events);
	for (const FJamShootEvent& Event : Events)
	{
		HandleShootEvent(Event);
	}
}

void AJamGameMode::TickStage(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	FJamShoot& Shoot = GS->Shoot;
	const FJamStageDef* Stage = Shoot.GetStage();
	if (!Stage || Shoot.GetState() != EJamShootState::Rolling) { return; }

	// Collect: walking over a pickup gathers it.
	if (Stage->Verb == EJamStageVerb::Collect && Hero.IsValid())
	{
		for (int32 i = Pickups.Num() - 1; i >= 0; --i)
		{
			AJamPickup* P = Pickups[i];
			if (P && FVector::Dist2D(P->GetActorLocation(), Hero->GetActorLocation()) < JamStageTuning::CollectRadius)
			{
				P->Destroy();
				Pickups.RemoveAt(i);
				UE_LOG(LogJam, Log, TEXT("JamGameMode: collected a pickup"));
				if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Collect); }
				Shoot.ReportProgress(1);
			}
		}
	}

	// Waves: the rest of the finale's bugs arrive one by one.
	if (WaveLeft > 0 && GetWorld()->GetTimeSeconds() >= NextWaveAt)
	{
		SpawnTargets(*Stage, 1, false, true);
		--WaveLeft;
		NextWaveAt = GetWorld()->GetTimeSeconds() + JamStageTuning::WaveSeconds;
	}

	// Meter: loose nuisances eat the crops.
	if (Stage->FailMeter > 0)
	{
		int32 Loose = 0;
		for (const AJamNuisance* N : Nuisances) { Loose += (N && !N->IsPacified() && !N->IsTreated()) ? 1 : 0; }
		CropsEaten += DeltaSeconds * Loose / JamStageTuning::SecondsPerCropPerBug;
		while (CropsEaten >= 1.f && Shoot.GetState() == EJamShootState::Rolling)
		{
			CropsEaten -= 1.f;
			Shoot.ReportMeter(1);
		}
	}
}

void AJamGameMode::HandleShootEvent(const FJamShootEvent& Event)
{
	AJamGameState* GS = GetJamGameState();
	const FJamShoot& Shoot = GS->Shoot;
	const FJamStageDef* Stage = Shoot.GetScene().Stages.IsValidIndex(Event.Stage) ? &Shoot.GetScene().Stages[Event.Stage] : nullptr;

	switch (Event.Type)
	{
	case EJamShootEventType::StageStarted:
		UE_LOG(LogJam, Log, TEXT("JamShoot: scene %d stage %d (%s) take %d rolling"), SceneIndex + 1, Event.Stage, Stage ? *Stage->Id.ToString() : TEXT("?"), Event.Take);
		CropsEaten = 0.f;
		GS->Dignity.Reset();   // every stage and every retake starts with his Dignity whole
		bDignityBroke = false;
		if (Stage)
		{
			switch (Stage->Verb)
			{
			case EJamStageVerb::Pacify:
				ClearStageActors();
				if (IsWaveStage(Shoot))
				{
					// Two at once, the rest in waves over the far wall.
					const int32 First = FMath::Min(2, Stage->Count);
					SpawnTargets(*Stage, First, false);
					WaveLeft = Stage->Count - First;
					NextWaveAt = GetWorld()->GetTimeSeconds() + JamStageTuning::WaveSeconds;
				}
				else
				{
					SpawnTargets(*Stage, Stage->Count, false);
				}
				// "This time the rats know their lines": no fleeing until the forced retake.
				if (!Shoot.TargetsFlee()) { for (AJamNuisance* N : Nuisances) { if (N) { N->FleeRadius = 0.f; } } }
				break;
			case EJamStageVerb::Interact:
				PrepareTreatStage(*Stage);
				break;
			case EJamStageVerb::Collect:
				SpawnPickups(Stage->Count);
				break;
			}
		}
		ShowCard(EJamCardKind::Action, TEXT("ACTION!"), FString(), 1.2f);   // Fenella's caption carries the line
		JamSfx::Play(this, TEXT("Clapper"));
		if (bOpenMode && Stage) { Direct(TEXT("Hero, ") + Stage->Objective + TEXT(". Follow the objective above; I will film you.")); }
		break;

	case EJamShootEventType::Progress:
		SinceProgress = 0.f;
		break;

	case EJamShootEventType::StageCleared:
		UE_LOG(LogJam, Log, TEXT("JamShoot: stage %d cleared on take %d"), Event.Stage, Event.Take);
		JamSfx::Play(this, TEXT("Stage_Complete"));
		break;

	case EJamShootEventType::NoteAnswered:
		UE_LOG(LogJam, Log, TEXT("JamShoot: note %s"), Event.Value ? TEXT("taken") : TEXT("refused"));
		ShowCard(EJamCardKind::Info, Event.Value ? TEXT("NOTE TAKEN") : TEXT("NOTE REFUSED"), FString(), 2.f);
		(Event.Value ? GS->RunNotesTaken : GS->RunNotesRefused) += 1;
		if (!(Event.Value && bTypedNoteActive)) { Say(Event.Value ? EJamLineTrigger::NoteTaken : EJamLineTrigger::NoteRefused); }
		break;

	case EJamShootEventType::TakeFailed:
		UE_LOG(LogJam, Log, TEXT("JamShoot: take %d failed (%s)"), Event.Take, Event.Value ? TEXT("forced") : TEXT("failure"));
		if (Event.Value) { ShowCard(EJamCardKind::Retake, TEXT("Perfect! Again."), FString(), JamShootPacing::FailedSeconds); }
		else { ShowCard(EJamCardKind::Retake, TEXT("RETAKE!"), FailReason(bFailForcedByFlag), JamShootPacing::FailedSeconds); }
		bFailForcedByFlag = false;
		bDignityBroke = false;
		JamSfx::Play(this, TEXT("Retake_Rewind"));
		GS->TakeFailedAt = GetWorld()->GetTimeSeconds();
		Say(Event.Value ? EJamLineTrigger::ForcedRetake : EJamLineTrigger::FailedTake);
		break;

	case EJamShootEventType::Wrapped:
	{
		UE_LOG(LogJam, Log, TEXT("JamShoot: scene %d wrapped after %d takes"), SceneIndex + 1, Shoot.GetTakesUsed());
		++ScenesWrapped;
		TakesTotal += Shoot.GetTakesUsed();
		NotesTaken += Shoot.GetNotesTaken();
		NotesRefused += Shoot.GetNotesRefused();
		const bool bLast = !JamScenes::All().IsValidIndex(SceneIndex + 1) || SceneIndex + 1 >= NumSets();
		ShowCard(EJamCardKind::None, FString(), FString(), 0.f);   // the wrap panel says THAT'S A WRAP
		RollWrap(bLast);
		if (bLast) { RequestGeneratedPremiere(); }
		Say(EJamLineTrigger::Wrap);
		if (bLast)
		{
			if (bOpenMode)
			{
				VillageQuests.FinishMain();
				GS->bVillageReturnAvailable = true;
				if (QuestMarker) { QuestMarker->SetActorHiddenInGame(true); }
			}
			FinishRun(true, FString());
		}
		else
		{
			bBetweenScenes = true;
			SceneTime = 0.f;
			if (bOpenMode) { BeginWalkToSet(SceneIndex + 1); }
			else { GS->Hint = FString::Printf(TEXT("Walk up the street to %s"), *JamBacklot::Sets()[SceneIndex + 1].Sign); }
		}
		break;
	}

	default:
		break;
	}
}

void AJamGameMode::FinishRun(bool bPass, const FString& Why)
{
	if (bRunOver) { return; }
	bRunOver = true;
	for (const AJamNuisance* N : Nuisances) { FleeCountTotal += N ? N->GetFleeCount() : 0; }
	if (Hero.IsValid()) { Hero->SetAimOverride(FVector::ZeroVector); }
	if (!bAutoplay) { return; }
	if (bPass)
	{
		// BUILD_SPEC 2: mostly taken = the Director's Cut, mostly refused = the Theatrical Release.
		const TCHAR* Ending = NotesTaken >= NotesRefused ? TEXT("directors_cut") : TEXT("theatrical");
		if (bOpenMode) { UE_LOG(LogJam, Log, TEXT("JamAutoplay: PASS mode=open main=%d side=0"), ScenesWrapped); }
		UE_LOG(LogJam, Log, TEXT("JamAutoplay: PASS scenes=%d first=%d takes=%d notes=%d/%d ending=%s flees=%d lines=%d skills=%d hits=%d dignityfails=%d time=%.1f"),
			ScenesWrapped, FirstSceneIndex + 1, TakesTotal, NotesTaken, NotesRefused, Ending, FleeCountTotal, LinesSpoken, SkillCasts, HitsTaken, DignityFails, AutoplayTime);
	}
	else
	{
		UE_LOG(LogJam, Log, TEXT("JamAutoplay: FAIL %s"), *Why);
	}
}

void AJamGameMode::TickAutoplay(float DeltaSeconds)
{
	AJamHero* Bot = Hero.Get();
	AutoplayTime += DeltaSeconds;
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	// The title: the bot rolls camera once the rating card has gone.
	if (GS->Phase == EJamFilmPhase::Title)
	{
		if (GetWorld()->GetTimeSeconds() - GS->PhaseSince > 3.f)
		{
			UE_LOG(LogJam, Log, TEXT("JamAutoplay: title, Roll Camera"));
			RequestRollCamera();
		}
		return;
	}
	const FJamShoot& Shoot = GS->GetShoot();

	if (bRecruitmentPending && Faerie)
	{
		const FVector To = Faerie->GetActorLocation() - Bot->GetActorLocation();
		if (SceneTime > JamAutoplayTuning::SceneTimeoutSeconds) { FinishRun(false, FString::Printf(TEXT("recruitment timeout hero=%s fenella=%s"), *Bot->GetActorLocation().ToString(), *Faerie->GetActorLocation().ToString())); return; }
		if (To.Size2D() < 240.f) { Bot->RequestInteract(); }
		else { AutoWalkTo(FVector2D(Faerie->GetActorLocation()), DeltaSeconds); }
		return;
	}

	if (SceneTime > JamAutoplayTuning::SceneTimeoutSeconds)
	{
		FinishRun(false, FString::Printf(TEXT("timeout scene=%d between=%d stage=%d take=%d progress=%d"),
			SceneIndex + 1, bBetweenScenes ? 1 : 0, Shoot.GetStageIndex(), Shoot.GetTake(), Shoot.GetProgress()));
		return;
	}

	if (bBetweenScenes && GS->IsWrapVisible()) { Bot->RequestInteract(); }

	// Between scenes: walk up the street onto the next set. Line up with the street (Y = 0) first, so the bot
	// never pushes into the wall beside an opening.
	if (bBetweenScenes && bOpenMode)
	{
		// Open: head for the middle of the next set; if the terrain stops it, side-step for a moment.
		AutoSpendPoints();
		if (SceneIndex + 1 >= NumSets()) { return; }
		AutoWalkTo(SceneFloor(SceneIndex + 1).GetCenter(), DeltaSeconds);
		return;
	}
	if (bBetweenScenes)
	{
		AutoSpendPoints();
		Bot->SetAimOverride(FVector::ZeroVector);
		const FVector2D Goal = JamBacklot::HeroMark(SceneIndex + 1) + FVector2D(JamStageTuning::SetEntryDepth, 0.f);
		const FVector2D Here(Bot->GetActorLocation());
		FVector2D Dir = Goal - Here;
		if (FMath::Abs(Here.Y) > 60.f && Here.X < Goal.X - 600.f) { Dir = FVector2D(0.3f, -Here.Y); }
		Bot->AddMovementInput(FVector(Dir.GetSafeNormal(), 0.f));
		return;
	}

	if (Shoot.GetState() == EJamShootState::NotePrompt)
	{
		if (GetWorld()->GetTimeSeconds() - StateEnteredAt >= JamAutoplayTuning::NoteThinkSeconds)
		{
			if (!AutoTypedNote.IsEmpty() || bAutoSuggest)
			{
				if (!GS->bReadingNote)
				{
					HandleTypedNote(Bot, bAutoSuggest && GS->SuggestedNotes.Num() > 0 ? GS->SuggestedNotes[0] : AutoTypedNote);
				}
			}
			else if (bAutoNoteTake) { Bot->RequestInteract(); }
			else { Bot->RequestRefuseNote(); }
		}
		return;
	}

	const FJamStageDef* Stage = Shoot.GetStage();
	if (!Stage || Shoot.GetState() != EJamShootState::Rolling) { return; }

	// Collect: walk to the nearest pickup.
	if (Stage->Verb == EJamStageVerb::Collect)
	{
		const AJamPickup* Best = nullptr;
		float BestD = TNumericLimits<float>::Max();
		for (const AJamPickup* P : Pickups)
		{
			const float D = P ? FVector::Dist2D(P->GetActorLocation(), Bot->GetActorLocation()) : BestD;
			if (D < BestD) { BestD = D; Best = P; }
		}
		if (Best)
		{
			const FVector To = (Best->GetActorLocation() - Bot->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
			Bot->SetAimOverride(To);
			Bot->AddMovementInput(To.GetSafeNormal());
		}
		return;
	}

	// Pacify: the nearest loose target. Interact: lecture any loose target first (a note's boss), then treat.
	bool bTreating = false;
	AJamNuisance* Target = nullptr;
	float Best = TNumericLimits<float>::Max();
	for (int32 Pass = 0; Pass < 2 && !Target; ++Pass)
	{
		bTreating = Pass == 1;
		if (bTreating && Stage->Verb != EJamStageVerb::Interact) { break; }
		for (AJamNuisance* N : Nuisances)
		{
			if (!N || N->IsTreated() || N->IsPacified() != bTreating) { continue; }
			const float D = FVector::Dist2D(N->GetActorLocation(), Bot->GetActorLocation());
			if (D < Best) { Best = D; Target = N; }
		}
	}
	if (!Target) { return; }

	const FVector ToTarget = (Target->GetActorLocation() - Bot->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
	Bot->SetAimOverride(ToTarget);
	const float Reach = bTreating ? JamAutoplayTuning::InteractDistance + Target->ExtraReach()
		: JamAutoplayTuning::LectureDistance;
	if (ToTarget.Size() > Reach) { Bot->AddMovementInput(ToTarget.GetSafeNormal()); }
	else if (bTreating) { Bot->RequestInteract(); }
	else
	{
		// Bigger skills first when they are unlocked and cool; the cooldowns refuse the spam.
		bool bCast = false;
		for (EJamSkill Skill : { EJamSkill::LastJudgment, EJamSkill::FormALine })
		{
			const AJamPlayerState* PS = GetHeroState();
			if (!bCast && PS && PS->GetProgression().IsUnlocked(Skill)) { bCast = Bot->RequestSkill(Skill); }
		}
		if (!bCast) { Bot->RequestLecture(); }
	}
}

// ---- Fenella (T11) -------------------------------------------------------------------------------------------

void AJamGameMode::NotifySkillCast(AJamHero* InHero, EJamSkill Skill)
{
	if (!InHero || InHero != Hero.Get()) { return; }
	// The sound of each skill (R1): the lecture winds up (raise, inhale) and lands; the others land.
	// Fixes3 #9/#12: the mage, archer and barbarian have their own sounds; only the knight raises a sword.
	const int32 Cast = InHero->GetCastIndex();
	auto PlayEach = [this](std::initializer_list<TPair<const TCHAR*, float>> Cues) { for (const auto& C : Cues) { JamSfx::Play(this, C.Key, C.Value); } };
	if (Cast == 1)
	{
		if (Skill == EJamSkill::Lecture) { PlayEach({ { TEXT("Hero_Inhale"), 0.f }, { TEXT("Sugar_Volley"), 0.15f }, { TEXT("Sugar_Impact"), 0.55f } }); }
		else if (Skill == EJamSkill::FormALine) { PlayEach({ { TEXT("Soap_Finish"), 0.1f }, { TEXT("Sugar_Impact"), 0.6f } }); }
		else if (Skill == EJamSkill::LastJudgment) { PlayEach({ { TEXT("Peppermint_Fall"), 0.f }, { TEXT("Peppermint_Impact"), 0.7f }, { TEXT("Peppermint_Impact"), 1.0f } }); }
		else if (Skill == EJamSkill::DisappointedSigh) { JamSfx::Play(this, TEXT("DisappointedSigh")); }
	}
	else if (Cast == 2)
	{
		if (Skill == EJamSkill::Lecture) { PlayEach({ { TEXT("Celery_Launch"), 0.05f }, { TEXT("Celery_Impact"), 0.45f } }); }
		else if (Skill == EJamSkill::FormALine) { PlayEach({ { TEXT("Floss_Launch"), 0.f }, { TEXT("Floss_Snare"), 0.35f }, { TEXT("Floss_Retract"), 0.9f } }); }
		else if (Skill == EJamSkill::LastJudgment) { PlayEach({ { TEXT("Celery_Launch"), 0.f }, { TEXT("Celery_Launch"), 0.15f }, { TEXT("Celery_Launch"), 0.3f }, { TEXT("Celery_Impact"), 0.6f }, { TEXT("Celery_Impact"), 0.8f } }); }
		else if (Skill == EJamSkill::DisappointedSigh) { JamSfx::Play(this, TEXT("DisappointedSigh")); }
	}
	else if (Cast == 3)
	{
		if (Skill == EJamSkill::Lecture) { PlayEach({ { TEXT("Hero_Inhale"), 0.f }, { TEXT("Knead_Impact"), 0.3f }, { TEXT("Knead_Impact"), 0.6f } }); }
		else if (Skill == EJamSkill::FormALine) { PlayEach({ { TEXT("Encouragement_Blast"), 0.1f } }); }
		else if (Skill == EJamSkill::LastJudgment) { PlayEach({ { TEXT("RollingPin_Stray"), 0.f }, { TEXT("RollingPin_Stray"), 0.5f }, { TEXT("RollingPin_Finale"), 1.2f } }); }
		else if (Skill == EJamSkill::DisappointedSigh) { JamSfx::Play(this, TEXT("DisappointedSigh")); }
	}
	else
	switch (Skill)
	{
	case EJamSkill::Lecture:
		JamSfx::Play(this, TEXT("Sword_Raise"));
		JamSfx::Play(this, TEXT("Hero_Inhale"), 0.2f);
		JamSfx::Play(this, TEXT("Lecture_Impact"), 0.5f);
		break;
	case EJamSkill::FormALine:        JamSfx::Play(this, TEXT("FormLine_Contact"), 0.15f); break;
	case EJamSkill::DisappointedSigh: JamSfx::Play(this, TEXT("DisappointedSigh")); break;
	case EJamSkill::LastJudgment:     JamSfx::Play(this, TEXT("Manners_Dome_Open")); JamSfx::Play(this, TEXT("LastJudgment_Contact"), 0.2f); JamSfx::Play(this, TEXT("Manners_Dome_Rebuke"), 0.6f); break;
	default: break;
	}
	if (Skill == EJamSkill::Lecture)
	{
		Say(EJamLineTrigger::LectureCast);
		// The lecture itself: one jagged line over the knight, sadder once she sulks (BUILD_SPEC 6, T19).
		const AJamGameState* GS = GetJamGameState();
		const bool bSad = GS && GS->GetShoot().IsSulking();
		const EJamLineTrigger Pool = GS && GS->bBudgetOut ? EJamLineTrigger::BubbleBudget : (bSad ? EJamLineTrigger::BubbleSadder : EJamLineTrigger::BubbleDefault);
		Bubble(Pool, InHero, FVector(0.f, 0.f, 190.f), 1.4f, true);
	}
	else
	{
		++SkillCasts;
		// Fixes3 #25: every attack, every class, gets its jagged shout.
		const AJamGameState* GS = GetJamGameState();
		Bubble(GS && GS->bBudgetOut ? EJamLineTrigger::BubbleBudget : EJamLineTrigger::BubbleDefault, InHero, FVector(0.f, 0.f, 190.f), 1.4f, true);
	}
}

bool AJamGameMode::IsNotePromptUp() const
{
	const AJamGameState* GS = GetJamGameState();
	return GS && GS->IsNotePrompt();
}

bool AJamGameMode::HandleSpendPoint(AJamHero* InHero, EJamSkill Skill, bool bFromCharacterSheet)
{
	AJamGameState* GS = GetJamGameState();
	AJamPlayerState* PS = GetHeroState();
	if (!GS || !PS || !InHero || InHero != Hero.Get()) { return false; }
    const APlayerController* PC = Cast<APlayerController>(InHero->GetController());
    const AJamHUD* HUD = PC ? Cast<AJamHUD>(PC->GetHUD()) : nullptr;
    const bool bSheet = bFromCharacterSheet && HUD && HUD->IsCharacterSheetOpen() && GS->Phase == EJamFilmPhase::Shooting;
    if (!GS->IsWrapVisible() && !bSheet) { return false; }
	FJamProgression& Prog = PS->EditProgression();
	if (!Prog.SpendPoint(Skill)) { return false; }
	ApplyHeroStats();
	UE_LOG(LogJam, Log, TEXT("JamGameMode: point spent on %s, rank %d, %d left"), JamProgression::SkillName(Skill), Prog.GetRank(Skill), Prog.GetPoints());
	return true;
}

void AJamGameMode::AutoSpendPoints()
{
	const AJamGameState* GS = GetJamGameState();
	AJamPlayerState* PS = GetHeroState();
	if (!GS || !PS || !Hero.IsValid() || !GS->IsWrapVisible() || PS->GetProgression().GetPoints() <= 0) { return; }
	for (EJamSkill Skill : { EJamSkill::FormALine, EJamSkill::Lecture })
	{
		// Through the same validation the button uses, but never falling back to a cast.
		if ((!PS->GetProgression().IsUnlocked(Skill) || Skill == EJamSkill::Lecture) && HandleSpendPoint(Hero.Get(), Skill)) { return; }
	}
}

FJamNagSnapshot AJamGameMode::MakeSnapshot() const
{
	FJamNagSnapshot Snap;
	Snap.Now = GetWorld()->GetTimeSeconds();
	Snap.SecondsSinceProgress = SinceProgress;
	Snap.SecondsStrayed = StrayedFor;
	if (const AJamGameState* GS = GetJamGameState())
	{
		Snap.bSilenced = GS->IsNotePrompt();
		Snap.bSulking = GS->GetShoot().IsSulking();
		const UJamSettings* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamSettings>() : nullptr;
		Snap.bCommentary = (Settings && Settings->bCommentary) || FParse::Param(FCommandLine::Get(), TEXT("commentary"));
	}
	return Snap;
}

void AJamGameMode::Say(EJamLineTrigger Trigger)
{
	const FJamLine* Line = Nag.Request(Trigger, MakeSnapshot());
	if (!Line) { return; }
	const AJamGameState* GS = GetJamGameState();
	const bool bBusy = !CaptionQueue.IsEmpty() || (GS && GS->IsCaptionVisible());
	if (JamFenellaTuning::IsStory(Trigger)) { CaptionQueue.Add(Line->Id); }
	else if (!bBusy) { ShowCaption(*Line); }
}

void AJamGameMode::SayOnce(FName LineId)
{
	// Spent when shown (ShowCaption), not when queued: a Direct() that clears the queue leaves the line for next time.
	if (SaidOnce.Contains(LineId) || CaptionQueue.Contains(LineId) || !JamLines::Find(LineId)) { return; }
	CaptionQueue.Add(LineId);
}

void AJamGameMode::ShowCaption(const FJamLine& Line)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	const bool bLong = Line.Trigger == EJamLineTrigger::RealityIntro
		|| (Line.Trigger >= EJamLineTrigger::RealityPickup && Line.Trigger <= EJamLineTrigger::ReturnPickup);
	float Seconds = bLong ? FMath::Clamp(2.f + Line.Text.Len() * 0.055f, 5.f, 12.f) : JamFenellaTuning::CaptionSeconds(Line.Text);
	// Keep the caption up for the whole recording (the HUD plays /Game/Audio/VO/VO_<LineId> while it shows).
	const FString Clip = FString::Printf(TEXT("VO_%s"), *Line.Id.ToString());
	if (const USoundBase* Voice = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Audio/VO/%s.%s"), *Clip, *Clip), nullptr, LOAD_NoWarn | LOAD_Quiet))
	{
		const float Duration = Voice->GetDuration();
		if (Duration > 0.f && Duration < 30.f) { Seconds = FMath::Max(Seconds, Duration + 0.4f); }
	}
	SaidOnce.Add(Line.Id);
	GS->CaptionLineId = Line.Id;
	GS->CaptionText = Line.Text;
	GS->CaptionUntil = GetWorld()->GetTimeSeconds() + Seconds;
	if (Faerie) { Faerie->Speak(Seconds); }
	++LinesSpoken;
	UE_LOG(LogJam, Log, TEXT("Fenella: %s \"%s\""), *Line.Id.ToString(), *Line.Text);
}

void AJamGameMode::TickFenella(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	const bool bRolling = !bBetweenScenes && GS->GetShoot().GetState() == EJamShootState::Rolling;

	// Idle and strayed clocks only run while a take rolls.
	SinceProgress = bRolling ? SinceProgress + DeltaSeconds : 0.f;
	const bool bOffSet = Hero.IsValid() && !FieldBounds.IsInside(FVector2D(Hero->GetActorLocation()));
	StrayedFor = (bRolling && bOffSet) ? StrayedFor + DeltaSeconds : 0.f;

	// Story lines wait their turn; ambient lines come from the poll when she has nothing else to say.
	if (!GS->IsCaptionVisible())
	{
		if (!CaptionQueue.IsEmpty())
		{
			const FName Id = CaptionQueue[0];
			CaptionQueue.RemoveAt(0);
			if (const FJamLine* Line = JamLines::Find(Id)) { ShowCaption(*Line); }
		}
		else if (!GS->CaptionLineId.IsNone())
		{
			GS->CaptionLineId = NAME_None;
			GS->CaptionText.Reset();
		}
	}
	NagPollIn -= DeltaSeconds;
	if (NagPollIn <= 0.f)
	{
		NagPollIn = 1.f;
		if (bRolling && CaptionQueue.IsEmpty() && !GS->IsCaptionVisible())
		{
			if (const FJamLine* Line = Nag.Poll(MakeSnapshot())) { ShowCaption(*Line); }
		}
	}

	// She films the nearest loose nuisance, or else the hero.
	if (Faerie && Hero.IsValid())
	{
		AActor* Best = nullptr;
		float BestDist = JamFenellaTuning::WatchRadius;
		for (AJamNuisance* N : Nuisances)
		{
			if (!N || N->IsPacified() || N->IsTreated()) { continue; }
			const float D = FVector::Dist2D(N->GetActorLocation(), Hero->GetActorLocation());
			if (D < BestDist) { BestDist = D; Best = N; }
		}
		Faerie->SetSubject(Best);
		Faerie->SetSulking(!bVillageExploration && GS->GetShoot().IsSulking());
	}
}

// ---- They hit back (R4b) ----------------------------------------------------------------------------------------

bool AJamGameMode::RequestCharge(AJamNuisance* Nuisance, float HeroDistance)
{
	const AJamGameState* GS = GetJamGameState();
	if (!Nuisance || !GS || !Hero.IsValid() || bRunOver) { return false; }
	if (GS->GetShoot().GetState() != EJamShootState::Rolling || !GS->GetDignity().CanBeHit()) { return false; }
	int32 Charging = 0;
	for (const AJamNuisance* N : Nuisances) { Charging += (N && N->IsCharging()) ? 1 : 0; }
	return JamAttack::MayCharge(Nuisance->Attack, !Nuisance->IsPacified() && !Nuisance->IsTreated(), 0.f, HeroDistance, Charging);
}

bool AJamGameMode::HandleNuisanceHit(AJamNuisance* Nuisance)
{
	AJamGameState* GS = GetJamGameState();
	AJamHero* H = Hero.Get();
	if (!Nuisance || !GS || !H || GS->Shoot.GetState() != EJamShootState::Rolling || !GS->Dignity.CanBeHit()) { return false; }
	const FJamAttackTuning& A = Nuisance->Attack;
	const bool bBroke = GS->Dignity.Hit(A.Damage, A.StaggerSeconds);
	H->TakeHit(Nuisance->GetActorLocation(), A.Knockback, A.StaggerSeconds);
	++HitsTaken;
	JamSfx::Play(this, TEXT("Hero_Hit"));
	UE_LOG(LogJam, Log, TEXT("JamGameMode: %s hit the hero, dignity %.0f"), *Nuisance->GetName(), GS->Dignity.GetValue());
	if (bBroke)
	{
		++DignityFails;
		bDignityBroke = true;
		UE_LOG(LogJam, Log, TEXT("JamGameMode: no dignity left, take %d fails"), GS->Shoot.GetTake());
		GS->Shoot.ReportFail();
	}
	return true;
}

// ---- Failure (T13) -------------------------------------------------------------------------------------------

FString AJamGameMode::FailReason(bool bForcedByFlag) const
{
	const AJamGameState* GS = GetJamGameState();
	const FJamStageDef* Stage = GS ? GS->GetShoot().GetStage() : nullptr;
	if (bForcedByFlag || !Stage) { return TEXT("Director's call. Back to one!"); }
	if (bDignityBroke) { return TEXT("No dignity left. Back to one!"); }
	if (Stage->FailMeter > 0 && GS->GetShoot().GetMeter() >= Stage->FailMeter) { return TEXT("The crops are gone. Back to one!"); }
	if (Stage->TimeLimit > 0.f && GS->GetShoot().GetTimeLeft() <= 0.f) { return TEXT("Out of time. Back to one!"); }
	return TEXT("From the top. Back to one!");
}

void AJamGameMode::ResetHeroToMark()
{
	AJamHero* H = Hero.Get();
	if (!H || SceneIndex < 0 || SceneIndex >= NumSets()) { return; }
	const FVector2D Mark = SceneMark(SceneIndex);
	H->TeleportTo(FVector(Mark, (bOpenMode ? TraceGround(Mark) : GroundZ) + H->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.f), FRotator::ZeroRotator);
	H->GetCharacterMovement()->StopMovementImmediately();
	UE_LOG(LogJam, Log, TEXT("JamGameMode: back to one (take %d)"), GetJamGameState()->GetShoot().GetTake());
}

// ---- Wrap, ledger, loot, progression (T14) ---------------------------------------------------------------------

namespace JamWrapTuning
{
	constexpr float PanelSeconds = 14.f;     // closes itself; interact closes it sooner
	constexpr float RefusedGross = 0.8f;     // each refused note in the scene (BUILD_SPEC 3: refusing lowers the gross)
}

AJamPlayerState* AJamGameMode::GetHeroState() const
{
	return Hero.IsValid() ? Hero->GetPlayerState<AJamPlayerState>() : nullptr;
}

void AJamGameMode::ApplyHeroStats()
{
	AJamPlayerState* PS = GetHeroState();
	if (!PS || !Hero.IsValid()) { return; }
	Hero->ApplyStats(PS->GetProgression().GetStats());
}

void AJamGameMode::RollWrap(bool bLastScene)
{
	AJamGameState* GS = GetJamGameState();
	AJamPlayerState* PS = GetHeroState();
	if (!GS || !PS) { return; }
	const FJamShoot& Shoot = GS->Shoot;
	FJamProgression& Prog = PS->EditProgression();

	const float GrossMultiplier = Shoot.GetGrossMultiplier() * FMath::Pow(JamWrapTuning::RefusedGross, Shoot.GetNotesRefused());
	const int32 Seed = SceneIndex * 100 + (Shoot.GetTakesUsed() - 1);
	const FJamWrapOutcome Out = JamWrap::Roll(SceneIndex, GrossMultiplier, Seed);

	const FJamStatBlock Before = Prog.GetStats();
	if (Out.LootIndices.IsValidIndex(Out.EquippedSlot)) { Prog.AddLoot(Out.LootIndices[Out.EquippedSlot]); }
	int32 Gained = bOpenMode ? Prog.CompleteQuest(SceneIndex) : 0;
	if (!bOpenMode && SceneIndex == 0) { Gained = Prog.ReachMilestone(EJamMilestone::Scene1Wrap); }
	else if (!bOpenMode && SceneIndex == 1) { Gained = Prog.ReachMilestone(EJamMilestone::Scene2Wrap); }
	PS->Sprinkles += Out.Ledger.HeroCut;
	ApplyHeroStats();

	GS->Wrap = JamWrap::MakeView(Out, GS->SceneTitleOverride.IsEmpty() ? Shoot.GetScene().Title : GS->SceneTitleOverride,
		Before, Prog.GetStats(), Gained, Seed);
	// The note named the loot: the hero's piece takes the name (the stats stay the table's).
	if (bTypedNoteActive && !NoteCosmetics.LootName.IsEmpty())
	{
		for (FJamWrapLoot& L : GS->Wrap.Loot)
		{
			if (L.bEquipped) { L.Name = NoteCosmetics.LootName; if (!NoteCosmetics.LootFlavour.IsEmpty()) { L.Flavour = NoteCosmetics.LootFlavour; } }
		}
	}
	FilmGross += Out.Ledger.Gross;
	DirectorTotal += Out.Ledger.DeductionsTotal;
	bLastWrapped = bLastScene;
	GS->WrapUntil = bLastScene ? -1.f : GetWorld()->GetTimeSeconds() + JamWrapTuning::PanelSeconds;
	// The ledger reads out: one deduction per line, then what is left trickles in.
	const int32 Deductions = GS->Wrap.Lines.Num();
	for (int32 i = 0; i < Deductions; ++i) { JamSfx::Play(this, TEXT("Ledger_Deduction"), 0.5f + 0.3f * i); }
	JamSfx::Play(this, TEXT("Coin_Trickle"), 0.7f + 0.3f * Deductions);

	const FJamLootDef& Equipped = JamProgression::LootTable()[Out.LootIndices[Out.EquippedSlot]];
	UE_LOG(LogJam, Log, TEXT("JamWrap: scene %d gross %d cut %d (sprinkles %d), equipped %s (%s), level %d, radius %.0f -> %.0f"),
		SceneIndex + 1, Out.Ledger.Gross, Out.Ledger.HeroCut, PS->Sprinkles, *Equipped.Name, *JamWrap::StatText(Equipped),
		Prog.GetLevel(), Before.Radius, Prog.GetStats().Radius);
	if (bOpenMode) { UE_LOG(LogJam, Log, TEXT("JamXP: quest=%d total=%d level=%d progress=%.3f"), SceneIndex + 1, Prog.GetXp(), Prog.GetLevel(), Prog.GetLevelProgress()); }
}

bool AJamGameMode::HandleDismissWrap(AJamHero* InHero)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || !InHero || !GS->IsWrapVisible()) { return false; }
	if (GS->WrapUntil < 0.f)
	{
		if (bOpenMode) { ReturnToVillage(); }
		else { RequestPremiere(); }
		return true;
	}   // the last wrap opens the Premiere
	GS->WrapUntil = GetWorld()->GetTimeSeconds();
	return true;
}

// ---- The film around the scenes (T16) -------------------------------------------------------------------------

void AJamGameMode::SetPhase(EJamFilmPhase Phase)
{
	if (AJamGameState* GS = GetJamGameState())
	{
		GS->Phase = Phase;
		GS->PhaseSince = GetWorld()->GetTimeSeconds();
	}
}

void AJamGameMode::RequestRollCamera()
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || (GS->Phase != EJamFilmPhase::Title && GS->GetShoot().GetState() != EJamShootState::Idle)) { return; }
	SetPhase(EJamFilmPhase::Shooting);
	UE_LOG(LogJam, Log, TEXT("JamGameMode: roll camera"));
	if (bOpenMode)
	{
		if (!VillageQuests.IsRecruited()) { BeginRecruitment(); }
		else { BeginWalkToSet(FirstSceneIndex); }
		return;
	}   // R4: the first scene starts at its film set
	StartScene(FirstSceneIndex);
}

void AJamGameMode::RequestPremiere()
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || GS->Phase != EJamFilmPhase::Shooting || !bLastWrapped) { return; }
	CancelVillageOffer();
	const AJamPlayerState* PS = GetHeroState();
	GS->Premiere.bDirectorsCut = JamScreens::IsDirectorsCut(GS->RunNotesTaken, GS->RunNotesRefused);
	GS->Premiere.FilmGross = FilmGross;
	GS->Premiere.Director = DirectorTotal;
	GS->Premiere.Hero = PS ? PS->Sprinkles : 0;
	GS->Premiere.Title = GeneratedPremiere.Title;
	GS->Premiere.Tagline = GeneratedPremiere.Tagline;
	GS->Premiere.Reviews = GeneratedPremiere.Reviews;
	GS->Wrap.bVisible = false;
	SetPhase(EJamFilmPhase::Premiere);
	UE_LOG(LogJam, Log, TEXT("JamGameMode: premiere, %s, gross %d, director %d, hero %d"),
		GS->Premiere.bDirectorsCut ? TEXT("THE DIRECTOR'S CUT") : TEXT("THE THEATRICAL RELEASE"), FilmGross, DirectorTotal, GS->Premiere.Hero);
}

void AJamGameMode::RequestCredits()
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || GS->Phase != EJamFilmPhase::Premiere) { return; }
	SetPhase(EJamFilmPhase::Credits);
	UE_LOG(LogJam, Log, TEXT("JamGameMode: credits"));
}

void AJamGameMode::RequestBackToTitle()
{
	if (bOpenMode && bVillageExploration && GetJamGameState() && GetJamGameState()->Phase == EJamFilmPhase::Credits && !bAutoplay)
	{
		ReturnToVillage();
		return;
	}
	UE_LOG(LogJam, Log, TEXT("JamGameMode: back to the title"));
	if (bAutoplay)
	{
		// Reloading would start the autoplay over; the run is a loop test, so it stops here.
		UE_LOG(LogJam, Log, TEXT("JamAutoplay: title to title (no reload under autoplay)"));
		return;
	}
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void AJamGameMode::TickAutoplayAfterRun(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || bAutoplayEndLogged) { return; }
	AfterRunTime += DeltaSeconds;
	AutoSpendPoints();
	if (bAutoVillage && bOpenMode && bLastWrapped) { TickAutoplayVillage(DeltaSeconds); return; }
	const float InPhase = GetWorld()->GetTimeSeconds() - GS->PhaseSince;
	if (GS->Phase == EJamFilmPhase::Shooting && bLastWrapped && AfterRunTime > 3.f) { RequestPremiere(); }
	else if (GS->Phase == EJamFilmPhase::Premiere && InPhase > 4.f) { RequestCredits(); }
	else if (GS->Phase == EJamFilmPhase::Credits && InPhase > 4.f)
	{
		bAutoplayEndLogged = true;
		RequestBackToTitle();
		if (FParse::Param(FCommandLine::Get(), TEXT("quitafterrun"))) { FPlatformMisc::RequestExit(false, TEXT("Main autoplay complete")); }
	}
}

// ---- Bubbles (T19) ----------------------------------------------------------------------------------------------

void AJamGameMode::Bubble(EJamLineTrigger Pool, AActor* Anchor, const FVector& Offset, float Seconds, bool bJagged)
{
	AJamGameState* GS = GetJamGameState();
	FJamLine Typed;
	const FJamLine* Line = Nag.PickFromPool(Pool);
	// A typed note rewrote what the knight shouts and what they answer, until the money runs out.
	if (GS && bTypedNoteActive && !GS->bBudgetOut)
	{
		const TArray<FString>* Own = Pool == EJamLineTrigger::NuisanceReply ? &NoteCosmetics.Replies
			: (Pool == EJamLineTrigger::BubbleDefault || Pool == EJamLineTrigger::BubbleSadder) ? &NoteCosmetics.LectureBubbles : nullptr;
		if (Own && Own->Num() > 0)
		{
			Typed.Id = TEXT("TypedNote");
			Typed.Text = (*Own)[FMath::RandRange(0, Own->Num() - 1)];
			Line = &Typed;
		}
	}
	if (!GS || !Line || !Anchor) { return; }
	const float Now = GetWorld()->GetTimeSeconds();
	GS->Bubbles.RemoveAll([Now](const AJamGameState::FWorldBubble& B) { return B.Until <= Now || !B.Anchor.IsValid(); });
	// One bubble per speaker at a time: a new line replaces the old one.
	GS->Bubbles.RemoveAll([Anchor](const AJamGameState::FWorldBubble& B) { return B.Anchor.Get() == Anchor; });
	AJamGameState::FWorldBubble B;
	B.LineId = Line->Id;
	B.Text = Line->Text;
	B.Anchor = Anchor;
	B.Offset = Offset;
	B.Until = Now + Seconds;
	B.bJagged = bJagged;
	GS->Bubbles.Add(B);
	JamSfx::Play(this, TEXT("Bubble_Pop"));
}

// ---- The budget runs out (T21) --------------------------------------------------------------------------------

void AJamGameMode::TickBudget()
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || bBetweenScenes) { return; }
	const FJamShoot& Shoot = GS->GetShoot();
	const FJamStageDef* Stage = Shoot.GetStage();
	const bool bFinale = !JamScenes::All().IsValidIndex(SceneIndex + 1);
	const bool bLastStage = Stage && Shoot.GetStageIndex() == Shoot.GetScene().Stages.Num() - 1;
	if (!GS->bBudgetOut && bFinale && Stage && Shoot.GetState() == EJamShootState::Rolling)
	{
		// As the finale's last stage rolls, right after its CUT! (BUILD_SPEC 3 says halfway, but the Last Judgment
		// clears that stage in one frame, and nobody should miss this gag). -budgetout: as soon as the finale rolls.
		if (bForceBudgetOut || bLastStage) { BudgetOut(); }
	}
	// She hums the score in place of the music, now and then.
	if (GS->bBudgetOut && GS->Phase == EJamFilmPhase::Shooting && GetWorld()->GetTimeSeconds() >= NextHumAt && !GS->IsCaptionVisible() && CaptionQueue.IsEmpty())
	{
		NextHumAt = GetWorld()->GetTimeSeconds() + 9.f;
		GS->CaptionLineId = TEXT("Hum");
		GS->CaptionText = TEXT("\u266A Hmm hm hmmm, hm HMMM \u266A   (Fenella hums the score)");
		GS->CaptionUntil = GetWorld()->GetTimeSeconds() + 3.5f;
	}
}

void AJamGameMode::BudgetOut()
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || GS->bBudgetOut) { return; }
	GS->bBudgetOut = true;
	UE_LOG(LogJam, Log, TEXT("JamGameMode: the budget runs out"));
	JamSfx::Play(this, TEXT("Budget_Warning"));
	JamSfx::Play(this, TEXT("Budget_Empty"), 1.4f);
	Say(EJamLineTrigger::BudgetOut);
	NextHumAt = GetWorld()->GetTimeSeconds() + 6.f;
	// Open keeps its authored terrain and gear: replacing every material looked like a rendering failure.
	// Preserve the abandoned Backlot path; the cardboard extras still sell the gag in Open.
	if (!bOpenMode)
	{
		JamLook::BudgetOut(GetWorld());

		// Every mesh in the world drops to the engine's grey checker, the one every unfinished game has.
		UMaterialInterface* Grid = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial"));
		if (Grid)
		{
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				TInlineComponentArray<UStaticMeshComponent*> Meshes(*It);
				for (UStaticMeshComponent* Mesh : Meshes)
				{
					for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i) { Mesh->SetMaterial(i, Grid); }
				}
			}
		}
	}
	// The cast become boxes with their parts written on them.
	const FJamStageDef* Stage = GS->GetShoot().GetStage();
	const TCHAR* Label = TEXT("RAT");
	if (Stage)
	{
		switch (Stage->Target)
		{
		case EJamTargetKind::Goblin: Label = TEXT("GOBLIN"); break;
		case EJamTargetKind::Bug:    Label = TEXT("BUG"); break;
		default: break;
		}
	}
	for (AJamNuisance* N : Nuisances) { if (N) { N->BecomeBox(Label); } }
}

// ---- Typed notes (T12b) ---------------------------------------------------------------------------------------

namespace
{
	const TCHAR* NoteKindName(EJamTargetKind Kind)
	{
		switch (Kind)
		{
		case EJamTargetKind::Rat:    return TEXT("rat");
		case EJamTargetKind::Goblin: return TEXT("goblin");
		case EJamTargetKind::Bug:    return TEXT("bug");
		case EJamTargetKind::Almond: return TEXT("almond");
		default:                     return TEXT("rat");
		}
	}
}

bool AJamGameMode::HandleTypedNote(AJamHero* InHero, const FString& InTyped)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || !InHero || !GS->IsNotePrompt() || GS->bReadingNote) { return false; }
	const FString Typed = JamNoteClamp::SanitizeNote(InTyped);
	if (Typed.IsEmpty()) { return false; }

	GS->bReadingNote = true;
	GS->ReadingNote = Typed;
	const int32 Serial = ++NoteSerial;
	UE_LOG(LogJam, Log, TEXT("JamGameMode: typed note \"%s\""), *Typed);

	const FJamShoot& Shoot = GS->GetShoot();
	const FJamSceneDef& Scene = Shoot.GetScene();
	const FJamNoteContext Ctx = JamNoteClamp::MakeContext(Scene, Scene.NoteBeforeStage);
	UJamNoteClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamNoteClient>() : nullptr;
	if (Client && Client->IsOnline())
	{
		TArray<FString> Kinds;
		for (const TPair<EJamTargetKind, int32>& K : Ctx.Kinds) { Kinds.Add(NoteKindName(K.Key)); }
		const AJamPlayerState* PS = GetHeroState();
		TWeakObjectPtr<AJamGameMode> WeakThis(this);
		Client->RequestNote(Scene.Id.ToString(), Ctx.BaseStage.Id.ToString(), Kinds, Ctx.MaxCapacity(), PS ? PS->Motif : FString(), Typed,
			[WeakThis, Serial, Typed](bool bOk, const FString& Body)
			{
				if (WeakThis.IsValid()) { WeakThis->ResolveTypedNote(Serial, Typed, bOk, Body); }
			});
	}
	else
	{
		// Offline: she still takes a moment to read it.
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this, Serial, Typed]()
			{
				ResolveTypedNote(Serial, Typed, false, FString());
			}), JamNoteSuggestions::OfflineReadSeconds, false);
	}
	return true;
}

void AJamGameMode::ResolveTypedNote(int32 Serial, const FString& Typed, bool bOk, const FString& Body)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || Serial != NoteSerial || !GS->bReadingNote) { return; }
	GS->bReadingNote = false;
	GS->ReadingNote.Reset();
	if (!GS->IsNotePrompt()) { return; }   // the film moved on (back to the title) while she read

	FJamShoot& Shoot = GS->Shoot;
	const FJamSceneDef& Scene = Shoot.GetScene();
	const FJamNoteContext Ctx = JamNoteClamp::MakeContext(Scene, Scene.NoteBeforeStage);
	FJamClampedNote Note;
	FString Why = bOk ? FString() : FString(TEXT("offline or no answer"));
	bool bHave = bOk && JamNoteClamp::FromResponse(Body, Ctx, Note, Why);
	if (!bHave)
	{
		UE_LOG(LogJam, Log, TEXT("JamGameMode: note service fallback (%s)"), *Why);
		bHave = JamNoteClamp::FromKeywords(Typed, Ctx, SceneIndex * 7 + NoteSerial, Note);
	}

	AJamPlayerState* PS = GetHeroState();
	if (PS) { PS->TypedNotes.Add(Typed); }
	if (!bHave)
	{
		// She can't read the handwriting: her own note it is.
		UE_LOG(LogJam, Log, TEXT("JamGameMode: typed note not understood; her own note"));
		Shoot.AnswerNote(true);
		FJamLine Line;
		Line.Id = TEXT("TypedNote_Handwriting");
		Line.Text = TEXT("I can't read your handwriting. We'll do mine. It's better anyway.");
		ShowCaption(Line);
		return;
	}

	NoteCosmetics = Note.Cosmetics;
	bTypedNoteActive = true;
	if (PS && !NoteCosmetics.Motif.IsEmpty()) { PS->Motif = NoteCosmetics.Motif; }
	GS->Weather = (uint8)NoteCosmetics.Weather;
	GS->Mood = (uint8)NoteCosmetics.Mood;
	if (!NoteCosmetics.SceneTitle.IsEmpty()) { GS->SceneTitleOverride = NoteCosmetics.SceneTitle; }
	const FJamStageDef& R = Note.Def.Replacement;
	UE_LOG(LogJam, Log, TEXT("JamGameMode: typed note applied (%s): count %d resolve x%.2f speed x%.2f scale %.2f time %.0f tint %d weather %d mood %d gross x%.2f \"%s\""),
		*Note.Source, R.Count, R.ResolveMultiplier, R.SpeedMultiplier, NoteCosmetics.Scale, R.TimeLimit, (int32)NoteCosmetics.Tint,
		(int32)NoteCosmetics.Weather, (int32)NoteCosmetics.Mood, Note.Def.GrossMultiplier, *NoteCosmetics.SceneTitle);

	Shoot.AnswerNoteWith(Note.Def);
	// Her own reaction, in place of the generic "note taken" line (HandleShootEvent skips it for a typed note).
	FJamLine Reaction;
	Reaction.Id = TEXT("TypedNote_Reaction");
	Reaction.Text = NoteCosmetics.FenellaReaction;
	if (!Reaction.Text.IsEmpty()) { ShowCaption(Reaction); }
}

void AJamGameMode::RequestGeneratedPremiere()
{
	// Only a film with typed notes has anything to generate (backend.md); otherwise the authored ending.
	const AJamPlayerState* PS = GetHeroState();
	UJamNoteClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamNoteClient>() : nullptr;
	const AJamGameState* GS = GetJamGameState();
	if (!PS || !Client || !GS || !Client->IsOnline() || PS->TypedNotes.Num() == 0) { return; }
	TArray<FString> Notes = PS->TypedNotes;
	while (Notes.Num() > 6) { Notes.RemoveAt(0); }
	TWeakObjectPtr<AJamGameMode> WeakThis(this);
	Client->RequestPremiere(PS->Motif, Notes, TakesTotal, GS->RunNotesTaken, GS->RunNotesRefused,
		JamScreens::IsDirectorsCut(GS->RunNotesTaken, GS->RunNotesRefused),
		[WeakThis](bool bOk, const FString& Body)
		{
			AJamGameMode* Self = WeakThis.Get();
			if (!Self) { return; }
			FJamPremiereTotals Read;
			if (bOk && JamScreens::ReadPremiere(Body, Read))
			{
				Self->GeneratedPremiere = Read;
				UE_LOG(LogJam, Log, TEXT("JamGameMode: generated premiere \"%s\""), *Read.Title);
				// Already on the Premiere: it changes in place.
				if (AJamGameState* G = Self->GetJamGameState())
				{
					if (G->Phase == EJamFilmPhase::Premiere) { G->Premiere.Title = Read.Title; G->Premiere.Tagline = Read.Tagline; G->Premiere.Reviews = Read.Reviews; }
				}
			}
			else { UE_LOG(LogJam, Log, TEXT("JamGameMode: premiere fallback (authored ending)")); }
		});
}

void AJamGameMode::ClearNoteCosmetics()
{
	bTypedNoteActive = false;
	NoteCosmetics = FJamNoteCosmetics();
	++NoteSerial;   // an answer still in flight is for a scene that is gone
	if (AJamGameState* GS = GetJamGameState())
	{
		GS->bReadingNote = false;
		GS->ReadingNote.Reset();
		GS->SuggestedNotes.Reset();
		GS->Weather = 0;
		GS->Mood = 0;
		GS->SceneTitleOverride.Reset();
	}
}

void AJamGameMode::StartOpenWorld(AJamHero* NewHero, const TCHAR* Cmd)
{
	if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().EnableNumericXp(); }
	int32 QuestSeed = 2026;
	FParse::Value(Cmd, TEXT("questseed="), QuestSeed);
	VillageQuests = FJamVillageQuestLog(QuestSeed);
	FString OfferPath;
	if (FParse::Value(Cmd, TEXT("questoffers="), OfferPath))
	{
		FString Json, Error;
		if (!FFileHelper::LoadFileToString(Json, *OfferPath)) { UE_LOG(LogJam, Warning, TEXT("JamQuest: cannot read offer file %s; keeping seeded recipes"), *OfferPath); }
		else if (!VillageQuests.ApplyOverridesJson(Json, Error)) { UE_LOG(LogJam, Warning, TEXT("JamQuest: invalid offer file: %s; keeping seeded recipes"), *Error); }
		else { UE_LOG(LogJam, Log, TEXT("JamQuest: loaded bounded offer overrides from %s"), *OfferPath); }
	}
	// O1: the world from its table. On the template map, build the flat fallback; on TERRAIN's map, only find
	// the ground under the village square (gameplay actors at the table's locations arrive with O4).
	const FString MapPath = GetWorld()->GetOutermost()->GetName();
	const FVector2D Start(JamOpenTuning::HeroStartX, JamOpenTuning::HeroStartY);
	if (JamOpenWorld::IsFallbackMap(MapPath))
	{
		if (AJamOpenWorld* World = GetWorld()->SpawnActor<AJamOpenWorld>(AJamOpenWorld::StaticClass(), FTransform::Identity))
		{
			World->Build(GroundZ);
		}
	}
	else
	{
		FHitResult Hit;
		FCollisionQueryParams Params(TEXT("JamGround"), false, NewHero);
		if (GetWorld()->LineTraceSingleByChannel(Hit, FVector(Start, 50000.f), FVector(Start, -50000.f), ECC_WorldStatic, Params))
		{
			GroundZ = Hit.ImpactPoint.Z;
		}
	}
	JamLook::Apply(GetWorld(), true);
	NewHero->TeleportTo(FVector(Start, GroundZ + NewHero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 2.f), FRotator::ZeroRotator);
	UE_LOG(LogJam, Log, TEXT("JamGameMode: open mode on %s (%s), %d locations"), *MapPath,
		JamOpenWorld::IsFallbackMap(MapPath) ? TEXT("flat fallback") : TEXT("terrain map"), JamWorld::Def().Locations.Num());

	Nag.Reset(JamFenellaTuning::RunSeed);
	FActorSpawnParameters FaerieParams;
	FaerieParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Faerie = GetWorld()->SpawnActor<AJamFaerie>(AJamFaerie::StaticClass(), NewHero->GetActorTransform(), FaerieParams);
	if (Faerie) { Faerie->SetFollow(NewHero); }

	// Face Sugarborough and Fenella at the square on arrival.
	BuildVillage();
	const float VillageYaw = FMath::RadiansToDegrees(FMath::Atan2(-Start.Y, -Start.X));
	NewHero->SetActorRotation(FRotator(0.f, VillageYaw, 0.f));
	NewHero->EnableFollowCamera(VillageYaw);
	float CamYaw = 0.f, CamPitch = -14.f;
	const bool bYaw = FParse::Value(Cmd, TEXT("camyaw="), CamYaw);
	const bool bPitch = FParse::Value(Cmd, TEXT("campitch="), CamPitch);
	if (bYaw || bPitch) { NewHero->PinFollowCamera(bYaw ? CamYaw : NewHero->GetCameraYaw(), CamPitch); }
	float CamDist = 0.f;
	if (FParse::Value(Cmd, TEXT("camdist="), CamDist)) { NewHero->SetCameraDistance(CamDist); }

	FString Dummy;
	const bool bSkipTitle = FParse::Value(Cmd, TEXT("scene="), Dummy) || FParse::Value(Cmd, TEXT("uiscreen="), Dummy) || FParse::Param(Cmd, TEXT("notitle"));
	if (bSkipTitle) { RequestRollCamera(); }
	else { SetPhase(EJamFilmPhase::Title); }
}

int32 AJamGameMode::NumSets() const
{
	if (!bOpenMode) { return JamBacklot::Sets().Num(); }
	int32 N = 0;
	while (JamWorld::FilmSet(N)) { ++N; }
	return N;
}

FBox2D AJamGameMode::SceneFloor(int32 Index) const
{
	if (!bOpenMode) { return JamBacklot::Sets().IsValidIndex(Index) ? JamBacklot::Sets()[Index].Bounds : FBox2D(ForceInit); }
	const FJamLocation* Set = JamWorld::FilmSet(Index);
	return Set ? JamWorld::FilmSetFloor(*Set) : FBox2D(ForceInit);
}

FVector2D AJamGameMode::SceneMark(int32 Index) const
{
	if (!bOpenMode) { return JamBacklot::HeroMark(Index); }
	// The near edge, the side facing the village, 300 uu in: where the hero walked on.
	const FJamLocation* Set = JamWorld::FilmSet(Index);
	if (!Set) { return FVector2D::ZeroVector; }
	return Set->Position - Set->Position.GetSafeNormal() * (Set->Radius - 300.f);
}

FString AJamGameMode::SceneSign(int32 Index) const
{
	if (!bOpenMode) { return JamBacklot::Sets().IsValidIndex(Index) ? JamBacklot::Sets()[Index].Sign : FString(); }
	const FJamLocation* Set = JamWorld::FilmSet(Index);
	const TArray<FJamSceneDef> Scenes = JamScenes::All();
	const FString Title = Scenes.IsValidIndex(Index) ? Scenes[Index].Title : FString();
	return Set ? FString::Printf(TEXT("%s, in %s"), *Title, JamWorld::BiomeName(Set->Biome)) : FString();
}

float AJamGameMode::TraceGround(const FVector2D& At) const
{
	FHitResult Hit;
	FCollisionQueryParams Params(TEXT("JamGround"), false, Hero.Get());
	// Object queries also hit nonblocking volumes (the village's volume top is 1000 uu).
	// Match the hero's initial blocking-ground trace so NPCs and markers stand on the terrain.
	if (GetWorld()->LineTraceSingleByChannel(Hit, FVector(At, 50000.f), FVector(At, -50000.f), ECC_WorldStatic, Params))
	{
		return Hit.ImpactPoint.Z;
	}
	return GroundZ;
}

void AJamGameMode::BeginWalkToSet(int32 Next)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || Next < 0 || Next >= NumSets()) { return; }
	SceneIndex = Next - 1;
	bBetweenScenes = true;
	SceneTime = 0.f;
	BotStuckFor = BotSideStepFor = 0.f;
	GS->SceneNumber = Next + 1;
	GS->Hint = FString::Printf(TEXT("Fenella: follow the candy cane to %s"), *SceneSign(Next));
	const FVector2D Centre = SceneFloor(Next).GetCenter();
	GS->bCompass = true;
	GS->CompassTarget = FVector(Centre, TraceGround(Centre) + 150.f);
	const TArray<FJamSceneDef> Scenes = JamScenes::All();
	GS->CompassLabel = Scenes.IsValidIndex(Next) ? Scenes[Next].Title.ToUpper() : FString(TEXT("NEXT SET"));
	MarkQuest(Centre);
	Direct(FString::Printf(TEXT("%s Next, %s. I have marked the starting point with a candy cane. Walk into the clearing and we will begin."),
		Next == FirstSceneIndex ? TEXT("You are hired! I will stay with you.") : TEXT("Good work, hero!"), *SceneSign(Next)));
	// The reality show keeps rolling between sets: one pickup per walk, in order.
	SayOnce(*FString::Printf(TEXT("RealityPickup_%02d"), FMath::Clamp(Next - FirstSceneIndex, 0, 4) + 1));
	TickOpenWalk();   // the biome's music from the first frame of the walk
	if (Hero.IsValid()) { Hero->SetFollowFraming(false); }
	UE_LOG(LogJam, Log, TEXT("JamGameMode: walk to film set %d (%s)"), Next + 1, *SceneSign(Next));
}

void AJamGameMode::TickOpenWalk()
{
	AJamGameState* GS = GetJamGameState();
	const AJamHero* H = Hero.Get();
	if (!GS || !H) { return; }
	// Music by biome (R4): the village's own cue in the village, the biome's outside it.
	const FJamBiomeDef* Biome = JamWorld::Biome(JamWorld::BiomeAt(FVector2D(H->GetActorLocation())));
	GS->WalkMusicCue = Biome ? Biome->MusicCue : NAME_None;
}
