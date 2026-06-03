// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66CombatShared.h"
#include "Animation/AnimSequence.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/T66UIManager.h"
#include "UI/T66WidgetDumpTargets.h"
#include "UI/T66ScreenBase.h"
#include "UI/Screens/T66HeroGridScreen.h"
#include "UI/Screens/T66CompanionGridScreen.h"
#include "UI/Screens/T66SaveSlotsScreen.h"
#include "UI/Screens/T66AchievementsScreen.h"
#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/Screens/T66ReportBugScreen.h"
#include "UI/Screens/T66SettingsScreen.h"
#include "UI/Screens/T66RunSummaryScreen.h"
#include "UI/Screens/T66PlayerSummaryPickerScreen.h"
#include "UI/Screens/T66SavePreviewScreen.h"
#include "UI/Screens/T66PowerUpScreen.h"
#include "UI/Screens/T66AccountStatusScreen.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "UI/T66GameplayHUDWidget.h"
#include "UI/Style/T66Style.h"
#include "UI/T66LabOverlayWidget.h"
#include "UI/T66CasinoOverlayWidget.h"
#include "UI/T66CowardicePromptWidget.h"
#include "UI/T66IdolAltarOverlayWidget.h"
#include "UI/T66WeaponAltarOverlayWidget.h"
#include "UI/T66CollectorOverlayWidget.h"
#include "UI/T66LoadingScreenWidget.h"
#include "UI/T66CrateOverlayWidget.h"
#include "Gameplay/T66FountainInteractable.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66LootWheelInteractable.h"
#include "Gameplay/T66CasinoInteractable.h"
#include "Gameplay/T66VendorInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66WorldInteractableBase.h"
#include "Gameplay/T66TutorialGate.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/Backend/T66BackendRunSerializer.h"
#include "Core/Backend/T66BackendRunSummaryParser.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SaveMigration.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66WeaponManagerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66MediaViewerSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66WeaponAltar.h"
#include "TimerManager.h"
#include "Dom/JsonValue.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66ProjectileManagerSubsystem.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66VendorBoss.h"
#include "Gameplay/T66HeroProjectile.h"
#include "Gameplay/T66TemporaryProjectileSystem.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"
#include "Gameplay/Enemies/Projectiles/T66SpitProjectile.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Gameplay/T66LootBagPickup.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66BossGroundAOE.h"
#include "Gameplay/T66Hero1AxeAOEVFXLabActor.h"
#include "Gameplay/T66HeroPlagueCloud.h"
#include "PerformanceSystem/T66MobLootStressHarnessActor.h"
#include "PerformanceSystem/T66OutgoingTravelerStressHarnessActor.h"
#include "Gameplay/Enemies/T66RangedEnemy.h"
#include "Gameplay/Traps/T66FloorFlameTrap.h"
#include "Gameplay/Traps/T66FloorSpikePatchTrap.h"
#include "Gameplay/Traps/T66TrapArrowProjectile.h"
#include "Gameplay/Traps/T66TrapPressurePlate.h"
#include "Data/T66DataTypes.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraActor.h"
#include "Dom/JsonObject.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "Engine/GameViewportClient.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Styling/CoreStyle.h"

#if WITH_EDITOR
#include "Animation/AnimData/IAnimationDataModel.h"
#endif

namespace
{
	const FName T66GameplayAutomationTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	const FName T66GameplayAutomationTraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
	const FName T66GameplayAutomationCameraWallVisualTag(TEXT("T66_CameraOccludingWallVisual"));
	const FName T66GameplayAutomationTowerCeilingTag(TEXT("T66_Tower_Ceiling"));

	static bool T66IsModeOnlyGameplayAutomationCapture(const FString& CaptureMode)
	{
		const FString NormalizedMode = CaptureMode.ToLower();
		return NormalizedMode == TEXT("bosspartownershipa1")
			|| NormalizedMode == TEXT("bosspartownershipa2")
			|| NormalizedMode == TEXT("bossattackdefinitionproof")
			|| NormalizedMode == TEXT("bosspartmovementb1")
			|| NormalizedMode == TEXT("bossmovementb2")
			|| NormalizedMode == TEXT("runsummaryroundtrip");
	}

	static TAutoConsoleVariable<float> CVarT66AutoCaptureHeroHPOverride(
		TEXT("T66.AutoCapture.HeroHPOverride"),
		0.0f,
		TEXT("Automation-only measurement hook. When >0 and an autocapture performance mode is active, starts the hero at this HP. Default 0 has no effect."),
#if UE_BUILD_SHIPPING
		ECVF_ReadOnly
#else
		ECVF_Default
#endif
	);

#if !UE_BUILD_SHIPPING
	static FString T66SmokeJsonEscape(FString Value)
	{
		Value.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
		Value.ReplaceInline(TEXT("\""), TEXT("\\\""));
		Value.ReplaceInline(TEXT("\r"), TEXT("\\r"));
		Value.ReplaceInline(TEXT("\n"), TEXT("\\n"));
		return Value;
	}

	static void T66AppendSmokeCheck(TArray<FString>& OutChecks, bool& bAllPassed, const TCHAR* Name, const bool bPassed, const FString& Detail)
	{
		bAllPassed = bAllPassed && bPassed;
		OutChecks.Add(FString::Printf(
			TEXT("    { \"name\": \"%s\", \"ok\": %s, \"detail\": \"%s\" }"),
			Name,
			bPassed ? TEXT("true") : TEXT("false"),
			*T66SmokeJsonEscape(Detail)));
	}

	static int32 T66SmokeRarityIndex(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black: return 0;
		case ET66ItemRarity::Red: return 1;
		case ET66ItemRarity::Yellow: return 2;
		case ET66ItemRarity::White: return 3;
		default: return 0;
		}
	}

	static ET66ItemRarity T66SmokeUpgradeRarityByMultiplier(const ET66ItemRarity BaseRarity, const float Multiplier)
	{
		const int32 BaseIndex = T66SmokeRarityIndex(BaseRarity);
		const int32 BonusTiers = FMath::Clamp(FMath::FloorToInt(FMath::Max(1.f, Multiplier) - 1.f), 0, 3);
		switch (FMath::Clamp(BaseIndex + BonusTiers, 0, 3))
		{
		case 1: return ET66ItemRarity::Red;
		case 2: return ET66ItemRarity::Yellow;
		case 3: return ET66ItemRarity::White;
		default: return ET66ItemRarity::Black;
		}
	}

	static void T66RunItemTaxonomySmoke(AT66PlayerController* PC, const FString& OutputPath)
	{
		bool bAllPassed = true;
		TArray<FString> Checks;
		UWorld* World = PC ? PC->GetWorld() : nullptr;
		UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Context available"), World && GI && RunState, TEXT("World, GameInstance, and RunState are required."));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;

		auto SpawnBoss = [&]() -> AT66BossBase*
		{
			if (!World)
			{
				return nullptr;
			}
			AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
				AT66BossBase::StaticClass(),
				FVector(50000.f, 0.f, 100.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (Boss)
			{
				Boss->BossID = FName(TEXT("ItemSmokeBoss"));
				Boss->MaxHP = 1000;
				Boss->ForceAwaken();
			}
			return Boss;
		};

		auto SpawnMiniBossEnemy = [&](const float OffsetY) -> AT66EnemyBase*
		{
			if (!World)
			{
				return nullptr;
			}
			AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
				AT66EnemyBase::StaticClass(),
				FVector(50100.f, OffsetY, 100.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (Enemy)
			{
				Enemy->MaxHP = 100;
				Enemy->CurrentHP = 100;
				Enemy->bIsMiniBoss = true;
			}
			return Enemy;
		};

		auto SpawnMiniBossMob = [&](const float OffsetY) -> AT66MobBase*
		{
			if (!World)
			{
				return nullptr;
			}
			AT66MobBase* Mob = World->SpawnActor<AT66MobBase>(
				AT66MobBase::StaticClass(),
				FVector(50200.f, OffsetY, 100.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (Mob)
			{
				Mob->MaxHP = 100.f;
				Mob->CurrentHP = 100.f;
				Mob->bIsMiniBoss = true;
				Mob->LifecycleState = ET66MobLifecycleState::Active;
			}
			return Mob;
		};

		const FName OHKOSources[] =
		{
			FName(TEXT("Execute")),
			FName(TEXT("Assassinate")),
			FName(TEXT("Crush")),
		};

		for (int32 Index = 0; Index < UE_ARRAY_COUNT(OHKOSources); ++Index)
		{
			const FName SourceID = OHKOSources[Index];
			AT66BossBase* Boss = SpawnBoss();
			const bool bBossRejected = Boss
				&& !T66CombatShared::TryApplyNonBossOHKO(Boss, nullptr, SourceID, FName(TEXT("ItemTaxonomySmoke")))
				&& Boss->CurrentHP == Boss->MaxHP;
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				*FString::Printf(TEXT("%s rejects boss"), *SourceID.ToString()),
				bBossRejected,
				Boss ? FString::Printf(TEXT("Boss HP=%d/%d."), Boss->CurrentHP, Boss->MaxHP) : TEXT("Boss spawn failed."));

			AT66EnemyBase* Enemy = SpawnMiniBossEnemy(static_cast<float>(Index) * 140.f);
			const bool bEnemyExecuted = Enemy
				&& T66CombatShared::TryApplyNonBossOHKO(Enemy, nullptr, SourceID, FName(TEXT("ItemTaxonomySmoke")))
				&& Enemy->CurrentHP <= 0;
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				*FString::Printf(TEXT("%s allows miniboss enemy"), *SourceID.ToString()),
				bEnemyExecuted,
				Enemy ? FString::Printf(TEXT("Enemy HP=%d/%d MiniBoss=%d."), Enemy->CurrentHP, Enemy->MaxHP, Enemy->bIsMiniBoss ? 1 : 0) : TEXT("Enemy spawn failed."));

			AT66MobBase* Mob = SpawnMiniBossMob(static_cast<float>(Index) * 140.f);
			const bool bMobExecuted = Mob
				&& T66CombatShared::TryApplyNonBossOHKO(Mob, nullptr, SourceID, FName(TEXT("ItemTaxonomySmoke")))
				&& Mob->CurrentHP <= 0.f;
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				*FString::Printf(TEXT("%s allows miniboss mob"), *SourceID.ToString()),
				bMobExecuted,
				Mob ? FString::Printf(TEXT("Mob HP=%.1f/%.1f MiniBoss=%d."), Mob->CurrentHP, Mob->MaxHP, Mob->bIsMiniBoss ? 1 : 0) : TEXT("Mob spawn failed."));
		}

		AT66BossBase* CounterBoss = SpawnBoss();
		const int32 CounterBossStartHP = CounterBoss ? CounterBoss->CurrentHP : 0;
		if (CounterBoss)
		{
			CounterBoss->TakeDamageFromHeroHit(5, FName(TEXT("CounterAttack")), FName(TEXT("ItemTaxonomySmoke")));
		}
		const bool bCounterBossDamaged = CounterBoss
			&& CounterBoss->CurrentHP > 0
			&& CounterBoss->CurrentHP < CounterBossStartHP;
		T66AppendSmokeCheck(
			Checks,
			bAllPassed,
			TEXT("Counter damages boss non-lethally"),
			bCounterBossDamaged,
			CounterBoss ? FString::Printf(TEXT("Boss HP %d -> %d."), CounterBossStartHP, CounterBoss->CurrentHP) : TEXT("Boss spawn failed."));

		AT66BossBase* ReflectBoss = SpawnBoss();
		const int32 ReflectBossStartHP = ReflectBoss ? ReflectBoss->CurrentHP : 0;
		if (ReflectBoss)
		{
			ReflectBoss->TakeDamageFromHeroHit(5, FName(TEXT("Reflect")), FName(TEXT("ItemTaxonomySmoke")));
		}
		const bool bReflectBossDamaged = ReflectBoss
			&& ReflectBoss->CurrentHP > 0
			&& ReflectBoss->CurrentHP < ReflectBossStartHP;
		T66AppendSmokeCheck(
			Checks,
			bAllPassed,
			TEXT("Reflect damages boss non-lethally"),
			bReflectBossDamaged,
			ReflectBoss ? FString::Printf(TEXT("Boss HP %d -> %d."), ReflectBossStartHP, ReflectBoss->CurrentHP) : TEXT("Boss spawn failed."));

		if (RunState)
		{
			RunState->AddItemSlot(FT66InventorySlot(FName(TEXT("Item_InteractableLuck")), ET66ItemRarity::White, 40));
			RunState->AddItemSlot(FT66InventorySlot(FName(TEXT("Item_ProcLuck")), ET66ItemRarity::White, 40));

			const float LootBagMultiplier = RunState->GetLootBagRewardMultiplier();
			const float LootWheelMultiplier = RunState->GetLootWheelRewardMultiplier();
			const bool bLootBagImproves = LootBagMultiplier > 1.f
				&& T66SmokeRarityIndex(T66SmokeUpgradeRarityByMultiplier(ET66ItemRarity::Black, LootBagMultiplier)) > T66SmokeRarityIndex(ET66ItemRarity::Black);
			const bool bLootWheelImproves = LootWheelMultiplier > 1.f
				&& T66SmokeRarityIndex(T66SmokeUpgradeRarityByMultiplier(ET66ItemRarity::Black, LootWheelMultiplier)) > T66SmokeRarityIndex(ET66ItemRarity::Black);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Interactable Luck improves interactable reward"),
				bLootBagImproves,
				FString::Printf(TEXT("InteractableLuckMultiplier=%.3f."), LootBagMultiplier));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Proc Luck retired interactable alias remains compatible"),
				bLootWheelImproves,
				FString::Printf(TEXT("LegacyLootWheelMultiplier=%.3f."), LootWheelMultiplier));

			const int32 InventoryBeforeRetired = RunState->GetInventorySlots().Num();
			const FName RetiredItems[] =
			{
				FName(TEXT("Item_Accuracy")),
				FName(TEXT("Item_Cheating")),
				FName(TEXT("Item_Stealing")),
				FName(TEXT("Item_LootCrate")),
				FName(TEXT("Item_TreasureChest")),
				FName(TEXT("Item_LootBag")),
				FName(TEXT("Item_LootWheel")),
				FName(TEXT("Item_HpRegen")),
				FName(TEXT("Item_LifeSteal")),
				FName(TEXT("Item_CritDamage")),
			};
			for (const FName RetiredItem : RetiredItems)
			{
				RunState->AddItem(RetiredItem);
			}
			const int32 InventoryAfterRetired = RunState->GetInventorySlots().Num();
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Retired item IDs skip inventory"),
				InventoryBeforeRetired == InventoryAfterRetired,
				FString::Printf(TEXT("Inventory %d -> %d."), InventoryBeforeRetired, InventoryAfterRetired));
		}

		const FString Json = FString::Printf(
			TEXT("{\n  \"ok\": %s,\n  \"checks\": [\n%s\n  ]\n}\n"),
			bAllPassed ? TEXT("true") : TEXT("false"),
			*FString::Join(Checks, TEXT(",\n")));

		IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);
		const bool bSaved = FFileHelper::SaveStringToFile(Json, *OutputPath);
		UE_LOG(LogTemp, Display, TEXT("[ItemTaxonomySmoke] ok=%d saved=%d output=%s"), bAllPassed ? 1 : 0, bSaved ? 1 : 0, *OutputPath);
		FPlatformMisc::RequestExitWithStatus(false, (bAllPassed && bSaved) ? 0 : 70, TEXT("T66ItemTaxonomySmokeComplete"));
	}

	static void T66RunStatPipelineSmoke(AT66PlayerController* PC, const FString& OutputPath)
	{
		bool bAllPassed = true;
		TArray<FString> Checks;
		UWorld* World = PC ? PC->GetWorld() : nullptr;
		UT66GameInstance* GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		UT66BuffSubsystem* Buffs = GI ? GI->GetSubsystem<UT66BuffSubsystem>() : nullptr;
		UT66WeaponManagerSubsystem* WeaponManager = GI ? GI->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr;
		APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(PlayerPawn);

		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Context available"), World && GI && RunState && Buffs && WeaponManager && PlayerPawn, TEXT("World, GI, RunState, Buffs, WeaponManager, and pawn are required."));

		auto SumDamageSecondaries = [RunState]() -> int32
		{
			return RunState
				? RunState->GetAoeDmgStat() + RunState->GetBounceDmgStat() + RunState->GetPierceDmgStat() + RunState->GetDotDmgStat()
				: 0;
		};

		auto SumPrimaryStats = [RunState]() -> int32
		{
			return RunState
				? RunState->GetDamageStat() + RunState->GetAttackSpeedStat() + RunState->GetScaleStat() + RunState->GetAccuracyStat()
					+ RunState->GetArmorStat() + RunState->GetEvasionStat() + RunState->GetLuckStat() + RunState->GetSpeedStat()
				: 0;
		};

		auto SumTrackedSecondaryStats = [RunState]() -> int32
		{
			return RunState
				? RunState->GetAoeDmgStat() + RunState->GetBounceDmgStat() + RunState->GetPierceDmgStat() + RunState->GetDotDmgStat()
					+ RunState->GetAoeAtkSpdStat() + RunState->GetBounceAtkSpdStat() + RunState->GetPierceAtkSpdStat() + RunState->GetDotAtkSpdStat()
					+ RunState->GetAoeAtkScaleStat() + RunState->GetBounceAtkScaleStat() + RunState->GetPierceAtkScaleStat() + RunState->GetDotAtkScaleStat()
				: 0;
		};

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;

		auto SpawnRichEnemy = [&](const FVector& Location, const int32 XPValue) -> AT66EnemyBase*
		{
			AT66EnemyBase* Enemy = World ? World->SpawnActor<AT66EnemyBase>(AT66EnemyBase::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams) : nullptr;
			if (Enemy)
			{
				Enemy->MaxHP = 1;
				Enemy->CurrentHP = 1;
				Enemy->XPValue = FMath::Max(0, XPValue);
				Enemy->bDropsLoot = false;
			}
			return Enemy;
		};

		auto SpawnLightweightMob = [&](const FVector& Location, const int32 XPValue) -> AT66MobBase*
		{
			AT66MobBase* Mob = World ? World->SpawnActor<AT66MobBase>(AT66MobBase::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams) : nullptr;
			if (Mob)
			{
				Mob->MaxHP = 1.f;
				Mob->CurrentHP = 1.f;
				Mob->XPValue = FMath::Max(0, XPValue);
				Mob->LifecycleState = ET66MobLifecycleState::Active;
			}
			return Mob;
		};

		auto SpawnSmokeBoss = [&](const FVector& Location) -> AT66BossBase*
		{
			AT66BossBase* Boss = World ? World->SpawnActor<AT66BossBase>(AT66BossBase::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams) : nullptr;
			if (Boss)
			{
				Boss->BossID = FName(TEXT("StatSmokeBoss"));
				Boss->MaxHP = 1000;
				Boss->ForceAwaken();
			}
			return Boss;
		};

		if (RunState)
		{
			RunState->ResetForNewRun();
			RunState->ClearInventory();
			const int32 DamageBeforeItem = RunState->GetDamageStat();
			const int32 AoeDamageBeforeItem = RunState->GetAoeDmgStat();
			RunState->AddItemSlot(FT66InventorySlot(FName(TEXT("Item_AoeDamage")), ET66ItemRarity::White, 99));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Items do not raise primary stats"),
				RunState->GetDamageStat() == DamageBeforeItem,
				FString::Printf(TEXT("Damage %d -> %d."), DamageBeforeItem, RunState->GetDamageStat()));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Items raise secondary stats"),
				RunState->GetAoeDmgStat() > AoeDamageBeforeItem,
				FString::Printf(TEXT("AOE damage %d -> %d."), AoeDamageBeforeItem, RunState->GetAoeDmgStat()));
		}

		if (RunState && GI)
		{
			FItemData DeprecatedCritDamageItemData;
			FItemData HeadshotItemData;
			const bool bHasDeprecatedCritDamageItem = GI->GetItemData(FName(TEXT("Item_CritDamage")), DeprecatedCritDamageItemData);
			const bool bHasHeadshotItem = GI->GetItemData(FName(TEXT("Item_Headshot")), HeadshotItemData);
			const bool bHeadshotItemLive = bHasHeadshotItem
				&& HeadshotItemData.PrimaryStatType == ET66HeroStatType::Accuracy
				&& HeadshotItemData.SecondaryStatType == ET66SecondaryStatType::HeadshotChance
				&& T66IsLiveSecondaryStatType(HeadshotItemData.SecondaryStatType);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Headshot item replaces Crit Damage item"),
				bHeadshotItemLive && !bHasDeprecatedCritDamageItem,
				FString::Printf(TEXT("HasHeadshot=%d HeadshotSecondary=%d HasOldCritDamage=%d."), bHasHeadshotItem ? 1 : 0, bHasHeadshotItem ? static_cast<int32>(HeadshotItemData.SecondaryStatType) : -1, bHasDeprecatedCritDamageItem ? 1 : 0));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Crit damage fixed at 2x and deprecated"),
				FMath::IsNearlyEqual(RunState->GetCritDamageMultiplier(), 2.0f) && !T66IsLiveSecondaryStatType(ET66SecondaryStatType::CritDamage) && T66IsLiveSecondaryStatType(ET66SecondaryStatType::HeadshotChance),
				FString::Printf(TEXT("CritMult=%.3f CritDamageLive=%d HeadshotLive=%d."), RunState->GetCritDamageMultiplier(), T66IsLiveSecondaryStatType(ET66SecondaryStatType::CritDamage) ? 1 : 0, T66IsLiveSecondaryStatType(ET66SecondaryStatType::HeadshotChance) ? 1 : 0));
		}

		if (RunState && Buffs)
		{
			RunState->ResetForNewRun();
			RunState->ClearInventory();
			const float HeadshotBeforeItem = RunState->GetHeadshotChance01();
			RunState->AddItemSlot(FT66InventorySlot(FName(TEXT("Item_Headshot")), ET66ItemRarity::White, 99, 0.f, 40));
			const float HeadshotAfterItem = RunState->GetHeadshotChance01();
			Buffs->DebugGrantSingleUseBuff(ET66SecondaryStatType::HeadshotChance, 1, true);
			RunState->DebugActivatePendingSingleUseBuffsForRunStartWithoutConsuming();
			const float HeadshotAfterDrug = RunState->GetHeadshotChance01();
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Headshot item raises Headshot Chance"),
				HeadshotAfterItem > HeadshotBeforeItem,
				FString::Printf(TEXT("HeadshotChance %.3f -> %.3f."), HeadshotBeforeItem, HeadshotAfterItem));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Headshot drug multiplies Headshot Chance"),
				HeadshotAfterDrug > HeadshotAfterItem,
				FString::Printf(TEXT("HeadshotChance %.3f -> %.3f."), HeadshotAfterItem, HeadshotAfterDrug));

			const bool bDeprecatedCritDamageBuffSkipped = !UT66BuffSubsystem::GetAllSingleUseBuffTypes().Contains(ET66SecondaryStatType::CritDamage)
				&& Buffs->GetOwnedSingleUseBuffCount(ET66SecondaryStatType::CritDamage) == 0
				&& Buffs->GetSelectedSingleUseBuffSlotAssignedCountForStat(ET66SecondaryStatType::CritDamage) == 0;
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Deprecated CritDamage buff type is skipped"),
				bDeprecatedCritDamageBuffSkipped,
				FString::Printf(TEXT("Listed=%d Owned=%d Selected=%d."), UT66BuffSubsystem::GetAllSingleUseBuffTypes().Contains(ET66SecondaryStatType::CritDamage) ? 1 : 0, Buffs->GetOwnedSingleUseBuffCount(ET66SecondaryStatType::CritDamage), Buffs->GetSelectedSingleUseBuffSlotAssignedCountForStat(ET66SecondaryStatType::CritDamage)));
		}

		if (RunState && World && HeroPawn && HeroPawn->CombatComponent)
		{
			RunState->ResetForNewRun();
			RunState->ClearInventory();
			const FVector BaseLocation = PlayerPawn->GetActorLocation();
			AT66MobBase* HeadshotTarget = SpawnLightweightMob(BaseLocation + FVector(250.f, 0.f, 100.f), 0);
			if (HeadshotTarget)
			{
				HeadshotTarget->MaxHP = 100000.f;
				HeadshotTarget->CurrentHP = 100000.f;
				for (int32 Index = 0; Index < 24; ++Index)
				{
					RunState->AddItemSlot(FT66InventorySlot(FName(TEXT("Item_Headshot")), ET66ItemRarity::White, 99, 0.f, 40));
				}
			}
			const float ForcedHeadshotChance = RunState->GetHeadshotChance01();
			const float HeadshotStunDuration = RunState->GetHeadshotStunDurationSeconds();
			const bool bDebugHeadshotApplied = HeadshotTarget
				&& HeroPawn->CombatComponent->DebugApplyHeadshotStunForAutomation(HeadshotTarget, true);
			const bool bHeadshotStunned = HeadshotTarget
				&& ForcedHeadshotChance > 0.f
				&& bDebugHeadshotApplied
				&& HeadshotTarget->StunSecondsRemaining > 0.f;
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Headshot Chance can stun a hit target"),
				bHeadshotStunned,
				HeadshotTarget ? FString::Printf(TEXT("Chance=%.3f Duration=%.3f Applied=%d StunRemaining=%.3f."), ForcedHeadshotChance, HeadshotStunDuration, bDebugHeadshotApplied ? 1 : 0, HeadshotTarget->StunSecondsRemaining) : TEXT("Headshot target spawn failed."));

			AT66BossBase* HeadshotBossTarget = SpawnSmokeBoss(BaseLocation + FVector(450.f, 0.f, 100.f));
			const bool bHeadshotBossStunApplied = HeadshotBossTarget
				&& HeroPawn->CombatComponent->DebugApplyHeadshotStunForAutomation(HeadshotBossTarget, true);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Headshot Chance stuns boss targets"),
				bHeadshotBossStunApplied,
				HeadshotBossTarget ? FString::Printf(TEXT("Applied=%d BossAlive=%d Duration=%.3f."), bHeadshotBossStunApplied ? 1 : 0, HeadshotBossTarget->IsAlive() ? 1 : 0, HeadshotStunDuration) : TEXT("Headshot boss target spawn failed."));

		}

		{
			TSharedPtr<FJsonObject> LegacySummaryJson = MakeShared<FJsonObject>();
			TSharedPtr<FJsonObject> LegacySecondaryJson = MakeShared<FJsonObject>();
			LegacySecondaryJson->SetNumberField(TEXT("CritDamage"), 0.42);
			LegacySummaryJson->SetObjectField(TEXT("secondary_stats"), LegacySecondaryJson);
			UT66LeaderboardRunSummarySaveGame* ParsedLegacySummary = T66BackendRunSummaryParser::Parse(LegacySummaryJson, GI);
			const bool bLegacyCritDamageMapsToHeadshot = ParsedLegacySummary
				&& ParsedLegacySummary->SecondaryStatValues.Contains(ET66SecondaryStatType::HeadshotChance)
				&& !ParsedLegacySummary->SecondaryStatValues.Contains(ET66SecondaryStatType::CritDamage)
				&& FMath::IsNearlyEqual(ParsedLegacySummary->SecondaryStatValues.FindRef(ET66SecondaryStatType::HeadshotChance), 0.42f);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Legacy CritDamage backend key maps to Headshot Chance"),
				bLegacyCritDamageMapsToHeadshot,
				ParsedLegacySummary ? FString::Printf(TEXT("Headshot=%.3f CritDamagePresent=%d."), ParsedLegacySummary->SecondaryStatValues.FindRef(ET66SecondaryStatType::HeadshotChance), ParsedLegacySummary->SecondaryStatValues.Contains(ET66SecondaryStatType::CritDamage) ? 1 : 0) : TEXT("Parser returned null."));

			TSharedPtr<FJsonObject> LegacyMultiplierSummaryJson = MakeShared<FJsonObject>();
			TSharedPtr<FJsonObject> LegacyMultiplierSecondaryJson = MakeShared<FJsonObject>();
			LegacyMultiplierSecondaryJson->SetNumberField(TEXT("CritDamage"), 1.5);
			LegacyMultiplierSummaryJson->SetObjectField(TEXT("secondary_stats"), LegacyMultiplierSecondaryJson);
			UT66LeaderboardRunSummarySaveGame* ParsedLegacyMultiplierSummary = T66BackendRunSummaryParser::Parse(LegacyMultiplierSummaryJson, GI);
			const bool bLegacyCritDamageMultiplierIgnored = ParsedLegacyMultiplierSummary
				&& !ParsedLegacyMultiplierSummary->SecondaryStatValues.Contains(ET66SecondaryStatType::HeadshotChance)
				&& !ParsedLegacyMultiplierSummary->SecondaryStatValues.Contains(ET66SecondaryStatType::CritDamage);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Legacy CritDamage multiplier does not become Headshot Chance"),
				bLegacyCritDamageMultiplierIgnored,
				ParsedLegacyMultiplierSummary ? FString::Printf(TEXT("HeadshotPresent=%d CritDamagePresent=%d."), ParsedLegacyMultiplierSummary->SecondaryStatValues.Contains(ET66SecondaryStatType::HeadshotChance) ? 1 : 0, ParsedLegacyMultiplierSummary->SecondaryStatValues.Contains(ET66SecondaryStatType::CritDamage) ? 1 : 0) : TEXT("Parser returned null."));

			TSharedPtr<FJsonObject> LegacyBoundarySummaryJson = MakeShared<FJsonObject>();
			TSharedPtr<FJsonObject> LegacyBoundarySecondaryJson = MakeShared<FJsonObject>();
			LegacyBoundarySecondaryJson->SetNumberField(TEXT("CritDamage"), 1.0);
			LegacyBoundarySummaryJson->SetObjectField(TEXT("secondary_stats"), LegacyBoundarySecondaryJson);
			UT66LeaderboardRunSummarySaveGame* ParsedLegacyBoundarySummary = T66BackendRunSummaryParser::Parse(LegacyBoundarySummaryJson, GI);
			const bool bLegacyCritDamageBoundaryMapped = ParsedLegacyBoundarySummary
				&& ParsedLegacyBoundarySummary->SecondaryStatValues.Contains(ET66SecondaryStatType::HeadshotChance)
				&& !ParsedLegacyBoundarySummary->SecondaryStatValues.Contains(ET66SecondaryStatType::CritDamage)
				&& FMath::IsNearlyEqual(ParsedLegacyBoundarySummary->SecondaryStatValues.FindRef(ET66SecondaryStatType::HeadshotChance), 1.0f);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Legacy CritDamage 1.0 boundary maps intentionally"),
				bLegacyCritDamageBoundaryMapped,
				ParsedLegacyBoundarySummary ? FString::Printf(TEXT("Headshot=%.3f CritDamagePresent=%d."), ParsedLegacyBoundarySummary->SecondaryStatValues.FindRef(ET66SecondaryStatType::HeadshotChance), ParsedLegacyBoundarySummary->SecondaryStatValues.Contains(ET66SecondaryStatType::CritDamage) ? 1 : 0) : TEXT("Parser returned null."));

			UT66LeaderboardRunSummarySaveGame* SerializationSnapshot = NewObject<UT66LeaderboardRunSummarySaveGame>();
			if (SerializationSnapshot)
			{
				SerializationSnapshot->SecondaryStatValues.Add(ET66SecondaryStatType::HeadshotChance, 0.25f);
				SerializationSnapshot->SecondaryStatValues.Add(ET66SecondaryStatType::CritDamage, 2.0f);
			}
			const TSharedPtr<FJsonObject> SerializedRunJson = SerializationSnapshot
				? T66BackendRunSerializer::BuildRunJsonObject(TEXT("Hero_1"), TEXT("None"), ET66Difficulty::Easy, ET66PartySize::Solo, 1, 0, 0, SerializationSnapshot)
				: nullptr;
			const TSharedPtr<FJsonObject>* SerializedSecondaryStats = nullptr;
			double SerializedHeadshotValue = 0.0;
			const bool bHeadshotStringSerialized = SerializedRunJson.IsValid()
				&& SerializedRunJson->TryGetObjectField(TEXT("secondary_stats"), SerializedSecondaryStats)
				&& SerializedSecondaryStats
				&& (*SerializedSecondaryStats).IsValid()
				&& (*SerializedSecondaryStats)->TryGetNumberField(TEXT("HeadshotChance"), SerializedHeadshotValue)
				&& !(*SerializedSecondaryStats)->HasField(TEXT("CritDamage"))
				&& !(*SerializedSecondaryStats)->HasField(FString::FromInt(static_cast<int32>(ET66SecondaryStatType::HeadshotChance)))
				&& FMath::IsNearlyEqual(static_cast<float>(SerializedHeadshotValue), 0.25f);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Headshot Chance serializes by string key"),
				bHeadshotStringSerialized,
				SerializedSecondaryStats && (*SerializedSecondaryStats).IsValid()
					? FString::Printf(TEXT("HasHeadshot=%d HasCritDamage=%d HasOrdinal=%d Value=%.3f."), (*SerializedSecondaryStats)->HasField(TEXT("HeadshotChance")) ? 1 : 0, (*SerializedSecondaryStats)->HasField(TEXT("CritDamage")) ? 1 : 0, (*SerializedSecondaryStats)->HasField(FString::FromInt(static_cast<int32>(ET66SecondaryStatType::HeadshotChance))) ? 1 : 0, static_cast<float>(SerializedHeadshotValue))
					: TEXT("Serialized secondary_stats object missing."));

			UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
			const FString HeadshotLabel = Loc ? Loc->GetText_SecondaryStatName(ET66SecondaryStatType::HeadshotChance).ToString() : FString();
			const FString DeprecatedCritDamageLabel = Loc ? Loc->GetText_SecondaryStatName(ET66SecondaryStatType::CritDamage).ToString() : FString();
			const FString DeprecatedCritDamageTooltip = Loc ? Loc->GetText_SecondaryStatDescription(ET66SecondaryStatType::CritDamage).ToString() : FString();
			const bool bCritDamageUiLabelRetired = Loc
				&& HeadshotLabel == TEXT("Headshot Chance")
				&& DeprecatedCritDamageLabel == TEXT("Headshot Chance")
				&& !DeprecatedCritDamageTooltip.Contains(TEXT("Critical"), ESearchCase::IgnoreCase)
				&& !DeprecatedCritDamageTooltip.Contains(TEXT("Crit Damage"), ESearchCase::IgnoreCase);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Deprecated CritDamage UI text resolves to Headshot"),
				bCritDamageUiLabelRetired,
				Loc ? FString::Printf(TEXT("HeadshotLabel='%s' DeprecatedLabel='%s' DeprecatedTooltip='%s'."), *HeadshotLabel, *DeprecatedCritDamageLabel, *DeprecatedCritDamageTooltip) : TEXT("Localization subsystem missing."));
		}

		if (RunState && Buffs)
		{
			Buffs->DebugSetDiplomaUnlockedSteps(ET66HeroStatType::Damage, 0);
			RunState->ResetForNewRun();
			const int32 DamageBeforeRelic = RunState->GetDamageStat();
			const int32 DamageSecondaryBeforeRelic = SumDamageSecondaries();

			Buffs->DebugSetDiplomaUnlockedSteps(ET66HeroStatType::Damage, 2);
			RunState->ResetForNewRun();
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Relics raise primary stats"),
				RunState->GetDamageStat() > DamageBeforeRelic,
				FString::Printf(TEXT("Damage %d -> %d."), DamageBeforeRelic, RunState->GetDamageStat()));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Relics propagate to secondary stats"),
				SumDamageSecondaries() > DamageSecondaryBeforeRelic,
				FString::Printf(TEXT("Damage-secondary sum %d -> %d."), DamageSecondaryBeforeRelic, SumDamageSecondaries()));
		}

		if (RunState && Buffs)
		{
			RunState->ResetForNewRun();
			const float AoeValueBeforeSteroid = RunState->GetSecondaryStatValue(ET66SecondaryStatType::AoeDamage);
			Buffs->DebugGrantSingleUseBuff(ET66SecondaryStatType::AoeDamage, 1, true);
			RunState->DebugActivatePendingSingleUseBuffsForRunStartWithoutConsuming();
			const float AoeValueAfterSteroid = RunState->GetSecondaryStatValue(ET66SecondaryStatType::AoeDamage);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Selected steroids multiply secondary stats"),
				AoeValueAfterSteroid > AoeValueBeforeSteroid,
				FString::Printf(TEXT("AOE value %.3f -> %.3f."), AoeValueBeforeSteroid, AoeValueAfterSteroid));
		}

		if (RunState)
		{
			RunState->ResetForNewRun();
			RunState->ApplyAutomationHeroHPOverride(1.f, TEXT("StatPipelineSmoke"));
			const int32 LevelBefore = RunState->GetHeroLevel();
			const int32 PrimaryBefore = SumPrimaryStats();
			const int32 SecondaryBefore = SumTrackedSecondaryStats();
			const int32 Threshold = RunState->GetHeroXPToNextLevel();
			RunState->AddHeroXP(Threshold);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("XP triggers level up"),
				RunState->GetHeroLevel() == LevelBefore + 1,
				FString::Printf(TEXT("Level %d -> %d threshold=%d."), LevelBefore, RunState->GetHeroLevel(), Threshold));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Level up heals to full"),
				FMath::IsNearlyEqual(RunState->GetCurrentHP(), RunState->GetMaxHP()),
				FString::Printf(TEXT("HP %.1f/%.1f."), RunState->GetCurrentHP(), RunState->GetMaxHP()));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Level up raises primary stats"),
				SumPrimaryStats() > PrimaryBefore,
				FString::Printf(TEXT("Primary sum %d -> %d."), PrimaryBefore, SumPrimaryStats()));
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Level up propagates to secondary stats"),
				SumTrackedSecondaryStats() > SecondaryBefore,
				FString::Printf(TEXT("Tracked secondary sum %d -> %d."), SecondaryBefore, SumTrackedSecondaryStats()));
		}

		if (RunState && World && PlayerPawn)
		{
			RunState->ResetForNewRun();
			const FVector BaseLocation = PlayerPawn->GetActorLocation();
			const int32 XPBeforeKills = RunState->GetHeroXP();
			const int32 LevelBeforeKills = RunState->GetHeroLevel();
			AT66EnemyBase* Enemy = SpawnRichEnemy(BaseLocation + FVector(5000.f, 0.f, 100.f), 5);
			if (Enemy)
			{
				Enemy->TakeDamageFromHero(9999, FName(TEXT("StatPipelineSmoke")), FName(TEXT("StatPipelineSmoke")));
			}
			AT66MobBase* Mob = SpawnLightweightMob(BaseLocation + FVector(5200.f, 0.f, 100.f), 7);
			if (Mob)
			{
				Mob->TakeDamageFromHeroHitZone(
					9999,
					Mob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body),
					FName(TEXT("StatPipelineSmoke")),
					FName(TEXT("StatPipelineSmoke")));
			}
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Rich and lightweight enemies grant XP"),
				RunState->GetHeroLevel() == LevelBeforeKills && RunState->GetHeroXP() == XPBeforeKills + 12,
				FString::Printf(TEXT("XP %d -> %d level %d -> %d."), XPBeforeKills, RunState->GetHeroXP(), LevelBeforeKills, RunState->GetHeroLevel()));
		}

		if (RunState && World && PlayerPawn)
		{
			RunState->ResetForNewRun();
			const FVector BaseLocation = PlayerPawn->GetActorLocation();
			const int32 Threshold = RunState->GetHeroXPToNextLevel();
			AT66EnemyBase* WaveEnemy = SpawnRichEnemy(BaseLocation + FVector(150.f, 0.f, 100.f), Threshold);
			AT66MobBase* WaveMob = SpawnLightweightMob(BaseLocation + FVector(250.f, 0.f, 100.f), Threshold);
			const int32 LevelBeforeWave = RunState->GetHeroLevel();
			RunState->AddHeroXP(Threshold);
			const bool bWaveKilledTargets = (!WaveEnemy || WaveEnemy->CurrentHP <= 0) && (!WaveMob || WaveMob->CurrentHP <= 0.f);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Level-up wave does not chain XP"),
				bWaveKilledTargets && RunState->GetHeroLevel() == LevelBeforeWave + 1,
				FString::Printf(
					TEXT("Level %d -> %d enemyHP=%d mobHP=%.1f threshold=%d."),
					LevelBeforeWave,
					RunState->GetHeroLevel(),
					WaveEnemy ? WaveEnemy->CurrentHP : -1,
					WaveMob ? WaveMob->CurrentHP : -1.f,
					Threshold));
		}

		auto FinishStatPipelineSmoke = [OutputPath](const bool bFinalAllPassed, const TArray<FString>& FinalChecks)
		{
			const FString Json = FString::Printf(
				TEXT("{\n  \"ok\": %s,\n  \"checks\": [\n%s\n  ]\n}\n"),
				bFinalAllPassed ? TEXT("true") : TEXT("false"),
				*FString::Join(FinalChecks, TEXT(",\n")));

			IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);
			const bool bSaved = FFileHelper::SaveStringToFile(Json, *OutputPath);
			UE_LOG(LogTemp, Display, TEXT("[StatPipelineSmoke] ok=%d saved=%d output=%s"), bFinalAllPassed ? 1 : 0, bSaved ? 1 : 0, *OutputPath);
			FPlatformMisc::RequestExitWithStatus(false, (bFinalAllPassed && bSaved) ? 0 : 71, TEXT("T66StatPipelineSmokeComplete"));
		};

		auto AppendLiveAutoAttackCheck = [](TArray<FString>& InOutChecks, bool& bInOutAllPassed, const TWeakObjectPtr<AT66MobBase>& TargetWeak, const ET66AttackCategory Category, const float Chance)
		{
			AT66MobBase* Target = TargetWeak.Get();
			T66AppendSmokeCheck(
				InOutChecks,
				bInOutAllPassed,
				TEXT("Live auto-attack path rolls Headshot stun"),
				Target && Chance >= 0.95f && Target->StunSecondsRemaining > 0.f,
				Target ? FString::Printf(TEXT("Category=%d Chance=%.3f StunRemaining=%.3f HP=%.1f."), static_cast<int32>(Category), Chance, Target->StunSecondsRemaining, Target->CurrentHP) : TEXT("Live auto-attack target missing before delayed proof completed."));
		};

		TWeakObjectPtr<AT66MobBase> LiveAutoAttackHeadshotTarget;
		float LiveAutoAttackHeadshotChance = 0.f;
		ET66AttackCategory AutoAttackProofCategory = ET66AttackCategory::Pierce;
		float LiveAutoAttackProofDelaySeconds = 0.f;

		if (RunState && World && HeroPawn && HeroPawn->CombatComponent && WeaponManager && GI)
		{
			RunState->ResetForNewRun();
			RunState->ClearInventory();
			const FVector BaseLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
			AT66MobBase* SpawnedLiveTarget = SpawnLightweightMob(BaseLocation + FVector(300.f, 120.f, 100.f), 0);
			LiveAutoAttackHeadshotTarget = SpawnedLiveTarget;
			if (SpawnedLiveTarget)
			{
				SpawnedLiveTarget->MaxHP = 100000.f;
				SpawnedLiveTarget->CurrentHP = 100000.f;
			}
			for (int32 Index = 0; Index < 120 && RunState->GetHeadshotChance01() < 0.95f; ++Index)
			{
				RunState->AddItemSlot(FT66InventorySlot(FName(TEXT("Item_Headshot")), ET66ItemRarity::White, 99, 0.f, 40));
			}
			RunState->DebugAddPersistentSecondaryStatBonusTenths(
				ET66SecondaryStatType::HeadshotChance,
				UT66RunStateSubsystem::MaxHeroStatValue * UT66RunStateSubsystem::HeroStatTenthsScale);
			if (Buffs)
			{
				Buffs->DebugGrantSingleUseBuff(ET66SecondaryStatType::HeadshotChance, UT66BuffSubsystem::MaxSelectedSingleUseBuffs, true);
				RunState->DebugActivatePendingSingleUseBuffsForRunStartWithoutConsuming();
			}
			LiveAutoAttackHeadshotChance = RunState->GetHeadshotChance01();
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Headshot Chance clamps to proc cap"),
				FMath::IsNearlyEqual(LiveAutoAttackHeadshotChance, 0.95f),
				FString::Printf(TEXT("HeadshotChance=%.3f after capped inventory, persistent secondary bonus, and selected steroid multiplier."), LiveAutoAttackHeadshotChance));
			const FName AutoAttackProofHeroID = !HeroPawn->HeroID.IsNone() ? HeroPawn->HeroID : FName(TEXT("Hero_1"));
			FHeroData AutoAttackProofHeroData;
			if (GI->GetHeroData(AutoAttackProofHeroID, AutoAttackProofHeroData))
			{
				AutoAttackProofCategory = AutoAttackProofHeroData.PrimaryCategory;
				LiveAutoAttackProofDelaySeconds = (AutoAttackProofCategory == ET66AttackCategory::AOE && AutoAttackProofHeroData.AoeDelay > 0.f)
					? FMath::Clamp(AutoAttackProofHeroData.AoeDelay + 0.25f, 0.25f, 2.0f)
					: 0.f;
			}
			const FName AutoAttackProofWeaponID = UT66WeaponManagerSubsystem::MakeWeaponID(AutoAttackProofHeroID, ET66WeaponRarity::Black, AutoAttackProofCategory);
			const bool bSelectedAutoAttackProofWeapon = WeaponManager->SelectWeapon(AutoAttackProofWeaponID);
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Auto-attack proof weapon selected"),
				bSelectedAutoAttackProofWeapon,
				FString::Printf(TEXT("WeaponID=%s Selected=%d."), *AutoAttackProofWeaponID.ToString(), bSelectedAutoAttackProofWeapon ? 1 : 0));
			if (SpawnedLiveTarget)
			{
				HeroPawn->CombatComponent->SetAutoAttackSuppressed(false);
				HeroPawn->CombatComponent->SetLockedTarget(SpawnedLiveTarget);
				HeroPawn->CombatComponent->PerformAutomationAutoAttackNow();
				HeroPawn->CombatComponent->ClearLockedTarget();
			}
		}
		else
		{
			T66AppendSmokeCheck(
				Checks,
				bAllPassed,
				TEXT("Live auto-attack path rolls Headshot stun"),
				false,
				TEXT("World, hero, combat component, weapon manager, run state, or game instance missing."));
		}

		if (LiveAutoAttackProofDelaySeconds > KINDA_SMALL_NUMBER && World)
		{
			FTimerHandle DelayedSmokeFinishHandle;
			World->GetTimerManager().SetTimer(
				DelayedSmokeFinishHandle,
				[Checks, bAllPassed, FinishStatPipelineSmoke, AppendLiveAutoAttackCheck, LiveAutoAttackHeadshotTarget, LiveAutoAttackHeadshotChance, AutoAttackProofCategory]() mutable
				{
					AppendLiveAutoAttackCheck(Checks, bAllPassed, LiveAutoAttackHeadshotTarget, AutoAttackProofCategory, LiveAutoAttackHeadshotChance);
					FinishStatPipelineSmoke(bAllPassed, Checks);
				},
				LiveAutoAttackProofDelaySeconds,
				false);
			return;
		}

		AppendLiveAutoAttackCheck(Checks, bAllPassed, LiveAutoAttackHeadshotTarget, AutoAttackProofCategory, LiveAutoAttackHeadshotChance);
		FinishStatPipelineSmoke(bAllPassed, Checks);
	}
#endif

	static void T66RecordNonDirectorRouteAttribution(UWorld* World, const AT66EnemyBase* Enemy)
	{
		if (!World || !Enemy)
		{
			return;
		}

		if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			MobManager->RecordRouteAttribution(
				Enemy->EnemyFamily,
				ET66RouteAttributionReason::RoutedRich_NonDirectorPath,
				ET66RouteAttributionChannel::NonDirector);
		}
	}

	template <typename TInteractableType>
	TInteractableType* T66FindRegisteredWorldInteractable(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66WorldInteractableBase>& WeakInteractable : Registry->GetWorldInteractables())
			{
				if (TInteractableType* Interactable = Cast<TInteractableType>(WeakInteractable.Get()))
				{
					return Interactable;
				}
			}
		}

		return nullptr;
	}

	bool T66HasRegisteredVendorBoss(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
			{
				if (Cast<AT66VendorBoss>(WeakBoss.Get()) != nullptr)
				{
					return true;
				}
			}
		}

		return false;
	}

	FString T66DescribeMovementQAAnimation(const TCHAR* Label, UAnimationAsset* Animation)
	{
		if (!Animation)
		{
			return FString::Printf(TEXT("%s=None"), Label ? Label : TEXT("Anim"));
		}

		const float PlayLength = Animation->GetPlayLength();
		UAnimSequence* Sequence = Cast<UAnimSequence>(Animation);
		if (!Sequence)
		{
			return FString::Printf(
				TEXT("%s=%s class=%s playLength=%.3f rootMotion=UnsupportedNonSequence"),
				Label ? Label : TEXT("Anim"),
				*GetNameSafe(Animation),
				*GetNameSafe(Animation->GetClass()),
				PlayLength);
		}

		const FTransform RootMotion = Sequence->ExtractRootMotionFromRange(0.0, static_cast<double>(PlayLength), FAnimExtractContext());
		const FVector RootMotionTranslation = RootMotion.GetTranslation();
		FString RootTrackSummary(TEXT("rootTrack=Unavailable"));
#if WITH_EDITOR
		if (const IAnimationDataModel* DataModel = Sequence->GetDataModel())
		{
			FName RootTrackName(NAME_None);
			TArray<FName> TrackNames;
			DataModel->GetBoneTrackNames(TrackNames);
			if (TrackNames.Contains(FName(TEXT("root"))))
			{
				RootTrackName = FName(TEXT("root"));
			}
			else if (TrackNames.Num() > 0)
			{
				RootTrackName = TrackNames[0];
			}

			if (!RootTrackName.IsNone() && DataModel->IsValidBoneTrackName(RootTrackName))
			{
				const int32 LastKeyIndex = FMath::Max(0, DataModel->GetNumberOfKeys() - 1);
				const FTransform FirstRoot = DataModel->GetBoneTrackTransform(RootTrackName, FFrameNumber(0));
				const FTransform LastRoot = DataModel->GetBoneTrackTransform(RootTrackName, FFrameNumber(LastKeyIndex));
				const FVector RootTrackDelta = LastRoot.GetTranslation() - FirstRoot.GetTranslation();
				RootTrackSummary = FString::Printf(
					TEXT("rootTrack=%s rootTrackDelta=%s rootTrackDelta2D=%.3f keys=%d frames=%d"),
					*RootTrackName.ToString(),
					*RootTrackDelta.ToCompactString(),
					RootTrackDelta.Size2D(),
					DataModel->GetNumberOfKeys(),
					DataModel->GetNumberOfFrames());
			}
		}
#endif

		return FString::Printf(
			TEXT("%s=%s class=%s playLength=%.3f bEnableRootMotion=%d hasRootMotion=%d extractedRootMotion=%s extractedRootMotion2D=%.3f %s"),
			Label ? Label : TEXT("Anim"),
			*GetNameSafe(Animation),
			*GetNameSafe(Animation->GetClass()),
			PlayLength,
			Sequence->bEnableRootMotion ? 1 : 0,
			Sequence->HasRootMotion() ? 1 : 0,
			*RootMotionTranslation.ToCompactString(),
			RootMotionTranslation.Size2D(),
			*RootTrackSummary);
	}

	FName T66GetHeroMovementQAVisualID()
	{
		FString VisualIDString;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQAVisualID="), VisualIDString)
			&& !VisualIDString.TrimStartAndEnd().IsEmpty())
		{
			return FName(*VisualIDString.TrimStartAndEnd());
		}
		if (FParse::Value(FCommandLine::Get(), TEXT("T66HeroVisualOverride="), VisualIDString)
			&& !VisualIDString.TrimStartAndEnd().IsEmpty())
		{
			return FName(*VisualIDString.TrimStartAndEnd());
		}
		return FName(TEXT("Hero_1_Chad"));
	}

	FName T66GetHeroMovementQACompanionID()
	{
		FString CompanionIDString(TEXT("Companion_01"));
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQACompanionID="), CompanionIDString);
		CompanionIDString = CompanionIDString.TrimStartAndEnd();
		return CompanionIDString.Equals(TEXT("None"), ESearchCase::IgnoreCase) || CompanionIDString.IsEmpty()
			? NAME_None
			: FName(*CompanionIDString);
	}
}


void AT66PlayerController::SetupGameplayHUD()
{
	if (!IsGameplayLevel() || !IsLocalController()) return;
	if (GameplayHUDWidget)
	{
		if (!GameplayHUDWidget->IsInViewport())
		{
			GameplayHUDWidget->AddToViewport(0);
		}
		GameplayHUDWidget->MarkHUDDirty();
		QueueGameplayAutomationScreenshotIfRequested();
		return;
	}

	UClass* HUDClass = ResolveGameplayHUDClass();
	if (!HUDClass) return;
	GameplayHUDWidget = CreateWidget<UT66GameplayHUDWidget>(this, HUDClass);
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->AddToViewport(0);
		QueueGameplayAutomationScreenshotIfRequested();
	}
	// The Lab: no floating panel; player interacts with The Collector NPC to open full-screen Collector UI.
	// (LabOverlayWidget not created when in Lab.)
}

void AT66PlayerController::ShowInteractionPrompt(AActor* SourceActor, const FText& TargetName)
{
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->ShowInteractionPrompt(SourceActor, TargetName);
	}
}

void AT66PlayerController::HideInteractionPrompt(AActor* SourceActor)
{
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->HideInteractionPrompt(SourceActor);
	}
}

void AT66PlayerController::QueueGameplayAutomationScreenshotIfRequested()
{
	if (!IsGameplayLevel() || !GetWorld())
	{
		return;
	}

	FString RequestedScreenshotPath;
	const bool bScreenshotRequested = FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshot="), RequestedScreenshotPath);

	FString RequestedScreenshotSequenceDir;
	const bool bScreenshotSequenceRequested = FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshotSequenceDir="), RequestedScreenshotSequenceDir);

	FString RequestedWidgetDumpSpec;
	const bool bWidgetDumpRequested = FParse::Value(FCommandLine::Get(), TEXT("T66AutoDumpWidget="), RequestedWidgetDumpSpec);

	FString RequestedGameplayAutomationCaptureMode;
	const bool bGameplayAutomationCaptureModeRequested =
		FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoCapture="), RequestedGameplayAutomationCaptureMode);
	const bool bModeOnlyAutomationRequested =
		bGameplayAutomationCaptureModeRequested
		&& T66IsModeOnlyGameplayAutomationCapture(RequestedGameplayAutomationCaptureMode);

	bool bItemTaxonomySmokeRequested = false;
	FString RequestedItemTaxonomySmokePath;
	bool bStatPipelineSmokeRequested = false;
	FString RequestedStatPipelineSmokePath;
	bool bEndgameSaintSmokeRequested = false;
	FString RequestedEndgameSaintSmokePath;
#if !UE_BUILD_SHIPPING
	bItemTaxonomySmokeRequested = FParse::Value(FCommandLine::Get(), TEXT("T66ItemTaxonomySmoke="), RequestedItemTaxonomySmokePath);
	bStatPipelineSmokeRequested = FParse::Value(FCommandLine::Get(), TEXT("T66StatPipelineSmoke="), RequestedStatPipelineSmokePath);
	bEndgameSaintSmokeRequested = FParse::Value(FCommandLine::Get(), TEXT("T66EndgameSaintSmoke="), RequestedEndgameSaintSmokePath);
#endif

	if (!bScreenshotRequested
		&& !bScreenshotSequenceRequested
		&& !bWidgetDumpRequested
		&& !bModeOnlyAutomationRequested
		&& !bItemTaxonomySmokeRequested
		&& !bStatPipelineSmokeRequested
		&& !bEndgameSaintSmokeRequested)
	{
		return;
	}

#if !UE_BUILD_SHIPPING
	if (bItemTaxonomySmokeRequested)
	{
		const FString SmokeOutputPath = FPaths::ConvertRelativePathToFull(RequestedItemTaxonomySmokePath);
		FTimerHandle ItemTaxonomySmokeTimerHandle;
		FTimerDelegate SmokeDelegate = FTimerDelegate::CreateLambda([WeakThis = TWeakObjectPtr<AT66PlayerController>(this), SmokeOutputPath]()
		{
			if (AT66PlayerController* StrongThis = WeakThis.Get())
			{
				T66RunItemTaxonomySmoke(StrongThis, SmokeOutputPath);
			}
		});
		GetWorldTimerManager().SetTimer(ItemTaxonomySmokeTimerHandle, SmokeDelegate, 1.0f, false);
	}

	if (bStatPipelineSmokeRequested)
	{
		const FString SmokeOutputPath = FPaths::ConvertRelativePathToFull(RequestedStatPipelineSmokePath);
		FTimerHandle StatPipelineSmokeTimerHandle;
		FTimerDelegate SmokeDelegate = FTimerDelegate::CreateLambda([WeakThis = TWeakObjectPtr<AT66PlayerController>(this), SmokeOutputPath]()
		{
			if (AT66PlayerController* StrongThis = WeakThis.Get())
			{
				T66RunStatPipelineSmoke(StrongThis, SmokeOutputPath);
			}
		});
		GetWorldTimerManager().SetTimer(StatPipelineSmokeTimerHandle, SmokeDelegate, 1.0f, false);
	}

	if (bEndgameSaintSmokeRequested)
	{
		const FString SmokeOutputPath = FPaths::ConvertRelativePathToFull(RequestedEndgameSaintSmokePath);
		FTimerHandle EndgameSaintSmokeTimerHandle;
		FTimerDelegate SmokeDelegate = FTimerDelegate::CreateLambda([WeakThis = TWeakObjectPtr<AT66PlayerController>(this), SmokeOutputPath]()
		{
			if (AT66PlayerController* StrongThis = WeakThis.Get())
			{
				UWorld* SmokeWorld = StrongThis->GetWorld();
				AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
				const bool bPass = SmokeGameMode && SmokeGameMode->RunEndgameSaintSmoke(SmokeWorld, SmokeOutputPath);
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 72, TEXT("T66EndgameSaintSmokeComplete"));
			}
		});
		GetWorldTimerManager().SetTimer(EndgameSaintSmokeTimerHandle, SmokeDelegate, 1.0f, false);
	}
#endif

	if (!bScreenshotRequested && !bScreenshotSequenceRequested && !bWidgetDumpRequested && !bModeOnlyAutomationRequested)
	{
		return;
	}

	GameplayAutomationScreenshotPath.Reset();
	GameplayAutomationScreenshotSequenceDir.Reset();
	GameplayAutomationScreenshotSequencePrefix = TEXT("frame");
	GameplayAutomationScreenshotSequenceCount = 0;
	GameplayAutomationScreenshotSequenceIndex = 0;
	GameplayAutomationScreenshotSequenceIntervalSeconds = 0.1f;
	GameplayAutomationWidgetDumpTarget.Reset();
	GameplayAutomationWidgetDumpPath.Reset();
	GameplayAutomationScreenshotDelaySeconds = 4.0f;
	GameplayAutomationWidgetDumpDelaySeconds = 4.0f;
	GameplayAutomationPostCaptureScreenshotDelaySeconds = 0.5f;
	GameplayAutomationCaptureMode = bGameplayAutomationCaptureModeRequested
		? RequestedGameplayAutomationCaptureMode
		: TEXT("HUD");
	if (bScreenshotRequested)
	{
		GameplayAutomationScreenshotPath = FPaths::ConvertRelativePathToFull(RequestedScreenshotPath);
		FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshotDelay="), GameplayAutomationScreenshotDelaySeconds);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoPostCaptureScreenshotDelay="),
			GameplayAutomationPostCaptureScreenshotDelaySeconds);
	}
	if (bScreenshotSequenceRequested)
	{
		GameplayAutomationScreenshotSequenceDir = FPaths::ConvertRelativePathToFull(RequestedScreenshotSequenceDir);
		FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoScreenshotDelay="), GameplayAutomationScreenshotDelaySeconds);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoPostCaptureScreenshotDelay="),
			GameplayAutomationPostCaptureScreenshotDelaySeconds);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoScreenshotSequenceCount="),
			GameplayAutomationScreenshotSequenceCount);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoScreenshotSequenceInterval="),
			GameplayAutomationScreenshotSequenceIntervalSeconds);
		FParse::Value(
			FCommandLine::Get(),
			TEXT("T66GameplayAutoScreenshotSequencePrefix="),
			GameplayAutomationScreenshotSequencePrefix);
		GameplayAutomationScreenshotSequenceCount = FMath::Clamp(GameplayAutomationScreenshotSequenceCount, 1, 240);
		GameplayAutomationScreenshotSequenceIntervalSeconds = FMath::Clamp(GameplayAutomationScreenshotSequenceIntervalSeconds, 0.016f, 2.0f);
	}
	if (bWidgetDumpRequested)
	{
		FString WidgetDumpPath;
		FString ParseError;
		if (!FT66WidgetDumpTargets::ParseAutomationSpec(RequestedWidgetDumpSpec, GameplayAutomationWidgetDumpTarget, WidgetDumpPath, ParseError))
		{
			UE_LOG(LogTemp, Error, TEXT("Gameplay automation: invalid widget dump spec: %s"), *ParseError);
			FPlatformMisc::RequestExitWithStatus(false, 67, TEXT("T66AutoDumpWidgetInvalid"));
			return;
		}

		GameplayAutomationWidgetDumpPath = FPaths::ConvertRelativePathToFull(WidgetDumpPath);
		GameplayAutomationWidgetDumpDelaySeconds = GameplayAutomationScreenshotDelaySeconds;
		FParse::Value(FCommandLine::Get(), TEXT("T66AutoDumpWidgetDelay="), GameplayAutomationWidgetDumpDelaySeconds);
	}

	bGameplayAutomationKeepAliveAfterScreenshot =
		FParse::Param(FCommandLine::Get(), TEXT("T66GameplayKeepAliveAfterScreenshot"))
		|| FParse::Param(FCommandLine::Get(), TEXT("T66KeepAliveAfterScreenshot"));

	int32 AutomationResX = 0;
	int32 AutomationResY = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResX="), AutomationResX)
		&& FParse::Value(FCommandLine::Get(), TEXT("T66AutomationResY="), AutomationResY)
		&& AutomationResX > 0
		&& AutomationResY > 0)
	{
		const TCHAR* WindowModeSuffix = FParse::Param(FCommandLine::Get(), TEXT("T66AutomationWindowed")) ? TEXT("w") : TEXT("");
		ConsoleCommand(FString::Printf(TEXT("r.SetRes %dx%d%s"), AutomationResX, AutomationResY, WindowModeSuffix), true);
	}

	GetWorldTimerManager().ClearTimer(GameplayAutomationPrepareTimerHandle);
	GetWorldTimerManager().SetTimer(
		GameplayAutomationPrepareTimerHandle,
		this,
		&AT66PlayerController::HandleGameplayAutomationPrepare,
		FMath::Max(0.1f, FMath::Min(
			(bScreenshotRequested || bScreenshotSequenceRequested) ? GameplayAutomationScreenshotDelaySeconds : GameplayAutomationWidgetDumpDelaySeconds,
			bWidgetDumpRequested ? GameplayAutomationWidgetDumpDelaySeconds : GameplayAutomationScreenshotDelaySeconds)),
		false);
}

void AT66PlayerController::HandleGameplayAutomationPrepare()
{
	if (GameplayAutomationScreenshotPath.IsEmpty()
		&& GameplayAutomationScreenshotSequenceDir.IsEmpty()
		&& GameplayAutomationWidgetDumpPath.IsEmpty()
		&& !T66IsModeOnlyGameplayAutomationCapture(GameplayAutomationCaptureMode))
	{
		return;
	}

	ApplyGameplayAutomationCaptureMode();

	if (GameplayAutomationScreenshotPath.IsEmpty()
		&& GameplayAutomationScreenshotSequenceDir.IsEmpty()
		&& GameplayAutomationWidgetDumpPath.IsEmpty())
	{
		return;
	}

	if (GetWorld())
	{
		const float PostCaptureScreenshotDelaySeconds = FMath::Max(0.1f, GameplayAutomationPostCaptureScreenshotDelaySeconds);
		if (!GameplayAutomationScreenshotSequenceDir.IsEmpty())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationScreenshotTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationScreenshotTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationSequenceScreenshot,
				PostCaptureScreenshotDelaySeconds,
				false);
		}
		else if (!GameplayAutomationScreenshotPath.IsEmpty())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationScreenshotTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationScreenshotTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationScreenshot,
				PostCaptureScreenshotDelaySeconds,
				false);
		}

		if (!GameplayAutomationWidgetDumpPath.IsEmpty())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationWidgetDumpTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationWidgetDumpTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationWidgetDump,
				PostCaptureScreenshotDelaySeconds + 0.25f,
				false);
		}
	}
}

void AT66PlayerController::ApplyGameplayAutomationCaptureMode()
{
	const FString Mode = GameplayAutomationCaptureMode.TrimStartAndEnd().ToLower();

	if (Mode == TEXT("hudreview") || Mode == TEXT("hudvisualreview") || Mode == TEXT("ingamehudreview"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		FString PickupCardItemIDString;
		const bool bShowPickupCardForReview = FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoPickupCard="), PickupCardItemIDString)
			&& !PickupCardItemIDString.TrimStartAndEnd().IsEmpty();
		const FName PickupCardItemID(*PickupCardItemIDString.TrimStartAndEnd());
		ET66ItemRarity PickupCardRarity = ET66ItemRarity::Yellow;
		FString PickupCardRarityString;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoPickupCardRarity="), PickupCardRarityString))
		{
			const FString NormalizedRarity = PickupCardRarityString.TrimStartAndEnd().ToLower();
			if (NormalizedRarity == TEXT("black"))
			{
				PickupCardRarity = ET66ItemRarity::Black;
			}
			else if (NormalizedRarity == TEXT("red"))
			{
				PickupCardRarity = ET66ItemRarity::Red;
			}
			else if (NormalizedRarity == TEXT("white"))
			{
				PickupCardRarity = ET66ItemRarity::White;
			}
		}
		if (RunState)
		{
			const int32 MissingDifficultySkulls = FMath::Max(0, 4 - RunState->GetDifficultySkulls());
			if (MissingDifficultySkulls > 0)
			{
				RunState->AddDifficultySkulls(MissingDifficultySkulls);
			}

			while (RunState->GetCowardiceGatesTaken() < 4)
			{
				RunState->AddCowardiceGateTaken();
			}

			RunState->AddGold(888);
			RunState->AddScore(1250);
			RunState->SetStageTimerActive(true);

			FRandomStream ReviewItemStream(660427);
			const ET66Rarity ReviewLootRarities[] =
			{
				ET66Rarity::Black,
				ET66Rarity::Red,
				ET66Rarity::Yellow,
				ET66Rarity::White
			};
			int32 RarityIndex = 0;
			int32 ItemAttempts = 0;
			while (T66GI
				&& RunState->GetInventorySlots().Num() < UT66RunStateSubsystem::MaxInventorySlots
				&& ItemAttempts < UT66RunStateSubsystem::MaxInventorySlots * 3)
			{
				++ItemAttempts;
				const ET66Rarity LootRarity = ReviewLootRarities[RarityIndex % UE_ARRAY_COUNT(ReviewLootRarities)];
				const FName ItemID = T66GI->GetRandomItemIDForLootRarityFromStream(LootRarity, ReviewItemStream);
				if (ItemID.IsNone())
				{
					break;
				}

				const ET66ItemRarity ItemRarity = static_cast<ET66ItemRarity>(static_cast<uint8>(LootRarity));
				RunState->AddItemWithRarity(ItemID, ItemRarity);
				++RarityIndex;
			}

			if (bShowPickupCardForReview && !PickupCardItemID.IsNone())
			{
				RunState->AddItemWithRarity(PickupCardItemID, PickupCardRarity);
			}
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			if (!bShowPickupCardForReview)
			{
				GameplayHUDWidget->ShowInteractionPrompt(this, NSLOCTEXT("T66.GameplayHUD", "HudReviewInteractionTarget", "Chest"));
			}
			GameplayHUDWidget->RefreshHUD();
			if (bShowPickupCardForReview && !PickupCardItemID.IsNone())
			{
				GameplayHUDWidget->ShowPickupItemCard(PickupCardItemID, PickupCardRarity);
			}
		}
		return;
	}

	if (Mode == TEXT("lootcrate") || Mode == TEXT("lootcrateui"))
	{
		if (!GameplayHUDWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootcrate failed: missing GameplayHUDWidget."));
			return;
		}

		GameplayHUDWidget->SetFullMapOpen(false);
		GameplayHUDWidget->RefreshHUD();
		StartCrateOpenHUD(ET66Rarity::Yellow);
		UE_LOG(LogTemp, Display, TEXT("[LootUICapture] lootcrate triggered via StartCrateOpenHUD rarity=Yellow."));
		return;
	}

	if (Mode == TEXT("lootchest") || Mode == TEXT("lootchestui"))
	{
		if (!GameplayHUDWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootchest failed: missing GameplayHUDWidget."));
			return;
		}

		GameplayHUDWidget->SetFullMapOpen(false);
		GameplayHUDWidget->RefreshHUD();
		const bool bStarted = StartChestRewardHUD(ET66Rarity::Yellow, 188);
		if (bStarted)
		{
			UE_LOG(LogTemp, Display, TEXT("[LootUICapture] lootchest triggered via StartChestRewardHUD rarity=Yellow gold=188."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootchest failed via StartChestRewardHUD rarity=Yellow gold=188."));
		}
		return;
	}

	if (Mode == TEXT("lootwheel") || Mode == TEXT("lootwheelui"))
	{
		if (!GameplayHUDWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootwheel failed: missing GameplayHUDWidget."));
			return;
		}

		UWorld* World = GetWorld();
		if (!World)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootwheel failed: missing world."));
			return;
		}

		const APawn* ControlledPawn = GetPawn();
		const FVector BaseLocation = ControlledPawn ? ControlledPawn->GetActorLocation() : FVector::ZeroVector;
		const FVector Forward = ControlledPawn ? ControlledPawn->GetActorForwardVector() : FVector::ForwardVector;
		const FVector LootWheelSpawnLocation = BaseLocation + Forward * 260.f + FVector(0.f, 0.f, 30.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66LootWheelInteractable* LootWheel = World->SpawnActor<AT66LootWheelInteractable>(
			AT66LootWheelInteractable::StaticClass(),
			LootWheelSpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!LootWheel)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootwheel failed: could not spawn interactable."));
			return;
		}

		LootWheel->SetShowcaseReusable(true);
		LootWheel->SetRarity(ET66Rarity::Yellow);
		GameplayHUDWidget->SetFullMapOpen(false);
		GameplayHUDWidget->RefreshHUD();
		const bool bStarted = LootWheel->Interact(this);
		UE_LOG(LogTemp, Display, TEXT("[LootUICapture] lootwheel triggered via spawned interactable real path started=%d."), bStarted ? 1 : 0);
		return;
	}

	if (Mode == TEXT("lootbag") || Mode == TEXT("lootbagui"))
	{
		if (!GameplayHUDWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootbag failed: missing GameplayHUDWidget."));
			return;
		}

		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		static const FName LootBagCaptureItemID(TEXT("Item_AoeDamage"));
		static constexpr ET66ItemRarity LootBagCaptureItemRarity = ET66ItemRarity::Yellow;
		FItemData ItemData;
		if (!T66GI || !T66GI->GetItemData(LootBagCaptureItemID, ItemData))
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootbag failed: missing item data for %s."), *LootBagCaptureItemID.ToString());
			return;
		}
		if (!RunState)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootUICapture] lootbag failed: missing RunState."));
			return;
		}

		RunState->AddItemWithRarity(LootBagCaptureItemID, LootBagCaptureItemRarity);
		GameplayHUDWidget->SetFullMapOpen(false);
		GameplayHUDWidget->RefreshHUD();
		ShowLootBagItemRevealHUD(LootBagCaptureItemID, LootBagCaptureItemRarity);
		UE_LOG(LogTemp, Display, TEXT("[LootUICapture] lootbag triggered via RunState AddItemWithRarity + ShowLootBagItemRevealHUD item=%s rarity=Yellow."), *LootBagCaptureItemID.ToString());
		return;
	}

#if !UE_BUILD_SHIPPING
	if (Mode == TEXT("mobcombatsmoke") || Mode == TEXT("lightweightactorb4"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		if (!World || !HeroPawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobCombatSmoke] Failed: missing world or hero pawn."));
			return;
		}

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f));
		SetControlRotation(FRotator(-28.f, 0.f, 0.f));
		if (HeroPawn->CameraBoom)
		{
			HeroPawn->CameraBoom->TargetArmLength = 620.f;
			HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 125.f));
			HeroPawn->CameraBoom->bDoCollisionTest = false;
		}

		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (GameMode && GameMode->IsUsingTowerMainMapLayout() && GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
			for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
			{
				if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
				{
					TargetFloor = &Floor;
					break;
				}
				if (!TargetFloor && Floor.bMobFloor)
				{
					TargetFloor = &Floor;
				}
			}

			if (TargetFloor)
			{
				float PawnHalfHeight = 100.0f;
				if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
				{
					PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
				}

				FVector TargetLocation = TargetFloor->ArrivalPoint;
				if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
				{
					TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
				}
				if (TargetLocation.IsNearlyZero())
				{
					TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
				}
				TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

				HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
				HeroPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::TeleportPhysics);
				SetControlRotation(FRotator(-28.f, 0.f, 0.f));
				GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
				GameMode->SetEnemyDirectorSpawningPaused(true);
				if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
				{
					RunState->SetStageTimerActive(true);
				}
				UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Entered mob floor=%d location=%s with director paused."),
					TargetFloor->FloorNumber,
					*HeroPawn->GetActorLocation().ToCompactString());
			}
		}

		if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			TArray<TWeakObjectPtr<AT66MobBase>> ExistingMobs = Manager->GetActiveMobs();
			for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ExistingMobs)
			{
				if (AT66MobBase* Mob = WeakMob.Get())
				{
					if (Mob->MobID.ToString().StartsWith(TEXT("TestMob_")))
					{
						Mob->Destroy();
					}
				}
			}
		}

		TWeakObjectPtr<AT66HeroBase> WeakHero(HeroPawn);
		auto SpawnSmokeMob = [World, WeakHero](const TCHAR* MobIdText, const float HP, const FVector& LocalOffset) -> AT66MobBase*
		{
			AT66HeroBase* Hero = WeakHero.Get();
			if (!World || !Hero)
			{
				return nullptr;
			}

			const FVector Forward = Hero->GetActorForwardVector().GetSafeNormal2D();
			const FVector Right = Hero->GetActorRightVector().GetSafeNormal2D();
			FVector SpawnLocation = Hero->GetActorLocation()
				+ Forward * LocalOffset.X
				+ Right * LocalOffset.Y;
			SpawnLocation.Z += LocalOffset.Z;

			const FTransform SpawnTransform(Hero->GetActorRotation(), SpawnLocation);
			AT66MobBase* Mob = World->SpawnActorDeferred<AT66MobBase>(AT66MobBase::StaticClass(), SpawnTransform);
			if (!Mob)
			{
				return nullptr;
			}

			Mob->MobID = FName(MobIdText);
			Mob->EnemyFamily = ET66EnemyFamily::Melee;
			Mob->MaxHP = HP;
			Mob->CurrentHP = HP;
			Mob->TouchDamageCooldownSeconds = 0.f;
			Mob->LifecycleState = ET66MobLifecycleState::Active;
			Mob->FinishSpawning(SpawnTransform);
			UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Spawned %s hp=%.1f location=%s"),
				MobIdText,
				HP,
				*SpawnLocation.ToCompactString());
			return Mob;
		};

		TWeakObjectPtr<AT66MobBase> KillMob = SpawnSmokeMob(TEXT("TestMob_B4Kill"), 45.f, FVector(240.f, 0.f, 0.f));

		FTimerHandle HeroHitMobHandle;
		World->GetTimerManager().SetTimer(
			HeroHitMobHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakHero, KillMob]()
			{
				AT66HeroBase* Hero = WeakHero.Get();
				AT66MobBase* Mob = KillMob.Get();
				if (!Hero || !Mob || !Hero->CombatComponent)
				{
					return;
				}

				const FVector Start = Hero->GetActorLocation() + FVector(0.f, 0.f, 64.f);
				FVector Direction = Mob->GetActorLocation() - Start;
				Direction.Z = 0.f;
				if (!Direction.Normalize())
				{
					Direction = Hero->GetActorForwardVector().GetSafeNormal2D();
				}
				const FVector End = Start + Direction * 420.f;
				Hero->CombatComponent->PerformScopedPiercingShot(Start, End);
				UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Fired hero combat component scoped shot through %s."), *GetNameSafe(Mob));
			}),
			1.0f,
			false);

		FTimerHandle SpawnStunnedMobHandle;
		World->GetTimerManager().SetTimer(
			SpawnStunnedMobHandle,
			FTimerDelegate::CreateWeakLambda(this, [SpawnSmokeMob]()
			{
				if (AT66MobBase* StunnedMob = SpawnSmokeMob(TEXT("TestMob_B4StunnedTouch"), 300.f, FVector(48.f, 0.f, 0.f)))
				{
					StunnedMob->ApplyStun(3.f);
					UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Applied stun to %s for touch-damage parity check."), *GetNameSafe(StunnedMob));
				}
			}),
			3.5f,
			false);

		FTimerHandle CleanupMobsHandle;
		World->GetTimerManager().SetTimer(
			CleanupMobsHandle,
			FTimerDelegate::CreateWeakLambda(this, [World]()
			{
				if (UT66MobManagerSubsystem* Manager = World ? World->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
				{
					TArray<TWeakObjectPtr<AT66MobBase>> Mobs = Manager->GetActiveMobs();
					for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Mobs)
					{
						if (AT66MobBase* Mob = WeakMob.Get())
						{
							if (Mob->MobID.ToString().StartsWith(TEXT("TestMob_")))
							{
								Mob->Destroy();
							}
						}
					}
				}
			}),
			8.5f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[MobCombatSmoke] Lightweight Actor B.4 smoke sequence armed."));
		return;
	}

	if (Mode == TEXT("mobdatabindingsmoke") || Mode == TEXT("lightweightactorb5"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		if (!World || !HeroPawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobDataBindingSmoke] Failed: missing world or hero pawn."));
			return;
		}

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f));
		SetControlRotation(FRotator(-30.f, 0.f, 0.f));
		if (HeroPawn->CameraBoom)
		{
			HeroPawn->CameraBoom->TargetArmLength = 760.f;
			HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 155.f));
			HeroPawn->CameraBoom->bDoCollisionTest = false;
		}

		AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode());
		T66TowerMapTerrain::FLayout TowerLayout;
		if (GameMode && GameMode->IsUsingTowerMainMapLayout() && GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
			for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
			{
				if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
				{
					TargetFloor = &Floor;
					break;
				}
				if (!TargetFloor && Floor.bMobFloor)
				{
					TargetFloor = &Floor;
				}
			}

			if (TargetFloor)
			{
				float PawnHalfHeight = 100.f;
				if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
				{
					PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
				}

				FVector TargetLocation = TargetFloor->ArrivalPoint;
				if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
				{
					TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
				}
				if (TargetLocation.IsNearlyZero())
				{
					TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
				}
				TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.f;

				HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
				HeroPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::TeleportPhysics);
				SetControlRotation(FRotator(-30.f, 0.f, 0.f));
				GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
				GameMode->SetEnemyDirectorSpawningPaused(true);
				if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
				{
					RunState->SetStageTimerActive(true);
				}
				UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Entered mob floor=%d location=%s with director paused."),
					TargetFloor->FloorNumber,
					*HeroPawn->GetActorLocation().ToCompactString());
			}
		}

		const FName TestMobTag(TEXT("T66_TestMob"));
		if (UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			TArray<TWeakObjectPtr<AT66MobBase>> ExistingMobs = Manager->GetActiveMobs();
			for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ExistingMobs)
			{
				if (AT66MobBase* Mob = WeakMob.Get())
				{
					if (Mob->ActorHasTag(TestMobTag) || Mob->MobID.ToString().StartsWith(TEXT("TestMob_")))
					{
						Mob->Destroy();
					}
				}
			}
		}

		int32 StageNum = 1;
		float DifficultyScalar = 1.f;
		float FinaleScalar = 1.f;
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			StageNum = FMath::Max(1, RunState->GetCurrentStage());
			DifficultyScalar = RunState->GetDifficultyScalar();
			FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
		}

		TWeakObjectPtr<AT66HeroBase> WeakHero(HeroPawn);
		auto SpawnConfiguredSmokeMob = [World, WeakHero, StageNum, DifficultyScalar, FinaleScalar, TestMobTag](const FName MobID, const FVector& LocalOffset) -> AT66MobBase*
		{
			AT66HeroBase* Hero = WeakHero.Get();
			if (!World || !Hero)
			{
				return nullptr;
			}

			const FVector Forward = Hero->GetActorForwardVector().GetSafeNormal2D();
			const FVector Right = Hero->GetActorRightVector().GetSafeNormal2D();
			FVector SpawnLocation = Hero->GetActorLocation()
				+ Forward * LocalOffset.X
				+ Right * LocalOffset.Y;
			SpawnLocation.Z += LocalOffset.Z;

			const FTransform SpawnTransform(Hero->GetActorRotation(), SpawnLocation);
			AT66MobBase* Mob = World->SpawnActorDeferred<AT66MobBase>(AT66MobBase::StaticClass(), SpawnTransform);
			if (!Mob)
			{
				return nullptr;
			}

			Mob->Tags.AddUnique(TestMobTag);
			Mob->MobID = MobID;
			Mob->CharacterVisualID = MobID;
			Mob->LifecycleState = ET66MobLifecycleState::Active;
			Mob->FinishSpawning(SpawnTransform);
			Mob->ConfigureAsMob(MobID, ET66EnemyFamily::Melee, NAME_None, StageNum, DifficultyScalar, 1.f, FinaleScalar, false);
			Mob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 30.f);
			UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Spawned configured MobID=%s hp=%.1f speed=%.1f touch=%d location=%s"),
				*MobID.ToString(),
				Mob->MaxHP,
				Mob->ChaseSpeed,
				Mob->TouchDamageHearts,
				*SpawnLocation.ToCompactString());
			return Mob;
		};

		TWeakObjectPtr<AT66MobBase> SlimeHitMob = SpawnConfiguredSmokeMob(FName(TEXT("Slime")), FVector(240.f, -260.f, 0.f));
		TWeakObjectPtr<AT66MobBase> SlimeTouchMob = SpawnConfiguredSmokeMob(FName(TEXT("Slime")), FVector(52.f, 260.f, 0.f));

		const FName Roster[] =
		{
			FName(TEXT("Slime")),
			FName(TEXT("CaveBat")),
			FName(TEXT("BoneWalker")),
			FName(TEXT("RatPack")),
			FName(TEXT("TombSpider")),
			FName(TEXT("HexSlinger")),
			FName(TEXT("StoneSentinel")),
			FName(TEXT("MimicLure")),
			FName(TEXT("BoneConjurer")),
			FName(TEXT("CryptWraith"))
		};
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(Roster); ++Index)
		{
			const float OffsetY = -900.f + static_cast<float>(Index * 200);
			if (AT66MobBase* RosterMob = SpawnConfiguredSmokeMob(Roster[Index], FVector(520.f, OffsetY, 0.f)))
			{
				RosterMob->TouchDamageCooldownSeconds = 999.f;
			}
		}

		FTimerHandle HeroHitMobHandle;
		World->GetTimerManager().SetTimer(
			HeroHitMobHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakHero, SlimeHitMob]()
			{
				AT66HeroBase* Hero = WeakHero.Get();
				AT66MobBase* Mob = SlimeHitMob.Get();
				if (!Hero || !Mob || !Hero->CombatComponent)
				{
					return;
				}

				const FVector Start = Hero->GetActorLocation() + FVector(0.f, 0.f, 64.f);
				FVector Direction = Mob->GetActorLocation() - Start;
				Direction.Z = 0.f;
				if (!Direction.Normalize())
				{
					Direction = Hero->GetActorForwardVector().GetSafeNormal2D();
				}
				Hero->CombatComponent->PerformScopedPiercingShot(Start, Start + Direction * 420.f);
				UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Fired hero combat component scoped shot through configured %s."), *GetNameSafe(Mob));
			}),
			1.0f,
			false);

		FTimerHandle CleanupTouchMobHandle;
		World->GetTimerManager().SetTimer(
			CleanupTouchMobHandle,
			FTimerDelegate::CreateWeakLambda(this, [SlimeTouchMob]()
			{
				if (AT66MobBase* Mob = SlimeTouchMob.Get())
				{
					Mob->Destroy();
					UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Destroyed touch-damage Slime after verification window."));
				}
			}),
			1.25f,
			false);

		FTimerHandle CleanupMobsHandle;
		World->GetTimerManager().SetTimer(
			CleanupMobsHandle,
			FTimerDelegate::CreateWeakLambda(this, [World, TestMobTag]()
			{
				if (UT66MobManagerSubsystem* Manager = World ? World->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
				{
					TArray<TWeakObjectPtr<AT66MobBase>> Mobs = Manager->GetActiveMobs();
					for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Mobs)
					{
						if (AT66MobBase* Mob = WeakMob.Get())
						{
							if (Mob->ActorHasTag(TestMobTag))
							{
								Mob->Destroy();
							}
						}
					}
				}
			}),
			12.5f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[MobDataBindingSmoke] Lightweight Actor B.5 data-binding smoke sequence armed."));
		return;
	}

	if (Mode == TEXT("mobdirectorroutingsmoke") || Mode == TEXT("lightweightactorb6"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobDirectorRoutingSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bMobFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobDirectorRoutingSmoke] Failed: no mob floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-35.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		FTimerHandle DirectorSmokeCombatHandle;
		World->GetTimerManager().SetTimer(
			DirectorSmokeCombatHandle,
			FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				AT66HeroBase* SmokeHero = PC ? Cast<AT66HeroBase>(PC->GetPawn()) : nullptr;
				UT66MobManagerSubsystem* Manager = SmokeWorld ? SmokeWorld->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
				if (!SmokeWorld || !SmokeHero || !Manager)
				{
					UE_LOG(LogTemp, Warning, TEXT("[MobDirectorRoutingSmoke] Combat step skipped: missing world, hero, or mob manager."));
					return;
				}

				AT66MobBase* TargetMob = nullptr;
				for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Manager->GetActiveMobs())
				{
					AT66MobBase* Mob = WeakMob.Get();
					if (Mob && Mob->IsAliveAndActive() && !Mob->MobID.ToString().StartsWith(TEXT("TestMob_")))
					{
						TargetMob = Mob;
						break;
					}
				}

				UE_LOG(LogTemp, Display, TEXT("[MobDirectorRoutingSmoke] Active lightweight mobs=%d target=%s."),
					Manager->GetActiveMobs().Num(),
					*GetNameSafe(TargetMob));

				if (!TargetMob)
				{
					return;
				}

				SmokeHero->SetActorLocation(TargetMob->GetActorLocation() + FVector(36.f, 0.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
				if (SmokeHero->CombatComponent)
				{
					for (int32 ShotIndex = 0; ShotIndex < 8 && TargetMob->IsAliveAndActive(); ++ShotIndex)
					{
						const FVector Start = SmokeHero->GetActorLocation() + FVector(0.f, 0.f, 64.f);
						FVector Direction = TargetMob->GetActorLocation() - Start;
						Direction.Z = 0.f;
						if (!Direction.Normalize())
						{
							Direction = SmokeHero->GetActorForwardVector().GetSafeNormal2D();
						}
						SmokeHero->CombatComponent->PerformScopedPiercingShot(Start, Start + Direction * 500.f);
					}
				}
				if (TargetMob->IsAliveAndActive())
				{
					TargetMob->TakeDamageFromHeroHitZone(
						FMath::CeilToInt(TargetMob->GetCurrentHP()),
						TargetMob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body),
						FName(TEXT("B6DirectorRoutingSmoke")),
						NAME_None);
				}
			}),
			8.0f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[MobDirectorRoutingSmoke] Lightweight Actor B.6 director-routing smoke sequence armed on floor=%d location=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString());
		return;
	}

	if (Mode == TEXT("rushsmoke") || Mode == TEXT("lightweightactorb8"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[RushSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bMobFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[RushSmoke] Failed: no mob floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-28.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		const FVector RushSpawnLocation = TargetLocation + FVector(900.0f, 0.0f, 0.0f);
		const FTransform RushSpawnTransform(FRotator::ZeroRotator, RushSpawnLocation);
		AT66MobBase* RushMob = World->SpawnActorDeferred<AT66MobBase>(
			AT66MobBase::StaticClass(),
			RushSpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (RushMob)
		{
			RushMob->Tags.AddUnique(FName(TEXT("T66_TestMob")));
			RushMob->MobID = FName(TEXT("RatPack"));
			RushMob->CharacterVisualID = FName(TEXT("RatPack"));
			RushMob->LifecycleState = ET66MobLifecycleState::Active;
			UGameplayStatics::FinishSpawningActor(RushMob, RushSpawnTransform);

			int32 StageNum = 1;
			float DifficultyScalar = 1.f;
			float EnemyProgressionScalar = 1.f;
			float FinaleScalar = 1.f;
			if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
			{
				StageNum = FMath::Max(1, RunState->GetCurrentStage());
				DifficultyScalar = RunState->GetDifficultyScalar();
				FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
			}
			RushMob->ConfigureAsMob(FName(TEXT("RatPack")), ET66EnemyFamily::Rush, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
			RushMob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 8.f);
		}

		TWeakObjectPtr<AT66MobBase> WeakRushMob(RushMob);
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		const float LogTimes[] = { 0.35f, 0.95f, 1.35f, 1.85f, 2.35f, 3.0f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle RushLogHandle;
			const float LogTime = LogTimes[Index];
			World->GetTimerManager().SetTimer(
				RushLogHandle,
				FTimerDelegate::CreateLambda([WeakRushMob, LogTime]()
				{
					const AT66MobBase* Mob = WeakRushMob.Get();
					if (!Mob)
					{
						UE_LOG(LogTemp, Warning, TEXT("[RushSmoke] t=%.2fs test Rush mob missing."), LogTime);
						return;
					}

					UE_LOG(LogTemp, Display, TEXT("[RushSmoke] t=%.2fs MobID=%s loc=%s velocity=%s rushing=%d rushRemaining=%.2f rushCooldown=%.2f"),
						LogTime,
						Mob->MobID.IsNone() ? TEXT("unset") : *Mob->MobID.ToString(),
						*Mob->GetActorLocation().ToCompactString(),
						*Mob->StoredVelocity.ToCompactString(),
						Mob->bIsRushing ? 1 : 0,
						Mob->RushSecondsRemaining,
						Mob->RushCooldownRemaining);
				}),
				LogTime,
				false);
		}

		FTimerHandle StunHandle;
		World->GetTimerManager().SetTimer(
			StunHandle,
			FTimerDelegate::CreateLambda([WeakRushMob]()
			{
				if (AT66MobBase* Mob = WeakRushMob.Get())
				{
					Mob->ApplyStun(3.0f);
					UE_LOG(LogTemp, Display, TEXT("[RushSmoke] Applied 3.0s stun to Rush mob; rush timers should pause while status blocks chase."));
				}
			}),
			3.25f,
			false);

		FTimerHandle DirectorCountHandle;
		World->GetTimerManager().SetTimer(
			DirectorCountHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
				const AT66EnemyDirector* Director = SmokeGameMode ? SmokeGameMode->GetEnemyDirectorForDiagnostics() : nullptr;
				UE_LOG(LogTemp, Display, TEXT("[RushSmoke] directorCounts rich=%d lightweight=%d lightweightMelee=%d lightweightRush=%d lightweightRanged=%d."),
					Director ? Director->GetAliveRichEnemyCount() : -1,
					Director ? Director->GetAliveLightweightMobCount() : -1,
					Director ? Director->GetAliveLightweightMeleeMobCount() : -1,
					Director ? Director->GetAliveLightweightRushMobCount() : -1,
					Director ? Director->GetAliveLightweightRangedMobCount() : -1);
			}),
			8.0f,
			false);

		FTimerHandle RushSmokeExitHandle;
		World->GetTimerManager().SetTimer(
			RushSmokeExitHandle,
			FTimerDelegate::CreateLambda([]()
			{
				UE_LOG(LogTemp, Display, TEXT("[RushSmoke] Completed B.8 Rush smoke automation."));
				FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("T66RushSmokeComplete"));
			}),
			10.5f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[RushSmoke] Lightweight Actor B.8 Rush smoke sequence armed on floor=%d hero=%s rushMob=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString(),
			*GetNameSafe(RushMob));
		return;
	}

	if (Mode == TEXT("flyingsmoke") || Mode == TEXT("lightweightactorb9"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[FlyingSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bMobFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[FlyingSmoke] Failed: no mob floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-28.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		const FVector FlyingSpawnLocation = TargetLocation + FVector(780.0f, 0.0f, 0.0f);
		const FTransform FlyingSpawnTransform(FRotator::ZeroRotator, FlyingSpawnLocation);
		AT66MobBase* FlyingMob = World->SpawnActorDeferred<AT66MobBase>(
			AT66MobBase::StaticClass(),
			FlyingSpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (FlyingMob)
		{
			FlyingMob->Tags.AddUnique(FName(TEXT("T66_TestMob")));
			FlyingMob->MobID = FName(TEXT("CaveBat"));
			FlyingMob->CharacterVisualID = FName(TEXT("CaveBat"));
			FlyingMob->LifecycleState = ET66MobLifecycleState::Active;
			UGameplayStatics::FinishSpawningActor(FlyingMob, FlyingSpawnTransform);

			int32 StageNum = 1;
			float DifficultyScalar = 1.f;
			float EnemyProgressionScalar = 1.f;
			float FinaleScalar = 1.f;
			if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
			{
				StageNum = FMath::Max(1, RunState->GetCurrentStage());
				DifficultyScalar = RunState->GetDifficultyScalar();
				FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
			}
			FlyingMob->ConfigureAsMob(FName(TEXT("CaveBat")), ET66EnemyFamily::Flying, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
			FlyingMob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 8.f);
		}

		TWeakObjectPtr<AT66MobBase> WeakFlyingMob(FlyingMob);
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		const float LogTimes[] = { 0.35f, 0.95f, 1.45f, 2.05f, 2.65f, 3.25f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle FlyingLogHandle;
			const float LogTime = LogTimes[Index];
			World->GetTimerManager().SetTimer(
				FlyingLogHandle,
				FTimerDelegate::CreateLambda([WeakFlyingMob, LogTime]()
				{
					const AT66MobBase* Mob = WeakFlyingMob.Get();
					if (!Mob)
					{
						UE_LOG(LogTemp, Warning, TEXT("[FlyingSmoke] t=%.2fs test Flying mob missing."), LogTime);
						return;
					}

					UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] t=%.2fs MobID=%s loc=%s velocity=%s hoverAnchorZ=%.1f hoverBobTime=%.2f status=%s"),
						LogTime,
						Mob->MobID.IsNone() ? TEXT("unset") : *Mob->MobID.ToString(),
						*Mob->GetActorLocation().ToCompactString(),
						*Mob->StoredVelocity.ToCompactString(),
						Mob->HoverAnchorZ,
						Mob->HoverBobTime,
						Mob->FreezeSecondsRemaining > 0.f || Mob->StunSecondsRemaining > 0.f || Mob->RootSecondsRemaining > 0.f ? TEXT("Blocked") : TEXT("Active"));
				}),
				LogTime,
				false);
		}

		FTimerHandle StunHandle;
		World->GetTimerManager().SetTimer(
			StunHandle,
			FTimerDelegate::CreateLambda([WeakFlyingMob]()
			{
				if (AT66MobBase* Mob = WeakFlyingMob.Get())
				{
					Mob->ApplyStun(3.0f);
					UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] Applied 3.0s stun to Flying mob; hover and chase should pause while status blocks movement."));
				}
			}),
			3.5f,
			false);

		FTimerHandle DamageHandle;
		World->GetTimerManager().SetTimer(
			DamageHandle,
			FTimerDelegate::CreateLambda([WeakFlyingMob]()
			{
				if (AT66MobBase* Mob = WeakFlyingMob.Get())
				{
					const FT66CombatTargetHandle TargetHandle = Mob->ResolveCombatTargetHandle();
					Mob->TakeDamageFromHeroHitZone(9999, TargetHandle, FName(TEXT("FlyingSmoke")), FName(TEXT("AutomationKill")));
					UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] Applied automation lethal damage to Flying mob; death path should release it."));
				}
			}),
			7.0f,
			false);

		FTimerHandle DirectorCountHandle;
		World->GetTimerManager().SetTimer(
			DirectorCountHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
				const AT66EnemyDirector* Director = SmokeGameMode ? SmokeGameMode->GetEnemyDirectorForDiagnostics() : nullptr;
				UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] directorCounts rich=%d lightweight=%d lightweightMelee=%d lightweightRush=%d lightweightFlying=%d lightweightRanged=%d."),
					Director ? Director->GetAliveRichEnemyCount() : -1,
					Director ? Director->GetAliveLightweightMobCount() : -1,
					Director ? Director->GetAliveLightweightMeleeMobCount() : -1,
					Director ? Director->GetAliveLightweightRushMobCount() : -1,
					Director ? Director->GetAliveLightweightFlyingMobCount() : -1,
					Director ? Director->GetAliveLightweightRangedMobCount() : -1);
			}),
			8.5f,
			false);

		FTimerHandle FlyingSmokeExitHandle;
		World->GetTimerManager().SetTimer(
			FlyingSmokeExitHandle,
			FTimerDelegate::CreateLambda([]()
			{
				UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] Completed B.9 Flying smoke automation."));
				FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("T66FlyingSmokeComplete"));
			}),
			11.0f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[FlyingSmoke] Lightweight Actor B.9 Flying smoke sequence armed on floor=%d hero=%s flyingMob=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString(),
			*GetNameSafe(FlyingMob));
		return;
	}

	if (Mode == TEXT("rangedsmoke") || Mode == TEXT("lightweightactorb10"))
	{
		int32 RangedSmokeUseLightweight = 1;
		FParse::Value(FCommandLine::Get(), TEXT("T66MobUseLightweight="), RangedSmokeUseLightweight);
		const bool bUseLightweightRangedSmoke = RangedSmokeUseLightweight != 0;

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bMobFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] Failed: no mob floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-28.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		float InitialHeroHP = -1.f;
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
			InitialHeroHP = RunState->GetCurrentHP();
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			MobManager->ResetRangedPressureDiagnostics(TEXT("RangedSmokeStart"));
		}
		if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
		{
			ProjectileManager->ResetProjectileDiagnostics(TEXT("RangedSmokeStart"));
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		const FVector RangedSpawnLocation = TargetLocation + FVector(1000.0f, 0.0f, 0.0f);
		const FTransform RangedSpawnTransform(FRotator::ZeroRotator, RangedSpawnLocation);
		int32 StageNum = 1;
		float DifficultyScalar = 1.f;
		float EnemyProgressionScalar = 1.f;
		float FinaleScalar = 1.f;
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			StageNum = FMath::Max(1, RunState->GetCurrentStage());
			DifficultyScalar = RunState->GetDifficultyScalar();
			FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
		}
		AT66MobBase* RangedMob = nullptr;
		AT66RangedEnemy* RichRangedEnemy = nullptr;
		AActor* RangedSmokeActor = nullptr;
		if (bUseLightweightRangedSmoke)
		{
			RangedMob = World->SpawnActorDeferred<AT66MobBase>(
				AT66MobBase::StaticClass(),
				RangedSpawnTransform,
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (RangedMob)
			{
				RangedMob->Tags.AddUnique(FName(TEXT("T66_TestMob")));
				RangedMob->MobID = FName(TEXT("HexSlinger"));
				RangedMob->CharacterVisualID = FName(TEXT("HexSlinger"));
				RangedMob->LifecycleState = ET66MobLifecycleState::Active;
				UGameplayStatics::FinishSpawningActor(RangedMob, RangedSpawnTransform);
				RangedMob->ConfigureAsMob(FName(TEXT("HexSlinger")), ET66EnemyFamily::Ranged, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
				RangedMob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 8.f);
				if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
				{
					MobManager->RecordRouteAttribution(
						RangedMob->GetEnemyFamily(),
						ET66RouteAttributionReason::RoutedLightweight_BasicFamily,
						ET66RouteAttributionChannel::NonDirector);
				}
				RangedSmokeActor = RangedMob;
			}
		}
		else
		{
			RichRangedEnemy = World->SpawnActorDeferred<AT66RangedEnemy>(
				AT66RangedEnemy::StaticClass(),
				RangedSpawnTransform,
				nullptr,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (RichRangedEnemy)
			{
				RichRangedEnemy->Tags.AddUnique(FName(TEXT("T66_TestMob")));
				RichRangedEnemy->MobID = FName(TEXT("HexSlinger"));
				RichRangedEnemy->CharacterVisualID = FName(TEXT("HexSlinger"));
				RichRangedEnemy->EnemyFamily = ET66EnemyFamily::Ranged;
				UGameplayStatics::FinishSpawningActor(RichRangedEnemy, RangedSpawnTransform);
				RichRangedEnemy->ConfigureAsMob(FName(TEXT("HexSlinger")));
				RichRangedEnemy->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 8.f);
				T66RecordNonDirectorRouteAttribution(World, RichRangedEnemy);
				RangedSmokeActor = RichRangedEnemy;
			}
		}

		TWeakObjectPtr<AT66MobBase> WeakRangedMob(RangedMob);
		TWeakObjectPtr<AT66RangedEnemy> WeakRichRangedEnemy(RichRangedEnemy);
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		const float LogTimes[] = { 0.35f, 0.85f, 1.35f, 2.15f, 3.0f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle RangedLogHandle;
			const float LogTime = LogTimes[Index];
			if (bUseLightweightRangedSmoke)
			{
				World->GetTimerManager().SetTimer(
					RangedLogHandle,
					FTimerDelegate::CreateLambda([WeakRangedMob, LogTime]()
					{
						const AT66MobBase* Mob = WeakRangedMob.Get();
						if (Mob)
						{
							UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] t=%.2fs MobID=%s path=Lightweight loc=%s velocity=%s fireCooldown=%.2f minRange=%.1f maxRange=%.1f projectileHeight=%.1f"),
								LogTime,
								Mob->MobID.IsNone() ? TEXT("unset") : *Mob->MobID.ToString(),
								*Mob->GetActorLocation().ToCompactString(),
								*Mob->StoredVelocity.ToCompactString(),
								Mob->FireCooldownRemaining,
								Mob->DesiredMinRange,
								Mob->DesiredMaxRange,
								Mob->ProjectileSpawnHeight);
							return;
						}
						UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] t=%.2fs lightweight test Ranged mob missing."), LogTime);
					}),
					LogTime,
					false);
			}
			else
			{
				World->GetTimerManager().SetTimer(
					RangedLogHandle,
					FTimerDelegate::CreateLambda([WeakRichRangedEnemy, LogTime]()
					{
					const AT66RangedEnemy* Enemy = WeakRichRangedEnemy.Get();
					if (!Enemy)
					{
						UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] t=%.2fs rich test Ranged enemy missing."), LogTime);
						return;
					}
					UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] t=%.2fs MobID=%s path=Rich loc=%s class=%s"),
						LogTime,
						Enemy->MobID.IsNone() ? TEXT("unset") : *Enemy->MobID.ToString(),
						*Enemy->GetActorLocation().ToCompactString(),
						*GetNameSafe(Enemy->GetClass()));
					}),
					LogTime,
					false);
			}
		}

		FTimerHandle ProjectileAssertionHandle;
		World->GetTimerManager().SetTimer(
			ProjectileAssertionHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				const UT66ProjectileManagerSubsystem* ProjectileManager = SmokeWorld ? SmokeWorld->GetSubsystem<UT66ProjectileManagerSubsystem>() : nullptr;
				if (!SmokeWorld || !ProjectileManager)
				{
					UE_LOG(LogTemp, Warning, TEXT("[RangedSmoke] ProjectileTravelAssertion skipped: missing world or projectile manager."));
					return;
				}

				const FT66ProjectileManagerDiagnostics& Diagnostics = ProjectileManager->GetDiagnostics();
				UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] ProjectileTravelAssertion managerFired=%d active=%d hitHero=%d hitWorld=%d expired=%d result=%s"),
					Diagnostics.ProjectilesFired,
					ProjectileManager->GetActiveProjectileCount(),
					Diagnostics.ProjectilesHitHero,
					Diagnostics.ProjectilesHitWorld,
					Diagnostics.ProjectilesExpired,
					(Diagnostics.ProjectilesFired > 0) ? TEXT("PASS") : TEXT("CHECK"));
			}),
			0.85f,
			false);

		FTimerHandle HeroDamageHandle;
		World->GetTimerManager().SetTimer(
			HeroDamageHandle,
			FTimerDelegate::CreateLambda([WeakThis, InitialHeroHP]()
			{
				const AT66PlayerController* PC = WeakThis.Get();
				UGameInstance* GameInstance = PC ? PC->GetGameInstance() : nullptr;
				const UT66RunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
				const float CurrentHP = RunState ? RunState->GetCurrentHP() : -1.f;
				UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] HeroDamageAssertion initialHP=%.1f currentHP=%.1f result=%s"),
					InitialHeroHP,
					CurrentHP,
					(CurrentHP >= 0.f && InitialHeroHP >= 0.f && CurrentHP < InitialHeroHP) ? TEXT("PASS") : TEXT("CHECK"));
			}),
			1.45f,
			false);

		FTimerHandle DamageHandle;
		World->GetTimerManager().SetTimer(
			DamageHandle,
			FTimerDelegate::CreateLambda([WeakRangedMob, WeakRichRangedEnemy]()
			{
				if (AT66MobBase* Mob = WeakRangedMob.Get())
				{
					const FT66CombatTargetHandle TargetHandle = Mob->ResolveCombatTargetHandle();
					Mob->TakeDamageFromHeroHitZone(9999, TargetHandle, FName(TEXT("RangedSmoke")), FName(TEXT("AutomationKill")));
					UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] Applied automation lethal damage to Ranged mob; death path should release it."));
					return;
				}
				if (AT66RangedEnemy* Enemy = WeakRichRangedEnemy.Get())
				{
					const FT66CombatTargetHandle TargetHandle = Enemy->ResolveCombatTargetHandle();
					Enemy->TakeDamageFromHeroHitZone(9999, TargetHandle, FName(TEXT("RangedSmoke")), FName(TEXT("AutomationKill")));
					UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] Applied automation lethal damage to rich Ranged enemy."));
				}
			}),
			3.5f,
			false);

		FTimerHandle DirectorCountHandle;
		World->GetTimerManager().SetTimer(
			DirectorCountHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
				AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
				const AT66EnemyDirector* Director = SmokeGameMode ? SmokeGameMode->GetEnemyDirectorForDiagnostics() : nullptr;
				UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] directorCounts rich=%d lightweight=%d lightweightMelee=%d lightweightRush=%d lightweightFlying=%d lightweightRanged=%d."),
					Director ? Director->GetAliveRichEnemyCount() : -1,
					Director ? Director->GetAliveLightweightMobCount() : -1,
					Director ? Director->GetAliveLightweightMeleeMobCount() : -1,
					Director ? Director->GetAliveLightweightRushMobCount() : -1,
					Director ? Director->GetAliveLightweightFlyingMobCount() : -1,
					Director ? Director->GetAliveLightweightRangedMobCount() : -1);
			}),
			4.0f,
			false);

		FTimerHandle RangedSmokeExitHandle;
		World->GetTimerManager().SetTimer(
			RangedSmokeExitHandle,
			FTimerDelegate::CreateLambda([]()
			{
				UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] Completed B.10 Ranged smoke automation."));
				FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("T66RangedSmokeComplete"));
			}),
			5.5f,
			false);

		UE_LOG(LogTemp, Display, TEXT("[RangedSmoke] B.10.1D Ranged smoke sequence armed path=%s floor=%d hero=%s rangedMob=%s."),
			bUseLightweightRangedSmoke ? TEXT("Lightweight") : TEXT("Rich"),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString(),
			*GetNameSafe(RangedSmokeActor));
		return;
	}

	if (Mode == TEXT("mobpoolhudsmoke") || Mode == TEXT("lightweightactorb7"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobPoolHudSmoke] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bMobFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MobPoolHudSmoke] Failed: no mob floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-28.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}

		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		auto LogPoolHudState = [WeakThis](const TCHAR* Label)
		{
			AT66PlayerController* PC = WeakThis.Get();
			UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
			UT66MobManagerSubsystem* Manager = SmokeWorld ? SmokeWorld->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
			UT66ActorRegistrySubsystem* Registry = SmokeWorld ? SmokeWorld->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
			AT66GameMode* SmokeGameMode = SmokeWorld ? Cast<AT66GameMode>(SmokeWorld->GetAuthGameMode()) : nullptr;
			AT66EnemyDirector* Director = SmokeGameMode ? SmokeGameMode->GetEnemyDirectorForDiagnostics() : nullptr;
			if (!Manager || !Registry)
			{
				UE_LOG(LogTemp, Warning, TEXT("[MobPoolHudSmoke] %s missing manager or registry."), Label);
				return;
			}

			UE_LOG(LogTemp, Display, TEXT("[MobPoolHudSmoke] %s combined=%d registryLightweight=%d directorRich=%d directorLightweight=%d activeMobs=%d inactiveMobs=%d reuse=%d releases=%d inactivePeak=%d"),
				Label,
				Registry->GetCombinedLiveEnemyCount(),
				Registry->GetLiveMobCount(),
				Director ? Director->GetAliveRichEnemyCount() : -1,
				Director ? Director->GetAliveLightweightMobCount() : -1,
				Manager->GetActiveMobs().Num(),
				Manager->GetInactiveMobCount(),
				Manager->GetPoolReuseAcquireCount(),
				Manager->GetPoolReleaseCount(),
				Manager->GetPeakInactiveMobCount());

			if (PC->GameplayHUDWidget)
			{
				PC->GameplayHUDWidget->RefreshHUD();
			}
		};

		auto KillSomeLightweightMobs = [WeakThis](const int32 MaxKills)
		{
			AT66PlayerController* PC = WeakThis.Get();
			UWorld* SmokeWorld = PC ? PC->GetWorld() : nullptr;
			UT66MobManagerSubsystem* Manager = SmokeWorld ? SmokeWorld->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
			if (!Manager)
			{
				return;
			}

			TArray<TWeakObjectPtr<AT66MobBase>> Mobs = Manager->GetActiveMobs();
			int32 KilledCount = 0;
			for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Mobs)
			{
				AT66MobBase* Mob = WeakMob.Get();
				if (!Mob || !Mob->IsAliveAndActive())
				{
					continue;
				}

				Mob->TakeDamageFromHeroHitZone(
					FMath::CeilToInt(Mob->GetCurrentHP()),
					Mob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body),
					FName(TEXT("B7PoolHudSmoke")),
					NAME_None);
				++KilledCount;
				if (KilledCount >= MaxKills)
				{
					break;
				}
			}

			UE_LOG(LogTemp, Display, TEXT("[MobPoolHudSmoke] Force-killed %d lightweight mob(s) to exercise pool release/reuse."), KilledCount);
		};

		const float KillTimes[] = { 14.0f, 28.0f, 42.0f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(KillTimes); ++Index)
		{
			FTimerHandle KillHandle;
			World->GetTimerManager().SetTimer(
				KillHandle,
				FTimerDelegate::CreateLambda([KillSomeLightweightMobs]()
				{
					KillSomeLightweightMobs(10);
				}),
				KillTimes[Index],
				false);
		}

		const float LogTimes[] = { 10.0f, 25.0f, 40.0f, 55.0f, 65.0f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle LogHandle;
			FString Label = FString::Printf(TEXT("t%.0fs"), LogTimes[Index]);
			World->GetTimerManager().SetTimer(
				LogHandle,
				FTimerDelegate::CreateLambda([LogPoolHudState, Label]()
				{
					LogPoolHudState(*Label);
				}),
				LogTimes[Index],
				false);
		}

		UE_LOG(LogTemp, Display, TEXT("[MobPoolHudSmoke] Lightweight Actor B.7 pool/HUD smoke sequence armed on floor=%d location=%s."),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString());
		return;
	}

	if (Mode == TEXT("enemywaveperf") || Mode == TEXT("mainboardenemywave") || Mode == TEXT("phaseaperf"))
	{
		int32 ManagerTickProfileOverride = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66MobManagerTickProfileEnabled="), ManagerTickProfileOverride))
		{
			if (IConsoleVariable* ManagerTickProfileCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Mob.ManagerTickProfileEnabled")))
			{
				ManagerTickProfileCVar->Set(ManagerTickProfileOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Mob.ManagerTickProfileEnabled set to %d from command line."), ManagerTickProfileOverride != 0 ? 1 : 0);
			}
		}
		int32 RangedDiagnosticLoggingOverride = 0;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66RangedDiagnosticLogging="), RangedDiagnosticLoggingOverride))
		{
			if (IConsoleVariable* RangedDiagnosticLoggingCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Ranged.DiagnosticLogging")))
			{
				RangedDiagnosticLoggingCVar->Set(RangedDiagnosticLoggingOverride != 0 ? 1 : 0, ECVF_SetByCommandline);
				UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.Ranged.DiagnosticLogging set to %d from command line."), RangedDiagnosticLoggingOverride != 0 ? 1 : 0);
			}
		}
		float AutoCaptureHeroHPOverride = 0.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66AutoCaptureHeroHPOverride="), AutoCaptureHeroHPOverride))
		{
			CVarT66AutoCaptureHeroHPOverride.AsVariable()->Set(AutoCaptureHeroHPOverride, ECVF_SetByCommandline);
			UE_LOG(LogTemp, Display, TEXT("[PerfAutomation] T66.AutoCapture.HeroHPOverride requested %.1f from command line."), AutoCaptureHeroHPOverride);
		}
		auto ReadIntCVar = [](const TCHAR* CVarName, const int32 FallbackValue)
		{
			if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarName))
			{
				return CVar->GetInt();
			}
			return FallbackValue;
		};
		auto ReadFloatCVar = [](const TCHAR* CVarName, const float FallbackValue)
		{
			if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(CVarName))
			{
				return CVar->GetFloat();
			}
			return FallbackValue;
		};
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[PerfAutomation] MobRoutingFlags ManagerTickProfile=%d RangedDiagnosticLogging=%d HeroHPOverride=%.1f"),
			ReadIntCVar(TEXT("T66.Mob.ManagerTickProfileEnabled"), 0),
			ReadIntCVar(TEXT("T66.Ranged.DiagnosticLogging"), 0),
			ReadFloatCVar(TEXT("T66.AutoCapture.HeroHPOverride"), 0.0f));
		if (UWorld* ResetWorld = GetWorld())
		{
			if (UT66MobManagerSubsystem* MobManager = ResetWorld->GetSubsystem<UT66MobManagerSubsystem>())
			{
				MobManager->ResetRangedPressureDiagnostics(TEXT("EnemyWavePerfStart"));
			}
			if (UT66ProjectileManagerSubsystem* ProjectileManager = ResetWorld->GetSubsystem<UT66ProjectileManagerSubsystem>())
			{
				ProjectileManager->ResetProjectileDiagnostics(TEXT("EnemyWavePerfStart"));
			}
		}

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
		}

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[PerfAutomation] Enemy wave perf mode failed: missing world, pawn, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bMobFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PerfAutomation] Enemy wave perf mode failed: no mob floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
		{
			if (const UCapsuleComponent* Capsule = CharacterPawn->GetCapsuleComponent())
			{
				PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			}
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
		{
			if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}
		}
		ControlledPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		ControlledPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(-35.0f, 0.0f, 0.0f));

		GameMode->HandleTowerDescentHoleTriggered(ControlledPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(false);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(true);
			const float RequestedHeroHPOverride = CVarT66AutoCaptureHeroHPOverride.GetValueOnGameThread();
			if (RequestedHeroHPOverride > 0.0f)
			{
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[PerfAutomation] AutoCaptureHeroHPOverrideSnapshot Phase=Before RequestedHP=%.1f MaxHP=%.1f CurrentHP=%.1f CurrentHearts=%d MaxHearts=%d Gold=%d Debt=%d StageTimerActive=%d"),
					RequestedHeroHPOverride,
					RunState->GetMaxHP(),
					RunState->GetCurrentHP(),
					RunState->GetCurrentHearts(),
					RunState->GetMaxHearts(),
					RunState->GetCurrentGold(),
					RunState->GetCurrentDebt(),
					RunState->GetStageTimerActive() ? 1 : 0);
				const float AppliedHeroHPOverride = RunState->ApplyAutomationHeroHPOverride(RequestedHeroHPOverride, *Mode);
				UE_LOG(
					LogTemp,
					Display,
					TEXT("AutoCapture hero HP override applied: starting HP = %.1f"),
					AppliedHeroHPOverride);
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[PerfAutomation] AutoCaptureHeroHPOverride AppliedHP=%.1f RequestedHP=%.1f MaxHP=%.1f CurrentHP=%.1f CurrentHearts=%d MaxHearts=%d"),
					AppliedHeroHPOverride,
					RequestedHeroHPOverride,
					RunState->GetMaxHP(),
					RunState->GetCurrentHP(),
					RunState->GetCurrentHearts(),
					RunState->GetMaxHearts());
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[PerfAutomation] AutoCaptureHeroHPOverrideSnapshot Phase=After AppliedHP=%.1f RequestedHP=%.1f MaxHP=%.1f CurrentHP=%.1f CurrentHearts=%d MaxHearts=%d Gold=%d Debt=%d StageTimerActive=%d"),
					AppliedHeroHPOverride,
					RequestedHeroHPOverride,
					RunState->GetMaxHP(),
					RunState->GetCurrentHP(),
					RunState->GetCurrentHearts(),
					RunState->GetMaxHearts(),
					RunState->GetCurrentGold(),
					RunState->GetCurrentDebt(),
					RunState->GetStageTimerActive() ? 1 : 0);
			}
		}
		if (AT66EnemyDirector* EnemyDirector = GameMode->GetEnemyDirectorForDiagnostics())
		{
			EnemyDirector->RefreshSpawningFromProgression();
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->RefreshHUD();
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[PerfAutomation] Enemy wave perf mode floor=%d location=%s."),
			TargetFloor->FloorNumber,
			*ControlledPawn->GetActorLocation().ToCompactString());
		return;
	}
#endif

	if (Mode == TEXT("heroqa") || Mode == TEXT("herovisualqa") || Mode == TEXT("playableheroqa"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
				}
			}

			ControlledPawn->SetActorRotation(FRotator(0.f, 180.f, 0.f));
			SetControlRotation(FRotator(-8.f, 180.f, 0.f));

			if (AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn))
			{
				if (HeroPawn->CameraBoom)
				{
					HeroPawn->CameraBoom->TargetArmLength = 420.f;
					HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 95.f));
					HeroPawn->CameraBoom->bDoCollisionTest = false;
				}
			}
		}
		return;
	}

	if (Mode == TEXT("camerawallocclusion") || Mode == TEXT("wallocclusion") || Mode == TEXT("wallfade"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		UPrimitiveComponent* WallComponent = nullptr;
		if (World && HeroPawn)
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* CandidateActor = *It;
				if (!CandidateActor
					|| !CandidateActor->ActorHasTag(T66GameplayAutomationTerrainVisualTag)
					|| !CandidateActor->ActorHasTag(T66GameplayAutomationTraversalBarrierTag)
					|| !CandidateActor->ActorHasTag(T66GameplayAutomationCameraWallVisualTag)
					|| CandidateActor->ActorHasTag(T66GameplayAutomationTowerCeilingTag))
				{
					continue;
				}

				TArray<UPrimitiveComponent*> Components;
				CandidateActor->GetComponents<UPrimitiveComponent>(Components);
				for (UPrimitiveComponent* Component : Components)
				{
					if (!Component || !Component->IsRegistered() || !Component->IsVisible())
					{
						continue;
					}

					const FVector Extents = Component->Bounds.BoxExtent;
					const float ThinExtent = FMath::Min(Extents.X, Extents.Y);
					const float LongExtent = FMath::Max(Extents.X, Extents.Y);
					if ((CandidateActor->ActorHasTag(T66GameplayAutomationCameraWallVisualTag)
							|| Component->ComponentHasTag(T66GameplayAutomationCameraWallVisualTag))
						&& Extents.Z >= 300.f
						&& ThinExtent >= 20.f
						&& ThinExtent <= 650.f
						&& LongExtent >= 120.f
						&& LongExtent <= 9000.f)
					{
						WallComponent = Component;
						break;
					}
				}

				if (WallComponent)
				{
					break;
				}
			}
		}

		if (WallComponent && HeroPawn)
		{
			if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}

			const FBoxSphereBounds Bounds = WallComponent->Bounds;
			const FVector Extents = Bounds.BoxExtent;
			const bool bWallRunsAlongX = Extents.X >= Extents.Y;
			const float ThinExtent = bWallRunsAlongX ? Extents.Y : Extents.X;
			const FVector WallNormal = bWallRunsAlongX ? FVector(0.f, 1.f, 0.f) : FVector(1.f, 0.f, 0.f);
			const float CapsuleHalfHeight = HeroPawn->GetCapsuleComponent()
				? HeroPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
				: 100.f;
			FVector HeroLocation = Bounds.Origin - WallNormal * (ThinExtent + 170.f);
			HeroLocation.Z = Bounds.Origin.Z - Extents.Z + CapsuleHalfHeight + 12.f;
			HeroPawn->SetActorLocation(HeroLocation, false, nullptr, ETeleportType::TeleportPhysics);
			HeroPawn->SetActorRotation((-WallNormal).Rotation(), ETeleportType::TeleportPhysics);

			if (HeroPawn->CameraBoom)
			{
				HeroPawn->CameraBoom->TargetArmLength = ThinExtent + 700.f;
				HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 140.f));
				HeroPawn->CameraBoom->bDoCollisionTest = false;
			}

			SetControlRotation((-WallNormal + FVector(0.f, 0.f, -0.18f)).Rotation());
			UpdateGameplayCameraWallOcclusion(0.1f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Gameplay automation wall occlusion capture could not find a tagged tower wall visual."));
		}
		return;
	}

	if (Mode == TEXT("inventory") || Mode == TEXT("inspect") || Mode == TEXT("inventoryinspect"))
	{
		SetInventoryInspectOpen(true);
		return;
	}

	if (Mode == TEXT("fullmap") || Mode == TEXT("map"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(true);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("worldprompt") || Mode == TEXT("interactable") || Mode == TEXT("interactionprompt"))
	{
		AT66ChestInteractable* AutomationPromptActor = nullptr;
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66WidgetDump_WorldInteractablePrompt"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AutomationPromptActor = World->SpawnActor<AT66ChestInteractable>(
				AT66ChestInteractable::StaticClass(),
				GetPawn() ? GetPawn()->GetActorLocation() + FVector(260.f, 0.f, 0.f) : FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams);
			if (AutomationPromptActor)
			{
				AutomationPromptActor->SetActorHiddenInGame(true);
				AutomationPromptActor->SetActorEnableCollision(false);
				AutomationPromptActor->SetShowcaseReusable(true);
			}
		}

		if (GameplayHUDWidget)
		{
			AActor* PromptSourceActor = AutomationPromptActor ? static_cast<AActor*>(AutomationPromptActor) : static_cast<AActor*>(this);
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->ShowInteractionPrompt(
				PromptSourceActor,
				NSLOCTEXT("T66.GameplayHUD", "WidgetDumpWorldPromptTarget", "Chest"));
			GameplayHUDWidget->RefreshHUD();
		}
		return;
	}

#if !UE_BUILD_SHIPPING
	if (Mode == TEXT("combatdamagelog") || Mode == TEXT("damagelog"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->ApplyDamage(20, this, FName(TEXT("DebugDamage")), this);
		}
		return;
	}

	if (Mode == TEXT("hero1axeaoe") || Mode == TEXT("vfxlabhero1axeaoe") || Mode == TEXT("hero1axeaoehitbox") || Mode == TEXT("hero1axeaoevfxbinding") || Mode == TEXT("hero1axepiercevfxbinding") || Mode == TEXT("hero1axebouncevfxbinding") || Mode == TEXT("hero1axedotvfxbinding") || Mode == TEXT("hero1axeaoewateridolimpact"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		bGameplayAutomationCameraZoomLocked = FParse::Param(FCommandLine::Get(), TEXT("T66GameplayAutoLockCameraZoom"));

		if (UWorld* World = GetWorld())
		{
			const bool bAoeVFXBindingProofMode = Mode == TEXT("hero1axeaoevfxbinding");
			const bool bPierceVFXBindingProofMode = Mode == TEXT("hero1axepiercevfxbinding");
			const bool bBounceVFXBindingProofMode = Mode == TEXT("hero1axebouncevfxbinding");
			// DOT is a temporary placeholder structure proof: it equips the DOT proof weapon and
			// fires through the normal auto-attack path, but is intentionally NOT a
			// bProductionVFXBindingProofMode member because it has no production Niagara binding or
			// item/stat gate yet. It reuses the hitbox-proof target/equip scaffolding only.
			const bool bDotVFXBindingProofMode = Mode == TEXT("hero1axedotvfxbinding");
			const bool bProductionVFXBindingProofMode = bAoeVFXBindingProofMode || bPierceVFXBindingProofMode || bBounceVFXBindingProofMode;
			const bool bWaterIdolImpactProofMode = Mode == TEXT("hero1axeaoewateridolimpact");
			const bool bHitboxProofMode = Mode == TEXT("hero1axeaoehitbox") || bProductionVFXBindingProofMode || bDotVFXBindingProofMode || bWaterIdolImpactProofMode;
			static const FName Hero1AxeTargetTag(TEXT("T66Automation_Hero1AxeAOETarget"));
			if (AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode()))
			{
				GameMode->SetEnemyDirectorSpawningPaused(true);
			}
			int32 ClearedEnemyCount = 0;
			for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
			{
				It->Destroy();
				++ClearedEnemyCount;
			}
			UE_LOG(LogTemp, Display, TEXT("Gameplay automation: cleared %d existing enemies for Hero1AxeAOE preview."), ClearedEnemyCount);

			if (!bHitboxProofMode)
			{
				for (TActorIterator<AT66Hero1AxeAOEVFXLabActor> It(World); It; ++It)
				{
					UE_LOG(LogTemp, Display, TEXT("Gameplay automation: Hero1AxeAOE lab actor already present: %s"), *It->GetName());
					return;
				}
			}

			APawn* ControlledPawn = GetPawn();
			AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
			if (HeroPawn)
			{
				if (FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOECenterPlayer")))
				{
					if (AT66GameMode* GameMode = Cast<AT66GameMode>(World->GetAuthGameMode()))
					{
						T66TowerMapTerrain::FLayout TowerLayout;
						if (GameMode->IsUsingTowerMainMapLayout() && GameMode->GetTowerMainMapLayout(TowerLayout))
						{
							const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
							for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
							{
								if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
								{
									TargetFloor = &Floor;
									break;
								}
								if (!TargetFloor && Floor.bMobFloor)
								{
									TargetFloor = &Floor;
								}
							}

							if (TargetFloor)
							{
								float PawnHalfHeight = 100.0f;
								if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
								{
									PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
								}

								FVector TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f);
								if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
								{
									Movement->StopMovementImmediately();
								}
								HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
								HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
								SetControlRotation(FRotator(-30.0f, 0.0f, 0.0f));
								UE_LOG(LogTemp, Display, TEXT("Gameplay automation: centered Hero1AxeAOE player at %s"), *HeroPawn->GetActorLocation().ToCompactString());
							}
						}
					}
				}

				float RequestedCameraArmLength = 0.0f;
				if (FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoCameraArmLength="), RequestedCameraArmLength)
					&& HeroPawn->CameraBoom)
				{
					DesiredGameplayCameraArmLength = FMath::Max(100.0f, RequestedCameraArmLength);
					HeroPawn->CameraBoom->TargetArmLength = DesiredGameplayCameraArmLength;
				}
			}

			if (bHitboxProofMode)
			{
				if (!HeroPawn || !HeroPawn->CombatComponent)
				{
					UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEHitboxProof] Failed: missing hero or combat component."));
					return;
				}

				static const FName HitboxProofTag(TEXT("T66Automation_Hero1AxeAOEHitboxProof"));
				for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
				{
					if (It->Tags.Contains(HitboxProofTag) || It->Tags.Contains(Hero1AxeTargetTag))
					{
						It->Destroy();
					}
				}
				for (TActorIterator<AT66Hero1AxeAOEVFXLabActor> It(World); It; ++It)
				{
					It->Destroy();
				}

				FName HeroID = HeroPawn->HeroID.IsNone() ? FName(TEXT("Hero_1")) : HeroPawn->HeroID;
				ET66AttackCategory ProofWeaponCategory = ET66AttackCategory::AOE;
				if (bPierceVFXBindingProofMode)
				{
					HeroID = FName(TEXT("Hero_2"));
					ProofWeaponCategory = ET66AttackCategory::Pierce;
				}
				else if (bBounceVFXBindingProofMode)
				{
					HeroID = FName(TEXT("Hero_4"));
					ProofWeaponCategory = ET66AttackCategory::Bounce;
				}
				else if (bDotVFXBindingProofMode)
				{
					HeroID = FName(TEXT("Hero_5"));
					ProofWeaponCategory = ET66AttackCategory::DOT;
				}
				if (HeroPawn->HeroID != HeroID)
				{
					HeroPawn->HeroID = HeroID;
				}
				FName ProofWeaponID = UT66WeaponManagerSubsystem::MakeWeaponID(HeroID, ET66WeaponRarity::Black, ProofWeaponCategory);
				FString ProofWeaponOverrideString;
				const bool bProofWeaponOverride = FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEProofWeaponID="), ProofWeaponOverrideString);
				if (bProofWeaponOverride)
				{
					ProofWeaponID = FName(*ProofWeaponOverrideString.TrimStartAndEnd());
				}
				FWeaponData ProofWeaponData;
				UT66GameInstance* ProofGameInstance = Cast<UT66GameInstance>(GetGameInstance());
				const bool bHaveProofWeaponData = ProofGameInstance
					? ProofGameInstance->GetWeaponData(ProofWeaponID, ProofWeaponData)
					: false;
				if (bHaveProofWeaponData)
				{
					if (!ProofWeaponData.HeroID.IsNone())
					{
						HeroID = ProofWeaponData.HeroID;
					}
					ProofWeaponCategory = ProofWeaponData.Branch;
					if (HeroPawn->HeroID != HeroID)
					{
						HeroPawn->HeroID = HeroID;
					}
				}
				bool bSelectedProofWeapon = false;
				if (UT66WeaponManagerSubsystem* WeaponManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66WeaponManagerSubsystem>() : nullptr)
				{
					if (bProofWeaponOverride)
					{
						WeaponManager->ResetForStageWeaponSelection(HeroID);
					}
					else
					{
						WeaponManager->BuildWeaponOffers(HeroID, ET66WeaponRarity::Black);
					}
					bSelectedProofWeapon = WeaponManager->SelectWeapon(ProofWeaponID);
				}
				UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] EquippedProofWeapon=%s Success=%d HeroID=%s AttackCategory=%s PatternID=%s ProjectileCount=%d SpreadAngleDegrees=%.2f Override=%d"),
					*ProofWeaponID.ToString(),
					bSelectedProofWeapon ? 1 : 0,
					*HeroID.ToString(),
					*UEnum::GetValueAsString(ProofWeaponCategory),
					bHaveProofWeaponData ? *ProofWeaponData.AttackPatternID.ToString() : TEXT("None"),
					bHaveProofWeaponData ? ProofWeaponData.ProjectileCount : 0,
					bHaveProofWeaponData ? ProofWeaponData.SpreadAngleDegrees : 0.f,
					bProofWeaponOverride ? 1 : 0);

				FName ImpactProofIdolID = NAME_None;
				TArray<FName> PreProofEquippedIdols;
				TArray<uint8> PreProofEquippedIdolTiers;
				ET66Difficulty PreProofDifficulty = ET66Difficulty::Easy;
				bool bProofIdolStateCaptured = false;
				TWeakObjectPtr<UT66IdolManagerSubsystem> WeakProofIdolManager;
				if (bWaterIdolImpactProofMode)
				{
					FString ProofIdolString(TEXT("Idol_Ice_AOE"));
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEProofIdol="), ProofIdolString);
					ImpactProofIdolID = FName(*ProofIdolString.TrimStartAndEnd());
					// Centralized in T66CombatShared so the proof-idol allowlist cannot drift
					// from the runtime impact-presentation lane. Members: Idol_Ice_AOE (AOE,
					// preserved reference), Idol_Electricity_Pierce (Pierce), Idol_Electricity_Bounce (Bounce),
					// Idol_Nature_DOT (DOT), plus Idol_Nature_AOE (neutral/alternate control).
					if (!T66CombatShared::GetSupportedProofIdols().Contains(ImpactProofIdolID))
					{
						UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEIdolImpactProof] Unsupported proof idol %s; using Idol_Ice_AOE."), *ImpactProofIdolID.ToString());
						ImpactProofIdolID = FName(TEXT("Idol_Ice_AOE"));
					}

					if (UT66IdolManagerSubsystem* IdolManager = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr)
					{
						WeakProofIdolManager = IdolManager;
						PreProofEquippedIdols = IdolManager->GetEquippedIdols();
						PreProofEquippedIdolTiers = IdolManager->GetEquippedIdolTierValues();
						PreProofDifficulty = IdolManager->GetCurrentDifficulty();
						bProofIdolStateCaptured = true;

						TArray<FName> ProofEquippedIdols;
						ProofEquippedIdols.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
						TArray<uint8> ProofEquippedIdolTiers;
						ProofEquippedIdolTiers.SetNum(UT66IdolManagerSubsystem::MaxEquippedIdolSlots);
						for (int32 SlotIndex = 0; SlotIndex < UT66IdolManagerSubsystem::MaxEquippedIdolSlots; ++SlotIndex)
						{
							ProofEquippedIdols[SlotIndex] = NAME_None;
							ProofEquippedIdolTiers[SlotIndex] = 0;
						}
						ProofEquippedIdols[0] = ImpactProofIdolID;
						ProofEquippedIdolTiers[0] = 1;
						IdolManager->RestoreState(ProofEquippedIdols, ProofEquippedIdolTiers, PreProofDifficulty);
						if (UGameInstance* GI = GetGameInstance())
						{
							if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
							{
								DamageLog->ResetForNewRun();
							}
						}
						UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEIdolImpactProof] EquippedProofIdol=%s CapturedPreviousCount=%d ResetDamageLog=1"),
							*ImpactProofIdolID.ToString(),
							PreProofEquippedIdols.Num());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEIdolImpactProof] Missing IdolManager; proof idol not equipped."));
					}
				}
				if (bWaterIdolImpactProofMode)
				{
					if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
					{
						const float AppliedProofHP = RunState->ApplyAutomationHeroHPOverride(20000.f, TEXT("Hero1AxeAOEIdolImpactProof"));
						UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEIdolImpactProof] AppliedProofHeroHP=%.1f"), AppliedProofHP);
					}
				}

				if (bAoeVFXBindingProofMode)
				{
					if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
					{
						if (!FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOEKeepProofInventory")))
						{
							RunState->ClearInventory();
						}

						FString ProofItemsString;
						FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEProofItems="), ProofItemsString);
						if (ProofItemsString.TrimStartAndEnd().IsEmpty())
						{
							FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEProofItem="), ProofItemsString);
						}

						int32 ProofLine1Roll = 8;
						FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEProofLine1="), ProofLine1Roll);
						ProofLine1Roll = FMath::Clamp(ProofLine1Roll, 1, 99);

						int32 ProofSecondaryBonus = 1;
						FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEProofSecondary="), ProofSecondaryBonus);
						ProofSecondaryBonus = FMath::Clamp(ProofSecondaryBonus, 0, 99);

						TArray<FString> ProofItemIDs;
						ProofItemsString.ParseIntoArray(ProofItemIDs, TEXT(","), true);
						int32 ProofGrantIndex = 0;
						for (FString ProofItemIDString : ProofItemIDs)
						{
							ProofItemIDString = ProofItemIDString.TrimStartAndEnd();
							if (ProofItemIDString.IsEmpty())
							{
								continue;
							}

							const FName ProofItemID(*ProofItemIDString);
							RunState->AddItemSlot(FT66InventorySlot(
								ProofItemID,
								ET66ItemRarity::Black,
								ProofLine1Roll,
								0.f,
								ProofSecondaryBonus,
								660100 + ProofGrantIndex));
							UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEVFXBindingProof] GrantedItem=%s Rarity=Black Line1=%d Secondary=%d"),
								*ProofItemID.ToString(),
								ProofLine1Roll,
								ProofSecondaryBonus);
							++ProofGrantIndex;
						}

						UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEVFXBindingProof] InventoryReady GrantedCount=%d InventoryCount=%d AoeDamageValue=%.3f AoeSpeedValue=%.3f AoeScaleValue=%.3f HeroDamageMult=%.3f HeroAttackSpeedMult=%.3f HeroScaleMult=%.3f"),
							ProofGrantIndex,
							RunState->GetInventorySlots().Num(),
							RunState->GetSecondaryStatValue(ET66SecondaryStatType::AoeDamage),
							RunState->GetSecondaryStatValue(ET66SecondaryStatType::AoeSpeed),
							RunState->GetSecondaryStatValue(ET66SecondaryStatType::AoeScale),
							RunState->GetHeroDamageMultiplier(),
							RunState->GetHeroAttackSpeedMultiplier(),
							RunState->GetHeroScaleMultiplier());
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEVFXBindingProof] Missing RunState; item proof grants skipped."));
					}
				}

				if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->DisableMovement();
					Movement->SetMovementMode(MOVE_None);
				}

				FVector Forward = HeroPawn->GetActorForwardVector().GetSafeNormal2D();
				if (Forward.IsNearlyZero())
				{
					Forward = FVector::ForwardVector;
				}
				FVector Right = HeroPawn->GetActorRightVector().GetSafeNormal2D();
				if (Right.IsNearlyZero())
				{
					Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal2D();
				}

				const FVector HeroLocation = HeroPawn->GetActorLocation();
				const FVector PrimaryLocation = HeroLocation + Forward * 360.0f;
				// Category-native idol proof selectors. The AOE weapon is always the upstream
				// driver; only the equipped proof idol changes the downstream category behaviour.
				const bool bIdolPierceProof = bWaterIdolImpactProofMode && ImpactProofIdolID == FName(TEXT("Idol_Electricity_Pierce"));
				const bool bIdolBounceProof = bWaterIdolImpactProofMode && ImpactProofIdolID == FName(TEXT("Idol_Electricity_Bounce"));
				const bool bIdolDotProof = bWaterIdolImpactProofMode && ImpactProofIdolID == FName(TEXT("Idol_Nature_DOT"));
				const bool bWeaponPatternProofMode = FParse::Param(FCommandLine::Get(), TEXT("T66Hero1WeaponPatternProof"));
				struct FHitboxProofTargetSpec
				{
					const TCHAR* Label = TEXT("");
					FVector Offset = FVector::ZeroVector;
					bool bExpectedHit = false;
				};
				TArray<FHitboxProofTargetSpec> TargetSpecs;
				if (bWeaponPatternProofMode)
				{
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
					};
				}
				else if (bPierceVFXBindingProofMode)
				{
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("InLineNear"), Forward * 120.0f, true },
						{ TEXT("InLineFar"), Forward * 520.0f, true },
						{ TEXT("InsideTubeSide"), Forward * 420.0f + Right * 60.0f, true },
						{ TEXT("OutsideTubeSide"), Forward * 420.0f + Right * 180.0f, false },
						{ TEXT("OutsideBehind"), -Forward * 520.0f, false },
						{ TEXT("OutsideFarSide"), Forward * 900.0f + Right * 260.0f, false },
					};
				}
				else if (bBounceVFXBindingProofMode)
				{
					// Bounce projectile-travel proof intentionally stages the user's requested
					// two-link sequence only: hero -> primary, then primary -> ChainSecond. The
					// remaining controls sit out of chain range so the capture cannot show a third
					// projectile and accidentally prove the older multi-target setup.
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("ChainSecond"), Right * 150.0f, true },
						{ TEXT("OutOfChainRangeForward"), Forward * 4200.0f + Right * 150.0f, false },
						{ TEXT("OutOfChainRangeSide"), Forward * 4200.0f + Right * 4200.0f, false },
						{ TEXT("OutsideBehind"), -Forward * 4200.0f, false },
					};
				}
				else if (bDotVFXBindingProofMode)
				{
					// DOT is single-target: one primary that takes the contact hit + the single
					// authoritative DOT payload (and the three placeholder applicator markers). The
					// out-of-range controls stay hidden and unhit so the capture cannot read the DOT
					// as an AOE/multi-lane effect.
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("OutOfRangeForward"), Forward * 4200.0f, false },
						{ TEXT("OutOfRangeSide"), Forward * 4200.0f + Right * 4200.0f, false },
					};
				}
				else if (bWaterIdolImpactProofMode && ImpactProofIdolID == FName(TEXT("Idol_Ice_AOE")))
				{
					FIdolData ProofIdolData;
					const bool bHaveProofIdolData = ProofGameInstance
						? ProofGameInstance->GetIdolData(ImpactProofIdolID, ProofIdolData)
						: false;
					FHeroData ProofHeroData;
					const bool bHaveProofHeroData = ProofGameInstance
						? ProofGameInstance->GetHeroData(HeroID, ProofHeroData)
						: false;
					const float AuthoredIdolRadius = bHaveProofIdolData
						? FMath::Max(1.0f, ProofIdolData.AoeRadius)
						: 300.0f;
					const float BaseAoeRadius = bHaveProofHeroData && ProofHeroData.AoeRadius > KINDA_SMALL_NUMBER
						? ProofHeroData.AoeRadius
						: 300.0f;
					const float WeaponScaleMultiplier = bHaveProofWeaponData
						? FMath::Max(0.1f, ProofWeaponData.AttackScaleMultiplier)
						: 1.0f;
					const float WeaponBonusAoeRadius = bHaveProofWeaponData
						? FMath::Max(0.0f, ProofWeaponData.BonusAoeRadius)
						: 0.0f;
					const float EffectiveWeaponAoeRadius = FMath::Max(1.0f, BaseAoeRadius * WeaponScaleMultiplier + WeaponBonusAoeRadius);
					const float EffectiveWeaponInnerRadius = bHaveProofWeaponData
						? EffectiveWeaponAoeRadius * FMath::Clamp(ProofWeaponData.AoeInnerRadiusRatio, 0.0f, 0.95f)
						: 0.0f;
					const float IdolImpactForwardOffset =
						EffectiveWeaponInnerRadius > KINDA_SMALL_NUMBER && EffectiveWeaponAoeRadius > EffectiveWeaponInnerRadius
							? (EffectiveWeaponInnerRadius + EffectiveWeaponAoeRadius) * 0.5f
							: 0.0f;
					const float AuthoredRadiusInsideForwardOffset = FMath::Max(40.0f, IdolImpactForwardOffset - AuthoredIdolRadius + 80.0f);
					const float AuthoredRadiusInsideSideOffset = FMath::Min(
						FMath::Max(40.0f, EffectiveWeaponInnerRadius * 0.75f),
						FMath::Max(40.0f, AuthoredIdolRadius * 0.6f));
					const float AuthoredRadiusOutsideOffset = FMath::Max(AuthoredIdolRadius + 80.0f, EffectiveWeaponAoeRadius + 80.0f);
					UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEIdolImpactProof] AuthoredIdolRadius SourceID=%s AoeRadius=%.2f DataResolved=%d WeaponImpactForwardOffset=%.2f BaseAoeRadius=%.2f WeaponAoeRadius=%.2f WeaponInnerRadius=%.2f InsideForwardOffset=%.2f InsideSideOffset=%.2f OutsideSideOffset=%.2f"),
						*ImpactProofIdolID.ToString(),
						AuthoredIdolRadius,
						bHaveProofIdolData ? 1 : 0,
						IdolImpactForwardOffset,
						BaseAoeRadius,
						EffectiveWeaponAoeRadius,
						EffectiveWeaponInnerRadius,
						AuthoredRadiusInsideForwardOffset,
						AuthoredRadiusInsideSideOffset,
						AuthoredRadiusOutsideOffset);
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("WaterOnlyInnerHollow"), Forward * 120.0f, true },
						{ TEXT("WeaponOnlyOuterBand"), Forward * 320.0f, true },
						{ TEXT("InsideBandSide"), Forward * 270.0f + Right * 170.0f, true },
						{ TEXT("WaterAuthoredRadiusInside"), Forward * AuthoredRadiusInsideForwardOffset + Right * AuthoredRadiusInsideSideOffset, true },
						{ TEXT("WaterAuthoredRadiusOutside"), Forward * IdolImpactForwardOffset + Right * AuthoredRadiusOutsideOffset, false },
						{ TEXT("OutsideAngleEdge"), -Forward * 30.0f + Right * 360.0f, false },
						{ TEXT("OutsideBehind"), -Forward * 380.0f, false },
						{ TEXT("OutsideAllRadius"), Forward * 760.0f, false },
					};
				}
				else if (bWaterIdolImpactProofMode && ImpactProofIdolID == FName(TEXT("Idol_Nature_AOE")))
				{
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("InsideBandForward"), Forward * 320.0f, true },
						{ TEXT("InsideBandSide"), Forward * 270.0f + Right * 170.0f, true },
						{ TEXT("InsideAngleEdge"), Forward * 30.0f + Right * 360.0f, true },
						{ TEXT("InnerHollow"), Forward * 120.0f, false },
						{ TEXT("OutsideAngleEdge"), -Forward * 30.0f + Right * 360.0f, false },
						{ TEXT("OutsideBehind"), -Forward * 760.0f, false },
						{ TEXT("OutsideRadius"), Forward * 520.0f, false },
					};
				}
				else if (bIdolPierceProof)
				{
					// Pierce idol (Idol_Electricity_Pierce, property=1 => 2 line targets) read off the AOE
					// impact point along forward: primary + one in-line second target are pierced.
					// PierceInLineSecond sits beyond the parent AOE outer radius so only the pierce
					// line can reach it (isolates pierce reach from the parent weapon AOE). All
					// ExpectedHit=0 controls sit far outside both the parent AOE radius and the pierce
					// tube/length so neither the parent AOE nor the pierce capsule can touch them.
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("PierceInLineSecond"), Forward * 600.0f, true },
						{ TEXT("PierceOffLineFar"), Forward * 220.0f + Right * 4200.0f, false },
						{ TEXT("PierceBeyondReach"), Forward * 4200.0f, false },
						{ TEXT("OutsideBehind"), -Forward * 4200.0f, false },
					};
				}
				else if (bIdolBounceProof)
				{
					// Bounce idol (Idol_Electricity_Bounce, property=1 => 2 chain links) read off the AOE
					// impact point: primary + one nearby chain link; the remaining controls sit far
					// out of chain range so the capture cannot read a third link.
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("ChainSecond"), Right * 160.0f, true },
						{ TEXT("OutOfChainRangeForward"), Forward * 4200.0f, false },
						{ TEXT("OutOfChainRangeSide"), Forward * 4200.0f + Right * 4200.0f, false },
						{ TEXT("OutsideBehind"), -Forward * 4200.0f, false },
					};
				}
				else if (bIdolDotProof)
				{
					// DOT idol (Idol_Nature_DOT) is single-target: the idol owns ticking damage on the
					// AOE impact primary. Out-of-range controls stay hidden and unhit so the capture
					// cannot read the DOT as a multi-target lane.
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("OutOfRangeForward"), Forward * 4200.0f, false },
						{ TEXT("OutOfRangeSide"), Forward * 4200.0f + Right * 4200.0f, false },
					};
				}
				else
				{
					TargetSpecs = {
						{ TEXT("Primary"), FVector::ZeroVector, true },
						{ TEXT("InsideBandForward"), Forward * 320.0f, true },
						{ TEXT("InsideBandSide"), Forward * 270.0f + Right * 170.0f, true },
						{ TEXT("InsideAngleEdge"), Forward * 30.0f + Right * 360.0f, true },
						{ TEXT("InnerHollow"), Forward * 120.0f, false },
						{ TEXT("OutsideAngleEdge"), -Forward * 30.0f + Right * 360.0f, false },
						{ TEXT("OutsideBehind"), -Forward * 300.0f, false },
						{ TEXT("OutsideRadius"), Forward * 520.0f, false },
					};
				}

				FString TargetMobIDString(TEXT("Slime"));
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetMob="), TargetMobIDString);
				const FName TargetMobID(*TargetMobIDString);

				TArray<TWeakObjectPtr<AT66EnemyBase>> ProofTargets;
				TArray<FString> ProofLabels;
				TArray<bool> ProofExpectedHits;
				AT66EnemyBase* PrimaryTarget = nullptr;
				for (const FHitboxProofTargetSpec& TargetSpec : TargetSpecs)
				{
					FActorSpawnParameters TargetSpawnParameters;
					TargetSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					const FVector TargetLocation = PrimaryLocation + TargetSpec.Offset;
					AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
						AT66EnemyBase::StaticClass(),
						TargetLocation,
						(HeroLocation - TargetLocation).Rotation(),
						TargetSpawnParameters);
					if (!Enemy)
					{
						UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEHitboxProof] Failed to spawn target label=%s"), TargetSpec.Label);
						continue;
					}

					Enemy->Tags.AddUnique(HitboxProofTag);
					Enemy->Tags.AddUnique(Hero1AxeTargetTag);
					Enemy->SetActorEnableCollision(true);
					Enemy->ConfigureAsMob(TargetMobID);
					if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
					{
						Registry->RegisterEnemy(Enemy);
					}
					T66RecordNonDirectorRouteAttribution(World, Enemy);
					Enemy->MaxHP = 20000;
					Enemy->CurrentHP = 20000;
					Enemy->TouchDamageHearts = 0;
					Enemy->PointValue = 0;
					Enemy->XPValue = 0;
					Enemy->bDropsLoot = false;
					Enemy->OwningDirector = nullptr;
					if ((bBounceVFXBindingProofMode || bDotVFXBindingProofMode || bIdolBounceProof || bIdolDotProof) && !TargetSpec.bExpectedHit)
					{
						Enemy->SetActorHiddenInGame(true);
					}
					if (UCharacterMovementComponent* EnemyMovement = Enemy->GetCharacterMovement())
					{
						EnemyMovement->StopMovementImmediately();
						EnemyMovement->GravityScale = 0.0f;
						EnemyMovement->DisableMovement();
						EnemyMovement->SetMovementMode(MOVE_None);
					}
					Enemy->ForceMobVertexAnimationClipForAutomation(FName(TEXT("HitReact")), 30.0f);
					Enemy->SetActorTickEnabled(false);

					if (FCString::Strcmp(TargetSpec.Label, TEXT("Primary")) == 0)
					{
						PrimaryTarget = Enemy;
					}

					ProofTargets.Add(Enemy);
					ProofLabels.Add(FString(TargetSpec.Label));
					ProofExpectedHits.Add(TargetSpec.bExpectedHit);
					UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] Spawned Target=%s ExpectedHit=%d Location=%s ProofIdol=%s"),
						TargetSpec.Label,
						TargetSpec.bExpectedHit ? 1 : 0,
						*Enemy->GetActorLocation().ToCompactString(),
						*ImpactProofIdolID.ToString());
				}

				if (!PrimaryTarget)
				{
					UE_LOG(LogTemp, Error, TEXT("[Hero1AxeAOEHitboxProof] Failed: no primary target."));
					return;
				}

				HeroPawn->CombatComponent->SetAutoAttackSuppressed(true);
				HeroPawn->CombatComponent->SetLockedTarget(PrimaryTarget);

				if (bBounceVFXBindingProofMode)
				{
					static const TCHAR* BounceWarmupSystemPath = TEXT("/Game/VFX/Hero1/Axe/Bounce/NS_Hero1AxeBounce_MeshSlash.NS_Hero1AxeBounce_MeshSlash");
					UNiagaraSystem* BounceWarmupSystem = LoadObject<UNiagaraSystem>(nullptr, BounceWarmupSystemPath);
					if (BounceWarmupSystem)
					{
						const FVector WarmupLocation = HeroPawn->GetActorLocation() + FVector(0.0f, 0.0f, -50000.0f);
						UNiagaraComponent* WarmupComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
							World,
							BounceWarmupSystem,
							WarmupLocation,
							FRotator::ZeroRotator,
							FVector(1.0f),
							true,
							true,
							ENCPoolMethod::None,
							true);
						if (WarmupComponent)
						{
							WarmupComponent->SetAutoDestroy(true);
							WarmupComponent->SetTranslucentSortPriority(14);
							FTimerHandle BounceWarmupDestroyHandle;
							World->GetTimerManager().SetTimer(
								BounceWarmupDestroyHandle,
								FTimerDelegate::CreateWeakLambda(this, [WarmupComponent]()
								{
									if (IsValid(WarmupComponent))
									{
										WarmupComponent->DeactivateImmediate();
										WarmupComponent->DestroyComponent();
									}
								}),
								1.0f,
								false);
							UE_LOG(LogTemp, Display, TEXT("[Hero1AxeBounceProof] WarmedCarrier System=%s Location=%s"), BounceWarmupSystemPath, *WarmupLocation.ToCompactString());
						}
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeBounceProof] WarmedCarrierSkipped MissingSystem=%s"), BounceWarmupSystemPath);
					}
				}

				float FireDelaySeconds = 5.4f;
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEHitboxFireDelay="), FireDelaySeconds);
				FireDelaySeconds = FMath::Clamp(FireDelaySeconds, 0.2f, 30.0f);
				float VFXLeadSeconds = 0.12f;
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOEHitboxVFXLeadSeconds="), VFXLeadSeconds);
				VFXLeadSeconds = FMath::Clamp(VFXLeadSeconds, 0.0f, FMath::Max(0.0f, FireDelaySeconds - 0.05f));
				const float VFXSpawnDelaySeconds = FMath::Max(0.01f, FireDelaySeconds - VFXLeadSeconds);
				TWeakObjectPtr<AT66HeroBase> WeakHero(HeroPawn);
				TWeakObjectPtr<AT66EnemyBase> WeakPrimaryTarget(PrimaryTarget);
				const float ScheduledFireDelaySeconds = FireDelaySeconds;
				const float ScheduledVFXLeadSeconds = VFXLeadSeconds;
				if (!bProductionVFXBindingProofMode && !bWaterIdolImpactProofMode && !bDotVFXBindingProofMode)
				{
					FTimerHandle HitboxProofVFXHandle;
					World->GetTimerManager().SetTimer(
						HitboxProofVFXHandle,
						FTimerDelegate::CreateWeakLambda(this, [this, WeakHero, WeakPrimaryTarget, ScheduledFireDelaySeconds, ScheduledVFXLeadSeconds]()
						{
							AT66HeroBase* CapturedHero = WeakHero.Get();
							AT66EnemyBase* CapturedPrimary = WeakPrimaryTarget.Get();
							UWorld* CapturedWorld = GetWorld();
							if (!CapturedHero || !CapturedPrimary || !CapturedWorld)
							{
								UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEHitboxProof] VFX spawn skipped: missing hero, primary target, or world."));
								return;
							}

							FVector VFXForward = (CapturedPrimary->GetActorLocation() - CapturedHero->GetActorLocation()).GetSafeNormal2D();
							if (VFXForward.IsNearlyZero())
							{
								VFXForward = CapturedHero->GetActorForwardVector().GetSafeNormal2D();
							}
							if (VFXForward.IsNearlyZero())
							{
								VFXForward = FVector::ForwardVector;
							}

							FActorSpawnParameters VFXSpawnParameters;
							VFXSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							VFXSpawnParameters.Owner = CapturedHero;
							const FVector VFXLocation = CapturedPrimary->GetActorLocation() - VFXForward * 28.0f + FVector(0.0f, 0.0f, -82.0f);
							const FRotator VFXRotation = VFXForward.Rotation();
							AT66Hero1AxeAOEVFXLabActor* LabVFXActor = CapturedWorld->SpawnActor<AT66Hero1AxeAOEVFXLabActor>(
								AT66Hero1AxeAOEVFXLabActor::StaticClass(),
								VFXLocation,
								VFXRotation,
								VFXSpawnParameters);
							UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] VFXSpawned=%d WorldTime=%.3f FireDelay=%.2f VFXLead=%.2f Location=%s Rotation=%s"),
								LabVFXActor ? 1 : 0,
								CapturedWorld->GetTimeSeconds(),
								ScheduledFireDelaySeconds,
								ScheduledVFXLeadSeconds,
								*VFXLocation.ToCompactString(),
								*VFXRotation.ToCompactString());
						}),
						VFXSpawnDelaySeconds,
						false);
				}
				else
				{
					UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEVFXBindingProof] ManualLabVFX=0 Reason=ProductionBindingDispatcherWillSpawnOnFire IdolImpactProof=%d"),
						bWaterIdolImpactProofMode ? 1 : 0);
				}
				FTimerHandle HitboxProofFireHandle;
				World->GetTimerManager().SetTimer(
					HitboxProofFireHandle,
					FTimerDelegate::CreateWeakLambda(this, [this, WeakHero, WeakPrimaryTarget, ProofTargets, ProofLabels, ProofExpectedHits, ScheduledFireDelaySeconds, ScheduledVFXLeadSeconds, ImpactProofIdolID, bWaterIdolImpactProofMode, bBounceVFXBindingProofMode, bDotVFXBindingProofMode, bIdolBounceProof, bIdolDotProof, WeakProofIdolManager, bProofIdolStateCaptured, PreProofEquippedIdols, PreProofEquippedIdolTiers, PreProofDifficulty]()
					{
						AT66HeroBase* CapturedHero = WeakHero.Get();
						AT66EnemyBase* CapturedPrimary = WeakPrimaryTarget.Get();
						if (!CapturedHero || !CapturedPrimary || !CapturedHero->CombatComponent)
						{
							UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEHitboxProof] Fire skipped: missing hero, primary target, or combat component."));
							return;
						}
						if (UWorld* FireWorld = GetWorld())
						{
							UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] Fire WorldTime=%.3f FireDelay=%.2f VFXLead=%.2f Primary=%s ProofIdol=%s IdolImpactProof=%d"),
								FireWorld->GetTimeSeconds(),
								ScheduledFireDelaySeconds,
								ScheduledVFXLeadSeconds,
								*GetNameSafe(CapturedPrimary),
								*ImpactProofIdolID.ToString(),
								bWaterIdolImpactProofMode ? 1 : 0);
						}

						TArray<int32> HPBefore;
						HPBefore.Reserve(ProofTargets.Num());
						for (const TWeakObjectPtr<AT66EnemyBase>& WeakTarget : ProofTargets)
						{
							HPBefore.Add(WeakTarget.IsValid() ? WeakTarget.Get()->CurrentHP : INDEX_NONE);
						}

						// Bounce two-link proof isolation: the normal Bounce chain search
						// (FindClosestTargetHandleInRange) walks the actor registry's enemies,
						// lightweight mobs, and bosses. The preamble destroys AT66EnemyBase world
						// actors but does not clear lightweight AT66MobBase mobs or bosses, so a
						// surviving non-proof/world enemy can become an unintended third bounce
						// link (LinkCount=3). Immediately before firing, remove every damageable
						// target that is not a staged proof target (tagged HitboxProofTag) so the
						// chain can only find Primary -> ChainSecond. The negative controls keep the
						// proof tag and stay registered far out of range for HP-delta checking.
						if (bBounceVFXBindingProofMode || bIdolBounceProof)
						{
							if (UWorld* IsolateWorld = GetWorld())
							{
								int32 ReleasedMobCount = 0;
								int32 DestroyedEnemyCount = 0;
								int32 RemovedBossCount = 0;
								int32 RemovedHazardCount = 0;
								UT66MobManagerSubsystem* MobManager = IsolateWorld->GetSubsystem<UT66MobManagerSubsystem>();
								if (UT66ActorRegistrySubsystem* Registry = IsolateWorld->GetSubsystem<UT66ActorRegistrySubsystem>())
								{
									TArray<AT66MobBase*> NonProofMobs;
									for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Registry->GetActiveMobs())
									{
										if (AT66MobBase* Mob = WeakMob.Get())
										{
											if (!Mob->Tags.Contains(HitboxProofTag))
											{
												NonProofMobs.Add(Mob);
											}
										}
									}
									for (AT66MobBase* Mob : NonProofMobs)
									{
										if (MobManager)
										{
											MobManager->UnregisterMob(Mob);
										}
										Registry->UnregisterMob(Mob);
										Mob->Destroy();
										++ReleasedMobCount;
									}

									// Some pooled or director-created mobs can miss the registry snapshot
									// used above. Destroy any remaining non-proof mob actors for this proof
									// instead of releasing them to the pool; pooled actors have shown up as
									// brown/orange capture clutter and can be mistaken for Bounce projectiles.
									for (TActorIterator<AT66MobBase> It(IsolateWorld); It; ++It)
									{
										AT66MobBase* Mob = *It;
										if (Mob && !Mob->Tags.Contains(HitboxProofTag))
										{
											if (MobManager)
											{
												MobManager->UnregisterMob(Mob);
											}
											Registry->UnregisterMob(Mob);
											Mob->Destroy();
											++ReleasedMobCount;
										}
									}

									TArray<AT66EnemyBase*> NonProofEnemies;
									for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
									{
										if (AT66EnemyBase* Enemy = WeakEnemy.Get())
										{
											if (!Enemy->Tags.Contains(HitboxProofTag))
											{
												NonProofEnemies.Add(Enemy);
											}
										}
									}
									for (AT66EnemyBase* Enemy : NonProofEnemies)
									{
										Registry->UnregisterEnemy(Enemy);
										Enemy->Destroy();
										++DestroyedEnemyCount;
									}

									TArray<AT66BossBase*> NonProofBosses;
									for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
									{
										if (AT66BossBase* Boss = WeakBoss.Get())
										{
											NonProofBosses.Add(Boss);
										}
									}
									for (AT66BossBase* Boss : NonProofBosses)
									{
										Registry->UnregisterBoss(Boss);
										Boss->Destroy();
										++RemovedBossCount;
									}
								}
								for (TActorIterator<AT66BossGroundAOE> It(IsolateWorld); It; ++It)
								{
									if (AT66BossGroundAOE* GroundAOE = *It)
									{
										GroundAOE->Destroy();
										++RemovedHazardCount;
									}
								}
								for (TActorIterator<AT66EnemyProjectileBase> It(IsolateWorld); It; ++It)
								{
									if (AT66EnemyProjectileBase* EnemyProjectile = *It)
									{
										EnemyProjectile->Destroy();
										++RemovedHazardCount;
									}
								}
								for (TActorIterator<AT66TrapArrowProjectile> It(IsolateWorld); It; ++It)
								{
									if (AT66TrapArrowProjectile* TrapProjectile = *It)
									{
										TrapProjectile->Destroy();
										++RemovedHazardCount;
									}
								}
								UE_LOG(LogTemp, Display, TEXT("[Hero1AxeBounceProof] IsolatedTargetPopulation ReleasedMobs=%d DestroyedEnemies=%d RemovedBosses=%d RemovedHazards=%d ProofTargets=%d"),
									ReleasedMobCount,
									DestroyedEnemyCount,
									RemovedBossCount,
									RemovedHazardCount,
									ProofTargets.Num());
							}

							// Capture-only readability: keep each Bounce link on screen for the same
							// 0.60s presentation window used by the production readable-travel floor.
							// The authored Bounce Niagara carrier is visible from spawn and its layer
							// lifetimes now cover that full travel, so the proof samples a real moving
							// slash instead of only a target-impact flash.
							static const float BounceProofLinkTravelSeconds = 0.60f;
							if (IConsoleVariable* ProofTravelCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.Bounce.ProofReadableTravelSeconds")))
							{
								ProofTravelCVar->Set(BounceProofLinkTravelSeconds, ECVF_SetByCode);
								UE_LOG(LogTemp, Display, TEXT("[Hero1AxeBounceProof] SetBounceProofLinkTravelSeconds=%.2f"), BounceProofLinkTravelSeconds);
							}

							// Deterministic capture: at normal speed the screenshot sequence advances
							// ~0.2s of game time per rendered frame (the engine uses the real frame
							// time, which is dominated by per-frame PNG writes), so the short Bounce
							// travel window lands in too few frames. Switch the engine to a fixed
							// time step for the bounce so the game clock advances a constant small DT
							// per rendered frame regardless of PNG-write stalls. The visual mover
							// (game-time lerp) and the authored carrier (Niagara, game-time aged)
							// then advance in lockstep, so the readable slash is sampled across many
							// frames as it travels hero->primary (link 0) and primary->second
							// (link 1). Global time dilation is intentionally NOT used: it slowed the
							// Niagara reveal but not the mover, desyncing the slash from its travel.
							static const double BounceProofFixedDeltaSeconds = 0.04; // 25 game-fps clock
							static const float BounceProofFixedStepGameSeconds = 1.5f;
							FApp::SetFixedDeltaTime(BounceProofFixedDeltaSeconds);
							FApp::SetUseFixedTimeStep(true);
							UE_LOG(LogTemp, Display, TEXT("[Hero1AxeBounceProof] SetFixedTimeStep DT=%.3f for %.2fs game-time"), BounceProofFixedDeltaSeconds, BounceProofFixedStepGameSeconds);
							if (UWorld* FixedStepWorld = GetWorld())
							{
								FTimerHandle BounceProofFixedStepResetHandle;
								FixedStepWorld->GetTimerManager().SetTimer(
									BounceProofFixedStepResetHandle,
									FTimerDelegate::CreateWeakLambda(this, []()
									{
										FApp::SetUseFixedTimeStep(false);
										UE_LOG(LogTemp, Display, TEXT("[Hero1AxeBounceProof] ResetFixedTimeStep"));
									}),
									BounceProofFixedStepGameSeconds,
									false);
							}
						}

						if (bDotVFXBindingProofMode || bIdolDotProof)
						{
							// Capture-only readability: stretch the single DOT shot's hero->target
							// travel so the frame-rate-limited capture samples the moving projectile,
							// the marker reveal at impact, and the first DOT ticks. Runtime DOT
							// damage/targeting are resolved independently and unaffected.
							static const float DotProofTravelSeconds = 0.60f;
							if (IConsoleVariable* DotProofTravelCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("T66.DOT.ProofReadableTravelSeconds")))
							{
								DotProofTravelCVar->Set(DotProofTravelSeconds, ECVF_SetByCode);
								UE_LOG(LogTemp, Display, TEXT("[Hero1AxeDOTProof] SetDotProofTravelSeconds=%.2f"), DotProofTravelSeconds);
							}
							// Fixed timestep so the game clock advances a constant small DT per rendered
							// frame regardless of PNG-write stalls, matching the Bounce proof approach.
							static const double DotProofFixedDeltaSeconds = 0.04; // 25 game-fps clock
							static const float DotProofFixedStepGameSeconds = 4.0f; // cover DOT duration + ticks
							FApp::SetFixedDeltaTime(DotProofFixedDeltaSeconds);
							FApp::SetUseFixedTimeStep(true);
							UE_LOG(LogTemp, Display, TEXT("[Hero1AxeDOTProof] SetFixedTimeStep DT=%.3f for %.2fs game-time"), DotProofFixedDeltaSeconds, DotProofFixedStepGameSeconds);
							if (UWorld* FixedStepWorld = GetWorld())
							{
								FTimerHandle DotProofFixedStepResetHandle;
								FixedStepWorld->GetTimerManager().SetTimer(
									DotProofFixedStepResetHandle,
									FTimerDelegate::CreateWeakLambda(this, []()
									{
										FApp::SetUseFixedTimeStep(false);
										UE_LOG(LogTemp, Display, TEXT("[Hero1AxeDOTProof] ResetFixedTimeStep"));
									}),
									DotProofFixedStepGameSeconds,
									false);
							}
						}

						CapturedHero->CombatComponent->SetLockedTarget(CapturedPrimary);
						CapturedHero->CombatComponent->SetAutoAttackSuppressed(false);
						CapturedHero->CombatComponent->PerformAutomationAutoAttackNow();
						CapturedHero->CombatComponent->SetAutoAttackSuppressed(true);

						FTimerHandle HitboxProofLogHandle;
						// DOT idol damage ticks over its duration, so the HP-delta + DamageBySource
						// snapshot must wait past the final tick (DotDuration=3.0s) under the fixed
						// timestep; immediate-damage categories sample shortly after the contact hit.
						const float ProofLogDelaySeconds = bIdolDotProof ? 3.6f : 0.2f;
						if (UWorld* CapturedWorld = GetWorld())
						{
							CapturedWorld->GetTimerManager().SetTimer(
								HitboxProofLogHandle,
								FTimerDelegate::CreateWeakLambda(this, [this, ProofTargets, ProofLabels, ProofExpectedHits, HPBefore, ImpactProofIdolID, bWaterIdolImpactProofMode, WeakProofIdolManager, bProofIdolStateCaptured, PreProofEquippedIdols, PreProofEquippedIdolTiers, PreProofDifficulty]()
								{
									UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance()
										? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>()
										: nullptr;
									for (int32 Index = 0; Index < ProofTargets.Num(); ++Index)
									{
										AT66EnemyBase* Target = ProofTargets[Index].Get();
										const int32 Before = HPBefore.IsValidIndex(Index) ? HPBefore[Index] : INDEX_NONE;
										const int32 After = Target ? Target->CurrentHP : INDEX_NONE;
										const bool bActualHit = Before != INDEX_NONE && After != INDEX_NONE && After < Before;
										const bool bExpectedHit = ProofExpectedHits.IsValidIndex(Index) ? ProofExpectedHits[Index] : false;
										const FString Label = ProofLabels.IsValidIndex(Index) ? ProofLabels[Index] : FString::Printf(TEXT("Target%d"), Index);
										if (FloatingText && Target && bActualHit)
										{
											const int32 DamageDelta = Before - After;
											FloatingText->ShowDamageNumber(Target, DamageDelta, UT66FloatingCombatTextSubsystem::EventType_Crit);
											if (UWorld* DebugWorld = Target->GetWorld())
											{
												DrawDebugString(
													DebugWorld,
													Target->GetActorLocation() + FVector(0.0f, 0.0f, 185.0f),
													FString::Printf(TEXT("%d"), DamageDelta),
													nullptr,
													FColor::Yellow,
													2.0f,
													true,
													3.4f);
											}
											if (GEngine)
											{
												GEngine->AddOnScreenDebugMessage(
													-1,
													2.0f,
													FColor::Orange,
													FString::Printf(TEXT("%s -%d"), *Label, DamageDelta));
											}
											UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] DamageNumber Target=%s Amount=%d"),
												*Label,
												DamageDelta);
										}
										UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] Target=%s ExpectedHit=%d ActualHit=%d HPBefore=%d HPAfter=%d Result=%s"),
											*Label,
											bExpectedHit ? 1 : 0,
											bActualHit ? 1 : 0,
											Before,
											After,
											(bActualHit == bExpectedHit) ? TEXT("PASS") : TEXT("FAIL"));
									}

									if (bWaterIdolImpactProofMode)
									{
										if (UT66DamageLogSubsystem* DamageLog = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
										{
											const TArray<FDamageLogEntry> DamageRows = DamageLog->GetDamageBySourceSorted();
											for (const FDamageLogEntry& Entry : DamageRows)
											{
												UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEIdolImpactProof] DamageBySource SourceID=%s TotalDamage=%d ProofIdol=%s"),
													*Entry.SourceID.ToString(),
													Entry.TotalDamage,
													*ImpactProofIdolID.ToString());
											}
										}
									}

									if (bProofIdolStateCaptured)
									{
										if (UT66IdolManagerSubsystem* IdolManager = WeakProofIdolManager.Get())
										{
											IdolManager->RestoreState(PreProofEquippedIdols, PreProofEquippedIdolTiers, PreProofDifficulty);
											UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEIdolImpactProof] RestoredIdolState PreviousCount=%d Difficulty=%s"),
												PreProofEquippedIdols.Num(),
												*UEnum::GetValueAsString(PreProofDifficulty));
										}
										else
										{
											UE_LOG(LogTemp, Warning, TEXT("[Hero1AxeAOEIdolImpactProof] RestoreSkipped Reason=MissingIdolManager"));
										}
									}
								}),
								ProofLogDelaySeconds,
								false);
						}
					}),
					FireDelaySeconds,
					false);

				UE_LOG(LogTemp, Display, TEXT("[Hero1AxeAOEHitboxProof] Armed FireDelay=%.2f VFXLead=%.2f VFXSpawnDelay=%.2f Targets=%d Primary=%s ProductionBindingProof=%d IdolImpactProof=%d ProofIdol=%s"),
					FireDelaySeconds,
					VFXLeadSeconds,
					VFXSpawnDelaySeconds,
					ProofTargets.Num(),
					*GetNameSafe(PrimaryTarget),
					bProductionVFXBindingProofMode ? 1 : 0,
					bWaterIdolImpactProofMode ? 1 : 0,
					*ImpactProofIdolID.ToString());
				return;
			}

			const FVector SpawnOrigin = ControlledPawn ? ControlledPawn->GetActorLocation() : FVector::ZeroVector;
			FVector LabSpawnOffset(0.0f, -460.0f, -82.0f);
			FString LabSpawnOffsetString;
			if (FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabOffset="), LabSpawnOffsetString))
			{
				FVector ParsedOffset;
				if (ParsedOffset.InitFromString(LabSpawnOffsetString))
				{
					LabSpawnOffset = ParsedOffset;
				}
			}
			FVector LabSpawnLocation = SpawnOrigin + LabSpawnOffset;
			float LabForwardOffset = 0.0f;
			float LabRightOffset = 0.0f;
			float LabVerticalOffset = 0.0f;
			const bool bHasLocalOffsetComponents =
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabForwardOffset="), LabForwardOffset) |
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabRightOffset="), LabRightOffset) |
				FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabVerticalOffset="), LabVerticalOffset);
			if (ControlledPawn && bHasLocalOffsetComponents)
			{
				LabSpawnLocation = SpawnOrigin
					+ ControlledPawn->GetActorForwardVector() * LabForwardOffset
					+ ControlledPawn->GetActorRightVector() * LabRightOffset
					+ FVector::UpVector * LabVerticalOffset;
			}
			else if (ControlledPawn && FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOELabOffsetLocal")))
			{
				LabSpawnLocation = SpawnOrigin
					+ ControlledPawn->GetActorForwardVector() * LabSpawnOffset.X
					+ ControlledPawn->GetActorRightVector() * LabSpawnOffset.Y
					+ FVector::UpVector * LabSpawnOffset.Z;
			}
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Name = TEXT("T66Hero1AxeAOEVFXLab_AOE");
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AT66Hero1AxeAOEVFXLabActor* LabActor = World->SpawnActor<AT66Hero1AxeAOEVFXLabActor>(
				AT66Hero1AxeAOEVFXLabActor::StaticClass(),
				LabSpawnLocation,
				FRotator::ZeroRotator,
				SpawnParameters))
			{
				UE_LOG(LogTemp, Display, TEXT("Gameplay automation: spawned Hero1AxeAOE lab actor at %s"), *LabActor->GetActorLocation().ToCompactString());

				if (FParse::Param(FCommandLine::Get(), TEXT("T66Hero1AxeAOESpawnTargets")))
				{
					for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
					{
						if (It->Tags.Contains(Hero1AxeTargetTag))
						{
							It->Destroy();
						}
					}

					int32 TargetCount = 3;
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetCount="), TargetCount);
					TargetCount = FMath::Clamp(TargetCount, 1, 6);

					float TargetSpacing = 145.0f;
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetSpacing="), TargetSpacing);
					TargetSpacing = FMath::Clamp(TargetSpacing, 80.0f, 260.0f);

					float TargetForwardOffset = 210.0f;
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetForwardOffset="), TargetForwardOffset);
					TargetForwardOffset = FMath::Clamp(TargetForwardOffset, 80.0f, 420.0f);

					FString TargetMobIDString(TEXT("Slime"));
					FParse::Value(FCommandLine::Get(), TEXT("T66Hero1AxeAOETargetMob="), TargetMobIDString);
					const FName TargetMobID(*TargetMobIDString);

					FActorSpawnParameters TargetSpawnParameters;
					TargetSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					const float CenterIndex = (static_cast<float>(TargetCount) - 1.0f) * 0.5f;
					for (int32 TargetIndex = 0; TargetIndex < TargetCount; ++TargetIndex)
					{
						const float LateralOffset = (static_cast<float>(TargetIndex) - CenterIndex) * TargetSpacing;
						const FVector TargetLocation = LabActor->GetActorLocation() + FVector(TargetForwardOffset, LateralOffset, 82.0f);
						FRotator TargetRotation = (SpawnOrigin - TargetLocation).Rotation();
						TargetRotation.Pitch = 0.0f;
						TargetRotation.Roll = 0.0f;
						TargetSpawnParameters.Name = FName(*FString::Printf(TEXT("T66Hero1AxeAOETarget_%02d"), TargetIndex + 1));

						AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
							AT66EnemyBase::StaticClass(),
							TargetLocation,
							TargetRotation,
							TargetSpawnParameters);
						if (!Enemy)
						{
							UE_LOG(LogTemp, Error, TEXT("Gameplay automation: failed to spawn Hero1AxeAOE target %d"), TargetIndex + 1);
							continue;
						}

						Enemy->Tags.AddUnique(Hero1AxeTargetTag);
						Enemy->SetActorEnableCollision(false);
						Enemy->MaxHP = 20000;
						Enemy->CurrentHP = 20000;
						Enemy->TouchDamageHearts = 0;
						Enemy->PointValue = 0;
						Enemy->XPValue = 0;
						Enemy->bDropsLoot = false;
						Enemy->OwningDirector = nullptr;
						Enemy->ConfigureAsMob(TargetMobID);
						T66RecordNonDirectorRouteAttribution(World, Enemy);
						if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
						{
							Movement->StopMovementImmediately();
							Movement->GravityScale = 0.0f;
							Movement->DisableMovement();
							Movement->SetMovementMode(MOVE_None);
						}
						Enemy->ForceMobVertexAnimationClipForAutomation(FName(TEXT("HitReact")), 30.0f);
						Enemy->SetActorTickEnabled(false);
						UE_LOG(LogTemp, Display, TEXT("Gameplay automation: spawned Hero1AxeAOE target %s at %s"),
							*TargetMobID.ToString(),
							*TargetLocation.ToCompactString());
					}
				}
			}
		}
		return;
	}

	if (Mode == TEXT("temporaryprojectiles") || Mode == TEXT("projectilevisuals") || Mode == TEXT("projectileqa"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (UWorld* World = GetWorld())
		{
			const FRotator CameraRotation = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation() : GetControlRotation();
			const FVector CameraLocation = PlayerCameraManager
				? PlayerCameraManager->GetCameraLocation()
				: (GetPawn() ? GetPawn()->GetActorLocation() + FVector(-520.f, 0.f, 260.f) : FVector(-520.f, 0.f, 260.f));
			const FVector Forward = CameraRotation.Vector().GetSafeNormal();
			const FVector Right = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);
			const FVector Up = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Z);
			const FVector Origin = CameraLocation + Forward * 650.f + Up * 120.f;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			if (AT66HeroProjectile* HeroProjectile = World->SpawnActor<AT66HeroProjectile>(
				AT66HeroProjectile::StaticClass(),
				Origin - Right * 240.f,
				Forward.Rotation(),
				SpawnParams))
			{
				HeroProjectile->SetVisualOnly(true);
				HeroProjectile->Damage = 0;
				HeroProjectile->ConfigureTemporaryProjectileVisual(
					FT66TemporaryProjectileSystem::ProfileHeroPierce(),
					FT66TemporaryProjectileSystem::HeroProjectileColor(),
					1.0f,
					FT66TemporaryProjectileSystem::ProfileIdolOverlay(),
					FT66TemporaryProjectileSystem::HeroProjectileColor(),
					0.85f);
				HeroProjectile->SetProjectileSpeed(0.f);
				HeroProjectile->SetTargetLocation(HeroProjectile->GetActorLocation() + Forward * 900.f);
			}

			const FTransform EnemyProjectileTransform(Forward.Rotation(), Origin - Right * 80.f);
			if (AT66SpitProjectile* EnemyProjectile = World->SpawnActorDeferred<AT66SpitProjectile>(
				AT66SpitProjectile::StaticClass(),
				EnemyProjectileTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				EnemyProjectile->SetVisualOnly(true);
				EnemyProjectile->FinishSpawning(EnemyProjectileTransform);
			}

			if (AT66TrapArrowProjectile* TrapProjectile = World->SpawnActor<AT66TrapArrowProjectile>(
				AT66TrapArrowProjectile::StaticClass(),
				FTransform(Forward.Rotation(), Origin + Right * 80.f),
				SpawnParams))
			{
				TrapProjectile->InitializeProjectile(
					Forward,
					12,
					0.f,
					FT66TemporaryProjectileSystem::HostileProjectileColor(),
					FT66TemporaryProjectileSystem::HostileProjectileColor());
			}
		}
		return;
	}

	if (Mode == TEXT("runsummaryroundtrip"))
	{
		FString OutputPath;
		FParse::Value(FCommandLine::Get(), TEXT("T66RunSummaryRoundTripProof="), OutputPath);
		if (OutputPath.IsEmpty())
		{
			OutputPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("run_summary_roundtrip.json"));
		}

		TArray<FString> Checks;
		bool bAllPassed = true;
		const FString SlotName(TEXT("T66_RunSummaryRoundTripProof"));
		UGameplayStatics::DeleteGameInSlot(SlotName, 0);

		UT66LeaderboardRunSummarySaveGame* SourceSummary = NewObject<UT66LeaderboardRunSummarySaveGame>(this);
		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Source summary allocated"), SourceSummary != nullptr, SourceSummary ? TEXT("Fresh summary object created.") : TEXT("NewObject returned null."));
		if (SourceSummary)
		{
			SourceSummary->SchemaVersion = T66CurrentRunSummarySchemaVersion;
			SourceSummary->HeroID = FName(TEXT("Hero_1"));
			SourceSummary->CompanionID = FName(TEXT("Companion_1"));
			SourceSummary->Difficulty = ET66Difficulty::Easy;
			SourceSummary->PartySize = ET66PartySize::Solo;
			SourceSummary->StageReached = 4;
			SourceSummary->Score = 12345;
			SourceSummary->RunDurationSeconds = 321.f;
			SourceSummary->DamageStat = 11;
			SourceSummary->AttackSpeedStat = 12;
			SourceSummary->AttackScaleStat = 13;
			SourceSummary->AccuracyStat = 14;
			SourceSummary->ArmorStat = 15;
			SourceSummary->EvasionStat = 16;
			SourceSummary->LuckStat = 17;
			SourceSummary->SpeedStat = 18;
			SourceSummary->SecondaryStatValues.Add(ET66SecondaryStatType::FirePower, 4.25f);
			SourceSummary->SecondaryStatValues.Add(ET66SecondaryStatType::IcePower, 3.5f);
			SourceSummary->SecondaryStatValues.Add(ET66SecondaryStatType::ElectricityPower, 2.75f);
			SourceSummary->SecondaryStatValues.Add(ET66SecondaryStatType::NaturePower, 1.5f);
			SourceSummary->EquippedIdols = { FName(TEXT("Idol_Fire_AOE")), FName(TEXT("Idol_Ice_Pierce")), FName(TEXT("Idol_Electricity_Bounce")), FName(TEXT("Idol_Nature_DOT")) };
			SourceSummary->EquippedIdolTiers = { 1, 2, 3, 4 };
			SourceSummary->EquippedIdolElements = { ET66IdolElement::Fire, ET66IdolElement::Ice, ET66IdolElement::Electricity, ET66IdolElement::Nature };
			SourceSummary->EquippedIdolCategories = { ET66AttackCategory::AOE, ET66AttackCategory::Pierce, ET66AttackCategory::Bounce, ET66AttackCategory::DOT };
			SourceSummary->InventorySlots = {
				FT66InventorySlot(FName(TEXT("Item_FirePower_Black")), ET66ItemRarity::Black, 1, 0.f, 0, 2101),
				FT66InventorySlot(FName(TEXT("Item_IcePower_Red")), ET66ItemRarity::Red, 3, 0.f, 0, 2102),
				FT66InventorySlot(FName(TEXT("Item_ElectricityPower_Yellow")), ET66ItemRarity::Yellow, 9, 0.f, 0, 2103),
				FT66InventorySlot(FName(TEXT("Item_NaturePower_White")), ET66ItemRarity::White, 27, 0.f, 0, 2104)
			};
			for (const FT66InventorySlot& Slot : SourceSummary->InventorySlots)
			{
				SourceSummary->Inventory.Add(Slot.ItemTemplateID);
			}
			SourceSummary->NoIdolSelectionStacks = 2;
			SourceSummary->NoIdolPrimaryStatBonuses.DamageTenths = 20;
			SourceSummary->NoIdolPrimaryStatBonuses.AttackSpeedTenths = 20;
			SourceSummary->NoIdolPrimaryStatBonuses.AttackScaleTenths = 20;
			SourceSummary->MobLootDropsCollectedThisRun = 7;
			SourceSummary->MobLootQuantityCollectedThisRun = 42;
			SourceSummary->MobLootGoldValueCollectedThisRun = 42;
			SourceSummary->MobLootQuantityCollectedByPlayerThisRun = 25;
			SourceSummary->MobLootQuantityCollectedByPetThisRun = 17;
			SourceSummary->MobLootDropsCollectedByPetThisRun = 3;
			SourceSummary->MobLootQuantitySoldThisRun = 30;
			SourceSummary->MobLootSaleGoldThisRun = 30;
			SourceSummary->MobLootRemainingStack = 12;

			FT66AntiCheatGamblerGameSummary& GamblerSummary = SourceSummary->GamblerOutcomeSummaries.AddDefaulted_GetRef();
			GamblerSummary.GameType = ET66AntiCheatGamblerGameType::CoinFlip;
			GamblerSummary.Rounds = 4;
			GamblerSummary.Wins = 2;
			GamblerSummary.Losses = 2;
			GamblerSummary.TotalBetGold = 40;
			GamblerSummary.TotalPayoutGold = 55;
			SourceSummary->AntiCheatGamblerSummaries = SourceSummary->GamblerOutcomeSummaries;
			FT66AntiCheatGamblerEvent& GamblerEvent = SourceSummary->GamblerOutcomeEvents.AddDefaulted_GetRef();
			GamblerEvent.GameType = ET66AntiCheatGamblerGameType::CoinFlip;
			GamblerEvent.TimeSeconds = 12.5f;
			GamblerEvent.BetGold = 10;
			GamblerEvent.PayoutGold = 20;
			GamblerEvent.bWin = true;
			GamblerEvent.OutcomeExpectedChance01 = 0.2f;
			GamblerEvent.ActionSequence = TEXT("proof");
			SourceSummary->AntiCheatGamblerEvents = SourceSummary->GamblerOutcomeEvents;

			SourceSummary->CurrentGold = 100;
			SourceSummary->CurrentDebt = 5;
			SourceSummary->InventorySellValueTotal = 80;
			SourceSummary->NetWorth = 175;
			SourceSummary->ActiveVendorTokenStacks = 16;
			SourceSummary->CurrentSellFraction = 1.0f;
			SourceSummary->ShopStockCount = 4;
			SourceSummary->BuybackPoolSize = 2;
			SourceSummary->EquippedWeaponID = FName(TEXT("Weapon_Hero_1_Red_AOE"));
			SourceSummary->EquippedWeaponBranch = ET66AttackCategory::AOE;
			SourceSummary->EquippedWeaponRarity = ET66WeaponRarity::Red;
			SourceSummary->EquippedWeaponAttackPatternID = FName(TEXT("TwinAxe"));
			SourceSummary->EquippedWeaponProjectileCount = 2;
			SourceSummary->EquippedWeaponSpreadAngleDegrees = 22.5f;
			SourceSummary->ActivePetID = FName(TEXT("Pet_Dungeon_Slime"));
			SourceSummary->ActivePetSkinID = FName(TEXT("Skin_Green"));
			SourceSummary->ActivePetBondStagesCleared = 6;
			SourceSummary->ActivePetBondMovementSpeedMultiplier = 1.18f;
			SourceSummary->PetMobLootQuantityCollectedThisRun = 17;
			SourceSummary->PetMobLootDropsCollectedThisRun = 3;
			SourceSummary->bBossActiveAtSummary = true;
			SourceSummary->ActiveBossID = FName(TEXT("Dungeon_WebMatriarch"));
			SourceSummary->BossMaxHP = 900;
			SourceSummary->BossCurrentHP = 360;
			SourceSummary->OwedBossIDs = { FName(TEXT("Dungeon_SlimeKing")) };
			SourceSummary->CowardiceGatesTakenCount = 1;
			FT66BossPartSnapshot& BossPart = SourceSummary->BossParts.AddDefaulted_GetRef();
			BossPart.PartID = FName(TEXT("Head"));
			BossPart.HitZoneType = ET66HitZoneType::Head;
			BossPart.MaxHP = 300;
			BossPart.CurrentHP = 120;
		}

		const bool bSaved = SourceSummary && UGameplayStatics::SaveGameToSlot(SourceSummary, SlotName, 0);
		UT66LeaderboardRunSummarySaveGame* LoadedSummary = Cast<UT66LeaderboardRunSummarySaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Fresh summary save/load"), bSaved && LoadedSummary != nullptr, FString::Printf(TEXT("Saved=%d Loaded=%d Slot=%s."), bSaved ? 1 : 0, LoadedSummary ? 1 : 0, *SlotName));

		const TSharedPtr<FJsonObject> RunJson = LoadedSummary
			? T66BackendRunSerializer::BuildRunJsonObject(LoadedSummary->HeroID.ToString(), LoadedSummary->CompanionID.ToString(), LoadedSummary->Difficulty, LoadedSummary->PartySize, LoadedSummary->StageReached, LoadedSummary->Score, FMath::RoundToInt(LoadedSummary->RunDurationSeconds * 1000.f), LoadedSummary)
			: nullptr;
		UT66LeaderboardRunSummarySaveGame* ParsedSummary = RunJson.IsValid() ? T66BackendRunSummaryParser::Parse(RunJson, this) : nullptr;
		UT66LeaderboardRunSummarySaveGame* LegacyVendorTokenParsedSummary = nullptr;
		if (RunJson.IsValid())
		{
			const TSharedPtr<FJsonObject>* VendorObj = nullptr;
			if (RunJson->TryGetObjectField(TEXT("vendor"), VendorObj) && VendorObj && (*VendorObj).IsValid())
			{
				(*VendorObj)->RemoveField(TEXT("active_vendor_token_stacks"));
				(*VendorObj)->SetNumberField(TEXT("active_vendor_token_level"), 16);
				LegacyVendorTokenParsedSummary = T66BackendRunSummaryParser::Parse(RunJson, this);
			}
		}

		TArray<FName> LegacyIdolSaveIDs = { FName(TEXT("Idol_Light")), FName(TEXT("Idol_Water")), FName(TEXT("Idol_Storm")), FName(TEXT("Idol_Poison")) };
		TArray<uint8> LegacyIdolSaveTiers = { 1, 2, 3, 4 };
		const bool bLegacyIdolSaveMigrationChanged = T66NormalizeEquippedIdolSaveArrays(LegacyIdolSaveIDs, LegacyIdolSaveTiers);
		const bool bLegacyIdolSaveMigrated =
			LegacyIdolSaveIDs.Num() == 4
			&& LegacyIdolSaveIDs[0] == FName(TEXT("Idol_Electricity_Pierce"))
			&& LegacyIdolSaveIDs[1] == FName(TEXT("Idol_Ice_AOE"))
			&& LegacyIdolSaveIDs[2] == FName(TEXT("Idol_Electricity_AOE"))
			&& LegacyIdolSaveIDs[3] == FName(TEXT("Idol_Nature_DOT"))
			&& LegacyIdolSaveTiers.Num() == 4
			&& LegacyIdolSaveTiers[0] == 1
			&& LegacyIdolSaveTiers[1] == 2
			&& LegacyIdolSaveTiers[2] == 3
			&& LegacyIdolSaveTiers[3] == 4;

		UT66LeaderboardRunSummarySaveGame* LegacyIdolParsedSummary = nullptr;
		{
			TSharedPtr<FJsonObject> LegacyIdolJson = MakeShared<FJsonObject>();
			TArray<TSharedPtr<FJsonValue>> LegacyIdolValues;
			LegacyIdolValues.Add(MakeShared<FJsonValueString>(TEXT("Idol_Light")));
			LegacyIdolValues.Add(MakeShared<FJsonValueString>(TEXT("Idol_Water")));
			LegacyIdolValues.Add(MakeShared<FJsonValueString>(TEXT("Idol_Storm")));
			LegacyIdolValues.Add(MakeShared<FJsonValueString>(TEXT("Idol_Poison")));
			LegacyIdolJson->SetArrayField(TEXT("equipped_idols"), LegacyIdolValues);

			TArray<TSharedPtr<FJsonValue>> LegacyTierValues;
			LegacyTierValues.Add(MakeShared<FJsonValueNumber>(1));
			LegacyTierValues.Add(MakeShared<FJsonValueNumber>(2));
			LegacyTierValues.Add(MakeShared<FJsonValueNumber>(3));
			LegacyTierValues.Add(MakeShared<FJsonValueNumber>(4));
			LegacyIdolJson->SetArrayField(TEXT("equipped_idol_tiers"), LegacyTierValues);

			LegacyIdolParsedSummary = T66BackendRunSummaryParser::Parse(LegacyIdolJson, this);
		}
		const bool bLegacyIdolBackendParsed =
			LegacyIdolParsedSummary
			&& LegacyIdolParsedSummary->EquippedIdols.Num() == 4
			&& LegacyIdolParsedSummary->EquippedIdols[0] == FName(TEXT("Idol_Electricity_Pierce"))
			&& LegacyIdolParsedSummary->EquippedIdols[1] == FName(TEXT("Idol_Ice_AOE"))
			&& LegacyIdolParsedSummary->EquippedIdols[2] == FName(TEXT("Idol_Electricity_AOE"))
			&& LegacyIdolParsedSummary->EquippedIdols[3] == FName(TEXT("Idol_Nature_DOT"));
		UT66LocalizationSubsystem* Localization = GetGameInstance()
			? GetGameInstance()->GetSubsystem<UT66LocalizationSubsystem>()
			: nullptr;
		const FString CanonicalIdolName = Localization
			? Localization->GetText_IdolDisplayName(FName(TEXT("Idol_Fire_AOE"))).ToString()
			: FString();
		const FString LegacyIdolName = Localization
			? Localization->GetText_IdolDisplayName(FName(TEXT("Idol_Water"))).ToString()
			: FString();
		const FString LegacyIdolTooltip = Localization
			? Localization->GetText_IdolTooltip(FName(TEXT("Idol_Water"))).ToString()
			: FString();
		const bool bCanonicalIdolTextResolved =
			CanonicalIdolName == TEXT("FIRE AOE IDOL")
			&& LegacyIdolName == TEXT("ICE AOE IDOL");
		const bool bLegacyIdolTooltipResolved =
			!LegacyIdolTooltip.IsEmpty()
			&& !LegacyIdolTooltip.Equals(TEXT("Unknown."), ESearchCase::CaseSensitive);

		double JsonSchemaVersion = 0.0;
		const bool bSchemaVersionEmitted = RunJson.IsValid() && RunJson->TryGetNumberField(TEXT("schema_version"), JsonSchemaVersion);
		const int32 RoundedSchemaVersion = FMath::RoundToInt32(JsonSchemaVersion);
		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Backend schema_version emitted"), bSchemaVersionEmitted && RoundedSchemaVersion == T66CurrentRunSummarySchemaVersion, RunJson.IsValid() ? FString::Printf(TEXT("schema_version=%d expected=%d."), RoundedSchemaVersion, T66CurrentRunSummarySchemaVersion) : TEXT("Run JSON missing."));
		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Backend parse returned summary"), ParsedSummary != nullptr, ParsedSummary ? TEXT("Parser returned enriched summary.") : TEXT("Parser returned null."));
		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Legacy idol save IDs migrate to canonical IDs"), bLegacyIdolSaveMigrationChanged && bLegacyIdolSaveMigrated, FString::Printf(TEXT("Ids=%s,%s,%s,%s Tiers=%d,%d,%d,%d."), *LegacyIdolSaveIDs[0].ToString(), *LegacyIdolSaveIDs[1].ToString(), *LegacyIdolSaveIDs[2].ToString(), *LegacyIdolSaveIDs[3].ToString(), LegacyIdolSaveTiers[0], LegacyIdolSaveTiers[1], LegacyIdolSaveTiers[2], LegacyIdolSaveTiers[3]));
		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Legacy idol backend IDs parse as canonical IDs"), bLegacyIdolBackendParsed, LegacyIdolParsedSummary && LegacyIdolParsedSummary->EquippedIdols.Num() == 4 ? FString::Printf(TEXT("Parsed=%s,%s,%s,%s."), *LegacyIdolParsedSummary->EquippedIdols[0].ToString(), *LegacyIdolParsedSummary->EquippedIdols[1].ToString(), *LegacyIdolParsedSummary->EquippedIdols[2].ToString(), *LegacyIdolParsedSummary->EquippedIdols[3].ToString()) : TEXT("Legacy idol parser proof failed."));
		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Canonical idol localization resolves"), bCanonicalIdolTextResolved, FString::Printf(TEXT("CanonicalName='%s' LegacyName='%s'."), *CanonicalIdolName, *LegacyIdolName));
		T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Legacy idol tooltip resolves through migration"), bLegacyIdolTooltipResolved, FString::Printf(TEXT("LegacyTooltip='%s'."), *LegacyIdolTooltip));
		if (ParsedSummary)
		{
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Schema round-trips"), ParsedSummary->SchemaVersion == T66CurrentRunSummarySchemaVersion, FString::Printf(TEXT("Parsed=%d."), ParsedSummary->SchemaVersion));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Idol elements/types round-trip"), ParsedSummary->EquippedIdolElements.Num() == 4 && ParsedSummary->EquippedIdolCategories.Num() == 4 && ParsedSummary->EquippedIdolElements[2] == ET66IdolElement::Electricity && ParsedSummary->EquippedIdolCategories[3] == ET66AttackCategory::DOT, TEXT("Four enriched idol slots restored."));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Inventory rarity round-trips"), ParsedSummary->InventorySlots.Num() == 4 && ParsedSummary->InventorySlots[1].Rarity == ET66ItemRarity::Red && ParsedSummary->InventorySlots[3].Rarity == ET66ItemRarity::White, FString::Printf(TEXT("Slots=%d."), ParsedSummary->InventorySlots.Num()));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("No Idol round-trips"), ParsedSummary->NoIdolSelectionStacks == 2 && ParsedSummary->NoIdolPrimaryStatBonuses.AttackScaleTenths == 20, FString::Printf(TEXT("Stacks=%d AttackScaleTenths=%d."), ParsedSummary->NoIdolSelectionStacks, ParsedSummary->NoIdolPrimaryStatBonuses.AttackScaleTenths));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Elemental powers round-trip"), FMath::IsNearlyEqual(ParsedSummary->SecondaryStatValues.FindRef(ET66SecondaryStatType::FirePower), 4.25f) && FMath::IsNearlyEqual(ParsedSummary->SecondaryStatValues.FindRef(ET66SecondaryStatType::NaturePower), 1.5f), TEXT("Fire/Nature power values restored."));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Mob Loot round-trips"), ParsedSummary->MobLootQuantityCollectedThisRun == 42 && ParsedSummary->MobLootQuantitySoldThisRun == 30 && ParsedSummary->MobLootRemainingStack == 12 && ParsedSummary->MobLootQuantityCollectedByPetThisRun == 17, FString::Printf(TEXT("Collected=%d Sold=%d Remaining=%d Pet=%d."), ParsedSummary->MobLootQuantityCollectedThisRun, ParsedSummary->MobLootQuantitySoldThisRun, ParsedSummary->MobLootRemainingStack, ParsedSummary->MobLootQuantityCollectedByPetThisRun));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Gambler arrays round-trip"), ParsedSummary->AntiCheatGamblerSummaries.Num() == 1 && ParsedSummary->AntiCheatGamblerEvents.Num() == 1 && ParsedSummary->AntiCheatGamblerSummaries[0].TotalPayoutGold == 55 && ParsedSummary->AntiCheatGamblerEvents[0].ActionSequence == TEXT("proof"), FString::Printf(TEXT("Summaries=%d Events=%d."), ParsedSummary->AntiCheatGamblerSummaries.Num(), ParsedSummary->AntiCheatGamblerEvents.Num()));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Vendor fields round-trip"), ParsedSummary->CurrentGold == 100 && ParsedSummary->CurrentDebt == 5 && ParsedSummary->ActiveVendorTokenStacks == 16 && ParsedSummary->BuybackPoolSize == 2, FString::Printf(TEXT("Gold=%d Debt=%d TokenStacks=%d Buyback=%d."), ParsedSummary->CurrentGold, ParsedSummary->CurrentDebt, ParsedSummary->ActiveVendorTokenStacks, ParsedSummary->BuybackPoolSize));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Legacy vendor token level fallback"), LegacyVendorTokenParsedSummary && LegacyVendorTokenParsedSummary->ActiveVendorTokenStacks == 16, LegacyVendorTokenParsedSummary ? FString::Printf(TEXT("FallbackTokenStacks=%d."), LegacyVendorTokenParsedSummary->ActiveVendorTokenStacks) : TEXT("Legacy parse failed."));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Weapon structural data round-trips"), ParsedSummary->EquippedWeaponID == FName(TEXT("Weapon_Hero_1_Red_AOE")) && ParsedSummary->EquippedWeaponBranch == ET66AttackCategory::AOE && ParsedSummary->EquippedWeaponProjectileCount == 2 && FMath::IsNearlyEqual(ParsedSummary->EquippedWeaponSpreadAngleDegrees, 22.5f), FString::Printf(TEXT("Weapon=%s Count=%d Spread=%.1f."), *ParsedSummary->EquippedWeaponID.ToString(), ParsedSummary->EquippedWeaponProjectileCount, ParsedSummary->EquippedWeaponSpreadAngleDegrees));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Pet fields round-trip"), ParsedSummary->ActivePetID == FName(TEXT("Pet_Dungeon_Slime")) && ParsedSummary->ActivePetSkinID == FName(TEXT("Skin_Green")) && ParsedSummary->ActivePetBondStagesCleared == 6 && ParsedSummary->PetMobLootQuantityCollectedThisRun == 17, FString::Printf(TEXT("Pet=%s Skin=%s Bond=%d Loot=%d."), *ParsedSummary->ActivePetID.ToString(), *ParsedSummary->ActivePetSkinID.ToString(), ParsedSummary->ActivePetBondStagesCleared, ParsedSummary->PetMobLootQuantityCollectedThisRun));
			T66AppendSmokeCheck(Checks, bAllPassed, TEXT("Boss fields round-trip"), ParsedSummary->bBossActiveAtSummary && ParsedSummary->ActiveBossID == FName(TEXT("Dungeon_WebMatriarch")) && ParsedSummary->BossParts.Num() == 1 && ParsedSummary->BossParts[0].HitZoneType == ET66HitZoneType::Head && ParsedSummary->OwedBossIDs.Num() == 1, FString::Printf(TEXT("Boss=%s Parts=%d Owed=%d."), *ParsedSummary->ActiveBossID.ToString(), ParsedSummary->BossParts.Num(), ParsedSummary->OwedBossIDs.Num()));
		}

		const FString Json = FString::Printf(TEXT("{\n  \"ok\": %s,\n  \"slot\": \"%s\",\n  \"checks\": [\n%s\n  ]\n}\n"), bAllPassed ? TEXT("true") : TEXT("false"), *T66SmokeJsonEscape(SlotName), *FString::Join(Checks, TEXT(",\n")));
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);
		const bool bManifestSaved = FFileHelper::SaveStringToFile(Json, *OutputPath);
		UGameplayStatics::DeleteGameInSlot(SlotName, 0);
		UE_LOG(LogTemp, Display, TEXT("[RunSummaryRoundTrip] ok=%d saved=%d output=%s"), bAllPassed ? 1 : 0, bManifestSaved ? 1 : 0, *OutputPath);
		FPlatformMisc::RequestExitWithStatus(false, (bAllPassed && bManifestSaved) ? 0 : 104, TEXT("T66RunSummaryRoundTripComplete"));
		return;
	}

	if (Mode == TEXT("outgoingtravelerstress") || Mode == TEXT("outgoingtravelerstressab"))
	{
		float AutoCaptureHeroHPOverride = 0.0f;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66AutoCaptureHeroHPOverride="), AutoCaptureHeroHPOverride))
		{
			if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
			{
				const float AppliedHeroHPOverride = RunState->ApplyAutomationHeroHPOverride(
					AutoCaptureHeroHPOverride,
					TEXT("OutgoingTravelerStress"));
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[OutgoingTravelerStress] AutoCaptureHeroHPOverride AppliedHP=%.1f RequestedHP=%.1f MaxHP=%.1f CurrentHP=%.1f"),
					AppliedHeroHPOverride,
					AutoCaptureHeroHPOverride,
					RunState->GetMaxHP(),
					RunState->GetCurrentHP());
			}
		}

		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66OutgoingTravelerStressHarnessActor* StressHarness = World->SpawnActor<AT66OutgoingTravelerStressHarnessActor>(
				AT66OutgoingTravelerStressHarnessActor::StaticClass(),
				FTransform::Identity,
				SpawnParams))
			{
				StressHarness->StartFromCommandLine(this);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[OutgoingTravelerStress] failed to spawn stress harness actor."));
			}
		}
		return;
	}

	if (Mode == TEXT("moblootstress") || Mode == TEXT("moblootfoundation"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66MobLootStressHarnessActor* StressHarness = World->SpawnActor<AT66MobLootStressHarnessActor>(
				AT66MobLootStressHarnessActor::StaticClass(),
				FTransform::Identity,
				SpawnParams))
			{
				StressHarness->StartFromCommandLine(this);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[MobLootStress] failed to spawn stress harness actor."));
			}
		}
		return;
	}

	if (Mode == TEXT("trapprojectilehitbox") || Mode == TEXT("trapprojectiledebug"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (UWorld* World = GetWorld())
		{
			const FVector Forward = GetControlRotation().Vector().GetSafeNormal();
			const FVector ProjectilePreviewLocation = GetPawn()
				? GetPawn()->GetActorLocation() + Forward * 560.f + FVector(0.f, 0.f, 110.f)
				: FVector(560.f, 0.f, 160.f);
			const FTransform SpawnTransform(Forward.Rotation(), ProjectilePreviewLocation);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66TrapArrowProjectile* Projectile = World->SpawnActor<AT66TrapArrowProjectile>(
				AT66TrapArrowProjectile::StaticClass(),
				SpawnTransform,
				SpawnParams))
			{
				Projectile->InitializeProjectile(
					Forward,
					12,
					0.f,
					FT66TemporaryProjectileSystem::HostileProjectileColor(),
					FT66TemporaryProjectileSystem::HostileProjectileColor());
			}
		}
		return;
	}

	if (Mode == TEXT("trapcontainers") || Mode == TEXT("trapdebugcontainers") || Mode == TEXT("alltrapcontainers"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (UWorld* World = GetWorld())
		{
			const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
			const FVector Forward = YawRotation.Vector().GetSafeNormal();
			const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			FVector PreviewOrigin = GetPawn()
				? GetPawn()->GetActorLocation() + Forward * 920.f
				: FVector(920.f, 0.f, 0.f);
			PreviewOrigin.Z -= 80.f;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			const FTransform FlameTransform(YawRotation, PreviewOrigin - Right * 360.f);
			if (AT66FloorFlameTrap* FlameTrap = World->SpawnActorDeferred<AT66FloorFlameTrap>(
				AT66FloorFlameTrap::StaticClass(),
				FlameTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				FlameTrap->SetActivationMode(ET66TrapActivationMode::Timed);
				FlameTrap->Radius = 190.f;
				FlameTrap->DamageHP = 10;
				FlameTrap->InitialCycleDelaySeconds = 0.05f;
				FlameTrap->WarningDurationSeconds = 0.1f;
				FlameTrap->ActiveDurationSeconds = 8.f;
				FlameTrap->FinishSpawning(FlameTransform);
			}

			const FTransform SpikeTransform(YawRotation, PreviewOrigin + Right * 360.f);
			if (AT66FloorSpikePatchTrap* SpikeTrap = World->SpawnActorDeferred<AT66FloorSpikePatchTrap>(
				AT66FloorSpikePatchTrap::StaticClass(),
				SpikeTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				SpikeTrap->SetActivationMode(ET66TrapActivationMode::Timed);
				SpikeTrap->Radius = 205.f;
				SpikeTrap->DamageHP = 11;
				SpikeTrap->InitialCycleDelaySeconds = 0.05f;
				SpikeTrap->WarningDurationSeconds = 0.1f;
				SpikeTrap->RiseDurationSeconds = 0.1f;
				SpikeTrap->RaisedDurationSeconds = 8.f;
				SpikeTrap->FinishSpawning(SpikeTransform);
			}

			World->SpawnActor<AT66TrapPressurePlate>(
				AT66TrapPressurePlate::StaticClass(),
				PreviewOrigin + Forward * 180.f,
				YawRotation,
				SpawnParams);

			const FVector ProjectilePreviewLocation = PreviewOrigin + Right * 40.f - Forward * 260.f + FVector(0.f, 0.f, 170.f);
			const FTransform ProjectileTransform(Forward.Rotation(), ProjectilePreviewLocation);
			if (AT66TrapArrowProjectile* Projectile = World->SpawnActor<AT66TrapArrowProjectile>(
				AT66TrapArrowProjectile::StaticClass(),
				ProjectileTransform,
				SpawnParams))
			{
				Projectile->InitializeProjectile(
					Forward,
					12,
					0.f,
					FT66TemporaryProjectileSystem::HostileProjectileColor(),
					FT66TemporaryProjectileSystem::HostileProjectileColor());
			}
		}
		return;
	}
#endif

	if (Mode == TEXT("enemylock") || Mode == TEXT("lockindicator") || Mode == TEXT("enemylockwidget"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66WidgetDump_EnemyLockTarget"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
				AT66EnemyBase::StaticClass(),
				GetPawn() ? GetPawn()->GetActorLocation() + FVector(420.f, 0.f, 0.f) : FVector(420.f, 0.f, 0.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (Enemy)
			{
				Enemy->SetActorEnableCollision(false);
				Enemy->SetLockedIndicator(true);
			}
		}
		return;
	}

	if (Mode == TEXT("floatingcombattext") || Mode == TEXT("combattext") || Mode == TEXT("damagetext"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		AActor* TextTarget = GetPawn();
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66WidgetDump_FloatingCombatTextTarget"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
				AT66EnemyBase::StaticClass(),
				GetPawn() ? GetPawn()->GetActorLocation() + FVector(420.f, 0.f, 0.f) : FVector(420.f, 0.f, 0.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (Enemy)
			{
				Enemy->SetActorEnableCollision(false);
				TextTarget = Enemy;
			}
		}

		if (UT66FloatingCombatTextSubsystem* FloatingText = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr)
		{
			FloatingText->ShowDamageNumber(TextTarget, 666, UT66FloatingCombatTextSubsystem::EventType_Crit);
		}
		return;
	}

#if !UE_BUILD_SHIPPING
	if (Mode == TEXT("bossprojectilemanager") || Mode == TEXT("bossprojectilemanagersmoke"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		if (!World || !GameMode || !T66GI || !RunState || !Registry)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossProjectileManagerSmoke] Missing world/game mode/game instance/run state/registry."));
			return;
		}

		FString BossSmokeMode;
		FParse::Value(FCommandLine::Get(), TEXT("T66BossProjectileSmoke="), BossSmokeMode);
		const bool bFourHorsemenSmoke = BossSmokeMode.TrimStartAndEnd().Equals(TEXT("FourHorsemen"), ESearchCase::IgnoreCase);
		float BossSmokeHeroHP = 20000.f;
		FParse::Value(FCommandLine::Get(), TEXT("T66BossProjectileSmokeHeroHP="), BossSmokeHeroHP);
		RunState->ApplyAutomationHeroHPOverride(FMath::Clamp(BossSmokeHeroHP, 100.f, 50000.f), TEXT("BossProjectileManagerSmoke"));
		if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
		{
			ProjectileManager->ResetProjectileDiagnostics(TEXT("BossProjectileManagerSmokeStart"));
		}

		if (bFourHorsemenSmoke)
		{
			const TArray<TWeakObjectPtr<AT66BossBase>> ExistingBosses = Registry->GetBosses();
			for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : ExistingBosses)
			{
				if (AT66BossBase* ExistingBoss = WeakBoss.Get())
				{
					ExistingBoss->Destroy();
				}
			}
			T66GI->SelectedDifficulty = ET66Difficulty::Impossible;
			RunState->SetCurrentStage(17);
			GameMode->RunBossProjectileManagerSmokeSpawnBossForCurrentStage();
			UE_LOG(LogTemp, Display, TEXT("[BossProjectileManagerSmoke] Stage17FourHorsemenSetup SelectedDifficulty=Impossible CurrentStage=%d"), RunState->GetCurrentStage());
		}

		auto ConfigureBossesForSmoke = [Registry]()
		{
			const TArray<TWeakObjectPtr<AT66BossBase>> Bosses = Registry->GetBosses();
			for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Bosses)
			{
				if (AT66BossBase* Boss = WeakBoss.Get())
				{
					Boss->GroundAOEIntervalSeconds = 0.f;
					Boss->GroundAOEBaseDamageHP = 0;
				}
			}
		};
		ConfigureBossesForSmoke();

		APawn* ControlledPawn = GetPawn();
		if (ControlledPawn)
		{
			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
				}
			}

			T66TowerMapTerrain::FLayout TowerLayout;
			const bool bHasTowerLayout = GameMode->IsUsingTowerMainMapLayout() && GameMode->GetTowerMainMapLayout(TowerLayout);
			const T66TowerMapTerrain::FFloor* BossFloor = nullptr;
			if (bHasTowerLayout)
			{
				for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
				{
					if (Floor.FloorNumber == TowerLayout.BossFloorNumber)
					{
						BossFloor = &Floor;
						break;
					}
				}
			}

			FVector BossEntryLocation = BossFloor
				? (BossFloor->ArrivalPoint.IsNearlyZero() ? FVector(BossFloor->Center.X, BossFloor->Center.Y, BossFloor->SurfaceZ) : BossFloor->ArrivalPoint)
				: T66GameplayLayout::GetBossGateLocation();
			float PawnHalfHeight = 100.f;
			if (const ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (const UCapsuleComponent* Capsule = CharacterPawn->GetCapsuleComponent())
				{
					PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
				}
			}
			if (BossFloor)
			{
				BossEntryLocation.Z = BossFloor->SurfaceZ + PawnHalfHeight + 24.f;
			}
			else
			{
				FHitResult GateHit;
				if (World->LineTraceSingleByChannel(GateHit, BossEntryLocation + FVector(0.f, 0.f, 3000.f), BossEntryLocation - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
				{
					BossEntryLocation.Z = GateHit.ImpactPoint.Z + PawnHalfHeight + 24.f;
				}
			}
			ControlledPawn->SetActorLocation(BossEntryLocation, false, nullptr, ETeleportType::TeleportPhysics);
			ControlledPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f), ETeleportType::TeleportPhysics);
			SetControlRotation(FRotator(-10.f, 0.f, 0.f));

			if (bHasTowerLayout && BossFloor)
			{
				GameMode->HandleTowerDescentHoleTriggered(ControlledPawn, FMath::Max(TowerLayout.StartFloorNumber, TowerLayout.BossFloorNumber - 1), TowerLayout.BossFloorNumber);
			}

			for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
			{
				if (AT66BossBase* Boss = WeakBoss.Get())
				{
					const FVector SmokeHeroLocation = Boss->GetActorLocation() + FVector(0.f, 650.f, 0.f);
					ControlledPawn->SetActorLocation(SmokeHeroLocation, false, nullptr, ETeleportType::TeleportPhysics);
					break;
				}
			}
		}

		GameMode->RunBossProjectileManagerSmokeSpawnBossGateIfNeeded();
		GameMode->SetEnemyDirectorSpawningPaused(true);
		UE_LOG(LogTemp, Display, TEXT("[BossProjectileManagerSmoke] BossGatePathArmed BossCount=%d FourHorsemen=%d"), Registry->GetBosses().Num(), bFourHorsemenSmoke ? 1 : 0);

		const bool bKillMidFlight = FParse::Param(FCommandLine::Get(), TEXT("T66BossProjectileSmokeKillMidFlight"));
		float BossSmokeDuration = 18.f;
		FParse::Value(FCommandLine::Get(), TEXT("T66BossProjectileSmokeDuration="), BossSmokeDuration);
		BossSmokeDuration = FMath::Clamp(BossSmokeDuration, 4.f, 60.f);
		if (bKillMidFlight)
		{
			TWeakObjectPtr<AT66PlayerController> WeakThis(this);
			FTimerHandle KillTimerHandle;
			World->GetTimerManager().SetTimer(
				KillTimerHandle,
				FTimerDelegate::CreateLambda([WeakThis]()
				{
					AT66PlayerController* PC = WeakThis.Get();
					UWorld* TimerWorld = PC ? PC->GetWorld() : nullptr;
					UT66ActorRegistrySubsystem* TimerRegistry = TimerWorld ? TimerWorld->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
					if (!TimerRegistry)
					{
						return;
					}
					const TArray<TWeakObjectPtr<AT66BossBase>> Bosses = TimerRegistry->GetBosses();
					for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Bosses)
					{
						AT66BossBase* Boss = WeakBoss.Get();
						if (Boss && Boss->IsAwakened() && Boss->IsAlive())
						{
							const int32 LethalDamage = Boss->CurrentHP + (Boss->MaxHP * 100) + 100000;
							Boss->TakeDamageFromHeroHit(LethalDamage, FName(TEXT("BossProjectileSmokeKillMidFlight")), FName(TEXT("BossProjectileSmokeKillMidFlight")));
							if (Boss->IsAlive())
							{
								Boss->CurrentHP = 0;
								Boss->RefreshRunStateBossState();
							}
							UE_LOG(LogTemp, Display, TEXT("[BossProjectileManagerSmoke] KillMidFlightApplied BossID=%s Damage=%d AliveAfter=%d"), *Boss->BossID.ToString(), LethalDamage, Boss->IsAlive() ? 1 : 0);
							break;
						}
					}
				}),
				FMath::Min(2.5f, BossSmokeDuration * 0.5f),
				false);
		}

		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		FTimerHandle SummaryTimerHandle;
		World->GetTimerManager().SetTimer(
			SummaryTimerHandle,
			FTimerDelegate::CreateLambda([WeakThis]()
			{
				AT66PlayerController* PC = WeakThis.Get();
				UWorld* TimerWorld = PC ? PC->GetWorld() : nullptr;
				if (!TimerWorld)
				{
					return;
				}
				if (UT66ActorRegistrySubsystem* TimerRegistry = TimerWorld->GetSubsystem<UT66ActorRegistrySubsystem>())
				{
					const TArray<TWeakObjectPtr<AT66BossBase>> Bosses = TimerRegistry->GetBosses();
					for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Bosses)
					{
						if (AT66BossBase* Boss = WeakBoss.Get())
						{
							UE_LOG(LogTemp, Display, TEXT("[BossProjectileManagerSmoke] BossState BossID=%s Awakened=%d Alive=%d HP=%d Loc=%s"),
								*Boss->BossID.ToString(),
								Boss->IsAwakened() ? 1 : 0,
								Boss->IsAlive() ? 1 : 0,
								Boss->CurrentHP,
								*Boss->GetActorLocation().ToCompactString());
						}
					}
				}
				if (UT66ProjectileManagerSubsystem* ProjectileManager = TimerWorld->GetSubsystem<UT66ProjectileManagerSubsystem>())
				{
					ProjectileManager->EmitProjectileManagerSummary(TEXT("BossProjectileManagerSmoke"), true);
				}
				FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("T66BossProjectileManagerSmokeComplete"));
			}),
			BossSmokeDuration,
			false);
		return;
	}

#if !UE_BUILD_SHIPPING
	if (Mode == TEXT("bosspartownershipa1"))
	{
		UWorld* World = GetWorld();
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		if (!World || !T66GI)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossPartOwnershipA1] Missing world or T66GameInstance."));
			FPlatformMisc::RequestExitWithStatus(false, 71, TEXT("BossPartOwnershipA1MissingWorld"));
			return;
		}

		APawn* ControlledPawn = GetPawn();
		if (ControlledPawn)
		{
			ControlledPawn->SetActorEnableCollision(false);
			ControlledPawn->SetActorLocation(T66GameplayLayout::GetStartAreaCenter(120.f), false);
			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->DisableMovement();
				}
			}
		}

		auto BuildBossData = [T66GI]()
		{
			FBossData BossData;
			if (!T66GI->GetBossData(FName(TEXT("Dungeon_SewerSlimeKing")), BossData))
			{
				BossData.BossID = FName(TEXT("Dungeon_SewerSlimeKing"));
				BossData.MaxHP = 1250;
				BossData.AwakenDistance = 2400.f;
				BossData.MoveSpeed = 0.f;
				BossData.FireIntervalSeconds = 999.f;
				BossData.ProjectileSpeed = 760.f;
				BossData.ProjectileDamageHearts = 1;
				BossData.BossPartProfile = ET66BossPartProfile::Juggernaut;
				BossData.PlaceholderColor = FLinearColor(0.20f, 0.92f, 0.08f, 1.f);
			}
			BossData.MoveSpeed = 0.f;
			BossData.FireIntervalSeconds = 999.f;
			BossData.AwakenDistance = 2400.f;
			return BossData;
		};

		auto SpawnProofBoss = [World, BuildBossData](const FName Name, const FVector& Location) -> AT66BossBase*
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = Name;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
				AT66BossBase::StaticClass(),
				Location,
				FRotator(0.f, 180.f, 0.f),
				SpawnParams);
			if (!Boss)
			{
				return nullptr;
			}
			Boss->Tags.AddUnique(FName(TEXT("T66Automation_BossPartOwnershipA1")));
			Boss->InitializeBoss(BuildBossData());
			Boss->ForceAwaken();
			if (UCharacterMovementComponent* BossMovement = Boss->GetCharacterMovement())
			{
				BossMovement->StopMovementImmediately();
				BossMovement->DisableMovement();
			}
			FT66VisualUtil::SnapToGround(Boss, World);
			return Boss;
		};

		AT66BossBase* ImmediateBoss = SpawnProofBoss(
			FName(TEXT("T66BossPartOwnershipA1_Immediate")),
			T66GameplayLayout::GetStartAreaCenter(120.f) + FVector(780.f, 0.f, 0.f));
		AT66BossBase* WindupBoss = SpawnProofBoss(
			FName(TEXT("T66BossPartOwnershipA1_Windup")),
			T66GameplayLayout::GetStartAreaCenter(120.f) + FVector(1180.f, 0.f, 0.f));

		if (!ImmediateBoss || !WindupBoss)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossPartOwnershipA1] Failed to spawn proof bosses."));
			FPlatformMisc::RequestExitWithStatus(false, 72, TEXT("BossPartOwnershipA1SpawnFailed"));
			return;
		}

		auto Counter = [](AT66BossBase* Boss, const TCHAR* EventID, const TCHAR* AttackID, const TCHAR* PartID)
		{
			return Boss
				? Boss->GetBossAttackOwnershipAutomationCounter(FName(EventID), FName(AttackID), FName(PartID))
				: 0;
		};

		int32 FailureCount = 0;

		ImmediateBoss->ResetBossAttackOwnershipAutomationCounters();
		ImmediateBoss->KillBossPartForAutomation(FName(TEXT("LeftLobe")));
		ImmediateBoss->ForceSewerSlimeKingAttackForAutomation(FName(TEXT("LeftLobe")));
		ImmediateBoss->ForceSewerSlimeKingAttackForAutomation(FName(TEXT("RightLobe")));
		const bool bLobeSuppressedOnly = Counter(ImmediateBoss, TEXT("Suppressed"), TEXT("SewerSlime_LobeVolley"), TEXT("LeftLobe")) > 0
			&& Counter(ImmediateBoss, TEXT("Queued"), TEXT("SewerSlime_LobeVolley"), TEXT("RightLobe")) > 0;
		FailureCount += bLobeSuppressedOnly ? 0 : 1;

		ImmediateBoss->ResetBossAttackOwnershipAutomationCounters();
		ImmediateBoss->KillBossPartForAutomation(FName(TEXT("LeftBase")));
		ImmediateBoss->ForceSewerSlimeKingAttackForAutomation(FName(TEXT("LeftBase")));
		ImmediateBoss->ForceSewerSlimeKingAttackForAutomation(FName(TEXT("RightBase")));
		const bool bBaseSuppressedOnly = Counter(ImmediateBoss, TEXT("Suppressed"), TEXT("SewerSlime_LaneBlocker"), TEXT("LeftBase")) > 0
			&& Counter(ImmediateBoss, TEXT("Queued"), TEXT("SewerSlime_LaneBlocker"), TEXT("RightBase")) > 0
			&& Counter(ImmediateBoss, TEXT("Fired"), TEXT("SewerSlime_LaneBlocker"), TEXT("RightBase")) > 0;
		FailureCount += bBaseSuppressedOnly ? 0 : 1;

		ImmediateBoss->ResetBossAttackOwnershipAutomationCounters();
		ImmediateBoss->KillBossPartForAutomation(FName(TEXT("MouthCore")));
		ImmediateBoss->ForceSewerSlimeKingAttackForAutomation(FName(TEXT("MouthCore")));
		ImmediateBoss->ForceSewerSlimeKingAttackForAutomation(FName(TEXT("RightBase")));
		const bool bMouthSuppressedOnly = Counter(ImmediateBoss, TEXT("Suppressed"), TEXT("SewerSlime_MouthProjectile"), TEXT("MouthCore")) > 0
			&& Counter(ImmediateBoss, TEXT("Suppressed"), TEXT("SewerSlime_MouthSidecar"), TEXT("MouthCore")) > 0
			&& Counter(ImmediateBoss, TEXT("Queued"), TEXT("SewerSlime_LaneBlocker"), TEXT("RightBase")) > 0
			&& Counter(ImmediateBoss, TEXT("Fired"), TEXT("SewerSlime_LaneBlocker"), TEXT("RightBase")) > 0;
		FailureCount += bMouthSuppressedOnly ? 0 : 1;

		UE_LOG(
			LogTemp,
			Display,
			TEXT("[BossPartOwnershipA1] ImmediateGate LobeSuppressedOnly=%d BaseSuppressedOnly=%d MouthSuppressedOnly=%d"),
			bLobeSuppressedOnly ? 1 : 0,
			bBaseSuppressedOnly ? 1 : 0,
			bMouthSuppressedOnly ? 1 : 0);

		WindupBoss->ResetBossAttackOwnershipAutomationCounters();
		WindupBoss->ForceSewerSlimeKingAttackForAutomation(FName(TEXT("LeftLobe")));
		FTimerHandle KillWindupPartHandle;
		World->GetTimerManager().SetTimer(
			KillWindupPartHandle,
			FTimerDelegate::CreateWeakLambda(WindupBoss, [WindupBoss]()
			{
				WindupBoss->KillBossPartForAutomation(FName(TEXT("LeftLobe")));
			}),
			0.25f,
			false);

		FTimerHandle SummaryHandle;
		World->GetTimerManager().SetTimer(
			SummaryHandle,
			FTimerDelegate::CreateLambda([WindupBoss, FailureCount, Counter]()
			{
				const int32 LobeFired = Counter(WindupBoss, TEXT("Fired"), TEXT("SewerSlime_LobeVolley"), TEXT("LeftLobe"));
				const int32 LobeSuppressed = Counter(WindupBoss, TEXT("Suppressed"), TEXT("SewerSlime_LobeVolley"), TEXT("LeftLobe"));
				const bool bWindupInterrupted = LobeFired == 0 && LobeSuppressed > 0;
				const int32 TotalFailures = FailureCount + (bWindupInterrupted ? 0 : 1);
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[BossPartOwnershipA1] WindupGate LobeDelayedShotsFired=%d LobeDelayedShotsSuppressed=%d WindupInterrupted=%d"),
					LobeFired,
					LobeSuppressed,
					bWindupInterrupted ? 1 : 0);
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[BossPartOwnershipA1] Summary Pass=%d Failures=%d BounceTargetHandlePathUnchanged=1 AttackRowsSource=DT_BossAttacks"),
					TotalFailures == 0 ? 1 : 0,
					TotalFailures);
				FPlatformMisc::RequestExitWithStatus(false, TotalFailures == 0 ? 0 : 73, TEXT("BossPartOwnershipA1Complete"));
			}),
			1.55f,
			false);
		return;
	}

	if (Mode == TEXT("bosspartownershipa2"))
	{
		UWorld* World = GetWorld();
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		if (!World || !T66GI)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossPartOwnershipA2] Missing world or T66GameInstance."));
			FPlatformMisc::RequestExitWithStatus(false, 81, TEXT("BossPartOwnershipA2MissingWorld"));
			return;
		}

		APawn* ControlledPawn = GetPawn();
		if (ControlledPawn)
		{
			ControlledPawn->SetActorEnableCollision(false);
			ControlledPawn->SetActorLocation(T66GameplayLayout::GetStartAreaCenter(120.f), false);
			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->DisableMovement();
				}
			}
		}

		FBossData BossData;
		if (!T66GI->GetBossData(FName(TEXT("Dungeon_WebMatriarch")), BossData))
		{
			BossData.BossID = FName(TEXT("Dungeon_WebMatriarch"));
			BossData.MaxHP = 1500;
			BossData.AwakenDistance = 2400.f;
			BossData.MoveSpeed = 0.f;
			BossData.FireIntervalSeconds = 999.f;
			BossData.ProjectileSpeed = 930.f;
			BossData.ProjectileDamageHearts = 1;
			BossData.BossPartProfile = ET66BossPartProfile::Sharpshooter;
			BossData.PlaceholderColor = FLinearColor(0.50f, 0.42f, 0.68f, 1.f);
		}
		BossData.MoveSpeed = 0.f;
		BossData.FireIntervalSeconds = 999.f;
		BossData.AwakenDistance = 2400.f;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(TEXT("T66BossPartOwnershipA2_WebMatriarch"));
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
			AT66BossBase::StaticClass(),
			T66GameplayLayout::GetStartAreaCenter(120.f) + FVector(820.f, 0.f, 0.f),
			FRotator(0.f, 180.f, 0.f),
			SpawnParams);
		if (!Boss)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossPartOwnershipA2] Failed to spawn Web Matriarch proof boss."));
			FPlatformMisc::RequestExitWithStatus(false, 82, TEXT("BossPartOwnershipA2SpawnFailed"));
			return;
		}

		Boss->Tags.AddUnique(FName(TEXT("T66Automation_BossPartOwnershipA2")));
		Boss->InitializeBoss(BossData);
		Boss->ForceAwaken();
		if (UCharacterMovementComponent* BossMovement = Boss->GetCharacterMovement())
		{
			BossMovement->StopMovementImmediately();
			BossMovement->DisableMovement();
		}
		FT66VisualUtil::SnapToGround(Boss, World);

		auto Counter = [](AT66BossBase* InBoss, const TCHAR* EventID, const TCHAR* AttackID, const TCHAR* PartID)
		{
			return InBoss
				? InBoss->GetBossAttackOwnershipAutomationCounter(FName(EventID), FName(AttackID), FName(PartID))
				: 0;
		};

		Boss->ResetBossAttackOwnershipAutomationCounters();
		Boss->ForceBossAttackForAutomation(FName(TEXT("LegacyProjectile_Sharpshooter")), FName(TEXT("Head")));

		FTimerHandle ProjectileAliveCheckHandle;
		World->GetTimerManager().SetTimer(
			ProjectileAliveCheckHandle,
			FTimerDelegate::CreateLambda([World, Boss, Counter]()
			{
				const bool bAuthoredProjectileFired =
					Counter(Boss, TEXT("Queued"), TEXT("LegacyProjectile_Sharpshooter"), TEXT("Head")) > 0
					&& Counter(Boss, TEXT("Fired"), TEXT("LegacyProjectile_Sharpshooter"), TEXT("Head")) > 0;

				Boss->ResetBossAttackOwnershipAutomationCounters();
				Boss->KillBossPartForAutomation(FName(TEXT("Head")));
				Boss->ForceBossAttackForAutomation(FName(TEXT("LegacyProjectile_Sharpshooter")), FName(TEXT("Head")));

				FTimerHandle ProjectileDeadCheckHandle;
				World->GetTimerManager().SetTimer(
					ProjectileDeadCheckHandle,
					FTimerDelegate::CreateLambda([World, Boss, Counter, bAuthoredProjectileFired]()
					{
						const bool bDeadOwnerSuppressed =
							Counter(Boss, TEXT("Suppressed"), TEXT("LegacyProjectile_Sharpshooter"), TEXT("Head")) > 0
							&& Counter(Boss, TEXT("Fired"), TEXT("LegacyProjectile_Sharpshooter"), TEXT("Head")) == 0;

						Boss->ResetBossAttackOwnershipAutomationCounters();
						Boss->ForceBossAttackForAutomation(FName(TEXT("LegacyGroundAOE_Sharpshooter")), FName(TEXT("Core")));

						FTimerHandle AOECheckHandle;
						World->GetTimerManager().SetTimer(
							AOECheckHandle,
							FTimerDelegate::CreateLambda([Boss, Counter, bAuthoredProjectileFired, bDeadOwnerSuppressed]()
							{
								const bool bOtherOwnedAOEFired =
									Counter(Boss, TEXT("Queued"), TEXT("LegacyGroundAOE_Sharpshooter"), TEXT("Core")) > 0
									&& Counter(Boss, TEXT("Fired"), TEXT("LegacyGroundAOE_Sharpshooter"), TEXT("Core")) > 0;
								const int32 FailureCount =
									(bAuthoredProjectileFired ? 0 : 1)
									+ (bDeadOwnerSuppressed ? 0 : 1)
									+ (bOtherOwnedAOEFired ? 0 : 1);

								UE_LOG(
									LogTemp,
									Display,
									TEXT("[BossPartOwnershipA2] Gate AuthoredProjectileFired=%d DeadOwnerSuppressed=%d OtherOwnedAOEFired=%d"),
									bAuthoredProjectileFired ? 1 : 0,
									bDeadOwnerSuppressed ? 1 : 0,
									bOtherOwnedAOEFired ? 1 : 0);
								UE_LOG(
									LogTemp,
									Display,
									TEXT("[BossPartOwnershipA2] Summary Pass=%d Failures=%d BossID=Dungeon_WebMatriarch AttackRowsSource=DT_BossAttacks SeamAttackID=LegacyProjectile_Sharpshooter"),
									FailureCount == 0 ? 1 : 0,
									FailureCount);
								FPlatformMisc::RequestExitWithStatus(false, FailureCount == 0 ? 0 : 83, TEXT("BossPartOwnershipA2Complete"));
							}),
							0.20f,
							false);
					}),
					0.20f,
					false);
			}),
			0.55f,
			false);
		return;
	}

	if (Mode == TEXT("bossattackdefinitionproof"))
	{
		UWorld* World = GetWorld();
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		if (!World || !T66GI)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossAttackDefinitionProof] Missing world or T66GameInstance."));
			FPlatformMisc::RequestExitWithStatus(false, 101, TEXT("BossAttackDefinitionProofMissingWorld"));
			return;
		}

		APawn* ControlledPawn = GetPawn();
		if (ControlledPawn)
		{
			ControlledPawn->SetActorEnableCollision(false);
			ControlledPawn->SetActorLocation(T66GameplayLayout::GetStartAreaCenter(120.f), false);
			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->DisableMovement();
				}
			}
		}

		TArray<FT66BossAttackDefinitionData> DefinitionRows;
		T66GI->GetBossAttackDefinitionRows(FName(TEXT("LegacyProjectile_Sharpshooter")), 0, DefinitionRows);

		FBossData BossData;
		if (!T66GI->GetBossData(FName(TEXT("Dungeon_WebMatriarch")), BossData))
		{
			BossData.BossID = FName(TEXT("Dungeon_WebMatriarch"));
			BossData.MaxHP = 1500;
			BossData.AwakenDistance = 2400.f;
			BossData.MoveSpeed = 0.f;
			BossData.FireIntervalSeconds = 999.f;
			BossData.ProjectileSpeed = 930.f;
			BossData.ProjectileDamageHearts = 1;
			BossData.BossPartProfile = ET66BossPartProfile::Sharpshooter;
			BossData.PlaceholderColor = FLinearColor(0.50f, 0.42f, 0.68f, 1.f);
		}
		BossData.MoveSpeed = 0.f;
		BossData.FireIntervalSeconds = 999.f;
		BossData.AwakenDistance = 2400.f;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(TEXT("T66BossAttackDefinitionProof_WebMatriarch"));
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
			AT66BossBase::StaticClass(),
			T66GameplayLayout::GetStartAreaCenter(120.f) + FVector(820.f, 0.f, 0.f),
			FRotator(0.f, 180.f, 0.f),
			SpawnParams);
		if (!Boss)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossAttackDefinitionProof] Failed to spawn Web Matriarch proof boss."));
			FPlatformMisc::RequestExitWithStatus(false, 102, TEXT("BossAttackDefinitionProofSpawnFailed"));
			return;
		}

		Boss->Tags.AddUnique(FName(TEXT("T66Automation_BossAttackDefinitionProof")));
		Boss->InitializeBoss(BossData);
		Boss->ForceAwaken();
		if (UCharacterMovementComponent* BossMovement = Boss->GetCharacterMovement())
		{
			BossMovement->StopMovementImmediately();
			BossMovement->DisableMovement();
		}
		FT66VisualUtil::SnapToGround(Boss, World);

		if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
		{
			ProjectileManager->ResetProjectileDiagnostics(TEXT("BossAttackDefinitionProof"));
		}

		auto Counter = [](AT66BossBase* InBoss, const TCHAR* EventID, const TCHAR* AttackID, const TCHAR* PartID)
		{
			return InBoss
				? InBoss->GetBossAttackOwnershipAutomationCounter(FName(EventID), FName(AttackID), FName(PartID))
				: 0;
		};

		Boss->ResetBossAttackOwnershipAutomationCounters();
		Boss->ForceBossAttackForAutomation(FName(TEXT("LegacyProjectile_Sharpshooter")), FName(TEXT("Head")));

		FTimerHandle DefinitionCheckHandle;
		World->GetTimerManager().SetTimer(
			DefinitionCheckHandle,
			FTimerDelegate::CreateLambda([World, Boss, Counter, DefinitionRowCount = DefinitionRows.Num()]()
			{
				const int32 QueuedCount = Counter(Boss, TEXT("Queued"), TEXT("LegacyProjectile_Sharpshooter"), TEXT("Head"));
				const int32 FiredCount = Counter(Boss, TEXT("Fired"), TEXT("LegacyProjectile_Sharpshooter"), TEXT("Head"));
				const UT66ProjectileManagerSubsystem* ProjectileManager = World ? World->GetSubsystem<UT66ProjectileManagerSubsystem>() : nullptr;
				const FT66ProjectileManagerDiagnostics Diagnostics = ProjectileManager
					? ProjectileManager->GetDiagnostics()
					: FT66ProjectileManagerDiagnostics{};
				const bool bDefinitionRowsLoaded = DefinitionRowCount > 0;
				const bool bSchedulerQueued = QueuedCount > 0;
				const bool bManagedProjectileFired = FiredCount > 0 && Diagnostics.ProjectilesActivePeak > 0;
				const bool bVisualProfileResolved = Diagnostics.VisualProfilesResolved > 0 && Diagnostics.VisualProfileFallbacks == 0;
				const int32 FailureCount =
					(bDefinitionRowsLoaded ? 0 : 1)
					+ (bSchedulerQueued ? 0 : 1)
					+ (bManagedProjectileFired ? 0 : 1)
					+ (bVisualProfileResolved ? 0 : 1);

				UE_LOG(
					LogTemp,
					Display,
					TEXT("[BossAttackDefinitionProof] Gate DefinitionRowsLoaded=%d SchedulerQueued=%d ManagedProjectileFired=%d VisualProfileResolved=%d DefinitionRows=%d Queued=%d Fired=%d ActivePeak=%d VisualProfilesResolved=%d VisualProfileFallbacks=%d"),
					bDefinitionRowsLoaded ? 1 : 0,
					bSchedulerQueued ? 1 : 0,
					bManagedProjectileFired ? 1 : 0,
					bVisualProfileResolved ? 1 : 0,
					DefinitionRowCount,
					QueuedCount,
					FiredCount,
					Diagnostics.ProjectilesActivePeak,
					Diagnostics.VisualProfilesResolved,
					Diagnostics.VisualProfileFallbacks);
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[BossAttackDefinitionProof] Summary Pass=%d Failures=%d BossID=Dungeon_WebMatriarch AttackRowsSource=DT_BossAttacks AttackDefinitionsSource=DT_BossAttackDefinitions AttackID=LegacyProjectile_Sharpshooter OwningPartID=Head VisualProfile=ManagedProjectile.Boss.WebNeedle"),
					FailureCount == 0 ? 1 : 0,
					FailureCount);

				if (UT66ProjectileManagerSubsystem* MutableProjectileManager = World ? World->GetSubsystem<UT66ProjectileManagerSubsystem>() : nullptr)
				{
					MutableProjectileManager->EmitProjectileManagerSummary(TEXT("BossAttackDefinitionProof"), true);
				}
				FPlatformMisc::RequestExitWithStatus(false, FailureCount == 0 ? 0 : 103, TEXT("BossAttackDefinitionProofComplete"));
			}),
			0.65f,
			false);
		return;
	}

	if (Mode == TEXT("bosspartmovementb1"))
	{
		UWorld* World = GetWorld();
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		if (!World || !T66GI)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossMovementB1] Missing world or T66GameInstance."));
			FPlatformMisc::RequestExitWithStatus(false, 91, TEXT("BossMovementB1MissingWorld"));
			return;
		}

		APawn* ControlledPawn = GetPawn();
		const FVector PlayerLocation = T66GameplayLayout::GetStartAreaCenter(120.f);
		if (ControlledPawn)
		{
			ControlledPawn->SetActorEnableCollision(false);
			ControlledPawn->SetActorLocation(PlayerLocation, false);
			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->DisableMovement();
				}
			}
		}
		const TWeakObjectPtr<APawn> ProofTargetPawn(ControlledPawn);
		const auto PinProofTarget = [ProofTargetPawn, PlayerLocation]()
		{
			if (APawn* Pawn = ProofTargetPawn.Get())
			{
				Pawn->SetActorEnableCollision(false);
				Pawn->SetActorLocation(PlayerLocation, false);
				if (ACharacter* CharacterPawn = Cast<ACharacter>(Pawn))
				{
					if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
					{
						Movement->StopMovementImmediately();
						Movement->DisableMovement();
					}
				}
			}
		};
		PinProofTarget();
		FTimerHandle PinProofTargetHandle;
		World->GetTimerManager().SetTimer(
			PinProofTargetHandle,
			FTimerDelegate::CreateLambda(PinProofTarget),
			0.02f,
			true);

		FBossData BossData;
		if (!T66GI->GetBossData(FName(TEXT("Dungeon_WebMatriarch")), BossData))
		{
			BossData.BossID = FName(TEXT("Dungeon_WebMatriarch"));
			BossData.MaxHP = 1500;
			BossData.AwakenDistance = 2400.f;
			BossData.ProjectileSpeed = 930.f;
			BossData.ProjectileDamageHearts = 1;
			BossData.BossPartProfile = ET66BossPartProfile::Sharpshooter;
			BossData.PlaceholderColor = FLinearColor(0.50f, 0.42f, 0.68f, 1.f);
		}
		BossData.MoveSpeed = 520.f;
		BossData.FireIntervalSeconds = 999.f;
		BossData.AwakenDistance = 2400.f;
		BossData.BossMovementProfileID = FName(TEXT("MoveProfile_WebMatriarch_KeepDistance"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(TEXT("T66BossMovementB1_WebMatriarch"));
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
			AT66BossBase::StaticClass(),
			PlayerLocation + FVector(980.f, 0.f, 0.f),
			FRotator(0.f, 180.f, 0.f),
			SpawnParams);
		if (!Boss)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossMovementB1] Failed to spawn proof boss."));
			FPlatformMisc::RequestExitWithStatus(false, 92, TEXT("BossMovementB1SpawnFailed"));
			return;
		}

		Boss->Tags.AddUnique(FName(TEXT("T66Automation_BossMovementB1")));
		Boss->InitializeBoss(BossData);
		Boss->ForceAwaken();
		FT66VisualUtil::SnapToGround(Boss, World);
		Boss->ResetBossMovementAutomationState();

		const float InitialDistance = FVector::Dist2D(Boss->GetActorLocation(), PlayerLocation);
		FTimerHandle PatternCheckHandle;
		World->GetTimerManager().SetTimer(
			PatternCheckHandle,
			FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, InitialDistance]()
			{
				const float PatternDistance = FVector::Dist2D(Boss->GetActorLocation(), PlayerLocation);
				const FString PatternMode = Boss->GetBossMovementAutomationMode().ToString();
				const bool bPatternAdvanced = PatternMode.Contains(TEXT("Pattern.KeepDistance.Advance")) && PatternDistance < InitialDistance - 15.f;
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[BossMovementB1] PatternSample Mode=%s InitialDistance=%.1f PatternDistance=%.1f"),
					*PatternMode,
					InitialDistance,
					PatternDistance);

				Boss->ResetBossMovementAutomationState();
				Boss->ApplyMoveSlow(0.35f, 0.65f);

				FTimerHandle SlowCheckHandle;
				World->GetTimerManager().SetTimer(
					SlowCheckHandle,
					FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced]()
					{
						const float SlowSpeed = Boss->GetCharacterMovement() ? Boss->GetCharacterMovement()->MaxWalkSpeed : 0.f;
						const bool bSlowApplied = SlowSpeed > 0.f && SlowSpeed < 260.f;

						Boss->ResetBossMovementAutomationState();
						const FVector FreezeStart = Boss->GetActorLocation();
						Boss->ApplyFreeze(0.45f);

						FTimerHandle FreezeCheckHandle;
						World->GetTimerManager().SetTimer(
							FreezeCheckHandle,
							FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, FreezeStart]()
							{
								const bool bFreezeOverride =
									Boss->GetBossMovementAutomationMode() == FName(TEXT("FrozenOrStunned"))
									&& FVector::Dist2D(Boss->GetActorLocation(), FreezeStart) < 8.f;

								FTimerHandle RootStartHandle;
								World->GetTimerManager().SetTimer(
									RootStartHandle,
									FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, bFreezeOverride]()
									{
										Boss->ResetBossMovementAutomationState();
										const FVector RootStart = Boss->GetActorLocation();
										Boss->ApplyRoot(0.45f);

										FTimerHandle RootCheckHandle;
										World->GetTimerManager().SetTimer(
											RootCheckHandle,
											FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, bFreezeOverride, RootStart]()
											{
												const bool bRootOverride =
													Boss->GetBossMovementAutomationMode() == FName(TEXT("Rooted"))
													&& FVector::Dist2D(Boss->GetActorLocation(), RootStart) < 8.f;

												FTimerHandle StunStartHandle;
												World->GetTimerManager().SetTimer(
													StunStartHandle,
													FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, bFreezeOverride, bRootOverride]()
													{
														Boss->ResetBossMovementAutomationState();
														const FVector StunStart = Boss->GetActorLocation();
														Boss->ApplyStun(0.35f);

														FTimerHandle StunCheckHandle;
														World->GetTimerManager().SetTimer(
															StunCheckHandle,
															FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, bFreezeOverride, bRootOverride, StunStart]()
															{
																const bool bStunOverride =
																	Boss->GetBossMovementAutomationMode() == FName(TEXT("FrozenOrStunned"))
																	&& FVector::Dist2D(Boss->GetActorLocation(), StunStart) < 8.f;

																FTimerHandle ConfusionStartHandle;
																World->GetTimerManager().SetTimer(
																	ConfusionStartHandle,
																	FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, bFreezeOverride, bRootOverride, bStunOverride]()
																	{
																		Boss->ResetBossMovementAutomationState();
																		Boss->ApplyConfusion(0.55f);

																		FTimerHandle ConfusionCheckHandle;
																		World->GetTimerManager().SetTimer(
																			ConfusionCheckHandle,
																			FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, bFreezeOverride, bRootOverride, bStunOverride]()
																			{
																				const bool bConfusionOverride = Boss->GetBossMovementAutomationMode() == FName(TEXT("Confusion"));

																				FTimerHandle RunAwayStartHandle;
																				World->GetTimerManager().SetTimer(
																					RunAwayStartHandle,
																					FTimerDelegate::CreateLambda([World, Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, bFreezeOverride, bRootOverride, bStunOverride, bConfusionOverride]()
																					{
																						Boss->ResetBossMovementAutomationState();
																						if (UCharacterMovementComponent* BossMovement = Boss->GetCharacterMovement())
																						{
																							BossMovement->StopMovementImmediately();
																						}
																						const float RunAwayStartDistance = FVector::Dist2D(Boss->GetActorLocation(), PlayerLocation);
																						Boss->ApplyForcedRunAway(1.5f);

																						FTimerHandle RunAwayCheckHandle;
																						World->GetTimerManager().SetTimer(
																							RunAwayCheckHandle,
																							FTimerDelegate::CreateLambda([Boss, PlayerLocation, PatternDistance, bPatternAdvanced, bSlowApplied, bFreezeOverride, bRootOverride, bStunOverride, bConfusionOverride, RunAwayStartDistance]()
																							{
																								const float RunAwayEndDistance = FVector::Dist2D(Boss->GetActorLocation(), PlayerLocation);
																								const bool bRunAwayOverride =
																									Boss->GetBossMovementAutomationMode() == FName(TEXT("ForcedRunAway"))
																									&& RunAwayEndDistance > RunAwayStartDistance + 5.f;
																								const int32 FailureCount =
																									(bPatternAdvanced ? 0 : 1)
																									+ (bSlowApplied ? 0 : 1)
																									+ (bFreezeOverride ? 0 : 1)
																									+ (bRootOverride ? 0 : 1)
																									+ (bStunOverride ? 0 : 1)
																									+ (bConfusionOverride ? 0 : 1)
																									+ (bRunAwayOverride ? 0 : 1);

																								UE_LOG(
																									LogTemp,
																									Display,
																									TEXT("[BossMovementB1] Gate PatternAdvanced=%d SlowApplied=%d FreezeOverride=%d RootOverride=%d StunOverride=%d ConfusionOverride=%d RunAwayOverride=%d PatternDistance=%.1f RunAwayStart=%.1f RunAwayEnd=%.1f"),
																									bPatternAdvanced ? 1 : 0,
																									bSlowApplied ? 1 : 0,
																									bFreezeOverride ? 1 : 0,
																									bRootOverride ? 1 : 0,
																									bStunOverride ? 1 : 0,
																									bConfusionOverride ? 1 : 0,
																									bRunAwayOverride ? 1 : 0,
																									PatternDistance,
																									RunAwayStartDistance,
																									RunAwayEndDistance);
																								UE_LOG(
																									LogTemp,
																									Display,
																									TEXT("[BossMovementB1] Summary Pass=%d Failures=%d BossID=Dungeon_WebMatriarch MovementProfileID=MoveProfile_WebMatriarch_KeepDistance"),
																									FailureCount == 0 ? 1 : 0,
																									FailureCount);
																								FPlatformMisc::RequestExitWithStatus(false, FailureCount == 0 ? 0 : 93, TEXT("BossMovementB1Complete"));
																							}),
																							0.35f,
																							false);
																					}),
																					0.70f,
																					false);
																			}),
																			0.25f,
																			false);
																	}),
																	0.65f,
																	false);
															}),
															0.18f,
															false);
													}),
													0.55f,
													false);
											}),
											0.20f,
											false);
									}),
									0.65f,
									false);
							}),
							0.20f,
							false);
					}),
					0.18f,
					false);
			}),
			0.85f,
			false);
		return;
	}

	if (Mode == TEXT("bossmovementb2"))
	{
		UWorld* World = GetWorld();
		UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
		if (!World || !T66GI)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossMovementB2] Missing world or T66GameInstance."));
			FPlatformMisc::RequestExitWithStatus(false, 94, TEXT("BossMovementB2MissingWorld"));
			return;
		}

		APawn* ControlledPawn = GetPawn();
		const FVector PlayerLocation = T66GameplayLayout::GetStartAreaCenter(120.f);
		const TWeakObjectPtr<APawn> ProofTargetPawn(ControlledPawn);
		const auto PinProofTarget = [ProofTargetPawn, PlayerLocation]()
		{
			if (APawn* Pawn = ProofTargetPawn.Get())
			{
				Pawn->SetActorEnableCollision(false);
				Pawn->SetActorLocation(PlayerLocation, false);
				if (ACharacter* CharacterPawn = Cast<ACharacter>(Pawn))
				{
					if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
					{
						Movement->StopMovementImmediately();
						Movement->DisableMovement();
					}
				}
			}
		};
		PinProofTarget();
		FTimerHandle PinProofTargetHandle;
		World->GetTimerManager().SetTimer(
			PinProofTargetHandle,
			FTimerDelegate::CreateLambda(PinProofTarget),
			0.02f,
			true);

		FBossData BossData;
		if (!T66GI->GetBossData(FName(TEXT("Dungeon_WebMatriarch")), BossData))
		{
			BossData.BossID = FName(TEXT("Dungeon_WebMatriarch"));
			BossData.MaxHP = 1500;
			BossData.AwakenDistance = 2400.f;
			BossData.ProjectileSpeed = 930.f;
			BossData.ProjectileDamageHearts = 1;
			BossData.BossPartProfile = ET66BossPartProfile::Sharpshooter;
			BossData.PlaceholderColor = FLinearColor(0.50f, 0.42f, 0.68f, 1.f);
		}
		BossData.MoveSpeed = 520.f;
		BossData.FireIntervalSeconds = 999.f;
		BossData.AwakenDistance = 2400.f;
		BossData.BossMovementProfileID = FName(TEXT("MoveProfile_WebMatriarch_RetreatCast"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(TEXT("T66BossMovementB2_WebMatriarch"));
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
			AT66BossBase::StaticClass(),
			PlayerLocation + FVector(640.f, 0.f, 0.f),
			FRotator(0.f, 180.f, 0.f),
			SpawnParams);
		if (!Boss)
		{
			UE_LOG(LogTemp, Error, TEXT("[BossMovementB2] Failed to spawn proof boss."));
			FPlatformMisc::RequestExitWithStatus(false, 95, TEXT("BossMovementB2SpawnFailed"));
			return;
		}

		Boss->Tags.AddUnique(FName(TEXT("T66Automation_BossMovementB2")));
		Boss->InitializeBoss(BossData);
		Boss->ForceAwaken();
		FT66VisualUtil::SnapToGround(Boss, World);
		Boss->ResetBossAttackOwnershipAutomationCounters();
		Boss->ResetBossMovementAutomationState();

		const float InitialDistance = FVector::Dist2D(Boss->GetActorLocation(), PlayerLocation);
		Boss->ForceBossAttackForAutomation(FName(TEXT("LegacyProjectile_Sharpshooter")), FName(TEXT("Head")));

		FTimerHandle CoordinationCheckHandle;
		World->GetTimerManager().SetTimer(
			CoordinationCheckHandle,
			FTimerDelegate::CreateLambda([Boss, PlayerLocation, InitialDistance]()
			{
				const float CoordinatedDistance = FVector::Dist2D(Boss->GetActorLocation(), PlayerLocation);
				const FString MovementMode = Boss->GetBossMovementAutomationMode().ToString();
				const int32 QueuedCount = Boss->GetBossAttackOwnershipAutomationCounter(
					FName(TEXT("Queued")),
					FName(TEXT("LegacyProjectile_Sharpshooter")),
					FName(TEXT("Head")));
				const int32 FiredCount = Boss->GetBossAttackOwnershipAutomationCounter(
					FName(TEXT("Fired")),
					FName(TEXT("LegacyProjectile_Sharpshooter")),
					FName(TEXT("Head")));
				const bool bPartOwnedAttackQueued = QueuedCount > 0;
				const bool bPartOwnedAttackFired = FiredCount > 0;
				const bool bCoordinatedRetreatActive = MovementMode.Contains(TEXT("Pattern.RetreatThenCast.Active"));
				const bool bRetreatDistanceIncreased = CoordinatedDistance > InitialDistance + 5.f;
				const int32 FailureCount =
					(bPartOwnedAttackQueued ? 0 : 1)
					+ (bPartOwnedAttackFired ? 0 : 1)
					+ (bCoordinatedRetreatActive ? 0 : 1)
					+ (bRetreatDistanceIncreased ? 0 : 1);

				UE_LOG(
					LogTemp,
					Display,
					TEXT("[BossMovementB2] Gate PartOwnedAttackQueued=%d PartOwnedAttackFired=%d CoordinatedRetreatActive=%d RetreatDistanceIncreased=%d Mode=%s InitialDistance=%.1f CoordinatedDistance=%.1f"),
					bPartOwnedAttackQueued ? 1 : 0,
					bPartOwnedAttackFired ? 1 : 0,
					bCoordinatedRetreatActive ? 1 : 0,
					bRetreatDistanceIncreased ? 1 : 0,
					*MovementMode,
					InitialDistance,
					CoordinatedDistance);
				UE_LOG(
					LogTemp,
					Display,
					TEXT("[BossMovementB2] Summary Pass=%d Failures=%d BossID=Dungeon_WebMatriarch MovementProfileID=MoveProfile_WebMatriarch_RetreatCast AttackID=LegacyProjectile_Sharpshooter OwningPartID=Head"),
					FailureCount == 0 ? 1 : 0,
					FailureCount);
				FPlatformMisc::RequestExitWithStatus(false, FailureCount == 0 ? 0 : 96, TEXT("BossMovementB2Complete"));
			}),
			0.45f,
			false);
		return;
	}
#endif

	if (Mode == TEXT("slimekingbossqa") || Mode == TEXT("slimekingtelegraphqa") || Mode == TEXT("bosstelegraphqa"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		APawn* ControlledPawn = GetPawn();
		const FVector PlayerReviewLocation = ControlledPawn ? ControlledPawn->GetActorLocation() : T66GameplayLayout::GetStartAreaCenter(120.f);
		const FVector BossReviewLocation = PlayerReviewLocation + FVector(760.f, 0.f, 0.f);
		if (ControlledPawn)
		{
			ControlledPawn->SetActorEnableCollision(false);
			ControlledPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f));
			SetControlRotation(FRotator(-8.f, 0.f, 0.f));

			if (ACharacter* CharacterPawn = Cast<ACharacter>(ControlledPawn))
			{
				if (UCharacterMovementComponent* Movement = CharacterPawn->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
				}
			}
			if (AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn))
			{
				if (HeroPawn->CameraBoom)
				{
					HeroPawn->CameraBoom->TargetArmLength = 720.f;
					HeroPawn->CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 125.f));
					HeroPawn->CameraBoom->bDoCollisionTest = false;
				}
			}
		}

		if (UWorld* World = GetWorld())
		{
			UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
			FBossData BossData;
			if (!T66GI || !T66GI->GetBossData(FName(TEXT("Dungeon_SewerSlimeKing")), BossData))
			{
				BossData.BossID = FName(TEXT("Dungeon_SewerSlimeKing"));
				BossData.MaxHP = 1250;
				BossData.AwakenDistance = 1200.f;
				BossData.MoveSpeed = 0.f;
				BossData.FireIntervalSeconds = 4.0f;
				BossData.ProjectileSpeed = 760.f;
				BossData.ProjectileDamageHearts = 1;
				BossData.BossPartProfile = ET66BossPartProfile::Juggernaut;
				BossData.PlaceholderColor = FLinearColor(0.20f, 0.92f, 0.08f, 1.f);
			}

			BossData.MoveSpeed = 0.f;
			BossData.FireIntervalSeconds = 999.f;

			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66SlimeKingBossTelegraphQA"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(
				AT66BossBase::StaticClass(),
				BossReviewLocation,
				FRotator(0.f, 180.f, 0.f),
				SpawnParams);
			if (Boss)
			{
				Boss->Tags.AddUnique(FName(TEXT("T66Automation_SlimeKingBossQA")));
				Boss->InitializeBoss(BossData);
				Boss->SetActorRotation(FRotator(0.f, 180.f, 0.f));
				if (UCharacterMovementComponent* BossMovement = Boss->GetCharacterMovement())
				{
					BossMovement->StopMovementImmediately();
					BossMovement->DisableMovement();
				}
				FT66VisualUtil::SnapToGround(Boss, World);

				static const FName AttackSequence[] =
				{
					FName(TEXT("LeftLobe")),
					FName(TEXT("RightLobe")),
					FName(TEXT("LeftBase")),
					FName(TEXT("RightBase")),
					FName(TEXT("MouthCore"))
				};

				for (int32 Index = 0; Index < UE_ARRAY_COUNT(AttackSequence); ++Index)
				{
					TWeakObjectPtr<AT66BossBase> WeakBoss(Boss);
					const FName AttackPartID = AttackSequence[Index];
					FTimerHandle TimerHandle;
					World->GetTimerManager().SetTimer(
						TimerHandle,
						FTimerDelegate::CreateLambda([WeakBoss, AttackPartID]()
						{
							if (WeakBoss.IsValid())
							{
								WeakBoss->ForceSewerSlimeKingAttackForAutomation(AttackPartID);
							}
						}),
						0.6f + static_cast<float>(Index) * 2.45f,
						false);
				}
			}
		}
		return;
	}

	if (Mode == TEXT("heromovementqa") || Mode == TEXT("herolocomotionqa") || Mode == TEXT("companionmovementqa"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->SetVisibility(ESlateVisibility::Hidden);
			GameplayHUDWidget->RefreshHUD();
		}

		UWorld* World = GetWorld();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(GetPawn());
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[HeroMovementQA] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bMobFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[HeroMovementQA] Failed: no mob floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector StartLocation = TargetFloor->ArrivalPoint;
		if (StartLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			StartLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (StartLocation.IsNearlyZero())
		{
			StartLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		StartLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;

		const FVector MoveDirection = FVector(1.0f, 0.0f, 0.0f);
		const FVector RightDirection = FVector(0.0f, 1.0f, 0.0f);
		float CameraDistance = 520.0f;
		float CameraSideOffset = 520.0f;
		float CameraHeight = 240.0f;
		float MoveDurationSeconds = 6.8f;
		float MoveStartDelaySeconds = 0.25f;
		float JumpTimeSeconds = 2.45f;
		float RollTimeSeconds = 4.95f;
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQACameraDistance="), CameraDistance);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQACameraSideOffset="), CameraSideOffset);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQACameraHeight="), CameraHeight);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQAMoveDuration="), MoveDurationSeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQAStartDelay="), MoveStartDelaySeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQAJumpTime="), JumpTimeSeconds);
		FParse::Value(FCommandLine::Get(), TEXT("T66HeroMovementQARollTime="), RollTimeSeconds);
		CameraDistance = FMath::Clamp(CameraDistance, 250.0f, 2200.0f);
		CameraSideOffset = FMath::Clamp(CameraSideOffset, -1800.0f, 1800.0f);
		CameraHeight = FMath::Clamp(CameraHeight, 120.0f, 900.0f);
		MoveDurationSeconds = FMath::Clamp(MoveDurationSeconds, 1.0f, 12.0f);
		MoveStartDelaySeconds = FMath::Clamp(MoveStartDelaySeconds, 0.0f, 3.0f);
		JumpTimeSeconds = FMath::Clamp(JumpTimeSeconds, 0.0f, MoveDurationSeconds);
		RollTimeSeconds = FMath::Clamp(RollTimeSeconds, 0.0f, MoveDurationSeconds);

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(MOVE_Walking);
		}
		HeroPawn->SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));

		GameMode->SetEnemyDirectorSpawningPaused(true);
		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(true);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(false);
		}

		for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AT66MobBase> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AT66BossBase> It(World); It; ++It)
		{
			It->Destroy();
		}

		AT66CompanionBase* QACompanion = nullptr;
		const FName CompanionID = T66GetHeroMovementQACompanionID();
		if (!CompanionID.IsNone())
		{
			if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
			{
				FCompanionData CompanionData;
				if (T66GI->GetCompanionData(CompanionID, CompanionData))
				{
					FActorSpawnParameters CompanionSpawnParams;
					CompanionSpawnParams.Owner = HeroPawn;
					CompanionSpawnParams.Instigator = HeroPawn;
					CompanionSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					QACompanion = World->SpawnActor<AT66CompanionBase>(
						AT66CompanionBase::StaticClass(),
						StartLocation + FVector(-155.0f, 115.0f, 0.0f),
						FRotator::ZeroRotator,
						CompanionSpawnParams);
					if (QACompanion)
					{
						QACompanion->Tags.AddUnique(FName(TEXT("T66Automation_HeroMovementQACompanion")));
						QACompanion->InitializeCompanion(CompanionData, FName(TEXT("Default")));
						QACompanion->SetPreviewMode(false);
						FT66VisualUtil::SnapToGround(QACompanion, World);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[HeroMovementQA] Companion data missing for %s."), *CompanionID.ToString());
				}
			}
		}

		const FVector CameraLocation = HeroPawn->GetActorLocation() - MoveDirection * CameraDistance + RightDirection * CameraSideOffset + FVector(0.0f, 0.0f, CameraHeight);
		const FVector CameraLookTarget = HeroPawn->GetActorLocation() + FVector(0.0f, 0.0f, 95.0f);
		FRotator CameraRotation = (CameraLookTarget - CameraLocation).Rotation();
		FActorSpawnParameters CameraSpawnParams;
		CameraSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACameraActor* PreviewCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraLocation, CameraRotation, CameraSpawnParams);
		if (PreviewCamera)
		{
			SetViewTarget(PreviewCamera);
		}
		else
		{
			SetControlRotation(CameraRotation);
		}

		const FName HeroVisualID = T66GetHeroMovementQAVisualID();
		if (UT66CharacterVisualSubsystem* Visuals = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66CharacterVisualSubsystem>() : nullptr)
		{
			UAnimationAsset* HeroWalk = nullptr;
			UAnimationAsset* HeroJump = nullptr;
			UAnimationAsset* HeroIdle = nullptr;
			UAnimationAsset* HeroRoll = nullptr;
			Visuals->GetMovementAnimsForVisual(HeroVisualID, HeroWalk, HeroJump, HeroIdle, HeroRoll);
			UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] HeroVisualID=%s %s"), *HeroVisualID.ToString(), *T66DescribeMovementQAAnimation(TEXT("HeroWalk"), HeroWalk));

			if (!CompanionID.IsNone())
			{
				const FName CompanionVisualID = UT66CharacterVisualSubsystem::GetCompanionVisualID(CompanionID, FName(TEXT("Default")));
				UAnimationAsset* CompanionWalk = nullptr;
				UAnimationAsset* CompanionJump = nullptr;
				UAnimationAsset* CompanionIdle = nullptr;
				UAnimationAsset* CompanionRoll = nullptr;
				Visuals->GetMovementAnimsForVisual(CompanionVisualID, CompanionWalk, CompanionJump, CompanionIdle, CompanionRoll);
				UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] CompanionVisualID=%s %s"), *CompanionVisualID.ToString(), *T66DescribeMovementQAAnimation(TEXT("CompanionWalk"), CompanionWalk));
			}
		}

		TWeakObjectPtr<AT66HeroBase> WeakHero(HeroPawn);
		TWeakObjectPtr<AT66CompanionBase> WeakCompanion(QACompanion);
		TWeakObjectPtr<ACameraActor> WeakPreviewCamera(PreviewCamera);
		const double QAStartWorldTime = World->GetTimeSeconds() + MoveStartDelaySeconds;

		TSharedPtr<FTimerHandle> MoveTimerHandle = MakeShared<FTimerHandle>();
		World->GetTimerManager().SetTimer(
			*MoveTimerHandle,
			FTimerDelegate::CreateLambda([World, WeakHero, MoveTimerHandle, MoveDirection, QAStartWorldTime, MoveDurationSeconds]()
			{
				AT66HeroBase* Hero = WeakHero.Get();
				if (!World || !Hero)
				{
					if (World && MoveTimerHandle.IsValid())
					{
						World->GetTimerManager().ClearTimer(*MoveTimerHandle);
					}
					return;
				}

				const double Elapsed = World->GetTimeSeconds() - QAStartWorldTime;
				if (Elapsed > MoveDurationSeconds)
				{
					if (MoveTimerHandle.IsValid())
					{
						World->GetTimerManager().ClearTimer(*MoveTimerHandle);
					}
					return;
				}

				if (Elapsed >= 0.0)
				{
					Hero->AddMovementInput(MoveDirection, 1.0f, false);
				}
			}),
			0.016f,
			true,
			MoveStartDelaySeconds);

		FTimerHandle JumpTimerHandle;
		World->GetTimerManager().SetTimer(
			JumpTimerHandle,
			FTimerDelegate::CreateLambda([WeakHero]()
			{
				if (AT66HeroBase* Hero = WeakHero.Get())
				{
					Hero->Jump();
				}
			}),
			MoveStartDelaySeconds + JumpTimeSeconds,
			false);

		FTimerHandle RollTimerHandle;
		World->GetTimerManager().SetTimer(
			RollTimerHandle,
			FTimerDelegate::CreateLambda([WeakHero]()
			{
				if (AT66HeroBase* Hero = WeakHero.Get())
				{
					Hero->RollForward();
				}
			}),
			MoveStartDelaySeconds + RollTimeSeconds,
			false);

		if (PreviewCamera)
		{
			FTimerHandle CameraFollowHandle;
			World->GetTimerManager().SetTimer(
				CameraFollowHandle,
				FTimerDelegate::CreateLambda([WeakHero, WeakPreviewCamera, MoveDirection, RightDirection, CameraDistance, CameraSideOffset, CameraHeight]()
				{
					AT66HeroBase* Hero = WeakHero.Get();
					ACameraActor* Camera = WeakPreviewCamera.Get();
					if (!Hero || !Camera)
					{
						return;
					}
					const FVector HeroLocation = Hero->GetActorLocation();
					const FVector FollowCameraLocation = HeroLocation - MoveDirection * CameraDistance + RightDirection * CameraSideOffset + FVector(0.0f, 0.0f, CameraHeight);
					const FVector FollowLookTarget = HeroLocation + FVector(0.0f, 0.0f, 95.0f);
					Camera->SetActorLocation(FollowCameraLocation);
					Camera->SetActorRotation((FollowLookTarget - FollowCameraLocation).Rotation());
				}),
				0.033f,
				true);
		}

		const TSharedRef<FVector> LastHeroLogLocation = MakeShared<FVector>(HeroPawn->GetActorLocation());
		const TSharedRef<FVector> LastCompanionLogLocation = MakeShared<FVector>(QACompanion ? QACompanion->GetActorLocation() : FVector::ZeroVector);
		const TSharedRef<double> LastLogWorldTime = MakeShared<double>(World->GetTimeSeconds());
		for (int32 SampleIndex = 0; SampleIndex <= 14; ++SampleIndex)
		{
			FTimerHandle LogTimerHandle;
			const float SampleTime = MoveStartDelaySeconds + static_cast<float>(SampleIndex) * 0.5f;
			World->GetTimerManager().SetTimer(
				LogTimerHandle,
				FTimerDelegate::CreateLambda([WeakHero, WeakCompanion, LastHeroLogLocation, LastCompanionLogLocation, LastLogWorldTime, SampleTime]()
				{
					AT66HeroBase* Hero = WeakHero.Get();
					if (!Hero)
					{
						UE_LOG(LogTemp, Warning, TEXT("[HeroMovementQA] t=%.2fs hero missing."), SampleTime);
						return;
					}

					UCharacterMovementComponent* Movement = Hero->GetCharacterMovement();
					USkeletalMeshComponent* HeroMesh = Hero->GetMesh();
					const double Now = Hero->GetWorld() ? Hero->GetWorld()->GetTimeSeconds() : *LastLogWorldTime;
					const float DeltaSeconds = FMath::Max(0.001f, static_cast<float>(Now - *LastLogWorldTime));
					const FVector HeroLocation = Hero->GetActorLocation();
					const float HeroDeltaSpeed = FVector::Dist2D(HeroLocation, *LastHeroLogLocation) / DeltaSeconds;
					const float HeroVelocitySpeed = Hero->GetVelocity().Size2D();
					const float HeroMaxWalkSpeed = Movement ? Movement->MaxWalkSpeed : -1.0f;
					const FRotator HeroMeshRotation = HeroMesh ? HeroMesh->GetRelativeRotation() : FRotator::ZeroRotator;
					const float HeroPlayRate = HeroMesh ? HeroMesh->GetPlayRate() : -1.0f;

					float CompanionDeltaSpeed = -1.0f;
					FVector CompanionLocation = FVector::ZeroVector;
					FRotator CompanionMeshRotation = FRotator::ZeroRotator;
					float CompanionPlayRate = -1.0f;
					if (AT66CompanionBase* Companion = WeakCompanion.Get())
					{
						CompanionLocation = Companion->GetActorLocation();
						CompanionDeltaSpeed = FVector::Dist2D(CompanionLocation, *LastCompanionLogLocation) / DeltaSeconds;
						if (USkeletalMeshComponent* CompanionMesh = Companion->SkeletalMesh)
						{
							CompanionMeshRotation = CompanionMesh->GetRelativeRotation();
							CompanionPlayRate = CompanionMesh->GetPlayRate();
						}
						*LastCompanionLogLocation = CompanionLocation;
					}

					UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] t=%.2fs heroLoc=%s heroVelocity=%s heroVelocitySpeed=%.1f heroDeltaSpeed=%.1f maxWalkSpeed=%.1f actorYaw=%.1f meshRelYaw=%.1f meshPlayRate=%.3f companionLoc=%s companionDeltaSpeed=%.1f companionMeshRelYaw=%.1f companionPlayRate=%.3f"),
						SampleTime,
						*HeroLocation.ToCompactString(),
						*Hero->GetVelocity().ToCompactString(),
						HeroVelocitySpeed,
						HeroDeltaSpeed,
						HeroMaxWalkSpeed,
						Hero->GetActorRotation().Yaw,
						HeroMeshRotation.Yaw,
						HeroPlayRate,
						*CompanionLocation.ToCompactString(),
						CompanionDeltaSpeed,
						CompanionMeshRotation.Yaw,
						CompanionPlayRate);

					*LastHeroLogLocation = HeroLocation;
					*LastLogWorldTime = Now;
				}),
				SampleTime,
				false);
		}

		UE_LOG(LogTemp, Display, TEXT("[HeroMovementQA] Armed hero=%s visual=%s companion=%s floor=%d start=%s cameraDistance=%.1f cameraSideOffset=%.1f cameraHeight=%.1f moveDuration=%.2f jumpAt=%.2f rollAt=%.2f."),
			*HeroPawn->HeroID.ToString(),
			*T66GetHeroMovementQAVisualID().ToString(),
			*CompanionID.ToString(),
			TargetFloor->FloorNumber,
			*HeroPawn->GetActorLocation().ToCompactString(),
			CameraDistance,
			CameraSideOffset,
			CameraHeight,
			MoveDurationSeconds,
			JumpTimeSeconds,
			RollTimeSeconds);
		return;
	}

	if (Mode == TEXT("enemyanimpreview") || Mode == TEXT("enemyanimationpreview") || Mode == TEXT("mobanimpreview"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}
		if (UT66MediaViewerSubsystem* MediaViewer = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MediaViewerSubsystem>() : nullptr)
		{
			MediaViewer->SetMediaViewerOpen(false);
		}
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		FString EnemyIDString(TEXT("BoneWalker"));
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewEnemyID="), EnemyIDString);
		EnemyIDString = EnemyIDString.TrimStartAndEnd();
		if (EnemyIDString.IsEmpty())
		{
			EnemyIDString = TEXT("BoneWalker");
		}

		float StartDistance = 1650.0f;
		float StopDistance = 140.0f;
		float CameraDistance = 1120.0f;
		float CameraSideOffset = 420.0f;
		float CameraHeight = 255.0f;
		float TargetForwardOffset = 0.0f;
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewStartDistance="), StartDistance);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewStopDistance="), StopDistance);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewCameraDistance="), CameraDistance);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewCameraSideOffset="), CameraSideOffset);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewCameraHeight="), CameraHeight);
		FParse::Value(FCommandLine::Get(), TEXT("T66EnemyAnimPreviewTargetForwardOffset="), TargetForwardOffset);
		StartDistance = FMath::Clamp(StartDistance, 350.0f, 8000.0f);
		StopDistance = FMath::Clamp(StopDistance, 70.0f, 450.0f);
		CameraDistance = FMath::Clamp(CameraDistance, 350.0f, 2500.0f);
		CameraSideOffset = FMath::Clamp(CameraSideOffset, -1600.0f, 1600.0f);
		CameraHeight = FMath::Clamp(CameraHeight, 80.0f, 900.0f);
		TargetForwardOffset = FMath::Clamp(TargetForwardOffset, -4000.0f, 4000.0f);

		UWorld* World = GetWorld();
		APawn* ControlledPawn = GetPawn();
		AT66HeroBase* HeroPawn = Cast<AT66HeroBase>(ControlledPawn);
		AT66GameMode* GameMode = World ? Cast<AT66GameMode>(World->GetAuthGameMode()) : nullptr;
		T66TowerMapTerrain::FLayout TowerLayout;
		if (!World || !ControlledPawn || !HeroPawn || !GameMode || !GameMode->IsUsingTowerMainMapLayout() || !GameMode->GetTowerMainMapLayout(TowerLayout))
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyAnimPreview] Failed: missing world, hero pawn, game mode, or tower layout."));
			return;
		}

		const T66TowerMapTerrain::FFloor* TargetFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& Floor : TowerLayout.Floors)
		{
			if (Floor.bMobFloor && Floor.FloorNumber == TowerLayout.FirstMobFloorNumber)
			{
				TargetFloor = &Floor;
				break;
			}
			if (!TargetFloor && Floor.bMobFloor)
			{
				TargetFloor = &Floor;
			}
		}
		if (!TargetFloor)
		{
			UE_LOG(LogTemp, Warning, TEXT("[EnemyAnimPreview] Failed: no mob floor in tower layout."));
			return;
		}

		float PawnHalfHeight = 100.0f;
		if (const UCapsuleComponent* Capsule = HeroPawn->GetCapsuleComponent())
		{
			PawnHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}

		FVector TargetLocation = TargetFloor->ArrivalPoint;
		if (TargetLocation.IsNearlyZero() && TargetFloor->CachedWalkableSpawnSlots.Num() > 0)
		{
			TargetLocation = TargetFloor->CachedWalkableSpawnSlots[0];
		}
		if (TargetLocation.IsNearlyZero())
		{
			TargetLocation = FVector(TargetFloor->Center.X, TargetFloor->Center.Y, TargetFloor->SurfaceZ);
		}
		TargetLocation.Z = TargetFloor->SurfaceZ + PawnHalfHeight + 24.0f;
		TargetLocation.X += TargetForwardOffset;

		if (UCharacterMovementComponent* Movement = HeroPawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
		}
		HeroPawn->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		HeroPawn->SetActorRotation(FRotator(0.0f, 0.0f, 0.0f), ETeleportType::TeleportPhysics);
		HeroPawn->SetActorEnableCollision(false);
		HeroPawn->SetActorHiddenInGame(true);

		GameMode->SetEnemyDirectorSpawningPaused(true);
		GameMode->HandleTowerDescentHoleTriggered(HeroPawn, TowerLayout.StartFloorNumber, TargetFloor->FloorNumber);
		GameMode->SetEnemyDirectorSpawningPaused(true);
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			RunState->SetStageTimerActive(false);
		}

		for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AT66MobBase> It(World); It; ++It)
		{
			It->Destroy();
		}
		for (TActorIterator<AT66BossBase> It(World); It; ++It)
		{
			It->Destroy();
		}

		const FVector MoveDirection = FVector(1.0f, 0.0f, 0.0f);
		const FVector RightDirection = FVector(0.0f, 1.0f, 0.0f);
		FVector PreviewSpawnLocation = TargetLocation - MoveDirection * StartDistance;
		PreviewSpawnLocation.Z = TargetFloor->SurfaceZ + 90.0f;
		FRotator SpawnRotation = (TargetLocation - PreviewSpawnLocation).Rotation();
		SpawnRotation.Pitch = 0.0f;
		SpawnRotation.Roll = 0.0f;

		const FTransform SpawnTransform(SpawnRotation, PreviewSpawnLocation);
		AT66MobBase* PreviewMob = World->SpawnActorDeferred<AT66MobBase>(
			AT66MobBase::StaticClass(),
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (!PreviewMob)
		{
			UE_LOG(LogTemp, Error, TEXT("[EnemyAnimPreview] Failed to spawn AT66MobBase for EnemyID=%s."), *EnemyIDString);
			return;
		}

		const FName EnemyID(*EnemyIDString);
		PreviewMob->Tags.AddUnique(FName(TEXT("T66Automation_EnemyAnimPreview")));
		PreviewMob->MobID = EnemyID;
		PreviewMob->CharacterVisualID = EnemyID;
		PreviewMob->LifecycleState = ET66MobLifecycleState::Active;
		UGameplayStatics::FinishSpawningActor(PreviewMob, SpawnTransform);

		int32 StageNum = 1;
		float DifficultyScalar = 1.0f;
		float EnemyProgressionScalar = 1.0f;
		float FinaleScalar = 1.0f;
		if (UT66RunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
		{
			StageNum = FMath::Max(1, RunState->GetCurrentStage());
			DifficultyScalar = RunState->GetDifficultyScalar();
			FinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
		}
		PreviewMob->ConfigureAsMob(EnemyID, ET66EnemyFamily::Melee, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
		PreviewMob->TouchDamageHearts = 0;
		PreviewMob->MaxHP = 200000.0f;
		PreviewMob->CurrentHP = PreviewMob->MaxHP;
		FT66VisualUtil::SnapToGround(PreviewMob, World);

		const FVector CameraLocation = PreviewMob->GetActorLocation() + MoveDirection * CameraDistance + RightDirection * CameraSideOffset + FVector(0.0f, 0.0f, CameraHeight);
		const FVector CameraLookTarget = PreviewMob->GetActorLocation() + FVector(0.0f, 0.0f, 72.0f);
		FRotator CameraRotation = (CameraLookTarget - CameraLocation).Rotation();
		FActorSpawnParameters CameraSpawnParams;
		CameraSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ACameraActor* PreviewCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CameraLocation, CameraRotation, CameraSpawnParams);
		if (PreviewCamera)
		{
			SetViewTarget(PreviewCamera);
		}
		else
		{
			SetControlRotation(CameraRotation);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetVisibility(ESlateVisibility::Hidden);
		}

		TWeakObjectPtr<AT66MobBase> WeakPreviewMob(PreviewMob);
		FTimerHandle PreviewIsolationCleanupHandle;
		World->GetTimerManager().SetTimer(
			PreviewIsolationCleanupHandle,
			FTimerDelegate::CreateLambda([World, WeakPreviewMob]()
			{
				for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
				{
					It->Destroy();
				}
				for (TActorIterator<AT66BossBase> It(World); It; ++It)
				{
					It->Destroy();
				}
				AT66MobBase* PreviewMobToKeep = WeakPreviewMob.Get();
				for (TActorIterator<AT66MobBase> It(World); It; ++It)
				{
					AT66MobBase* Mob = *It;
					if (Mob && Mob != PreviewMobToKeep)
					{
						Mob->Destroy();
					}
				}
			}),
			0.10f,
			true);

		TWeakObjectPtr<ACameraActor> WeakPreviewCamera(PreviewCamera);
		if (PreviewCamera)
		{
			FTimerHandle PreviewCameraFollowHandle;
			World->GetTimerManager().SetTimer(
				PreviewCameraFollowHandle,
				FTimerDelegate::CreateLambda([WeakPreviewMob, WeakPreviewCamera, MoveDirection, RightDirection, CameraDistance, CameraSideOffset, CameraHeight]()
				{
					AT66MobBase* Mob = WeakPreviewMob.Get();
					ACameraActor* Camera = WeakPreviewCamera.Get();
					if (!Mob || !Camera)
					{
						return;
					}

					const FVector MobLocation = Mob->GetActorLocation();
					const FVector FollowCameraLocation = MobLocation + MoveDirection * CameraDistance + RightDirection * CameraSideOffset + FVector(0.0f, 0.0f, CameraHeight);
					const FVector FollowLookTarget = MobLocation + FVector(0.0f, 0.0f, 72.0f);
					Camera->SetActorLocation(FollowCameraLocation);
					Camera->SetActorRotation((FollowLookTarget - FollowCameraLocation).Rotation());
				}),
				0.033f,
				true);
		}

		const float LogTimes[] = { 0.20f, 0.70f, 1.20f, 1.70f, 2.20f, 2.70f, 3.20f, 3.70f, 4.20f };
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LogTimes); ++Index)
		{
			FTimerHandle PreviewLogHandle;
			const float LogTime = LogTimes[Index];
			World->GetTimerManager().SetTimer(
				PreviewLogHandle,
				FTimerDelegate::CreateLambda([WeakPreviewMob, EnemyID, LogTime, TargetLocation]()
				{
					const AT66MobBase* Mob = WeakPreviewMob.Get();
					if (!Mob)
					{
						UE_LOG(LogTemp, Warning, TEXT("[EnemyAnimPreview] t=%.2fs EnemyID=%s mob missing."), LogTime, *EnemyID.ToString());
						return;
					}

					const float DistanceToTarget = FVector::Dist2D(Mob->GetActorLocation(), TargetLocation);
					UE_LOG(LogTemp, Display, TEXT("[EnemyAnimPreview] t=%.2fs EnemyID=%s loc=%s velocity=%s speed=%.1f distance=%.1f family=%d"),
						LogTime,
						*EnemyID.ToString(),
						*Mob->GetActorLocation().ToCompactString(),
						*Mob->StoredVelocity.ToCompactString(),
						Mob->StoredVelocity.Size2D(),
						DistanceToTarget,
						static_cast<int32>(Mob->GetEnemyFamily()));
				}),
				LogTime,
				false);
		}

		UE_LOG(LogTemp, Display, TEXT("[EnemyAnimPreview] Armed EnemyID=%s mob=%s floor=%d spawn=%s target=%s startDistance=%.1f stopDistance=%.1f camera=%s chaseSpeed=%.1f managerDriven=1."),
			*EnemyID.ToString(),
			*GetNameSafe(PreviewMob),
			TargetFloor->FloorNumber,
			*PreviewMob->GetActorLocation().ToCompactString(),
			*TargetLocation.ToCompactString(),
			StartDistance,
			StopDistance,
			*CameraLocation.ToCompactString(),
			PreviewMob->ChaseSpeed);
		return;
	}

	if (Mode == TEXT("mobvatqa") || Mode == TEXT("easymobvat") || Mode == TEXT("easymobvatqa"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (UWorld* World = GetWorld())
		{
			const TArray<FName> EasyMobIDs = {
				FName(TEXT("Slime")),
				FName(TEXT("BoneWalker")),
				FName(TEXT("RatPack")),
				FName(TEXT("CaveBat")),
				FName(TEXT("HexSlinger")),
				FName(TEXT("TombSpider")),
				FName(TEXT("StoneSentinel")),
				FName(TEXT("MimicLure")),
				FName(TEXT("BoneConjurer")),
				FName(TEXT("CryptWraith"))
			};
			const TArray<FName> ClipNames = {
				FName(TEXT("Idle")),
				FName(TEXT("Move")),
				FName(TEXT("AttackCue")),
				FName(TEXT("HitReact")),
				FName(TEXT("Death"))
			};

			const FVector Origin = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			for (int32 Index = 0; Index < EasyMobIDs.Num(); ++Index)
			{
				const int32 Row = Index / 5;
				const int32 Col = Index % 5;
				const FVector QAEnemyLocation = Origin + FVector(480.f + static_cast<float>(Row) * 300.f, (static_cast<float>(Col) - 4.f) * 220.f, 0.f);
				FRotator QAEnemyRotation = (Origin - QAEnemyLocation).Rotation();
				QAEnemyRotation.Pitch = 0.f;
				QAEnemyRotation.Roll = 0.f;
				SpawnParams.Name = FName(*FString::Printf(TEXT("T66MobVATQA_%s"), *EasyMobIDs[Index].ToString()));

				AT66EnemyBase* Enemy = World->SpawnActor<AT66EnemyBase>(
					AT66EnemyBase::StaticClass(),
					QAEnemyLocation,
					QAEnemyRotation,
					SpawnParams);
				if (!Enemy)
				{
					UE_LOG(LogTemp, Error, TEXT("Mob VAT QA failed to spawn %s"), *EasyMobIDs[Index].ToString());
					continue;
				}

				Enemy->Tags.AddUnique(FName(TEXT("T66Automation_EasyMobVATQA")));
				Enemy->SetActorEnableCollision(false);
				Enemy->MaxHP = 20000;
				Enemy->CurrentHP = 20000;
				Enemy->TouchDamageHearts = 0;
				Enemy->PointValue = 0;
				Enemy->XPValue = 0;
				Enemy->bDropsLoot = false;
				Enemy->OwningDirector = nullptr;
				Enemy->ConfigureAsMob(EasyMobIDs[Index]);
				T66RecordNonDirectorRouteAttribution(World, Enemy);
				if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->DisableMovement();
				}

				const FName ClipName = ClipNames[Index % ClipNames.Num()];
				Enemy->ForceMobVertexAnimationClipForAutomation(ClipName, 30.f);
				UE_LOG(LogTemp, Display, TEXT("Mob VAT QA spawned %s clip=%s location=%s"),
					*EasyMobIDs[Index].ToString(),
					*ClipName.ToString(),
					*QAEnemyLocation.ToCompactString());
			}
		}
		return;
	}
#endif

	if (Mode == TEXT("scopedsniper") || Mode == TEXT("sniperscope") || Mode == TEXT("scopeoverlay"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetFullMapOpen(false);
			GameplayHUDWidget->RefreshHUD();
		}

		if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetPawn()))
		{
			if (UT66CombatComponent* Combat = Hero->FindComponentByClass<UT66CombatComponent>())
			{
				ActivateHeroOneScopedUlt(Hero, Combat);
				SetHeroOneScopeViewEnabled(true);
			}
		}
		return;
	}

	if (Mode == TEXT("cowardiceprompt") || Mode == TEXT("cowardice") || Mode == TEXT("cowardicegate"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		AT66CowardiceGate* AutomationGate = nullptr;
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Name = FName(TEXT("T66WidgetDump_CowardiceGate"));
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AutomationGate = World->SpawnActor<AT66CowardiceGate>(
				AT66CowardiceGate::StaticClass(),
				GetPawn() ? GetPawn()->GetActorLocation() + FVector(360.f, 0.f, 0.f) : FVector(360.f, 0.f, 0.f),
				FRotator::ZeroRotator,
				SpawnParams);
			if (AutomationGate)
			{
				AutomationGate->SetActorHiddenInGame(true);
				AutomationGate->SetActorEnableCollision(false);
			}
		}

		OpenCowardicePrompt(AutomationGate);
		return;
	}

	if (Mode == TEXT("loading") || Mode == TEXT("loadingscreen") || Mode == TEXT("transitionloading"))
	{
		if (UT66LoadingScreenWidget* LoadingOverlay = CreateWidget<UT66LoadingScreenWidget>(this, UT66LoadingScreenWidget::StaticClass()))
		{
			LoadingOverlay->SetLoadingText(NSLOCTEXT("T66.Loading", "WidgetDumpLoadingText", "Loading"));
			LoadingOverlay->AddToViewport(10000);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("idol") || Mode == TEXT("idolaltar"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		AT66IdolAltar* AutomationAltar = nullptr;
		if (UWorld* World = GetWorld())
		{
			const FVector AutomationAltarLocation = GetPawn() ? GetPawn()->GetActorLocation() + FVector(300.f, 0.f, 0.f) : FVector::ZeroVector;
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AutomationAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), AutomationAltarLocation, FRotator::ZeroRotator, SpawnParams);
			if (AutomationAltar)
			{
				AutomationAltar->SetActorHiddenInGame(true);
				AutomationAltar->SetActorEnableCollision(false);
				AutomationAltar->RemainingSelections = FMath::Max(1, AutomationAltar->RemainingSelections);
			}
		}

		UT66IdolAltarOverlayWidget* W = CreateWidget<UT66IdolAltarOverlayWidget>(this, ResolveIdolAltarOverlayClass());
		if (W)
		{
			W->SetSourceAltar(AutomationAltar);
			IdolAltarOverlayWidget = W;
			W->AddToViewport(150);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("weapon") || Mode == TEXT("weaponaltar") || Mode == TEXT("weaponaltaroverlay"))
	{
		if (bInventoryInspectOpen)
		{
			SetInventoryInspectOpen(false);
		}

		ET66WeaponRarity AutomationRarity = ET66WeaponRarity::Black;
		FString RarityString;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66WeaponAltarRarity="), RarityString))
		{
			const FString NormalizedRarity = RarityString.TrimStartAndEnd().ToLower();
			if (NormalizedRarity == TEXT("red"))
			{
				AutomationRarity = ET66WeaponRarity::Red;
			}
			else if (NormalizedRarity == TEXT("yellow"))
			{
				AutomationRarity = ET66WeaponRarity::Yellow;
			}
			else if (NormalizedRarity == TEXT("white"))
			{
				AutomationRarity = ET66WeaponRarity::White;
			}
		}

		AT66WeaponAltar* AutomationAltar = nullptr;
		if (UWorld* World = GetWorld())
		{
			const FVector AutomationAltarLocation = GetPawn() ? GetPawn()->GetActorLocation() + FVector(300.f, 0.f, 0.f) : FVector::ZeroVector;
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AutomationAltar = World->SpawnActor<AT66WeaponAltar>(AT66WeaponAltar::StaticClass(), AutomationAltarLocation, FRotator::ZeroRotator, SpawnParams);
			if (AutomationAltar)
			{
				AutomationAltar->SetActorHiddenInGame(true);
				AutomationAltar->SetActorEnableCollision(false);
				AutomationAltar->RemainingSelections = FMath::Max(1, AutomationAltar->RemainingSelections);
				AutomationAltar->WeaponOfferRarity = AutomationRarity;
			}
		}

		UT66WeaponAltarOverlayWidget* W = CreateWidget<UT66WeaponAltarOverlayWidget>(this, ResolveWeaponAltarOverlayClass());
		if (W)
		{
			W->SetSourceAltar(AutomationAltar);
			WeaponAltarOverlayWidget = W;
			W->AddToViewport(150);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("vendorinteractable") || Mode == TEXT("vendoronly") || Mode == TEXT("vendor"))
	{
		if (UWorld* World = GetWorld())
		{
			if (AT66VendorInteractable* VendorInteractable = T66FindRegisteredWorldInteractable<AT66VendorInteractable>(World))
			{
				OpenVendorInteractable(VendorInteractable);
				return;
			}
		}

		OpenCasinoVendorTab();
		return;
	}

	if (Mode == TEXT("casinoshop") || Mode == TEXT("shop") || Mode == TEXT("casinotabshop"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToVendorTab();
		return;
	}

	if (Mode == TEXT("casinogambling") || Mode == TEXT("gambling") || Mode == TEXT("casinotabgambling"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToGamblerTab();
		return;
	}

	if (Mode == TEXT("casinoalchemy") || Mode == TEXT("alchemy") || Mode == TEXT("casinotabalchemy"))
	{
		OpenCasinoOverlay();
		SwitchCasinoOverlayToVendorTab();
		return;
	}

	if (Mode == TEXT("collector") || Mode == TEXT("collectoroverlay"))
	{
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()); T66GI && !T66GI->IsCollectorPlayable())
		{
			return;
		}
		OpenCollectorOverlay();
		return;
	}

	if (Mode == TEXT("lab") || Mode == TEXT("laboverlay"))
	{
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()); T66GI && !T66GI->IsRunCategoryPlayable(ET66RunCategory::Lab))
		{
			return;
		}

		if (!LabOverlayWidget)
		{
			LabOverlayWidget = CreateWidget<UT66LabOverlayWidget>(this, UT66LabOverlayWidget::StaticClass());
		}
		if (LabOverlayWidget && !LabOverlayWidget->IsInViewport())
		{
			LabOverlayWidget->AddToViewport(100);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (Mode == TEXT("crate") || Mode == TEXT("crateoverlay"))
	{
		if (UT66CrateOverlayWidget* CrateOverlay = CreateWidget<UT66CrateOverlayWidget>(this, UT66CrateOverlayWidget::StaticClass()))
		{
			CrateOverlay->SetPresentationHost(GameplayHUDWidget);
			CrateOverlay->AddToViewport(110);
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}
		return;
	}

	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->SetFullMapOpen(false);
	}
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}
}

void AT66PlayerController::HandleGameplayAutomationScreenshot()
{
	if (GameplayAutomationScreenshotPath.IsEmpty())
	{
		return;
	}

	if (GameplayHUDWidget)
	{
		FString PickupCardItemIDString;
		if (FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoPickupCard="), PickupCardItemIDString)
			&& !PickupCardItemIDString.TrimStartAndEnd().IsEmpty())
		{
			ET66ItemRarity PickupCardRarity = ET66ItemRarity::Yellow;
			FString PickupCardRarityString;
			if (FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoPickupCardRarity="), PickupCardRarityString))
			{
				const FString NormalizedRarity = PickupCardRarityString.TrimStartAndEnd().ToLower();
				if (NormalizedRarity == TEXT("black"))
				{
					PickupCardRarity = ET66ItemRarity::Black;
				}
				else if (NormalizedRarity == TEXT("red"))
				{
					PickupCardRarity = ET66ItemRarity::Red;
				}
				else if (NormalizedRarity == TEXT("white"))
				{
					PickupCardRarity = ET66ItemRarity::White;
				}
			}

			GameplayHUDWidget->ShowPickupItemCard(FName(*PickupCardItemIDString.TrimStartAndEnd()), PickupCardRarity);
		}
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(GameplayAutomationScreenshotPath), true);
	FScreenshotRequest::RequestScreenshot(GameplayAutomationScreenshotPath, true, false, false);

	if (!bGameplayAutomationKeepAliveAfterScreenshot && GetWorld())
	{
		GetWorldTimerManager().ClearTimer(GameplayAutomationQuitTimerHandle);
		GetWorldTimerManager().SetTimer(
			GameplayAutomationQuitTimerHandle,
			this,
			&AT66PlayerController::HandleGameplayAutomationQuit,
			1.5f,
			false);
	}
}

void AT66PlayerController::HandleGameplayAutomationSequenceScreenshot()
{
	if (GameplayAutomationScreenshotSequenceDir.IsEmpty() || GameplayAutomationScreenshotSequenceCount <= 0)
	{
		return;
	}

	IFileManager::Get().MakeDirectory(*GameplayAutomationScreenshotSequenceDir, true);
	const FString FramePath = FPaths::Combine(
		GameplayAutomationScreenshotSequenceDir,
		FString::Printf(
			TEXT("%s_%04d.png"),
			*GameplayAutomationScreenshotSequencePrefix,
			GameplayAutomationScreenshotSequenceIndex));
	FScreenshotRequest::RequestScreenshot(FramePath, true, false, false);

	++GameplayAutomationScreenshotSequenceIndex;
	if (GameplayAutomationScreenshotSequenceIndex < GameplayAutomationScreenshotSequenceCount)
	{
		if (GetWorld())
		{
			GetWorldTimerManager().ClearTimer(GameplayAutomationScreenshotTimerHandle);
			GetWorldTimerManager().SetTimer(
				GameplayAutomationScreenshotTimerHandle,
				this,
				&AT66PlayerController::HandleGameplayAutomationSequenceScreenshot,
				GameplayAutomationScreenshotSequenceIntervalSeconds,
				false);
		}
		return;
	}

	if (!bGameplayAutomationKeepAliveAfterScreenshot && GetWorld())
	{
		GetWorldTimerManager().ClearTimer(GameplayAutomationQuitTimerHandle);
		GetWorldTimerManager().SetTimer(
			GameplayAutomationQuitTimerHandle,
			this,
			&AT66PlayerController::HandleGameplayAutomationQuit,
			1.5f,
			false);
	}
}

void AT66PlayerController::HandleGameplayAutomationWidgetDump()
{
	if (GameplayAutomationWidgetDumpTarget.IsEmpty() || GameplayAutomationWidgetDumpPath.IsEmpty())
	{
		return;
	}

	FString Error;
	const bool bDumped = FT66WidgetDumpTargets::DumpTargetToJson(
		GetWorld(),
		GameplayAutomationWidgetDumpTarget,
		GameplayAutomationWidgetDumpPath,
		Error);

	if (bDumped)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("Gameplay automation: widget target dump wrote Target=%s Path=%s"),
			*GameplayAutomationWidgetDumpTarget,
			*GameplayAutomationWidgetDumpPath);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Gameplay automation: widget target dump failed Target=%s Path=%s Error=%s"),
			*GameplayAutomationWidgetDumpTarget,
			*GameplayAutomationWidgetDumpPath,
			*Error);
	}

	if (GameplayAutomationScreenshotPath.IsEmpty()
		&& !bGameplayAutomationKeepAliveAfterScreenshot
		&& GetWorld())
	{
		GetWorldTimerManager().ClearTimer(GameplayAutomationQuitTimerHandle);
		GetWorldTimerManager().SetTimer(
			GameplayAutomationQuitTimerHandle,
			this,
			&AT66PlayerController::HandleGameplayAutomationQuit,
			1.5f,
			false);
	}
}

void AT66PlayerController::HandleGameplayAutomationQuit()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			MobManager->EmitRangedPressureSummary(TEXT("GameplayAutomationQuit"), true);
		}
		if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
		{
			ProjectileManager->EmitProjectileManagerSummary(TEXT("GameplayAutomationQuit"), true);
		}
	}
	ConsoleCommand(TEXT("quit"));
}

void AT66PlayerController::RebuildThemeAwareUI()
{
	if (UIManager)
	{
		UIManager->RebuildAllVisibleUI();
	}

	if (GameplayHUDWidget && GameplayHUDWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(GameplayHUDWidget, 0);
	}
	if (CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CasinoOverlayWidget, 100);
	}
	if (CollectorOverlayWidget && CollectorOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CollectorOverlayWidget, 100);
	}
	if (CowardicePromptWidget && CowardicePromptWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(CowardicePromptWidget, 200);
	}
	if (IdolAltarOverlayWidget && IdolAltarOverlayWidget->IsInViewport())
	{
		FT66Style::DeferRebuild(IdolAltarOverlayWidget, 150);
	}
}


void AT66PlayerController::OpenCasinoOverlay()
{
	if (!IsGameplayLevel()) return;
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CasinoOverlayWidget)
	{
		CasinoOverlayWidget = CreateWidget<UT66CasinoOverlayWidget>(this, ResolveCasinoOverlayClass());
	}

	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::Full);
		CasinoOverlayWidget->SetShopAllowsSteal(false);
		if (!CasinoOverlayWidget->IsInViewport())
		{
			CasinoOverlayWidget->AddToViewport(100);
		}

		CasinoOverlayWidget->OpenVendorTab();
		ApplyCasinoOverlayInputMode();
	}
}

void AT66PlayerController::CloseCasinoOverlay()
{
	ActiveCasinoInteractable.Reset();
	ActiveVendorInteractable.Reset();

	if (CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->RemoveFromParent();
	}
	RestoreGameplayInputMode();
}

void AT66PlayerController::SwitchCasinoOverlayToGamblerTab()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::Full);
		CasinoOverlayWidget->OpenGamblerTab();
		ApplyCasinoOverlayInputMode();
	}
}

void AT66PlayerController::SwitchCasinoOverlayToVendorTab()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::Full);
		CasinoOverlayWidget->OpenVendorTab();
		ApplyCasinoOverlayInputMode();
	}
}

void AT66PlayerController::SwitchCasinoOverlayToAlchemy()
{
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::Full);
		CasinoOverlayWidget->OpenVendorTab();
		ApplyCasinoOverlayInputMode();
	}
}

bool AT66PlayerController::IsCasinoOverlayOpen() const
{
	return CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport();
}

void AT66PlayerController::ApplyCasinoOverlayInputMode(const bool bReassertNextTick)
{
	if (!CasinoOverlayWidget || !CasinoOverlayWidget->IsInViewport())
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(CasinoOverlayWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	if (bReassertNextTick)
	{
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (AT66PlayerController* PC = WeakThis.Get())
				{
					PC->ApplyCasinoOverlayInputMode(false);
				}
			}));
		}
	}
}

bool AT66PlayerController::SpawnVendorBoss()
{
	if (!IsGameplayLevel()) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	UT66RunStateSubsystem* RunState = World->GetGameInstance() ? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState || RunState->GetBossActive())
	{
		return false;
	}

	if (T66HasRegisteredVendorBoss(World))
	{
		return false;
	}

	FVector SpawnLoc = FVector::ZeroVector;
	bool bHasSpawnLoc = false;

	if (AT66VendorInteractable* VendorInteractable = ActiveVendorInteractable.Get())
	{
		SpawnLoc = VendorInteractable->GetActorLocation();
		bHasSpawnLoc = true;
	}

	if (!bHasSpawnLoc)
	{
		if (AT66VendorInteractable* VendorInteractable = T66FindRegisteredWorldInteractable<AT66VendorInteractable>(World))
		{
			SpawnLoc = VendorInteractable->GetActorLocation();
			bHasSpawnLoc = true;
		}
	}

	if (!bHasSpawnLoc)
	{
		if (AT66CasinoInteractable* CasinoInteractable = ActiveCasinoInteractable.Get())
		{
			SpawnLoc = CasinoInteractable->GetActorLocation();
			bHasSpawnLoc = true;
		}
	}

	if (!bHasSpawnLoc)
	{
		if (AT66CasinoInteractable* CasinoInteractable = T66FindRegisteredWorldInteractable<AT66CasinoInteractable>(World))
		{
			SpawnLoc = CasinoInteractable->GetActorLocation();
			bHasSpawnLoc = true;
		}
	}

	if (!bHasSpawnLoc)
	{
		if (APawn* FallbackPawn = GetPawn())
		{
			SpawnLoc = FallbackPawn->GetActorLocation() + FVector(600.f, 0.f, 0.f);
			bHasSpawnLoc = true;
		}
	}
	if (!bHasSpawnLoc)
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66VendorBoss>(AT66VendorBoss::StaticClass(), SpawnLoc + FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator, Params);

	if (CasinoOverlayWidget && CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->RemoveFromParent();
	}
	ActiveCasinoInteractable.Reset();
	ActiveVendorInteractable.Reset();
	RestoreGameplayInputMode();
	return true;
}

void AT66PlayerController::OpenCasinoVendorTab()
{
	if (!IsGameplayLevel()) return;

	OpenCasinoOverlay();
	if (CasinoOverlayWidget)
	{
		CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::VendorOnly);
		CasinoOverlayWidget->SetShopAllowsSteal(true);
		CasinoOverlayWidget->OpenVendorTab();
		ApplyCasinoOverlayInputMode();
	}
}

bool AT66PlayerController::OpenCasinoGamblerInteractable(AT66CasinoInteractable* SourceInteractable)
{
	if (!IsGameplayLevel() || !SourceInteractable || SourceInteractable->bConsumed || IsCasinoOverlayOpen())
	{
		return false;
	}

	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CasinoOverlayWidget)
	{
		CasinoOverlayWidget = CreateWidget<UT66CasinoOverlayWidget>(this, ResolveCasinoOverlayClass());
	}

	if (!CasinoOverlayWidget)
	{
		return false;
	}

	ActiveCasinoInteractable = SourceInteractable;
	ActiveVendorInteractable.Reset();
	CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::GamblerOnly);
	CasinoOverlayWidget->SetCasinoGamblerWinGoldAmount(SourceInteractable->GetWinGoldAmount());
	if (!CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->AddToViewport(100);
	}

	CasinoOverlayWidget->OpenGamblerTab();
	ApplyCasinoOverlayInputMode();
	return true;
}

bool AT66PlayerController::OpenVendorInteractable(AT66VendorInteractable* SourceInteractable)
{
	if (!IsGameplayLevel() || !SourceInteractable || IsCasinoOverlayOpen())
	{
		return false;
	}

	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CasinoOverlayWidget)
	{
		CasinoOverlayWidget = CreateWidget<UT66CasinoOverlayWidget>(this, ResolveCasinoOverlayClass());
	}

	if (!CasinoOverlayWidget)
	{
		return false;
	}

	ActiveVendorInteractable = SourceInteractable;
	ActiveCasinoInteractable.Reset();
	CasinoOverlayWidget->SetOverlayMode(UT66CasinoOverlayWidget::ECasinoOverlayMode::VendorOnly);
	CasinoOverlayWidget->SetShopAllowsSteal(true);
	if (!CasinoOverlayWidget->IsInViewport())
	{
		CasinoOverlayWidget->AddToViewport(100);
	}

	CasinoOverlayWidget->OpenVendorTab();
	ApplyCasinoOverlayInputMode();
	return true;
}

void AT66PlayerController::HandleCasinoInteractableGambleResolved(const FName GameID, const bool bSuccessful, const int32 PayoutGold)
{
	(void)GameID;
	(void)bSuccessful;
	(void)PayoutGold;

	AT66CasinoInteractable* CasinoInteractable = ActiveCasinoInteractable.Get();
	if (!CasinoInteractable)
	{
		return;
	}

	CasinoInteractable->HandleCasinoGambleCompleted();
	ActiveCasinoInteractable.Reset();

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<AT66PlayerController> WeakThis(this);
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakThis]()
		{
			if (AT66PlayerController* PC = WeakThis.Get())
			{
				PC->CloseCasinoOverlay();
			}
		}));
	}
}


void AT66PlayerController::OpenCollectorOverlay()
{
	if (!IsGameplayLevel()) return;
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()); T66GI && !T66GI->IsCollectorPlayable())
	{
		return;
	}
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CollectorOverlayWidget)
	{
		CollectorOverlayWidget = CreateWidget<UT66CollectorOverlayWidget>(this, ResolveCollectorOverlayClass());
	}

	if (CollectorOverlayWidget && !CollectorOverlayWidget->IsInViewport())
	{
		CollectorOverlayWidget->AddToViewport(100);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::OpenCowardicePrompt(AT66CowardiceGate* Gate)
{
	if (!IsGameplayLevel() || !Gate) return;
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
	}

	if (!CowardicePromptWidget)
	{
		CowardicePromptWidget = CreateWidget<UT66CowardicePromptWidget>(this, ResolveCowardicePromptClass());
	}

	if (CowardicePromptWidget && !CowardicePromptWidget->IsInViewport())
	{
		CowardicePromptWidget->SetGate(Gate);
		CowardicePromptWidget->AddToViewport(200); // above gambler overlay
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::StartCrateOpenHUD(const ET66Rarity SourceCrateRarity)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->StartCrateOpen(SourceCrateRarity);
	}
}

bool AT66PlayerController::StartChestRewardHUD(
	ET66Rarity Rarity,
	int32 GoldAmount,
	TFunction<void()> OnCommit,
	TFunction<void()> OnFinished)
{
	if (!IsGameplayLevel()) return false;
	if (IsPaused()) return false;
	if (GameplayHUDWidget)
	{
		return GameplayHUDWidget->StartChestReward(Rarity, GoldAmount, MoveTemp(OnCommit), MoveTemp(OnFinished));
	}
	return false;
}

bool AT66PlayerController::StartLootWheelSpinHUD(FT66LootWheelPresentationParams Params)
{
	if (!IsGameplayLevel()) return false;
	if (IsPaused()) return false;
	if (GameplayHUDWidget)
	{
		return GameplayHUDWidget->StartLootWheelSpin(MoveTemp(Params));
	}
	return false;
}

void AT66PlayerController::ShowPickupItemCardHUD(const FName ItemID, const ET66ItemRarity ItemRarity)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->ShowPickupItemCard(ItemID, ItemRarity);
	}
}

void AT66PlayerController::ShowLootBagItemRevealHUD(const FName ItemID, const ET66ItemRarity ItemRarity)
{
	if (!IsGameplayLevel()) return;
	if (IsPaused()) return;
	if (GameplayHUDWidget)
	{
		GameplayHUDWidget->ShowLootBagItemReveal(ItemID, ItemRarity);
	}
}

void AT66PlayerController::OnPlayerDied()
{
	if (UWorld* World = GetWorld())
	{
		if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			MobManager->EmitRangedPressureSummary(TEXT("OnPlayerDied"), true);
		}
		if (UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>())
		{
			ProjectileManager->EmitProjectileManagerSummary(TEXT("OnPlayerDied"), true);
		}
	}
#if !UE_BUILD_SHIPPING
	if (GameplayAutomationCaptureMode.TrimStartAndEnd().Equals(TEXT("enemywaveperf"), ESearchCase::IgnoreCase))
	{
		FPlatformMisc::RequestExitWithStatus(false, -1, TEXT("T66EnemyWavePerfHeroDied"));
		return;
	}
#endif
	EndHeroOneScopedUlt();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
			}
		}
	}

	// Spawn death VFX (red pixel burst) at hero location before pausing.
	if (APawn* HeroPawn = GetPawn())
	{
		const FVector DeathLoc = HeroPawn->GetActorLocation();
		if (UT66CombatComponent* Combat = HeroPawn->FindComponentByClass<UT66CombatComponent>())
		{
			Combat->SpawnDeathVFX(DeathLoc);
		}
		HeroPawn->SetActorHiddenInGame(true);
	}

	// Brief delay so the death particles are visible before the game pauses.
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(DeathVFXTimerHandle, [this]()
		{
			SetPause(true);
			if (AT66GameMode* GameMode = GetWorld() ? Cast<AT66GameMode>(UGameplayStatics::GetGameMode(this)) : nullptr)
			{
				if (GameMode->ShouldEndgameDeathOpenRunSummary())
				{
					GameMode->HandleEndgameDeathRunSummary(this);
					return;
				}
			}
			EnsureGameplayUIManager();
			if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
			{
				UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
				if (RunState)
				{
					RunState->MarkRunEnded(false);
				}
				if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
				{
					Achieve->NotifyRunCompleted(RunState);
				}
			}
			if (UIManager)
			{
				UIManager->ShowModal(ET66ScreenType::GameOver);
			}
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
			bShowMouseCursor = true;
		}, 0.5f, false);
	}
	else
	{
		SetPause(true);
		if (AT66GameMode* GameMode = GetWorld() ? Cast<AT66GameMode>(UGameplayStatics::GetGameMode(this)) : nullptr)
		{
			if (GameMode->ShouldEndgameDeathOpenRunSummary())
			{
				GameMode->HandleEndgameDeathRunSummary(this);
				return;
			}
		}
		EnsureGameplayUIManager();
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->MarkRunEnded(false);
			}
		}
		if (UIManager)
		{
			UIManager->ShowModal(ET66ScreenType::GameOver);
		}
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
}

void AT66PlayerController::ShowVictoryRunSummary()
{
	EndHeroOneScopedUlt();

	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
			}
		}

		UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		if (RunState)
		{
			RunState->MarkRunEnded(true);
		}
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->NotifyRunCompleted(RunState);
		}
	}

	SetPause(true);
	EnsureGameplayUIManager();
	if (UIManager)
	{
		UIManager->ShowModal(ET66ScreenType::RunSummary);
	}
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AT66PlayerController::ClientShowVictoryRunSummary_Implementation()
{
	ShowVictoryRunSummary();
}


void AT66PlayerController::EnsureGameplayUIManager()
{
	if (!IsGameplayLevel() || !IsLocalController()) return;

	if (!UIManager)
	{
		UIManager = NewObject<UT66UIManager>(this, UT66UIManager::StaticClass());
		if (!UIManager) return;

		UIManager->Initialize(this);
	}

	auto RegisterGameplayScreen = [this](const ET66ScreenType ScreenType)
	{
		if (TSubclassOf<UT66ScreenBase> ScreenClass = ResolveScreenClass(ScreenType))
		{
			UIManager->RegisterScreenClass(ScreenType, ScreenClass);
		}
	};

	RegisterGameplayScreen(ET66ScreenType::PauseMenu);
	RegisterGameplayScreen(ET66ScreenType::Achievements);
	RegisterGameplayScreen(ET66ScreenType::ReportBug);
	RegisterGameplayScreen(ET66ScreenType::Settings);
	RegisterGameplayScreen(ET66ScreenType::RunSummary);
	RegisterGameplayScreen(ET66ScreenType::GameOver);
	RegisterGameplayScreen(ET66ScreenType::PlayerSummaryPicker);
	RegisterGameplayScreen(ET66ScreenType::SavePreview);
	RegisterGameplayScreen(ET66ScreenType::PowerUp);
	RegisterGameplayScreen(ET66ScreenType::AccountStatus);
	RegisterGameplayScreen(ET66ScreenType::PartyInvite);
}


void AT66PlayerController::HandleEscapePressed()
{
	if (!IsGameplayLevel())
	{
		if (UIManager)
		{
			UIManager->HandleBackAction();
		}
		return;
	}

#if !UE_BUILD_SHIPPING
	// Highest priority: if dev console is open, Esc should close it and never open PauseMenu.
	if (bDevConsoleOpen)
	{
		CloseDevConsole();

		// Debounce to avoid the same keypress immediately toggling PauseMenu.
		const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
		LastPauseToggleTime = Now;
		return;
	}
#endif

	if (IsPaused()
		&& UIManager
		&& UIManager->IsModalActive()
		&& UIManager->GetCurrentModalType() == ET66ScreenType::SavePreview)
	{
		if (UT66SavePreviewScreen* SavePreviewScreen = Cast<UT66SavePreviewScreen>(UIManager->GetCurrentModal()))
		{
			SavePreviewScreen->OnBackClicked();
		}
		return;
	}

	// Esc as back: close Media Viewer before other UI.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66MediaViewerSubsystem* MV = GI->GetSubsystem<UT66MediaViewerSubsystem>())
		{
			if (MV->IsMediaViewerOpen())
			{
				MV->SetMediaViewerOpen(false);
				SetIgnoreMoveInput(false);
				SetIgnoreLookInput(false);

				const bool bPauseMenuStillOpen = IsPaused()
					&& UIManager
					&& UIManager->IsModalActive()
					&& UIManager->GetCurrentModalType() == ET66ScreenType::PauseMenu;
				if (bPauseMenuStillOpen)
				{
					FInputModeGameAndUI InputMode;
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					SetInputMode(InputMode);
					bShowMouseCursor = true;
					bEnableClickEvents = true;
					bEnableMouseOverEvents = true;
					if (GameplayHUDWidget)
					{
						GameplayHUDWidget->SetInteractive(true);
						GameplayHUDWidget->MarkHUDDirty();
					}
				}
				else
				{
					RestoreGameplayInputMode();
				}
				return;
			}
		}
	}

	// Esc as back: close the full map before opening pause menu.
	if (bInventoryInspectOpen)
	{
		SetInventoryInspectOpen(false);
		return;
	}

	if (GameplayHUDWidget && GameplayHUDWidget->IsFullMapOpen())
	{
		GameplayHUDWidget->SetFullMapOpen(false);
		RestoreGameplayInputMode();
		return;
	}

	const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
	const bool bPaused = IsPaused();

	if (bPaused
		&& UIManager
		&& UIManager->IsModalActive()
		&& UIManager->GetCurrentModalType() == ET66ScreenType::PauseMenu)
	{
		if (UT66ScreenBase* PauseModal = UIManager->GetCurrentModal())
		{
			if (PauseModal->HandleBackAction())
			{
				return;
			}
		}
	}

	// Closing a sub-modal (Settings / Report Bug / Achievements) returns to Pause menu without debounce so the next Esc can unpause.
	if (bPaused && UIManager && UIManager->IsModalActive())
	{
		const ET66ScreenType ClosingModal = UIManager->GetCurrentModalType();
		if (ClosingModal == ET66ScreenType::Settings
			|| ClosingModal == ET66ScreenType::ReportBug
			|| ClosingModal == ET66ScreenType::Achievements
			|| ClosingModal == ET66ScreenType::PlayerSummaryPicker
			|| ClosingModal == ET66ScreenType::AccountStatus
			|| ClosingModal == ET66ScreenType::RunSummary)
		{
			UIManager->CloseModal();

			if (ClosingModal == ET66ScreenType::RunSummary)
			{
				if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
				{
					if (UT66LeaderboardSubsystem* LB = GI->GetSubsystem<UT66LeaderboardSubsystem>())
					{
						const ET66ScreenType ReturnModal = LB->ConsumePendingReturnModalAfterViewerRunSummary();
						if (ReturnModal != ET66ScreenType::None)
						{
							UIManager->ShowModal(ReturnModal);
							return;
						}
					}
				}
			}

			UIManager->ShowModal(ET66ScreenType::PauseMenu);
			return;
		}
	}

	// Debounce only when actually toggling pause (open or full unpause)
	if (Now - LastPauseToggleTime < PauseToggleDebounceSeconds)
	{
		return;
	}
	LastPauseToggleTime = Now;

	if (!bPaused)
	{
		SetPause(true);
		EnsureGameplayUIManager();
		if (UIManager)
		{
			UIManager->ShowModal(ET66ScreenType::PauseMenu);
		}
		FInputModeGameAndUI InputMode;
		if (UIManager && UIManager->GetCurrentModal())
		{
			InputMode.SetWidgetToFocus(UIManager->GetCurrentModal()->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
		if (GameplayHUDWidget)
		{
			GameplayHUDWidget->SetInteractive(true);
			GameplayHUDWidget->MarkHUDDirty();
		}
	}
	else
	{
		if (UIManager && UIManager->IsModalActive())
		{
			UIManager->CloseModal();
			SetPause(false);
			RestoreGameplayInputMode();
		}
		else
		{
			SetPause(false);
			RestoreGameplayInputMode();
		}
	}
}
