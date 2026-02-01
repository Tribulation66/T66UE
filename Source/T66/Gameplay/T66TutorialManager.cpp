// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TutorialManager.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66TutorialPortal.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AT66TutorialManager::AT66TutorialManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AT66TutorialManager::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	// Reset input flags so the tutorial reliably progresses on first-time input.
	RunState->ResetTutorialInputFlags();

	StartMoveAndJumpStep();
}

void AT66TutorialManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearStepTimeout();

	if (RunState)
	{
		RunState->TutorialInputChanged.RemoveDynamic(this, &AT66TutorialManager::HandleTutorialInputChanged);
		RunState->InventoryChanged.RemoveDynamic(this, &AT66TutorialManager::HandleInventoryChanged);
		RunState->ClearTutorialHint();
	}

	Super::EndPlay(EndPlayReason);
}

void AT66TutorialManager::StartMoveAndJumpStep()
{
	Step = ET66TutorialStep::MoveAndJump;
	if (!RunState) return;

	RunState->SetTutorialHint(
		NSLOCTEXT("T66.Tutorial", "Step_Move", "WASD to move"),
		NSLOCTEXT("T66.Tutorial", "Step_Jump", "Space bar for jump"));

	RunState->TutorialInputChanged.RemoveDynamic(this, &AT66TutorialManager::HandleTutorialInputChanged);
	RunState->TutorialInputChanged.AddDynamic(this, &AT66TutorialManager::HandleTutorialInputChanged);

	// Safety: auto-advance if the player doesn't move/jump quickly.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(StepTimeoutHandle, this, &AT66TutorialManager::StartAutoAttackEnemyStep, 6.0f, false);
	}
}

void AT66TutorialManager::StartAutoAttackEnemyStep()
{
	if (!RunState) return;
	if (Step != ET66TutorialStep::MoveAndJump) return;
	ClearStepTimeout();

	Step = ET66TutorialStep::AutoAttackEnemy;
	RunState->TutorialInputChanged.RemoveDynamic(this, &AT66TutorialManager::HandleTutorialInputChanged);

	RunState->SetTutorialHint(
		NSLOCTEXT("T66.Tutorial", "Step_AutoAttack", "You will auto attack enemies when they enter your attack radius"),
		FText::GetEmpty());

	// Spawn the first enemy inside the tutorial arena.
	const FVector SpawnLoc = SnapEnemySpawnToGround(FVector(900.f, 7900.f, 200.f));
	FirstEnemy = SpawnTutorialEnemyAt(SpawnLoc, PickStage1MobID(), false);
	if (FirstEnemy)
	{
		FirstEnemy->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleFirstEnemyDestroyed);
	}
}

void AT66TutorialManager::StartPickupFirstBagStep(const FVector& BagWorldLocation)
{
	if (!RunState) return;
	Step = ET66TutorialStep::PickupFirstBag;

	InventoryCountAtFirstBagSpawn = RunState->GetInventory().Num();
	RunState->SetTutorialHint(
		NSLOCTEXT("T66.Tutorial", "Step_PickupBag", "That bag has an item press f to pick it up"),
		FText::GetEmpty());

	// Spawn a deterministic bag item matching the hero's highest stat.
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FVector BagLoc = GetGroundPoint(BagWorldLocation + FVector(0.f, 0.f, 20.f));
		AT66LootBagPickup* Bag = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), BagLoc, FRotator::ZeroRotator, P);
		if (Bag)
		{
			Bag->SetLootRarity(ET66Rarity::Black);
			Bag->SetItemID(PickTutorialPrimaryStatItemID());
		}
	}

	RunState->InventoryChanged.RemoveDynamic(this, &AT66TutorialManager::HandleInventoryChanged);
	RunState->InventoryChanged.AddDynamic(this, &AT66TutorialManager::HandleInventoryChanged);
}

void AT66TutorialManager::StartItemExplanationStep()
{
	if (!RunState) return;
	Step = ET66TutorialStep::ItemExplanation;

	RunState->InventoryChanged.RemoveDynamic(this, &AT66TutorialManager::HandleInventoryChanged);

	RunState->SetTutorialHint(
		NSLOCTEXT("T66.Tutorial", "Step_ItemsExplain", "Items have primary stat lines but might have more."),
		FText::GetEmpty());

	// Let the player read, then spawn the elite mob + ult hint.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(StepTimeoutHandle, this, &AT66TutorialManager::StartUltMiniBossStep, 3.5f, false);
	}
}

void AT66TutorialManager::StartUltMiniBossStep()
{
	if (!RunState) return;
	if (Step != ET66TutorialStep::ItemExplanation) return;
	ClearStepTimeout();

	Step = ET66TutorialStep::UltMiniBoss;
	RunState->SetTutorialHint(
		NSLOCTEXT("T66.Tutorial", "Step_Ult", "ULT with R when you need extra power"),
		FText::GetEmpty());

	const FVector SpawnLoc = SnapEnemySpawnToGround(FVector(1200.f, 7600.f, 200.f));
	MiniBossEnemy = SpawnTutorialEnemyAt(SpawnLoc, PickStage1MobID(), true);
	if (MiniBossEnemy)
	{
		MiniBossEnemy->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleMiniBossDestroyed);
	}
}

void AT66TutorialManager::StartWhiteItemExplanationStep(const FVector& BagWorldLocation)
{
	if (!RunState) return;
	Step = ET66TutorialStep::WhiteItemExplanation;

	// White bag drop: random item is fine for now (per request).
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FVector BagLoc = GetGroundPoint(BagWorldLocation + FVector(0.f, 0.f, 20.f));
		AT66LootBagPickup* Bag = World->SpawnActor<AT66LootBagPickup>(AT66LootBagPickup::StaticClass(), BagLoc, FRotator::ZeroRotator, P);
		if (Bag)
		{
			Bag->SetLootRarity(ET66Rarity::White);
			if (UT66GameInstance* GI = Cast<UT66GameInstance>(World->GetGameInstance()))
			{
				Bag->SetItemID(GI->GetRandomItemIDForLootRarity(ET66Rarity::White));
			}
			else
			{
				Bag->SetItemID(FName(TEXT("Item_White_01")));
			}
		}
	}

	RunState->SetTutorialHint(
		NSLOCTEXT("T66.Tutorial", "Step_WhiteItems", "White items are the rarest in the game they have special effects"),
		FText::GetEmpty());

	// After a short pause, start the big wave.
	if (World)
	{
		World->GetTimerManager().SetTimer(StepTimeoutHandle, this, &AT66TutorialManager::StartBigWaveStep, 3.5f, false);
	}
}

void AT66TutorialManager::StartBigWaveStep()
{
	if (!RunState) return;
	if (Step != ET66TutorialStep::WhiteItemExplanation) return;
	ClearStepTimeout();

	Step = ET66TutorialStep::BigWave;

	// Spawn a large wave (10 enemies) inside the tutorial arena.
	RemainingWaveEnemies = 0;

	UWorld* World = GetWorld();
	if (!World) return;

	const FName MobID = PickStage1MobID();
	const FVector Center(0.f, 7500.f, 0.f);
	const float Radius = 1500.f;

	for (int32 i = 0; i < 10; ++i)
	{
		const float Angle = (360.f / 10.f) * static_cast<float>(i);
		const FVector Offset = FVector(FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius, FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius, 0.f);
		const FVector SpawnLoc = SnapEnemySpawnToGround(Center + Offset + FVector(0.f, 0.f, 200.f));
		if (AT66EnemyBase* E = SpawnTutorialEnemyAt(SpawnLoc, MobID, false))
		{
			RemainingWaveEnemies++;
			E->OnDestroyed.AddDynamic(this, &AT66TutorialManager::HandleWaveEnemyDestroyed);
		}
	}

	// No additional required text here; the prior step covers the last requested message.
}

void AT66TutorialManager::SpawnPortalAndFinish()
{
	if (!RunState) return;
	Step = ET66TutorialStep::PortalReady;

	// Clear tutorial text once the combat portion is complete; the portal itself is the affordance.
	RunState->ClearTutorialHint();

	UWorld* World = GetWorld();
	if (!World) return;

	if (TutorialPortal)
	{
		return;
	}

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector PortalLoc = GetGroundPoint(FVector(0.f, 8800.f, 200.f));
	TutorialPortal = World->SpawnActor<AT66TutorialPortal>(AT66TutorialPortal::StaticClass(), PortalLoc, FRotator::ZeroRotator, P);
}

FName AT66TutorialManager::PickTutorialPrimaryStatItemID() const
{
	if (!RunState) return FName(TEXT("Item_Black_01"));

	struct FStatPick
	{
		ET66HeroStatType Type;
		int32 Value;
	};
	const TArray<FStatPick> Picks = {
		{ ET66HeroStatType::Damage,      RunState->GetDamageStat() },
		{ ET66HeroStatType::AttackSpeed, RunState->GetAttackSpeedStat() },
		{ ET66HeroStatType::AttackSize,  RunState->GetScaleStat() },
		{ ET66HeroStatType::Armor,       RunState->GetArmorStat() },
		{ ET66HeroStatType::Evasion,     RunState->GetEvasionStat() },
		{ ET66HeroStatType::Luck,        RunState->GetLuckStat() },
	};

	ET66HeroStatType BestType = ET66HeroStatType::Damage;
	int32 BestValue = -999999;
	for (const FStatPick& P : Picks)
	{
		if (P.Value > BestValue)
		{
			BestValue = P.Value;
			BestType = P.Type;
		}
	}

	// Replace the tutorial-only items with a real item whose main stat matches the strongest stat.
	UWorld* World = GetWorld();
	UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
	if (!GI) return FName(TEXT("Item_Black_01"));

	if (UDataTable* ItemsDT = GI->GetItemsDataTable())
	{
		const TArray<FName> RowNames = ItemsDT->GetRowNames();
		for (const FName& ItemID : RowNames)
		{
			if (ItemID.IsNone()) continue;
			FItemData D;
			if (!GI->GetItemData(ItemID, D)) continue;
			if (D.ItemRarity != ET66ItemRarity::Black) continue;
			if (D.BuyValueGold <= 0) continue;
			if (D.MainStatType == BestType)
			{
				return ItemID;
			}
		}
	}

	// Safe fallback.
	return GI->GetRandomItemIDForLootRarity(ET66Rarity::Black);
}

FName AT66TutorialManager::PickStage1MobID() const
{
	UWorld* World = GetWorld();
	if (!World) return NAME_None;
	UT66GameInstance* GI = Cast<UT66GameInstance>(World->GetGameInstance());
	if (!GI) return NAME_None;

	FStageData StageData;
	if (GI->GetStageData(1, StageData))
	{
		return !StageData.EnemyA.IsNone() ? StageData.EnemyA : (!StageData.EnemyB.IsNone() ? StageData.EnemyB : StageData.EnemyC);
	}
	return NAME_None;
}

FVector AT66TutorialManager::GetGroundPoint(const FVector& InLocation) const
{
	UWorld* World = GetWorld();
	if (!World) return InLocation;

	FHitResult Hit;
	const FVector Start = InLocation + FVector(0.f, 0.f, 3000.f);
	const FVector End = InLocation + FVector(0.f, 0.f, -9000.f);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66TutorialSnapToGround), false);
	// Prefer WorldStatic (dev floors/walls). Fall back to Visibility in case collision presets differ.
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) ||
		World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.ImpactPoint;
	}
	return InLocation;
}

FVector AT66TutorialManager::SnapEnemySpawnToGround(const FVector& InLocation) const
{
	// Enemies are Characters; lift them to avoid capsule intersecting the floor.
	const FVector Ground = GetGroundPoint(InLocation);
	return Ground + FVector(0.f, 0.f, 110.f);
}

AT66EnemyBase* AT66TutorialManager::SpawnTutorialEnemyAt(const FVector& InLocation, FName MobID, bool bMiniBoss)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(AT66EnemyBase::StaticClass(), InLocation, FRotator::ZeroRotator, P);
	if (!Enemy) return nullptr;

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
		Enemy->ApplyDifficultyScalar(RunState->GetDifficultyScalar());
	}

	if (bMiniBoss)
	{
		Enemy->ApplyMiniBossMultipliers(3.0f, 2.0f, 2.5f);
	}
	return Enemy;
}

void AT66TutorialManager::ClearStepTimeout()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StepTimeoutHandle);
	}
}

void AT66TutorialManager::HandleTutorialInputChanged()
{
	if (!RunState) return;
	if (Step != ET66TutorialStep::MoveAndJump) return;

	// Advance as soon as the player has demonstrated both move + jump at least once.
	if (RunState->HasSeenTutorialMoveInput() && RunState->HasSeenTutorialJumpInput())
	{
		StartAutoAttackEnemyStep();
	}
}

void AT66TutorialManager::HandleInventoryChanged()
{
	if (!RunState) return;
	if (Step != ET66TutorialStep::PickupFirstBag) return;

	if (RunState->GetInventory().Num() > InventoryCountAtFirstBagSpawn)
	{
		StartItemExplanationStep();
	}
}

void AT66TutorialManager::HandleFirstEnemyDestroyed(AActor* DestroyedActor)
{
	if (Step != ET66TutorialStep::AutoAttackEnemy) return;

	const FVector BagLoc = DestroyedActor ? DestroyedActor->GetActorLocation() : FVector(0.f, 7600.f, 0.f);
	StartPickupFirstBagStep(BagLoc);
}

void AT66TutorialManager::HandleMiniBossDestroyed(AActor* DestroyedActor)
{
	if (Step != ET66TutorialStep::UltMiniBoss) return;

	const FVector BagLoc = DestroyedActor ? DestroyedActor->GetActorLocation() : FVector(0.f, 7600.f, 0.f);
	StartWhiteItemExplanationStep(BagLoc);
}

void AT66TutorialManager::HandleWaveEnemyDestroyed(AActor* DestroyedActor)
{
	if (Step != ET66TutorialStep::BigWave) return;
	RemainingWaveEnemies = FMath::Max(0, RemainingWaveEnemies - 1);
	if (RemainingWaveEnemies <= 0)
	{
		SpawnPortalAndFinish();
	}
}

