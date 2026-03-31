// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66EnemyDirector.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66StartGate.h"
#include "Gameplay/T66StageGate.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66MiasmaManager.h"
#include "Gameplay/T66MiasmaBoundary.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66GamblerNPC.h"
#include "Gameplay/T66SaintNPC.h"
#include "Gameplay/T66OuroborosNPC.h"
#include "Gameplay/T66LoanShark.h"
#include "Gameplay/T66HouseNPCBase.h"
#include "Gameplay/T66CowardiceGate.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66TricksterNPC.h"
#include "Gameplay/T66DifficultyTotem.h"
#include "Gameplay/T66BossGate.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66GoblinThiefEnemy.h"
#include "Gameplay/T66IdolAltar.h"
#include "Gameplay/T66FountainOfLifeInteractable.h"
#include "Gameplay/T66ChestInteractable.h"
#include "Gameplay/T66WheelSpinInteractable.h"
#include "Gameplay/T66CrateInteractable.h"
#include "Gameplay/T66CircusInteractable.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66QuickReviveVendingMachine.h"
#include "Gameplay/T66TeleportPadInteractable.h"
#include "Gameplay/T66StageCatchUpGate.h"
#include "Gameplay/T66StageCatchUpGoldInteractable.h"
#include "Gameplay/T66StageCatchUpLootInteractable.h"
#include "Gameplay/T66StageEffects.h"
#include "Gameplay/T66LavaPatch.h"
#include "Gameplay/T66SpawnPlateau.h"
#include "Gameplay/T66LabCollector.h"
#include "Gameplay/T66QuakeSkyActor.h"
#include "Gameplay/T66TutorialManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66RetroFXSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PosterizeSubsystem.h"
#include "Gameplay/T66EclipseActor.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RngSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66PlayerExperienceSubSystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66SkillRatingSubsystem.h"
#include "Core/T66GameplayLayout.h"
#include "Core/T66PropSubsystem.h"
#include "Gameplay/T66RecruitableCompanion.h"
#include "Gameplay/T66PlayerController.h"
#include "UI/T66LoadingScreenWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/TextureCube.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

#if WITH_EDITORONLY_DATA
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Math/Float16.h"
#endif
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "Gameplay/T66MainMapTerrain.h"
#include "Gameplay/T66ProceduralLandscapeParams.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Landscape.h"
#include "UI/Style/T66Style.h"
#include "T66.h"

namespace
{
	// Helper: avoid PIE warnings ("StaticMeshComponent has to be 'Movable' if you'd like to move")
	// by temporarily setting mobility to Movable while we apply transforms.
	static void T66_SetStaticMeshActorMobility(AStaticMeshActor* Actor, EComponentMobility::Type Mobility)
	{
		if (!Actor) return;
		if (UStaticMeshComponent* SMC = Actor->GetStaticMeshComponent())
		{
			if (SMC->Mobility != Mobility)
			{
				SMC->SetMobility(Mobility);
			}
		}
	}

	static bool T66_IsCompanionUnlockStage(int32 StageNum)
	{
		// Per design: 24 total companions unlock on every non-boss stage in 1..30.
		// Stages 31..33 never unlock companions.
		return StageNum >= 1 && StageNum <= 30 && (StageNum % 5) != 0;
	}

	static int32 T66_CompanionIndexForStage(int32 StageNum)
	{
		if (!T66_IsCompanionUnlockStage(StageNum))
		{
			return INDEX_NONE;
		}

		int32 Count = 0;
		for (int32 S = 1; S <= StageNum; ++S)
		{
			if (T66_IsCompanionUnlockStage(S))
			{
				++Count;
			}
		}
		return Count; // 1..24
	}

	static FName T66_CompanionIDForStage(int32 StageNum)
	{
		const int32 Index = T66_CompanionIndexForStage(StageNum);
		if (Index == INDEX_NONE)
		{
			return NAME_None;
		}
		return FName(*FString::Printf(TEXT("Companion_%02d"), Index));
	}

	static bool T66_HasAnyFloorTag(const AActor* A)
	{
		if (!A) return false;
		// Covers: T66_Floor_Main/Start/Boss/Conn*, and also Boost/Tutorial/Coliseum floors.
		static const FName Prefix(TEXT("T66_Floor"));
		for (const FName& Tag : A->Tags)
		{
			const FString S = Tag.ToString();
			if (S.StartsWith(Prefix.ToString()))
			{
				return true;
			}
		}
		return false;
	}

	static const FName T66MainMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	static const FName T66MainMapTerrainMaterialsReadyTag(TEXT("T66_MainMapTerrain_MaterialsReady"));

	static bool T66AreMainMapTerrainMaterialsReady(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || !Actor->ActorHasTag(T66MainMapTerrainVisualTag))
			{
				continue;
			}

			return Actor->ActorHasTag(T66MainMapTerrainMaterialsReadyTag);
		}

		return false;
	}

	static bool T66UsesMainMapTerrainStage(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return !MapName.Contains(TEXT("Coliseum"))
			&& !MapName.Contains(TEXT("Tutorial"))
			&& !MapName.Contains(TEXT("Lab"));
	}

	static bool T66IsStandaloneColiseumMap(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return MapName.Contains(TEXT("Coliseum"));
	}

	static int32 T66EnsureRunSeed(UT66GameInstance* GI)
	{
		if (!GI)
		{
			return FMath::Rand();
		}

		if (GI->RunSeed == 0)
		{
			GI->RunSeed = FMath::Rand();
		}

		return GI->RunSeed;
	}

	static FT66MapPreset T66BuildMainMapPreset(UT66GameInstance* GI)
	{
		const ET66Difficulty Difficulty = GI ? GI->SelectedDifficulty : ET66Difficulty::Easy;
		return T66MainMapTerrain::BuildPresetForDifficulty(Difficulty, T66EnsureRunSeed(GI));
	}

	static bool T66IsStandaloneTutorialMap(const UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		const FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		return MapName.Contains(TEXT("Tutorial"));
	}

	static const FName T66_CombatIterationEnemyTag(TEXT("T66.Dev.CombatIterationEnemy"));
	static TAutoConsoleVariable<int32> CVarT66DevCombatIterationEquipStageTestIdol(
		TEXT("T66.Dev.CombatIterationEquipStageTestIdol"),
		1,
		TEXT("PIE-only combat iteration helper. When enabled, equips one deterministic idol for the current VFX stage after clearing the startup loadout so idol VFX stages can be tested in isolation."),
		ECVF_Default);
	static TAutoConsoleVariable<int32> CVarT66DevCombatIterationStageTestIdolIndex(
		TEXT("T66.Dev.CombatIterationStageTestIdolIndex"),
		13,
		TEXT("PIE-only combat iteration helper. 1..24 selects the deterministic idol equipped in slot 0 for isolated VFX tests. Default 13 = Idol_Fire."),
		ECVF_Default);
	static int32 T66_CountEquippedEntries(const TArray<FName>& Entries)
	{
		int32 Count = 0;
		for (const FName Entry : Entries)
		{
			if (!Entry.IsNone())
			{
				++Count;
			}
		}
		return Count;
	}

	static const TArray<FName>& T66_GetCombatIterationOrderedIdolIDs()
	{
		return UT66IdolManagerSubsystem::GetAllIdolIDs();
	}

	static int32 T66_GetCombatIterationStageTestIdolIndex()
	{
		const TArray<FName>& OrderedIdols = T66_GetCombatIterationOrderedIdolIDs();
		if (OrderedIdols.Num() <= 0)
		{
			return INDEX_NONE;
		}

		const int32 RawIndex = CVarT66DevCombatIterationStageTestIdolIndex.GetValueOnGameThread();
		return FMath::Clamp(RawIndex, 1, OrderedIdols.Num());
	}

	static FName T66_GetCombatIterationStageTestIdolID()
	{
		if (CVarT66DevCombatIterationEquipStageTestIdol.GetValueOnGameThread() == 0)
		{
			return NAME_None;
		}

		const TArray<FName>& OrderedIdols = T66_GetCombatIterationOrderedIdolIDs();
		const int32 IdolIndex = T66_GetCombatIterationStageTestIdolIndex();
		return OrderedIdols.IsValidIndex(IdolIndex - 1) ? OrderedIdols[IdolIndex - 1] : NAME_None;
	}

	static FName T66_GetCombatIterationStageTestHeroID()
	{
		return FName(TEXT("Hero_1"));
	}

	static const TCHAR* T66_GetCombatIterationRarityName(const ET66ItemRarity Rarity)
	{
		switch (Rarity)
		{
		case ET66ItemRarity::Black:  return TEXT("Black");
		case ET66ItemRarity::Red:    return TEXT("Red");
		case ET66ItemRarity::Yellow: return TEXT("Yellow");
		case ET66ItemRarity::White:  return TEXT("White");
		default:                     return TEXT("Unknown");
		}
	}

	static UWorld* T66_GetCombatIterationConsoleWorld()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		if (UWorld* PlayWorld = GEngine->GetCurrentPlayWorld())
		{
			return PlayWorld;
		}

		if (GEngine->GameViewport)
		{
			return GEngine->GameViewport->GetWorld();
		}

		return nullptr;
	}

	static void T66_LogCombatIterationIdolOrder()
	{
		const TArray<FName>& OrderedIdols = T66_GetCombatIterationOrderedIdolIDs();
		UE_LOG(LogTemp, Log, TEXT("[DEV][CombatIteration] Idol alias order:"));
		for (int32 Index = 0; Index < OrderedIdols.Num(); ++Index)
		{
			UE_LOG(LogTemp, Log, TEXT("[DEV][CombatIteration]   idol%d = %s"), Index + 1, *OrderedIdols[Index].ToString());
		}
	}

	static void T66_ApplyCombatIterationStageTestIdolSelection(UWorld* World, const int32 IdolIndex)
	{
		if (!World)
		{
			return;
		}

		UGameInstance* GI = World->GetGameInstance();
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		if (!RunState)
		{
			return;
		}

		const TArray<FName>& OrderedIdols = T66_GetCombatIterationOrderedIdolIDs();
		if (!OrderedIdols.IsValidIndex(IdolIndex - 1))
		{
			return;
		}

		const FName IdolID = OrderedIdols[IdolIndex - 1];
		RunState->ClearEquippedIdols();
		const bool bEquipped = RunState->EquipIdolInSlot(0, IdolID);
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[DEV][CombatIteration] Applied test idol selection immediately: Index=%d Idol=%s Slot=0 Equipped=%d World=%s"),
			IdolIndex,
			*IdolID.ToString(),
			bEquipped ? 1 : 0,
			*World->GetMapName());
	}

	static void T66_SetCombatIterationStageTestIdolIndex(const int32 RequestedIndex, UWorld* World)
	{
		const TArray<FName>& OrderedIdols = T66_GetCombatIterationOrderedIdolIDs();
		if (OrderedIdols.Num() <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DEV][CombatIteration] No idols are registered for selection."));
			return;
		}

		const int32 ClampedIndex = FMath::Clamp(RequestedIndex, 1, OrderedIdols.Num());
		CVarT66DevCombatIterationStageTestIdolIndex->Set(ClampedIndex, ECVF_SetByConsole);
		CVarT66DevCombatIterationEquipStageTestIdol->Set(1, ECVF_SetByConsole);

		const FName IdolID = OrderedIdols[ClampedIndex - 1];
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[DEV][CombatIteration] Selected test idol alias: idol%d -> %s (Requested=%d, NextPIE=enabled)"),
			ClampedIndex,
			*IdolID.ToString(),
			RequestedIndex);

		UWorld* EffectiveWorld = World ? World : T66_GetCombatIterationConsoleWorld();
		if (EffectiveWorld)
		{
			T66_ApplyCombatIterationStageTestIdolSelection(EffectiveWorld, ClampedIndex);
		}
	}

	static void T66_SelectCombatIterationStageTestIdolByArgs(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() <= 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[DEV][CombatIteration] Usage: idol <1-24>"));
			T66_LogCombatIterationIdolOrder();
			return;
		}

		const int32 RequestedIndex = FCString::Atoi(*Args[0]);
		if (RequestedIndex <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DEV][CombatIteration] Invalid idol index '%s'. Usage: idol <1-24>"), *Args[0]);
			return;
		}

		T66_SetCombatIterationStageTestIdolIndex(RequestedIndex, World);
	}

	static void T66_ListCombatIterationIdolAliases(const TArray<FString>& Args, UWorld* World)
	{
		(void)Args;
		(void)World;
		T66_LogCombatIterationIdolOrder();
	}

	static FAutoConsoleCommandWithWorldAndArgs T66CombatIterationIdolCommand(
		TEXT("idol"),
		TEXT("Select the combat-iteration test idol by 1-based index. Example: idol 13"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_SelectCombatIterationStageTestIdolByArgs));

	static FAutoConsoleCommandWithWorldAndArgs T66CombatIterationIdolListCommand(
		TEXT("idollist"),
		TEXT("Log the 1..24 idol alias order used by idol1..idol24."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&T66_ListCombatIterationIdolAliases));

#define T66_REGISTER_IDOL_ALIAS(N) \
	static FAutoConsoleCommandWithWorldAndArgs T66CombatIterationIdolAlias##N( \
		TEXT("idol" #N), \
		TEXT("Select combat-iteration test idol #" #N "."), \
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World) \
		{ \
			(void)Args; \
			T66_SetCombatIterationStageTestIdolIndex(N, World); \
		}) \
	);

	T66_REGISTER_IDOL_ALIAS(1)
	T66_REGISTER_IDOL_ALIAS(2)
	T66_REGISTER_IDOL_ALIAS(3)
	T66_REGISTER_IDOL_ALIAS(4)
	T66_REGISTER_IDOL_ALIAS(5)
	T66_REGISTER_IDOL_ALIAS(6)
	T66_REGISTER_IDOL_ALIAS(7)
	T66_REGISTER_IDOL_ALIAS(8)
	T66_REGISTER_IDOL_ALIAS(9)
	T66_REGISTER_IDOL_ALIAS(10)
	T66_REGISTER_IDOL_ALIAS(11)
	T66_REGISTER_IDOL_ALIAS(12)
	T66_REGISTER_IDOL_ALIAS(13)
	T66_REGISTER_IDOL_ALIAS(14)
	T66_REGISTER_IDOL_ALIAS(15)
	T66_REGISTER_IDOL_ALIAS(16)
	T66_REGISTER_IDOL_ALIAS(17)
	T66_REGISTER_IDOL_ALIAS(18)
	T66_REGISTER_IDOL_ALIAS(19)
	T66_REGISTER_IDOL_ALIAS(20)
	T66_REGISTER_IDOL_ALIAS(21)
	T66_REGISTER_IDOL_ALIAS(22)
	T66_REGISTER_IDOL_ALIAS(23)
	T66_REGISTER_IDOL_ALIAS(24)

#undef T66_REGISTER_IDOL_ALIAS

	static bool T66_ShouldApplyCombatIterationSetup(const UWorld* World, const UT66GameInstance* T66GI, const UT66RunStateSubsystem* RunState)
	{
#if WITH_EDITOR
		if (!World || World->WorldType != EWorldType::PIE)
		{
			return false;
		}
#else
		return false;
#endif

		if (!RunState)
		{
			return false;
		}

		if (T66GI && T66GI->bLoadAsPreview)
		{
			return false;
		}

		const bool bIsLab = T66GI && T66GI->bIsLabLevel;
		return bIsLab || RunState->GetCurrentStage() == 1;
	}

	static void T66_ResetCombatIterationLoadout(UT66RunStateSubsystem* RunState)
	{
		if (!RunState)
		{
			return;
		}

		const int32 RemovedItemCount = RunState->GetInventorySlots().Num();
		const int32 RemovedIdolCount = T66_CountEquippedEntries(RunState->GetEquippedIdols());
		RunState->ClearInventory();
		RunState->ClearEquippedIdols();
		RunState->SetInStageCatchUp(false);
		UE_LOG(LogTemp, Log, TEXT("[DEV][CombatIteration] Cleared startup loadout for isolated VFX tests. RemovedItems=%d RemovedIdols=%d"), RemovedItemCount, RemovedIdolCount);
	}

	static void T66_SeedCombatIterationHeroSelection(UT66GameInstance* T66GI)
	{
		if (!T66GI)
		{
			return;
		}

		const FName TestHeroID = T66_GetCombatIterationStageTestHeroID();
		const FName DefaultSkinID(TEXT("Default"));

		T66GI->SelectedHeroID = TestHeroID;
		T66GI->SelectedHeroBodyType = ET66BodyType::TypeA;
		T66GI->SelectedHeroSkinID = DefaultSkinID;

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[DEV][CombatIteration] Forced hero selection for isolated VFX tests: Hero=%s BodyType=%s Skin=%s"),
			*TestHeroID.ToString(),
			TEXT("TypeA"),
			*DefaultSkinID.ToString());
	}

	static void T66_SeedCombatIterationStageTestIdol(UT66RunStateSubsystem* RunState)
	{
		if (!RunState)
		{
			return;
		}

		const FName TestIdolID = T66_GetCombatIterationStageTestIdolID();
		if (TestIdolID.IsNone())
		{
			UE_LOG(LogTemp, Log, TEXT("[DEV][CombatIteration] Stage test idol disabled. Idol loadout remains empty."));
			return;
		}

		const bool bEquipped = RunState->EquipIdolInSlot(0, TestIdolID);
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[DEV][CombatIteration] Equipped stage test idol: Idol=%s Slot=0 Equipped=%d Rarity=%s"),
			*TestIdolID.ToString(),
			bEquipped ? 1 : 0,
			T66_GetCombatIterationRarityName(RunState->GetEquippedIdolRarityInSlot(0)));
	}

	static void T66_SpawnCombatIterationEnemies(UWorld* World, const APawn* Pawn)
	{
		if (!World || !Pawn)
		{
			return;
		}

		TArray<AActor*> ExistingTaggedEnemies;
		for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
		{
			if (AT66EnemyBase* ExistingEnemy = *It)
			{
				if (ExistingEnemy->Tags.Contains(T66_CombatIterationEnemyTag))
				{
					ExistingTaggedEnemies.Add(ExistingEnemy);
				}
			}
		}
		for (AActor* ExistingEnemy : ExistingTaggedEnemies)
		{
			if (ExistingEnemy)
			{
				ExistingEnemy->Destroy();
			}
		}

		FVector Forward = Pawn->GetActorForwardVector();
		Forward.Z = 0.f;
		if (!Forward.Normalize())
		{
			Forward = FVector::ForwardVector;
		}

		FVector Right = Pawn->GetActorRightVector();
		Right.Z = 0.f;
		if (!Right.Normalize())
		{
			Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();
		}

		const FVector PawnLocation = Pawn->GetActorLocation();
		const FVector BaseSpawnLocation = PawnLocation + Forward * 475.f;
		const float LateralSpacing = 170.f;
		const float EnemyHeightOffset = 90.f;
		const FRotator EnemyRotation = (-Forward).Rotation();

		for (int32 EnemyIndex = 0; EnemyIndex < 3; ++EnemyIndex)
		{
			const float SideOffset = static_cast<float>(EnemyIndex - 1) * LateralSpacing;
			FVector SpawnLocation = BaseSpawnLocation + Right * SideOffset;

			FHitResult GroundHit;
			const FVector TraceStart = SpawnLocation + FVector(0.f, 0.f, 600.f);
			const FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, 1400.f);
			if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic))
			{
				SpawnLocation = GroundHit.ImpactPoint + FVector(0.f, 0.f, EnemyHeightOffset);
			}
			else
			{
				SpawnLocation.Z = PawnLocation.Z;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			if (AT66EnemyBase* SpawnedEnemy = World->SpawnActor<AT66EnemyBase>(AT66EnemyBase::StaticClass(), SpawnLocation, EnemyRotation, SpawnParams))
			{
				SpawnedEnemy->MaxHP = 999999;
				SpawnedEnemy->CurrentHP = SpawnedEnemy->MaxHP;
				SpawnedEnemy->bDropsLoot = false;
				SpawnedEnemy->XPValue = 0;
				SpawnedEnemy->PointValue = 0;
				SpawnedEnemy->Tags.AddUnique(T66_CombatIterationEnemyTag);
				SpawnedEnemy->AutoAttackKnockbackSpeed = 0.f;
				if (UCharacterMovementComponent* Movement = SpawnedEnemy->GetCharacterMovement())
				{
					Movement->DisableMovement();
					Movement->StopMovementImmediately();
				}
				SpawnedEnemy->UpdateHealthBar();
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[DEV][CombatIteration] Spawned 3 stationary invulnerable enemy targets in front of the hero start."));
	}

	static void T66_ApplyCombatIterationSetup(UWorld* World, APawn* Pawn, UT66RunStateSubsystem* RunState, UT66GameInstance* T66GI)
	{
		if (!T66_ShouldApplyCombatIterationSetup(World, T66GI, RunState) || !Pawn)
		{
			return;
		}

		T66_SeedCombatIterationHeroSelection(T66GI);
		T66_ResetCombatIterationLoadout(RunState);
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[DEV][CombatIteration] Active hero for isolated VFX tests: SelectedHero=%s InventorySlots=%d EquippedIdols=%d PreviewMode=%d"),
			(T66GI && !T66GI->SelectedHeroID.IsNone()) ? *T66GI->SelectedHeroID.ToString() : TEXT("None"),
			RunState ? RunState->GetInventorySlots().Num() : -1,
			RunState ? T66_CountEquippedEntries(RunState->GetEquippedIdols()) : -1,
			(T66GI && T66GI->bLoadAsPreview) ? 1 : 0);
		T66_SeedCombatIterationStageTestIdol(RunState);
		T66_SpawnCombatIterationEnemies(World, Pawn);
	}
}

AT66GameMode::AT66GameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// Default to our hero base class
	DefaultPawnClass = AT66HeroBase::StaticClass();
	DefaultHeroClass = AT66HeroBase::StaticClass();

	// Coliseum arena lives inside GameplayLevel, off to the side (walled off). Scaled for 100k map.
	ColiseumCenter = FVector(-45455.f, -23636.f, 200.f);

	// Default terrain materials; used as a fallback for runtime floors and terrain when source files are absent.
	GroundFloorMaterials.Empty();
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile1.MI_GroundTile1"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile2.MI_GroundTile2"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile3.MI_GroundTile3"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile4.MI_GroundTile4"))));
	GroundFloorMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Ground/MI_GroundTile5.MI_GroundTile5"))));

	CliffSideMaterials.Empty();
	CliffSideMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile1.MI_HillTile1"))));
	CliffSideMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile2.MI_HillTile2"))));
	CliffSideMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile3.MI_HillTile3"))));
	CliffSideMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile4.MI_HillTile4"))));
}

UStaticMesh* AT66GameMode::GetCubeMesh()
{
	if (!CachedCubeMesh)
	{
		CachedCubeMesh = FT66VisualUtil::GetBasicShapeCube();
	}
	return CachedCubeMesh;
}


void AT66GameMode::BeginPlay()
{
	Super::BeginPlay();

	// Main flat floor is spawned in SpawnLevelContentAfterLandscapeReady (no external asset packs).

	// Reset run state when entering gameplay level unless this is a stage transition (keep progress)
	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (GI && T66GI)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			// Bind to timer changes so we can spawn LoanShark exactly when timer starts.
			RunState->StageTimerChanged.AddDynamic(this, &AT66GameMode::HandleStageTimerChanged);
			RunState->DifficultyChanged.AddDynamic(this, &AT66GameMode::HandleDifficultyChanged);

			// Robust: treat any stage > 1 as a stage transition even if a gate forgot to set the flag.
			const bool bKeepProgress = T66GI->bIsStageTransition || (RunState->GetCurrentStage() > 1);
			if (bKeepProgress)
			{
				T66GI->bIsStageTransition = false;
				RunState->ResetStageTimerToFull(); // New stage: timer frozen at 60 until start gate
				RunState->ResetBossState(); // New stage: boss is dormant again; hide boss UI
			}
			else
			{
				T66GI->bRunIneligibleForLeaderboard = false; // Fresh run is eligible
				RunState->ResetForNewRun();
				if (UT66DamageLogSubsystem* DamageLog = GI->GetSubsystem<UT66DamageLogSubsystem>())
				{
					DamageLog->ResetForNewRun();
				}
			}
		}
	}

	if (bAutoSetupLevel)
	{
		EnsureLevelSetup();
	}

	if (UGameInstance* GIP = GetGameInstance())
	{
		if (UT66PlayerSettingsSubsystem* PS = GIP->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.AddDynamic(this, &AT66GameMode::HandleSettingsChanged);
		}
	}
	HandleSettingsChanged();

	// Coliseum mode: only spawn owed bosses + miasma (no houses, no waves, no NPCs, no start gate/pillars).
	if (IsColiseumStage())
	{
		ResetColiseumState();
		if (UGameInstance* GI2 = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI2->GetSubsystem<UT66RunStateSubsystem>())
			{
				// Coliseum countdown begins immediately (no start gate).
				RunState->ResetStageTimerToFull();
				RunState->SetStageTimerActive(true);
			}
		}

		// Coliseum still has miasma.
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			MiasmaManager = GetWorld()->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}

		SpawnColiseumArenaIfNeeded();
		SpawnAllOwedBossesInColiseum();
		UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - Coliseum"));
		return;
	}

	// The Lab: minimal arena, hero + companion only; no waves, NPCs, gates, or run progress.
	if (IsLabLevel())
	{
		if (UGameInstance* GILab = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GILab->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->ResetForNewRun();
			}
			if (UT66DamageLogSubsystem* DamageLog = GILab->GetSubsystem<UT66DamageLogSubsystem>())
			{
				DamageLog->ResetForNewRun();
			}
		}
		if (bAutoSetupLevel)
		{
			EnsureLevelSetup();
		}
		UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - Lab"));
		return;
	}

	UT66GameInstance* GIAsT66 = GetT66GameInstance();
	const bool bStageCatchUp = (GIAsT66 && GIAsT66->bStageCatchUpPending);

	if (bStageCatchUp)
	{
		GIAsT66->bStageCatchUpPending = false;
		if (UT66RunStateSubsystem* RunState = GIAsT66->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->SetInStageCatchUp(false);
		}
	}

	// Normal stage: defer all ground-dependent spawns until next tick so the landscape is fully formed and collision is ready.
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimerForNextTick(this, &AT66GameMode::SpawnLevelContentAfterLandscapeReady);
	}
	UE_LOG(LogTemp, Log, TEXT("T66GameMode BeginPlay - Level setup scheduled; content will spawn after landscape is ready."));
}

void AT66GameMode::SpawnLevelContentAfterLandscapeReady()
{
	UWorld* World = GetWorld();
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	TWeakObjectPtr<UT66LoadingScreenWidget> GameplayWarmupOverlay;
	if (bUsingMainMapTerrain)
	{
		if (APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr)
		{
			if (UT66LoadingScreenWidget* Overlay = CreateWidget<UT66LoadingScreenWidget>(PC, UT66LoadingScreenWidget::StaticClass()))
			{
				Overlay->AddToViewport(10000);
				GameplayWarmupOverlay = Overlay;
			}
		}
	}
	auto ScheduleLightingRefresh = [this, World]()
	{
		if (!World)
		{
			return;
		}

		// Recapture after runtime terrain/props register so first PIE does not keep a cold sky capture.
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			ApplyThemeToAtmosphereAndLighting();
		}));

		FTimerHandle DelayedLightingRefreshHandle;
		World->GetTimerManager().SetTimer(
			DelayedLightingRefreshHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				ApplyThemeToAtmosphereAndLighting();
			}),
			0.35f,
			false);

		FTimerHandle FinalLightingRefreshHandle;
		World->GetTimerManager().SetTimer(
			FinalLightingRefreshHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				ApplyThemeToAtmosphereAndLighting();
			}),
			0.65f,
			false);
	};
	auto ScheduleOverlayHide = [this, World, GameplayWarmupOverlay]()
	{
		if (!World || !GameplayWarmupOverlay.IsValid())
		{
			return;
		}

		TSharedPtr<int32> OverlayPollCount = MakeShared<int32>(0);
		TSharedPtr<FTimerHandle> HideOverlayHandle = MakeShared<FTimerHandle>();
		World->GetTimerManager().SetTimer(
			*HideOverlayHandle,
			FTimerDelegate::CreateWeakLambda(this, [World, GameplayWarmupOverlay, OverlayPollCount, HideOverlayHandle]()
			{
				if (!World)
				{
					return;
				}

				if (!GameplayWarmupOverlay.IsValid())
				{
					World->GetTimerManager().ClearTimer(*HideOverlayHandle);
					return;
				}

				const bool bMaterialsReady = T66AreMainMapTerrainMaterialsReady(World);
				++(*OverlayPollCount);
				const bool bTimedOut = *OverlayPollCount >= 50;
				if (bMaterialsReady || bTimedOut)
				{
					GameplayWarmupOverlay->RemoveFromParent();
					World->GetTimerManager().ClearTimer(*HideOverlayHandle);
					if (bTimedOut && !bMaterialsReady)
					{
						UE_LOG(LogTemp, Warning, TEXT("T66GameMode - Gameplay warmup overlay timed out before main map terrain materials reported ready."));
					}
				}
			}),
			0.10f,
			true);
	};

	// Phase 0: Spawn the runtime main map terrain before any ground-traced content.
	SpawnMainMapTerrain();

	// Start gate: spawned here (after floor) so the ground trace hits the flat floor.
	if (!bUsingMainMapTerrain && !IsColiseumStage() && !IsLabLevel())
	{
		AController* PC = World ? World->GetFirstPlayerController() : nullptr;
		if (PC)
		{
			SpawnStartGateForPlayer(PC);
		}
	}

	// Phase 1: Spawn ground-dependent structures (houses, NPCs, world interactables, tiles).
	SpawnCornerHousesAndNPCs();
	SpawnCircusInteractableIfNeeded();
	SpawnSupportVendorAtStartIfNeeded();
	if (AController* PC = World ? World->GetFirstPlayerController() : nullptr)
	{
		SpawnIdolAltarForPlayer(PC);
	}
	if (!bUsingMainMapTerrain)
	{
		SpawnTricksterAndCowardiceGate();
		SpawnBossBeaconIfNeeded();
	}
	if (!bUsingMainMapTerrain)
	{
		SpawnWorldInteractablesForStage();
	}

	// Scatter decorative props.
	if (UGameInstance* PropGI = GetGameInstance())
	{
		if (UT66PropSubsystem* PropSub = PropGI->GetSubsystem<UT66PropSubsystem>())
		{
			UT66GameInstance* T66GI_Props = GetT66GameInstance();
			const int32 PropSeed = (T66GI_Props && T66GI_Props->RunSeed != 0) ? T66GI_Props->RunSeed : FMath::Rand();
			if (bUsingMainMapTerrain)
			{
				const TArray<FName> MainMapPropRows = {
					FName(TEXT("Barn")),
					FName(TEXT("Haybell")),
					FName(TEXT("Windmill"))
				};
				PropSub->SpawnMainMapPropsForStage(GetWorld(), PropSeed, MainMapPropRows);
			}
			else
			{
				PropSub->SpawnPropsForStage(GetWorld(), PropSeed);
			}
		}
	}

	if (!bUsingMainMapTerrain)
	{
		SpawnStageEffectsForStage();
	}
	if (!bUsingMainMapTerrain)
	{
		SpawnTutorialIfNeeded();
	}

	if (bUsingMainMapTerrain)
	{
		SpawnTricksterAndCowardiceGate();
		SpawnBossForCurrentStage();
		TArray<AT66EnemyDirector*> ExistingEnemyDirectors;
		for (TActorIterator<AT66EnemyDirector> It(World); It; ++It)
		{
			if (AT66EnemyDirector* ExistingDirector = *It)
			{
				ExistingEnemyDirectors.Add(ExistingDirector);
			}
		}
		for (AT66EnemyDirector* ExistingDirector : ExistingEnemyDirectors)
		{
			if (ExistingDirector)
			{
				ExistingDirector->Destroy();
			}
		}
		bMainMapCombatStarted = false;
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->ResetStageTimerToFull();
				RunState->SetStageTimerActive(false);
			}
		}

		if (APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr)
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				UGameInstance* GI = GetGameInstance();
				T66_ApplyCombatIterationSetup(World, Pawn, GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr, GetT66GameInstance());
			}
		}

		ScheduleLightingRefresh();
		ScheduleOverlayHide();
		UE_LOG(LogTemp, Log, TEXT("T66GameMode - Main map terrain content spawned. Main-board combat and random interactables are waiting for the player to enter the board."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("T66GameMode - Phase 1 content spawned (structures + NPCs)."));

	// [GOLD] Phase 2 is now staggered across 4 ticks to eliminate the ~1 second hitch:
	//   Tick 1: Preload character visuals for this stage's mobs + boss (sync loads happen here, before combat)
	//   Tick 2: Spawn miasma boundary + miasma manager (wall visuals + dynamic materials)
	//   Tick 3: Spawn enemy director (triggers first wave ??? enemies are cheap now because visuals are cached)
	//   Tick 4: Spawn boss + boss gate
	if (World)
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				UGameInstance* GI = GetGameInstance();
				T66_ApplyCombatIterationSetup(World, Pawn, GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr, GetT66GameInstance());
			}
		}

		// --- Tick 1: Preload character visuals ---
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			const double PreloadStart = FPlatformTime::Seconds();

			UGameInstance* GI = GetGameInstance();
			UT66GameInstance* T66GI = GetT66GameInstance();
			UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
			UT66CharacterVisualSubsystem* Visuals = GI ? GI->GetSubsystem<UT66CharacterVisualSubsystem>() : nullptr;

			if (T66GI && RunState && Visuals)
			{
				const int32 StageNum = RunState->GetCurrentStage();
				FStageData StageData;
				if (T66GI->GetStageData(StageNum, StageData))
				{
					// Preload mob visuals (these are the ones that cause sync loads on first enemy spawn).
					if (!StageData.EnemyA.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyA);
					if (!StageData.EnemyB.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyB);
					if (!StageData.EnemyC.IsNone()) Visuals->PreloadCharacterVisual(StageData.EnemyC);
					// Preload shared boss visual (all stages use "Boss" mesh).
					Visuals->PreloadCharacterVisual(FName(TEXT("Boss")));

					UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Preload: pre-resolved visuals for stage %d (EnemyA=%s, EnemyB=%s, EnemyC=%s, Boss=%s) in %.1fms"),
						StageNum,
						*StageData.EnemyA.ToString(), *StageData.EnemyB.ToString(), *StageData.EnemyC.ToString(),
						*StageData.BossID.ToString(),
						(FPlatformTime::Seconds() - PreloadStart) * 1000.0);
				}
				// Also preload fallback mob IDs in case DT_Stages isn't fully wired.
				const FName FallbackA = FName(*FString::Printf(TEXT("Mob_Stage%02d_A"), RunState->GetCurrentStage()));
				const FName FallbackB = FName(*FString::Printf(TEXT("Mob_Stage%02d_B"), RunState->GetCurrentStage()));
				const FName FallbackC = FName(*FString::Printf(TEXT("Mob_Stage%02d_C"), RunState->GetCurrentStage()));
				Visuals->PreloadCharacterVisual(FallbackA);
				Visuals->PreloadCharacterVisual(FallbackB);
				Visuals->PreloadCharacterVisual(FallbackC);
			}

			UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Preload: total preload time %.1fms"), (FPlatformTime::Seconds() - PreloadStart) * 1000.0);

			// --- Tick 2: Spawn miasma systems ---
			if (UWorld* W2 = GetWorld())
			{
				W2->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					UWorld* W = GetWorld();
					if (W)
					{
						MiasmaManager = W->SpawnActor<AT66MiasmaManager>(AT66MiasmaManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
						W->SpawnActor<AT66MiasmaBoundary>(AT66MiasmaBoundary::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
					}
					UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Tick2: miasma systems spawned."));

					// --- Tick 3: Spawn enemy director ---
					if (UWorld* W3 = GetWorld())
					{
						W3->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
						{
							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							UWorld* W = GetWorld();
							if (W)
							{
								W->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
							}
							UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Tick3: enemy director spawned."));

							// --- Tick 4: Spawn boss + boss gate ---
							if (UWorld* W4 = GetWorld())
							{
								W4->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
								{
									SpawnBossForCurrentStage();
									SpawnBossGateIfNeeded();
									UE_LOG(LogTemp, Log, TEXT("[GOLD] Phase2-Tick4: boss + boss gate spawned. Phase 2 complete."));
									UE_LOG(LogTemp, Log, TEXT("T66GameMode - Phase 2 content spawned (combat systems + boss)."));
								}));
							}
						}));
					}
				}));
			}
		}));
	}

	ScheduleLightingRefresh();
	ScheduleOverlayHide();
}

void AT66GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind from long-lived RunState delegates (RunState is a GameInstanceSubsystem).
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->StageTimerChanged.RemoveDynamic(this, &AT66GameMode::HandleStageTimerChanged);
			RunState->DifficultyChanged.RemoveDynamic(this, &AT66GameMode::HandleDifficultyChanged);
		}
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			PS->OnSettingsChanged.RemoveDynamic(this, &AT66GameMode::HandleSettingsChanged);
		}
	}

	ActiveAsyncLoadHandles.Reset();
	PlayerCompanions.Reset();
	StageBoss = nullptr;
	IdolAltar = nullptr;
	BossBeaconActor = nullptr;
	BossBeaconUpdateAccumulator = 0.f;

	Super::EndPlay(EndPlayReason);
}

void AT66GameMode::SpawnTutorialIfNeeded()
{
	if (IsColiseumStage()) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;
	if (T66GI->bStageCatchUpPending) return;

	// v0 per your request: stage 1 always shows tutorial prompts.
	if (RunState->GetCurrentStage() != 1) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// The duplicated tutorial map should always run the tutorial flow.
	if (!T66IsStandaloneTutorialMap(World) && !bForceSpawnInTutorialArea) return;

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AT66TutorialManager* M = World->SpawnActor<AT66TutorialManager>(AT66TutorialManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);
	if (M)
	{
		SpawnedSetupActors.Add(M);
	}
}

void AT66GameMode::SpawnStageEffectsForStage()
{
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!RunState || !T66GI) return;
	if (T66GI->bStageCatchUpPending) return;

	const ET66Difficulty Difficulty = T66GI->SelectedDifficulty;
	const int32 StageNum = RunState->GetCurrentStage();

	const int32 RunSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	FRandomStream Rng(RunSeed + StageNum * 971 + 17);

	static constexpr float MainHalfExtent = 50000.f;
	static constexpr float SpawnZ = 40.f;
	static constexpr float SafeBubbleMargin = 350.f;

	int32 LavaPatchCount = 0;
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:       LavaPatchCount = 4;  break;
	case ET66Difficulty::Medium:     LavaPatchCount = 5;  break;
	case ET66Difficulty::Hard:       LavaPatchCount = 6;  break;
	case ET66Difficulty::VeryHard:   LavaPatchCount = 7;  break;
	case ET66Difficulty::Impossible: LavaPatchCount = 8;  break;
	case ET66Difficulty::Perdition:  LavaPatchCount = 9;  break;
	case ET66Difficulty::Final:      LavaPatchCount = 10; break;
	default:                         LavaPatchCount = 5;  break;
	}

	const int32 ShroomCount = (Difficulty == ET66Difficulty::Easy) ? 12 : 0;
	if (LavaPatchCount <= 0 && ShroomCount <= 0) return;

	struct FUsedStageEffectLoc
	{
		FVector Loc = FVector::ZeroVector;
		float ExclusionRadius = 0.f;
	};
	TArray<FUsedStageEffectLoc> UsedLocs;

	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{ -22727.f, 34091.f, ArenaHalf }, // Catch Up
			{      0.f, 34091.f, ArenaHalf },  // Coliseum
			{      0.f, 61364.f, TutorialArenaHalf }, // Tutorial
		};
		for (const FArena2D& A : Arenas)
		{
			if (FMath::Abs(L.X - A.X) <= (A.Half + ArenaMargin) && FMath::Abs(L.Y - A.Y) <= (A.Half + ArenaMargin))
			{
				return true;
			}
		}

		return false;
	};

	auto IsGoodLoc = [&](const FVector& L, float CandidateRadius) -> bool
	{
		if (IsInsideNoSpawnZone(L)) return false;
		for (const FUsedStageEffectLoc& Used : UsedLocs)
		{
			const float RequiredRadius = FMath::Max(CandidateRadius, Used.ExclusionRadius);
			if (FVector::DistSquared2D(L, Used.Loc) < (RequiredRadius * RequiredRadius)) return false;
		}
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			const AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin + CandidateRadius * 0.35f;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R)) return false;
		}
		return true;
	};

	struct FStageEffectSpawnHit
	{
		FVector Loc = FVector::ZeroVector;
		FVector Normal = FVector::UpVector;
		bool bFound = false;
	};

	auto FindSpawnLoc = [&](float CandidateRadius, int32 MaxTries) -> FStageEffectSpawnHit
	{
		const FVector Up = FVector::UpVector;
		for (int32 Try = 0; Try < MaxTries; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);
			FVector Normal = Up;

			FHitResult Hit;
			const FVector Start = Loc + FVector(0.f, 0.f, 2000.f);
			const FVector End = Loc - FVector(0.f, 0.f, 6000.f);
			if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				continue;
			}
			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}

			Loc = Hit.ImpactPoint;
			Normal = Hit.ImpactNormal.GetSafeNormal(1e-4f, Up);

			if (IsGoodLoc(Loc, CandidateRadius))
			{
				return { Loc, Normal, true };
			}
		}
		return {};
	};

	auto NoteUsedLoc = [&](const FVector& Loc, float Radius)
	{
		UsedLocs.Add({ Loc, Radius });
	};

	auto GetLavaDamagePerTick = [&]() -> int32
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Easy:       return 6;
		case ET66Difficulty::Medium:     return 8;
		case ET66Difficulty::Hard:       return 10;
		case ET66Difficulty::VeryHard:   return 12;
		case ET66Difficulty::Impossible: return 14;
		case ET66Difficulty::Perdition:  return 16;
		case ET66Difficulty::Final:      return 18;
		default:                         return 8;
		}
	};

	auto ConfigureLavaPatch = [&](AT66LavaPatch* Lava)
	{
		if (!Lava) return;

		FVector2D Flow(Rng.FRandRange(-1.f, 1.f), Rng.FRandRange(-1.f, 1.f));
		if (Flow.IsNearlyZero())
		{
			Flow = FVector2D(1.f, 0.f);
		}
		else
		{
			Flow.Normalize();
		}

		Lava->PatchSize = Rng.FRandRange(1400.f, 2400.f);
		Lava->HoverHeight = 1.5f;
		Lava->CollisionHeight = 160.f;
		Lava->TextureResolution = 64;
		Lava->AnimationFrames = 18;
		Lava->AnimationFPS = 12.f;
		Lava->WarpSpeed = Rng.FRandRange(0.90f, 1.25f);
		Lava->WarpIntensity = Rng.FRandRange(0.10f, 0.15f);
		Lava->WarpCloseness = Rng.FRandRange(1.80f, 2.40f);
		Lava->FlowDir = Flow;
		Lava->FlowSpeed = Rng.FRandRange(0.12f, 0.26f);
		Lava->UVScale = Rng.FRandRange(3.60f, 5.40f);
		Lava->CellDensity = Rng.FRandRange(5.20f, 7.40f);
		Lava->EdgeContrast = Rng.FRandRange(6.40f, 8.80f);
		Lava->Brightness = Rng.FRandRange(2.10f, 2.90f);
		Lava->PatternOffset = FVector2D(Rng.FRandRange(-12.f, 12.f), Rng.FRandRange(-12.f, 12.f));
		Lava->DamagePerTick = GetLavaDamagePerTick();
		Lava->DamageIntervalSeconds = 0.45f;
		Lava->bDamageHero = true;
	};

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 SpawnedLavaCount = 0;
	int32 SpawnedShroomCount = 0;

	static constexpr float LavaExclusionRadius = 1800.f;
	for (int32 i = 0; i < LavaPatchCount; ++i)
	{
		const FStageEffectSpawnHit Th = FindSpawnLoc(LavaExclusionRadius, 120);
		if (!Th.bFound)
		{
			continue;
		}

		const FRotator LavaRot(0.f, Rng.FRandRange(0.f, 360.f), 0.f);
		AT66LavaPatch* Lava = World->SpawnActorDeferred<AT66LavaPatch>(
			AT66LavaPatch::StaticClass(),
			FTransform(LavaRot, Th.Loc),
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Lava)
		{
			ConfigureLavaPatch(Lava);
			Lava->FinishSpawning(FTransform(LavaRot, Th.Loc));
			NoteUsedLoc(Th.Loc, LavaExclusionRadius);
			++SpawnedLavaCount;
		}
	}

	static constexpr float ShroomExclusionRadius = 420.f;
	for (int32 i = 0; i < ShroomCount; ++i)
	{
		const FStageEffectSpawnHit Th = FindSpawnLoc(ShroomExclusionRadius, 80);
		if (!Th.bFound)
		{
			continue;
		}

		AT66Shroom* Shroom = World->SpawnActor<AT66Shroom>(AT66Shroom::StaticClass(), Th.Loc, FRotator::ZeroRotator, P);
		if (Shroom)
		{
			NoteUsedLoc(Th.Loc, ShroomExclusionRadius);
			++SpawnedShroomCount;
		}
	}

	UE_LOG(LogTemp, Log,
		TEXT("[StageEffects] Spawned %d lava patches and %d shrooms for stage %d (diff=%d)."),
		SpawnedLavaCount,
		SpawnedShroomCount,
		StageNum,
		static_cast<int32>(Difficulty));
}

void AT66GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MaintainPlayerTerrainSafety();
	TryActivateMainMapCombat();

	BossBeaconUpdateAccumulator += DeltaTime;
	if (BossBeaconUpdateAccumulator >= 0.10f)
	{
		BossBeaconUpdateAccumulator = 0.f;
		UpdateBossBeaconTransform(true);
	}

	// Frame-level lag: log when a frame exceeded budget (e.g. >20ms).
	if (DeltaTime > 0.02f)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UT66LagTrackerSubsystem* Lag = GI->GetSubsystem<UT66LagTrackerSubsystem>())
			{
				if (Lag->IsEnabled())
				{
					UE_LOG(LogTemp, Warning, TEXT("[LAG] Frame: %.2fms (%.1f FPS)"), DeltaTime * 1000.f, 1.f / FMath::Max(0.001f, DeltaTime));
				}
			}
		}
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->TickStageTimer(DeltaTime);
			RunState->TickSpeedRunTimer(DeltaTime);
			RunState->TickHeroTimers(DeltaTime);

			// Skill Rating: time is driven here; damage is an input event from RunState.
			if (UT66SkillRatingSubsystem* Skill = GI->GetSubsystem<UT66SkillRatingSubsystem>())
			{
				// Track only during combat time (stage timer active) and not during last-stand invulnerability.
				Skill->SetTrackingActive(RunState->GetStageTimerActive() && !RunState->IsInLastStand());
				Skill->TickSkillRating(DeltaTime);
			}

			// Robust: if the timer is already active (even if we missed the delegate),
			// try spawning the LoanShark when pending.
			if (!IsColiseumStage() && !IsLabLevel() && RunState->GetStageTimerActive())
			{
				TrySpawnLoanSharkIfNeeded();
			}

			// Despawn loan shark immediately once debt is paid.
			if (LoanShark && RunState->GetCurrentDebt() <= 0)
			{
				LoanShark->Destroy();
				LoanShark = nullptr;
			}
		}
	}
}

void AT66GameMode::HandleStageTimerChanged()
{
	if (IsLabLevel()) return;
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	FLagScopedScope LagScope(GetWorld(), TEXT("HandleStageTimerChanged (Miasma+LoanShark)"));

	// Perf: miasma updates are event-driven (StageTimerChanged broadcasts at most once per second).
	if (MiasmaManager)
	{
		MiasmaManager->UpdateFromRunState();
	}

	if (!IsColiseumStage() && RunState->GetStageTimerActive())
	{
		TrySpawnLoanSharkIfNeeded();
	}
}

void AT66GameMode::HandleDifficultyChanged()
{
	if (IsLabLevel()) return;
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const int32 Stage = RunState->GetCurrentStage();
	const float Scalar = RunState->GetDifficultyScalar();
	UT66ActorRegistrySubsystem* Registry = GetWorld() ? GetWorld()->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
	const TArray<TWeakObjectPtr<AT66EnemyBase>>& Enemies = Registry ? Registry->GetEnemies() : TArray<TWeakObjectPtr<AT66EnemyBase>>();
	for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Enemies)
	{
		if (AT66EnemyBase* E = WeakEnemy.Get())
		{
			E->ApplyStageScaling(Stage);
			E->ApplyDifficultyScalar(Scalar);
		}
	}

	// Bosses: scale HP + projectile damage (count unchanged).
	const TArray<TWeakObjectPtr<AT66BossBase>>& Bosses = Registry ? Registry->GetBosses() : TArray<TWeakObjectPtr<AT66BossBase>>();
	for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Bosses)
	{
		if (AT66BossBase* B = WeakBoss.Get())
		{
			B->ApplyDifficultyScalar(Scalar);
		}
	}
}

void AT66GameMode::TrySpawnLoanSharkIfNeeded()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	// Only spawn if the previous stage was exited with debt.
	if (!RunState->GetLoanSharkPending())
	{
		return;
	}
	if (RunState->GetCurrentDebt() <= 0)
	{
		RunState->SetLoanSharkPending(false);
		return;
	}
	if (LoanShark)
	{
		RunState->SetLoanSharkPending(false);
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;
	UWorld* World = GetWorld();
	if (!World) return;

	FVector PlayerLoc = PlayerPawn->GetActorLocation();
	FVector SpawnLoc = PlayerLoc + FVector(-1200.f, 0.f, 0.f);

	// Try a few times to avoid spawning inside safe zones
	for (int32 Try = 0; Try < 10; ++Try)
	{
		float Angle = FMath::RandRange(0.f, 2.f * PI);
		float Dist = FMath::RandRange(900.f, 1400.f);
		FVector Offset(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);
		SpawnLoc = PlayerLoc + Offset;

		bool bInSafe = false;
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius();
			if (FVector::DistSquared2D(SpawnLoc, NPC->GetActorLocation()) < (R * R))
			{
				bInSafe = true;
				break;
			}
		}
		if (!bInSafe)
		{
			break;
		}
	}

	// Trace down for ground
	FHitResult Hit;
	FVector Start = SpawnLoc + FVector(0.f, 0.f, 500.f);
	FVector End = SpawnLoc - FVector(0.f, 0.f, 1000.f);
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
	{
		SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 90.f);
	}
	else
	{
		SpawnLoc.Z = PlayerLoc.Z;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	LoanShark = World->SpawnActor<AT66LoanShark>(AT66LoanShark::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	RunState->SetLoanSharkPending(false);
}

void AT66GameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
	SpawnCompanionForPlayer(NewPlayer);
	// Start gate is spawned in SpawnLevelContentAfterLandscapeReady (after floor exists) so its ground trace hits the floor.

	UT66GameInstance* GI = GetT66GameInstance();
	APawn* Pawn = NewPlayer ? NewPlayer->GetPawn() : nullptr;
	if (GI && Pawn && GI->bApplyLoadedTransform)
	{
		Pawn->SetActorTransform(GI->PendingLoadedTransform);
		GI->bApplyLoadedTransform = false;
		GI->PendingLoadedTransform = FTransform();
	}

	if (GI && GI->bLoadAsPreview && NewPlayer)
	{
		if (AT66PlayerController* PC = Cast<AT66PlayerController>(NewPlayer))
		{
			PC->SetPause(true);
			PC->ShowLoadPreviewOverlay();
		}
	}

	MaintainPlayerTerrainSafety();

	if (IsLabLevel())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, WeakPlayer = TWeakObjectPtr<AController>(NewPlayer)]()
			{
				AController* Player = WeakPlayer.Get();
				APawn* SpawnedPawn = Player ? Player->GetPawn() : nullptr;
				UGameInstance* GIForSetup = GetGameInstance();
				T66_ApplyCombatIterationSetup(GetWorld(), SpawnedPawn, GIForSetup ? GIForSetup->GetSubsystem<UT66RunStateSubsystem>() : nullptr, GetT66GameInstance());
			}));
		}
	}
}

void AT66GameMode::SpawnBossGateIfNeeded()
{
	if (IsColiseumStage()) return;
	if (BossGate) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Trigger right at the boss-area threshold. The visible pillars are hidden so the fight starts on entry.
	FVector BossGateLoc = T66GameplayLayout::GetBossGateLocation();
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, BossGateLoc + FVector(0.f, 0.f, 3000.f), BossGateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
	{
		BossGateLoc.Z = Hit.ImpactPoint.Z;
	}
	BossGate = World->SpawnActor<AT66BossGate>(AT66BossGate::StaticClass(), BossGateLoc, FRotator::ZeroRotator, SpawnParams);
	if (BossGate)
	{
		BossGate->TriggerDistance2D = 220.f;
		if (BossGate->TriggerBox)
		{
			BossGate->TriggerBox->SetBoxExtent(FVector(120.f, T66GameplayLayout::CorridorHalfHeightY * 0.92f, 220.f));
		}
		if (BossGate->PoleLeft)
		{
			BossGate->PoleLeft->SetVisibility(false, true);
			BossGate->PoleLeft->SetHiddenInGame(true, true);
		}
		if (BossGate->PoleRight)
		{
			BossGate->PoleRight->SetVisibility(false, true);
			BossGate->PoleRight->SetHiddenInGame(true, true);
		}
	}
}

bool AT66GameMode::IsColiseumStage() const
{
	if (T66IsStandaloneColiseumMap(GetWorld()))
	{
		return true;
	}

	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->bForceColiseumMode;
	}
	return false;
}

bool AT66GameMode::IsLabLevel() const
{
	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->bIsLabLevel;
	}
	return false;
}

bool AT66GameMode::IsDemoMapForTribulation() const
{
	UWorld* World = GetWorld();
	if (!World) return false;
	FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
	return MapName.Equals(UT66GameInstance::GetDemoMapLevelNameForTribulation().ToString(), ESearchCase::IgnoreCase);
}

void AT66GameMode::SpawnAllOwedBossesInColiseum()
{
	// Spawn ALL owed bosses at once. No tick polling: each boss death calls HandleBossDefeatedAtLocation.
	UWorld* World = GetWorld();
	if (!World) return;
	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;

	if (bColiseumExitGateSpawned)
	{
		// Coliseum already cleared this session.
		return;
	}

	const TArray<FName>& Owed = RunState->GetOwedBossIDs();
	if (Owed.Num() <= 0)
	{
		// Nothing owed: exit gate immediately.
		bColiseumExitGateSpawned = true;
		SpawnStageGateAtLocation(ColiseumCenter);
		return;
	}

	// Preload any BossClass soft references asynchronously to avoid hitches.
	// If a load fails/misses, we still spawn with AT66BossBase as a fallback.
	if (!bColiseumBossesAsyncLoadAttempted && !bColiseumBossesAsyncLoadInFlight)
	{
		TArray<FSoftObjectPath> PathsToLoad;
		PathsToLoad.Reserve(Owed.Num());
		for (const FName& BossID : Owed)
		{
			if (BossID.IsNone()) continue;

			FBossData BossData;
			if (T66GI->GetBossData(BossID, BossData))
			{
				if (!BossData.BossClass.IsNull())
				{
					// Only request async load if the class isn't already resident.
					if (!BossData.BossClass.Get())
					{
						PathsToLoad.AddUnique(BossData.BossClass.ToSoftObjectPath());
					}
				}
			}
		}

		if (PathsToLoad.Num() > 0)
		{
			bColiseumBossesAsyncLoadAttempted = true;
			bColiseumBossesAsyncLoadInFlight = true;

			TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
				PathsToLoad,
				FStreamableDelegate::CreateWeakLambda(this, [this]()
				{
					bColiseumBossesAsyncLoadInFlight = false;
					SpawnAllOwedBossesInColiseum();
				}));
			if (Handle.IsValid())
			{
				ActiveAsyncLoadHandles.Add(Handle);
				return;
			}
			// If async load couldn't start, fall through to immediate spawning (with fallbacks).
			bColiseumBossesAsyncLoadInFlight = false;
		}
	}

	// Reset and spawn.
	ColiseumBossesRemaining = 0;

	// Simple ring layout around ColiseumCenter.
	const int32 N = FMath::Clamp(Owed.Num(), 1, 12);
	const float Radius = 900.f + (static_cast<float>(N) * 55.f);
	const float AngleStep = (2.f * PI) / static_cast<float>(N);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < Owed.Num(); ++i)
	{
		const FName BossID = Owed[i];
		if (BossID.IsNone()) continue;

		FBossData BossData;
		if (!T66GI->GetBossData(BossID, BossData))
		{
			// Fallback boss tuning if DT row missing.
			BossData.BossID = BossID;
			BossData.MaxHP = 6000;
			BossData.AwakenDistance = 999999.f;
			BossData.MoveSpeed = 380.f;
			BossData.FireIntervalSeconds = 1.6f;
			BossData.ProjectileSpeed = 1100.f;
			BossData.ProjectileDamageHearts = 2;
			BossData.PlaceholderColor = FLinearColor(0.85f, 0.15f, 0.15f, 1.f);
		}

		UClass* BossClass = AT66BossBase::StaticClass();
		if (!BossData.BossClass.IsNull())
		{
			if (UClass* Loaded = BossData.BossClass.Get())
			{
				if (Loaded->IsChildOf(AT66BossBase::StaticClass()))
				{
					BossClass = Loaded;
				}
			}
		}

		const float A = static_cast<float>(i) * AngleStep;
		FVector SpawnLoc = ColiseumCenter + FVector(FMath::Cos(A) * Radius, FMath::Sin(A) * Radius, 0.f);

		// Trace down to ground.
		FHitResult Hit;
		const FVector Start = SpawnLoc + FVector(0.f, 0.f, 4000.f);
		const FVector End = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			SpawnLoc = Hit.ImpactPoint + FVector(0.f, 0.f, 200.f);
		}

		AActor* Spawned = World->SpawnActor<AActor>(BossClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (AT66BossBase* Boss = Cast<AT66BossBase>(Spawned))
		{
			Boss->InitializeBoss(BossData);
			Boss->ForceAwaken();
			ColiseumBossesRemaining++;
		}
	}

	// If something went wrong and none spawned, still provide an exit.
	if (ColiseumBossesRemaining <= 0)
	{
		bColiseumExitGateSpawned = true;
		RunState->ClearOwedBosses();
		SpawnStageGateAtLocation(ColiseumCenter);
	}
}


void AT66GameMode::SpawnTricksterAndCowardiceGate()
{
	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;

	const int32 StageNum = RunState->GetCurrentStage();
	// Only on normal stages (NOT ending in 5 or 0) => stage % 5 != 0
	if ((StageNum % 5) == 0) return;

	UWorld* World = GetWorld();
	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector GateLoc = T66GameplayLayout::GetCowardiceGateLocation();
	FVector TricksterLoc = T66GameplayLayout::GetTricksterLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (CowardiceGate)
	{
		CowardiceGate->Destroy();
		CowardiceGate = nullptr;
	}
	if (TricksterNPC)
	{
		TricksterNPC->Destroy();
		TricksterNPC = nullptr;
	}

	if (T66UsesMainMapTerrainStage(World) && !MainMapBossAnchorSurfaceLocation.IsNearlyZero())
	{
		const T66MainMapTerrain::FSettings MainMapSettings = T66MainMapTerrain::MakeSettings(T66BuildMainMapPreset(T66GI));
		const FVector SideOffset(MainMapSettings.CellSize * 0.28f, 0.f, 0.f);
		GateLoc = MainMapBossAnchorSurfaceLocation;
		TricksterLoc = MainMapBossAnchorSurfaceLocation + SideOffset;
		SpawnRotation = FRotator(0.f, 180.f, 0.f);
	}
	else
	{
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, GateLoc + FVector(0.f, 0.f, 3000.f), GateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
		{
			GateLoc.Z = Hit.ImpactPoint.Z;
		}
		if (World->LineTraceSingleByChannel(Hit, TricksterLoc + FVector(0.f, 0.f, 3000.f), TricksterLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
		{
			TricksterLoc.Z = Hit.ImpactPoint.Z;
		}
	}

	CowardiceGate = World->SpawnActor<AT66CowardiceGate>(AT66CowardiceGate::StaticClass(), GateLoc, SpawnRotation, SpawnParams);
	TricksterNPC = World->SpawnActor<AT66TricksterNPC>(AT66TricksterNPC::StaticClass(), TricksterLoc, SpawnRotation, SpawnParams);
	if (TricksterNPC)
	{
		TricksterNPC->ApplyVisuals();
	}
}

bool AT66GameMode::TryGetMainMapStartAxes(FVector& OutCenter, FVector& OutInwardDirection, FVector& OutSideDirection, float& OutCellSize) const
{
	if (!bHasMainMapSpawnSurfaceLocation || MainMapStartPathSurfaceLocation.IsNearlyZero() || MainMapStartAnchorSurfaceLocation.IsNearlyZero())
	{
		return false;
	}

	const FVector Center = MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
		? MainMapSpawnSurfaceLocation
		: MainMapStartAreaCenterSurfaceLocation;
	const FVector Inward2D = (MainMapStartAnchorSurfaceLocation - MainMapStartPathSurfaceLocation).GetSafeNormal2D();
	if (Inward2D.IsNearlyZero())
	{
		return false;
	}

	const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
	const T66MainMapTerrain::FSettings Settings = T66MainMapTerrain::MakeSettings(Preset);
	if (Settings.CellSize <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	OutCenter = Center;
	OutInwardDirection = FVector(Inward2D.X, Inward2D.Y, 0.f);
	OutSideDirection = FVector(-Inward2D.Y, Inward2D.X, 0.f);
	OutCellSize = Settings.CellSize;
	return true;
}

bool AT66GameMode::TryGetMainMapStartPlacementLocation(float SideCells, float InwardCells, FVector& OutLocation) const
{
	FVector Center = FVector::ZeroVector;
	FVector InwardDirection = FVector::ZeroVector;
	FVector SideDirection = FVector::ZeroVector;
	float CellSize = 0.f;
	if (!TryGetMainMapStartAxes(Center, InwardDirection, SideDirection, CellSize))
	{
		return false;
	}

	const FVector DesiredLocation = Center + (SideDirection * (SideCells * CellSize)) + (InwardDirection * (InwardCells * CellSize));
	OutLocation = DesiredLocation;

	UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
	const T66MainMapTerrain::FSettings Settings = T66MainMapTerrain::MakeSettings(Preset);
	const float TraceStartZ = T66MainMapTerrain::GetTraceZ(Preset);
	const float TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(Preset) - Settings.StepHeight;

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(
		Hit,
		FVector(DesiredLocation.X, DesiredLocation.Y, TraceStartZ),
		FVector(DesiredLocation.X, DesiredLocation.Y, TraceEndZ),
		ECC_WorldStatic))
	{
		OutLocation = Hit.ImpactPoint;
	}
	else
	{
		OutLocation.Z = Center.Z;
	}

	return true;
}

bool AT66GameMode::TryFindRandomMainMapSurfaceLocation(int32 SeedOffset, FVector& OutLocation, float ExtraSafeBubbleMargin) const
{
	UWorld* World = GetWorld();
	if (!World || !T66UsesMainMapTerrainStage(World))
	{
		return false;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return false;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const FT66MapPreset MainMapPreset = T66BuildMainMapPreset(T66GI);
	const T66MainMapTerrain::FSettings MainMapSettings = T66MainMapTerrain::MakeSettings(MainMapPreset);
	const float MainHalfExtent = FMath::Max(0.0f, MainMapSettings.HalfExtent - MainMapSettings.CellSize * 1.25f);
	const float TraceStartZ = T66MainMapTerrain::GetTraceZ(MainMapPreset);
	const float TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight;
	const float RoomReserveRadius = MainMapSettings.CellSize * 1.70f;
	const float CorridorReserveRadius = MainMapSettings.CellSize * 0.80f;
	const float SafeBubbleMargin = 250.f + ExtraSafeBubbleMargin;

	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
	FRandomStream Rng(RunSeed + StageNum * 1337 + SeedOffset);

	auto IsInsideReservedZone = [&](const FVector& Location) -> bool
	{
		if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
		{
			return true;
		}
		if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapBossAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
		{
			return true;
		}
		if (!MainMapStartPathSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartPathSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapStartAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero()
			&& FVector::DistSquared2D(Location, MainMapBossAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
		{
			return true;
		}
		return false;
	};

	UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>();
	for (int32 Try = 0; Try < 60; ++Try)
	{
		const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
		const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);

		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(
			Hit,
			FVector(X, Y, TraceStartZ),
			FVector(X, Y, TraceEndZ),
			ECC_WorldStatic))
		{
			continue;
		}
		if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
		{
			continue;
		}

		const FVector Candidate = Hit.ImpactPoint;
		if (IsInsideReservedZone(Candidate))
		{
			continue;
		}

		bool bBlockedByNPC = false;
		if (Registry)
		{
			for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : Registry->GetNPCs())
			{
				const AT66HouseNPCBase* NPC = WeakNPC.Get();
				if (!NPC)
				{
					continue;
				}
				const float Clearance = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
				if (FVector::DistSquared2D(Candidate, NPC->GetActorLocation()) < FMath::Square(Clearance))
				{
					bBlockedByNPC = true;
					break;
				}
			}
		}

		if (bBlockedByNPC)
		{
			continue;
		}

		OutLocation = Candidate;
		return true;
	}

	return false;
}

void AT66GameMode::TryActivateMainMapCombat()
{
	UWorld* World = GetWorld();
	if (!World || !T66UsesMainMapTerrainStage(World) || !bTerrainCollisionReady || bMainMapCombatStarted)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	AController* PlayerController = World->GetFirstPlayerController();
	APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!PlayerPawn || MainMapStartAnchorSurfaceLocation.IsNearlyZero() || MainMapStartPathSurfaceLocation.IsNearlyZero())
	{
		return;
	}

	const FVector2D EntryMidpoint = FVector2D(
		(MainMapStartAnchorSurfaceLocation.X + MainMapStartPathSurfaceLocation.X) * 0.5f,
		(MainMapStartAnchorSurfaceLocation.Y + MainMapStartPathSurfaceLocation.Y) * 0.5f);
	const FVector2D EntryNormal = FVector2D(
		MainMapStartAnchorSurfaceLocation.X - MainMapStartPathSurfaceLocation.X,
		MainMapStartAnchorSurfaceLocation.Y - MainMapStartPathSurfaceLocation.Y).GetSafeNormal();
	if (EntryNormal.IsNearlyZero())
	{
		return;
	}

	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	const float SignedDistance = FVector2D::DotProduct(FVector2D(PlayerLocation.X, PlayerLocation.Y) - EntryMidpoint, EntryNormal);
	if (SignedDistance < 0.f)
	{
		return;
	}

	RunState->ResetStageTimerToFull();
	SpawnWorldInteractablesForStage();
	RunState->SetStageTimerActive(true);

	bool bHasEnemyDirector = false;
	for (TActorIterator<AT66EnemyDirector> It(World); It; ++It)
	{
		bHasEnemyDirector = true;
		break;
	}

	if (!bHasEnemyDirector)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66EnemyDirector>(AT66EnemyDirector::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	bMainMapCombatStarted = true;
	UE_LOG(LogTemp, Log, TEXT("T66GameMode - Main map combat activated after player crossed the start threshold."));
}

void AT66GameMode::ResetColiseumState()
{
	ColiseumBossesRemaining = 0;
	bColiseumExitGateSpawned = false;
}

void AT66GameMode::HandleBossDefeated(AT66BossBase* Boss)
{
	UWorld* World = GetWorld();
	if (!World) return;
	const FVector Location = Boss ? Boss->GetActorLocation() : FVector::ZeroVector;
	if (Boss && StageBoss.Get() == Boss)
	{
		StageBoss = nullptr;
		DestroyBossBeacon();
	}
	UGameInstance* GI = World->GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

	// Award boss score (scaled by current difficulty).
	if (RunState && Boss)
	{
		const float Scalar = RunState->GetDifficultyScalar();
		const int32 AwardPoints = FMath::Max(0, FMath::RoundToInt(static_cast<float>(Boss->GetPointValue()) * Scalar));
		if (AwardPoints > 0)
		{
			RunState->AddScore(AwardPoints);
		}
	}

	// Lab unlock + achievement: mark this boss as unlocked for The Lab and notify boss killed.
	if (GI)
	{
		if (UT66AchievementsSubsystem* Achieve = GI->GetSubsystem<UT66AchievementsSubsystem>())
		{
			Achieve->NotifyBossKilled(1);
			if (Boss && !Boss->BossID.IsNone())
			{
				Achieve->AddLabUnlockedEnemy(Boss->BossID);
			}
		}
	}

	// Power Coupons: 1 per boss kill (stage boss or Coliseum).
	if (RunState)
	{
		RunState->AddPowerCrystalsEarnedThisRun(1);
	}

	if (IsColiseumStage())
	{
		// Coliseum: keep the timer/miasma running; open the exit gate only when ALL owed bosses are dead.
		if (RunState)
		{
			RunState->SetBossInactive();
		}

		if (ColiseumBossesRemaining > 0)
		{
			ColiseumBossesRemaining = FMath::Max(0, ColiseumBossesRemaining - 1);
		}

		if (!bColiseumExitGateSpawned && ColiseumBossesRemaining <= 0)
		{
			bColiseumExitGateSpawned = true;
			if (RunState)
			{
				RunState->ClearOwedBosses();
			}
			SpawnStageGateAtLocation(Location);
		}
		return;
	}

	if (RunState)
	{
		RunState->SetBossInactive();
		RunState->SetStageTimerActive(false);
	}

	// First-time boss defeat unlock => spawn recruitable companion (stages 1..60 excluding 0/5).
	if (RunState)
	{
		const int32 StageNum = RunState->GetCurrentStage();
		const FName CompanionToUnlock = T66_CompanionIDForStage(StageNum);
		if (!CompanionToUnlock.IsNone())
		{
			if (UT66CompanionUnlockSubsystem* Unlocks = GI ? GI->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr)
			{
				const bool bNewlyUnlocked = Unlocks->UnlockCompanion(CompanionToUnlock);
				if (bNewlyUnlocked)
				{
					if (UT66GameInstance* T66GI = GetT66GameInstance())
					{
						FCompanionData Data;
						if (T66GI->GetCompanionData(CompanionToUnlock, Data))
						{
							FActorSpawnParameters SpawnParams;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
							if (AT66RecruitableCompanion* Recruit = World->SpawnActor<AT66RecruitableCompanion>(AT66RecruitableCompanion::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams))
							{
								Recruit->InitializeRecruit(Data);
							}
						}
					}
				}
			}
		}
	}

	// Normal stage: boss dead => miasma disappears and Stage Gate appears.
	ClearMiasma();
	SpawnStageGateAtLocation(Location);

	if (RunState)
	{
		const int32 ClearedStage = RunState->GetCurrentStage();
		UT66IdolManagerSubsystem* IdolManager = GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
		if (IdolManager && IdolManager->ShouldSpawnBossClearAltarForClearedStage(ClearedStage))
		{
			SpawnIdolAltarAtLocation(Location);
		}
	}
}

bool AT66GameMode::SwapCompanionForPlayer(AController* Player, FName NewCompanionID)
{
	if (!Player) return false;
	if (NewCompanionID.IsNone()) return false;

	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI) return false;

	// If this companion is already selected, treat as handled (no swap).
	if (GI->SelectedCompanionID == NewCompanionID)
	{
		return true;
	}

	// Validate new companion exists in DT_Companions (prevents setting an invalid ID).
	{
		FCompanionData NewData;
		if (!GI->GetCompanionData(NewCompanionID, NewData))
		{
			return false;
		}
	}

	UWorld* World = GetWorld();
	if (!World) return false;

	// If a companion is currently following, spawn a recruitable version where it currently is.
	FName OldCompanionID = NAME_None;
	FVector OldCompanionLoc = FVector::ZeroVector;
	if (TWeakObjectPtr<AT66CompanionBase>* Existing = PlayerCompanions.Find(Player))
	{
		if (AT66CompanionBase* ExistingComp = Existing->Get())
		{
			OldCompanionID = ExistingComp->CompanionID;
			OldCompanionLoc = ExistingComp->GetActorLocation();
			ExistingComp->Destroy();
		}
		PlayerCompanions.Remove(Player);
	}

	if (!OldCompanionID.IsNone())
	{
		FCompanionData OldData;
		if (GI->GetCompanionData(OldCompanionID, OldData))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66RecruitableCompanion* OldRecruit = World->SpawnActor<AT66RecruitableCompanion>(AT66RecruitableCompanion::StaticClass(), OldCompanionLoc, FRotator::ZeroRotator, SpawnParams))
			{
				OldRecruit->InitializeRecruit(OldData);
			}
		}
	}

	// Persist for the rest of the run (stage transitions read SelectedCompanionID).
	GI->SelectedCompanionID = NewCompanionID;

	SpawnCompanionForPlayer(Player);
	return true;
}

void AT66GameMode::ClearMiasma()
{
	if (MiasmaManager)
	{
		MiasmaManager->ClearAllMiasma();
	}
}

void AT66GameMode::SpawnCompanionForPlayer(AController* Player)
{
	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI || GI->SelectedCompanionID.IsNone()) return;

	FCompanionData CompanionData;
	if (!GI->GetCompanionData(GI->SelectedCompanionID, CompanionData)) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APawn* HeroPawn = Player ? Player->GetPawn() : nullptr;
	if (!HeroPawn) return;

	// Prevent duplicate companions on respawn.
	if (Player)
	{
		if (TWeakObjectPtr<AT66CompanionBase>* Existing = PlayerCompanions.Find(Player))
		{
			if (AT66CompanionBase* ExistingComp = Existing->Get())
			{
				ExistingComp->Destroy();
			}
			PlayerCompanions.Remove(Player);
		}
	}

	UClass* CompanionClass = AT66CompanionBase::StaticClass();
	const bool bWantsSpecificClass = !CompanionData.CompanionClass.IsNull();
	const bool bHasLoadedClass = bWantsSpecificClass && (CompanionData.CompanionClass.Get() != nullptr);
	if (bHasLoadedClass)
	{
		if (UClass* Loaded = CompanionData.CompanionClass.Get())
		{
			if (Loaded->IsChildOf(AT66CompanionBase::StaticClass()))
			{
				CompanionClass = Loaded;
			}
		}
	}

	FVector SpawnLoc = HeroPawn->GetActorLocation() + FVector(-150.f, 100.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FName CompanionSkinID = FName(TEXT("Default"));
	if (UGameInstance* GII = GetGameInstance())
	{
		if (UT66AchievementsSubsystem* Ach = GII->GetSubsystem<UT66AchievementsSubsystem>())
		{
			CompanionSkinID = Ach->GetEquippedCompanionSkinID(GI->SelectedCompanionID);
		}
	}

	AT66CompanionBase* Companion = World->SpawnActor<AT66CompanionBase>(CompanionClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Companion)
	{
		Companion->InitializeCompanion(CompanionData, CompanionSkinID);
		Companion->SetPreviewMode(false); // gameplay: follow hero
		if (Player)
		{
			PlayerCompanions.Add(Player, Companion);
		}
		// Snap companion to ground so it doesn't float.
		FHitResult Hit;
		const FVector Start = Companion->GetActorLocation() + FVector(0.f, 0.f, 2000.f);
		const FVector End = Companion->GetActorLocation() - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			Companion->SetActorLocation(Hit.ImpactPoint, false, nullptr, ETeleportType::TeleportPhysics);
		}
		UE_LOG(LogTemp, Log, TEXT("Spawned companion: %s"), *CompanionData.DisplayName.ToString());
	}

	// If the companion class is a soft reference and isn't loaded yet, load asynchronously and replace.
	if (bWantsSpecificClass && !bHasLoadedClass)
	{
		const FSoftObjectPath ClassPath = CompanionData.CompanionClass.ToSoftObjectPath();
		const TWeakObjectPtr<AController> WeakPlayer(Player);
		const TWeakObjectPtr<AT66CompanionBase> WeakExisting(Companion);
		const FCompanionData CompanionDataCopy = CompanionData;

		TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
			ClassPath,
			FStreamableDelegate::CreateWeakLambda(this, [this, WeakPlayer, WeakExisting, CompanionDataCopy]()
			{
				AController* PlayerCtrl = WeakPlayer.Get();
				if (!PlayerCtrl) return;

				UWorld* World2 = GetWorld();
				if (!World2) return;

				UClass* Loaded = CompanionDataCopy.CompanionClass.Get();
				if (!Loaded || !Loaded->IsChildOf(AT66CompanionBase::StaticClass()))
				{
					return;
				}

				AT66CompanionBase* ExistingComp = WeakExisting.Get();
				// If the existing companion is already the correct class (or was destroyed), do nothing.
				if (ExistingComp && ExistingComp->GetClass() == Loaded)
				{
					return;
				}

				// Remove the old companion if it's still around.
				if (TWeakObjectPtr<AT66CompanionBase>* Current = PlayerCompanions.Find(PlayerCtrl))
				{
					if (AT66CompanionBase* C = Current->Get())
					{
						C->Destroy();
					}
					PlayerCompanions.Remove(PlayerCtrl);
				}

				APawn* HeroPawn2 = PlayerCtrl->GetPawn();
				if (!HeroPawn2) return;

				FVector SpawnLoc2 = HeroPawn2->GetActorLocation() + FVector(-150.f, 100.f, 0.f);
				FActorSpawnParameters SpawnParams2;
				SpawnParams2.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				FName AsyncSkinID = FName(TEXT("Default"));
				if (UGameInstance* GII = GetGameInstance())
				{
					if (UT66AchievementsSubsystem* Ach = GII->GetSubsystem<UT66AchievementsSubsystem>())
					{
						AsyncSkinID = Ach->GetEquippedCompanionSkinID(CompanionDataCopy.CompanionID);
					}
				}
				AT66CompanionBase* NewComp = World2->SpawnActor<AT66CompanionBase>(Loaded, SpawnLoc2, FRotator::ZeroRotator, SpawnParams2);
				if (NewComp)
				{
					NewComp->InitializeCompanion(CompanionDataCopy, AsyncSkinID);
					NewComp->SetPreviewMode(false);
					PlayerCompanions.Add(PlayerCtrl, NewComp);
				}
			}));
		if (Handle.IsValid())
		{
			ActiveAsyncLoadHandles.Add(Handle);
		}
	}
}

void AT66GameMode::SpawnVendorForPlayer(AController* Player)
{
	APawn* HeroPawn = Player ? Player->GetPawn() : nullptr;
	if (!HeroPawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FVector SpawnLoc = HeroPawn->GetActorLocation() + FVector(300.f, 0.f, 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66VendorNPC* Vendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Vendor)
	{
		UE_LOG(LogTemp, Log, TEXT("Spawned vendor NPC near hero"));
	}
}

void AT66GameMode::SpawnCornerHousesAndNPCs()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	if (bUsingMainMapTerrain)
	{
		FVector SaintLoc = FVector::ZeroVector;
		if (!TryFindRandomMainMapSurfaceLocation(3101, SaintLoc, 150.f))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AActor* SpawnedNPC = World->SpawnActor<AActor>(AT66SaintNPC::StaticClass(), SaintLoc, FRotator::ZeroRotator, SpawnParams))
		{
			if (AT66HouseNPCBase* NPC = Cast<AT66HouseNPCBase>(SpawnedNPC))
			{
				NPC->NPCName = NSLOCTEXT("T66.NPC", "Saint", "Saint");
				NPC->NPCColor = FLinearColor(0.9f, 0.9f, 0.9f, 1.f);
				NPC->ApplyVisuals();
			}
		}
		return;
	}

	float TraceStartZ = 2000.f;
	float TraceEndZ = -4000.f;

	// Hero spawn location (same as PlayerStart / default spawn); NPCs spawn around it.
	float HeroX = -35455.f;
	float HeroY = 0.f;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		const FVector Loc = (*It)->GetActorLocation();
		HeroX = Loc.X;
		HeroY = Loc.Y;
		break;
	}

	const float Offset = 600.f;
	struct FNPCPosition { float X; float Y; };
	const FNPCPosition NPCPositions[] = {
		{ HeroX + Offset, HeroY },
		{ HeroX - Offset, HeroY },
		{ HeroX, HeroY + Offset },
		{ HeroX, HeroY - Offset },
	};

	struct FNPCDef
	{
		TSubclassOf<AActor> NPCClass;
		FText Name;
		FLinearColor Color;
	};
	TArray<FNPCDef> NPCDefs = {
		{ AT66OuroborosNPC::StaticClass(), NSLOCTEXT("T66.NPC", "Ouroboros", "Ouroboros"),  FLinearColor(0.1f,0.8f,0.2f,1.f) },
		{ AT66SaintNPC::StaticClass(),     NSLOCTEXT("T66.NPC", "Saint", "Saint"),          FLinearColor(0.9f,0.9f,0.9f,1.f) },
	};

	// Shuffle NPC assignment to corners so each run has different NPCs at each corner.
	UT66GameInstance* T66GI = GetT66GameInstance();
	const int32 RunSeed = (T66GI && T66GI->RunSeed != 0) ? T66GI->RunSeed : FMath::Rand();
	FRandomStream ShuffleRng(RunSeed + 991);
	for (int32 i = NPCDefs.Num() - 1; i > 0; --i)
	{
		const int32 j = ShuffleRng.RandRange(0, i);
		NPCDefs.Swap(i, j);
	}

	// Find closest flat surface to a reference point (flat = normal nearly vertical). Search in rings outward.
	auto FindClosestFlatSurface = [World, TraceStartZ, TraceEndZ](float RefX, float RefY) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;   // ~22?? max slope
		static constexpr float SearchRadiusMax = 1200.f;
		static constexpr float RadiusStep = 120.f;
		static constexpr int32 NumAngles = 16;

		FVector Fallback(RefX, RefY, 60.f);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, FVector(RefX, RefY, TraceStartZ), FVector(RefX, RefY, TraceEndZ), ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float R = 0.f; R <= SearchRadiusMax; R += RadiusStep)
		{
			const int32 AngleSteps = (R <= 0.f) ? 1 : NumAngles;
			for (int32 a = 0; a < AngleSteps; ++a)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(a) / static_cast<float>(AngleSteps));
				const float X = RefX + R * FMath::Cos(Angle);
				const float Y = RefY + R * FMath::Sin(Angle);
				const FVector Start(X, Y, TraceStartZ);
				const FVector End(X, Y, TraceEndZ);
				if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic) && Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}
		return Fallback;
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < NPCDefs.Num(); ++i)
	{
		const FVector FlatLoc = FindClosestFlatSurface(NPCPositions[i].X, NPCPositions[i].Y);
		AActor* SpawnedNPC = World->SpawnActor<AActor>(NPCDefs[i].NPCClass, FlatLoc, FRotator::ZeroRotator, SpawnParams);
		if (AT66HouseNPCBase* NPC = Cast<AT66HouseNPCBase>(SpawnedNPC))
		{
			NPC->NPCName = NPCDefs[i].Name;
			NPC->NPCColor = NPCDefs[i].Color;
			NPC->ApplyVisuals();
		}
	}
}

void AT66GameMode::SpawnCircusInteractableIfNeeded()
{
	if (IsColiseumStage() || IsLabLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AT66CircusInteractable> It(World); It; ++It)
	{
		return;
	}

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	float TraceStartZ = 2000.f;
	float TraceEndZ = -4000.f;
	float RefX = 0.f;
	float RefY = 0.f;
	if (bUsingMainMapTerrain)
	{
		FVector CircusLoc = FVector::ZeroVector;
		if (!TryFindRandomMainMapSurfaceLocation(3201, CircusLoc, 450.f))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66CircusInteractable>(AT66CircusInteractable::StaticClass(), CircusLoc, FRotator::ZeroRotator, SpawnParams);
		return;
	}

	auto FindClosestFlatSurface = [World, TraceStartZ, TraceEndZ](float InRefX, float InRefY) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 2200.f;
		static constexpr float RadiusStep = 140.f;
		static constexpr int32 NumAngles = 20;

		FVector Fallback(InRefX, InRefY, 60.f);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, FVector(InRefX, InRefY, TraceStartZ), FVector(InRefX, InRefY, TraceEndZ), ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float R = 0.f; R <= SearchRadiusMax; R += RadiusStep)
		{
			const int32 AngleSteps = (R <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = InRefX + R * FMath::Cos(Angle);
				const float Y = InRefY + R * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic) && Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	const FVector FlatLoc = FindClosestFlatSurface(RefX, RefY);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66CircusInteractable>(AT66CircusInteractable::StaticClass(), FlatLoc, FRotator::ZeroRotator, SpawnParams);
}

void AT66GameMode::SpawnSupportVendorAtStartIfNeeded()
{
	if (IsColiseumStage() || IsLabLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66GameInstance* T66GI = GetT66GameInstance();
	if (!T66GI)
	{
		return;
	}

	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>();
	if (!PlayerExperience || !PlayerExperience->ShouldSpawnSupportVendorAtRunStart(T66GI->SelectedDifficulty))
	{
		return;
	}

	static const FName SupportVendorTag(TEXT("T66_StartSupportVendor"));
	for (TActorIterator<AT66VendorNPC> It(World); It; ++It)
	{
		if (AT66VendorNPC* ExistingVendor = *It)
		{
			if (ExistingVendor->ActorHasTag(SupportVendorTag))
			{
				return;
			}
		}
	}

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	float TraceStartZ = 2000.f;
	float TraceEndZ = -4000.f;
	FVector ReferenceLoc = FVector(-35455.f, 0.f, 60.f);
	float SideOffset = 700.f;
	if (bUsingMainMapTerrain)
	{
		FVector SpawnLoc = FVector::ZeroVector;
		if (!TryGetMainMapStartPlacementLocation(1.0f, 0.0f, SpawnLoc))
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AT66VendorNPC* SupportVendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams))
		{
			SupportVendor->ConfigureAsSupportVendor(PlayerExperience->ShouldSupportVendorAllowSteal(T66GI->SelectedDifficulty));
			SupportVendor->Tags.AddUnique(SupportVendorTag);
			SupportVendor->ApplyVisuals();
		}
		return;
	}
	else if (AController* PC = World->GetFirstPlayerController())
	{
		if (APawn* HeroPawn = PC->GetPawn())
		{
			ReferenceLoc = HeroPawn->GetActorLocation();
		}
	}

	auto FindClosestFlatSurface = [World, TraceStartZ, TraceEndZ](const FVector& DesiredLoc) -> FVector
	{
		static constexpr float MinNormalZ = 0.92f;
		static constexpr float SearchRadiusMax = 900.f;
		static constexpr float RadiusStep = 100.f;
		static constexpr int32 NumAngles = 16;

		FVector Fallback = DesiredLoc;
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(
			Hit,
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceStartZ),
			FVector(DesiredLoc.X, DesiredLoc.Y, TraceEndZ),
			ECC_WorldStatic))
		{
			Fallback = Hit.ImpactPoint;
		}

		for (float Radius = 0.f; Radius <= SearchRadiusMax; Radius += RadiusStep)
		{
			const int32 AngleSteps = (Radius <= 0.f) ? 1 : NumAngles;
			for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
			{
				const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
				const float X = DesiredLoc.X + Radius * FMath::Cos(Angle);
				const float Y = DesiredLoc.Y + Radius * FMath::Sin(Angle);
				if (World->LineTraceSingleByChannel(Hit, FVector(X, Y, TraceStartZ), FVector(X, Y, TraceEndZ), ECC_WorldStatic)
					&& Hit.ImpactNormal.Z >= MinNormalZ)
				{
					return Hit.ImpactPoint;
				}
			}
		}

		return Fallback;
	};

	const FVector DesiredLoc = ReferenceLoc + FVector(SideOffset, 0.f, 0.f);
	const FVector SpawnLoc = FindClosestFlatSurface(DesiredLoc);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66VendorNPC* SupportVendor = World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams))
	{
		SupportVendor->ConfigureAsSupportVendor(PlayerExperience->ShouldSupportVendorAllowSteal(T66GI->SelectedDifficulty));
		SupportVendor->Tags.AddUnique(SupportVendorTag);
		SupportVendor->ApplyVisuals();
	}
}

void AT66GameMode::SpawnStartGateForPlayer(AController* Player)
{
	(void)Player;
	UWorld* World = GetWorld();
	if (!World) return;
	if (T66UsesMainMapTerrainStage(World)) return;

	// Spawn once per level (gate is a world landmark).
	if (StartGate) return;

	// Gate at the start-corridor exit so combat starts only when the player enters the main arena.
	static constexpr float GateZOffset = 5.f;
	FVector GateLoc = T66GameplayLayout::GetStartGateLocation();
	float GateGroundZ = 200.f;
	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, GateLoc + FVector(0.f, 0.f, 3000.f), GateLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
	{
		GateGroundZ = Hit.ImpactPoint.Z;
		GateLoc.Z = GateGroundZ + GateZOffset;
	}
	else
	{
		GateLoc.Z = GateGroundZ;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	StartGate = World->SpawnActor<AT66StartGate>(AT66StartGate::StaticClass(), GateLoc, FRotator::ZeroRotator, SpawnParams);
	if (StartGate)
	{
		StartGate->TriggerDistance2D = 220.f;
		if (StartGate->TriggerBox)
		{
			StartGate->TriggerBox->SetBoxExtent(FVector(120.f, T66GameplayLayout::CorridorHalfHeightY * 0.92f, 220.f));
		}
		if (StartGate->PoleLeft)
		{
			StartGate->PoleLeft->SetVisibility(false, true);
			StartGate->PoleLeft->SetHiddenInGame(true, true);
		}
		if (StartGate->PoleRight)
		{
			StartGate->PoleRight->SetVisibility(false, true);
			StartGate->PoleRight->SetHiddenInGame(true, true);
		}
		UE_LOG(LogTemp, Log, TEXT("Spawned Start Gate at main-area entrance (%.0f, %.0f, %.0f)"), GateLoc.X, GateLoc.Y, GateLoc.Z);
	}

}

void AT66GameMode::SpawnPlateauAtLocation(UWorld* World, const FVector& TopCenterLoc)
{
	if (!World) return;
	static constexpr float PlateauHalfHeight = 10.f; // top of disc at TopCenterLoc.Z
	const FVector PlateauLoc(TopCenterLoc.X, TopCenterLoc.Y, TopCenterLoc.Z - PlateauHalfHeight);
	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AT66SpawnPlateau>(AT66SpawnPlateau::StaticClass(), PlateauLoc, FRotator::ZeroRotator, P);
}

void AT66GameMode::SpawnWorldInteractablesForStage()
{
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UGameInstance* GI = GetGameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState) return;
	UT66RngSubsystem* RngSub = GI ? GI->GetSubsystem<UT66RngSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();

	// Run-level seed so positions change every time "Enter the Tribulation" or PIE is started (like procedural terrain).
	const int32 RunSeed = T66EnsureRunSeed(T66GI);
	const int32 StageNum = RunState->GetCurrentStage();
	FRandomStream Rng(RunSeed + StageNum * 1337 + 42);
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);

	FT66MapPreset MainMapPreset;
	T66MainMapTerrain::FSettings MainMapSettings;
	float MainHalfExtent = 50000.f;
	float TraceStartZ = 8000.f;
	float TraceEndZ = -16000.f;
	if (bUsingMainMapTerrain)
	{
		MainMapPreset = T66BuildMainMapPreset(T66GI);
		MainMapSettings = T66MainMapTerrain::MakeSettings(MainMapPreset);
		MainHalfExtent = FMath::Max(0.0f, MainMapSettings.HalfExtent - MainMapSettings.CellSize * 1.25f);
		TraceStartZ = T66MainMapTerrain::GetTraceZ(MainMapPreset);
		TraceEndZ = T66MainMapTerrain::GetLowestCollisionBottomZ(MainMapPreset) - MainMapSettings.StepHeight;
	}

	static constexpr float SpawnZ = 220.f;
	static constexpr float MinDistBetweenInteractables = 900.f;
	static constexpr float SafeBubbleMargin = 250.f;

	TArray<FVector> UsedLocs;

	// No-spawn zones: keep gameplay spawns out of reserved traversal spaces and special arenas.
	auto IsInsideNoSpawnZone = [&](const FVector& L) -> bool
	{
		if (bUsingMainMapTerrain)
		{
			const float RoomReserveRadius = MainMapSettings.CellSize * 1.70f;
			const float CorridorReserveRadius = MainMapSettings.CellSize * 0.80f;
			if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
			{
				return true;
			}
			if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapBossAreaCenterSurfaceLocation) < FMath::Square(RoomReserveRadius))
			{
				return true;
			}
			if (!MainMapStartPathSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartPathSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapStartAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero()
				&& FVector::DistSquared2D(L, MainMapBossAnchorSurfaceLocation) < FMath::Square(CorridorReserveRadius))
			{
				return true;
			}
			return false;
		}

		if (T66GameplayLayout::IsInsideReservedTraversalZone2D(L, 455.f))
		{
			return true;
		}

		static constexpr float ArenaHalf = 9091.f;
		static constexpr float ArenaMargin = 682.f;
		static constexpr float TutorialArenaHalf = 9091.f;
		struct FArena2D { float X; float Y; float Half; };
		static constexpr FArena2D Arenas[] = {
			{      0.f, 34091.f, ArenaHalf },  // Coliseum
			{      0.f, 61364.f, TutorialArenaHalf }, // Tutorial
		};
		for (const FArena2D& A : Arenas)
		{
			if (FMath::Abs(L.X - A.X) <= (A.Half + ArenaMargin) && FMath::Abs(L.Y - A.Y) <= (A.Half + ArenaMargin))
			{
				return true;
			}
		}

		return false;
	};

	auto IsGoodLoc = [&](const FVector& L) -> bool
	{
		if (IsInsideNoSpawnZone(L))
		{
			return false;
		}
		static constexpr float CasinoKeepClearRadius = 2200.f;
		if (!bUsingMainMapTerrain && FVector::DistSquared2D(L, FVector::ZeroVector) < (CasinoKeepClearRadius * CasinoKeepClearRadius))
		{
			return false;
		}
		for (const FVector& U : UsedLocs)
		{
			if (FVector::DistSquared2D(L, U) < (MinDistBetweenInteractables * MinDistBetweenInteractables))
			{
				return false;
			}
		}
		UT66ActorRegistrySubsystem* Registry = World ? World->GetSubsystem<UT66ActorRegistrySubsystem>() : nullptr;
		const TArray<TWeakObjectPtr<AT66HouseNPCBase>>& NPCs = Registry ? Registry->GetNPCs() : TArray<TWeakObjectPtr<AT66HouseNPCBase>>();
		for (const TWeakObjectPtr<AT66HouseNPCBase>& WeakNPC : NPCs)
		{
			const AT66HouseNPCBase* NPC = WeakNPC.Get();
			if (!NPC) continue;
			const float R = NPC->GetSafeZoneRadius() + SafeBubbleMargin;
			if (FVector::DistSquared2D(L, NPC->GetActorLocation()) < (R * R))
			{
				return false;
			}
		}
		return true;
	};

	struct FSpawnHitResult
	{
		FVector Loc = FVector::ZeroVector;
		bool bFound = false;
	};
	auto FindSpawnLoc = [&]() -> FSpawnHitResult
	{
		for (int32 Try = 0; Try < 40; ++Try)
		{
			const float X = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			const float Y = Rng.FRandRange(-MainHalfExtent, MainHalfExtent);
			FVector Loc(X, Y, SpawnZ);

			FHitResult Hit;
			const FVector Start(X, Y, TraceStartZ);
			const FVector End(X, Y, TraceEndZ);
			if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
			{
				continue;
			}
			if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal))
			{
				continue;
			}
			Loc = Hit.ImpactPoint;

			if (IsGoodLoc(Loc))
			{
				return { Loc, true };
			}
		}
		return {};
	};

	auto SpawnOne = [&](UClass* Cls) -> AActor*
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		const FSpawnHitResult HitResult = FindSpawnLoc();
		if (!HitResult.bFound)
		{
			return nullptr;
		}
		AActor* A = World->SpawnActor<AActor>(Cls, HitResult.Loc, FRotator::ZeroRotator, P);
		if (A) UsedLocs.Add(HitResult.Loc);
		return A;
	};

	if (RngSub)
	{
		RngSub->UpdateLuckStat(RunState->GetLuckStat());
	}

	const UT66RngTuningConfig* Tuning = RngSub ? RngSub->GetTuning() : nullptr;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI ? T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>() : nullptr;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const FT66IntRange ChestCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyChestCountRange(Difficulty)
		: FT66IntRange{ 4, 10 };
	const FT66IntRange WheelCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyWheelCountRange(Difficulty)
		: FT66IntRange{ 5, 11 };
	const FT66IntRange CrateCountRange = PlayerExperience
		? PlayerExperience->GetDifficultyCrateCountRange(Difficulty)
		: FT66IntRange{ 3, 6 };

	// Luck-affected counts use central tuning. Locations are still stage-seeded (not luck-affected).
	const int32 CountFountains = (RngSub && Tuning) ? RngSub->RollIntRangeBiased(Tuning->TreesPerStage, Rng) : Rng.RandRange(2, 5);
	const int32 CountChests = RngSub ? RngSub->RollIntRangeBiased(ChestCountRange, Rng) : Rng.RandRange(FMath::Min(ChestCountRange.Min, ChestCountRange.Max), FMath::Max(ChestCountRange.Min, ChestCountRange.Max));
	const int32 CountWheels = RngSub ? RngSub->RollIntRangeBiased(WheelCountRange, Rng) : Rng.RandRange(FMath::Min(WheelCountRange.Min, WheelCountRange.Max), FMath::Max(WheelCountRange.Min, WheelCountRange.Max));
	const int32 CountCrates = RngSub ? RngSub->RollIntRangeBiased(CrateCountRange, Rng) : Rng.RandRange(FMath::Min(CrateCountRange.Min, CrateCountRange.Max), FMath::Max(CrateCountRange.Min, CrateCountRange.Max));

	// Luck Rating tracking (quantity).
	if (RunState)
	{
		const int32 FountainsMin = (Tuning ? Tuning->TreesPerStage.Min : 2);
		const int32 FountainsMax = (Tuning ? Tuning->TreesPerStage.Max : 5);
		const int32 ChestsMin = FMath::Min(ChestCountRange.Min, ChestCountRange.Max);
		const int32 ChestsMax = FMath::Max(ChestCountRange.Min, ChestCountRange.Max);
		const int32 WheelsMin = FMath::Min(WheelCountRange.Min, WheelCountRange.Max);
		const int32 WheelsMax = FMath::Max(WheelCountRange.Min, WheelCountRange.Max);
		const int32 CratesMin = FMath::Min(CrateCountRange.Min, CrateCountRange.Max);
		const int32 CratesMax = FMath::Max(CrateCountRange.Min, CrateCountRange.Max);
		RunState->RecordLuckQuantityRoll(FName(TEXT("FountainsPerStage")), CountFountains, FountainsMin, FountainsMax);
		RunState->RecordLuckQuantityRoll(FName(TEXT("ChestsPerStage")), CountChests, ChestsMin, ChestsMax);
		RunState->RecordLuckQuantityRoll(FName(TEXT("WheelsPerStage")), CountWheels, WheelsMin, WheelsMax);
		RunState->RecordLuckQuantityRoll(FName(TEXT("CratesPerStage")), CountCrates, CratesMin, CratesMax);
	}

	// Not luck-affected (for now).
	const int32 CountTotems = Rng.RandRange(4, 10);

	for (int32 i = 0; i < CountFountains; ++i)
	{
		if (AT66FountainOfLifeInteractable* Fountain = Cast<AT66FountainOfLifeInteractable>(SpawnOne(AT66FountainOfLifeInteractable::StaticClass())))
		{
			Fountain->SetRarity(ET66Rarity::Black);
		}
	}
	for (int32 i = 0; i < CountChests; ++i)
	{
		if (AT66ChestInteractable* Chest = Cast<AT66ChestInteractable>(SpawnOne(AT66ChestInteractable::StaticClass())))
		{
			const FT66RarityWeights Weights = PlayerExperience
				? PlayerExperience->GetDifficultyChestRarityWeights(Difficulty)
				: FT66RarityWeights{};
			Chest->bIsMimic = (Rng.GetFraction() < (PlayerExperience ? PlayerExperience->GetDifficultyChestMimicChance(Difficulty) : 0.20f));
			const ET66Rarity R = (RngSub && PlayerExperience) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			Chest->SetRarity(R);
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(FName(TEXT("ChestRarity")), R);
			}
		}
	}
	for (int32 i = 0; i < CountWheels; ++i)
	{
		if (AT66WheelSpinInteractable* Wheel = Cast<AT66WheelSpinInteractable>(SpawnOne(AT66WheelSpinInteractable::StaticClass())))
		{
			const FT66RarityWeights Weights = PlayerExperience
				? PlayerExperience->GetDifficultyWheelRarityWeights(Difficulty)
				: FT66RarityWeights{};
			const ET66Rarity R = (RngSub && PlayerExperience) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			Wheel->SetRarity(R);
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(FName(TEXT("WheelRarity")), R);
			}
		}
	}

	if (!IsColiseumStage() && !IsLabLevel())
	{
		FVector TractorLoc = FVector::ZeroVector;
		bool bShouldSpawnGuaranteedTractor = false;
		if (!bUsingMainMapTerrain)
		{
			static constexpr float GuaranteedTractorOffsetX = 850.f; // just east of the start gate, outside the start corridor
			const FVector StartGateLoc = T66GameplayLayout::GetStartGateLocation();
			const float TractorX = StartGateLoc.X + GuaranteedTractorOffsetX;
			const float TractorY = StartGateLoc.Y;
			FHitResult TractorHit;
			const FVector TractorTraceStart(TractorX, TractorY, 8000.f);
			const FVector TractorTraceEnd(TractorX, TractorY, -16000.f);
			TractorLoc = FVector(TractorX, TractorY, SpawnZ);
			if (World->LineTraceSingleByChannel(TractorHit, TractorTraceStart, TractorTraceEnd, ECC_WorldStatic))
			{
				TractorLoc = TractorHit.ImpactPoint;
			}
			bShouldSpawnGuaranteedTractor = true;
		}

		if (bShouldSpawnGuaranteedTractor)
		{
			FActorSpawnParameters TractorSpawnParams;
			TractorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66PilotableTractor* GuaranteedTractor = World->SpawnActor<AT66PilotableTractor>(
				AT66PilotableTractor::StaticClass(), TractorLoc, FRotator::ZeroRotator, TractorSpawnParams))
			{
				UsedLocs.Add(TractorLoc);
			}
		}

		FVector VendingLoc = FVector::ZeroVector;
		bool bShouldSpawnGuaranteedVending = false;
		if (bUsingMainMapTerrain)
		{
			const FSpawnHitResult VendingFallback = FindSpawnLoc();
			if (VendingFallback.bFound)
			{
				VendingLoc = VendingFallback.Loc;
				bShouldSpawnGuaranteedVending = true;
			}
		}
		else
		{
			static constexpr float GuaranteedVendingOffsetX = 1450.f;
			static constexpr float GuaranteedVendingOffsetY = -950.f;
			const FVector StartGateLoc = T66GameplayLayout::GetStartGateLocation();
			const float VendingX = StartGateLoc.X + GuaranteedVendingOffsetX;
			const float VendingY = StartGateLoc.Y + GuaranteedVendingOffsetY;
			VendingLoc = FVector(VendingX, VendingY, SpawnZ);
			FHitResult VendingHit;
			const FVector VendingTraceStart(VendingX, VendingY, 8000.f);
			const FVector VendingTraceEnd(VendingX, VendingY, -16000.f);
			if (World->LineTraceSingleByChannel(VendingHit, VendingTraceStart, VendingTraceEnd, ECC_WorldStatic))
			{
				VendingLoc = VendingHit.ImpactPoint;
			}
			bShouldSpawnGuaranteedVending = true;
		}

		if (bShouldSpawnGuaranteedVending)
		{
			FActorSpawnParameters VendingSpawnParams;
			VendingSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			if (AT66QuickReviveVendingMachine* Vending = World->SpawnActor<AT66QuickReviveVendingMachine>(
				AT66QuickReviveVendingMachine::StaticClass(), VendingLoc, FRotator::ZeroRotator, VendingSpawnParams))
			{
				UsedLocs.Add(VendingLoc);
			}
		}

		if (!bUsingMainMapTerrain)
		{
			SpawnModelShowcaseRow();
		}
	}

	static constexpr int32 CountTeleportPads = 5;
	for (int32 i = 0; i < CountTeleportPads; ++i)
	{
		SpawnOne(AT66TeleportPadInteractable::StaticClass());
	}

	for (int32 i = 0; i < CountCrates; ++i)
	{
		if (AT66CrateInteractable* Crate = Cast<AT66CrateInteractable>(SpawnOne(AT66CrateInteractable::StaticClass())))
		{
			const FT66RarityWeights Weights = PlayerExperience
				? PlayerExperience->GetDifficultyCrateRarityWeights(Difficulty)
				: FT66RarityWeights{};
			const ET66Rarity R = (RngSub && PlayerExperience) ? RngSub->RollRarityWeighted(Weights, Rng) : FT66RarityUtil::RollDefaultRarity(Rng);
			Crate->SetRarity(R);
			if (RunState)
			{
				RunState->RecordLuckQualityRarity(FName(TEXT("CrateRarity")), R);
			}
		}
	}

	for (int32 i = 0; i < CountTotems; ++i)
	{
		if (AT66DifficultyTotem* Totem = Cast<AT66DifficultyTotem>(SpawnOne(AT66DifficultyTotem::StaticClass())))
		{
			// Not luck-affected (per current scope).
			Totem->SetRarity(FT66RarityUtil::RollDefaultRarity(Rng));
		}
	}
}

void AT66GameMode::SpawnModelShowcaseRow()
{
	if (IsColiseumStage() || IsLabLevel())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	struct FShowcaseMeshSpec
	{
		const TCHAR* MeshPath = nullptr;
		FVector Offset = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FVector Scale = FVector(1.f, 1.f, 1.f);
		FName Tag = NAME_None;
	};

	const FVector StartAreaCenter = T66GameplayLayout::GetStartAreaCenter();
	const FShowcaseMeshSpec ShowcaseSpecs[] = {
		{ TEXT("/Game/World/Props/StartAreaDecor/Cow.Cow"),      FVector(1800.f,  1500.f, 0.f), FRotator(0.f, -140.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaCow")) },
		{ TEXT("/Game/World/Props/StartAreaDecor/RoboCow.RoboCow"), FVector(1800.f, -1500.f, 0.f), FRotator(0.f,  140.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaRoboCow")) },
		{ TEXT("/Game/World/Props/StartAreaDecor/FullBody.FullBody"), FVector(-1800.f,    0.f, 0.f), FRotator(0.f,    0.f, 0.f), FVector(2.f, 2.f, 2.f), FName(TEXT("T66StartAreaFullBody")) },
	};

	auto FindExistingShowcaseActor = [&](const FName Tag) -> AStaticMeshActor*
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			AStaticMeshActor* Existing = *It;
			if (Existing && Existing->ActorHasTag(Tag))
			{
				return Existing;
			}
		}
		return nullptr;
	};

	for (const FShowcaseMeshSpec& Spec : ShowcaseSpecs)
	{
		if (Spec.MeshPath == nullptr || Spec.Tag.IsNone())
		{
			continue;
		}

		AStaticMeshActor* Actor = FindExistingShowcaseActor(Spec.Tag);
		if (!Actor)
		{
			FVector SpawnLoc = StartAreaCenter + Spec.Offset;
			FHitResult Hit;
			const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 8000.f);
			const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 16000.f);
			if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
			{
				SpawnLoc = Hit.ImpactPoint;
			}

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLoc, Spec.Rotation, SpawnParams);
			if (!Actor)
			{
				continue;
			}
			Actor->Tags.AddUnique(Spec.Tag);
		}

		TSoftObjectPtr<UStaticMesh> MeshSoft(FSoftObjectPath(Spec.MeshPath));
		UStaticMesh* Mesh = MeshSoft.LoadSynchronous();
		if (!Mesh)
		{
			Actor->Destroy();
			continue;
		}

		T66_SetStaticMeshActorMobility(Actor, EComponentMobility::Movable);
		if (UStaticMeshComponent* MeshComponent = Actor->GetStaticMeshComponent())
		{
			MeshComponent->SetStaticMesh(Mesh);
			MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			MeshComponent->SetGenerateOverlapEvents(false);
			MeshComponent->SetRelativeScale3D(Spec.Scale);
			FT66VisualUtil::GroundMeshToActorOrigin(MeshComponent, Mesh);
		}
		Actor->SetActorRotation(Spec.Rotation);
		T66_SetStaticMeshActorMobility(Actor, EComponentMobility::Static);
	}
}

void AT66GameMode::SpawnIdolAltarForPlayer(AController* Player)
{
	if (IsColiseumStage() || IsLabLevel()) return;

	UWorld* World = GetWorld();
	if (!World || !Player) return;

	if (IsValid(IdolAltar))
	{
		return;
	}
	IdolAltar = nullptr;

	UGameInstance* GI = World->GetGameInstance();
	UT66IdolManagerSubsystem* IdolManager = GI ? GI->GetSubsystem<UT66IdolManagerSubsystem>() : nullptr;
	if (!IdolManager || !IdolManager->HasCatchUpIdolPicksRemaining())
	{
		return;
	}

	FVector SpawnLoc = FVector::ZeroVector;
	if (T66UsesMainMapTerrainStage(World))
	{
		if (!TryGetMainMapStartPlacementLocation(-1.2f, 0.35f, SpawnLoc))
		{
			return;
		}
	}
	else
	{
		SpawnLoc = T66GameplayLayout::GetStartAreaCenter() + FVector(-900.f, 1250.f, 0.f);

		FHitResult Hit;
		const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 3000.f);
		const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic)
			|| World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
		{
			SpawnLoc = Hit.ImpactPoint;
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	IdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (IdolAltar)
	{
		IdolAltar->bConsumesCatchUpIdolPicks = true;
	}
}

void AT66GameMode::SpawnIdolAltarAtLocation(const FVector& Location)
{
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	if (IsValid(IdolAltar) && !IdolAltar->bConsumesCatchUpIdolPicks) return;
	IdolAltar = nullptr;

	// Place near boss death, but offset so it doesn't overlap the Stage Gate.
	FVector SpawnLoc = Location + FVector(420.f, 260.f, 0.f);

	// Trace down so altar sits on the ground.
	FHitResult Hit;
	const FVector TraceStart = SpawnLoc + FVector(0.f, 0.f, 3000.f);
	const FVector TraceEnd = SpawnLoc - FVector(0.f, 0.f, 9000.f);
	if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		SpawnLoc = Hit.ImpactPoint;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	IdolAltar = World->SpawnActor<AT66IdolAltar>(AT66IdolAltar::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (IdolAltar)
	{
		IdolAltar->bConsumesCatchUpIdolPicks = false;
	}
}

void AT66GameMode::SpawnStageCatchUpPlatformAndInteractables()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = T66GI ? T66GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;
	UT66PlayerExperienceSubSystem* PlayerExperience = T66GI->GetSubsystem<UT66PlayerExperienceSubSystem>();

	RunState->SetInStageCatchUp(true);

	const int32 DiffIndex = FMath::Max(0, static_cast<int32>(T66GI->SelectedDifficulty));
	const int32 StartStage = PlayerExperience
		? PlayerExperience->GetDifficultyStartStage(T66GI->SelectedDifficulty)
		: FMath::Clamp(1 + (DiffIndex * 5), 1, 33);

	const int32 GoldAmount = PlayerExperience
		? PlayerExperience->GetDifficultyStartGoldBonus(T66GI->SelectedDifficulty)
		: 200 * DiffIndex;
	const int32 LootBags = PlayerExperience
		? PlayerExperience->GetDifficultyStartLootBags(T66GI->SelectedDifficulty)
		: 2 + (DiffIndex * 2);

	const FVector PlatformCenter(-45455.f, 23636.f, -50.f);

	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AT66StageCatchUpGoldInteractable* Gold = World->SpawnActor<AT66StageCatchUpGoldInteractable>(
			AT66StageCatchUpGoldInteractable::StaticClass(),
			PlatformCenter + FVector(-260.f, -120.f, 120.f),
			FRotator::ZeroRotator,
			P);
		if (Gold)
		{
			Gold->GoldAmount = GoldAmount;
		}

		AT66StageCatchUpLootInteractable* Loot = World->SpawnActor<AT66StageCatchUpLootInteractable>(
			AT66StageCatchUpLootInteractable::StaticClass(),
			PlatformCenter + FVector(-260.f, 220.f, 120.f),
			FRotator::ZeroRotator,
			P);
		if (Loot)
		{
			Loot->LootBagCount = LootBags;
		}

		AT66StageCatchUpGate* Gate = World->SpawnActor<AT66StageCatchUpGate>(
			AT66StageCatchUpGate::StaticClass(),
			PlatformCenter + FVector(520.f, 0.f, 200.f),
			FRotator::ZeroRotator,
			P);
		if (Gate)
		{
			Gate->TargetStage = StartStage;
		}
	}
}

void AT66GameMode::SpawnStageGateAtLocation(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Snap to ground at the boss death XY so the gate is never floating/sunk.
	FVector SpawnLoc(Location.X, Location.Y, Location.Z);
	{
		FHitResult Hit;
		const FVector Start = SpawnLoc + FVector(0.f, 0.f, 3000.f);
		const FVector End = SpawnLoc - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic) ||
			World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			SpawnLoc = Hit.ImpactPoint;
		}
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66StageGate* StageGate = World->SpawnActor<AT66StageGate>(AT66StageGate::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (StageGate)
	{
		UE_LOG(LogTemp, Log, TEXT("Spawned Stage Gate at boss death location"));
	}
}

void AT66GameMode::SpawnBossForCurrentStage()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (IsColiseumStage())
	{
		SpawnAllOwedBossesInColiseum();
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!T66GI || !RunState) return;

	const int32 StageNum = RunState->GetCurrentStage();

	// Default/fallback stage config (if DT_Stages is not wired yet)
	FStageData StageData;
	StageData.StageNumber = StageNum;
	StageData.BossID = FName(*FString::Printf(TEXT("Boss_%02d"), StageNum));
	StageData.BossSpawnLocation = StageGateSpawnOffset; // far side by default

	FStageData FromDT;
	if (T66GI->GetStageData(StageNum, FromDT))
	{
		StageData = FromDT;
	}

	if (T66UsesMainMapTerrainStage(World) && !MainMapBossSpawnSurfaceLocation.IsNearlyZero())
	{
		StageData.BossSpawnLocation = MainMapBossSpawnSurfaceLocation + FVector(0.f, 0.f, 100.f);
	}
	else
	{
		// Map layout: spawn the stage boss in the dedicated boss square at the far end of the run.
		FVector BossLoc = T66GameplayLayout::GetBossAreaCenter(200.f);
		FHitResult BossHit;
		if (World->LineTraceSingleByChannel(BossHit, BossLoc + FVector(0.f, 0.f, 3000.f), BossLoc - FVector(0.f, 0.f, 9000.f), ECC_WorldStatic))
		{
			BossLoc.Z = BossHit.ImpactPoint.Z + 100.f;
		}
		StageData.BossSpawnLocation = BossLoc;
	}

	// Default/fallback boss data (if DT_Bosses is not wired yet)
	FBossData BossData;
	BossData.BossID = StageData.BossID;
	{
		// Stage-scaled fallback so all 33 stages work even if DT_Bosses isn't reimported yet.
		const int32 S = FMath::Clamp(StageNum, 1, 33);
		const float T = static_cast<float>(S - 1) / 32.f; // 0..1

		// Bosses are intended to be 1000+ HP always (BossBase will clamp too).
		BossData.MaxHP = 1000 + (S * 250);
		BossData.AwakenDistance = 900.f;
		BossData.MoveSpeed = 350.f + (S * 2.f);
		BossData.FireIntervalSeconds = FMath::Clamp(2.0f - (S * 0.015f), 0.65f, 3.5f);
		BossData.ProjectileSpeed = 900.f + (S * 15.f);
		BossData.ProjectileDamageHearts = 1 + (S / 20);

		// Visually differentiate bosses by stage (HSV wheel).
		const float Hue = FMath::Fmod(static_cast<float>(S) * 31.f, 360.f);
		FLinearColor C = FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue / 360.f * 255.f), 210, 245);
		C.A = 1.f;
		// Slightly darken early, brighten later.
		C.R = FMath::Lerp(C.R * 0.85f, C.R, T);
		C.G = FMath::Lerp(C.G * 0.85f, C.G, T);
		C.B = FMath::Lerp(C.B * 0.85f, C.B, T);
		BossData.PlaceholderColor = C;
	}

	FBossData FromBossDT;
	if (!StageData.BossID.IsNone() && T66GI->GetBossData(StageData.BossID, FromBossDT))
	{
		BossData = FromBossDT;
	}

	UClass* BossClass = AT66BossBase::StaticClass();
	const bool bWantsSpecificBossClass = !BossData.BossClass.IsNull();
	const bool bBossClassLoaded = bWantsSpecificBossClass && (BossData.BossClass.Get() != nullptr);
	if (bBossClassLoaded)
	{
		if (UClass* Loaded = BossData.BossClass.Get())
		{
			if (Loaded->IsChildOf(AT66BossBase::StaticClass()))
			{
				BossClass = Loaded;
			}
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Spawned = World->SpawnActor<AActor>(BossClass, StageData.BossSpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (AT66BossBase* Boss = Cast<AT66BossBase>(Spawned))
	{
		// Snap to ground so the boss never floats/sinks.
		FHitResult Hit;
		const FVector Start = Boss->GetActorLocation() + FVector(0.f, 0.f, 4000.f);
		const FVector End = Boss->GetActorLocation() - FVector(0.f, 0.f, 9000.f);
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			const float HalfHeight = Boss->GetCapsuleComponent() ? Boss->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.f;
			Boss->SetActorLocation(Hit.ImpactPoint + FVector(0.f, 0.f, HalfHeight), false, nullptr, ETeleportType::TeleportPhysics);
		}

		Boss->InitializeBoss(BossData);
		StageBoss = Boss;
		SpawnBossBeaconIfNeeded();
		UE_LOG(LogTemp, Log, TEXT("Spawned boss for Stage %d (BossID=%s)"), StageNum, *BossData.BossID.ToString());

		// If the boss class is a soft reference and isn't loaded yet, load asynchronously and replace the dormant boss.
		if (bWantsSpecificBossClass && !bBossClassLoaded)
		{
			const FSoftObjectPath ClassPath = BossData.BossClass.ToSoftObjectPath();
			const TWeakObjectPtr<AT66BossBase> WeakExistingBoss(Boss);
			const FBossData BossDataCopy = BossData;

			TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
				ClassPath,
				FStreamableDelegate::CreateWeakLambda(this, [this, WeakExistingBoss, BossDataCopy]()
				{
					UWorld* World2 = GetWorld();
					if (!World2) return;

					UClass* Loaded = BossDataCopy.BossClass.Get();
					if (!Loaded || !Loaded->IsChildOf(AT66BossBase::StaticClass()))
					{
						return;
					}

					AT66BossBase* ExistingBoss = WeakExistingBoss.Get();
					if (!ExistingBoss) return;
					if (ExistingBoss->GetClass() == Loaded) return;

					// Preserve the already-snapped location so the replacement doesn't float/sink.
					const FVector Loc = ExistingBoss->GetActorLocation();
					ExistingBoss->Destroy();

					FActorSpawnParameters SpawnParams2;
					SpawnParams2.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					AActor* Spawned2 = World2->SpawnActor<AActor>(Loaded, Loc, FRotator::ZeroRotator, SpawnParams2);
					if (AT66BossBase* NewBoss = Cast<AT66BossBase>(Spawned2))
					{
						NewBoss->InitializeBoss(BossDataCopy);
						StageBoss = NewBoss;
						SpawnBossBeaconIfNeeded();
					}
				}));
			if (Handle.IsValid())
			{
				ActiveAsyncLoadHandles.Add(Handle);
			}
		}
	}
}

void AT66GameMode::EnsureLevelSetup()
{
	UE_LOG(LogTemp, Log, TEXT("Checking level setup..."));

	// Destroy any Landscape or editor-placed foliage actors saved in the level (legacy from external asset packs).
	UWorld* CleanupWorld = GetWorld();
	if (CleanupWorld)
	{
		for (TActorIterator<ALandscape> It(CleanupWorld); It; ++It)
		{
			UE_LOG(LogTemp, Log, TEXT("[MAP] Destroying saved Landscape actor: %s"), *It->GetName());
			It->Destroy();
		}
		static const FName OldFoliageTag(TEXT("T66ProceduralFoliage"));
		TArray<AActor*> ToDestroy;
		for (TActorIterator<AActor> It(CleanupWorld); It; ++It)
		{
			if (It->Tags.Contains(OldFoliageTag))
			{
				ToDestroy.Add(*It);
			}
		}
		for (AActor* A : ToDestroy)
		{
			A->Destroy();
		}
		if (ToDestroy.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[MAP] Destroyed %d legacy foliage actors."), ToDestroy.Num());
		}
	}

	if (IsLabLevel())
	{
		SpawnLabFloorIfNeeded();
		SpawnLightingIfNeeded();  // Same sky and lighting as gameplay: SkyAtmosphere (blue sky), sun, sky light, fog, post process
		SpawnQuakeSkyIfNeeded();
		SpawnLabCollectorIfNeeded();
		SpawnPlayerStartIfNeeded();
		return;
	}

	SpawnFloorIfNeeded();
	SpawnColiseumArenaIfNeeded();
	SpawnTutorialArenaIfNeeded();
	SpawnStartAreaWallsIfNeeded();
	if (!T66UsesMainMapTerrainStage(CleanupWorld))
	{
		SpawnBossAreaWallsIfNeeded();
	}
	TryApplyGroundFloorMaterialToAllFloors();
	SpawnLightingIfNeeded();
	SpawnQuakeSkyIfNeeded();
	SpawnPlayerStartIfNeeded();
}

void AT66GameMode::TryApplyGroundFloorMaterialToAllFloors()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const int32 GroundVariantCount = GroundFloorMaterials.Num();
	if (GroundVariantCount == 0)
	{
		return;
	}

	// Check if all configured materials are loaded
	bool bAllLoaded = true;
	for (int32 i = 0; i < GroundVariantCount; ++i)
	{
		if (!GroundFloorMaterials[i].Get())
		{
			bAllLoaded = false;
			break;
		}
	}

	if (!bAllLoaded)
	{
		if (!bGroundFloorMaterialLoadRequested)
		{
			bGroundFloorMaterialLoadRequested = true;
			TArray<FSoftObjectPath> Paths;
			for (int32 i = 0; i < GroundVariantCount; ++i)
			{
				if (!GroundFloorMaterials[i].IsNull())
				{
					Paths.Add(GroundFloorMaterials[i].ToSoftObjectPath());
				}
			}
			if (Paths.Num() > 0)
			{
				TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
					Paths, FStreamableDelegate::CreateWeakLambda(this, [this]()
					{
						TryApplyGroundFloorMaterialToAllFloors();
					}));
				if (Handle.IsValid())
				{
					ActiveAsyncLoadHandles.Add(Handle);
				}
			}
		}
		return;
	}

	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* A = *It;
		if (!A) continue;
		if (!T66_HasAnyFloorTag(A)) continue;
		if (UStaticMeshComponent* SMC = A->GetStaticMeshComponent())
		{
			UMaterialInterface* Mat = GroundFloorMaterials[0].Get();  // fallback
			const FVector Loc = A->GetActorLocation();
			const float Seed = Loc.X * 0.000123f + Loc.Y * 0.000456f;
			const int32 Idx = FMath::Clamp(
				FMath::FloorToInt(FMath::Frac(FMath::Abs(Seed)) * static_cast<float>(GroundVariantCount)),
				0,
				GroundVariantCount - 1);
			if (Idx < GroundFloorMaterials.Num())
			{
				UMaterialInterface* M = GroundFloorMaterials[Idx].Get();
				if (M) Mat = M;
			}
			SMC->SetMaterial(0, Mat);
		}
	}

	// Also apply to non-StaticMeshActor floor-tagged actors (HISM walls, ProceduralMesh tops/ramps).
	UMaterialInterface* FallbackMat = GroundFloorMaterials[0].Get();
	if (FallbackMat)
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* A = *It;
			if (!A || Cast<AStaticMeshActor>(A)) continue;
			if (!T66_HasAnyFloorTag(A)) continue;

			TArray<UMeshComponent*> MeshComps;
			A->GetComponents<UMeshComponent>(MeshComps);
			for (UMeshComponent* MC : MeshComps)
			{
				if (MC)
				{
					MC->SetMaterial(0, FallbackMat);
				}
			}
		}
	}
}

void AT66GameMode::SpawnTutorialArenaIfNeeded()
{
	// Tutorial Arena is an enclosed side-area inside GameplayLevel. It is only relevant for normal gameplay stages.
	if (IsColiseumStage()) return;

	UWorld* World = GetWorld();
	if (!World) return;
	if (!T66IsStandaloneTutorialMap(World)) return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				return true;
			}
		}
		return false;
	};

	// Tutorial: to the side of the main map, separated by empty space. Scaled for 100k map.
	const FVector TutorialCenter(0.f, 61364.f, -50.f);
	const FName FloorTag(TEXT("T66_Floor_Tutorial"));

	// Floor (no overlap with main)
	if (!HasTag(FloorTag))
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), TutorialCenter, FRotator::ZeroRotator, P);
		if (Floor && Floor->GetStaticMeshComponent())
		{
			Floor->Tags.Add(FloorTag);
			T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
			Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
			Floor->SetActorScale3D(FVector(80.f, 80.f, 1.f)); // 8000x8000
			T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Static);
			if (UMaterialInstanceDynamic* Mat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
			{
				Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.14f, 0.14f, 0.16f, 1.f));
			}
			if (!SpawnedSetupActors.Contains(Floor))
			{
				SpawnedSetupActors.Add(Floor);
			}
		}
	}
}

void AT66GameMode::SpawnColiseumArenaIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				return true;
			}
		}
		return false;
	};

	// Coliseum arena: off to the side. No separate floor (hard rule: no overlapping grounds; main floor covers it).
}

bool AT66GameMode::TryComputeBossBeaconBase(FVector& OutBeaconBase) const
{
	if (IsColiseumStage() || IsLabLevel() || !StageBoss.IsValid())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (T66UsesMainMapTerrainStage(World))
	{
		if (!MainMapBossBeaconSurfaceLocation.IsNearlyZero())
		{
			OutBeaconBase = MainMapBossBeaconSurfaceLocation;
			return true;
		}

		if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero())
		{
			OutBeaconBase = MainMapBossAreaCenterSurfaceLocation;
			return true;
		}
	}

	FVector DesiredBase = StageBoss->GetActorLocation() + FVector(3400.f, 0.f, 0.f);
	DesiredBase.X = FMath::Clamp(DesiredBase.X, T66GameplayLayout::BossAreaWestX + 1000.f, T66GameplayLayout::BossPartitionEastX - 900.f);
	DesiredBase.Y = FMath::Clamp(DesiredBase.Y, -T66GameplayLayout::AreaHalfHeightY + 600.f, T66GameplayLayout::AreaHalfHeightY - 600.f);
	const float MinNormalZ = 0.88f;

	auto TryTraceBossBeaconSurface = [&](float X, float Y, FVector& OutLoc) -> bool
	{
		FHitResult Hit;
		const FVector TraceStart(X, Y, 5000.f);
		const FVector TraceEnd(X, Y, -12000.f);
		if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			return false;
		}

		if (!T66GameplayLayout::IsValidGameplayGroundNormal(Hit.ImpactNormal, MinNormalZ))
		{
			return false;
		}

		OutLoc = Hit.ImpactPoint;
		return true;
	};

	FVector FallbackLoc = DesiredBase;
	{
		FHitResult FallbackHit;
		const FVector TraceStart = DesiredBase + FVector(0.f, 0.f, 5000.f);
		const FVector TraceEnd = DesiredBase - FVector(0.f, 0.f, 12000.f);
		if (World->LineTraceSingleByChannel(FallbackHit, TraceStart, TraceEnd, ECC_WorldStatic))
		{
			FallbackLoc = FallbackHit.ImpactPoint;
		}
	}

	FVector BeaconBase = DesiredBase;
	BeaconBase.Z = 0.f;
	bool bFoundFlatSurface = false;
	for (float Radius = 0.f; Radius <= 1600.f && !bFoundFlatSurface; Radius += 220.f)
	{
		const int32 AngleSteps = (Radius <= KINDA_SMALL_NUMBER) ? 1 : 14;
		for (int32 AngleIndex = 0; AngleIndex < AngleSteps; ++AngleIndex)
		{
			const float Angle = (AngleSteps == 1) ? 0.f : (2.f * PI * static_cast<float>(AngleIndex) / static_cast<float>(AngleSteps));
			const float X = DesiredBase.X + (Radius * FMath::Cos(Angle));
			const float Y = DesiredBase.Y + (Radius * FMath::Sin(Angle));
			if (TryTraceBossBeaconSurface(X, Y, BeaconBase))
			{
				bFoundFlatSurface = true;
				break;
			}
		}
	}

	OutBeaconBase = bFoundFlatSurface ? BeaconBase : FallbackLoc;
	return true;
}

void AT66GameMode::DestroyBossBeacon()
{
	if (AActor* BeaconActor = BossBeaconActor.Get())
	{
		BeaconActor->Destroy();
	}
	BossBeaconActor = nullptr;
}

void AT66GameMode::UpdateBossBeaconTransform(bool bForceSpawnIfMissing)
{
	if (IsColiseumStage() || IsLabLevel() || !StageBoss.IsValid())
	{
		DestroyBossBeacon();
		return;
	}

	FVector BeaconBase = FVector::ZeroVector;
	if (!TryComputeBossBeaconBase(BeaconBase))
	{
		return;
	}

	if (AActor* BeaconActor = BossBeaconActor.Get())
	{
		BeaconActor->SetActorLocation(BeaconBase, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	if (bForceSpawnIfMissing)
	{
		SpawnBossBeaconIfNeeded();
	}
}

void AT66GameMode::SpawnBossBeaconIfNeeded()
{
	if (IsColiseumStage() || IsLabLevel())
	{
		DestroyBossBeacon();
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	static const FName BossBeaconTag(TEXT("T66_Boss_Beacon"));

	TArray<AActor*> ExistingBeacons;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(BossBeaconTag))
		{
			ExistingBeacons.Add(*It);
		}
	}
	for (AActor* Existing : ExistingBeacons)
	{
		if (Existing)
		{
			Existing->Destroy();
		}
	}
	BossBeaconActor = nullptr;

	// The beacon is meant to sit behind the live stage boss.
	// If the boss has not spawned yet, do not place a fallback beam in the arena.
	FVector BeaconBase = FVector::ZeroVector;
	if (!TryComputeBossBeaconBase(BeaconBase))
	{
		return;
	}

	constexpr float OuterHeight = 34000.f;
	constexpr float InnerHeight = 32000.f;
	constexpr float OuterRadius = 250.f;
	constexpr float InnerRadius = 90.f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* BeaconActor = World->SpawnActor<AActor>(AActor::StaticClass(), BeaconBase, FRotator::ZeroRotator, SpawnParams);
	if (!BeaconActor)
	{
		return;
	}

	BeaconActor->Tags.Add(BossBeaconTag);

	USceneComponent* Root = NewObject<USceneComponent>(BeaconActor, TEXT("BossBeaconRoot"));
	if (!Root)
	{
		BeaconActor->Destroy();
		return;
	}

	BeaconActor->SetRootComponent(Root);
	Root->SetMobility(EComponentMobility::Movable);
	Root->RegisterComponent();

	UStaticMesh* CylinderMesh = FT66VisualUtil::GetBasicShapeCylinder();
	UMaterialInterface* EnvUnlitBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
	UTexture* WhiteTexture = LoadObject<UTexture>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture"));

	auto CreateBeamMaterial = [&](const FLinearColor& Tint, float Brightness) -> UMaterialInterface*
	{
		if (!EnvUnlitBase)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(EnvUnlitBase, BeaconActor);
		if (!MID)
		{
			return nullptr;
		}

		if (WhiteTexture)
		{
			MID->SetTextureParameterValue(TEXT("DiffuseColorMap"), WhiteTexture);
			MID->SetTextureParameterValue(TEXT("BaseColorTexture"), WhiteTexture);
		}
		MID->SetVectorParameterValue(TEXT("Tint"), Tint);
		MID->SetVectorParameterValue(TEXT("BaseColor"), Tint);
		MID->SetScalarParameterValue(TEXT("Brightness"), Brightness);
		return MID;
	};

	auto ConfigureBeamMesh = [&](const TCHAR* ComponentName, float Radius, float Height, const FLinearColor& Tint, float Brightness)
	{
		if (!CylinderMesh)
		{
			return;
		}

		UStaticMeshComponent* BeamMesh = NewObject<UStaticMeshComponent>(BeaconActor, ComponentName);
		if (!BeamMesh)
		{
			return;
		}

		BeamMesh->SetStaticMesh(CylinderMesh);
		BeamMesh->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.5f));
		BeamMesh->SetRelativeScale3D(FVector(Radius / 50.f, Radius / 50.f, Height / 100.f));
		BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BeamMesh->SetCastShadow(false);
		BeamMesh->SetReceivesDecals(false);
		BeamMesh->SetCanEverAffectNavigation(false);
		BeamMesh->SetMobility(EComponentMobility::Movable);
		BeamMesh->SetupAttachment(Root);
		if (UMaterialInterface* BeamMaterial = CreateBeamMaterial(Tint, Brightness))
		{
			BeamMesh->SetMaterial(0, BeamMaterial);
		}
		BeamMesh->RegisterComponent();
	};

	ConfigureBeamMesh(TEXT("BossBeaconOuter"), OuterRadius, OuterHeight, FLinearColor(0.70f, 0.90f, 1.00f, 1.f), 9.5f);
	ConfigureBeamMesh(TEXT("BossBeaconInner"), InnerRadius, InnerHeight, FLinearColor(1.00f, 0.93f, 0.60f, 1.f), 26.0f);

	auto ConfigureGlowLight = [&](const TCHAR* ComponentName, float HeightAlpha, float Intensity, float AttenuationRadius)
	{
		UPointLightComponent* Glow = NewObject<UPointLightComponent>(BeaconActor, ComponentName);
		if (!Glow)
		{
			return;
		}

		Glow->SetMobility(EComponentMobility::Movable);
		Glow->SetupAttachment(Root);
		Glow->SetRelativeLocation(FVector(0.f, 0.f, OuterHeight * HeightAlpha));
		Glow->SetIntensity(Intensity);
		Glow->SetLightColor(FLinearColor(1.00f, 0.92f, 0.72f));
		Glow->SetAttenuationRadius(AttenuationRadius);
		Glow->SetCastShadows(false);
		Glow->bUseInverseSquaredFalloff = false;
		Glow->LightFalloffExponent = 3.0f;
		Glow->RegisterComponent();
	};

	ConfigureGlowLight(TEXT("BossBeaconMidGlow"), 0.42f, 85000.f, 9000.f);
	ConfigureGlowLight(TEXT("BossBeaconTopGlow"), 0.86f, 125000.f, 14000.f);

	BossBeaconActor = BeaconActor;
	SpawnedSetupActors.Add(BeaconActor);
}

void AT66GameMode::SpawnStartAreaWallsIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Start area walls disabled: remove any that were saved in the level so player can leave freely.
	static const FName WallTags[] = {
		FName("T66_Wall_Start_W"), FName("T66_Wall_Start_N"), FName("T66_Wall_Start_S"),
		FName("T66_Wall_Start_E_N"), FName("T66_Wall_Start_E_S")
	};
	for (FName Tag : WallTags)
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				It->Destroy();
				break;
			}
		}
	}
	return;

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag)) return true;
		}
		return false;
	};

	// Very short red wall fully around the start area. Square, with gap from miasma wall (so back wall is not on the boundary).
	// Square 4000x4000: X -17600..-13600, Y -2000..2000 (800 uu gap from miasma at -18400).
	static constexpr float FloorTopZ = 0.f;
	static constexpr float WallHeightShort = 80.f;
	static constexpr float WallThickness = 120.f;
	const float WallZ = FloorTopZ + (WallHeightShort * 0.5f);
	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeightShort / 100.f;
	const FLinearColor Red(0.75f, 0.12f, 0.12f, 1.f);

	const float BoxWestX = -17600.f;
	const float BoxEastX = -13600.f;
	const float BoxNorthY = 2000.f;
	const float BoxSouthY = -2000.f;
	const float BoxWidthX = BoxEastX - BoxWestX;   // 4000
	const float BoxHeightY = BoxNorthY - BoxSouthY; // 4000
	const float OpeningHalfY = 400.f;   // gap on east for Start Gate pillars (opening Y from -400 to +400)
	const float EastNorthLen = BoxNorthY - OpeningHalfY;   // north segment: 400 to 2000 -> length 1600
	const float EastSouthLen = (-OpeningHalfY) - BoxSouthY; // south segment: -2000 to -400 -> length 1600 (was 2400, closed the gap)
	const float EastNorthCenterY = BoxNorthY - (EastNorthLen * 0.5f);
	const float EastSouthCenterY = BoxSouthY + (EastSouthLen * 0.5f);

	struct FWallSpec { FName Tag; FVector Location; FVector Scale; };
	const TArray<FWallSpec> Walls = {
		{ FName("T66_Wall_Start_W"),   FVector(BoxWestX - (WallThickness * 0.5f), 0.f, WallZ), FVector(Thick, BoxHeightY / 100.f, Tall) },
		{ FName("T66_Wall_Start_N"),   FVector(BoxWestX + (BoxWidthX * 0.5f), BoxNorthY + (WallThickness * 0.5f), WallZ), FVector(BoxWidthX / 100.f, Thick, Tall) },
		{ FName("T66_Wall_Start_S"),   FVector(BoxWestX + (BoxWidthX * 0.5f), BoxSouthY - (WallThickness * 0.5f), WallZ), FVector(BoxWidthX / 100.f, Thick, Tall) },
		{ FName("T66_Wall_Start_E_N"), FVector(BoxEastX + (WallThickness * 0.5f), EastNorthCenterY, WallZ), FVector(Thick, EastNorthLen / 100.f, Tall) },
		{ FName("T66_Wall_Start_E_S"), FVector(BoxEastX + (WallThickness * 0.5f), EastSouthCenterY, WallZ), FVector(Thick, EastSouthLen / 100.f, Tall) },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FWallSpec& Spec : Walls)
	{
		if (HasTag(Spec.Tag)) continue;
		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;
		Wall->Tags.Add(Spec.Tag);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (UMaterialInstanceDynamic* Mat = Wall->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), Red);
		}
		SpawnedSetupActors.Add(Wall);
	}
}

void AT66GameMode::SpawnBossAreaWallsIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (T66UsesMainMapTerrainStage(World))
	{
		static const FName BossWallTags[] = {
			FName("T66_Wall_Boss_W"),
			FName("T66_Wall_Boss_E"),
			FName("T66_Wall_Boss_N"),
			FName("T66_Wall_Boss_S")
		};
		for (FName Tag : BossWallTags)
		{
			for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
			{
				if (It->Tags.Contains(Tag))
				{
					It->Destroy();
					break;
				}
			}
		}
		return;
	}

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto HasTag = [&](FName Tag) -> bool
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
			if (It->Tags.Contains(Tag)) return true;
		return false;
	};

	// Force synchronous load of ground materials so walls get the same Unlit atlas as platform walls.
	for (int32 i = 0; i < GroundFloorMaterials.Num(); ++i)
	{
		if (!GroundFloorMaterials[i].IsNull() && !GroundFloorMaterials[i].Get())
		{
			GroundFloorMaterials[i].LoadSynchronous();
		}
	}
	UMaterialInterface* FloorMat = (GroundFloorMaterials.Num() > 0) ? GroundFloorMaterials[0].Get() : nullptr;

	// Boss area: right after the boss pillars, inside safe zone. Scaled for 100k map.
	static constexpr float FloorTopZ = 0.f;
	static constexpr float WallHeightShort = 80.f;
	static constexpr float WallThickness = 120.f;
	const float WallZ = FloorTopZ + (WallHeightShort * 0.5f);
	const float Thick = WallThickness / 100.f;
	const float Tall = WallHeightShort / 100.f;

	const float BoxWestX = 30909.f;
	const float BoxEastX = 40000.f;
	const float BoxNorthY = 4545.f;
	const float BoxSouthY = -4545.f;
	const float BoxWidthX = BoxEastX - BoxWestX;
	const float BoxHeightY = BoxNorthY - BoxSouthY;
	const float BoxCenterX = 35455.f;

	struct FWallSpec { FName Tag; FVector Location; FVector Scale; };
	const TArray<FWallSpec> Walls = {
		{ FName("T66_Wall_Boss_W"), FVector(BoxWestX - (WallThickness * 0.5f), 0.f, WallZ), FVector(Thick, BoxHeightY / 100.f, Tall) },
		{ FName("T66_Wall_Boss_E"), FVector(BoxEastX + (WallThickness * 0.5f), 0.f, WallZ), FVector(Thick, BoxHeightY / 100.f, Tall) },
		{ FName("T66_Wall_Boss_N"), FVector(BoxCenterX, BoxNorthY + (WallThickness * 0.5f), WallZ), FVector(BoxWidthX / 100.f, Thick, Tall) },
		{ FName("T66_Wall_Boss_S"), FVector(BoxCenterX, BoxSouthY - (WallThickness * 0.5f), WallZ), FVector(BoxWidthX / 100.f, Thick, Tall) },
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FWallSpec& Spec : Walls)
	{
		if (HasTag(Spec.Tag)) continue;
		AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		if (!Wall || !Wall->GetStaticMeshComponent()) continue;
		Wall->Tags.Add(Spec.Tag);
		Wall->Tags.Add(FName("T66_Floor_Main"));
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Movable);
		Wall->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Wall->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Wall, EComponentMobility::Static);
		if (FloorMat)
		{
			Wall->GetStaticMeshComponent()->SetMaterial(0, FloorMat);
		}
		SpawnedSetupActors.Add(Wall);
	}
}

void AT66GameMode::SpawnFloorIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (IsLabLevel()) return;  // Lab uses SpawnLabFloorIfNeeded() only

	// Ground materials are loaded async: if not yet resident the floor spawns without material,
	// and the async callback at the bottom of this function re-applies once loaded.

	// Main run uses Landscape only (no main floor). Coliseum, Boost, and Tutorial are small island floors to the side.
	struct FFloorSpec
	{
		FName Tag;
		FVector Location;
		FVector Scale;
		FLinearColor Color;
	};

	auto DestroyTaggedIfExists = [&](FName Tag)
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				It->Destroy();
				break;
			}
		}
	};
	// T66_Floor_Main is spawned by SpawnMainMapTerrain(); do not destroy it here.
	DestroyTaggedIfExists(FName("T66_Floor_Conn1"));
	DestroyTaggedIfExists(FName("T66_Floor_Conn2"));
	DestroyTaggedIfExists(FName("T66_Floor_Start"));
	DestroyTaggedIfExists(FName("T66_Floor_Boss"));
	DestroyTaggedIfExists(FName("T66_Floor_Coliseum"));
	DestroyTaggedIfExists(FName("T66_Floor_CatchUp"));
	DestroyTaggedIfExists(FName("T66_Floor_Tutorial"));

	if (T66UsesMainMapTerrainStage(World) || T66IsStandaloneTutorialMap(World))
	{
		return;
	}

	if (!T66IsStandaloneColiseumMap(World))
	{
		return;
	}

	// Standalone coliseum map: spawn only the coliseum floor.
	constexpr float IslandFloorZ = -50.f;
	const TArray<FFloorSpec> Floors = {
		{ FName("T66_Floor_Coliseum"), FVector(ColiseumCenter.X, ColiseumCenter.Y, IslandFloorZ), FVector(40.f, 40.f, 1.f), FLinearColor(0.30f, 0.30f, 0.35f, 1.f) },
	};

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	auto FindTaggedActor = [&](FName Tag) -> AStaticMeshActor*
	{
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				return *It;
			}
		}
		return nullptr;
	};

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FFloorSpec& Spec : Floors)
	{
		AStaticMeshActor* Floor = FindTaggedActor(Spec.Tag);
		if (!Floor)
		{
			Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Spec.Location, FRotator::ZeroRotator, SpawnParams);
		}
		if (!Floor || !Floor->GetStaticMeshComponent()) continue;

		if (!Floor->Tags.Contains(Spec.Tag))
		{
			Floor->Tags.Add(Spec.Tag);
		}
		T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
		Floor->SetActorLocation(Spec.Location);
		Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
		Floor->SetActorScale3D(Spec.Scale);
		T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Static);

		// Pick one of the available ground material variants by floor position (deterministic variety).
		UMaterialInterface* GroundMat = nullptr;
		const int32 GroundVariantCount = GroundFloorMaterials.Num();
		if (GroundVariantCount > 0)
		{
			const float Seed = Spec.Location.X * 0.000123f + Spec.Location.Y * 0.000456f;
			const int32 Idx = FMath::Clamp(
				FMath::FloorToInt(FMath::Frac(FMath::Abs(Seed)) * static_cast<float>(GroundVariantCount)),
				0,
				GroundVariantCount - 1);
			GroundMat = GroundFloorMaterials[Idx].Get();
		}
		if (GroundMat)
		{
			Floor->GetStaticMeshComponent()->SetMaterial(0, GroundMat);
		}
		else
		{
			// Kick off async load once, then rerun SpawnFloorIfNeeded() to apply material to tagged floors.
			if (!bGroundFloorMaterialLoadRequested)
			{
				bool bAnyToLoad = false;
				for (int32 i = 0; i < GroundFloorMaterials.Num(); ++i)
				{
					if (!GroundFloorMaterials[i].IsNull()) bAnyToLoad = true;
				}
				if (bAnyToLoad)
				{
					bGroundFloorMaterialLoadRequested = true;
					TArray<FSoftObjectPath> Paths;
					for (int32 i = 0; i < GroundFloorMaterials.Num(); ++i)
					{
						if (!GroundFloorMaterials[i].IsNull())
						{
							Paths.Add(GroundFloorMaterials[i].ToSoftObjectPath());
						}
					}
					if (Paths.Num() > 0)
					{
						TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
							Paths, FStreamableDelegate::CreateWeakLambda(this, [this]() { SpawnFloorIfNeeded(); }));
						if (Handle.IsValid())
						{
							ActiveAsyncLoadHandles.Add(Handle);
						}
					}
				}
			}

			if (UMaterialInstanceDynamic* FloorMat = Floor->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
			{
				FloorMat->SetVectorParameterValue(TEXT("BaseColor"), Spec.Color);
			}
		}

		if (!SpawnedSetupActors.Contains(Floor))
		{
			SpawnedSetupActors.Add(Floor);
		}
	}
}

// ============================================================================
// Main map terrain
// ============================================================================

static const FName T66TraversalBarrierTag(TEXT("T66_Map_TraversalBarrier"));
static const FName T66MapPlatformTag(TEXT("T66_Map_Platform"));
static const FName T66MapRampTag(TEXT("T66_Map_Ramp"));
static const FName T66FloorMainTag(TEXT("T66_Floor_Main"));

static void DestroyActorsWithTag(UWorld* World, FName Tag)
{
	if (!World)
	{
		return;
	}

	TArray<AActor*> ToDestroy;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(Tag))
		{
			ToDestroy.Add(*It);
		}
	}

	for (AActor* Actor : ToDestroy)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

void AT66GameMode::SpawnMainMapTerrain()
{
	UWorld* World = GetWorld();
	if (!World || IsLabLevel() || IsColiseumStage() || !T66UsesMainMapTerrainStage(World))
	{
		return;
	}

	TArray<AActor*> CleanupActors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor)
		{
			continue;
		}

		if (Actor->Tags.Contains(T66MapPlatformTag)
			|| Actor->Tags.Contains(T66MapRampTag)
			|| Actor->Tags.Contains(T66FloorMainTag)
			|| Actor->Tags.Contains(T66TraversalBarrierTag)
			|| T66_HasAnyFloorTag(Actor))
		{
			CleanupActors.Add(Actor);
		}
	}

	for (AActor* Actor : CleanupActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	bTerrainCollisionReady = false;
	bMainMapCombatStarted = false;
	bHasMainMapSpawnSurfaceLocation = false;
	MainMapSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapStartAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapStartPathSurfaceLocation = FVector::ZeroVector;
	MainMapStartAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapBossAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapBossSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapBossBeaconSurfaceLocation = FVector::ZeroVector;
	MainMapBossAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapRescueAnchorLocations.Reset();

	UT66GameInstance* GI = GetT66GameInstance();
	const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
	const T66MainMapTerrain::FSettings MainMapSettings = T66MainMapTerrain::MakeSettings(Preset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UE_LOG(LogTemp, Log, TEXT("[MAP] Generating main map terrain (seed=%d, grid=%d, cell=%.0f, step=%.0f, scale=%.2f)"),
		Preset.Seed,
		MainMapSettings.BoardSize,
		MainMapSettings.CellSize,
		MainMapSettings.StepHeight,
		MainMapSettings.BoardScale);

	T66MainMapTerrain::FBoard Board;
	if (!T66MainMapTerrain::Generate(Preset, Board))
	{
		UE_LOG(LogTemp, Error, TEXT("[MAP] Main map terrain generation failed to fill the board (seed=%d, occupied=%d/%d)"),
			Preset.Seed,
			Board.OccupiedCount,
			Board.Cells.Num());
		return;
	}

	MainMapSpawnSurfaceLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Board, 0.f);
	bHasMainMapSpawnSurfaceLocation = true;
	MainMapRescueAnchorLocations.Add(MainMapSpawnSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.StartAnchor, 0.f, MainMapStartAnchorSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.StartPathCell, 0.f, MainMapStartPathSurfaceLocation);
	if (!T66MainMapTerrain::TryGetRegionCenterLocation(Board, T66MainMapTerrain::ECellRegion::StartArea, 0.f, MainMapStartAreaCenterSurfaceLocation))
	{
		MainMapStartAreaCenterSurfaceLocation = MainMapSpawnSurfaceLocation;
	}
	if (!MainMapStartAnchorSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapStartAnchorSurfaceLocation);
	}
	if (!MainMapStartAreaCenterSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapStartAreaCenterSurfaceLocation);
	}
	T66MainMapTerrain::TryGetCellLocation(Board, Board.BossAnchor, 0.f, MainMapBossAnchorSurfaceLocation);
	T66MainMapTerrain::TryGetCellLocation(Board, Board.BossSpawnCell, 0.f, MainMapBossSpawnSurfaceLocation);
	if (!T66MainMapTerrain::TryGetRegionCenterLocation(Board, T66MainMapTerrain::ECellRegion::BossArea, 0.f, MainMapBossAreaCenterSurfaceLocation))
	{
		MainMapBossAreaCenterSurfaceLocation = MainMapBossSpawnSurfaceLocation;
	}

	FVector BossBeaconLocation = FVector::ZeroVector;
	if (T66MainMapTerrain::TryGetCellLocation(Board, Board.BossSpawnCell - Board.BossOutwardDirection, 0.f, BossBeaconLocation))
	{
		MainMapBossBeaconSurfaceLocation = BossBeaconLocation;
	}
	else
	{
		MainMapBossBeaconSurfaceLocation = MainMapBossAreaCenterSurfaceLocation;
	}

	if (!MainMapBossAnchorSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossAnchorSurfaceLocation);
	}
	if (!MainMapBossSpawnSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossSpawnSurfaceLocation);
	}
	if (!MainMapBossAreaCenterSurfaceLocation.IsNearlyZero())
	{
		MainMapRescueAnchorLocations.Add(MainMapBossAreaCenterSurfaceLocation);
	}
	UE_LOG(LogTemp, Log, TEXT("[MAP] Cached main-map spawn surface at %s"), *MainMapSpawnSurfaceLocation.ToString());

	bool bMainMapCollisionReady = false;
	if (!T66MainMapTerrain::Spawn(World, Board, Preset, SpawnParams, bMainMapCollisionReady))
	{
		return;
	}

	bTerrainCollisionReady = bMainMapCollisionReady;
	if (bTerrainCollisionReady)
	{
		RestartPlayersMissingPawns();
		if (!T66UsesMainMapTerrainStage(World))
		{
			SnapPlayersToTerrain();
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("[MAP] Terrain collision ready=%d"), bTerrainCollisionReady ? 1 : 0);
	UE_LOG(LogTemp, Log, TEXT("[MAP] Main map terrain spawned: %d tiles (seed=%d, cell=%.0f)"),
		Board.OccupiedCount,
		Preset.Seed,
		Board.Settings.CellSize);
}

// ============================================================================
// HDRI Equirectangular ??? TextureCube conversion (editor-only, runs once)
// ============================================================================
#if WITH_EDITORONLY_DATA

static const TCHAR* HDRIEquirectPath = TEXT("/Game/Lighting/T_HDRI_Studio.T_HDRI_Studio");
static const TCHAR* HDRICubePackagePath = TEXT("/Game/Lighting/TC_HDRI_Studio");
static const TCHAR* HDRICubeAssetPath = TEXT("/Game/Lighting/TC_HDRI_Studio.TC_HDRI_Studio");

/** Map cube face + UV to a 3D direction vector (UE coordinate system: X=fwd, Y=right, Z=up). */
static FVector CubeFaceDirection(int32 Face, float U, float V)
{
	const float S = 2.f * U - 1.f;
	const float T = 2.f * V - 1.f;
	switch (Face)
	{
	case 0: return FVector( 1.f,  S,   -T);    // +X
	case 1: return FVector(-1.f, -S,   -T);    // -X
	case 2: return FVector(-S,    1.f, -T);     // +Y
	case 3: return FVector( S,   -1.f, -T);     // -Y
	case 4: return FVector(-S,   -T,    1.f);   // +Z
	case 5: return FVector(-S,    T,   -1.f);   // -Z
	default: return FVector::ForwardVector;
	}
}

/** Convert a 3D direction to equirectangular UV coordinates [0,1]. */
static FVector2D DirToEquirectUV(const FVector& Dir)
{
	const FVector N = Dir.GetSafeNormal();
	const float Phi   = FMath::Atan2(N.Y, N.X);                            // azimuth [-PI, PI]
	const float Theta = FMath::Asin(FMath::Clamp(N.Z, -1.f, 1.f));        // elevation [-PI/2, PI/2]
	return FVector2D(
		FMath::Frac(Phi / (2.f * UE_PI) + 0.5f),
		FMath::Clamp(0.5f - Theta / UE_PI, 0.f, 1.f)
	);
}

/**
 * If the HDRI Texture2D exists but no TextureCube has been created yet,
 * sample the equirectangular map into 6 cube faces and save as a persistent asset.
 */
static UTextureCube* EnsureHDRICubemap()
{
	// Already exists?
	UTextureCube* Existing = LoadObject<UTextureCube>(nullptr, HDRICubeAssetPath);
	if (Existing) return Existing;

	// Source equirectangular texture?
	UTexture2D* Src = LoadObject<UTexture2D>(nullptr, HDRIEquirectPath);
	if (!Src)
	{
		UE_LOG(LogTemp, Log, TEXT("[HDRI] No equirectangular source at %s ??? skipping cubemap creation"), HDRIEquirectPath);
		return nullptr;
	}

	FTextureSource& SrcSource = Src->Source;
	if (!SrcSource.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[HDRI] Source texture has no valid FTextureSource data"));
		return nullptr;
	}

	const int32 SrcW = SrcSource.GetSizeX();
	const int32 SrcH = SrcSource.GetSizeY();
	const ETextureSourceFormat SrcFmt = SrcSource.GetFormat();
	const int32 SrcBpp = SrcSource.GetBytesPerPixel();

	TArray64<uint8> SrcData;
	SrcSource.GetMipData(SrcData, 0);
	if (SrcData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HDRI] Could not read source mip data"));
		return nullptr;
	}

	// Face size: half the equirect height, clamped for sanity.
	const int32 FaceSize = FMath::Clamp(SrcH / 2, 64, 1024);
	const int32 FacePixels = FaceSize * FaceSize;
	const int32 OutBpp = 8; // RGBA16F = 4 channels ?? 2 bytes (FFloat16)

	UE_LOG(LogTemp, Log, TEXT("[HDRI] Creating cubemap from %dx%d equirect (format %d) ??? %dx%d faces"), SrcW, SrcH, (int32)SrcFmt, FaceSize, FaceSize);

	// Create the package and TextureCube
	UPackage* Package = CreatePackage(HDRICubePackagePath);
	UTextureCube* Cube = NewObject<UTextureCube>(Package, TEXT("TC_HDRI_Studio"), RF_Public | RF_Standalone);
	Cube->Source.Init(FaceSize, FaceSize, 6, 1, TSF_RGBA16F);
	Cube->SRGB = false;
	Cube->CompressionSettings = TC_HDR;
	Cube->LODGroup = TEXTUREGROUP_Skybox;

	uint8* RawOut = Cube->Source.LockMip(0);

	for (int32 Face = 0; Face < 6; ++Face)
	{
		for (int32 Y = 0; Y < FaceSize; ++Y)
		{
			for (int32 X = 0; X < FaceSize; ++X)
			{
				const float U = (X + 0.5f) / (float)FaceSize;
				const float V = (Y + 0.5f) / (float)FaceSize;

				const FVector Dir = CubeFaceDirection(Face, U, V);
				const FVector2D EqUV = DirToEquirectUV(Dir);

				// Nearest-neighbour sample from source (good enough for SkyLight ambient)
				const int32 SrcX = FMath::Clamp((int32)(EqUV.X * SrcW), 0, SrcW - 1);
				const int32 SrcY = FMath::Clamp((int32)(EqUV.Y * SrcH), 0, SrcH - 1);
				const int64 SrcOff = ((int64)SrcY * SrcW + SrcX) * SrcBpp;

				// Read source pixel as linear color
				FLinearColor Color = FLinearColor::Black;
				if (SrcFmt == TSF_RGBA16F && SrcOff + 8 <= SrcData.Num())
				{
					const FFloat16* H = reinterpret_cast<const FFloat16*>(&SrcData[SrcOff]);
					Color = FLinearColor(H[0].GetFloat(), H[1].GetFloat(), H[2].GetFloat(), 1.f);
				}
				else if (SrcFmt == TSF_BGRA8 && SrcOff + 4 <= SrcData.Num())
				{
					Color.B = SrcData[SrcOff + 0] / 255.f;
					Color.G = SrcData[SrcOff + 1] / 255.f;
					Color.R = SrcData[SrcOff + 2] / 255.f;
					Color.A = 1.f;
				}
				else if (SrcFmt == TSF_RGBA16 && SrcOff + 8 <= SrcData.Num())
				{
					const uint16* U16 = reinterpret_cast<const uint16*>(&SrcData[SrcOff]);
					Color.R = U16[0] / 65535.f;
					Color.G = U16[1] / 65535.f;
					Color.B = U16[2] / 65535.f;
					Color.A = 1.f;
				}

				// Write as RGBA16F
				const int64 DstOff = ((int64)Face * FacePixels + (int64)Y * FaceSize + X) * OutBpp;
				FFloat16* Out = reinterpret_cast<FFloat16*>(RawOut + DstOff);
				Out[0].Set(Color.R);
				Out[1].Set(Color.G);
				Out[2].Set(Color.B);
				Out[3].Set(1.f);
			}
		}
	}

	Cube->Source.UnlockMip(0);
	Cube->UpdateResource();
	Cube->PostEditChange();
	Package->MarkPackageDirty();

	// Persist to disk so it's available on next editor launch without re-generation.
	const FString Filename = FPackageName::LongPackageNameToFilename(HDRICubePackagePath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::Save(Package, Cube, *Filename, SaveArgs);

	UE_LOG(LogTemp, Log, TEXT("[HDRI] Saved cubemap: %s (%dx%d per face, RGBA16F)"), HDRICubeAssetPath, FaceSize, FaceSize);
	return Cube;
}

#endif // WITH_EDITORONLY_DATA

void AT66GameMode::SpawnLightingIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	static const FName MoonTag(TEXT("T66Moon"));

	ASkyAtmosphere* Atmosphere = nullptr;
	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
	{
		Atmosphere = *It;
		break;
	}

	ADirectionalLight* SunForAtmosphere = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		SunForAtmosphere = *It;
		break;
	}

	ASkyLight* SkyForCapture = nullptr;
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		SkyForCapture = *It;
		break;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Sky atmosphere (blue mid-day sky) if needed.
	if (!Atmosphere)
	{
		UE_LOG(LogTemp, Log, TEXT("No SkyAtmosphere found - spawning development sky atmosphere"));
		ASkyAtmosphere* SpawnedAtmosphere = World->SpawnActor<ASkyAtmosphere>(
			ASkyAtmosphere::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (SpawnedAtmosphere)
		{
			#if WITH_EDITOR
			SpawnedAtmosphere->SetActorLabel(TEXT("DEV_SkyAtmosphere"));
			#endif
			SpawnedSetupActors.Add(SpawnedAtmosphere);
			Atmosphere = SpawnedAtmosphere;
		}
	}

	// Spawn directional light (sun) if needed
	if (!SunForAtmosphere)
	{
		UE_LOG(LogTemp, Log, TEXT("No directional light found - spawning development sun"));

		ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector(0.f, 0.f, 1000.f),
			FRotator(-50.f, -45.f, 0.f), // Angled down
			SpawnParams
		);

		if (Sun)
		{
			if (UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
			{
				LightComp->SetMobility(EComponentMobility::Movable); // Dynamic lighting so landscape stays lit without Build Lighting
				LightComp->SetIntensity(3.f);  // Fill light ??? SkyLight is primary ambient
				LightComp->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f)); // Warm sunlight
				LightComp->CastShadows = false; // Shadows disabled ??? prevents dark bands on characters

				// Drive SkyAtmosphere sun/sky scattering (mid-day blue sky).
				LightComp->bAtmosphereSunLight = true;
				LightComp->AtmosphereSunLightIndex = 0;
				LightComp->SetForwardShadingPriority(1); // Primary for forward shading / translucency / water / volumetric fog
			}
			#if WITH_EDITOR
			Sun->SetActorLabel(TEXT("DEV_Sun"));
			#endif
			SpawnedSetupActors.Add(Sun);
			SunForAtmosphere = Sun;
			UE_LOG(LogTemp, Log, TEXT("Spawned development directional light"));
		}
	}

	// Ensure we have two directional lights (sun + moon) for theme-driven day/night.
	int32 DirLightCount = 0;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It) { ++DirLightCount; }
	if (DirLightCount < 2)
	{
		ADirectionalLight* Moon = World->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector(0.f, 0.f, 1000.f),
			FRotator(50.f, 135.f, 0.f), // ~180?? from sun so moon is "up" at night
			SpawnParams
		);
		if (Moon)
		{
			Moon->Tags.Add(MoonTag);
			if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(Moon->GetLightComponent()))
			{
				LC->SetMobility(EComponentMobility::Movable);
				LC->SetIntensity(0.f); // ApplyThemeToDirectionalLights() will set when Dark
				LC->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f)); // Cool moonlight
				LC->CastShadows = false; // Shadows disabled globally
				LC->bAtmosphereSunLight = true;
				LC->AtmosphereSunLightIndex = 1;
				LC->SetForwardShadingPriority(0); // Secondary; sun is primary for forward shading
			}
#if WITH_EDITOR
			Moon->SetActorLabel(TEXT("DEV_Moon"));
#endif
			SpawnedSetupActors.Add(Moon);
			UE_LOG(LogTemp, Log, TEXT("Spawned development moon light for dark-mode sky"));
		}
	}

	// Spawn sky light (ambient) if needed
	if (!SkyForCapture)
	{
		UE_LOG(LogTemp, Log, TEXT("No sky light found - spawning development ambient light"));

		ASkyLight* Sky = World->SpawnActor<ASkyLight>(
			ASkyLight::StaticClass(),
			FVector(0.f, 0.f, 500.f),
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Sky)
		{
			#if WITH_EDITOR
			Sky->SetActorLabel(TEXT("DEV_SkyLight"));
			#endif
			SpawnedSetupActors.Add(Sky);
			SkyForCapture = Sky;
			UE_LOG(LogTemp, Log, TEXT("Spawned development sky light"));
		}
	}

	// Exponential Height Fog for atmospheric depth (distant terrain lighter/softer, less flat/gamey).
	AExponentialHeightFog* HeightFog = nullptr;
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		HeightFog = *It;
		break;
	}
	if (!HeightFog)
	{
		UE_LOG(LogTemp, Log, TEXT("No Exponential Height Fog found - spawning for atmospheric depth"));
		AExponentialHeightFog* SpawnedFog = World->SpawnActor<AExponentialHeightFog>(
			AExponentialHeightFog::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (SpawnedFog)
		{
#if WITH_EDITOR
			SpawnedFog->SetActorLabel(TEXT("DEV_ExponentialHeightFog"));
#endif
			SpawnedSetupActors.Add(SpawnedFog);
			HeightFog = SpawnedFog;
		}
	}
	if (HeightFog)
	{
		ConfigureGameplayFogForWorld(World);
	}

	// PostProcessVolume (unbound) for exposure and color grading so distance reads less punchy.
	APostProcessVolume* PPVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* P = *It;
		if (P && P->bUnbound)
		{
			PPVolume = P;
			break;
		}
	}
	if (!PPVolume)
	{
		// Prefer finding any post process volume and making it unbound
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			PPVolume = *It;
			break;
		}
	}
	if (!PPVolume)
	{
		UE_LOG(LogTemp, Log, TEXT("No PostProcessVolume found - spawning for exposure/color grading"));
		APostProcessVolume* SpawnedPP = World->SpawnActor<APostProcessVolume>(
			APostProcessVolume::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
		if (SpawnedPP)
		{
			SpawnedPP->bUnbound = true;
			FPostProcessSettings& PPS = SpawnedPP->Settings;
			PPS.bOverride_AutoExposureMinBrightness = true;
			PPS.AutoExposureMinBrightness = 1.0f;  // Locked exposure ??? matches asset-preview consistency
			PPS.bOverride_AutoExposureMaxBrightness = true;
			PPS.AutoExposureMaxBrightness = 1.0f;  // Same as min = no auto-exposure variation
			PPS.bOverride_AmbientOcclusionIntensity = true;
			PPS.AmbientOcclusionIntensity = 0.0f;  // AO off ??? eliminates dark creases on characters
			PPS.bOverride_ColorSaturation = true;
			PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.f); // Slight desaturation so scene isn't uniformly punchy
#if WITH_EDITOR
			SpawnedPP->SetActorLabel(TEXT("DEV_PostProcessVolume"));
#endif
			SpawnedSetupActors.Add(SpawnedPP);
		}
	}
	else if (PPVolume)
	{
		// Align level-placed PP with frontend/gameplay defaults so preview and gameplay match.
		PPVolume->bUnbound = true;
		FPostProcessSettings& PPS = PPVolume->Settings;
		PPS.bOverride_AutoExposureMinBrightness = true;
		PPS.AutoExposureMinBrightness = 1.0f;  // Locked exposure
		PPS.bOverride_AutoExposureMaxBrightness = true;
		PPS.AutoExposureMaxBrightness = 1.0f;  // Locked exposure
		PPS.bOverride_AmbientOcclusionIntensity = true;
		PPS.AmbientOcclusionIntensity = 0.0f;  // AO off ??? eliminates dark creases on characters
		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(0.95f, 0.95f, 0.95f, 1.f);
	}

	// Force ALL directional and sky lights to Movable, no shadows, aligned colors (theme sets intensities).
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* DirLight = *It;
		if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(DirLight->GetLightComponent()))
		{
			LC->SetMobility(EComponentMobility::Movable);
			LC->CastShadows = false; // Shadows disabled globally ??? replicates asset-preview look
			LC->bAtmosphereSunLight = true;
			LC->AtmosphereSunLightIndex = DirLight->Tags.Contains(MoonTag) ? 1 : 0;
			LC->SetForwardShadingPriority(DirLight->Tags.Contains(MoonTag) ? 0 : 1); // Sun primary for forward shading
			// Align level-placed light colors with frontend so preview and gameplay match.
			if (DirLight->Tags.Contains(MoonTag))
			{
				LC->SetLightColor(FLinearColor(0.72f, 0.8f, 1.f));
			}
			else
			{
				LC->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
				LC->SetIntensity(3.f); // Fill light ??? SkyLight is primary ambient
			}
		}
		if (USceneComponent* Root = DirLight->GetRootComponent())
		{
			Root->SetMobility(EComponentMobility::Movable);
		}
	}
	ApplyThemeToDirectionalLights();

	// Configure all SkyLights: HDRI cubemap (asset-preview quality) or boosted sky capture fallback.
	// Lumen is disabled; the SkyLight is the primary source of ambient/indirect light.

	// In editor: auto-create the TextureCube from the equirectangular HDRI if it doesn't exist yet.
#if WITH_EDITORONLY_DATA
	EnsureHDRICubemap();
#endif

	UTextureCube* HDRICubemap = LoadObject<UTextureCube>(nullptr, TEXT("/Game/Lighting/TC_HDRI_Studio.TC_HDRI_Studio"));

	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		if (USkyLightComponent* SC = Cast<USkyLightComponent>(It->GetLightComponent()))
		{
			SC->SetMobility(EComponentMobility::Movable);

			if (HDRICubemap)
			{
				// Studio HDRI cubemap: replicates asset-preview lighting quality.
				SC->SourceType = ESkyLightSourceType::SLS_SpecifiedCubemap;
				SC->Cubemap = HDRICubemap;
				SC->SetIntensity(8.0f); // Dominant ambient ??? overshooting intentionally for bright characters
				SC->bLowerHemisphereIsBlack = false;
				SC->SetLowerHemisphereColor(FLinearColor(0.95f, 0.95f, 0.95f)); // Near-white underside fill
				UE_LOG(LogTemp, Log, TEXT("[LIGHT] SkyLight using HDRI cubemap (studio lighting, intensity 8.0)"));
			}
			else
			{
				// Fallback: capture sky atmosphere with boosted intensity + lower hemisphere fill.
				SC->SourceType = ESkyLightSourceType::SLS_CapturedScene;
				SC->SetIntensity(8.0f);
				SC->bLowerHemisphereIsBlack = false;
				SC->SetLowerHemisphereColor(FLinearColor(0.95f, 0.95f, 0.95f));
				UE_LOG(LogTemp, Log, TEXT("[LIGHT] SkyLight using boosted sky capture (no HDRI cubemap found, intensity 8.0)"));
			}

			SC->SetLightColor(FLinearColor::White);
			SC->RecaptureSky();
		}
		if (USceneComponent* Root = It->GetRootComponent())
		{
			Root->SetMobility(EComponentMobility::Movable);
		}
	}

	ApplyThemeToAtmosphereAndLighting();
}

void AT66GameMode::SpawnQuakeSkyIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (T66UsesMainMapTerrainStage(World))
	{
		TArray<AT66QuakeSkyActor*> SkyActorsToDestroy;
		for (TActorIterator<AT66QuakeSkyActor> It(World); It; ++It)
		{
			SkyActorsToDestroy.Add(*It);
		}
		for (AT66QuakeSkyActor* SkyActor : SkyActorsToDestroy)
		{
			if (SkyActor)
			{
				SkyActor->Destroy();
			}
		}
		if (SkyActorsToDestroy.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[QuakeSky] Removed %d Quake sky actor(s) for standalone Farm"), SkyActorsToDestroy.Num());
		}
		return;
	}

	for (TActorIterator<AT66QuakeSkyActor> It(World); It; ++It)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66QuakeSkyActor* SkyActor = World->SpawnActor<AT66QuakeSkyActor>(
		AT66QuakeSkyActor::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams);
	if (!SkyActor) return;

#if WITH_EDITOR
	SkyActor->SetActorLabel(TEXT("DEV_QuakeSky"));
#endif
	SpawnedSetupActors.Add(SkyActor);
	UE_LOG(LogTemp, Log, TEXT("[QuakeSky] Spawned retro sky dome"));
}

void AT66GameMode::ConfigureGameplayFogForWorld(UWorld* World)
{
	if (!World) return;

	UGameInstance* GI = World->GetGameInstance();
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const float FogIntensityPercent = PS ? PS->GetFogIntensityPercent() : 55.0f;
	const bool bFogEnabled = !PS || (PS->GetFogEnabled() && FogIntensityPercent > KINDA_SMALL_NUMBER);
	const float FogDensityScale = FMath::Clamp(FogIntensityPercent / 55.0f, 0.0f, 100.0f / 55.0f);

	struct FT66FogTuning
	{
		float BaseFogDensity = 0.0225f;
		float FogStartDistance = 1400.f;
		float FogHeightFalloff = 0.085f;
		float FogMaxOpacity = 0.96f;
		float DirectionalExponent = 10.0f;
		float DirectionalStartDistance = 2600.0f;
		float FogCutoffDistance = 0.0f;
		FLinearColor FogColor = FLinearColor(0.05f, 0.06f, 0.08f);
		FLinearColor DirectionalFogColor = FLinearColor(0.14f, 0.18f, 0.24f);
	};

	FT66FogTuning FogTuning;
	FogTuning.BaseFogDensity = 0.0105f;
	FogTuning.FogStartDistance = 6200.f;
	FogTuning.FogHeightFalloff = 0.008f;
	FogTuning.FogMaxOpacity = 1.0f;
	FogTuning.DirectionalExponent = 1.6f;
	FogTuning.DirectionalStartDistance = 6800.0f;
	FogTuning.FogColor = FLinearColor(0.0f, 0.53f, 0.60f);
	FogTuning.DirectionalFogColor = FLinearColor(0.10f, 0.64f, 0.74f);

	const float FogDensity = FogTuning.BaseFogDensity * FogDensityScale;

	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		UExponentialHeightFogComponent* FogComp = It->FindComponentByClass<UExponentialHeightFogComponent>();
		if (!FogComp) FogComp = Cast<UExponentialHeightFogComponent>(It->GetRootComponent());
		if (!FogComp) continue;

		FogComp->SetStartDistance(FogTuning.FogStartDistance);
		FogComp->SetFogDensity(FogDensity);
		FogComp->SetFogHeightFalloff(FogTuning.FogHeightFalloff);
		FogComp->SetFogMaxOpacity(FogTuning.FogMaxOpacity);
		FogComp->SetFogCutoffDistance(FogTuning.FogCutoffDistance);
		FogComp->SetFogInscatteringColor(FogTuning.FogColor);
		FogComp->SetDirectionalInscatteringColor(FogTuning.DirectionalFogColor);
		FogComp->SetDirectionalInscatteringExponent(FogTuning.DirectionalExponent);
		FogComp->SetDirectionalInscatteringStartDistance(FogTuning.DirectionalStartDistance);
		FogComp->SetSecondFogDensity(0.0f);
		FogComp->SetSecondFogHeightFalloff(0.0f);
		FogComp->SetSecondFogHeightOffset(0.0f);
		FogComp->SetVolumetricFog(false);
		FogComp->SetVisibility(bFogEnabled);
		break;
	}

	if (GI)
	{
		if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
		{
			FT66RetroFXSettings RetroSettings = PS ? PS->GetRetroFXSettings() : FT66RetroFXSettings();
			if (T66UsesMainMapTerrainStage(World))
			{
				RetroSettings.bEnableWorldGeometry = false;
				RetroSettings.WorldVertexSnapPercent = 0.0f;
				RetroSettings.WorldVertexSnapResolutionPercent = 0.0f;
				RetroSettings.WorldVertexNoisePercent = 0.0f;
				RetroSettings.WorldAffineBlendPercent = 0.0f;
				RetroSettings.WorldAffineDistance1Percent = 0.0f;
				RetroSettings.WorldAffineDistance2Percent = 0.0f;
				RetroSettings.WorldAffineDistance3Percent = 0.0f;
				if (RetroSettings.bEnableRetroFXMaster)
				{
					RetroSettings.PS1FogPercent = 100.0f;
					RetroSettings.PS1SceneDepthFogPercent = 100.0f;
					RetroSettings.PS1FogDensityPercent = 56.0f;
					RetroSettings.PS1FogStartDistancePercent = 48.0f;
					RetroSettings.PS1FogFallOffDistancePercent = 44.0f;
				}
			}
			RetroFX->ApplySettings(RetroSettings, World);
		}
	}
}

void AT66GameMode::ApplyThemeToDirectionalLightsForWorld(UWorld* World)
{
	if (!World) return;

	static const FName MoonTag(TEXT("T66Moon"));
	ADirectionalLight* SunLight = nullptr;
	ADirectionalLight* MoonLight = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* L = *It;
		if (L->Tags.Contains(MoonTag))
			MoonLight = L;
		else
			SunLight = L;
	}
	if (!SunLight) return;

	UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent());
	if (!SunComp) return;

	// Demo map (e.g. Map_Summer): lower sun intensity so it's not as harsh
	FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
	const bool bDemoMap = MapName.Equals(UT66GameInstance::GetDemoMapLevelNameForTribulation().ToString(), ESearchCase::IgnoreCase);
	const float SunIntensityDark = bDemoMap ? 1.2f : 2.f;

	// Dark-only lighting: the visible sky light comes from the eclipse/moon light.
	SunComp->SetIntensity(0.f);
	SunComp->bAtmosphereSunLight = false;
	if (MoonLight)
	{
		UDirectionalLightComponent* MoonComp = Cast<UDirectionalLightComponent>(MoonLight->GetLightComponent());
		if (MoonComp)
		{
			// Eclipse dusk: dimmed sun, warm white light, no shadows, hide built-in sun disk
			MoonComp->SetIntensity(5.0f);
			MoonComp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f));
			MoonComp->bAtmosphereSunLight = true;
			MoonComp->AtmosphereSunLightIndex = 0;
			MoonComp->SetAtmosphereSunDiskColorScale(FLinearColor(0.f, 0.f, 0.f));
			MoonComp->SetCastShadows(false);
			MoonLight->SetActorRotation(FRotator(-50.f, 135.f, 0.f));
		}
	}
}

void AT66GameMode::ApplyThemeToDirectionalLights()
{
	ApplyThemeToDirectionalLightsForWorld(GetWorld());
}

void AT66GameMode::ApplyThemeToAtmosphereAndLightingForWorld(UWorld* World)
{
	if (!World) return;

	// --- SkyAtmosphere: blood-red eclipse sky ---
	for (TActorIterator<ASkyAtmosphere> It(World); It; ++It)
	{
		USkyAtmosphereComponent* Atmos = It->FindComponentByClass<USkyAtmosphereComponent>();
		if (!Atmos) continue;

		Atmos->RayleighScattering = FLinearColor(0.028f, 0.005f, 0.004f);
		Atmos->RayleighScatteringScale = 0.8f;
		Atmos->MieScatteringScale = 0.01f;
		Atmos->MultiScatteringFactor = 0.5f;
		Atmos->MarkRenderStateDirty();
		break;
	}

	// --- SkyLight: captured scene with red tint ---
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		USkyLightComponent* SC = Cast<USkyLightComponent>(It->GetLightComponent());
		if (!SC) continue;

		SC->SourceType = ESkyLightSourceType::SLS_CapturedScene;
		SC->Cubemap = nullptr;
		SC->SetIntensity(4.0f);
		SC->SetLightColor(FLinearColor(0.25f, 0.2f, 0.25f));
		SC->bLowerHemisphereIsBlack = false;
		SC->SetLowerHemisphereColor(FLinearColor(0.12f, 0.1f, 0.12f));
		SC->RecaptureSky();
		break;
	}

	ConfigureGameplayFogForWorld(World);

	// --- PostProcess color grading: neutral (red comes from sky, not color grade) ---
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		if (!It->bUnbound) continue;
		FPostProcessSettings& PPS = It->Settings;

		PPS.bOverride_ColorSaturation = true;
		PPS.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.f);
		PPS.bOverride_ColorGamma = false;
		PPS.bOverride_ColorGain = false;
		PPS.bOverride_BloomIntensity = true;
		PPS.BloomIntensity = 0.0f;
		PPS.bOverride_BloomThreshold = true;
		PPS.BloomThreshold = 10.0f;
		break;
	}

	// --- Posterize: always on in the dark presentation ---
	if (UGameInstance* GI = World->GetGameInstance())
	{
		if (UT66PosterizeSubsystem* Post = GI->GetSubsystem<UT66PosterizeSubsystem>())
		{
			Post->SetEnabled(true);
		}
	}

	// --- Eclipse corona: always visible ---
	{
		AT66EclipseActor* ExistingEclipse = nullptr;
		for (TActorIterator<AT66EclipseActor> It(World); It; ++It)
		{
			ExistingEclipse = *It;
			break;
		}

		if (!ExistingEclipse)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<AT66EclipseActor>(AT66EclipseActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		}
	}

}

void AT66GameMode::ApplyThemeToAtmosphereAndLighting()
{
	ApplyThemeToAtmosphereAndLightingForWorld(GetWorld());
}

void AT66GameMode::HandleSettingsChanged()
{
	ApplyThemeToDirectionalLights();
	ApplyThemeToAtmosphereAndLighting();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66RetroFXSubsystem* RetroFX = GI->GetSubsystem<UT66RetroFXSubsystem>())
		{
			if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
			{
				FT66RetroFXSettings RetroSettings = PS->GetRetroFXSettings();
				if (T66UsesMainMapTerrainStage(GetWorld()))
				{
					RetroSettings.bEnableWorldGeometry = false;
					RetroSettings.WorldVertexSnapPercent = 0.0f;
					RetroSettings.WorldVertexSnapResolutionPercent = 0.0f;
					RetroSettings.WorldVertexNoisePercent = 0.0f;
					RetroSettings.WorldAffineBlendPercent = 0.0f;
					RetroSettings.WorldAffineDistance1Percent = 0.0f;
					RetroSettings.WorldAffineDistance2Percent = 0.0f;
					RetroSettings.WorldAffineDistance3Percent = 0.0f;
				}
				RetroFX->ApplySettings(RetroSettings, GetWorld());
			}
			else
			{
				RetroFX->ApplyCurrentSettings(GetWorld());
			}
		}
	}
}

void AT66GameMode::SpawnPlayerStartIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);

	if (bUsingMainMapTerrain)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			(*It)->Destroy();
		}
		return;
	}

	// Check for existing player start
	bool bHasPlayerStart = false;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		bHasPlayerStart = true;
		break;
	}

	if (!bHasPlayerStart)
	{
		UE_LOG(LogTemp, Log, TEXT("No PlayerStart found - spawning development PlayerStart"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UT66GameInstance* GI = GetT66GameInstance();

		// Default spawn:
		// - Demo map (e.g. Map_Summer): origin (0, 0, DefaultSpawnHeight)
		// - Coliseum mode: coliseum arena (timer starts immediately; no pillars)
		// - Normal stage: start area (so timer starts after passing start pillars)
		FVector SpawnLoc;
		FString MapName = UWorld::RemovePIEPrefix(World->GetMapName());
		if (MapName.Equals(UT66GameInstance::GetDemoMapLevelNameForTribulation().ToString(), ESearchCase::IgnoreCase))
		{
			SpawnLoc = FVector(0.f, 0.f, DefaultSpawnHeight);
		}
		else if (bUsingMainMapTerrain)
		{
			const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
			SpawnLoc = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, DefaultSpawnHeight);
		}
		else if (IsColiseumStage())
		{
			SpawnLoc = FVector(ColiseumCenter.X, ColiseumCenter.Y, DefaultSpawnHeight);
		}
		else
		{
			SpawnLoc = T66GameplayLayout::GetStartAreaCenter(DefaultSpawnHeight);
		}

		APlayerStart* Start = World->SpawnActor<APlayerStart>(
			APlayerStart::StaticClass(),
			SpawnLoc,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (Start)
		{
			#if WITH_EDITOR
			Start->SetActorLabel(TEXT("DEV_PlayerStart"));
			#endif
			SpawnedSetupActors.Add(Start);
			UE_LOG(LogTemp, Log, TEXT("Spawned development PlayerStart at %s"), *SpawnLoc.ToString());
		}
	}
}

void AT66GameMode::RestartPlayersMissingPawns()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || PC->GetPawn())
		{
			continue;
		}

		RestartPlayer(PC);
	}
}

void AT66GameMode::SpawnLabFloorIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World || !IsLabLevel()) return;

	// One central floor: ~1/4 of gameplay map (100k: MainHalfExtent 50000 -> Lab half 12500)
	static const FName LabFloorTag(TEXT("T66_Floor_Lab"));
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(LabFloorTag))
		{
			It->Destroy();
			break;
		}
	}

	UStaticMesh* CubeMesh = GetCubeMesh();
	if (!CubeMesh) return;

	const float LabHalfExtent = 12500.f;
	const float LabFloorHeight = 100.f;
	const FVector LabFloorScale(LabHalfExtent * 2.f / 100.f, LabHalfExtent * 2.f / 100.f, LabFloorHeight / 100.f);
	const FVector LabFloorLoc(0.f, 0.f, LabFloorHeight * 0.5f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), LabFloorLoc, FRotator::ZeroRotator, SpawnParams);
	if (!Floor || !Floor->GetStaticMeshComponent()) return;

	Floor->Tags.Add(LabFloorTag);
	T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Movable);
	Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
	Floor->SetActorScale3D(LabFloorScale);
	Floor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Floor->GetStaticMeshComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	T66_SetStaticMeshActorMobility(Floor, EComponentMobility::Static);

	if (GroundFloorMaterials.Num() > 0 && GroundFloorMaterials[0].Get())
	{
		Floor->GetStaticMeshComponent()->SetMaterial(0, GroundFloorMaterials[0].Get());
	}
	SpawnedSetupActors.Add(Floor);
	UE_LOG(LogTemp, Log, TEXT("Spawned Lab central floor (half-extent %.0f, top Z %.0f)"), LabHalfExtent, LabFloorHeight);
}

void AT66GameMode::SpawnLabCollectorIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World || !IsLabLevel()) return;

	for (TActorIterator<AT66LabCollector> It(World); It; ++It)
	{
		return;  // Already have a Collector
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// Place Collector in front of spawn (player spawns at 0,0,120)
	const FVector CollectorLoc(400.f, 0.f, 120.f);
	AT66LabCollector* Collector = World->SpawnActor<AT66LabCollector>(AT66LabCollector::StaticClass(), CollectorLoc, FRotator::ZeroRotator, SpawnParams);
	if (Collector)
	{
		SpawnedSetupActors.Add(Collector);
		UE_LOG(LogTemp, Log, TEXT("Spawned The Collector in Lab"));
	}
}

FVector AT66GameMode::GetRandomLabSpawnLocation() const
{
	// Lab floor: half extent 12500, top Z = 100. Keep margin from edge and min distance between spawns.
	static constexpr float LabHalfExtent = 12500.f;
	static constexpr float Margin = 800.f;
	static constexpr float MinDistBetween = 400.f;
	static constexpr float SpawnZ = 100.f;
	static constexpr int32 MaxAttempts = 20;

	UWorld* World = GetWorld();
	if (!World) return FVector(400.f, 0.f, SpawnZ);

	FRandomStream Rng(FMath::Rand());
	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const float X = Rng.FRandRange(-(LabHalfExtent - Margin), LabHalfExtent - Margin);
		const float Y = Rng.FRandRange(-(LabHalfExtent - Margin), LabHalfExtent - Margin);
		const FVector Candidate(X, Y, SpawnZ);

		bool bTooClose = false;
		for (AActor* A : LabSpawnedActors)
		{
			if (!A) continue;
			if (FVector::Dist2D(Candidate, A->GetActorLocation()) < MinDistBetween)
			{
				bTooClose = true;
				break;
			}
		}
		if (!bTooClose) return Candidate;
	}
	// Fallback: offset from last spawn or default
	if (LabSpawnedActors.Num() > 0 && LabSpawnedActors.Last())
	{
		FVector Last = LabSpawnedActors.Last()->GetActorLocation();
		return Last + FVector(MinDistBetween, 0.f, 0.f);
	}
	return FVector(400.f, 0.f, SpawnZ);
}

UT66GameInstance* AT66GameMode::GetT66GameInstance() const
{
	return Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
}

bool AT66GameMode::TrySnapActorToTerrain(AActor* Actor) const
{
	return TrySnapActorToTerrainAtLocation(Actor, Actor ? Actor->GetActorLocation() : FVector::ZeroVector);
}

bool AT66GameMode::TrySnapActorToTerrainAtLocation(AActor* Actor, const FVector& TraceLocation) const
{
	UWorld* World = GetWorld();
	if (!World || !Actor)
	{
		return false;
	}

	FHitResult GroundHit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66SnapActorToTerrain), false, Actor);
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		Params.AddIgnoredActor(*It);
	}
	const FVector TraceStart(TraceLocation.X, TraceLocation.Y, TraceLocation.Z + 8000.f);
	const FVector TraceEnd(TraceLocation.X, TraceLocation.Y, TraceLocation.Z - 16000.f);
	if (!World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, Params) &&
		!World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		return false;
	}

	float HalfHeight = 0.f;
	if (const UCapsuleComponent* Capsule = Actor->FindComponentByClass<UCapsuleComponent>())
	{
		HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	}

	const FVector GroundedLoc = GroundHit.ImpactPoint + FVector(0.f, 0.f, HalfHeight + 5.f);
	Actor->SetActorLocation(GroundedLoc, false, nullptr, ETeleportType::TeleportPhysics);
	return true;
}

void AT66GameMode::MaintainPlayerTerrainSafety()
{
	UWorld* World = GetWorld();
	if (!World || IsLabLevel() || IsColiseumStage())
	{
		return;
	}

	UT66GameInstance* GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	const FT66MapPreset Preset = T66BuildMainMapPreset(GI);
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(World);
	const float RescueThresholdZ = bUsingMainMapTerrain
		? (T66MainMapTerrain::GetLowestCollisionBottomZ(Preset) - 100.0f)
		: (Preset.ElevationMin - 200.f);
	const float AnchorTraceZ = bUsingMainMapTerrain
		? T66MainMapTerrain::GetTraceZ(Preset)
		: (Preset.ElevationMax + 3000.f);
	const bool bStageTimerActive = RunState && RunState->GetStageTimerActive();

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn)
		{
			continue;
		}

		ACharacter* Character = Cast<ACharacter>(Pawn);
		UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;

		if (!bTerrainCollisionReady)
		{
			if (Movement && Movement->MovementMode != MOVE_None)
			{
				Movement->StopMovementImmediately();
				Movement->SetMovementMode(MOVE_None);
			}
			continue;
		}

		const FVector PawnLoc = Pawn->GetActorLocation();
		const bool bNeedsRescue = PawnLoc.Z <= RescueThresholdZ || (Movement && Movement->MovementMode == MOVE_None);
		if (!bNeedsRescue)
		{
			continue;
		}

		TArray<FVector> RescueAnchors;
		if (bUsingMainMapTerrain)
		{
			if (MainMapRescueAnchorLocations.Num() > 0)
			{
				RescueAnchors = MainMapRescueAnchorLocations;
			}
			else
			{
				const int32 GridSize = T66MainMapTerrain::MakeSettings(Preset).BoardSize;
				RescueAnchors.Reserve(5);
				RescueAnchors.Add(T66MainMapTerrain::GetSpawnLocation(Preset, AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, GridSize / 2, FMath::Max(0, GridSize / 2 - 1), AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, GridSize / 2, FMath::Min(GridSize - 1, GridSize / 2 + 1), AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, FMath::Max(0, GridSize / 2 - 1), GridSize / 2, AnchorTraceZ));
				RescueAnchors.Add(T66MainMapTerrain::GetCellCenter(Preset, FMath::Min(GridSize - 1, GridSize / 2 + 1), GridSize / 2, AnchorTraceZ));
			}
		}
		else if (!bStageTimerActive || PawnLoc.X <= 0.f)
		{
			RescueAnchors.Reserve(4);
			RescueAnchors.Add(T66GameplayLayout::GetStartAreaCenter(AnchorTraceZ));
			RescueAnchors.Add(T66GameplayLayout::GetStartGateLocation(AnchorTraceZ));
		}
		if (!bUsingMainMapTerrain && bStageTimerActive && PawnLoc.X > 0.f)
		{
			RescueAnchors.Add(T66GameplayLayout::GetBossGateLocation(AnchorTraceZ));
			RescueAnchors.Add(T66GameplayLayout::GetBossAreaCenter(AnchorTraceZ));
		}

		bool bRecovered = TrySnapActorToTerrain(Pawn);
		if (!bRecovered)
		{
			RescueAnchors.Sort([PawnLoc](const FVector& A, const FVector& B)
			{
				return FVector::DistSquared2D(A, PawnLoc) < FVector::DistSquared2D(B, PawnLoc);
			});

			for (const FVector& Anchor : RescueAnchors)
			{
				if (TrySnapActorToTerrainAtLocation(Pawn, Anchor))
				{
					bRecovered = true;
					break;
				}
			}
		}

		if (Movement)
		{
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(bRecovered ? MOVE_Walking : MOVE_None);
		}
	}
}

void AT66GameMode::SnapPlayersToTerrain()
{
	UWorld* World = GetWorld();
	if (!World || !bTerrainCollisionReady)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn)
		{
			continue;
		}

		const bool bSnapped = TrySnapActorToTerrain(Pawn);
		if (!bSnapped)
		{
			continue;
		}

		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
				if (Movement->MovementMode == MOVE_None)
				{
					Movement->SetMovementMode(MOVE_Walking);
				}
			}
		}
	}
}

void AT66GameMode::RegenerateMainMapTerrain(int32 Seed)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66PropSubsystem* PropSub = GI->GetSubsystem<UT66PropSubsystem>())
		{
			PropSub->ClearProps();
		}
	}

	bTerrainCollisionReady = false;
	bMainMapCombatStarted = false;
	bHasMainMapSpawnSurfaceLocation = false;
	MainMapSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapStartAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapStartPathSurfaceLocation = FVector::ZeroVector;
	MainMapStartAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapBossAnchorSurfaceLocation = FVector::ZeroVector;
	MainMapBossSpawnSurfaceLocation = FVector::ZeroVector;
	MainMapBossBeaconSurfaceLocation = FVector::ZeroVector;
	MainMapBossAreaCenterSurfaceLocation = FVector::ZeroVector;
	MainMapRescueAnchorLocations.Reset();

	DestroyActorsWithTag(World, T66MapPlatformTag);
	DestroyActorsWithTag(World, T66MapRampTag);
	DestroyActorsWithTag(World, T66FloorMainTag);
	DestroyActorsWithTag(World, T66TraversalBarrierTag);

	if (StartGate)
	{
		StartGate->Destroy();
		StartGate = nullptr;
	}
	if (BossGate)
	{
		BossGate->Destroy();
		BossGate = nullptr;
	}
	if (CowardiceGate)
	{
		CowardiceGate->Destroy();
		CowardiceGate = nullptr;
	}
	if (TricksterNPC)
	{
		TricksterNPC->Destroy();
		TricksterNPC = nullptr;
	}
	if (AT66BossBase* ExistingBoss = StageBoss.Get())
	{
		ExistingBoss->Destroy();
	}
	StageBoss = nullptr;
	DestroyBossBeacon();

	UT66GameInstance* GI = GetT66GameInstance();
	if (GI)
	{
		GI->RunSeed = Seed;
	}

	SpawnMainMapTerrain();
	if (T66UsesMainMapTerrainStage(World))
	{
		SpawnTricksterAndCowardiceGate();
		SpawnBossForCurrentStage();
	}
	else
	{
		SpawnBossBeaconIfNeeded();
	}
}

// Console command: T66.MainMap [seed]
static FAutoConsoleCommandWithWorldAndArgs T66MainMapCmd(
	TEXT("T66.MainMap"),
	TEXT("Regenerate the difficulty-driven main map terrain. Usage: T66.MainMap [seed]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			if (!World) return;
			AGameModeBase* GM = World->GetAuthGameMode();
			AT66GameMode* T66GM = Cast<AT66GameMode>(GM);
			if (!T66GM)
			{
				UE_LOG(LogTemp, Error, TEXT("T66.MainMap: no T66GameMode active"));
				return;
			}

			int32 Seed = FMath::Rand();
			if (Args.Num() >= 1)
			{
				Seed = FCString::Atoi(*Args[0]);
			}

			UE_LOG(LogTemp, Log, TEXT("T66.MainMap: Regenerating main map terrain (seed=%d)"), Seed);
			T66GM->RegenerateMainMapTerrain(Seed);
		})
);

UClass* AT66GameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// Check if we have a specific hero class from the DataTable
	if (UT66GameInstance* GI = GetT66GameInstance())
	{
		FHeroData HeroData;
		if (GI->GetSelectedHeroData(HeroData))
		{
			// If the hero has a specific class defined, use that
			if (!HeroData.HeroClass.IsNull())
			{
				// Avoid sync loads during spawn; prefer already-loaded class, otherwise fall back safely.
				if (UClass* HeroClass = HeroData.HeroClass.Get())
				{
					return HeroClass;
				}

				// Kick off an async load so subsequent spawns can use the intended class without hitching.
				TSharedPtr<FStreamableHandle> Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
					HeroData.HeroClass.ToSoftObjectPath(),
					FStreamableDelegate());
				if (Handle.IsValid())
				{
					ActiveAsyncLoadHandles.Add(Handle);
				}
			}
		}
	}

	// Fall back to default hero class
	return DefaultHeroClass ? DefaultHeroClass.Get() : AT66HeroBase::StaticClass();
}

APawn* AT66GameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// Get the pawn class
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		UE_LOG(LogTemp, Error, TEXT("No pawn class for spawning!"));
		return nullptr;
	}

	// Get spawn transform - use a safe default height if no PlayerStart exists
	FVector SpawnLocation;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	const bool bUsingMainMapTerrain = T66UsesMainMapTerrainStage(GetWorld());

	if (bUsingMainMapTerrain && !bTerrainCollisionReady)
	{
		UE_LOG(LogTemp, Log, TEXT("Deferring main-map hero spawn until terrain collision is ready."));
		return nullptr;
	}

	if (bUsingMainMapTerrain && bHasMainMapSpawnSurfaceLocation)
	{
		SpawnLocation = MainMapSpawnSurfaceLocation + FVector(0.f, 0.f, DefaultSpawnHeight);
		SpawnRotation = FRotator::ZeroRotator;
	}
	else if (StartSpot)
	{
		SpawnLocation = StartSpot->GetActorLocation();
		SpawnRotation = StartSpot->GetActorRotation();
		UE_LOG(LogTemp, Log, TEXT("Spawning at PlayerStart: (%.1f, %.1f, %.1f)"), 
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}
	else
	{
		// No PlayerStart found - spawn at a safe default location (100k map or demo map).
		UWorld* World = GetWorld();
		FString MapName = World ? UWorld::RemovePIEPrefix(World->GetMapName()) : FString();
		if (MapName.Equals(UT66GameInstance::GetDemoMapLevelNameForTribulation().ToString(), ESearchCase::IgnoreCase))
		{
			SpawnLocation = FVector(0.f, 0.f, 200.f);
		}
		else
		{
			SpawnLocation = IsColiseumStage()
				? FVector(ColiseumCenter.X, ColiseumCenter.Y, 200.f)
				: FVector(-35455.f, 0.f, 200.f);
		}
		UE_LOG(LogTemp, Warning, TEXT("No PlayerStart found! Spawning at default location (%.0f, %.0f, %.0f)."),
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}

	// Robust: always ensure Gameplay spawns in the Start Area (scaled for 100k map).
	if (IsLabLevel())
	{
		SpawnLocation = FVector(0.f, 0.f, 120.f);
		SpawnRotation = FRotator::ZeroRotator;
	}
	else if (IsDemoMapForTribulation())
	{
		SpawnLocation = FVector(0.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;
	}
	else if (bUsingMainMapTerrain)
	{
		if (bHasMainMapSpawnSurfaceLocation)
		{
			SpawnLocation = MainMapSpawnSurfaceLocation + FVector(0.f, 0.f, 200.f);
		}
		else
		{
			const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
			SpawnLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, 200.f);
		}
		SpawnRotation = FRotator::ZeroRotator;
	}
	else if (!IsColiseumStage())
	{
		SpawnLocation = FVector(-35455.f, 0.f, 200.f);
		SpawnRotation = FRotator::ZeroRotator;

		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (T66IsStandaloneTutorialMap(GetWorld()) || bForceSpawnInTutorialArea)
			{
				SpawnLocation = FVector(-3636.f, 56818.f, 200.f);
				SpawnRotation = FRotator::ZeroRotator;
			}
		}
	}
	else
	{
		// Forced coliseum mode uses an enclosed arena off to the side (not the main arena).
		if (UT66GameInstance* T66GI = GetT66GameInstance())
		{
			if (T66GI->bForceColiseumMode)
			{
				SpawnLocation = ColiseumCenter;
				SpawnRotation = FRotator::ZeroRotator;
			}
		}
	}

	// Sky-drop: spawn the hero at a comfortable altitude; gravity + Landed() handle the rest.
	// This avoids trace-before-terrain-exists timing issues and gives a cinematic entrance.

	// Spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.SpawnCollisionHandlingOverride = bUsingMainMapTerrain
		? ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		: ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the pawn
	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (SpawnedPawn)
	{
		const bool bNeedsProceduralTerrain = !IsLabLevel() && !IsColiseumStage();
		if (bUsingMainMapTerrain && bTerrainCollisionReady)
		{
			float HalfHeight = 0.f;
			if (const UCapsuleComponent* Capsule = SpawnedPawn->FindComponentByClass<UCapsuleComponent>())
			{
				HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			}

			FVector ExactSpawnLocation = SpawnLocation;
			if (bHasMainMapSpawnSurfaceLocation)
			{
				ExactSpawnLocation = MainMapSpawnSurfaceLocation + FVector(0.f, 0.f, HalfHeight + 5.f);
			}
			else
			{
				const FT66MapPreset Preset = T66BuildMainMapPreset(GetT66GameInstance());
				ExactSpawnLocation = T66MainMapTerrain::GetPreferredSpawnLocation(Preset, HalfHeight + 5.f);
			}
			SpawnedPawn->SetActorLocation(ExactSpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
			if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
			{
				if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->SetMovementMode(MOVE_Walking);
				}
			}
		}
		else if (bTerrainCollisionReady)
		{
			TrySnapActorToTerrain(SpawnedPawn);
		}
		else if (bNeedsProceduralTerrain)
		{
			if (ACharacter* Character = Cast<ACharacter>(SpawnedPawn))
			{
				if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
				{
					Movement->StopMovementImmediately();
					Movement->SetMovementMode(MOVE_None);
				}
			}
		}
	}

	// If it's our hero class, initialize it with hero data and body type
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(SpawnedPawn))
	{

		if (UT66GameInstance* GI = GetT66GameInstance())
		{
			if (T66_ShouldApplyCombatIterationSetup(GetWorld(), GI, GI->GetSubsystem<UT66RunStateSubsystem>()))
			{
				T66_SeedCombatIterationHeroSelection(GI);
			}

			FHeroData HeroData;
			const FName EffectiveHeroID = GI->SelectedHeroID;

			if (!EffectiveHeroID.IsNone() && GI->GetHeroData(EffectiveHeroID, HeroData))
			{
				// Pass hero data, body type, and skin from Game Instance
				ET66BodyType SelectedBodyType = GI->SelectedHeroBodyType;
				FName SelectedSkinID = GI->SelectedHeroSkinID.IsNone() ? FName(TEXT("Default")) : GI->SelectedHeroSkinID;
				Hero->InitializeHero(HeroData, SelectedBodyType, SelectedSkinID, false);

				UE_LOG(LogTemp, Log, TEXT("Spawned hero: %s (%s), BodyType: %s, Skin: %s, Color: (%.2f, %.2f, %.2f)"),
					*HeroData.DisplayName.ToString(),
					*EffectiveHeroID.ToString(),
					SelectedBodyType == ET66BodyType::TypeA ? TEXT("TypeA") : TEXT("TypeB"),
					*SelectedSkinID.ToString(),
					HeroData.PlaceholderColor.R,
					HeroData.PlaceholderColor.G,
					HeroData.PlaceholderColor.B);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No hero selected in Game Instance - spawning with defaults"));
			}
		}
	}

	return SpawnedPawn;
}

AT66HeroBase* AT66GameMode::SpawnSelectedHero(AController* Controller)
{
	// Find a player start
	AActor* StartSpot = FindPlayerStart(Controller);

	// Use our spawn logic
	APawn* SpawnedPawn = SpawnDefaultPawnFor(Controller, StartSpot);

	// Possess the pawn
	if (SpawnedPawn && Controller)
	{
		Controller->Possess(SpawnedPawn);
	}

	return Cast<AT66HeroBase>(SpawnedPawn);
}

// ============================================
// The Lab: spawn / reset
// ============================================

FVector AT66GameMode::GetLabSpawnLocation() const
{
	UWorld* World = GetWorld();
	if (!World) return FVector(400.f, 0.f, 200.f);
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			APawn* Pawn = PC->GetPawn();
			if (Pawn)
			{
				FVector Loc = Pawn->GetActorLocation();
				FVector Fwd = Pawn->GetActorForwardVector();
				Fwd.Z = 0.f;
				Fwd.Normalize();
				return Loc + Fwd * 500.f + FVector(0.f, 0.f, 0.f);
			}
		}
	}
	return FVector(400.f, 0.f, 200.f);
}

AActor* AT66GameMode::SpawnLabMob(FName CharacterVisualID)
{
	if (!IsLabLevel()) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	TSubclassOf<AT66EnemyBase> ClassToSpawn;
	if (CharacterVisualID == FName(TEXT("GoblinThief")))
		ClassToSpawn = AT66GoblinThiefEnemy::StaticClass();
	else
		ClassToSpawn = AT66EnemyBase::StaticClass();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AT66EnemyBase* Mob = World->SpawnActor<AT66EnemyBase>(ClassToSpawn, Loc, Rot, SpawnParams);
	if (Mob)
	{
		if (!CharacterVisualID.IsNone())
		{
			Mob->CharacterVisualID = CharacterVisualID;
		}
		LabSpawnedActors.Add(Mob);
	}
	return Mob;
}

AActor* AT66GameMode::SpawnLabBoss(FName BossID)
{
	if (!IsLabLevel()) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	UT66GameInstance* GI = GetT66GameInstance();
	if (!GI) return nullptr;

	FBossData BossData;
	if (!GI->GetBossData(BossID, BossData)) return nullptr;

	UClass* ClassToSpawn = BossData.BossClass.Get();
	if (!ClassToSpawn)
	{
		ClassToSpawn = AT66BossBase::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AT66BossBase* Boss = World->SpawnActor<AT66BossBase>(ClassToSpawn, Loc, Rot, SpawnParams);
	if (Boss)
	{
		Boss->InitializeBoss(BossData);
		Boss->bAwakened = true;
		Boss->CurrentHP = Boss->MaxHP;
		LabSpawnedActors.Add(Boss);
	}
	return Boss;
}

AActor* AT66GameMode::SpawnLabFountainOfLife()
{
	if (!IsLabLevel()) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AT66FountainOfLifeInteractable* Fountain = World->SpawnActor<AT66FountainOfLifeInteractable>(AT66FountainOfLifeInteractable::StaticClass(), Loc, Rot, SpawnParams);
	if (Fountain)
	{
		Fountain->SetRarity(ET66Rarity::Black);
		LabSpawnedActors.Add(Fountain);
	}
	return Fountain;
}

AActor* AT66GameMode::SpawnLabInteractable(FName InteractableID)
{
	if (!IsLabLevel()) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	UClass* ClassToSpawn = nullptr;
	if (InteractableID == FName(TEXT("Fountain")))
		ClassToSpawn = AT66FountainOfLifeInteractable::StaticClass();
	else if (InteractableID == FName(TEXT("Chest")))
		ClassToSpawn = AT66ChestInteractable::StaticClass();
	else if (InteractableID == FName(TEXT("WheelSpin")))
		ClassToSpawn = AT66WheelSpinInteractable::StaticClass();
	else if (InteractableID == FName(TEXT("IdolAltar")))
		ClassToSpawn = AT66IdolAltar::StaticClass();
	else if (InteractableID == FName(TEXT("Crate")))
		ClassToSpawn = AT66CrateInteractable::StaticClass();
	else if (InteractableID == FName(TEXT("QuickReviveVending")))
		ClassToSpawn = AT66QuickReviveVendingMachine::StaticClass();
	if (!ClassToSpawn) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector Loc = GetRandomLabSpawnLocation();
	FRotator Rot = FRotator::ZeroRotator;
	AActor* Spawned = World->SpawnActor(ClassToSpawn, &Loc, &Rot, SpawnParams);
	if (Spawned)
	{
		LabSpawnedActors.Add(Spawned);
	}
	return Spawned;
}

void AT66GameMode::ResetLabSpawnedActors()
{
	for (AActor* A : LabSpawnedActors)
	{
		if (A)
		{
			A->Destroy();
		}
	}
	LabSpawnedActors.Empty();
}

