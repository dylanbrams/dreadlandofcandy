#include "Game/JamGameMode.h"
#include "Film/JamLines.h"
#include "Game/JamGameState.h"
#include "Game/JamHero.h"
#include "Game/JamFaerie.h"
#include "Game/JamVillager.h"
#include "Game/JamQuestMarker.h"
#include "Game/JamPickup.h"
#include "Game/JamNuisance.h"
#include "Game/JamPlayerState.h"
#include "Game/JamNoteClient.h"
#include "Engine/GameInstance.h"
#include "Game/JamSfx.h"
#include "World/JamWorld.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "MegaJam.h"
#include "Misc/CommandLine.h"

namespace JamVillageFlow
{
	constexpr float TalkRange = 300.f;
	constexpr int32 FenellaTarget = -2;
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
		if (Marker) { Marker->PointAt(At, Villager, 320.f); Marker->SetActorHiddenInGame(true); }
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
	GS->Hint = TEXT("Meet Fenella in Sugarborough. Follow the golden triangle; click her prompt or hold F / (A) to talk.");
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
	if (bRecruitmentPending) { Near(Faerie, JamVillageFlow::FenellaTarget); }
	// Fenella offers the Premiere only back in town, between errands.
	if (bVillageExploration && VillageQuests.Active() == INDEX_NONE && FVector2D(InHero->GetActorLocation()).Size() < JamWorld::Def().VillageRadius)
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
	if (InHero && bVillageExploration && ActiveQuest != INDEX_NONE && VillageQuests.Offer(ActiveQuest).Verb == EJamStageVerb::Interact)
	{
		AJamNuisance* N = InHero->FindInteractTarget();
		if (N && Nuisances.Contains(N))
		{
			Anchor = N;
			Text = InHero->IsUsingGamepad() ? TEXT("Hold (A)  ") : TEXT("Click / Hold [F]  ");
			Text += VillageQuests.Offer(ActiveQuest).Target == EJamTargetKind::Goblin ? TEXT("Brush teeth") : TEXT("Feed celery");
			return true;
		}
	}
	const int32 Target = VillageTarget(InHero);
	if (Target == INDEX_NONE) { return false; }
	const FString Key = InHero->IsUsingGamepad() ? TEXT("Hold (A)") : TEXT("Click / Hold [F]");
	if (Target == JamVillageFlow::FenellaTarget)
	{
		Anchor = Faerie;
		Text = Key + (bRecruitmentPending ? TEXT("  Talk to Fenella") : TEXT("  Fenella: watch the Premiere"));
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
	Text = Key + TEXT("  ") + Quest.GiverName;
	if (State == EJamVillageQuestState::Ready) { Text += TEXT(": claim reward"); }
	else if (State == EJamVillageQuestState::Available && VillageQuests.Active() == INDEX_NONE) { Text += FString::Printf(TEXT(": %s (%d sprinkles)"), *Quest.Title, Quest.Reward); }
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
		else if (bVillageExploration) { RequestPremiere(); }
		return true;
	}
	const int32 Index = VillageQuests.ForGiver(JamVillageQuests::Givers()[Target].Location);
	if (Index == INDEX_NONE) { Direct(JamVillageQuests::Givers()[Target].Name + TEXT(" says: Thanks again, hero. The village remembers. The invoice has been paid.")); return true; }
	const FJamVillageQuestDef& Quest = VillageQuests.Offer(Index);
	const EJamVillageQuestState State = VillageQuests.State(Index);
	if (State == EJamVillageQuestState::Ready && VillageQuests.TurnIn(Index))
	{
		if (AJamPlayerState* PS = GetHeroState())
		{
			PS->Sprinkles += Quest.Reward;
			PS->EditProgression().AddXp(EJamXpSource::Quest);
			ApplyHeroStats();
		}
		ClearStageActors();
		UpdateVillageObjective();
		JamSfx::Play(this, TEXT("Stage_Complete"));
		Direct(FString::Printf(TEXT("%s thanks you! %d sprinkles and 40 XP. A real payment. I shall try not to take it personally."), *Quest.GiverName, Quest.Reward));
		UE_LOG(LogJam, Log, TEXT("JamQuest: turned in %s reward=%d completed=%d"), *Quest.Title, Quest.Reward, VillageQuests.Completed());
	}
	else if (State == EJamVillageQuestState::Available && VillageQuests.Active() == INDEX_NONE) { OfferVillageQuest(Index); }
	else if (State == EJamVillageQuestState::Locked) { Direct(Quest.GiverName + TEXT(" has work for you after our three quests. First, follow my golden triangle.")); }
	else if (State == EJamVillageQuestState::Complete) { Direct(Quest.GiverName + TEXT(" says: Thanks again, hero. The village remembers. The invoice has been paid.")); }
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
	UE_LOG(LogJam, Log, TEXT("JamQuest: return to village, follow-up quests unlocked"));
}

void AJamGameMode::AcceptVillageQuest(int32 Index)
{
	if (!VillageQuests.Accept(Index)) { return; }
	const FJamVillageQuestDef& Quest = VillageQuests.Offer(Index);
	const FJamLocation* Destination = JamWorld::Find(Quest.Destination);
	if (!Destination) { return; }
	ClearStageActors();
	FieldBounds = FBox2D(Destination->Position - FVector2D(650.f), Destination->Position + FVector2D(650.f));
	GroundZ = TraceGround(Destination->Position);
	if (Quest.Verb == EJamStageVerb::Collect)
	{
		for (int32 i = 0; i < Quest.Count; ++i)
		{
			const float Angle = UE_TWO_PI * i / Quest.Count;
			const FVector2D Spot = Destination->Position + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * 330.f;
			const FTransform Transform(FVector(Spot, TraceGround(Spot) + 25.f));
			AJamPickup* P = GetWorld()->SpawnActorDeferred<AJamPickup>(AJamPickup::StaticClass(), Transform);
			if (P) { P->ArtMesh = Quest.PickupMesh; P->FinishSpawning(Transform); Pickups.Add(P); }
		}
	}
	else
	{
		FJamStageDef Stage;
		Stage.Target = Quest.Target;
		Stage.Count = Quest.Count;
		SpawnTargets(Stage, Quest.Count, false);
		// These are noisy constituents. Pacifying them is the errand; no film take or crop timer runs.
		for (AJamNuisance* N : Nuisances) { if (N) { N->Attack.bAttacks = false; } }
	}
	UpdateVillageObjective();
	Direct(FString::Printf(TEXT("%s: %s Target: %d. Reward: %d sprinkles and 40 XP. Follow the triangle, then return to me."), *Quest.GiverName, *Quest.Request, Quest.Count, Quest.Reward));
	JamSfx::Play(this, TEXT("Quest_Accept"));
	UE_LOG(LogJam, Log, TEXT("JamQuest: accepted %s"), *Quest.Title);
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
		const bool bReady = VillageQuests.State(Active) == EJamVillageQuestState::Ready;
		const FJamLocation* Location = JamWorld::Find(bReady ? Quest.GiverLocation : Quest.Destination);
		if (!Location) { return; }
		GS->CompassTarget = FVector(Location->Position, TraceGround(Location->Position) + 150.f);
		GS->CompassLabel = bReady ? TEXT("RETURN TO ") + Quest.GiverName.ToUpper() : Quest.Title.ToUpper();
		GS->Hint = bReady ? TEXT("Quest ready! Return to ") + Quest.GiverName + TEXT(" and talk to claim your reward.")
			: FString::Printf(TEXT("%s: %s %d/%d. Follow the golden triangle; return to %s afterwards."), *Quest.Title,
				*Quest.Objective, VillageQuests.Progress(), Quest.Count, *Quest.GiverName);
		MarkQuest(Location->Position);
		if (bReady && QuestMarker) { QuestMarker->SetActorHiddenInGame(true); } // giver already has a marker
	}
	else
	{
		GS->CompassTarget = FVector(0, 0, TraceGround(FVector2D::ZeroVector) + 150.f);
		GS->CompassLabel = TEXT("SUGARBOROUGH");
		GS->Hint = VillageQuests.Completed() == JamVillageQuests::All().Num()
			? TEXT("All village quests complete! Explore with Fenella, or talk to her in Sugarborough to watch the Premiere.")
			: TEXT("Return to Sugarborough. Talk to a villager under a golden triangle. Pause > Quest Journal lists every quest.");
		MarkQuest(FVector2D::ZeroVector);
		if (Hero.IsValid() && FVector2D(Hero->GetActorLocation()).Size() < 1200.f && QuestMarker) { QuestMarker->SetActorHiddenInGame(true); }
	}
}

void AJamGameMode::TickVillage(float DeltaSeconds)
{
	AJamGameState* GS = GetJamGameState();
	if (!GS || GS->Phase != EJamFilmPhase::Shooting || !Hero.IsValid()) { return; }
	if (bRecruitmentPending || bVillageExploration) { TickOpenWalk(); }
	if (bVillageExploration && VillageQuests.Active() != INDEX_NONE && VillageQuests.State(VillageQuests.Active()) == EJamVillageQuestState::Active)
	{
		const int32 Active = VillageQuests.Active();
		if (VillageQuests.Offer(Active).Verb == EJamStageVerb::Collect)
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
					if (VillageQuests.State(Active) == EJamVillageQuestState::Ready) { Direct(TEXT("That is everything. Return to the quest giver for your reward. Resist the urge to eat the evidence.")); }
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
			if (VillageQuests.State(QuestIndex) == EJamVillageQuestState::Available) { Goal = Villagers[i]; bTalk = true; break; }
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
		if (VillageQuests.Offer(Active).Verb == EJamStageVerb::Collect) { for (AJamPickup* P : Pickups) { Consider(P); } }
		else { for (AJamNuisance* N : Nuisances) { if (N && !N->IsTreated() && (!N->IsPacified() || VillageQuests.Offer(Active).Verb == EJamStageVerb::Interact)) { Consider(N); } } }
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
    return true;
}

void AJamGameMode::CancelVillageOffer()
{
    ++BranchRequestSerial;
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
    QuestOffer.BaseCount = JamVillageQuests::All()[Index].Count; QuestOffer.BaseReward = JamVillageQuests::All()[Index].Reward;
    QuestOffer.Title = Current.Title; QuestOffer.Giver = Current.GiverName; QuestOffer.Request = Current.Request;
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
    UJamNoteClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UJamNoteClient>() : nullptr;
    QuestOffer.bCanImprovise = Client && Client->IsOnline();
    QuestOffer.Status = TEXT("Choose how to help. Both approaches return to this villager for payment.");
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
    if (!QuestOffer.bVisible || QuestOffer.bReading || !QuestOffer.bCanImprovise || OfferedQuest == INDEX_NONE) { return; }
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
