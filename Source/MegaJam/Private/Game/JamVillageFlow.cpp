#include "Game/JamGameMode.h"
#include "Film/JamLines.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamFaerie.h"
#include "Game/JamVillager.h"
#include "Game/JamQuestMarker.h"
#include "Game/JamPickup.h"
#include "Game/JamNuisance.h"
#include "AbilitySystemComponent.h"
#include "Gas/JamResolveAttributeSet.h"
#include "Game/JamPlayerState.h"
#include "Game/JamNoteClient.h"
#include "Engine/GameInstance.h"
#include "Game/JamSfx.h"
#include "World/JamWorld.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "MegaJam.h"
#include "Misc/CommandLine.h"

namespace
{
    // Fixes3 #10: once a giver has nothing left to offer, each has their own afterglow instead of one shared thanks.
    FString VillageAfterglow(const FString& Giver)
    {
        static const TMap<FString, TArray<FString>> Lines = {
            { TEXT("Mayor Bonbon"), {
                TEXT("Mayor Bonbon says: I've named a pothole after you. It's our finest pothole."),
                TEXT("Mayor Bonbon says: Re-election is assured. Not mine. Yours. You're mayor now. Kidding. Unless?"),
                TEXT("Mayor Bonbon says: The council rats have filed a complaint about you. I framed it.") } },
            { TEXT("Baker Crumb"), {
                TEXT("Baker Crumb says: The almond cake is real now. It's terrible. I've never been happier."),
                TEXT("Baker Crumb says: I'm baking you a celery loaf. Nobody asked for this. I'm doing it anyway."),
                TEXT("Baker Crumb says: Flour on your boots? That's a Crumb customer. Wear it with pride.") } },
            { TEXT("Gardener Sprig"), {
                TEXT("Gardener Sprig says: The celery stands taller when you walk by. It's showing off."),
                TEXT("Gardener Sprig says: I planted a row in your honour. Then the rats ate it. In your honour."),
                TEXT("Gardener Sprig says: Soil's good, crops are smug. Couldn't ask for more.") } },
            { TEXT("Dr. Molar"), {
                TEXT("Dr. Molar says: Let me see those teeth. Mm. Heroic. Floss anyway."),
                TEXT("Dr. Molar says: My waiting room is empty. Do you know how rare that is? Unsettling. Thank you."),
                TEXT("Dr. Molar says: Two minutes, twice a day. I'll know if you don't. I always know.") } },
            { TEXT("Scout Taffy"), {
                TEXT("Scout Taffy says: Perimeter secure. I counted the trees twice. One of them blinked."),
                TEXT("Scout Taffy says: I've added you to the map. You're a small heroic dot. Very accurate."),
                TEXT("Scout Taffy says: Nothing out there but goblins with good dental habits now. Your fault.") } },
        };
        if (const TArray<FString>* Own = Lines.Find(Giver)) { return (*Own)[FMath::RandRange(0, Own->Num() - 1)]; }
        return Giver + TEXT(" says: Thanks again, hero. The village remembers. The invoice has been paid.");
    }
}

namespace JamVillageFlow
{
	constexpr float TalkRange = 300.f;
	constexpr int32 FenellaTarget = -2;

	/** "Pacify 3 rats at Meadow West, then gather 3 celery at Meadow East". */
	FString PlanText(const TArray<FJamQuestStep>& Steps)
	{
		FString Out;
		for (int32 i = 0; i < Steps.Num(); ++i)
		{
			FString Step = JamVillageQuests::StepText(Steps[i]);
			if (i > 0 && !Step.IsEmpty()) { Out += TEXT(", then "); Step[0] = FChar::ToLower(Step[0]); }
			Out += Step;
		}
		return Out;
	}

	// Docs/Design/Jokes.md, "Villagers: relationship levels".
	FString LevelNote(int32 Level)
	{
		switch (FMath::Clamp(Level, 0, JamVillageQuests::MaxLevel))
		{
		case 0: return TEXT("Stranger. Finish a job for them to become Acquaintances.");
		case 1: return TEXT("Acquaintance: one more target than usual, +25 sprinkles.");
		case 2: return TEXT("Friend: their jobs come in two parts, +50 sprinkles.");
		default: return TEXT("Bestie: bigger jobs across two camps, +75 sprinkles.");
		}
	}

	FString LevelUpLine(const FString& Giver, int32 Level)
	{
		switch (FMath::Clamp(Level, 0, JamVillageQuests::MaxLevel))
		{
		case 1: return Giver + TEXT(" nods at you in the street now. You're Acquaintances. Their next job will be a little bigger.");
		case 2: return Giver + TEXT(" has saved you a seat at the harvest supper. You're Friends. Their jobs now come in two parts.");
		case 3: return Giver + TEXT(" made you a friendship bracelet out of liquorice. Besties. I've already sold the rights.");
		default: return FString();
		}
	}

	// Fenella when a proposal can't be used (any failure, refusal or timeout).
	const TCHAR* const ScriptLines[] = {
		TEXT("Fenella: Let's stick to the script, darling. The script is very good. I wrote it."),
		TEXT("Fenella: Bold. Unfilmable, but bold. Pick one of mine."),
		TEXT("Fenella: Legal says no. I am legal. Choose a route."),
	};
}

void AJamGameMode::Direct(const FString& Text)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	CaptionQueue.Reset();
	GS->CaptionLineId = TEXT("QuestDirection");
	GS->CaptionText = Text;
	GS->CaptionUntil = GetWorld()->GetTimeSeconds() + FMath::Clamp(3.f + Text.Len() * 0.055f, 5.f, 12.f);
	if (Faerie) { Faerie->Speak(4.f); }
	UE_LOG(LogJam, Log, TEXT("JamQuest: Fenella: %s"), *Text);
}

void AJamGameMode::MarkQuest(const FVector2D& At, AActor* Anchor, float Height)
{
	if (!QuestMarker) { QuestMarker = GetWorld()->SpawnActor<AJamQuestMarker>(); }
	if (QuestMarker) { QuestMarker->PointAt(FVector(At, TraceGround(At)), Anchor, Height); }
}

void AJamGameMode::BuildVillage()
{
	for (const FJamVillageGiverDef& Giver : JamVillageQuests::Givers())
	{
		const FJamLocation* Location = JamWorld::Find(Giver.Location);
		if (!Location) { continue; }
		const FVector At(Location->Position, TraceGround(Location->Position));
		AJamVillager* Villager = GetWorld()->SpawnActor<AJamVillager>(AJamVillager::StaticClass(), At, FRotator(0, Location->Yaw, 0));
		if (Villager) { Villager->SetName(Giver.Name); Villager->SetRole(Giver.Location); }
		Villagers.Add(Villager);
		AJamQuestMarker* Marker = GetWorld()->SpawnActor<AJamQuestMarker>();
		if (Marker) { Marker->PointAt(At, Villager, 400.f); Marker->SetActorHiddenInGame(true); }
		VillagerMarkers.Add(Marker);
	}
	if (Faerie)
	{
		Faerie->SetFollow(nullptr);
		Faerie->SetActorLocation(FVector(0, 0, TraceGround(FVector2D::ZeroVector) + 180.f));
	}
}

void AJamGameMode::BeginRecruitment()
{
	bRecruitmentPending = true;
	bBetweenScenes = false;
	AJamGameState* GS = GetJamGameState();
	if (!GS || !Faerie) { return; }
	GS->bCompass = true;
	GS->CompassTarget = Faerie->GetActorLocation();
	GS->CompassLabel = TEXT("FENELLA - START YOUR ADVENTURE");
	GS->Hint = TEXT("Meet Fenella in Sugarborough. Follow the candy cane; click her prompt or hold F / (A) to talk.");
	MarkQuest(FVector2D(Faerie->GetActorLocation()), Faerie, 160.f);
	// Play the introduction in order on arrival; talking to her can skip to the quest.
	CaptionQueue.Reset();
	const TArray<const FJamLine*> Intro = JamLines::ForTrigger(EJamLineTrigger::RealityIntro);
	if (!Intro.IsEmpty())
	{
		ShowCaption(*Intro[0]);
		for (int32 i = 1; i < Intro.Num(); ++i) { CaptionQueue.Add(Intro[i]->Id); }
	}
	UE_LOG(LogJam, Log, TEXT("JamQuest: recruitment waiting hero=%s fenella=%s"), Hero.IsValid() ? *Hero->GetActorLocation().ToString() : TEXT("none"), *Faerie->GetActorLocation().ToString());
}

int32 AJamGameMode::VillageTarget(const AJamHero* InHero) const
{
	const AJamGameState* GS = GetJamGameState();
	if (!bOpenMode || !InHero || InHero != Hero.Get() || !GS || GS->Phase != EJamFilmPhase::Shooting || GS->IsWrapVisible() || GS->IsNotePrompt()) { return INDEX_NONE; }
	float Best = JamVillageFlow::TalkRange;
	int32 Target = INDEX_NONE;
	auto Near = [&](const AActor* Actor, int32 Id)
	{
		if (!Actor || FMath::Abs(Actor->GetActorLocation().Z - InHero->GetActorLocation().Z) > 350.f) { return; }
		const float Distance = FVector::Dist2D(Actor->GetActorLocation(), InHero->GetActorLocation());
		if (Distance < Best) { Best = Distance; Target = Id; }
	};
	// A treatable nuisance in reach wins over Fenella's always-on town prompt, or errands in town can't be finished.
	if (bVillageExploration && VillageQuests.Active() != INDEX_NONE)
	{
		AJamNuisance* N = InHero->FindInteractTarget();
		if (N && Nuisances.Contains(N)) { return INDEX_NONE; }
	}
	if (bRecruitmentPending) { Near(Faerie, JamVillageFlow::FenellaTarget); }
	// Fenella is available at the hero's shoulder in town, including during an errand.
	if (bVillageExploration && FVector2D(InHero->GetActorLocation()).Size() < JamWorld::Def().VillageRadius)
	{
		// She follows at a little over 300 uu: the offer belongs to the hero, at her shoulder.
		Target = JamVillageFlow::FenellaTarget;
	}
	for (int32 i = 0; i < Villagers.Num(); ++i) { Near(Villagers[i], i); }
	return Target;
}

bool AJamGameMode::VillageInteractPrompt(const AJamHero* InHero, AActor*& Anchor, FString& Text) const
{
	const int32 ActiveQuest = VillageQuests.Active();
	if (InHero && bVillageExploration && ActiveQuest != INDEX_NONE && VillageQuests.CurrentStep(ActiveQuest).Verb == EJamStageVerb::Interact)
	{
		AJamNuisance* N = InHero->FindInteractTarget();
		if (N && Nuisances.Contains(N))
		{
			Anchor = N;
			Text = InHero->IsUsingGamepad() ? TEXT("Hold (A)  ") : TEXT("Click / Hold [F]  ");
			Text += VillageQuests.CurrentStep(ActiveQuest).Target == EJamTargetKind::Goblin ? TEXT("Brush teeth") : TEXT("Feed celery");
			return true;
		}
	}
	const int32 Target = VillageTarget(InHero);
	if (Target == INDEX_NONE) { return false; }
	const FString Key = InHero->IsUsingGamepad() ? TEXT("Hold (A)") : TEXT("Click / Hold [F]");
	if (Target == JamVillageFlow::FenellaTarget)
	{
		Anchor = Faerie;
		Text = Key + (bRecruitmentPending ? TEXT("  Talk to Fenella") : ActiveQuest != INDEX_NONE ? TEXT("  Talk to Fenella") : TEXT("  Fenella: watch the Premiere"));
		return Anchor != nullptr;
	}
	Anchor = Villagers[Target];
	const int32 Index = VillageQuests.ForGiver(JamVillageQuests::Givers()[Target].Location);
	if (Index == INDEX_NONE)
	{
		Text = Key + TEXT("  ") + JamVillageQuests::Givers()[Target].Name + TEXT(": talk");
		return true;
	}
	const FJamVillageQuestDef& Quest = VillageQuests.Offer(Index);
	const EJamVillageQuestState State = VillageQuests.State(Index);
	Text = Key + TEXT("  ") + Quest.GiverName + TEXT(" (") + JamVillageQuests::LevelName(VillageQuests.Level(Target)) + TEXT(")");
	if (State == EJamVillageQuestState::Ready) { Text += TEXT(": claim reward"); }
	else if (State == EJamVillageQuestState::Available && VillageQuests.Active() == INDEX_NONE) { Text += FString::Printf(TEXT(": %s (%d sprinkles)"), *Quest.Title, VillageQuests.Payout(Index)); }
	else { Text += TEXT(": talk"); }
	return true;
}

bool AJamGameMode::HandleVillageInteract(AJamHero* InHero)
{
	const int32 Target = VillageTarget(InHero);
	if (Target == INDEX_NONE) { return false; }
	if (Target == JamVillageFlow::FenellaTarget)
	{
		if (bRecruitmentPending && VillageQuests.Recruit())
		{
			bRecruitmentPending = false;
			Faerie->SetFollow(InHero);
			JamSfx::Play(this, TEXT("Quest_Accept"));
			BeginWalkToSet(FirstSceneIndex);
			UE_LOG(LogJam, Log, TEXT("JamQuest: Fenella recruited"));
		}
		else if (bVillageExploration && VillageQuests.Active() != INDEX_NONE)
		{
			Direct(TEXT("Finish the marked errand, then return to its giver. The Premiere can wait for its star."));
		}
		else if (bVillageExploration) { RequestPremiere(); }
		return true;
	}
	const int32 Index = VillageQuests.ForGiver(JamVillageQuests::Givers()[Target].Location);
	if (Index == INDEX_NONE) { Direct(VillageAfterglow(JamVillageQuests::Givers()[Target].Name)); return true; }
	const FJamVillageQuestDef& Quest = VillageQuests.Offer(Index);
	const EJamVillageQuestState State = VillageQuests.State(Index);
	// Captured before TurnIn: an encore is rebuilt bigger the moment it is paid.
	const int32 Payout = VillageQuests.Payout(Index);
	const FString GiverName = Quest.GiverName, Title = Quest.Title;
	const int32 LevelBefore = VillageQuests.Level(Target);
	if (State == EJamVillageQuestState::Ready && VillageQuests.TurnIn(Index))
	{
		if (AJamPlayerState* PS = GetHeroState())
		{
			PS->Sprinkles += Payout;
			PS->EditProgression().AddXp(EJamXpSource::Quest);
			ApplyHeroStats();
		}
		ClearStageActors();
		UpdateVillageObjective();
		JamSfx::Play(this, TEXT("Stage_Complete"));
		const int32 LevelNow = VillageQuests.Level(Target);
		Direct(LevelNow > LevelBefore
			? FString::Printf(TEXT("%d sprinkles and 40 XP. %s"), Payout, *JamVillageFlow::LevelUpLine(GiverName, LevelNow))
			: FString::Printf(TEXT("%s thanks you! %d sprinkles and 40 XP. A real payment. I shall try not to take it personally."), *GiverName, Payout));
		if (VillageQuests.Completed() == JamVillageQuests::All().Num()) { SayOnce(TEXT("ReturnPickup_02")); }
		else if (VillageQuests.Completed() * 2 >= JamVillageQuests::All().Num()) { SayOnce(TEXT("HeroPickup_06")); }
		UE_LOG(LogJam, Log, TEXT("JamQuest: turned in %s reward=%d completed=%d level=%d"), *Title, Payout, VillageQuests.Completed(), LevelNow);
	}
	else if (State == EJamVillageQuestState::Available && VillageQuests.Active() == INDEX_NONE) { OfferVillageQuest(Index); }
	else if (State == EJamVillageQuestState::Locked) { Direct(Quest.GiverName + TEXT(" has work for you after our three quests. First, follow my candy cane.")); }
	else if (State == EJamVillageQuestState::Complete) { Direct(VillageAfterglow(Quest.GiverName)); }
	else if (VillageQuests.Active() == Index) { Direct(TEXT("Finish the marked errand, then come back here for your reward.")); }
	else { Direct(TEXT("One errand at a time, hero. Finish the current quest and return to its giver first.")); }
	return true;
}

void AJamGameMode::ReturnToVillage()
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || !Hero.IsValid()) { return; }
	bVillageExploration = true;
	bBetweenScenes = false;
	GS->Wrap.bVisible = false;
	GS->bDignityVisible = false;
	GS->bBudgetOut = false;
	ClearNoteCosmetics();
	GS->CardUntil = 0.f;
	GS->Bubbles.Reset();
	SetPhase(EJamFilmPhase::Shooting);
	Hero->SetFollowFraming(false);
	Hero->SetAimOverride(FVector::ZeroVector);
	if (Faerie) { Faerie->SetFollow(Hero.Get()); Faerie->SetSulking(false); }
	UpdateVillageObjective();
	Direct(TEXT("Our film is finished! Back to Sugarborough: Our five villagers have work for you. I'll stay with you. Ask me in town when you'd like the Premiere."));
	SayOnce(TEXT("ReturnPickup_01"));
	UE_LOG(LogJam, Log, TEXT("JamQuest: return to village, follow-up quests unlocked"));
}

void AJamGameMode::AcceptVillageQuest(int32 Index)
{
	if (!VillageQuests.Accept(Index)) { return; }
	const FJamVillageQuestDef& Quest = VillageQuests.Offer(Index);
	StartVillageStep();
	Direct(FString::Printf(TEXT("%s: %s Plan: %s. Reward: %d sprinkles and 40 XP. Follow the candy cane, then return to me."),
		*Quest.GiverName, *Quest.Request, *JamVillageFlow::PlanText(JamVillageQuests::Steps(Quest)), VillageQuests.Payout(Index)));
	JamSfx::Play(this, TEXT("Quest_Accept"));
	SayOnce(SaidOnce.Contains(TEXT("RealityPickup_04")) ? TEXT("RealityPickup_05") : TEXT("RealityPickup_04"));
	UE_LOG(LogJam, Log, TEXT("JamQuest: accepted %s route=%s steps=%d level=%d"), *Quest.Title, *Quest.Route.ToString(),
		VillageQuests.StepCount(Index), VillageQuests.LevelForQuest(Index));
}

void AJamGameMode::StartVillageStep()
{
	const int32 Active = VillageQuests.Active();
	if (Active == INDEX_NONE) { return; }
	SpawnedVillageStep = VillageQuests.Step();
	const FJamQuestStep Step = VillageQuests.CurrentStep(Active);
	const FJamLocation* Destination = JamWorld::Find(Step.Destination);
	if (!Destination) { return; }
	ClearStageActors();
	FieldBounds = FBox2D(Destination->Position - FVector2D(650.f), Destination->Position + FVector2D(650.f));
	GroundZ = TraceGround(Destination->Position);
	// Collect steps are always safe; never inherit the previous errand's danger.
	VillageDanger = 0;
	if (Step.Verb == EJamStageVerb::Collect)
	{
		for (int32 i = 0; i < Step.Count; ++i)
		{
			const float Angle = UE_TWO_PI * i / Step.Count;
			const FVector2D Spot = Destination->Position + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * 330.f;
			const FTransform Transform(FVector(Spot, TraceGround(Spot) + 25.f));
			AJamPickup* P = GetWorld()->SpawnActorDeferred<AJamPickup>(AJamPickup::StaticClass(), Transform);
			if (P) { P->ArtMesh = Step.PickupMesh; P->FinishSpawning(Transform); Pickups.Add(P); }
		}
	}
	else
	{
		FJamStageDef Stage;
		Stage.Target = Step.Target;
		Stage.Count = Step.Count;
		SpawnTargets(Stage, Step.Count, false);
		// These are noisy constituents. Pacifying them is the errand; no film take or crop timer runs.
		// Fixes3 #16: first errands are safe. Repeat errands (relationship level 2+) fight back, harder each level,
		// and can knock the hero flat (VillageDefeat restarts the step).
		VillageDanger = FMath::Max(0, VillageQuests.LevelForQuest(Active) - 1);
		for (AJamNuisance* N : Nuisances)
		{
			if (!N) { continue; }
			// Monsters toughen with every village job paid. BeginPlay has already set Resolve, so scale the attributes now
			// (Max first: PreAttributeChange clamps Resolve to it).
			if (UAbilitySystemComponent* Asc = N->GetAbilitySystemComponent())
			{
				N->MaxResolveValue *= JamVillageQuests::ThreatScale(VillageQuests.TotalJobs());
				Asc->SetNumericAttributeBase(UJamResolveAttributeSet::GetMaxResolveAttribute(), N->MaxResolveValue);
				Asc->SetNumericAttributeBase(UJamResolveAttributeSet::GetResolveAttribute(), N->MaxResolveValue);
			}
			if (VillageDanger <= 0) { N->Attack.bAttacks = false; continue; }
			N->Attack = JamAttack::For(EJamTargetKind::Goblin, false);
			N->Attack.bAttacks = true;
			N->Attack.Damage *= 1.f + 0.5f * (VillageDanger - 1);
			N->Attack.Damage *= JamVillageQuests::ThreatDamageScale(VillageQuests.TotalJobs());
			if (bAutoplay) { N->Attack.Damage *= 0.25f; }   // the unattended run must still finish
			N->Attack.Cooldown = FMath::Max(1.2f, N->Attack.Cooldown - 0.4f * (VillageDanger - 1));
		}
		if (AJamGameState* State = GetJamGameState()) { State->Dignity.Reset(); }
	}
	UpdateVillageObjective();
	UE_LOG(LogJam, Log, TEXT("JamQuest: step %d/%d %s"), SpawnedVillageStep + 1, VillageQuests.StepCount(Active), *JamVillageQuests::StepText(Step));
}

void AJamGameMode::UpdateVillageObjective()
{
	AJamGameState* GS = GetJamGameState();
	if (!GS) { return; }
	for (int32 i = 0; i < VillagerMarkers.Num(); ++i)
	{
		const int32 QuestIndex = VillageQuests.ForGiver(JamVillageQuests::Givers()[i].Location);
		const EJamVillageQuestState State = VillageQuests.State(QuestIndex);
		if (VillagerMarkers[i]) { VillagerMarkers[i]->SetActorHiddenInGame(State != EJamVillageQuestState::Available && State != EJamVillageQuestState::Ready); }
	}
	GS->bCompass = true;
	const int32 Active = VillageQuests.Active();
	if (Active != INDEX_NONE)
	{
		const FJamVillageQuestDef& Quest = VillageQuests.Offer(Active);
		const FJamQuestStep Step = VillageQuests.CurrentStep(Active);
		const int32 StepCount = VillageQuests.StepCount(Active);
		const FString StepTag = StepCount > 1 ? FString::Printf(TEXT("Step %d/%d: "), VillageQuests.Step() + 1, StepCount) : FString();
		const bool bReady = VillageQuests.State(Active) == EJamVillageQuestState::Ready;
		const FJamLocation* Location = JamWorld::Find(bReady ? Quest.GiverLocation : Step.Destination);
		if (!Location) { return; }
		GS->CompassTarget = FVector(Location->Position, TraceGround(Location->Position) + 150.f);
		GS->CompassLabel = bReady ? TEXT("RETURN TO ") + Quest.GiverName.ToUpper() : Quest.Title.ToUpper();
		GS->Hint = bReady ? TEXT("Quest ready! Return to ") + Quest.GiverName + TEXT(" and talk to claim your reward.")
			: FString::Printf(TEXT("%s: %s%s %d/%d. Follow the candy cane; return to %s afterwards."), *Quest.Title,
				*StepTag, *Step.Objective, VillageQuests.Progress(), Step.Count, *Quest.GiverName);
		MarkQuest(Location->Position);
		if (bReady && QuestMarker) { QuestMarker->SetActorHiddenInGame(true); } // giver already has a marker
	}
	else
	{
		GS->CompassTarget = FVector(0, 0, TraceGround(FVector2D::ZeroVector) + 150.f);
		GS->CompassLabel = TEXT("SUGARBOROUGH");
		GS->Hint = VillageQuests.Completed() == JamVillageQuests::All().Num()
			? TEXT("All village quests complete! Villagers offer encores, or talk to Fenella in Sugarborough to watch the Premiere.")
			: TEXT("Return to Sugarborough. Talk to a villager under a candy cane. Pause > Quest Journal lists every quest.");
		MarkQuest(FVector2D::ZeroVector);
		if (Hero.IsValid() && FVector2D(Hero->GetActorLocation()).Size() < 1200.f && QuestMarker) { QuestMarker->SetActorHiddenInGame(true); }
	}
}

bool AJamGameMode::VillageCombat() const
{
	return bVillageExploration && VillageDanger > 0 && VillageQuests.Active() != INDEX_NONE;
}

void AJamGameMode::VillageDefeat()
{
	// Knocked flat on a dangerous errand: the step starts over with full Dignity (deferred, since a nuisance's
	// own tick reports the hit and the restart destroys it).
	Direct(TEXT("Knocked flat! Dignity: none. The errand starts over. Fenella is pretending she didn't film that."));
	JamSfx::Play(this, TEXT("Cut_Horn"));
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (AJamGameState* State = GetJamGameState()) { State->Dignity.Reset(); }
		// The step really starts over: progress back to 0, so the fresh nuisances all count.
		if (VillageQuests.Active() != INDEX_NONE) { VillageQuests.RestartStep(); StartVillageStep(); UpdateVillageObjective(); }
	}));
}

void AJamGameMode::TickVillage(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || GS->Phase != EJamFilmPhase::Shooting || !Hero.IsValid()) { return; }
	if (bVillageExploration)
	{
		const bool bFight = VillageCombat();
		if (bFight) { GS->Dignity.Tick(DeltaSeconds); }
		GS->bDignityVisible = bFight;
	}
	if (bRecruitmentPending || bVillageExploration) { TickOpenWalk(); }
	if (bVillageExploration && VillageQuests.Active() != INDEX_NONE && VillageQuests.State(VillageQuests.Active()) == EJamVillageQuestState::Active)
	{
		const int32 Active = VillageQuests.Active();
		// A step finished (reported from a pacify, a treat or a pickup): set up the next one here, outside those callbacks.
		if (VillageQuests.Step() != SpawnedVillageStep)
		{
			StartVillageStep();
			Direct(TEXT("Step complete! Next: ") + JamVillageQuests::StepText(VillageQuests.CurrentStep(Active)) + TEXT(". Follow the candy cane."));
		}
		if (VillageQuests.CurrentStep(Active).Verb == EJamStageVerb::Collect)
		{
			for (int32 i = Pickups.Num() - 1; i >= 0; --i)
			{
				AJamPickup* P = Pickups[i];
				if (P && FVector::Dist(Hero->GetActorLocation() - FVector(0, 0, Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), P->GetActorLocation()) < 150.f)
				{
					P->Destroy(); Pickups.RemoveAt(i);
					VillageQuests.Report(EJamStageVerb::Collect);
					if (AJamPlayerState* PS = GetHeroState()) { PS->EditProgression().AddXp(EJamXpSource::Collect); }
					UpdateVillageObjective();
					if (VillageQuests.State(Active) == EJamVillageQuestState::Ready)
					{
						Direct(TEXT("That is everything. Return to the quest giver for your reward. Resist the urge to eat the evidence."));
						SayOnce(TEXT("HeroPickup_03"));
					}
				}
			}
		}
	}
	else if (bVillageExploration && VillageQuests.Active() == INDEX_NONE && QuestMarker)
	{
		QuestMarker->SetActorHiddenInGame(FVector2D(Hero->GetActorLocation()).Size() < 1200.f);
	}
	AActor* Anchor = nullptr;
	GS->VillagePrompt.Reset();
	GS->VillagePromptAnchor.Reset();
	if (VillageInteractPrompt(Hero.Get(), Anchor, GS->VillagePrompt)) { GS->VillagePromptAnchor = Anchor; }
}

void AJamGameMode::AutoWalkTo(const FVector2D& Goal, float DeltaSeconds)
{
	AJamHero* Bot = Hero.Get();
	if (!Bot) { return; }
	const FVector2D Here(Bot->GetActorLocation());
	FVector2D Dir = (Goal - Here).GetSafeNormal();
	BotStuckFor = FVector2D::Distance(Here, BotLastPos) < 60.f * DeltaSeconds ? BotStuckFor + DeltaSeconds : 0.f;
	BotLastPos = Here;
	if (BotStuckFor > 0.6f) { BotSideStepFor = 1.f; BotSideSign = -BotSideSign; BotStuckFor = 0.f; }
	if (BotSideStepFor > 0.f)
	{
		BotSideStepFor -= DeltaSeconds;
		Dir = (FVector2D(-Dir.Y, Dir.X) * BotSideSign + Dir * 0.25f).GetSafeNormal();
	}
	Bot->SetAimOverride(FVector(Dir, 0.f));
	Bot->AddMovementInput(FVector(Dir, 0.f));
}

void AJamGameMode::TickAutoplayVillage(float DeltaSeconds)
{
	AJamHero* Bot = Hero.Get();
	if (!Bot) { return; }
	if (AfterRunTime > 1500.f)
	{
		UE_LOG(LogJam, Error, TEXT("JamVillageAutoplay: FAIL timeout active=%d progress=%d"), VillageQuests.Active(), VillageQuests.Progress());
		bAutoplayEndLogged = true;
		return;
	}
	if (!bVillageExploration) { Bot->RequestInteract(); return; }
	if (QuestOffer.bVisible)
    {
        FString AutoProposal;
        if (FParse::Value(FCommandLine::Get(), TEXT("autopropose="), AutoProposal) && !AutoProposal.IsEmpty())
        {
            if (QuestOffer.bCanPropose) { RequestVillageProposal(AutoProposal); return; }
            if (QuestOffer.bProposing) { return; }
            const int32 Plan = QuestOffer.Branches.IndexOfByPredicate([](const FJamQuestBranch& B) { return !B.Plan.IsEmpty(); });
            if (Plan != INDEX_NONE) { ChooseVillageBranch(Plan); return; }
        }
        const bool bBranches = FParse::Param(FCommandLine::Get(), TEXT("autobranches"));
        if (bBranches && QuestOffer.bCanImprovise && !QuestOffer.bReading) { RequestVillageBranches(); return; }
        if (bBranches && QuestOffer.bReading) { return; }
        ChooseVillageBranch(bBranches ? 1 : 0);
        return;
    }
	if (VillageQuests.Completed() == JamVillageQuests::All().Num())
	{
		Bot->SetAimOverride(FVector::ZeroVector);
		bAutoplayEndLogged = true;
		UE_LOG(LogJam, Log, TEXT("JamVillageAutoplay: PASS main=%d side=%d"), ScenesWrapped, VillageQuests.Completed());
		if (FParse::Param(FCommandLine::Get(), TEXT("quitaftervillage"))) { FPlatformMisc::RequestExit(false, TEXT("Village autoplay complete")); }
		return;
	}
	const int32 Active = VillageQuests.Active();
	AActor* Goal = nullptr;
	bool bTalk = false;
	if (Active == INDEX_NONE)
	{
		for (int32 i = 0; i < Villagers.Num(); ++i)
		{
			const int32 QuestIndex = VillageQuests.ForGiver(JamVillageQuests::Givers()[i].Location);
			// Finish the ten authored jobs before considering repeatable encores.
			if (QuestIndex >= 0 && QuestIndex < JamVillageQuests::All().Num() && VillageQuests.State(QuestIndex) == EJamVillageQuestState::Available) { Goal = Villagers[i]; bTalk = true; break; }
		}
	}
	else if (VillageQuests.State(Active) == EJamVillageQuestState::Ready) { Goal = Villagers[VillageQuests.GiverIndex(Active)]; bTalk = true; }
	else
	{
		float Best = TNumericLimits<float>::Max();
		auto Consider = [&](AActor* Actor)
		{
			if (!IsValid(Actor)) { return; }
			const float Distance = FVector::DistSquared2D(Actor->GetActorLocation(), Bot->GetActorLocation());
			if (Distance < Best) { Best = Distance; Goal = Actor; }
		};
		const EJamStageVerb Verb = VillageQuests.CurrentStep(Active).Verb;
		if (Verb == EJamStageVerb::Collect) { for (AJamPickup* P : Pickups) { Consider(P); } }
		else { for (AJamNuisance* N : Nuisances) { if (N && !N->IsTreated() && (!N->IsPacified() || Verb == EJamStageVerb::Interact)) { Consider(N); } } }
	}
	if (!Goal) { return; }
	const FVector To = Goal->GetActorLocation() - Bot->GetActorLocation();
	if (bTalk && To.Size2D() < 230.f) { Bot->RequestInteract(); return; }
	if (AJamNuisance* N = Cast<AJamNuisance>(Goal); N && N->IsPacified() && To.Size2D() < 110.f) { Bot->RequestInteract(); return; }
	if (!bTalk && Cast<AJamNuisance>(Goal) && !Cast<AJamNuisance>(Goal)->IsPacified() && To.Size2D() < 450.f)
	{
		Bot->SetAimOverride(To.GetSafeNormal2D());
		if (!Bot->RequestSkill(EJamSkill::FormALine)) { Bot->RequestLecture(); }
		return;
	}
	AutoWalkTo(FVector2D(Goal->GetActorLocation()), DeltaSeconds);
}

bool AJamGameMode::AbandonVillageQuest()
{
    if (!bVillageExploration || !VillageQuests.Abandon()) { return false; }
    ClearStageActors();
    UpdateVillageObjective();
    Direct(TEXT("Errand set aside. Nothing paid, nothing owed. Talk to another villager, or take this job again when you are ready."));
    SayOnce(TEXT("RealityPickup_06"));
    return true;
}

void AJamGameMode::CancelVillageOffer()
{
    ++BranchRequestSerial;
    ++ProposalSerial;
    QuestOffer = FJamQuestOfferView();
    OfferedQuest = INDEX_NONE;
}

void AJamGameMode::OfferVillageQuest(int32 Index)
{
    if (QuestOffer.bVisible || VillageQuests.State(Index) != EJamVillageQuestState::Available) { return; }
    CancelVillageOffer();
    OfferedQuest = Index;
    const FJamVillageQuestDef& Current = VillageQuests.Offer(Index);
    QuestOffer.bVisible = true;
    QuestOffer.Title = Current.Title; QuestOffer.Giver = Current.GiverName; QuestOffer.Request = Current.Request;
    QuestOffer.Level = VillageQuests.LevelForQuest(Index);
    QuestOffer.LevelBonus = JamVillageQuests::LevelBonus(QuestOffer.Level);
    QuestOffer.LevelNote = JamVillageFlow::LevelNote(QuestOffer.Level);
    UJamNoteClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamNoteClient>() : nullptr;
    const bool bOnline = Client && Client->IsOnline();
    QuestOffer.bCanPropose = bOnline;
    if (Current.bEncore)
    {
        // An encore has one route, already built for the relationship; the player can still propose their own.
        QuestOffer.BaseCount = Current.Count; QuestOffer.BaseReward = Current.Reward;
        FJamQuestBranch Branch;
        Branch.Route = Current.Route; Branch.Label = TEXT("Take the encore");
        Branch.Objective = JamVillageFlow::PlanText(JamVillageQuests::Steps(Current));
        Branch.Variation.Count = Current.Count; Branch.Variation.Pitch = Current.Request; Branch.Reward = Current.Reward;
        QuestOffer.Branches.Add(MoveTemp(Branch));
        QuestOffer.Status = TEXT("An encore: bigger every time you finish one.");
        return;
    }
    QuestOffer.BaseCount = JamVillageQuests::All()[Index].Count; QuestOffer.BaseReward = JamVillageQuests::All()[Index].Reward;
    const TArray<FJamVillageQuestDef> Recipes = JamVillageQuests::Routes(Index);
    for (const FJamVillageQuestDef& Recipe : Recipes)
    {
        FJamQuestBranch Branch;
        Branch.Route = Recipe.Route; Branch.Objective = Recipe.Objective;
        Branch.Label = Recipe.Route == FName(TEXT("original")) ? TEXT("Take the original approach") : TEXT("Try the other approach");
        Branch.Variation.Count = FMath::Clamp(Current.Count, Recipe.MinCount, Recipe.MaxCount);
        Branch.Variation.Destination = Recipe.Destinations.Contains(Current.Destination) ? Current.Destination : Recipe.Destinations[0];
        Branch.Variation.Pitch = Recipe.Route == Current.Route ? Current.Request : Recipe.Request;
        QuestOffer.Branches.Add(MoveTemp(Branch));
    }
    QuestOffer.bCanImprovise = bOnline;
    QuestOffer.Status = TEXT("Choose how to help. Both approaches return to this villager for payment.");
    // Fenella pitches each villager's first job as television (VillagePickup_01.._05 in Givers() order), then the choice.
    SayOnce(*FString::Printf(TEXT("VillagePickup_%02d"), VillageQuests.GiverIndex(Index) + 1));
    SayOnce(TEXT("VillagePickup_06"));
}

void AJamGameMode::ChooseVillageBranch(int32 Choice)
{
	if (!GetJamGameState() || GetJamGameState()->Phase != EJamFilmPhase::Shooting || !bVillageExploration) { CancelVillageOffer(); return; }
    if (!QuestOffer.bVisible || !QuestOffer.Branches.IsValidIndex(Choice) || OfferedQuest == INDEX_NONE) { return; }
    const int32 Index = OfferedQuest;
    if (!VillageQuests.SelectBranch(Index, QuestOffer.Branches[Choice])) { CancelVillageOffer(); return; }
    CancelVillageOffer();
    AcceptVillageQuest(Index);
}

void AJamGameMode::RequestVillageBranches()
{
    if (!QuestOffer.bVisible || QuestOffer.bReading || QuestOffer.bProposing || !QuestOffer.bCanImprovise || OfferedQuest == INDEX_NONE) { return; }
    UJamNoteClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamNoteClient>() : nullptr;
    if (!Client || !Client->IsOnline()) { return; }
    const int32 Index = OfferedQuest;
    const int32 Serial = ++BranchRequestSerial;
    QuestOffer.bReading = true;
    QuestOffer.Status = TEXT("Fenella is thinking of two approaches. You can still choose an authored route.");
    const AJamPlayerState* PS = GetHeroState();
    TWeakObjectPtr<AJamGameMode> Weak(this);
    Client->RequestQuestBranches(VillageQuests.Offer(Index), JamVillageQuests::Routes(Index), VillageQuests.CompletedIds(), VillageQuests.CompletedRoutes(),
        PS ? PS->Motif : FString(), [Weak, Serial, Index](bool bOk, const FString& Body)
    {
        if (AJamGameMode* GM = Weak.Get()) { GM->ResolveVillageBranches(Serial, Index, bOk, Body); }
    });
}

void AJamGameMode::ResolveVillageBranches(int32 Serial, int32 Index, bool bOk, const FString& Body)
{
	if (!GetJamGameState() || GetJamGameState()->Phase != EJamFilmPhase::Shooting) { return; }
    if (BranchRequestSerial != Serial || OfferedQuest != Index || !QuestOffer.bVisible) { return; }
    QuestOffer.bReading = false;
    QuestOffer.bCanImprovise = false;
    TArray<FJamQuestBranch> Branches;
    if (bOk && JamVillageQuests::ReadBranches(Body, Index, Branches))
    {
        QuestOffer.Branches = MoveTemp(Branches);
        QuestOffer.Status = TEXT("Fenella has two new approaches. Your choice decides the task and destination.");
        UE_LOG(LogJam, Log, TEXT("JamQuest: backend branches ready quest=%d"), Index);
    }
    else
    {
        QuestOffer.Status = TEXT("Fenella keeps the original plans. Both routes are ready to play.");
        UE_LOG(LogJam, Log, TEXT("JamQuest: authored branches retained quest=%d"), Index);
    }
}

// ---- Player proposals (Fixes2 #4, Docs/Design/QuestBranches.md "Proposals") -------------------------------------

void AJamGameMode::RequestVillageProposal(const FString& Text)
{
    if (!QuestOffer.bVisible || QuestOffer.bProposing || QuestOffer.bReading || !QuestOffer.bCanPropose || OfferedQuest == INDEX_NONE) { return; }
    FString Clean;
    for (TCHAR C : Text) { if (C >= 32 && C != 127) { Clean.AppendChar(C); } }
    Clean = Clean.Left(160).TrimStartAndEnd();
    if (Clean.IsEmpty()) { return; }
    UJamNoteClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamNoteClient>() : nullptr;
    if (!Client || !Client->IsOnline()) { return; }
    const int32 Index = OfferedQuest;
    const int32 Serial = ++ProposalSerial;
    QuestOffer.bProposing = true;
    QuestOffer.bCanPropose = false;   // one proposal per offer
    // A later branch reply would replace the proposal's third route. Keep the
    // two generators exclusive for this offer while authored routes stay usable.
    QuestOffer.bCanImprovise = false;
    QuestOffer.Status = TEXT("Fenella is reading your plan. The routes above are still yours to pick.");
    const AJamPlayerState* PS = GetHeroState();
    TWeakObjectPtr<AJamGameMode> Weak(this);
    Client->RequestQuestProposal(VillageQuests.Offer(Index), VillageQuests.LevelForQuest(Index), Clean, VillageQuests.CompletedIds(),
        VillageQuests.CompletedRoutes(), PS ? PS->Motif : FString(), [Weak, Serial, Index](bool bOk, const FString& Body)
    {
        if (AJamGameMode* GM = Weak.Get()) { GM->ResolveVillageProposal(Serial, Index, bOk, Body); }
    });
    UE_LOG(LogJam, Log, TEXT("JamQuest: proposal sent quest=%d \"%s\""), Index, *Clean);
}

void AJamGameMode::ResolveVillageProposal(int32 Serial, int32 Index, bool bOk, const FString& Body)
{
    if (!GetJamGameState() || GetJamGameState()->Phase != EJamFilmPhase::Shooting) { return; }
    if (ProposalSerial != Serial || OfferedQuest != Index || !QuestOffer.bVisible) { return; }
    QuestOffer.bProposing = false;
    FJamQuestPlan Plan;
    if (bOk && JamVillageQuests::ReadProposal(Body, VillageQuests.Offer(Index).Id, VillageQuests.LevelForQuest(Index), Plan))
    {
        FJamQuestBranch Branch;
        Branch.Route = TEXT("proposal");
        Branch.Label = TEXT("Your plan: ") + Plan.Label;
        Branch.Plan = Plan.Steps;
        Branch.Objective = JamVillageFlow::PlanText(Plan.Steps);
        Branch.Variation.Pitch = Plan.Pitch;
        for (const FJamQuestStep& S : Plan.Steps) { Branch.Variation.Count += S.Count; }
        Branch.Reward = VillageQuests.PlanReward(Index, Plan.Steps);
        QuestOffer.Branches.Add(MoveTemp(Branch));
        // On the screen itself: captions hide while a menu is open.
        QuestOffer.Status = TEXT("Fenella: ") + Plan.Reaction;
        UE_LOG(LogJam, Log, TEXT("JamQuest: proposal ready quest=%d steps=%d"), Index, Plan.Steps.Num());
    }
    else
    {
        QuestOffer.Status = JamVillageFlow::ScriptLines[FMath::RandHelper(int32(UE_ARRAY_COUNT(JamVillageFlow::ScriptLines)))];
        UE_LOG(LogJam, Log, TEXT("JamQuest: proposal unusable quest=%d ok=%d; authored routes retained"), Index, bOk ? 1 : 0);
    }
}

TArray<FString> AJamGameMode::ProposalSuggestions() const
{
    if (OfferedQuest == INDEX_NONE) { return {}; }
    const FJamQuestStep First = JamVillageQuests::FirstStep(VillageQuests.Offer(OfferedQuest));
    if (First.Verb == EJamStageVerb::Collect)
    {
        return { TEXT("Gather twice as much"), TEXT("Calm the locals first, then gather"), TEXT("Gather celery instead and share it") };
    }
    switch (First.Target)
    {
    case EJamTargetKind::Goblin: return { TEXT("Calm the goblins and brush their teeth"), TEXT("Bribe them with candy shards"), TEXT("Visit two goblin camps") };
    case EJamTargetKind::Bug:    return { TEXT("Calm the bugs, then tidy up the candy shards"), TEXT("Lecture the bugs twice as hard"), TEXT("Gather candy shards to distract them") };
    default:                     return { TEXT("Calm the rats, then feed them celery"), TEXT("Bribe them with celery"), TEXT("Lecture rats at two camps") };
    }
}
