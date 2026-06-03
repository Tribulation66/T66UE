// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

#include "Core/T66ReleaseVariantSubsystem.h"
#include "Core/T66ShelvedFeatureGate.h"
#include "Gameplay/T66BossHazardSubsystem.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66ProjectileManagerSubsystem.h"
#include "Gameplay/T66VendorBoss.h"
#include "UI/T66CasinoVendorTabWidget.h"
#include "UI/WidgetGames/T66WidgetGameRegistry.h"

using namespace T66GameModePrivate;

namespace
{
	static const FName T66TowerMapTerrainVisualTag(TEXT("T66_MainMapTerrain_Visual"));
	static const FName T66TowerMapTerrainCollisionProxyTag(TEXT("T66_MainMapTerrain_CollisionProxy"));
	static const FName T66TowerDescentGuardianTag(TEXT("T66_Tower_DescentGuardian"));
	static const FName T66PlacedTowerMinibossMobID(TEXT("Slime"));
	static constexpr float T66PlacedTowerMinibossHPScalar = 3.0f;
	static constexpr float T66PlacedTowerMinibossDamageScalar = 2.0f;
	static constexpr float T66PlacedTowerMinibossScale = 1.75f;
	static const TCHAR* T66TowerTerrainFloorTagPrefix = TEXT("T66_Floor_Tower_");

	static int32 T66ReadTerrainFloorTag(const AActor* Actor)
	{
		if (!Actor)
		{
			return INDEX_NONE;
		}

		for (const FName& Tag : Actor->Tags)
		{
			const FString TagString = Tag.ToString();
			if (!TagString.StartsWith(T66TowerTerrainFloorTagPrefix))
			{
				continue;
			}

			const FString NumberString = TagString.RightChop(FCString::Strlen(T66TowerTerrainFloorTagPrefix));
			if (NumberString.Len() != 2 || !NumberString.IsNumeric())
			{
				continue;
			}

			return FCString::Atoi(*NumberString);
		}

		return INDEX_NONE;
	}

	static void T66SetTowerTerrainVisualFloor(UWorld* World, const int32 VisibleFloorNumber)
	{
		if (!World || VisibleFloorNumber == INDEX_NONE)
		{
			return;
		}

		int32 ChangedVisualActors = 0;
		int32 ChangedCollisionProxyActors = 0;
		// Floor-transition visibility/collision pass only; not part of per-frame gameplay.
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			const bool bIsVisualActor = Actor->ActorHasTag(T66TowerMapTerrainVisualTag);
			const bool bIsCollisionProxyActor = Actor->ActorHasTag(T66TowerMapTerrainCollisionProxyTag);
			if (!bIsVisualActor && !bIsCollisionProxyActor)
			{
				continue;
			}

			const int32 ActorFloorNumber = T66ReadTerrainFloorTag(Actor);
			if (ActorFloorNumber == INDEX_NONE)
			{
				continue;
			}

			const bool bActiveFloor = ActorFloorNumber == VisibleFloorNumber;
			if (bIsVisualActor)
			{
				const bool bShouldBeHidden = !bActiveFloor;
				if (Actor->IsHidden() != bShouldBeHidden)
				{
					Actor->SetActorHiddenInGame(bShouldBeHidden);
					++ChangedVisualActors;
				}
			}

			if (bIsCollisionProxyActor)
			{
				if (Actor->GetActorEnableCollision() != bActiveFloor)
				{
					Actor->SetActorEnableCollision(bActiveFloor);
					++ChangedCollisionProxyActors;
				}
			}
		}

		if (ChangedVisualActors > 0 || ChangedCollisionProxyActors > 0)
		{
			UE_LOG(
				LogT66GameMode,
				Log,
				TEXT("[MAP] Tower terrain active floor %d (visual actors changed=%d, collision proxies changed=%d)."),
				VisibleFloorNumber,
				ChangedVisualActors,
				ChangedCollisionProxyActors);
		}
	}

	static bool T66ValidateTowerGuardianCandidate(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const FVector& Candidate,
		FVector& OutLocation)
	{
		FVector AdjustedCandidate(Candidate.X, Candidate.Y, Floor.SurfaceZ + 120.0f);
		if (T66TowerMapTerrain::FindFloorIndexForLocation(Layout, AdjustedCandidate) != Floor.FloorNumber)
		{
			return false;
		}

		OutLocation = AdjustedCandidate;
		return true;
	}

	static FVector T66ResolveTowerGateGuardianLocation(
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor)
	{
		FVector SpawnLocation = FVector::ZeroVector;
		if (Floor.bHasDropHole && T66ValidateTowerGuardianCandidate(Layout, Floor, Floor.HoleCenter, SpawnLocation))
		{
			return SpawnLocation;
		}

		if (!Floor.ExitPoint.IsNearlyZero() && T66ValidateTowerGuardianCandidate(Layout, Floor, Floor.ExitPoint, SpawnLocation))
		{
			return SpawnLocation;
		}

		return FVector(Floor.Center.X, Floor.Center.Y, Floor.SurfaceZ + 120.0f);
	}

	// Per-gate mega-mob assignment. Each theme owns 12 gate slots (4 local stages x 3 descent gates),
	// each slot mapped to a distinct one of that theme's 12 mobs (EnemyA..EnemyL). The theme's final
	// local-stage row in DT_Stages holds the complete 12-mob roster, so the mapping is data-driven from
	// existing stage data. Slot = (LocalStage-1)*3 + GateIndex, GateIndex from the descent floor.
	static FName T66ResolveTowerGateGuardianMobID(
		UWorld* World,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor,
		const int32 StageNum)
	{
		const int32 GateIndex = Floor.FloorNumber - Layout.FirstMobFloorNumber;
		if (GateIndex < 0 || GateIndex > 2)
		{
			return T66PlacedTowerMinibossMobID;
		}

		UT66GameInstance* T66GI = World ? Cast<UT66GameInstance>(World->GetGameInstance()) : nullptr;
		FStageData StageData;
		if (!T66GI || !T66GI->GetStageData(StageNum, StageData))
		{
			return T66PlacedTowerMinibossMobID;
		}

		const int32 LocalStage = FMath::Clamp(StageData.LocalStageNumber, 1, 4);

		// Prefer the theme's final local-stage roster (all 12 slots populated); fall back to current stage.
		FStageData RosterData = StageData;
		FStageData FinalStageData;
		if (T66GI->GetStageData(StageNum + (4 - LocalStage), FinalStageData))
		{
			RosterData = FinalStageData;
		}

		const FName ThemeMobs[12] =
		{
			RosterData.EnemyA, RosterData.EnemyB, RosterData.EnemyC,
			RosterData.EnemyD, RosterData.EnemyE, RosterData.EnemyF,
			RosterData.EnemyG, RosterData.EnemyH, RosterData.EnemyI,
			RosterData.EnemyJ, RosterData.EnemyK, RosterData.EnemyL,
		};

		const int32 SlotIndex = (LocalStage - 1) * 3 + GateIndex;
		const FName SlotMob = ThemeMobs[SlotIndex];
		return SlotMob.IsNone() ? T66PlacedTowerMinibossMobID : SlotMob;
	}

	static AT66EnemyBase* T66SpawnTowerGateGuardian(
		UWorld* World,
		const T66TowerMapTerrain::FLayout& Layout,
		const T66TowerMapTerrain::FFloor& Floor)
	{
		if (!World)
		{
			return nullptr;
		}

		UGameInstance* GI = World->GetGameInstance();
		UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
		const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
		const FName MobID = T66ResolveTowerGateGuardianMobID(World, Layout, Floor, StageNum);
		const TSubclassOf<AT66EnemyBase> GuardianClass = FT66EnemyFamilyResolver::ResolveEnemyClass(MobID, AT66EnemyBase::StaticClass());
		const FVector SpawnLocation = T66ResolveTowerGateGuardianLocation(Layout, Floor);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66EnemyBase* Guardian = World->SpawnActor<AT66EnemyBase>(GuardianClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (!Guardian)
		{
			UE_LOG(LogT66GameMode, Warning, TEXT("[MAP] Tower placed miniboss spawn failed on floor %d (mob=%s)."), Floor.FloorNumber, *MobID.ToString());
			return nullptr;
		}

		Guardian->Tags.AddUnique(T66TowerDescentGuardianTag);
		Guardian->bDropsLoot = false;
		if (!MobID.IsNone())
		{
			Guardian->ConfigureAsMob(MobID);
		}
		if (RunState)
		{
			Guardian->ApplyStageScaling(StageNum);
			Guardian->ApplyDifficultyScalar(RunState->GetDifficultyScalar());
		}
		Guardian->ApplyMiniBossMultipliers(
			T66PlacedTowerMinibossHPScalar,
			T66PlacedTowerMinibossDamageScalar,
			T66PlacedTowerMinibossScale);
		if (UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>())
		{
			MobManager->RecordBossOrGuardianRouteAttribution(FT66EnemyFamilyResolver::ResolveFamily(MobID));
		}

		T66TrySnapActorToTowerFloor(World, Guardian, Layout, Floor.FloorNumber, Guardian->GetActorLocation());
		if (AT66GameMode* GameMode = World->GetAuthGameMode<AT66GameMode>())
		{
			const int32 ResolvedFloor = GameMode->GetTowerFloorIndexForLocation(Guardian->GetActorLocation());
			if (ResolvedFloor != Floor.FloorNumber)
			{
				UE_LOG(
					LogT66GameMode,
					Warning,
					TEXT("[MAP] Tower placed miniboss rejected after snap: requested floor=%d resolved floor=%d mob=%s loc=%s."),
					Floor.FloorNumber,
					ResolvedFloor,
					*MobID.ToString(),
					*Guardian->GetActorLocation().ToCompactString());
				Guardian->Destroy();
				return nullptr;
			}
		}

		T66AssignTowerFloorTag(Guardian, Floor.FloorNumber);
		UE_LOG(
			LogT66GameMode,
			Log,
			TEXT("[MAP] Tower placed miniboss spawned floor=%d mob=%s hp=%d scale=%.2f loc=%s."),
			Floor.FloorNumber,
			*MobID.ToString(),
			Guardian->MaxHP,
			Guardian->GetActorScale3D().X,
			*Guardian->GetActorLocation().ToCompactString());
		return Guardian;
	}
}

void AT66GameMode::ResetTowerMiasmaState()
{
	bTowerMiasmaActive = false;
	TowerMiasmaStartWorldSeconds = 0.f;
	TowerMiasmaUpdateAccumulator = 0.f;
	TowerIdolSelectionsAtStageStart = 0;

	if (MiasmaManager)
	{
		MiasmaManager->SetExpansionActive(false);
		MiasmaManager->ClearTowerSourceAnchors();
		T66PauseTowerMiasma(MiasmaManager);
	}
}

void AT66GameMode::UpdateTowerMiasma(float DeltaTime)
{
	if (!IsUsingTowerMainMapLayout() || !bTowerMiasmaActive || !MiasmaManager)
	{
		TowerMiasmaUpdateAccumulator = 0.f;
		return;
	}

	TowerMiasmaUpdateAccumulator += DeltaTime;
	if (TowerMiasmaUpdateAccumulator < 0.20f)
	{
		return;
	}

	TowerMiasmaUpdateAccumulator = 0.f;
	MiasmaManager->UpdateFromRunState();
}

void AT66GameMode::TryStartTowerMiasma(const FVector* SourceAnchor, const int32 SourceFloorNumber)
{
	if (!IsUsingTowerMainMapLayout() || bTowerMiasmaActive)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		TowerMiasmaStartWorldSeconds = World->GetTimeSeconds();
	}
	else
	{
		TowerMiasmaStartWorldSeconds = 0.f;
	}

	bTowerMiasmaActive = true;
	TowerMiasmaUpdateAccumulator = 0.20f;

	if (SourceAnchor && SourceFloorNumber != INDEX_NONE)
	{
		SyncTowerMiasmaSourceAnchor(SourceFloorNumber, *SourceAnchor);
	}

	if (MiasmaManager)
	{
		T66ActivateStageMiasma(MiasmaManager);
		MiasmaManager->SetExpansionActive(true, 0.f);
		MiasmaManager->UpdateFromRunState();
	}
}

void AT66GameMode::SyncTowerMiasmaSourceAnchor(const int32 FloorNumber, const FVector& WorldAnchor) const
{
	if (!IsUsingTowerMainMapLayout() || FloorNumber == INDEX_NONE || !MiasmaManager)
	{
		return;
	}

	MiasmaManager->SetTowerSourceAnchor(FloorNumber, WorldAnchor);
}

float AT66GameMode::GetTowerMiasmaElapsedSeconds() const
{
	if (!bTowerMiasmaActive)
	{
		return 0.f;
	}

	const UWorld* World = GetWorld();
	return World ? FMath::Max(World->GetTimeSeconds() - TowerMiasmaStartWorldSeconds, 0.f) : 0.f;
}

#if !UE_BUILD_SHIPPING
bool AT66GameMode::RunContentCorrectionsSmoke(UWorld* ProofWorld)
{
	TArray<FString> FailedChecks;
	auto RecordCheck = [&FailedChecks](const TCHAR* CheckName, const bool bPassed, const FString& Detail)
	{
		UE_LOG(
			LogT66GameMode,
			Log,
			TEXT("[ContentCorrectionsSmoke] Check=%s Result=%s Detail=%s"),
			CheckName,
			bPassed ? TEXT("PASS") : TEXT("FAIL"),
			*Detail);
		if (!bPassed)
		{
			FailedChecks.Add(FString(CheckName));
		}
	};

	UGameInstance* GI = ProofWorld ? ProofWorld->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GetT66GameInstance();
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66ReleaseVariantSubsystem* ReleaseVariant = GI ? GI->GetSubsystem<UT66ReleaseVariantSubsystem>() : nullptr;
	UT66CompanionUnlockSubsystem* CompanionUnlocks = GI ? GI->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr;

	const bool bShelvedGatesPass = !FT66ShelvedFeatureGate::IsDailyDescentEnabled();
	RecordCheck(
		TEXT("ShelvedFeatureGates"),
		bShelvedGatesPass,
		FString::Printf(
			TEXT("DailyDescent=%d"),
			FT66ShelvedFeatureGate::IsDailyDescentEnabled() ? 1 : 0));

	TArray<const FT66WidgetGameDescriptor*> CasinoDescriptors;
	T66WidgetGames::Registry::GetByCategory(ET66WidgetGameCategory::Casino, CasinoDescriptors);
	const TArray<FName> ExpectedCasinoGameIDs = {
		FName(TEXT("Casino_CoinFlip")),
		FName(TEXT("Casino_GuessTheCup")),
		FName(TEXT("Casino_PickLongestShortestStick")),
		FName(TEXT("Casino_FindJoker")),
	};
	const TArray<FName> ExpectedCasinoGateIDs = {
		FName(TEXT("CoinFlip")),
		FName(TEXT("GuessTheCup")),
		FName(TEXT("PickLongestShortestStick")),
		FName(TEXT("FindJoker")),
	};

	int32 CasinoDescriptorMatches = 0;
	int32 CasinoDescriptorAvailable = 0;
	int32 CasinoWidgetClassResolved = 0;
	for (int32 Index = 0; Index < ExpectedCasinoGameIDs.Num(); ++Index)
	{
		const FT66WidgetGameDescriptor* Descriptor = T66WidgetGames::Registry::FindDescriptor(ExpectedCasinoGameIDs[Index]);
		if (Descriptor
			&& Descriptor->Category == ET66WidgetGameCategory::Casino
			&& Descriptor->PlayModel == ET66WidgetGamePlayModel::TurnCasino
			&& Descriptor->LaunchKind == ET66WidgetGameLaunchKind::CasinoChildWidget
			&& Descriptor->DemoGateKind == ET66WidgetGameDemoGateKind::CasinoAllowList
			&& Descriptor->DemoGateID == ExpectedCasinoGateIDs[Index]
			&& Descriptor->Capabilities.bUsesWager)
		{
			++CasinoDescriptorMatches;
		}
		if (Descriptor && T66WidgetGames::Registry::IsAvailable(ProofWorld, *Descriptor))
		{
			++CasinoDescriptorAvailable;
		}
		if (Descriptor && T66WidgetGames::Registry::ResolveWidgetClass(*Descriptor).Get() != nullptr)
		{
			++CasinoWidgetClassResolved;
		}
	}

	const bool bCasinoRegistryPass =
		CasinoDescriptors.Num() == ExpectedCasinoGameIDs.Num()
		&& CasinoDescriptorMatches == ExpectedCasinoGameIDs.Num()
		&& CasinoDescriptorAvailable == ExpectedCasinoGameIDs.Num()
		&& CasinoWidgetClassResolved == ExpectedCasinoGameIDs.Num();
	RecordCheck(
		TEXT("CasinoFourGameRegistry"),
		bCasinoRegistryPass,
		FString::Printf(
			TEXT("Descriptors=%d Expected=%d Matches=%d Available=%d WidgetClasses=%d DemoMode=%d"),
			CasinoDescriptors.Num(),
			ExpectedCasinoGameIDs.Num(),
			CasinoDescriptorMatches,
			CasinoDescriptorAvailable,
			CasinoWidgetClassResolved,
			(ReleaseVariant && ReleaseVariant->IsDemoModeActive()) ? 1 : 0));

	int32 ShopSlotCount = 0;
	int32 ValidShopSlots = 0;
	if (RunState)
	{
		RunState->EnsureShopStockForCurrentStage();
		const TArray<FT66InventorySlot>& ShopSlots = RunState->GetShopStockSlots();
		ShopSlotCount = ShopSlots.Num();
		for (const FT66InventorySlot& Slot : ShopSlots)
		{
			if (!Slot.ItemTemplateID.IsNone()
				&& (Slot.Rarity == ET66ItemRarity::Black
					|| Slot.Rarity == ET66ItemRarity::Red
					|| Slot.Rarity == ET66ItemRarity::Yellow
					|| Slot.Rarity == ET66ItemRarity::White))
			{
				++ValidShopSlots;
			}
		}
	}
	RecordCheck(
		TEXT("ShopFourSlotsAlwaysFilled"),
		RunState && ShopSlotCount == UT66RunStateSubsystem::ShopDisplaySlotCount && ValidShopSlots == UT66RunStateSubsystem::ShopDisplaySlotCount,
		FString::Printf(
			TEXT("Slots=%d ValidSlots=%d Expected=%d"),
			ShopSlotCount,
			ValidShopSlots,
			UT66RunStateSubsystem::ShopDisplaySlotCount));
	RecordCheck(
		TEXT("VendorSellBuybackFourSlotSurfaces"),
		UT66CasinoVendorTabWidget::SellVisibleSlotCount == 4 && UT66RunStateSubsystem::BuybackDisplaySlotCount == 4,
		FString::Printf(
			TEXT("SellVisibleSlots=%d BuybackDisplaySlots=%d"),
			UT66CasinoVendorTabWidget::SellVisibleSlotCount,
			UT66RunStateSubsystem::BuybackDisplaySlotCount));

	static constexpr int32 ShopProofSamples = 100000;
	static constexpr int32 ShopProofSeed = 660066;
	FRandomStream ShopProofRng(ShopProofSeed);
	int32 BlackCount = 0;
	int32 RedCount = 0;
	int32 YellowCount = 0;
	int32 WhiteCount = 0;
	for (int32 Index = 0; Index < ShopProofSamples; ++Index)
	{
		switch (UT66RunStateSubsystem::RollShopSlotRarity(ShopProofRng))
		{
		case ET66ItemRarity::Black:
			++BlackCount;
			break;
		case ET66ItemRarity::Red:
			++RedCount;
			break;
		case ET66ItemRarity::Yellow:
			++YellowCount;
			break;
		case ET66ItemRarity::White:
			++WhiteCount;
			break;
		default:
			break;
		}
	}
	auto ShopRatio = [](const int32 Count)
	{
		return static_cast<float>(Count) / static_cast<float>(ShopProofSamples);
	};
	const float ExpectedBlack = UT66RunStateSubsystem::ShopRarityWeightBlack / UT66RunStateSubsystem::ShopRarityWeightTotal;
	const float ExpectedRed = UT66RunStateSubsystem::ShopRarityWeightRed / UT66RunStateSubsystem::ShopRarityWeightTotal;
	const float ExpectedYellow = UT66RunStateSubsystem::ShopRarityWeightYellow / UT66RunStateSubsystem::ShopRarityWeightTotal;
	const float ExpectedWhite = UT66RunStateSubsystem::ShopRarityWeightWhite / UT66RunStateSubsystem::ShopRarityWeightTotal;
	const bool bShopOddsPass =
		FMath::Abs(ShopRatio(BlackCount) - ExpectedBlack) <= 0.015f &&
		FMath::Abs(ShopRatio(RedCount) - ExpectedRed) <= 0.015f &&
		FMath::Abs(ShopRatio(YellowCount) - ExpectedYellow) <= 0.004f &&
		FMath::Abs(ShopRatio(WhiteCount) - ExpectedWhite) <= 0.002f;
	RecordCheck(
		TEXT("ShopWeightedOdds"),
		bShopOddsPass,
		FString::Printf(
			TEXT("Samples=%d Seed=%d Black=%d %.5f Red=%d %.5f Yellow=%d %.5f White=%d %.5f Expected=%.5f/%.5f/%.5f/%.5f Weights=%.1f/%.1f/%.1f/%.1f"),
			ShopProofSamples,
			ShopProofSeed,
			BlackCount,
			ShopRatio(BlackCount),
			RedCount,
			ShopRatio(RedCount),
			YellowCount,
			ShopRatio(YellowCount),
			WhiteCount,
			ShopRatio(WhiteCount),
			ExpectedBlack,
			ExpectedRed,
			ExpectedYellow,
			ExpectedWhite,
			UT66RunStateSubsystem::ShopRarityWeightBlack,
			UT66RunStateSubsystem::ShopRarityWeightRed,
			UT66RunStateSubsystem::ShopRarityWeightYellow,
			UT66RunStateSubsystem::ShopRarityWeightWhite));

	int32 ExpectedVendorFloors = 0;
	int32 VendorActorsOnMobFloors = 0;
	int32 ExpectedCasinoFloors = 0;
	int32 CasinoActorsOnMobFloors = 0;
	int32 StartFloorExtraInteractables = 0;
	FString StartFloorExtraInteractableNames;
	int32 CasinoInteractableOpenHandled = 0;
	int32 CasinoOverlayOpenAfterInteract = 0;
	if (ProofWorld && IsUsingTowerMainMapLayout())
	{
		ExpectedVendorFloors = FMath::Max(0, CachedTowerMainMapLayout.LastMobFloorNumber - CachedTowerMainMapLayout.FirstMobFloorNumber + 1);
		for (TActorIterator<AT66VendorInteractable> It(ProofWorld); It; ++It)
		{
			const int32 FloorNumber = T66ReadTowerFloorTag(*It);
			if (FloorNumber >= CachedTowerMainMapLayout.FirstMobFloorNumber && FloorNumber <= CachedTowerMainMapLayout.LastMobFloorNumber)
			{
				++VendorActorsOnMobFloors;
			}
		}

		const int32 RunSeed = T66GI ? T66EnsureRunSeed(T66GI) : 0;
		const int32 StageNum = RunState ? RunState->GetCurrentStage() : 1;
		for (int32 FloorNumber = CachedTowerMainMapLayout.FirstMobFloorNumber; FloorNumber <= CachedTowerMainMapLayout.LastMobFloorNumber; ++FloorNumber)
		{
			const int32 CasinoSeed = RunSeed + StageNum * 1901 + 5600 + FloorNumber * 53;
			FRandomStream CasinoFloorRng(CasinoSeed);
			if (CasinoFloorRng.FRand() <= T66TowerCasinoSpawnChance)
			{
				++ExpectedCasinoFloors;
			}
		}
		for (TActorIterator<AT66CasinoInteractable> It(ProofWorld); It; ++It)
		{
			const int32 FloorNumber = T66ReadTowerFloorTag(*It);
			if (FloorNumber >= CachedTowerMainMapLayout.FirstMobFloorNumber && FloorNumber <= CachedTowerMainMapLayout.LastMobFloorNumber)
			{
				++CasinoActorsOnMobFloors;
			}
		}

		for (TActorIterator<AActor> It(ProofWorld); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || T66ReadTowerFloorTag(Actor) != CachedTowerMainMapLayout.StartFloorNumber)
			{
				continue;
			}

			const bool bExtraWorldInteractable =
				Actor->IsA<AT66VendorInteractable>()
				|| Actor->IsA<AT66CasinoInteractable>()
				|| Actor->IsA<AT66ChestInteractable>()
				|| Actor->IsA<AT66CrateInteractable>()
				|| Actor->IsA<AT66LootWheelInteractable>()
				|| Actor->IsA<AT66LootBagPickup>()
				|| Actor->IsA<AT66FountainInteractable>()
				|| Actor->IsA<AT66DifficultyTotem>()
				|| Actor->IsA<AT66VehicleInteractable>()
				|| Actor->IsA<AT66SaintNPC>()
				|| Actor->IsA<AT66OuroborosNPC>();
			if (!bExtraWorldInteractable)
			{
				continue;
			}

			++StartFloorExtraInteractables;
			if (StartFloorExtraInteractableNames.Len() < 256)
			{
				if (!StartFloorExtraInteractableNames.IsEmpty())
				{
					StartFloorExtraInteractableNames += TEXT(",");
				}
				StartFloorExtraInteractableNames += Actor->GetClass()->GetName();
			}
		}

		AT66CasinoInteractable* ProofCasinoInteractable = nullptr;
		for (TActorIterator<AT66CasinoInteractable> It(ProofWorld); It; ++It)
		{
			ProofCasinoInteractable = *It;
			break;
		}

		if (AT66PlayerController* T66PC = Cast<AT66PlayerController>(ProofWorld->GetFirstPlayerController()))
		{
			CasinoInteractableOpenHandled = (ProofCasinoInteractable && T66PC->OpenCasinoGamblerInteractable(ProofCasinoInteractable)) ? 1 : 0;
			CasinoOverlayOpenAfterInteract = T66PC->IsCasinoOverlayOpen() ? 1 : 0;
			T66PC->CloseCasinoOverlay();
		}
	}
	RecordCheck(
		TEXT("VendorGuaranteedPerMobFloor"),
		IsUsingTowerMainMapLayout() && ExpectedVendorFloors > 0 && VendorActorsOnMobFloors == ExpectedVendorFloors,
		FString::Printf(
			TEXT("Vendors=%d Expected=%d Rule=GuaranteedPerMobFloor"),
			VendorActorsOnMobFloors,
			ExpectedVendorFloors));
	RecordCheck(
		TEXT("CasinoPerFloorChance"),
		IsUsingTowerMainMapLayout() && CasinoActorsOnMobFloors == ExpectedCasinoFloors,
		FString::Printf(
			TEXT("Casinos=%d Expected=%d Chance=%.3f Rule=PerFloorChance"),
			CasinoActorsOnMobFloors,
			ExpectedCasinoFloors,
			T66TowerCasinoSpawnChance));
	RecordCheck(
		TEXT("TowerStartFloorReservedForWeaponAltar"),
		IsUsingTowerMainMapLayout() && StartFloorExtraInteractables == 0,
		FString::Printf(
			TEXT("StartFloor=%d ExtraWorldInteractables=%d Classes=%s"),
			CachedTowerMainMapLayout.StartFloorNumber,
			StartFloorExtraInteractables,
			StartFloorExtraInteractableNames.IsEmpty() ? TEXT("None") : *StartFloorExtraInteractableNames));
	RecordCheck(
		TEXT("CasinoInteractableOpensGamblerOverlay"),
		CasinoInteractableOpenHandled == 1 && CasinoOverlayOpenAfterInteract == 1,
		FString::Printf(
			TEXT("OpenHandled=%d OverlayOpen=%d CasinoActors=%d"),
			CasinoInteractableOpenHandled,
			CasinoOverlayOpenAfterInteract,
			CasinoActorsOnMobFloors));

	int32 CagedSpawned = 0;
	int32 CagedFreed = 0;
	int32 CageInteractHandled = 0;
	int32 CageUnlockBranchGranted = 0;
	int32 CageUnlockedAfterInteract = 0;
	FName CageCompanionID = NAME_None;
	const int32 StageBeforeCageProof = RunState ? RunState->GetCurrentStage() : 1;
	if (ProofWorld && RunState && CompanionUnlocks)
	{
		APlayerController* PC = ProofWorld->GetFirstPlayerController();
		RunState->SetCurrentStage(5);
		ClearCagedStageCompanions(true);

		FVector CageAnchor = !MainMapBossSpawnSurfaceLocation.IsNearlyZero()
			? MainMapBossSpawnSurfaceLocation
			: (!CachedTowerMainMapLayout.BossSpawnSurfaceLocation.IsNearlyZero()
				? CachedTowerMainMapLayout.BossSpawnSurfaceLocation
				: FVector(0.f, 0.f, 200.f));

		CagedSpawned = SpawnCagedCompanionsForCurrentStage(CageAnchor);
		CagedFreed = FreeCagedCompanionsForBossClear(CageAnchor);
		AT66RecruitableCompanion* ProofCompanion = nullptr;
		for (const TWeakObjectPtr<AT66RecruitableCompanion>& WeakCompanion : CagedStageCompanions)
		{
			if (AT66RecruitableCompanion* Companion = WeakCompanion.Get())
			{
				ProofCompanion = Companion;
				break;
			}
		}

		if (ProofCompanion)
		{
			CageCompanionID = ProofCompanion->CompanionID;
			CageInteractHandled = (PC && ProofCompanion->Interact(PC)) ? 1 : 0;
			CageUnlockBranchGranted = ProofCompanion->HasGrantedBossCageUnlock() ? 1 : 0;
			CageUnlockedAfterInteract = CompanionUnlocks->IsCompanionUnlocked(CageCompanionID) ? 1 : 0;
		}

		ClearCagedStageCompanions(true);
		RunState->SetCurrentStage(StageBeforeCageProof);
	}
	const bool bCompanionCagePass =
		CagedSpawned > 0 &&
		CagedFreed > 0 &&
		CageInteractHandled &&
		CageUnlockBranchGranted &&
		CageUnlockedAfterInteract;
	RecordCheck(
		TEXT("BossCagedCompanionInteractUnlock"),
		bCompanionCagePass,
		FString::Printf(
			TEXT("Stage=5 CompanionID=%s Spawned=%d Freed=%d InteractHandled=%d UnlockBranchGranted=%d UnlockedAfter=%d RestoredStage=%d"),
			*CageCompanionID.ToString(),
			CagedSpawned,
			CagedFreed,
			CageInteractHandled,
			CageUnlockBranchGranted,
			CageUnlockedAfterInteract,
			StageBeforeCageProof));

	const FName IdolDisplayTag(TEXT("T66_PixalTest_IdolAltar_Display"));
	auto CountIdolDisplayActors = [ProofWorld, IdolDisplayTag]()
	{
		int32 Count = 0;
		if (!ProofWorld)
		{
			return Count;
		}
		for (TActorIterator<AActor> It(ProofWorld); It; ++It)
		{
			if (It->ActorHasTag(IdolDisplayTag))
			{
				++Count;
			}
		}
		return Count;
	};
	const int32 IdolDisplayBefore = CountIdolDisplayActors();
	AT66IdolAltar* ProofIdolAltar = SpawnIdolAltarAtLocation(FVector(1200.f, 0.f, 300.f), true);
	const int32 IdolDisplayAfter = CountIdolDisplayActors();
	RecordCheck(
		TEXT("LiveIdolAltarSpawnsNoDisplayModels"),
		ProofIdolAltar != nullptr && IdolDisplayAfter == IdolDisplayBefore,
		FString::Printf(
			TEXT("AltarSpawned=%d DisplayBefore=%d DisplayAfter=%d"),
			ProofIdolAltar ? 1 : 0,
			IdolDisplayBefore,
			IdolDisplayAfter));
	if (ProofIdolAltar)
	{
		if (IdolAltar == ProofIdolAltar)
		{
			IdolAltar = nullptr;
		}
		ProofIdolAltar->Destroy();
	}

	int32 MobTouchDamageApplied = 0;
	float MobTouchHPBefore = -1.f;
	float MobTouchHPAfter = -1.f;
	if (ProofWorld && RunState)
	{
		AT66PlayerController* PC = Cast<AT66PlayerController>(ProofWorld->GetFirstPlayerController());
		AT66HeroBase* Hero = PC ? Cast<AT66HeroBase>(PC->GetPawn()) : nullptr;
		UT66MobManagerSubsystem* MobManager = ProofWorld->GetSubsystem<UT66MobManagerSubsystem>();
		if (Hero && MobManager)
		{
			Hero->SetVehicleMounted(false);
			if (Hero->IsInSafeZone())
			{
				Hero->AddSafeZoneOverlap(-1000);
			}
			RunState->ApplyAutomationHeroHPOverride(200.f, TEXT("ContentCorrectionsMobTouchDamage"));
			RunState->AutomationResetDamageInvuln();

			FActorSpawnParameters MobSpawnParams;
			MobSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			const FVector MobSpawnLocation = Hero->GetActorLocation() + FVector(36.f, 0.f, 0.f);
			if (AT66MobBase* ProofMob = ProofWorld->SpawnActor<AT66MobBase>(AT66MobBase::StaticClass(), MobSpawnLocation, FRotator::ZeroRotator, MobSpawnParams))
			{
				ProofMob->ConfigureAsMob(FName(TEXT("Slime")), ET66EnemyFamily::Melee, NAME_None, RunState->GetCurrentStage(), RunState->GetDifficultyScalar(), 1.f, RunState->GetFinalSurvivalEnemyScalar(), false);
				ProofMob->TouchDamageHearts = 1;
				ProofMob->TouchDamageCooldownSeconds = 0.f;
				ProofMob->bIsTouchingHero = false;
				MobTouchHPBefore = RunState->GetCurrentHP();
				MobTouchDamageApplied = MobManager->AutomationApplyMobTouchDamageForTest(ProofMob, Hero, 0.1f) ? 1 : 0;
				MobTouchHPAfter = RunState->GetCurrentHP();
				ProofMob->Destroy();
			}
		}
	}
	RecordCheck(
		TEXT("EnemyMobTouchDamageApplies"),
		MobTouchDamageApplied && MobTouchHPAfter < MobTouchHPBefore,
		FString::Printf(
			TEXT("Applied=%d HP=%.1f->%.1f"),
			MobTouchDamageApplied,
			MobTouchHPBefore,
			MobTouchHPAfter));

	const bool bPass = FailedChecks.Num() == 0;
	if (bPass)
	{
		UE_LOG(
			LogT66GameMode,
			Log,
			TEXT("[ContentCorrectionsSmokeSummary] Terminal=1 Pass=1 Failures=0 FailedChecks=%s"),
			*FString::Join(FailedChecks, TEXT("|")));
	}
	else
	{
		UE_LOG(
			LogT66GameMode,
			Warning,
			TEXT("[ContentCorrectionsSmokeSummary] Terminal=1 Pass=0 Failures=%d FailedChecks=%s"),
			FailedChecks.Num(),
			*FString::Join(FailedChecks, TEXT("|")));
	}
	return bPass;
}
#endif

void AT66GameMode::SpawnTowerDescentHolesIfNeeded()
{
	if (!IsUsingTowerMainMapLayout())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || TowerDescentHoles.Num() > 0)
	{
		return;
	}

#if !UE_BUILD_SHIPPING
	auto ScheduleVerificationProofIfRequested = [this, World]()
	{
		FString AutomationMode;
		if (!World || !FParse::Value(FCommandLine::Get(), TEXT("T66GameplayAutoCapture="), AutomationMode))
		{
			return;
		}

		AutomationMode = AutomationMode.TrimStartAndEnd().ToLower();
		if (AutomationMode != TEXT("minibosstraversalproof")
			&& AutomationMode != TEXT("bossprojectilekillmidflightproof")
			&& AutomationMode != TEXT("managedprojectilevisualproof")
			&& AutomationMode != TEXT("bossprojectilevisualprofileproof")
			&& AutomationMode != TEXT("bosshazarddefinitionproof")
			&& AutomationMode != TEXT("bosshazarddamageproof")
			&& AutomationMode != TEXT("vendorshopcorrectionsproof")
			&& AutomationMode != TEXT("loansharkdebtproof")
			&& AutomationMode != TEXT("contentcorrectionssmoke"))
		{
			return;
		}

		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, AutomationMode]()
		{
			UWorld* ProofWorld = GetWorld();
			if (!ProofWorld)
			{
				return;
			}

			if (AutomationMode == TEXT("bossprojectilekillmidflightproof"))
			{
				const bool bPass = [&]()
				{
					if (UT66ProjectileManagerSubsystem* ProjectileManager = ProofWorld->GetSubsystem<UT66ProjectileManagerSubsystem>())
					{
						return ProjectileManager->RunBossProjectileKillMidFlightProof();
					}
					return false;
				}();

				if (!bPass)
				{
					UE_LOG(LogT66GameMode, Warning, TEXT("[BossProjectileKillMidFlightProofSummary] Terminal=1 Pass=0 Reason=MissingProjectileManagerOrProofFailed"));
				}
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("BossProjectileKillMidFlightProofComplete"));
				return;
			}

			if (AutomationMode == TEXT("contentcorrectionssmoke"))
			{
				const bool bPass = RunContentCorrectionsSmoke(ProofWorld);
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("T66ContentCorrectionsSmokeComplete"));
				return;
			}

			if (AutomationMode == TEXT("managedprojectilevisualproof"))
			{
				const bool bPass = [&]()
				{
					if (UT66ProjectileManagerSubsystem* ProjectileManager = ProofWorld->GetSubsystem<UT66ProjectileManagerSubsystem>())
					{
						return ProjectileManager->RunManagedProjectileVisualProfileProof();
					}
					return false;
				}();

				if (!bPass)
				{
					UE_LOG(LogT66GameMode, Warning, TEXT("[ManagedProjectileVisualProfileProofSummary] Terminal=1 Pass=0 Reason=MissingProjectileManagerOrProofFailed"));
				}
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("ManagedProjectileVisualProfileProofComplete"));
				return;
			}

			if (AutomationMode == TEXT("bossprojectilevisualprofileproof"))
			{
				const bool bPass = [&]()
				{
					if (UT66ProjectileManagerSubsystem* ProjectileManager = ProofWorld->GetSubsystem<UT66ProjectileManagerSubsystem>())
					{
						return ProjectileManager->RunBossProjectileVisualProfileProof();
					}
					return false;
				}();

				if (!bPass)
				{
					UE_LOG(LogT66GameMode, Warning, TEXT("[BossProjectileVisualProfileProofSummary] Terminal=1 Pass=0 Reason=MissingProjectileManagerOrProofFailed"));
				}
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("BossProjectileVisualProfileProofComplete"));
				return;
			}

			if (AutomationMode == TEXT("bosshazarddefinitionproof"))
			{
				const bool bPass = [&]()
				{
					if (UT66BossHazardSubsystem* HazardSubsystem = ProofWorld->GetSubsystem<UT66BossHazardSubsystem>())
					{
						return HazardSubsystem->RunBossHazardDefinitionProof();
					}
					return false;
				}();

				if (!bPass)
				{
					UE_LOG(LogT66GameMode, Warning, TEXT("[BossHazardDefinitionProofSummary] Terminal=1 Pass=0 Reason=MissingBossHazardSubsystemOrProofFailed"));
				}
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("BossHazardDefinitionProofComplete"));
				return;
			}

			if (AutomationMode == TEXT("bosshazarddamageproof"))
			{
				const bool bStarted = [&]()
				{
					if (UT66BossHazardSubsystem* HazardSubsystem = ProofWorld->GetSubsystem<UT66BossHazardSubsystem>())
					{
						return HazardSubsystem->StartBossHazardDamageProof();
					}
					return false;
				}();

				if (!bStarted)
				{
					UE_LOG(LogT66GameMode, Warning, TEXT("[BossHazardDamageProofSummary] Terminal=1 Pass=0 Reason=MissingBossHazardSubsystemOrProofStartFailed"));
					FPlatformMisc::RequestExitWithStatus(false, 1, TEXT("BossHazardDamageProofStartFailed"));
				}
				return;
			}

			if (AutomationMode == TEXT("vendorshopcorrectionsproof"))
			{
				UGameInstance* GI = ProofWorld->GetGameInstance();
				UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
				UT66GameInstance* T66GI = GetT66GameInstance();

				int32 StealAttempted = 0;
				int32 AttemptTriggeredVendorBoss = 0;
				int32 AttemptRecorded = 0;
				int32 DeterministicMiss = 0;
				int32 ProofShopSlotIndex = INDEX_NONE;
				int32 BaseBuyGold = 0;
				FT66InventorySlot ProofShopSlot;
				if (RunState)
				{
					RunState->ClearInventory();
					RunState->EnsureShopStockForCurrentStage();
					const TArray<FT66InventorySlot>& ShopSlots = RunState->GetShopStockSlots();
					const int32 SlotCount = ShopSlots.Num();
					for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
					{
						FItemData ItemData;
						if (T66GI
							&& ShopSlots.IsValidIndex(SlotIndex)
							&& ShopSlots[SlotIndex].IsValid()
							&& T66GI->GetItemData(ShopSlots[SlotIndex].ItemTemplateID, ItemData))
						{
							const int32 CandidateBuyGold = ItemData.GetBuyGoldForRarity(ShopSlots[SlotIndex].Rarity);
							if (CandidateBuyGold > 0)
							{
								ProofShopSlotIndex = SlotIndex;
								ProofShopSlot = ShopSlots[SlotIndex];
								BaseBuyGold = CandidateBuyGold;
								break;
							}
						}
					}

					if (ProofShopSlotIndex != INDEX_NONE)
					{
						// bTimingHit=false is the deterministic live attempt path: it records a miss
						// and must still arm the current always-trigger vendor anger behavior.
						RunState->ResolveShopStealAttempt(ProofShopSlotIndex, /*bTimingHit*/ false, /*bRngSuccess*/ false);
						const ET66ShopStealOutcome Outcome = RunState->GetLastShopStealOutcome();
						StealAttempted = (Outcome != ET66ShopStealOutcome::None) ? 1 : 0;
						DeterministicMiss = (Outcome == ET66ShopStealOutcome::Miss) ? 1 : 0;
						AttemptTriggeredVendorBoss = RunState->DidLastShopStealAttemptTriggerVendorBoss() ? 1 : 0;
						AttemptRecorded = RunState->GetShopAngerState().StealAttemptCount > 0 ? 1 : 0;
					}
				}

				// Spawn the Vendor hidden boss directly after the run-state anger trigger. The production
				// widget routes this same trigger into PlayerController::SpawnVendorBoss().
				FVector SpawnLocation = FVector::ZeroVector;
				if (APlayerController* PC = ProofWorld->GetFirstPlayerController())
				{
					if (APawn* Pawn = PC->GetPawn())
					{
						SpawnLocation = Pawn->GetActorLocation() + FVector(600.f, 0.f, 0.f);
					}
				}
				// A clean failed-steal path spawns no casino-anger boss, so before we spawn the vendor
				// boss there must be zero VendorBoss instances present.
				int32 PreSpawnVendorBossCount = 0;
				for (TActorIterator<AT66VendorBoss> It(ProofWorld); It; ++It)
				{
					++PreSpawnVendorBossCount;
				}

				AT66VendorBoss* VendorBoss = nullptr;
				if (AttemptTriggeredVendorBoss)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					VendorBoss = ProofWorld->SpawnActor<AT66VendorBoss>(AT66VendorBoss::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				}

				const int32 VendorBossSpawned = VendorBoss ? 1 : 0;
				const int32 VendorBossIdentity = (VendorBoss && VendorBoss->BossID == FName(TEXT("VendorBoss"))) ? 1 : 0;

				// After the spawn exactly one hidden VendorBoss must be present.
				int32 PostSpawnVendorBossCount = 0;
				for (TActorIterator<AT66VendorBoss> It(ProofWorld); It; ++It)
				{
					++PostSpawnVendorBossCount;
				}
				const int32 HiddenBossCount = PostSpawnVendorBossCount;
				const int32 CasinoAngerSpawnedBoss = PreSpawnVendorBossCount;

				int32 VendorBossDefeated = 0;
				if (VendorBoss)
				{
					// Awaken so damage is accepted, then keep hitting: each hit resolves to one alive
					// hit-zone part, so the boss only dies (and drops its token) once every part is gone.
					VendorBoss->ForceAwaken();
					for (int32 Hit = 0; Hit < 24 && !VendorBossDefeated; ++Hit)
					{
						if (VendorBoss->TakeDamageFromHeroHit(VendorBoss->MaxHP + 1000, FName(TEXT("VendorShopCorrectionsProof"))))
						{
							VendorBossDefeated = 1;
						}
					}
				}

				int32 VendorTokenDropped = 0;
				int32 VendorTokenDroppedStacks = 0;
				int32 VendorTokenStacksBeforePickup = RunState ? RunState->GetActiveVendorTokenStacks() : INDEX_NONE;
				int32 VendorTokenStacksAfterPickup = INDEX_NONE;
				float OneStackSellFraction = 0.f;
				float OneStackBuyDiscount = 0.f;
				int32 OneStackBuyGold = 0;
				int32 OneStackSellGold = 0;
				float MaxStackSellFraction = 0.f;
				float MaxStackBuyDiscount = 0.f;
				int32 MaxStackBuyGold = 0;
				int32 MaxStackSellGold = 0;
				for (TActorIterator<AT66LootBagPickup> It(ProofWorld); It; ++It)
				{
					if (It->GetItemID() == FName(TEXT("Item_VendorToken")))
					{
						VendorTokenDropped = 1;
						VendorTokenDroppedStacks = It->HasExplicitLine1RolledValue() ? It->GetExplicitLine1RolledValue() : 1;
						break;
					}
				}

				if (RunState && VendorTokenDropped && VendorTokenDroppedStacks > 0)
				{
					RunState->ApplyVendorTokenPickup(VendorTokenDroppedStacks);
					VendorTokenStacksAfterPickup = RunState->GetActiveVendorTokenStacks();
					OneStackSellFraction = RunState->GetCurrentSellFraction();
					OneStackBuyDiscount = RunState->GetCurrentBuyDiscountFraction();
					OneStackBuyGold = ProofShopSlotIndex != INDEX_NONE ? RunState->GetBuyGoldForShopStockSlot(ProofShopSlotIndex) : 0;
					OneStackSellGold = RunState->GetSellGoldForInventorySlot(ProofShopSlot);

					RunState->ApplyVendorTokenPickup(UT66RunStateSubsystem::MaxVendorTokenStacks);
					MaxStackSellFraction = RunState->GetCurrentSellFraction();
					MaxStackBuyDiscount = RunState->GetCurrentBuyDiscountFraction();
					MaxStackBuyGold = ProofShopSlotIndex != INDEX_NONE ? RunState->GetBuyGoldForShopStockSlot(ProofShopSlotIndex) : 0;
					MaxStackSellGold = RunState->GetSellGoldForInventorySlot(ProofShopSlot);
				}

				const int32 ExpectedOneStackBuyGold = FMath::Max(0, FMath::RoundToInt(static_cast<float>(BaseBuyGold) * 0.975f));
				const int32 ExpectedOneStackSellGold = FMath::Max(0, FMath::RoundToInt(static_cast<float>(BaseBuyGold) * 0.725f));
				const int32 ExpectedMaxStackBuyGold = FMath::Max(0, FMath::RoundToInt(static_cast<float>(BaseBuyGold) * 0.600f));
				const int32 ExpectedMaxStackSellGold = BaseBuyGold;
				const bool bOneStackMathPass =
					VendorTokenStacksBeforePickup == 0 &&
					VendorTokenDroppedStacks == 1 &&
					VendorTokenStacksAfterPickup == 1 &&
					FMath::IsNearlyEqual(OneStackSellFraction, 0.725f, 0.0001f) &&
					FMath::IsNearlyEqual(OneStackBuyDiscount, 0.025f, 0.0001f) &&
					OneStackBuyGold == ExpectedOneStackBuyGold &&
					OneStackSellGold == ExpectedOneStackSellGold;
				const bool bMaxStackMathPass =
					RunState &&
					RunState->GetActiveVendorTokenStacks() == UT66RunStateSubsystem::MaxVendorTokenStacks &&
					FMath::IsNearlyEqual(MaxStackSellFraction, 1.0f, 0.0001f) &&
					FMath::IsNearlyEqual(MaxStackBuyDiscount, 0.400f, 0.0001f) &&
					MaxStackBuyGold == ExpectedMaxStackBuyGold &&
					MaxStackSellGold == ExpectedMaxStackSellGold;
				const bool bFourSlotSurfaces =
					UT66RunStateSubsystem::ShopDisplaySlotCount == 4 &&
					UT66RunStateSubsystem::BuybackDisplaySlotCount == 4 &&
					UT66CasinoVendorTabWidget::SellVisibleSlotCount == 4;
				const bool bPass = RunState && ProofShopSlotIndex != INDEX_NONE && StealAttempted && DeterministicMiss
					&& AttemptRecorded && AttemptTriggeredVendorBoss && VendorBossSpawned && VendorBossIdentity
					&& VendorBossDefeated && VendorTokenDropped && (HiddenBossCount == 1) && (CasinoAngerSpawnedBoss == 0)
					&& bOneStackMathPass && bMaxStackMathPass && bFourSlotSurfaces;

				UE_LOG(
					LogT66GameMode,
					Log,
					TEXT("[VendorShopCorrectionsProofSummary] Terminal=1 Slot=%d BaseBuyGold=%d StealAttempted=%d DeterministicMiss=%d AttemptRecorded=%d AttemptTriggeredVendorBoss=%d VendorBossSpawned=%d VendorBossIdentity=%d VendorBossDefeated=%d VendorTokenDropped=%d DroppedStacks=%d TokenStacksBefore=%d TokenStacksAfterPickup=%d OneStackSellFraction=%.3f OneStackDiscount=%.3f OneStackBuyGold=%d ExpectedOneStackBuyGold=%d OneStackSellGold=%d ExpectedOneStackSellGold=%d MaxStacks=%d MaxSellFraction=%.3f MaxDiscount=%.3f MaxBuyGold=%d ExpectedMaxBuyGold=%d MaxSellGold=%d ExpectedMaxSellGold=%d ShopSlots=%d BuybackSlots=%d SellVisibleSlots=%d HiddenBossCount=%d CasinoAngerSpawnedBoss=%d Pass=%d"),
					ProofShopSlotIndex,
					BaseBuyGold,
					StealAttempted,
					DeterministicMiss,
					AttemptRecorded,
					AttemptTriggeredVendorBoss,
					VendorBossSpawned,
					VendorBossIdentity,
					VendorBossDefeated,
					VendorTokenDropped,
					VendorTokenDroppedStacks,
					VendorTokenStacksBeforePickup,
					VendorTokenStacksAfterPickup,
					OneStackSellFraction,
					OneStackBuyDiscount,
					OneStackBuyGold,
					ExpectedOneStackBuyGold,
					OneStackSellGold,
					ExpectedOneStackSellGold,
					RunState ? RunState->GetActiveVendorTokenStacks() : INDEX_NONE,
					MaxStackSellFraction,
					MaxStackBuyDiscount,
					MaxStackBuyGold,
					ExpectedMaxStackBuyGold,
					MaxStackSellGold,
					ExpectedMaxStackSellGold,
					UT66RunStateSubsystem::ShopDisplaySlotCount,
					UT66RunStateSubsystem::BuybackDisplaySlotCount,
					UT66CasinoVendorTabWidget::SellVisibleSlotCount,
					HiddenBossCount,
					CasinoAngerSpawnedBoss,
					bPass ? 1 : 0);
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("VendorShopCorrectionsProofComplete"));
				return;
			}

			if (AutomationMode == TEXT("loansharkdebtproof"))
			{
				UGameInstance* GI = ProofWorld->GetGameInstance();
				UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

				int32 DebtSet = 0;
				int32 PendingSet = 0;
				int32 LoanSharkSpawned = 0;
				int32 ChasingHero = 0;
				float LowDebtSpeed = 0.f;
				float HighDebtSpeed = 0.f;
				int32 LowDebtDamageHearts = 0;
				int32 HighDebtDamageHearts = 0;
				int32 SpeedScaledWithDebt = 0;
				int32 DamageScaledWithDebt = 0;
				int32 TouchDamageApplied = 0;
				int32 LoanSharkDespawned = 0;

				static constexpr int32 LowDebtAmount = 100;
				static constexpr int32 HighDebtAmount = 600;

				if (RunState)
				{
					// Provide net worth so genuine borrowing (CanBorrowGold) succeeds, then incur low debt.
					RunState->AddGold(HighDebtAmount + 2000);
					RunState->BorrowGold(LowDebtAmount);
					DebtSet = (RunState->GetCurrentDebt() > 0) ? 1 : 0;

					RunState->SetLoanSharkPending(true);
					PendingSet = RunState->GetLoanSharkPending() ? 1 : 0;

					TrySpawnLoanSharkIfNeeded();
					LoanSharkSpawned = (LoanShark != nullptr) ? 1 : 0;

					if (LoanShark)
					{
						LoanShark->AutomationRefreshTuningFromDebt();
						LowDebtSpeed = LoanShark->AutomationGetMaxWalkSpeed();
						LowDebtDamageHearts = LoanShark->AutomationGetCurrentDamageHearts();
						ChasingHero = LoanShark->AutomationIsChasingHero() ? 1 : 0;

						// Raise debt and re-read tuning to prove it scales.
						RunState->BorrowGold(HighDebtAmount - LowDebtAmount);
						LoanShark->AutomationRefreshTuningFromDebt();
						HighDebtSpeed = LoanShark->AutomationGetMaxWalkSpeed();
						HighDebtDamageHearts = LoanShark->AutomationGetCurrentDamageHearts();

						SpeedScaledWithDebt = (HighDebtSpeed > LowDebtSpeed) ? 1 : 0;
						DamageScaledWithDebt = (HighDebtDamageHearts > LowDebtDamageHearts) ? 1 : 0;

						// Apply one touch hit through the same gated path as the production capsule
						// overlap (debt>0, hero outside safe zones). Clear any post-hit invuln window
						// first, since the shark's spawn overlap may already have landed a touch this tick.
						RunState->AutomationResetDamageInvuln();
						const float HeroHPBefore = RunState->GetCurrentHP();
						const bool bTouchFired = LoanShark->AutomationApplyTouchDamageToHero();
						const float HeroHPAfter = RunState->GetCurrentHP();
						TouchDamageApplied = (bTouchFired && HeroHPAfter < HeroHPBefore) ? 1 : 0;

						// Pay debt to 0 and reproduce the production debt-paid despawn check.
						RunState->PayDebt(RunState->GetCurrentDebt());
						if (LoanShark && RunState->GetCurrentDebt() <= 0)
						{
							LoanShark->Destroy();
							LoanShark = nullptr;
						}
						LoanSharkDespawned = (LoanShark == nullptr) ? 1 : 0;
					}
				}

				const bool bPass = DebtSet && PendingSet && LoanSharkSpawned && ChasingHero
					&& SpeedScaledWithDebt && DamageScaledWithDebt && TouchDamageApplied && LoanSharkDespawned;

				UE_LOG(
					LogT66GameMode,
					Log,
					TEXT("[LoanSharkDebtProofSummary] Terminal=1 DebtSet=%d PendingSet=%d LoanSharkSpawned=%d ChasingHero=%d LowDebtSpeed=%.1f HighDebtSpeed=%.1f LowDebtDamageHearts=%d HighDebtDamageHearts=%d SpeedScaledWithDebt=%d DamageScaledWithDebt=%d TouchDamageApplied=%d LoanSharkDespawned=%d Pass=%d"),
					DebtSet,
					PendingSet,
					LoanSharkSpawned,
					ChasingHero,
					LowDebtSpeed,
					HighDebtSpeed,
					LowDebtDamageHearts,
					HighDebtDamageHearts,
					SpeedScaledWithDebt,
					DamageScaledWithDebt,
					TouchDamageApplied,
					LoanSharkDespawned,
					bPass ? 1 : 0);
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("LoanSharkDebtProofComplete"));
				return;
			}

			AT66HeroBase* Hero = nullptr;
			if (APlayerController* PC = ProofWorld->GetFirstPlayerController())
			{
				Hero = Cast<AT66HeroBase>(PC->GetPawn());
			}

			int32 GuardianSpawned[3] = { 0, 0, 0 };
			int32 BlockedWhileAlive[3] = { 0, 0, 0 };
			int32 UnblockedAfterDeath[3] = { 0, 0, 0 };
			int32 InteractAfterDeath[3] = { 0, 0, 0 };
			static constexpr int32 ProofFloors[3] = { 2, 3, 4 };

			if (Hero)
			{
				for (int32 Index = 0; Index < UE_ARRAY_COUNT(ProofFloors); ++Index)
				{
					const int32 FloorNumber = ProofFloors[Index];
					const T66TowerMapTerrain::FFloor* Floor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, FloorNumber);
					if (Floor)
					{
						FVector HeroLocation = Floor->ArrivalPoint;
						if (HeroLocation.IsNearlyZero())
						{
							HeroLocation = FVector(Floor->Center.X, Floor->Center.Y, Floor->SurfaceZ);
						}
						if (UCapsuleComponent* Capsule = Hero->GetCapsuleComponent())
						{
							HeroLocation.Z = Floor->SurfaceZ + Capsule->GetScaledCapsuleHalfHeight() + 24.0f;
						}
						Hero->SetActorLocation(HeroLocation, false, nullptr, ETeleportType::TeleportPhysics);
					}

					AT66TowerDescentHole* Hole = FindTowerDescentHoleForFloor(FloorNumber);
					AT66EnemyBase* Guardian = EnsurePlacedTowerMinibossForFloor(FloorNumber);
					if (!Guardian && Hole)
					{
						Guardian = Hole->AutomationGetGuardianEnemy();
					}

					GuardianSpawned[Index] = Guardian && Guardian->CurrentHP > 0 ? 1 : 0;
					const bool bCanOpenWhileAlive = Hole && Hole->AutomationCanOpenForHero(Hero);
					BlockedWhileAlive[Index] = GuardianSpawned[Index] && !bCanOpenWhileAlive ? 1 : 0;

					if (Guardian && Guardian->CurrentHP > 0)
					{
						Guardian->TakeDamageFromEnvironment(
							Guardian->CurrentHP + Guardian->MaxHP + 1000,
							this,
							FName(TEXT("MinibossTraversalProof")));
					}

					const bool bCanOpenAfterDeath = Hole && Hole->AutomationCanOpenForHero(Hero);
					UnblockedAfterDeath[Index] = bCanOpenAfterDeath ? 1 : 0;
					InteractAfterDeath[Index] = (bCanOpenAfterDeath && Hole->Interact(Hero)) ? 1 : 0;
				}
			}

			const bool bPass = Hero
				&& GuardianSpawned[0] && GuardianSpawned[1] && GuardianSpawned[2]
				&& BlockedWhileAlive[0] && BlockedWhileAlive[1] && BlockedWhileAlive[2]
				&& UnblockedAfterDeath[0] && UnblockedAfterDeath[1] && UnblockedAfterDeath[2];

			UE_LOG(
				LogT66GameMode,
				Log,
				TEXT("[MinibossTraversalProofSummary] Terminal=1 Floors=2->3->4 Floor2GuardianSpawned=%d Floor2BlockedWhileAlive=%d Floor2UnblockedAfterDeath=%d Floor2InteractAfterDeath=%d Floor3GuardianSpawned=%d Floor3BlockedWhileAlive=%d Floor3UnblockedAfterDeath=%d Floor3InteractAfterDeath=%d Floor4GuardianSpawned=%d Floor4BlockedWhileAlive=%d Floor4UnblockedAfterDeath=%d Floor4InteractAfterDeath=%d Pass=%d"),
				GuardianSpawned[0],
				BlockedWhileAlive[0],
				UnblockedAfterDeath[0],
				InteractAfterDeath[0],
				GuardianSpawned[1],
				BlockedWhileAlive[1],
				UnblockedAfterDeath[1],
				InteractAfterDeath[1],
				GuardianSpawned[2],
				BlockedWhileAlive[2],
				UnblockedAfterDeath[2],
				InteractAfterDeath[2],
				bPass ? 1 : 0);
			FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("MinibossTraversalProofComplete"));
		}));
	};
#endif

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
	{
		if (!Floor.bHasDropHole)
		{
			continue;
		}

		const T66TowerMapTerrain::FFloor* DestinationFloor = nullptr;
		for (const T66TowerMapTerrain::FFloor& CandidateFloor : CachedTowerMainMapLayout.Floors)
		{
			if (CandidateFloor.FloorNumber == (Floor.FloorNumber + 1))
			{
				DestinationFloor = &CandidateFloor;
				break;
			}
		}
		if (!DestinationFloor)
		{
			continue;
		}

		const float DropHeight = FMath::Max(Floor.SurfaceZ - DestinationFloor->SurfaceZ, 1200.0f);
		const float VerticalExtent = FMath::Clamp((DropHeight * 0.5f) - 550.0f, 800.0f, 1400.0f);
		const FVector BoxExtent(
			FMath::Max(250.0f, Floor.HoleHalfExtent.X * 0.88f),
			FMath::Max(250.0f, Floor.HoleHalfExtent.Y * 0.88f),
			VerticalExtent);
		const FVector HoleLocation = Floor.HoleCenter + FVector(0.0f, 0.0f, -VerticalExtent + 120.0f);

		AT66TowerDescentHole* HoleActor = World->SpawnActor<AT66TowerDescentHole>(
			AT66TowerDescentHole::StaticClass(),
			HoleLocation,
			FRotator::ZeroRotator,
			SpawnParams);
		if (!HoleActor)
		{
			continue;
		}

		const bool bRequiresWeaponSelection = Floor.FloorNumber == CachedTowerMainMapLayout.StartFloorNumber;
		const bool bRequiresGuardianDefeated = IsPlacedTowerMinibossFloor(Floor.FloorNumber);
		HoleActor->InitializeHole(
			Floor.FloorNumber,
			DestinationFloor->FloorNumber,
			BoxExtent,
			bRequiresWeaponSelection,
			bRequiresGuardianDefeated);
		HoleActor->Tags.AddUnique(FName(TEXT("T66_Tower_DescentHole")));
		TowerDescentHoles.Add(HoleActor);
	}

#if !UE_BUILD_SHIPPING
	ScheduleVerificationProofIfRequested();
#endif
}

bool AT66GameMode::IsPlacedTowerMinibossFloor(const int32 FloorNumber) const
{
	return IsUsingTowerMainMapLayout()
		&& FloorNumber >= CachedTowerMainMapLayout.FirstMobFloorNumber
		&& FloorNumber <= CachedTowerMainMapLayout.LastMobFloorNumber;
}

AT66TowerDescentHole* AT66GameMode::FindTowerDescentHoleForFloor(const int32 FloorNumber) const
{
	for (AT66TowerDescentHole* Hole : TowerDescentHoles)
	{
		if (Hole && Hole->GetFromFloorNumber() == FloorNumber)
		{
			return Hole;
		}
	}
	return nullptr;
}

AT66EnemyBase* AT66GameMode::EnsurePlacedTowerMinibossForFloor(const int32 FloorNumber)
{
	if (!IsPlacedTowerMinibossFloor(FloorNumber)
		|| TowerPlacedMinibossSpawnedFloors.Contains(FloorNumber)
		|| TowerPlacedMinibossDefeatedFloors.Contains(FloorNumber))
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const T66TowerMapTerrain::FFloor* Floor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, FloorNumber);
	if (!Floor)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("[MAP] Tower placed miniboss floor %d was requested but no floor layout was found."), FloorNumber);
		return nullptr;
	}

	AT66TowerDescentHole* HoleActor = FindTowerDescentHoleForFloor(FloorNumber);
	if (!HoleActor)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("[MAP] Tower placed miniboss floor %d was requested but no descent hole was found."), FloorNumber);
		return nullptr;
	}

	AT66EnemyBase* Miniboss = T66SpawnTowerGateGuardian(World, CachedTowerMainMapLayout, *Floor);
	if (!Miniboss)
	{
		UE_LOG(LogT66GameMode, Warning, TEXT("[MAP] Tower descent hole floor %d has no placed miniboss after spawn attempt."), FloorNumber);
		return nullptr;
	}

	HoleActor->SetGuardianEnemy(Miniboss);
	TowerPlacedMinibossSpawnedFloors.Add(FloorNumber);
	return Miniboss;
}

void AT66GameMode::HandleTowerGateGuardianDefeated(AT66EnemyBase* Guardian)
{
	if (!IsUsingTowerMainMapLayout() || !Guardian || !Guardian->ActorHasTag(T66TowerDescentGuardianTag))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 GuardianFloorNumber = T66ReadTowerFloorTag(Guardian);
	if (GuardianFloorNumber == INDEX_NONE)
	{
		GuardianFloorNumber = GetTowerFloorIndexForLocation(Guardian->GetActorLocation());
	}

	const FVector DropLocation = Guardian->GetActorLocation();
	Guardian->Tags.Remove(T66TowerDescentGuardianTag);
	if (GuardianFloorNumber != INDEX_NONE)
	{
		TowerPlacedMinibossDefeatedFloors.Add(GuardianFloorNumber);
	}

	AT66IdolAltar* SpawnedAltar = SpawnIdolAltarAtLocation(DropLocation, /*bAllowMultiple*/ true);
	if (SpawnedAltar && GuardianFloorNumber != INDEX_NONE)
	{
		T66TrySnapActorToTowerFloor(World, SpawnedAltar, CachedTowerMainMapLayout, GuardianFloorNumber, SpawnedAltar->GetActorLocation());
		T66AssignTowerFloorTag(SpawnedAltar, GuardianFloorNumber);
		SyncTowerMiasmaSourceAnchor(GuardianFloorNumber, SpawnedAltar->GetActorLocation());
	}

	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("[MAP] Tower gate guardian defeated floor=%d; idol altar %s at %s."),
		GuardianFloorNumber,
		SpawnedAltar ? TEXT("spawned") : TEXT("failed"),
		SpawnedAltar ? *SpawnedAltar->GetActorLocation().ToCompactString() : TEXT("None"));
}

bool AT66GameMode::IsUsingTowerMainMapLayout() const
{
	return bUsingTowerMainMapLayout && CachedTowerMainMapLayout.Floors.Num() > 0;
}

bool AT66GameMode::GetTowerMainMapLayout(T66TowerMapTerrain::FLayout& OutLayout) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return false;
	}

	OutLayout = CachedTowerMainMapLayout;
	return true;
}

bool AT66GameMode::GetTowerFloorLayout(const int32 FloorNumber, T66TowerMapTerrain::FFloor& OutFloor) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return false;
	}

	for (const T66TowerMapTerrain::FFloor& Floor : CachedTowerMainMapLayout.Floors)
	{
		if (Floor.FloorNumber == FloorNumber)
		{
			OutFloor = Floor;
			return true;
		}
	}

	return false;
}

int32 AT66GameMode::GetTowerFloorIndexForLocation(const FVector& Location) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return INDEX_NONE;
	}

	return T66TowerMapTerrain::FindFloorIndexForLocation(CachedTowerMainMapLayout, Location);
}

int32 AT66GameMode::GetCurrentTowerFloorIndex() const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return INDEX_NONE;
	}

	if (const UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (!Pawn)
			{
				continue;
			}

			const int32 FloorNumber = GetTowerFloorIndexForLocation(Pawn->GetActorLocation());
			if (FloorNumber != INDEX_NONE)
			{
				return FloorNumber;
			}
		}
	}

	return CachedTowerMainMapLayout.StartFloorNumber;
}

bool AT66GameMode::TryGetTowerEnemySpawnLocation(const FVector& PlayerLocation, float MinDistance, float MaxDistance, FRandomStream& Rng, FVector& OutLocation) const
{
	FVector WallNormal = FVector::ZeroVector;
	return TryGetTowerEnemySpawnLocation(PlayerLocation, MinDistance, MaxDistance, Rng, OutLocation, WallNormal);
}

bool AT66GameMode::TryGetTowerEnemySpawnLocation(
	const FVector& PlayerLocation,
	float MinDistance,
	float MaxDistance,
	FRandomStream& Rng,
	FVector& OutLocation,
	FVector& OutWallNormal) const
{
	if (!IsUsingTowerMainMapLayout())
	{
		return false;
	}

	return T66TowerMapTerrain::TryGetWallSpawnLocation(
		GetWorld(),
		CachedTowerMainMapLayout,
		PlayerLocation,
		MinDistance,
		MaxDistance,
		Rng,
		OutLocation,
		OutWallNormal);
}

void AT66GameMode::HandleTowerDescentHoleTriggered(APawn* Pawn, const int32 FromFloorNumber, const int32 ToFloorNumber)
{
	if (!IsUsingTowerMainMapLayout() || !Pawn)
	{
		return;
	}

	UE_LOG(
		LogT66GameMode,
		Log,
		TEXT("[MAP] Tower descent entered by %s (%d -> %d)"),
		*Pawn->GetName(),
		FromFloorNumber,
		ToFloorNumber);

	if (ToFloorNumber != INDEX_NONE && ToFloorNumber != CachedTowerMainMapLayout.BossFloorNumber)
	{
		SyncTowerMiasmaSourceAnchor(ToFloorNumber, Pawn->GetActorLocation());
	}

	if (ToFloorNumber != INDEX_NONE && ToFloorNumber != ActiveTowerTerrainVisualFloorNumber)
	{
		T66SetTowerTerrainVisualFloor(GetWorld(), ToFloorNumber);
		ActiveTowerTerrainVisualFloorNumber = ToFloorNumber;
	}

	if (!bTowerMiasmaActive && ToFloorNumber >= CachedTowerMainMapLayout.FirstMobFloorNumber)
	{
		const FVector FloorAnchor = Pawn->GetActorLocation();
		TryStartTowerMiasma(&FloorAnchor, ToFloorNumber);
	}

	if (ToFloorNumber >= CachedTowerMainMapLayout.FirstMobFloorNumber
		&& ToFloorNumber <= CachedTowerMainMapLayout.LastMobFloorNumber)
	{
		EnsurePlacedTowerMinibossForFloor(ToFloorNumber);
		if (AT66EnemyDirector* ExistingEnemyDirector = FindOrCacheEnemyDirector(GetWorld()))
		{
			ExistingEnemyDirector->SpawnInitialPopulationForTowerFloor(ToFloorNumber);
		}
	}

	if (ToFloorNumber == CachedTowerMainMapLayout.BossFloorNumber)
	{
		bTowerBossEntryTriggered = true;
		bTowerBossEntryApplied = false;
		SyncTowerBossEntryState();
	}
}

void AT66GameMode::SyncTowerBossEntryState()
{
	if (!IsUsingTowerMainMapLayout() || !bTowerBossEntryTriggered || bTowerBossEntryApplied)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	bool bHasEnemyDirector = false;
	if (AT66EnemyDirector* ExistingEnemyDirector = FindOrCacheEnemyDirector(World))
	{
		ExistingEnemyDirector->SetSpawningPaused(true);
		bHasEnemyDirector = true;
	}

	bool bHasBoss = false;
	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			if (AT66BossBase* Boss = WeakBoss.Get())
			{
				bHasBoss = true;
				if (!Boss->IsAwakened())
				{
					Boss->ForceAwaken();
				}
			}
		}
	}

	if (bHasEnemyDirector && bHasBoss)
	{
		bTowerBossEntryApplied = true;
		UE_LOG(LogT66GameMode, Log, TEXT("[MAP] Tower boss-floor entry activated via descent hole."));
	}
}

void AT66GameMode::SyncTowerTrapActivation(const bool bForce)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66TrapSubsystem* TrapSubsystem = World->GetSubsystem<UT66TrapSubsystem>();
	if (!TrapSubsystem)
	{
		return;
	}

	if (!IsUsingTowerMainMapLayout())
	{
		TowerTrapActivationAccumulator = 0.f;
		if (bForce || ActiveTowerTrapFloorNumber != INDEX_NONE)
		{
			ActiveTowerTrapFloorNumber = INDEX_NONE;
			TrapSubsystem->SetActiveTowerFloor(INDEX_NONE);
		}
		ActiveTowerTerrainVisualFloorNumber = INDEX_NONE;
		return;
	}

	const int32 CurrentFloorNumber = GetCurrentTowerFloorIndex();
	const bool bFloorChanged = CurrentFloorNumber != ActiveTowerTrapFloorNumber;
	if (!bForce && !bFloorChanged)
	{
		return;
	}

	TowerTrapActivationAccumulator = 0.f;
	ActiveTowerTrapFloorNumber = CurrentFloorNumber;
	TrapSubsystem->SetActiveTowerFloor(CurrentFloorNumber);
	if (bForce || CurrentFloorNumber != ActiveTowerTerrainVisualFloorNumber)
	{
		T66SetTowerTerrainVisualFloor(World, CurrentFloorNumber);
		ActiveTowerTerrainVisualFloorNumber = CurrentFloorNumber;
	}
}
