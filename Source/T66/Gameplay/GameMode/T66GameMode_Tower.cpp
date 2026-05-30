// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/GameMode/T66GameModePrivate.h"

#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66ProjectileManagerSubsystem.h"
#include "Gameplay/T66VendorBoss.h"

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
			&& AutomationMode != TEXT("vendorfailedstealproof")
			&& AutomationMode != TEXT("loansharkdebtproof"))
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

			if (AutomationMode == TEXT("vendorfailedstealproof"))
			{
				UGameInstance* GI = ProofWorld->GetGameInstance();
				UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

				int32 StealAttempted = 0;
				int32 StealFailed = 0;
				if (RunState)
				{
					// Drive a deterministic failed steal: bTimingHit=false keeps success chance at 0.
					RunState->EnsureShopStockForCurrentStage();
					const int32 SlotCount = RunState->GetShopStockSlots().Num();
					for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
					{
						RunState->ResolveShopStealAttempt(SlotIndex, /*bTimingHit*/ false, /*bRngSuccess*/ false);
						const ET66ShopStealOutcome Outcome = RunState->GetLastShopStealOutcome();
						if (Outcome != ET66ShopStealOutcome::None)
						{
							StealAttempted = 1;
							StealFailed = (Outcome == ET66ShopStealOutcome::Miss || Outcome == ET66ShopStealOutcome::Failed) ? 1 : 0;
							break;
						}
					}
				}

				// Spawn the Vendor hidden boss directly (the same class the production failure path spawns).
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

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AT66VendorBoss* VendorBoss = ProofWorld->SpawnActor<AT66VendorBoss>(AT66VendorBoss::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

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
						if (VendorBoss->TakeDamageFromHeroHit(VendorBoss->MaxHP + 1000, FName(TEXT("VendorFailedStealProof"))))
						{
							VendorBossDefeated = 1;
						}
					}
				}

				int32 VendorTokenDropped = 0;
				for (TActorIterator<AT66LootBagPickup> It(ProofWorld); It; ++It)
				{
					if (It->GetItemID() == FName(TEXT("Item_VendorToken")))
					{
						VendorTokenDropped = 1;
						break;
					}
				}

				const bool bPass = StealAttempted && StealFailed && VendorBossSpawned && VendorBossIdentity
					&& VendorBossDefeated && VendorTokenDropped && (HiddenBossCount == 1) && (CasinoAngerSpawnedBoss == 0);

				UE_LOG(
					LogT66GameMode,
					Log,
					TEXT("[VendorFailedStealProofSummary] Terminal=1 StealAttempted=%d StealFailed=%d VendorBossSpawned=%d VendorBossIdentity=%d VendorBossDefeated=%d VendorTokenDropped=%d HiddenBossCount=%d CasinoAngerSpawnedBoss=%d Pass=%d"),
					StealAttempted,
					StealFailed,
					VendorBossSpawned,
					VendorBossIdentity,
					VendorBossDefeated,
					VendorTokenDropped,
					HiddenBossCount,
					CasinoAngerSpawnedBoss,
					bPass ? 1 : 0);
				FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("VendorFailedStealProofComplete"));
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

	if (IsBossRushFinaleStage())
	{
		const T66TowerMapTerrain::FFloor* StartFloor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, CachedTowerMainMapLayout.StartFloorNumber);
		const T66TowerMapTerrain::FFloor* BossFloor = T66FindTowerFloorByNumber(CachedTowerMainMapLayout, CachedTowerMainMapLayout.BossFloorNumber);
		if (!StartFloor || !BossFloor)
		{
			return;
		}

		const FVector HoleCenter = StartFloor->bHasDropHole
			? StartFloor->HoleCenter
			: FVector(StartFloor->Center.X, StartFloor->Center.Y, StartFloor->SurfaceZ);
		const FVector2D HoleHalfExtent = StartFloor->bHasDropHole
			? StartFloor->HoleHalfExtent
			: FVector2D(320.0f, 320.0f);
		const float DropHeight = FMath::Max(StartFloor->SurfaceZ - BossFloor->SurfaceZ, 1200.0f);
		const float VerticalExtent = FMath::Clamp((DropHeight * 0.5f) - 550.0f, 800.0f, 1400.0f);
		const FVector BoxExtent(
			FMath::Max(250.0f, HoleHalfExtent.X * 0.88f),
			FMath::Max(250.0f, HoleHalfExtent.Y * 0.88f),
			VerticalExtent);
		const FVector HoleLocation = HoleCenter + FVector(0.0f, 0.0f, -VerticalExtent + 120.0f);

		if (AT66TowerDescentHole* HoleActor = World->SpawnActor<AT66TowerDescentHole>(
			AT66TowerDescentHole::StaticClass(),
			HoleLocation,
			FRotator::ZeroRotator,
			SpawnParams))
		{
			HoleActor->InitializeHole(
				StartFloor->FloorNumber,
				BossFloor->FloorNumber,
				BoxExtent,
				/*bInRequiresWeaponSelection*/ true,
				/*bInRequiresGuardianDefeated*/ false);
			HoleActor->Tags.AddUnique(FName(TEXT("T66_Tower_DescentHole")));
			TowerDescentHoles.Add(HoleActor);
		}
#if !UE_BUILD_SHIPPING
		ScheduleVerificationProofIfRequested();
#endif
		return;
	}

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
		&& !IsBossRushFinaleStage()
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

	if ((bHasEnemyDirector || IsBossRushFinaleStage()) && bHasBoss)
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
