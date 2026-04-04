// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TutorialManager.h"

#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RunStateSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66TreeOfLifeInteractable.h"
#include "Gameplay/T66TutorialGuideCompanion.h"
#include "Gameplay/T66TutorialPortal.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FName TutorialGuideCompanionID(TEXT("Companion_01"));
	const FName TutorialElectricIdolID(TEXT("Idol_Electric"));

	const FName TutorialGuideStartTag(TEXT("T66_Tutorial_GuideStart"));
	const FName TutorialMoveMarkerTag(TEXT("T66_Tutorial_Stop_Move"));
	const FName TutorialAutoAttackMarkerTag(TEXT("T66_Tutorial_Stop_AutoAttack"));
	const FName TutorialAutoAttackEnemyTag(TEXT("T66_Tutorial_Spawn_AutoAttackEnemy"));
	const FName TutorialLockMarkerTag(TEXT("T66_Tutorial_Stop_Lock"));
	const FName TutorialLockEnemyTag(TEXT("T66_Tutorial_Spawn_LockEnemy"));
	const FName TutorialItemMarkerTag(TEXT("T66_Tutorial_Stop_Item"));
	const FName TutorialItemBagTag(TEXT("T66_Tutorial_Spawn_ItemBag"));
	const FName TutorialIdolMarkerTag(TEXT("T66_Tutorial_Stop_Idol"));
	const FName TutorialIdolAnchorTag(TEXT("T66_Tutorial_Spawn_IdolAltar"));
	const FName TutorialFountainMarkerTag(TEXT("T66_Tutorial_Stop_Fountain"));
	const FName TutorialFountainAnchorTag(TEXT("T66_Tutorial_Spawn_Fountain"));
	const FName TutorialTotemMarkerTag(TEXT("T66_Tutorial_Stop_Totem"));
	const FName TutorialTotemAnchorTag(TEXT("T66_Tutorial_Spawn_Totem"));
	const FName TutorialTotemWaveAnchorTag(TEXT("T66_Tutorial_Spawn_TotemWave"));
	const FName TutorialFinalMarkerTag(TEXT("T66_Tutorial_Stop_Final"));
	const FName TutorialFinalArenaAnchorTag(TEXT("T66_Tutorial_Spawn_FinalArena"));
	const FName TutorialPortalMarkerTag(TEXT("T66_Tutorial_Stop_Portal"));
	const FName TutorialPortalAnchorTag(TEXT("T66_Tutorial_Spawn_Portal"));

	FText GetTutorialSpeakerText()
	{
		return NSLOCTEXT("T66.Tutorial", "Speaker_Aria", "Aria");
	}
}

AT66TutorialManager::AT66TutorialManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AT66TutorialManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	RunState->ResetTutorialInputFlags();
	RunState->ClearTutorialHint();
	RunState->ClearTutorialSubtitle();

	RunState->TutorialInputChanged.RemoveDynamic(this, &AT66TutorialManager::HandleTutorialInputChanged);
	RunState->TutorialInputChanged.AddDynamic(this, &AT66TutorialManager::HandleTutorialInputChanged);
	RunState->InventoryChanged.RemoveDynamic(this, &AT66TutorialManager::HandleInventoryChanged);
	RunState->InventoryChanged.AddDynamic(this, &AT66TutorialManager::HandleInventoryChanged);
	RunState->IdolsChanged.RemoveDynamic(this, &AT66TutorialManager::HandleIdolsChanged);
	RunState->IdolsChanged.AddDynamic(this, &AT66TutorialManager::HandleIdolsChanged);
	RunState->HeartsChanged.RemoveDynamic(this, &AT66TutorialManager::HandleHeartsChanged);
	RunState->HeartsChanged.AddDynamic(this, &AT66TutorialManager::HandleHeartsChanged);
	RunState->DifficultyChanged.RemoveDynamic(this, &AT66TutorialManager::HandleDifficultyChanged);
	RunState->DifficultyChanged.AddDynamic(this, &AT66TutorialManager::HandleDifficultyChanged);

	TrySpawnGuide();
	StartArrivalStep();
}

void AT66TutorialManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* PlayerPawn = GetPlayerPawn();
	if (GuideCompanion && PlayerPawn)
	{
		GuideCompanion->SetFacingActor(PlayerPawn);
	}

	if (Step == ET66TutorialStep::Arrival && PlayerPawn)
	{
		const FVector GuideLocation = GuideCompanion ? GuideCompanion->GetActorLocation() : GetTaggedLocation(TutorialGuideStartTag, FVector(-3200.f, 57000.f, 0.f));
		if (FVector::Dist2D(PlayerPawn->GetActorLocation(), GuideLocation) <= 300.f)
		{
			StartMoveLookJumpStep();
		}
	}
}

void AT66TutorialManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (RunState)
	{
		RunState->TutorialInputChanged.RemoveDynamic(this, &AT66TutorialManager::HandleTutorialInputChanged);
		RunState->InventoryChanged.RemoveDynamic(this, &AT66TutorialManager::HandleInventoryChanged);
		RunState->IdolsChanged.RemoveDynamic(this, &AT66TutorialManager::HandleIdolsChanged);
		RunState->HeartsChanged.RemoveDynamic(this, &AT66TutorialManager::HandleHeartsChanged);
		RunState->DifficultyChanged.RemoveDynamic(this, &AT66TutorialManager::HandleDifficultyChanged);
		RunState->ClearTutorialHint();
		RunState->ClearTutorialSubtitle();
	}

	Super::EndPlay(EndPlayReason);
}

void AT66TutorialManager::StartArrivalStep()
{
	Step = ET66TutorialStep::Arrival;
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "ArrivalSubtitle", "Welcome to the apocalypse. Good thing you're a chad, because this place only gets worse. Walk over here and I'll keep you alive long enough to explain it."),
		NSLOCTEXT("T66.Tutorial", "ArrivalObjective", "Walk to Aria"));
}

void AT66TutorialManager::StartMoveLookJumpStep()
{
	Step = ET66TutorialStep::MoveLookJump;
	if (RunState)
	{
		RunState->ResetTutorialInputFlags();
	}

	AdvanceGuideToMarker(TutorialMoveMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "MoveLookJumpSubtitle", "Move with your movement keys. Look with the mouse. Jump to clear junk. If your binds feel wrong, hit Escape, open Settings, then Keybinds. Fix them now, not mid-disaster."),
		NSLOCTEXT("T66.Tutorial", "MoveLookJumpObjective1", "Move, look, and jump"),
		NSLOCTEXT("T66.Tutorial", "MoveLookJumpObjective2", "Esc > Settings > Keybinds if needed"));
}

void AT66TutorialManager::StartAutoAttackStep()
{
	Step = ET66TutorialStep::AutoAttack;
	AdvanceGuideToMarker(TutorialAutoAttackMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "AutoAttackSubtitle", "Your attacks fire automatically once enemies are in range. Your job is positioning, timing, and not making stupid choices."),
		NSLOCTEXT("T66.Tutorial", "AutoAttackObjective", "Let auto attack kill the enemy"));

	const FVector SpawnLocation = SnapEnemySpawnToGround(GetTaggedLocation(TutorialAutoAttackEnemyTag, FVector(-400.f, 59250.f, 200.f)));
	AutoAttackEnemy = SpawnTutorialEnemyAt(SpawnLocation, PickStage1MobID(), false);
	if (AutoAttackEnemy)
	{
		AutoAttackEnemy->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleAutoAttackEnemyDestroyed);
	}
}

void AT66TutorialManager::StartLockTargetStep()
{
	Step = ET66TutorialStep::LockTarget;
	if (RunState)
	{
		RunState->ResetTutorialInputFlags();
	}

	AdvanceGuideToMarker(TutorialLockMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "LockSubtitle", "Use Attack Lock when chaos starts lying to you. It keeps your pressure on the target you chose."),
		NSLOCTEXT("T66.Tutorial", "LockObjective", "Lock a target and kill it"));

	const FVector SpawnLocation = SnapEnemySpawnToGround(GetTaggedLocation(TutorialLockEnemyTag, FVector(1800.f, 60350.f, 200.f)));
	LockEnemy = SpawnTutorialEnemyAt(SpawnLocation, PickStage1MobID(), false);
	if (LockEnemy)
	{
		LockEnemy->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleLockEnemyDestroyed);
	}
}

void AT66TutorialManager::StartPickupItemStep()
{
	Step = ET66TutorialStep::PickupItem;
	AdvanceGuideToMarker(TutorialItemMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "ItemSubtitle", "Loot matters. Pick up bags, read the stats, and build around what your hero actually does well."),
		NSLOCTEXT("T66.Tutorial", "ItemObjective", "Pick up the item"));

	InventoryCountAtItemStepStart = RunState ? RunState->GetInventory().Num() : 0;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector BagLocation = GetGroundPoint(GetTaggedLocation(TutorialItemBagTag, FVector(2350.f, 60600.f, 20.f))) + FVector(0.f, 0.f, 20.f);
	AT66LootBagPickup* Bag = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), BagLocation, FRotator::ZeroRotator, SpawnParams);
	if (Bag)
	{
		Bag->SetLootRarity(ET66Rarity::Black);
		Bag->SetItemID(PickTutorialPrimaryStatItemID());
	}
}

void AT66TutorialManager::StartIdolLessonStep()
{
	Step = ET66TutorialStep::IdolLesson;
	AdvanceGuideToMarker(TutorialIdolMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "IdolSubtitle", "This is an Idol Altar. Idols change how your run behaves. You're taking the Electric Idol here so you can see the system without the clutter."),
		NSLOCTEXT("T66.Tutorial", "IdolObjective", "Take the Electric Idol"));

	if (TutorialIdolAltar)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLocation = GetGroundPoint(GetTaggedLocation(TutorialIdolAnchorTag, FVector(1500.f, 61950.f, 0.f)));
	TutorialIdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLocation, GetTaggedRotation(TutorialIdolAnchorTag, FRotator::ZeroRotator), SpawnParams);
	if (TutorialIdolAltar)
	{
		TutorialIdolAltar->RemainingSelections = 1;
		TutorialIdolAltar->CatchUpSelectionsRemaining = 0;
		TutorialIdolAltar->bUseTutorialSingleOffer = true;
		TutorialIdolAltar->TutorialOfferedIdolID = TutorialElectricIdolID;
	}
}

void AT66TutorialManager::StartFountainLessonStep()
{
	Step = ET66TutorialStep::FountainLesson;
	AdvanceGuideToMarker(TutorialFountainMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "FountainSubtitle", "Fountain of Life. It heals you to full and gives you another max heart. Learn to value it before you're desperate."),
		NSLOCTEXT("T66.Tutorial", "FountainObjective", "Use the Fountain of Life"));

	MaxHPBeforeFountainUse = RunState ? RunState->GetMaxHP() : 0.f;
	if (RunState)
	{
		RunState->ApplyDamage(20);
	}

	if (TutorialFountain)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLocation = GetGroundPoint(GetTaggedLocation(TutorialFountainAnchorTag, FVector(-300.f, 63150.f, 0.f)));
	TutorialFountain = World->SpawnActor<AT66TreeOfLifeInteractable>(AT66TreeOfLifeInteractable::StaticClass(), SpawnLocation, GetTaggedRotation(TutorialFountainAnchorTag, FRotator::ZeroRotator), SpawnParams);
}

void AT66TutorialManager::StartTotemLessonStep()
{
	Step = ET66TutorialStep::TotemLesson;
	AdvanceGuideToMarker(TutorialTotemMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "TotemSubtitle", "Difficulty Totems trade safety for reward. Touch it when you're ahead, not when you're bluffing."),
		NSLOCTEXT("T66.Tutorial", "TotemObjective", "Activate the Difficulty Totem"));

	TotemsActivatedCountAtStepStart = RunState ? RunState->GetTotemsActivatedCount() : 0;
	bTotemPreviewWaveSpawned = false;
	RemainingTotemWaveEnemies = 0;

	if (TutorialDifficultyTotem)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLocation = GetGroundPoint(GetTaggedLocation(TutorialTotemAnchorTag, FVector(1500.f, 64350.f, 0.f)));
	TutorialDifficultyTotem = World->SpawnActor<AT66DifficultyTotem>(AT66DifficultyTotem::StaticClass(), SpawnLocation, GetTaggedRotation(TutorialTotemAnchorTag, FRotator::ZeroRotator), SpawnParams);
}

void AT66TutorialManager::StartFinalArenaStep()
{
	Step = ET66TutorialStep::FinalArena;
	if (RunState)
	{
		RunState->ResetTutorialInputFlags();
	}

	AdvanceGuideToMarker(TutorialFinalMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "FinalArenaSubtitle", "When the screen turns stupid, hit your ultimate and take the room back."),
		NSLOCTEXT("T66.Tutorial", "FinalArenaObjective", "Use your ultimate and clear the arena"));
	SpawnFinalArenaWave();
}

void AT66TutorialManager::SpawnPortalAndFinish()
{
	if (Step == ET66TutorialStep::PortalReady || Step == ET66TutorialStep::Done)
	{
		return;
	}

	Step = ET66TutorialStep::PortalReady;
	AdvanceGuideToMarker(TutorialPortalMarkerTag);
	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "PortalSubtitle", "That is all the hand-holding you're getting. Some things are better learned by surviving them. Step through when you're ready."),
		NSLOCTEXT("T66.Tutorial", "PortalObjective1", "Enter the portal"),
		NSLOCTEXT("T66.Tutorial", "PortalObjective2", "Stage 1 starts clean"));

	if (TutorialPortal)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector PortalLocation = GetGroundPoint(GetTaggedLocation(TutorialPortalAnchorTag, FVector(0.f, 68300.f, 0.f)));
	TutorialPortal = World->SpawnActor<AT66TutorialPortal>(AT66TutorialPortal::StaticClass(), PortalLocation, GetTaggedRotation(TutorialPortalAnchorTag, FRotator::ZeroRotator), SpawnParams);
}

bool AT66TutorialManager::TrySpawnGuide()
{
	if (GuideCompanion)
	{
		return true;
	}

	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	if (!World)
	{
		return false;
	}

	FCompanionData CompanionData;
	if (!(GI && GI->GetCompanionData(TutorialGuideCompanionID, CompanionData)))
	{
		CompanionData.CompanionID = TutorialGuideCompanionID;
		CompanionData.DisplayName = GetTutorialSpeakerText();
		CompanionData.PlaceholderColor = FLinearColor(0.95f, 0.43f, 0.43f, 1.f);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLocation = GetGroundPoint(GetTaggedLocation(TutorialGuideStartTag, FVector(-3200.f, 57000.f, 0.f)));
	GuideCompanion = World->SpawnActor<AT66TutorialGuideCompanion>(AT66TutorialGuideCompanion::StaticClass(), SpawnLocation, GetTaggedRotation(TutorialGuideStartTag, FRotator::ZeroRotator), SpawnParams);
	if (!GuideCompanion)
	{
		return false;
	}

	GuideCompanion->InitializeCompanion(CompanionData);
	GuideCompanion->StopGuideMovement();
	return true;
}

APawn* AT66TutorialManager::GetPlayerPawn() const
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		return PC->GetPawn();
	}

	return nullptr;
}

AActor* AT66TutorialManager::FindTaggedActor(FName Tag) const
{
	UWorld* World = GetWorld();
	if (!World || Tag.IsNone())
	{
		return nullptr;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor != this && Actor->ActorHasTag(Tag))
		{
			return Actor;
		}
	}

	return nullptr;
}

FVector AT66TutorialManager::GetTaggedLocation(const FName Tag, const FVector& Fallback) const
{
	if (const AActor* Actor = FindTaggedActor(Tag))
	{
		return Actor->GetActorLocation();
	}

	return Fallback;
}

FRotator AT66TutorialManager::GetTaggedRotation(const FName Tag, const FRotator& Fallback) const
{
	if (const AActor* Actor = FindTaggedActor(Tag))
	{
		return Actor->GetActorRotation();
	}

	return Fallback;
}

void AT66TutorialManager::AdvanceGuideToMarker(const FName Tag, const float AcceptanceRadius)
{
	if (!GuideCompanion)
	{
		return;
	}

	GuideCompanion->MoveToGuidePoint(GetTaggedLocation(Tag, GuideCompanion->GetActorLocation()), AcceptanceRadius);
}

void AT66TutorialManager::SetTutorialPresentation(const FText& SubtitleText, const FText& HintLine1, const FText& HintLine2) const
{
	if (!RunState)
	{
		return;
	}

	RunState->SetTutorialSubtitle(GetTutorialSpeakerText(), SubtitleText);
	RunState->SetTutorialHint(HintLine1, HintLine2);
}

bool AT66TutorialManager::HasElectricIdolEquipped() const
{
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	const UT66IdolManagerSubsystem* IdolManager = GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	if (!IdolManager)
	{
		return false;
	}

	return IdolManager->GetEquippedIdols().Contains(TutorialElectricIdolID);
}

void AT66TutorialManager::SpawnTotemPreviewWave()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Center = GetTaggedLocation(TutorialTotemWaveAnchorTag, FVector(1800.f, 64650.f, 0.f));
	const FName MobID = PickStage1MobID();
	RemainingTotemWaveEnemies = 0;

	for (int32 Index = 0; Index < 3; ++Index)
	{
		const float AngleRadians = FMath::DegreesToRadians(120.f * static_cast<float>(Index));
		const FVector Offset(FMath::Cos(AngleRadians) * 520.f, FMath::Sin(AngleRadians) * 520.f, 200.f);
		if (AT66EnemyBase* Enemy = SpawnTutorialEnemyAt(SnapEnemySpawnToGround(Center + Offset), MobID, false))
		{
			++RemainingTotemWaveEnemies;
			Enemy->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleTotemWaveEnemyDestroyed);
		}
	}
}

void AT66TutorialManager::SpawnFinalArenaWave()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Center = GetTaggedLocation(TutorialFinalArenaAnchorTag, FVector(0.f, 66500.f, 0.f));
	const FName MobID = PickStage1MobID();
	RemainingFinalArenaEnemies = 0;

	if (AT66EnemyBase* MiniBoss = SpawnTutorialEnemyAt(SnapEnemySpawnToGround(Center + FVector(0.f, 650.f, 200.f)), MobID, true))
	{
		++RemainingFinalArenaEnemies;
		MiniBoss->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleFinalArenaEnemyDestroyed);
	}

	static const FVector AddOffsets[] = {
		FVector(-900.f, -300.f, 200.f),
		FVector(0.f, -850.f, 200.f),
		FVector(900.f, -300.f, 200.f),
	};

	for (const FVector& Offset : AddOffsets)
	{
		if (AT66EnemyBase* Enemy = SpawnTutorialEnemyAt(SnapEnemySpawnToGround(Center + Offset), MobID, false))
		{
			++RemainingFinalArenaEnemies;
			Enemy->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleFinalArenaEnemyDestroyed);
		}
	}
}

void AT66TutorialManager::TryFinishFinalArenaStep()
{
	if (Step != ET66TutorialStep::FinalArena)
	{
		return;
	}

	if (RemainingFinalArenaEnemies > 0)
	{
		return;
	}

	if (RunState && !RunState->HasSeenTutorialUltimateUsed())
	{
		SetTutorialPresentation(
			NSLOCTEXT("T66.Tutorial", "FinalArenaNoUltSubtitle", "You can win ugly if you want. Still, your ultimate is how you take the screen back when it matters."),
			NSLOCTEXT("T66.Tutorial", "FinalArenaExitObjective", "Enter the portal when ready"));
	}

	SpawnPortalAndFinish();
}

FName AT66TutorialManager::PickTutorialPrimaryStatItemID() const
{
	if (!RunState)
	{
		return FName(TEXT("Item_Black_01"));
	}

	struct FStatPick
	{
		ET66HeroStatType Type = ET66HeroStatType::Damage;
		int32 Value = 0;
	};

	const TArray<FStatPick> Picks = {
		{ ET66HeroStatType::Damage, RunState->GetDamageStat() },
		{ ET66HeroStatType::AttackSpeed, RunState->GetAttackSpeedStat() },
		{ ET66HeroStatType::AttackScale, RunState->GetScaleStat() },
		{ ET66HeroStatType::Armor, RunState->GetArmorStat() },
		{ ET66HeroStatType::Evasion, RunState->GetEvasionStat() },
		{ ET66HeroStatType::Luck, RunState->GetLuckStat() },
	};

	ET66HeroStatType BestType = ET66HeroStatType::Damage;
	int32 BestValue = TNumericLimits<int32>::Lowest();
	for (const FStatPick& Pick : Picks)
	{
		if (Pick.Value > BestValue)
		{
			BestValue = Pick.Value;
			BestType = Pick.Type;
		}
	}

	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	if (!GI)
	{
		return FName(TEXT("Item_Black_01"));
	}

	if (UDataTable* ItemsDT = GI->GetItemsDataTable())
	{
		for (const FName& ItemID : ItemsDT->GetRowNames())
		{
			if (ItemID.IsNone())
			{
				continue;
			}

			FItemData ItemData;
			if (!GI->GetItemData(ItemID, ItemData))
			{
				continue;
			}

			if (ItemData.BaseBuyGold > 0 && ItemData.PrimaryStatType == BestType)
			{
				return ItemID;
			}
		}
	}

	return GI->GetRandomItemIDForLootRarity(ET66Rarity::Black);
}

FName AT66TutorialManager::PickStage1MobID() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return NAME_None;
	}

	UT66GameInstance* GI = Cast<UT66GameInstance>(World->GetGameInstance());
	if (!GI)
	{
		return NAME_None;
	}

	FStageData StageData;
	if (GI->GetStageData(1, StageData))
	{
		if (!StageData.EnemyA.IsNone()) return StageData.EnemyA;
		if (!StageData.EnemyB.IsNone()) return StageData.EnemyB;
		return StageData.EnemyC;
	}

	return NAME_None;
}

FVector AT66TutorialManager::GetGroundPoint(const FVector& InLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return InLocation;
	}

	FHitResult Hit;
	const FVector Start = InLocation + FVector(0.f, 0.f, 3000.f);
	const FVector End = InLocation + FVector(0.f, 0.f, -9000.f);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66TutorialSnapToGround), false);
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params)
		|| World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.ImpactPoint;
	}

	return InLocation;
}

FVector AT66TutorialManager::SnapEnemySpawnToGround(const FVector& InLocation) const
{
	return GetGroundPoint(InLocation) + FVector(0.f, 0.f, 110.f);
}

AT66EnemyBase* AT66TutorialManager::SpawnTutorialEnemyAt(const FVector& InLocation, const FName MobID, const bool bMiniBoss)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(AT66EnemyBase::StaticClass(), InLocation, FRotator::ZeroRotator, SpawnParams);
	if (!Enemy)
	{
		return nullptr;
	}

	Enemy->OwningDirector = nullptr;
	Enemy->bDropsLoot = false;
	Enemy->PointValue = 0;
	Enemy->XPValue = 0;

	if (!MobID.IsNone())
	{
		Enemy->ConfigureAsMob(MobID);
	}

	if (RunState)
	{
		Enemy->ApplyStageScaling(RunState->GetCurrentStage());
		Enemy->ApplyDifficultyScalar(RunState->GetDifficultyScalar());
	}

	if (bMiniBoss)
	{
		Enemy->ApplyMiniBossMultipliers(3.0f, 2.0f, 2.5f);
	}

	return Enemy;
}

void AT66TutorialManager::HandleTutorialInputChanged()
{
	if (!RunState)
	{
		return;
	}

	if (Step == ET66TutorialStep::MoveLookJump)
	{
		if (RunState->HasSeenTutorialMoveInput()
			&& RunState->HasSeenTutorialLookInput()
			&& RunState->HasSeenTutorialJumpInput())
		{
			StartAutoAttackStep();
		}
	}
}

void AT66TutorialManager::HandleInventoryChanged()
{
	if (!RunState)
	{
		return;
	}

	if (Step == ET66TutorialStep::PickupItem && RunState->GetInventory().Num() > InventoryCountAtItemStepStart)
	{
		StartIdolLessonStep();
	}
}

void AT66TutorialManager::HandleIdolsChanged()
{
	if (Step == ET66TutorialStep::IdolLesson && HasElectricIdolEquipped())
	{
		StartFountainLessonStep();
	}
}

void AT66TutorialManager::HandleHeartsChanged()
{
	if (!RunState)
	{
		return;
	}

	if (Step == ET66TutorialStep::FountainLesson && RunState->GetMaxHP() > MaxHPBeforeFountainUse)
	{
		StartTotemLessonStep();
	}
}

void AT66TutorialManager::HandleDifficultyChanged()
{
	if (!RunState || Step != ET66TutorialStep::TotemLesson)
	{
		return;
	}

	if (!bTotemPreviewWaveSpawned && RunState->GetTotemsActivatedCount() > TotemsActivatedCountAtStepStart)
	{
		bTotemPreviewWaveSpawned = true;
		SetTutorialPresentation(
			NSLOCTEXT("T66.Tutorial", "TotemWaveSubtitle", "There. Harder already. Clear this pack and keep moving."),
			NSLOCTEXT("T66.Tutorial", "TotemWaveObjective", "Clear the tougher pack"));
		SpawnTotemPreviewWave();
	}
}

void AT66TutorialManager::HandleAutoAttackEnemyDestroyed(AActor* DestroyedActor)
{
	if (Step == ET66TutorialStep::AutoAttack && DestroyedActor == AutoAttackEnemy)
	{
		AutoAttackEnemy = nullptr;
		StartLockTargetStep();
	}
}

void AT66TutorialManager::HandleLockEnemyDestroyed(AActor* DestroyedActor)
{
	if (Step != ET66TutorialStep::LockTarget || DestroyedActor != LockEnemy)
	{
		return;
	}

	LockEnemy = nullptr;
	if (RunState && RunState->HasSeenTutorialAttackLockInput())
	{
		StartPickupItemStep();
		return;
	}

	SetTutorialPresentation(
		NSLOCTEXT("T66.Tutorial", "LockRetrySubtitle", "You killed it, but you skipped the point. Lock the next one before you erase it."),
		NSLOCTEXT("T66.Tutorial", "LockRetryObjective", "Use Attack Lock and kill the enemy"));

	const FVector SpawnLocation = SnapEnemySpawnToGround(GetTaggedLocation(TutorialLockEnemyTag, FVector(1800.f, 60350.f, 200.f)) + FVector(180.f, -120.f, 0.f));
	LockEnemy = SpawnTutorialEnemyAt(SpawnLocation, PickStage1MobID(), false);
	if (LockEnemy)
	{
		LockEnemy->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleLockEnemyDestroyed);
	}
}

void AT66TutorialManager::HandleTotemWaveEnemyDestroyed(AActor* DestroyedActor)
{
	(void)DestroyedActor;
	if (Step != ET66TutorialStep::TotemLesson)
	{
		return;
	}

	RemainingTotemWaveEnemies = FMath::Max(0, RemainingTotemWaveEnemies - 1);
	if (bTotemPreviewWaveSpawned && RemainingTotemWaveEnemies <= 0)
	{
		StartFinalArenaStep();
	}
}

void AT66TutorialManager::HandleFinalArenaEnemyDestroyed(AActor* DestroyedActor)
{
	(void)DestroyedActor;
	if (Step != ET66TutorialStep::FinalArena)
	{
		return;
	}

	RemainingFinalArenaEnemies = FMath::Max(0, RemainingFinalArenaEnemies - 1);
	TryFinishFinalArenaStep();
}
