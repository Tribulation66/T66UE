// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniGameMode.h"

#include "Components/AudioComponent.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniLeaderboardSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Gameplay/T66MiniArena.h"
#include "Gameplay/T66MiniCompanionBase.h"
#include "Gameplay/T66MiniEnemyBase.h"
#include "Gameplay/T66MiniHazardTrap.h"
#include "Gameplay/T66MiniGameState.h"
#include "Gameplay/T66MiniInteractable.h"
#include "Gameplay/T66MiniPickup.h"
#include "Gameplay/T66MiniPlayerController.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "Sound/SoundBase.h"
#include "UI/T66MiniBattleHUD.h"
#include "UI/T66UITypes.h"
#include "VFX/T66MiniGroundTelegraphActor.h"

namespace
{
	float T66MiniResolveMasterVolume(const UGameInstance* GameInstance)
	{
		if (const UT66PlayerSettingsSubsystem* PlayerSettings = GameInstance ? GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr)
		{
			return FMath::Clamp(PlayerSettings->GetMasterVolume(), 0.f, 1.f);
		}

		return 1.0f;
	}

	float T66MiniResolveMusicVolume(const UGameInstance* GameInstance)
	{
		if (const UT66PlayerSettingsSubsystem* PlayerSettings = GameInstance ? GameInstance->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr)
		{
			return FMath::Clamp(PlayerSettings->GetMusicVolume(), 0.f, 1.f);
		}

		return 1.0f;
	}

	USoundBase* T66MiniLoadBattleMusic()
	{
		static TWeakObjectPtr<USoundBase> Cached;
		if (!Cached.IsValid())
		{
			Cached = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/OSTS/Survival.Survival"));
		}

		return Cached.Get();
	}

	TArray<AT66MiniPlayerPawn*> T66MiniGatherPlayerPawns(UWorld* World, const bool bOnlyLiving = false)
	{
		TArray<AT66MiniPlayerPawn*> PlayerPawns;
		if (!World)
		{
			return PlayerPawns;
		}

		if (const AT66MiniGameMode* MiniGameMode = World->GetAuthGameMode<AT66MiniGameMode>())
		{
			const TArray<TObjectPtr<AT66MiniPlayerPawn>>& CachedPlayerPawns = MiniGameMode->GetLivePlayerPawns();
			if (CachedPlayerPawns.Num() > 0)
			{
				for (AT66MiniPlayerPawn* PlayerPawn : CachedPlayerPawns)
				{
					if (!PlayerPawn || (bOnlyLiving && !PlayerPawn->IsHeroAlive()))
					{
						continue;
					}

					PlayerPawns.Add(PlayerPawn);
				}

				if (PlayerPawns.Num() > 0 || !bOnlyLiving)
				{
					return PlayerPawns;
				}
			}
		}

		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PlayerController = It->Get();
			AT66MiniPlayerPawn* PlayerPawn = PlayerController ? Cast<AT66MiniPlayerPawn>(PlayerController->GetPawn()) : nullptr;
			if (!PlayerPawn || (bOnlyLiving && !PlayerPawn->IsHeroAlive()))
			{
				continue;
			}

			PlayerPawns.Add(PlayerPawn);
		}

		return PlayerPawns;
	}

	AT66MiniPlayerPawn* T66MiniChooseAnchorPawn(UWorld* World)
	{
		const TArray<AT66MiniPlayerPawn*> LivingPawns = T66MiniGatherPlayerPawns(World, true);
		if (LivingPawns.Num() > 0)
		{
			return LivingPawns[0];
		}

		const TArray<AT66MiniPlayerPawn*> AllPawns = T66MiniGatherPlayerPawns(World, false);
		return AllPawns.Num() > 0 ? AllPawns[0] : nullptr;
	}

}

AT66MiniGameMode::AT66MiniGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AT66MiniPlayerPawn::StaticClass();
	PlayerControllerClass = AT66MiniPlayerController::StaticClass();
	PlayerStateClass = APlayerState::StaticClass();
	GameStateClass = AT66MiniGameState::StaticClass();
	HUDClass = AT66MiniBattleHUD::StaticClass();
	bUseSeamlessTravel = true;
}

AT66MiniEnemyBase* AT66MiniGameMode::GetActiveBossEnemy() const
{
	for (AT66MiniEnemyBase* Enemy : LiveEnemies)
	{
		if (Enemy && Enemy->IsBossEnemy() && !Enemy->IsEnemyDead())
		{
			return Enemy;
		}
	}

	return nullptr;
}

AT66MiniPlayerPawn* AT66MiniGameMode::FindClosestPlayerPawn(const FVector& WorldLocation, const bool bRequireAlive) const
{
	AT66MiniPlayerPawn* BestPawn = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();
	bool bUsedCachedPlayerSet = false;
	for (AT66MiniPlayerPawn* Candidate : LivePlayerPawns)
	{
		if (!Candidate || (bRequireAlive && !Candidate->IsHeroAlive()))
		{
			continue;
		}

		bUsedCachedPlayerSet = true;
		const float DistanceSq = FVector::DistSquared2D(WorldLocation, Candidate->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestPawn = Candidate;
		}
	}

	if (bUsedCachedPlayerSet || !GetWorld())
	{
		return BestPawn;
	}

	for (AT66MiniPlayerPawn* Candidate : T66MiniGatherPlayerPawns(GetWorld(), bRequireAlive))
	{
		if (!Candidate)
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(WorldLocation, Candidate->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestPawn = Candidate;
		}
	}

	return BestPawn;
}

float AT66MiniGameMode::GetRuntimeTuningValue(const TCHAR* Key, const float DefaultValue) const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	return DataSubsystem ? DataSubsystem->FindRuntimeTuningValue(FName(Key), DefaultValue) : DefaultValue;
}

int32 AT66MiniGameMode::GetRuntimeTuningInt(const TCHAR* Key, const int32 DefaultValue) const
{
	return FMath::RoundToInt(GetRuntimeTuningValue(Key, static_cast<float>(DefaultValue)));
}

void AT66MiniGameMode::RegisterLiveTrap(AT66MiniHazardTrap* Trap)
{
	if (Trap)
	{
		LiveTraps.AddUnique(Trap);
	}
}

void AT66MiniGameMode::UnregisterLiveTrap(const AT66MiniHazardTrap* Trap)
{
	LiveTraps.RemoveAllSwap(
		[Trap](const TObjectPtr<AT66MiniHazardTrap>& Candidate)
		{
			return Candidate.Get() == Trap;
		},
		EAllowShrinking::No);
}

void AT66MiniGameMode::RegisterLiveInteractable(AT66MiniInteractable* Interactable)
{
	if (Interactable)
	{
		LiveInteractables.AddUnique(Interactable);
	}
}

void AT66MiniGameMode::UnregisterLiveInteractable(const AT66MiniInteractable* Interactable)
{
	LiveInteractables.RemoveAllSwap(
		[Interactable](const TObjectPtr<AT66MiniInteractable>& Candidate)
		{
			return Candidate.Get() == Interactable;
		},
		EAllowShrinking::No);
}

void AT66MiniGameMode::RegisterLivePickup(AT66MiniPickup* Pickup)
{
	if (Pickup)
	{
		LivePickups.AddUnique(Pickup);
	}
}

void AT66MiniGameMode::UnregisterLivePickup(const AT66MiniPickup* Pickup)
{
	LivePickups.RemoveAllSwap(
		[Pickup](const TObjectPtr<AT66MiniPickup>& Candidate)
		{
			return Candidate.Get() == Pickup;
		},
		EAllowShrinking::No);
}

void AT66MiniGameMode::BeginPlay()
{
	Super::BeginPlay();

	UT66MiniRunSaveGame* ActiveRun = nullptr;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GameInstance))
		{
			T66GameInstance->HidePersistentGameplayTransitionCurtain();
		}

		if (UT66MiniRunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>())
		{
			ActiveRun = RunState->GetActiveRun();
			if (AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>())
			{
				MiniGameState->ApplyRunSave(ActiveRun);
			}
		}

		if (UT66MiniVisualSubsystem* VisualSubsystem = GameInstance->GetSubsystem<UT66MiniVisualSubsystem>())
		{
			VisualSubsystem->LoadTextureByAssetPath(TEXT("/Game/UI/Sprites/UI/Hearts/T_Heart_Red.T_Heart_Red"));
			VisualSubsystem->LoadHudTexture(TEXT("Ult_Generic"));
			VisualSubsystem->LoadHudTexture(TEXT("Passive_Generic"));
			VisualSubsystem->LoadHudTexture(TEXT("QuickRevive"));
			VisualSubsystem->LoadHudTexture(TEXT("MouseLeft"));
			VisualSubsystem->LoadHudTexture(TEXT("MouseRight"));
			VisualSubsystem->LoadHudTexture(TEXT("Interact_Hand"));
			VisualSubsystem->LoadEffectTexture(TEXT("EnemyProjectile_Ranged"));
			VisualSubsystem->LoadEffectTexture(TEXT("EnemyProjectile_Boss"));
			VisualSubsystem->LoadEffectTexture(TEXT("EnemyProjectile_Impact"));
			VisualSubsystem->LoadInteractableTexture(TEXT("LootBag_Red"));
			VisualSubsystem->LoadInteractableTexture(TEXT("LootBag_Black"));
			VisualSubsystem->LoadInteractableTexture(TEXT("LootBag_Yellow"));

			if (const UT66MiniDataSubsystem* DataSubsystem = GameInstance->GetSubsystem<UT66MiniDataSubsystem>())
			{
				if (ActiveRun)
				{
					if (const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem->FindHero(ActiveRun->HeroID))
					{
						VisualSubsystem->LoadHeroProjectileTexture(HeroDefinition->DisplayName);
						VisualSubsystem->LoadTextureByAssetPath(HeroDefinition->PortraitPath);
					}

					for (const FName ItemID : ActiveRun->OwnedItemIDs)
					{
						if (const FT66MiniItemDefinition* ItemDefinition = DataSubsystem->FindItem(ItemID))
						{
							VisualSubsystem->LoadItemTexture(ItemDefinition->ItemID, ItemDefinition->IconPath);
						}
					}

					for (const FName IdolID : ActiveRun->EquippedIdolIDs)
					{
						VisualSubsystem->LoadIdolEffectTexture(IdolID);
						if (const FT66MiniIdolDefinition* IdolDefinition = DataSubsystem->FindIdol(IdolID))
						{
							VisualSubsystem->LoadTextureByAssetPath(IdolDefinition->IconPath);
						}
					}
				}
			}
		}
	}

	if (USoundBase* BattleMusic = T66MiniLoadBattleMusic())
	{
		BattleMusicComponent = UGameplayStatics::SpawnSound2D(this, BattleMusic, 1.0f, 1.0f, 0.0f, nullptr, false);
		RefreshAudioMix();
	}

	SpawnArenaAndPositionPlayer();
	UpdateLivePlayerPawnCache();
	PositionPlayerPawns();
	TryApplySavedPawnState();
	SpawnCompanionActor();

	if (AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>())
	{
		const bool bHasMidWaveSnapshot = ActiveRun && ActiveRun->bHasMidWaveSnapshot;
		StartWave(FMath::Max(1, MiniGameState->WaveIndex), !bHasMidWaveSnapshot);
		if (bHasMidWaveSnapshot)
		{
			RestoreTransientWaveState(ActiveRun);
			RestoreWorldState(ActiveRun);
		}
	}
}

void AT66MiniGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AudioMixRefreshAccumulator += DeltaSeconds;
	if (AudioMixRefreshAccumulator >= 0.25f)
	{
		RefreshAudioMix();
		AudioMixRefreshAccumulator = 0.f;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	PlayerRuntimeRefreshAccumulator += DeltaSeconds;
	const bool bNeedsFastPlayerRefresh = !bAppliedSavedPawnState || PositionedPlayerPawns.Num() < LivePlayerPawns.Num();
	if (PlayerRuntimeRefreshAccumulator >= (bNeedsFastPlayerRefresh ? 0.05f : 0.50f))
	{
		UpdateLivePlayerPawnCache();
		PositionPlayerPawns();
		PlayerRuntimeRefreshAccumulator = 0.f;
	}
	UpdateCombatTexts(DeltaSeconds);
	if (!RunState || !DataSubsystem || !MiniGameState || !ActiveRun)
	{
		return;
	}

	TryApplySavedPawnState();
	LiveCacheRefreshAccumulator += DeltaSeconds;
	if (LiveCacheRefreshAccumulator >= 0.50f)
	{
		UpdateLiveEnemyCache();
		UpdateLiveTrapCache();
		UpdateLiveInteractableCache();
		UpdateLivePickupCache();
		LiveCacheRefreshAccumulator = 0.f;
	}

	if (!bRunCompleted)
	{
		if (PostBossDelayRemaining > 0.f)
		{
			PostBossDelayRemaining -= DeltaSeconds;
			if (PostBossDelayRemaining <= 0.f)
			{
				RecordClearedMiniStageProgression();
				if (MiniGameState->WaveIndex >= GetMaxStageIndexForCurrentDifficulty())
				{
					bRunCompleted = true;
					FinalizeRun(true, TEXT("Difficulty cleared"));
					return;
				}

				ReturnToShopIntermission();
				return;
			}
		}
		else if (BossTelegraphRemaining > 0.f)
		{
			BossTelegraphRemaining -= DeltaSeconds;
			if (BossTelegraphRemaining <= 0.f)
			{
				SpawnBossEnemy();
				bBossSpawnedForWave = true;
			}
		}
		else if (!bBossSpawnedForWave)
		{
			const FT66MiniWaveDefinition* WaveDefinition = GetWaveDefinition();
			const FT66MiniDifficultyDefinition* DifficultyDefinition = GetDifficultyDefinition();
			MiniGameState->WaveSecondsRemaining = FMath::Max(0.f, MiniGameState->WaveSecondsRemaining - DeltaSeconds);
			EnemySpawnAccumulator += DeltaSeconds;
			InteractableSpawnAccumulator += DeltaSeconds;
			TrapSpawnAccumulator += DeltaSeconds;

			float SpawnInterval = WaveDefinition ? WaveDefinition->SpawnInterval : GetRuntimeTuningValue(TEXT("SpawnIntervalFallback"), 1.2f);
			if (DifficultyDefinition)
			{
				SpawnInterval = SpawnInterval / FMath::Max(GetRuntimeTuningValue(TEXT("SpawnRateScalarMin"), 0.65f), DifficultyDefinition->SpawnRateScalar);
			}
			SpawnInterval = FMath::Max(GetRuntimeTuningValue(TEXT("SpawnIntervalMin"), 0.28f), SpawnInterval);

			while (EnemySpawnAccumulator >= SpawnInterval && MiniGameState->WaveSecondsRemaining > 0.f)
			{
				EnemySpawnAccumulator -= SpawnInterval;
				SpawnWaveEnemy();
			}

			const float InteractableInterval = WaveDefinition
				? WaveDefinition->InteractableInterval
				: (DifficultyDefinition ? DifficultyDefinition->InteractableInterval : GetRuntimeTuningValue(TEXT("InteractableIntervalFallback"), 18.f));
			if (InteractableSpawnAccumulator >= InteractableInterval)
			{
				InteractableSpawnAccumulator = 0.f;
				SpawnRandomInteractable();
			}

			const float TrapInterval = FMath::Max(
				GetRuntimeTuningValue(TEXT("TrapIntervalMin"), 5.5f),
				(GetRuntimeTuningValue(TEXT("TrapIntervalBase"), 13.5f) - (MiniGameState->WaveIndex * GetRuntimeTuningValue(TEXT("TrapIntervalPerWave"), 1.25f)))
					/ FMath::Max(GetRuntimeTuningValue(TEXT("TrapSpawnRateScalarMin"), 0.75f), DifficultyDefinition ? DifficultyDefinition->SpawnRateScalar : 1.0f));
			if (TrapSpawnAccumulator >= TrapInterval)
			{
				TrapSpawnAccumulator = 0.f;
				SpawnRandomTrap();
			}

			if (MiniGameState->WaveSecondsRemaining <= 0.f)
			{
				BeginBossSpawnTelegraph();
			}
		}
		else if (LiveEnemies.Num() == 0 && PostBossDelayRemaining <= 0.f)
		{
			PostBossDelayRemaining = GetRuntimeTuningValue(TEXT("PostBossDelaySeconds"), 2.0f);
		}
	}

	ActiveRun->WaveIndex = MiniGameState->WaveIndex;
	if (const FT66MiniStageDefinition* StageDefinition = GetCurrentStageDefinition())
	{
		ActiveRun->CurrentStageID = StageDefinition->StageID;
		ActiveRun->StageIndex = StageDefinition->StageIndex;
	}
	else
	{
		ActiveRun->CurrentStageID = NAME_None;
		ActiveRun->StageIndex = MiniGameState->WaveIndex;
	}
	ActiveRun->WaveSecondsRemaining = MiniGameState->WaveSecondsRemaining;
	ActiveRun->HeroID = MiniGameState->HeroID;
	ActiveRun->CompanionID = MiniGameState->CompanionID;
	ActiveRun->DifficultyID = MiniGameState->DifficultyID;
	ActiveRun->bBossSpawnedForWave = bBossSpawnedForWave;
	ActiveRun->BossTelegraphRemaining = FMath::Max(0.f, BossTelegraphRemaining);
	ActiveRun->PendingBossID = PendingBossID;
	ActiveRun->PendingBossSpawnLocation = PendingBossSpawnLocation;
	ActiveRun->EnemySpawnAccumulator = EnemySpawnAccumulator;
	ActiveRun->InteractableSpawnAccumulator = InteractableSpawnAccumulator;
	ActiveRun->PostBossDelayRemaining = PostBossDelayRemaining;
	ActiveRun->TrapSpawnAccumulator = TrapSpawnAccumulator;
	ActiveRun->TotalRunSeconds += DeltaSeconds;

	if (AT66MiniPlayerPawn* PlayerPawn = T66MiniChooseAnchorPawn(GetWorld()))
	{
		ActiveRun->PlayerLocation = PlayerPawn->GetActorLocation();
		ActiveRun->bHasPlayerLocation = true;
		ActiveRun->bHasMidWaveSnapshot = true;
		ActiveRun->HeroLevel = PlayerPawn->GetHeroLevel();
		ActiveRun->CurrentHealth = PlayerPawn->GetCurrentHealth();
		ActiveRun->MaxHealth = PlayerPawn->GetMaxHealth();
		ActiveRun->Materials = PlayerPawn->GetMaterials();
		ActiveRun->Gold = PlayerPawn->GetGold();
		ActiveRun->Experience = PlayerPawn->GetExperience();
		ActiveRun->UltimateCooldownRemaining = PlayerPawn->GetUltimateCooldownRemaining();
		ActiveRun->bEnduranceCheatUsedThisWave = PlayerPawn->HasEnduranceCheatUsedThisWave();
		ActiveRun->bQuickReviveReady = PlayerPawn->HasQuickReviveReady();
	}

	AutosaveAccumulator += DeltaSeconds;
	if (AutosaveAccumulator >= GetRuntimeTuningValue(TEXT("AutosaveInterval"), 1.0f))
	{
		PersistActiveRunSnapshot(true);
		AutosaveAccumulator = 0.f;
	}
}

void AT66MiniGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	UpdateLivePlayerPawnCache();
}

void AT66MiniGameMode::RefreshAudioMix()
{
	if (!BattleMusicComponent)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	const float EffectiveMusicVolume = 0.40f * T66MiniResolveMasterVolume(GameInstance) * T66MiniResolveMusicVolume(GameInstance);
	BattleMusicComponent->SetVolumeMultiplier(FMath::Clamp(EffectiveMusicVolume, 0.f, 1.f));
}

void AT66MiniGameMode::HandleEnemyKilled(AT66MiniEnemyBase* Enemy)
{
	if (Enemy)
	{
		for (AT66MiniPlayerPawn* PlayerPawn : T66MiniGatherPlayerPawns(GetWorld(), false))
		{
			PlayerPawn->HandleEnemyKilled(Enemy);
		}

		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnPulse(GetWorld(), Enemy->GetActorLocation() + FVector(0.f, 0.f, 8.f), Enemy->IsBossEnemy() ? FVector(0.62f, 0.62f, 1.f) : FVector(0.24f, 0.24f, 1.f), Enemy->IsBossEnemy() ? 0.22f : 0.12f, Enemy->IsBossEnemy() ? FLinearColor(1.0f, 0.56f, 0.24f, 0.34f) : FLinearColor(0.98f, 0.44f, 0.22f, 0.18f), Enemy->IsBossEnemy() ? 0.95f : 0.55f);
		}
	}

	LiveEnemies.Remove(Enemy);
}

void AT66MiniGameMode::HandlePlayerDefeated()
{
	if (bRunFinalized)
	{
		return;
	}

	if (!bRunFinalized)
	{
		FinalizeRun(false, TEXT("Run failed"));
	}
}

FVector AT66MiniGameMode::ClampPointToArena(const FVector& WorldLocation) const
{
	return FVector(
		FMath::Clamp(WorldLocation.X, ArenaOrigin.X - ArenaHalfExtent + 100.f, ArenaOrigin.X + ArenaHalfExtent - 100.f),
		FMath::Clamp(WorldLocation.Y, ArenaOrigin.Y - ArenaHalfExtent + 100.f, ArenaOrigin.Y + ArenaHalfExtent - 100.f),
		ArenaOrigin.Z + 20.f);
}

void AT66MiniGameMode::SaveRunProgressNow(const bool bMarkMidWaveSnapshot)
{
	PersistActiveRunSnapshot(bMarkMidWaveSnapshot);
}

void AT66MiniGameMode::PositionPlayerPawns()
{
	const TArray<AT66MiniPlayerPawn*> PlayerPawns = T66MiniGatherPlayerPawns(GetWorld(), false);
	if (PlayerPawns.Num() == 0)
	{
		return;
	}

	const float FormationRadius = PlayerPawns.Num() > 1 ? 150.f : 0.f;
	bool bPositionedAnyPawn = false;
	for (int32 Index = 0; Index < PlayerPawns.Num(); ++Index)
	{
		AT66MiniPlayerPawn* PlayerPawn = PlayerPawns[Index];
		if (!PlayerPawn || PositionedPlayerPawns.Contains(PlayerPawn))
		{
			continue;
		}

		const float Angle = PlayerPawns.Num() > 1 ? (UE_TWO_PI * static_cast<float>(Index) / static_cast<float>(PlayerPawns.Num())) : 0.f;
		const FVector SpawnOffset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * FormationRadius;
		PlayerPawn->SetActorLocation(ClampPointToArena(ArenaOrigin + SpawnOffset));
		PositionedPlayerPawns.Add(PlayerPawn);
		bPositionedAnyPawn = true;
	}

	if (bPositionedAnyPawn)
	{
		SpawnCompanionActor();
	}
}

void AT66MiniGameMode::SpawnArenaAndPositionPlayer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	MiniArenaActor = World->SpawnActor<AT66MiniArena>(AT66MiniArena::StaticClass(), ArenaOrigin, FRotator::ZeroRotator, SpawnParams);
	if (MiniArenaActor)
	{
		MiniArenaActor->InitializeArena(
			ArenaOrigin,
			ArenaHalfExtent,
			VisualSubsystem ? VisualSubsystem->LoadBackgroundTexture() : nullptr);
	}
}

void AT66MiniGameMode::StartWave(const int32 WaveIndex, const bool bResetTimer)
{
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	if (!MiniGameState)
	{
		return;
	}

	MiniGameState->WaveIndex = FMath::Clamp(WaveIndex, 1, GetMaxStageIndexForCurrentDifficulty());
	if (bResetTimer)
	{
		if (const FT66MiniWaveDefinition* WaveDefinition = GetWaveDefinition())
		{
			MiniGameState->WaveSecondsRemaining = WaveDefinition->DurationSeconds;
		}
		else
		{
			MiniGameState->WaveSecondsRemaining = 60.f;
		}
	}

	EnemySpawnAccumulator = 0.f;
	InteractableSpawnAccumulator = 0.f;
	TrapSpawnAccumulator = 0.f;
	PostBossDelayRemaining = 0.f;
	BossTelegraphRemaining = 0.f;
	PendingBossID = NAME_None;
	PendingBossSpawnLocation = FVector::ZeroVector;
	bBossSpawnedForWave = false;

	for (AT66MiniPlayerPawn* PlayerPawn : T66MiniGatherPlayerPawns(GetWorld(), false))
	{
		PlayerPawn->HandleWaveStarted(MiniGameState->WaveIndex);
	}
}

const FT66MiniWaveDefinition* AT66MiniGameMode::GetWaveDefinition() const
{
	UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	if (!DataSubsystem || !MiniGameState)
	{
		return nullptr;
	}

	return DataSubsystem->FindWave(MiniGameState->DifficultyID, MiniGameState->WaveIndex);
}

const FT66MiniStageDefinition* AT66MiniGameMode::GetCurrentStageDefinition() const
{
	UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	if (!DataSubsystem || !MiniGameState)
	{
		return nullptr;
	}

	if (const FT66MiniStageDefinition* StageDefinition = DataSubsystem->FindStage(MiniGameState->DifficultyID, MiniGameState->WaveIndex))
	{
		return StageDefinition;
	}

	return DataSubsystem->FindStageForWave(MiniGameState->DifficultyID, MiniGameState->WaveIndex);
}

const FT66MiniDifficultyDefinition* AT66MiniGameMode::GetDifficultyDefinition() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	if (!DataSubsystem || !MiniGameState)
	{
		return nullptr;
	}

	return DataSubsystem->FindDifficulty(MiniGameState->DifficultyID);
}

int32 AT66MiniGameMode::GetMaxStageIndexForCurrentDifficulty() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	const int32 DataMaxStage = (DataSubsystem && MiniGameState) ? DataSubsystem->GetMaxStageIndexForDifficulty(MiniGameState->DifficultyID) : 0;
	return DataMaxStage > 0 ? DataMaxStage : GetRuntimeTuningInt(TEXT("MaxWavesPerDifficulty"), 5);
}

const FT66MiniEnemyDefinition* AT66MiniGameMode::ChooseEnemyDefinition() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	if (!DataSubsystem)
	{
		return nullptr;
	}

	TArray<const FT66MiniEnemyDefinition*> CandidateDefinitions;
	float TotalWeight = 0.f;

	if (const FT66MiniWaveDefinition* WaveDefinition = GetWaveDefinition())
	{
		for (const FName EnemyID : WaveDefinition->EnemyIDs)
		{
			if (const FT66MiniEnemyDefinition* EnemyDefinition = DataSubsystem->FindEnemy(EnemyID))
			{
				CandidateDefinitions.Add(EnemyDefinition);
				TotalWeight += FMath::Max(0.1f, EnemyDefinition->SpawnWeight);
			}
		}
	}

	if (CandidateDefinitions.Num() == 0)
	{
		for (const FT66MiniEnemyDefinition& EnemyDefinition : DataSubsystem->GetEnemies())
		{
			CandidateDefinitions.Add(&EnemyDefinition);
			TotalWeight += FMath::Max(0.1f, EnemyDefinition.SpawnWeight);
		}
	}

	if (CandidateDefinitions.Num() == 0)
	{
		return nullptr;
	}

	float PickWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const FT66MiniEnemyDefinition* Candidate : CandidateDefinitions)
	{
		PickWeight -= FMath::Max(0.1f, Candidate->SpawnWeight);
		if (PickWeight <= 0.f)
		{
			return Candidate;
		}
	}

	return CandidateDefinitions.Last();
}

void AT66MiniGameMode::SpawnWaveEnemy()
{
	UWorld* World = GetWorld();
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	AT66MiniPlayerPawn* PlayerPawn = T66MiniChooseAnchorPawn(World);
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	const FT66MiniEnemyDefinition* EnemyDefinition = ChooseEnemyDefinition();
	const FT66MiniWaveDefinition* WaveDefinition = GetWaveDefinition();
	const FT66MiniDifficultyDefinition* DifficultyDefinition = GetDifficultyDefinition();
	if (!World || !MiniGameState || !PlayerPawn || !VisualSubsystem || !EnemyDefinition)
	{
		return;
	}

	const float SpawnAngle = FMath::FRandRange(0.f, UE_TWO_PI);
	const FVector SpawnDirection = FVector(FMath::Cos(SpawnAngle), FMath::Sin(SpawnAngle), 0.f);
	FVector SpawnLocation = PlayerPawn->GetActorLocation() + (SpawnDirection * FMath::FRandRange(1450.f, 1900.f));
	SpawnLocation = ClampPointToArena(SpawnLocation);

	const float DifficultyHealthScalar = DifficultyDefinition ? DifficultyDefinition->HealthScalar : 1.0f;
	const float DifficultyDamageScalar = DifficultyDefinition ? DifficultyDefinition->DamageScalar : 1.0f;
	const float DifficultySpeedScalar = DifficultyDefinition ? DifficultyDefinition->SpeedScalar : 1.0f;
	const float WaveHealthScalar = WaveDefinition ? WaveDefinition->EnemyHealthScalar : 1.0f;
	const float WaveDamageScalar = WaveDefinition ? WaveDefinition->EnemyDamageScalar : 1.0f;
	const float WaveSpeedScalar = WaveDefinition ? WaveDefinition->EnemySpeedScalar : 1.0f;
	const float ProgressScalar = GetRuntimeTuningValue(TEXT("EnemyProgressScalarBase"), 1.0f)
		+ ((MiniGameState->WaveIndex - 1) * GetRuntimeTuningValue(TEXT("EnemyProgressScalarPerWave"), 0.16f));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniEnemyBase* Enemy = World->SpawnActor<AT66MiniEnemyBase>(AT66MiniEnemyBase::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Enemy->InitializeEnemy(
			EnemyDefinition->EnemyID,
			false,
			VisualSubsystem->LoadEnemyTexture(EnemyDefinition->VisualID),
			EnemyDefinition->BaseHealth * DifficultyHealthScalar * WaveHealthScalar * ProgressScalar,
			EnemyDefinition->BaseSpeed * DifficultySpeedScalar * WaveSpeedScalar,
			EnemyDefinition->BaseTouchDamage * DifficultyDamageScalar * WaveDamageScalar,
			FMath::RoundToInt(EnemyDefinition->BaseMaterials * ProgressScalar),
			EnemyDefinition->BaseExperience * ProgressScalar,
			-1.f,
			EnemyDefinition->BehaviorProfile,
			EnemyDefinition->Family,
			EnemyDefinition->FireIntervalSeconds / FMath::Max(GetRuntimeTuningValue(TEXT("EnemyRangedFireSpawnRateMin"), 0.80f), DifficultyDefinition ? DifficultyDefinition->SpawnRateScalar : 1.0f),
			EnemyDefinition->ProjectileSpeed,
			EnemyDefinition->ProjectileDamage * DifficultyDamageScalar * WaveDamageScalar,
			EnemyDefinition->PreferredRange);
		LiveEnemies.Add(Enemy);
		if (UT66MiniVFXSubsystem* VfxSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnPulse(World, SpawnLocation + FVector(0.f, 0.f, 6.f), FVector(0.20f, 0.20f, 1.f), 0.14f, FLinearColor(0.96f, 0.20f, 0.18f, 0.18f), 0.55f);
		}
	}
}

void AT66MiniGameMode::BeginBossSpawnTelegraph()
{
	if (bBossSpawnedForWave || BossTelegraphRemaining > 0.f)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UWorld* World = GetWorld();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const FT66MiniWaveDefinition* WaveDefinition = GetWaveDefinition();
	AT66MiniPlayerPawn* PlayerPawn = T66MiniChooseAnchorPawn(World);
	if (!World || !DataSubsystem || !WaveDefinition || !PlayerPawn)
	{
		SpawnBossEnemy();
		bBossSpawnedForWave = true;
		return;
	}

	if (const FT66MiniStageDefinition* StageDefinition = GetCurrentStageDefinition(); StageDefinition && !StageDefinition->BossID.IsNone())
	{
		PendingBossID = StageDefinition->BossID;
	}
	else
	{
		PendingBossID = WaveDefinition->BossID;
	}
	PendingBossSpawnLocation = ClampPointToArena(PlayerPawn->GetActorLocation() + FVector(ArenaHalfExtent * GetRuntimeTuningValue(TEXT("BossSpawnOffsetScalar"), 0.72f), 0.f, 0.f));

	const FT66MiniBossDefinition* BossDefinition = DataSubsystem->FindBoss(PendingBossID);
	BossTelegraphRemaining = BossDefinition ? BossDefinition->TelegraphSeconds : GetRuntimeTuningValue(TEXT("BossTelegraphFallbackSeconds"), 1.2f);
	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->DeactivateTelegraph();
		ActiveBossTelegraphActor = nullptr;
	}
	if (UT66MiniVFXSubsystem* VfxSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		ActiveBossTelegraphActor = VfxSubsystem->SpawnGroundTelegraph(World, PendingBossSpawnLocation, GetRuntimeTuningValue(TEXT("BossTelegraphRadius"), 260.f), BossTelegraphRemaining, FLinearColor(0.98f, 0.26f, 0.20f, 0.38f));
		VfxSubsystem->PlayBossAlertSfx(this);
	}
}

void AT66MiniGameMode::SpawnBossEnemy()
{
	UWorld* World = GetWorld();
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	const FT66MiniDifficultyDefinition* DifficultyDefinition = GetDifficultyDefinition();
	if (!World || !MiniGameState || !DataSubsystem || !VisualSubsystem)
	{
		return;
	}

	const FName BossID = PendingBossID.IsNone()
		? (GetCurrentStageDefinition() && !GetCurrentStageDefinition()->BossID.IsNone()
			? GetCurrentStageDefinition()->BossID
			: (GetWaveDefinition() ? GetWaveDefinition()->BossID : NAME_None))
		: PendingBossID;
	const FT66MiniBossDefinition* BossDefinition = DataSubsystem->FindBoss(BossID);
	if (!BossDefinition)
	{
		return;
	}

	const FVector SpawnLocation = PendingBossSpawnLocation.IsNearlyZero()
		? ClampPointToArena(ArenaOrigin + FVector(ArenaHalfExtent * GetRuntimeTuningValue(TEXT("BossFallbackOffsetScalar"), 0.70f), 0.f, 0.f))
		: PendingBossSpawnLocation;
	const float DifficultyScalar = DifficultyDefinition ? DifficultyDefinition->BossScalar : 1.0f;
	const float WaveScalar = GetRuntimeTuningValue(TEXT("BossWaveScalarBase"), 1.0f)
		+ (MiniGameState->WaveIndex * GetRuntimeTuningValue(TEXT("BossWaveScalarPerWave"), 0.14f));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniEnemyBase* Boss = World->SpawnActor<AT66MiniEnemyBase>(AT66MiniEnemyBase::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Boss->InitializeEnemy(
			BossDefinition->BossID,
			true,
			VisualSubsystem->LoadBossTexture(BossDefinition->VisualID),
			BossDefinition->MaxHealth * DifficultyScalar * WaveScalar,
			BossDefinition->MoveSpeed,
			BossDefinition->TouchDamage * DifficultyScalar,
			FMath::RoundToInt(BossDefinition->MaterialReward * DifficultyScalar),
			BossDefinition->ExperienceReward * DifficultyScalar,
			-1.f,
			BossDefinition->BehaviorProfile,
			BossDefinition->Family,
			BossDefinition->FireIntervalSeconds / FMath::Max(GetRuntimeTuningValue(TEXT("BossFireIntervalScalarMin"), 0.85f), DifficultyScalar),
			BossDefinition->ProjectileSpeed,
			BossDefinition->ProjectileDamage * DifficultyScalar,
			GetRuntimeTuningValue(TEXT("BossPreferredRange"), 960.f));
		LiveEnemies.Add(Boss);
	}

	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->DeactivateTelegraph();
		ActiveBossTelegraphActor = nullptr;
	}
	if (UT66MiniVFXSubsystem* VfxSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		VfxSubsystem->SpawnPulse(World, SpawnLocation + FVector(0.f, 0.f, 8.f), FVector(0.72f, 0.72f, 1.f), 0.24f, FLinearColor(1.0f, 0.58f, 0.20f, 0.36f), 1.00f);
		VfxSubsystem->PlayBossSpawnSfx(this);
	}

	BossTelegraphRemaining = 0.f;
	PendingBossID = NAME_None;
	PendingBossSpawnLocation = FVector::ZeroVector;
}

void AT66MiniGameMode::SpawnRandomInteractable()
{
	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	if (!World || !DataSubsystem || !VisualSubsystem || DataSubsystem->GetInteractables().Num() == 0)
	{
		return;
	}

	float TotalWeight = 0.f;
	for (const FT66MiniInteractableDefinition& Definition : DataSubsystem->GetInteractables())
	{
		TotalWeight += FMath::Max(0.1f, Definition.SpawnWeight);
	}

	float PickWeight = FMath::FRandRange(0.f, TotalWeight);
	const FT66MiniInteractableDefinition* PickedDefinition = &DataSubsystem->GetInteractables()[0];
	for (const FT66MiniInteractableDefinition& Definition : DataSubsystem->GetInteractables())
	{
		PickWeight -= FMath::Max(0.1f, Definition.SpawnWeight);
		if (PickWeight <= 0.f)
		{
			PickedDefinition = &Definition;
			break;
		}
	}

	const FVector SpawnLocation(
		FMath::FRandRange(ArenaOrigin.X - (ArenaHalfExtent * 0.65f), ArenaOrigin.X + (ArenaHalfExtent * 0.65f)),
		FMath::FRandRange(ArenaOrigin.Y - (ArenaHalfExtent * 0.65f), ArenaOrigin.Y + (ArenaHalfExtent * 0.65f)),
		ArenaOrigin.Z + 20.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniInteractable* Interactable = World->SpawnActor<AT66MiniInteractable>(AT66MiniInteractable::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Interactable->InitializeInteractable(
			PickedDefinition->Type,
			VisualSubsystem->LoadInteractableTexture(PickedDefinition->VisualID),
			PickedDefinition->LifetimeSeconds,
			PickedDefinition->VisualID,
			PickedDefinition->MaterialReward,
			PickedDefinition->GoldReward,
			PickedDefinition->ExperienceReward,
			PickedDefinition->HealAmount,
			PickedDefinition->bRequiresManualInteract);
	}
}

void AT66MiniGameMode::SpawnRandomTrap()
{
	UWorld* World = GetWorld();
	AT66MiniPlayerPawn* PlayerPawn = T66MiniChooseAnchorPawn(World);
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	if (!World || !PlayerPawn || !MiniGameState)
	{
		return;
	}

	const float SpawnAngle = FMath::FRandRange(0.f, UE_TWO_PI);
	const FVector SpawnDirection = FVector(FMath::Cos(SpawnAngle), FMath::Sin(SpawnAngle), 0.f);
	FVector SpawnLocation = PlayerPawn->GetActorLocation() + (SpawnDirection * FMath::FRandRange(
		GetRuntimeTuningValue(TEXT("TrapSpawnMinDistance"), 360.f),
		GetRuntimeTuningValue(TEXT("TrapSpawnMaxDistance"), 980.f)));
	SpawnLocation = ClampPointToArena(SpawnLocation);

	const float Radius = GetRuntimeTuningValue(TEXT("TrapRadiusBase"), 180.f)
		+ (MiniGameState->WaveIndex * GetRuntimeTuningValue(TEXT("TrapRadiusPerWave"), 24.f))
		+ FMath::FRandRange(0.f, GetRuntimeTuningValue(TEXT("TrapRadiusRandomMax"), 80.f));
	const float Damage = GetRuntimeTuningValue(TEXT("TrapDamageBase"), 7.f)
		+ (MiniGameState->WaveIndex * GetRuntimeTuningValue(TEXT("TrapDamagePerWave"), 1.3f));
	const float Warmup = GetRuntimeTuningValue(TEXT("TrapWarmupBase"), 0.95f)
		+ FMath::FRandRange(0.0f, GetRuntimeTuningValue(TEXT("TrapWarmupRandomMax"), 0.35f));
	const float ActiveSeconds = GetRuntimeTuningValue(TEXT("TrapActiveBase"), 3.8f)
		+ (MiniGameState->WaveIndex * GetRuntimeTuningValue(TEXT("TrapActivePerWave"), 0.28f));
	const float PulseInterval = FMath::Max(
		GetRuntimeTuningValue(TEXT("TrapPulseIntervalMin"), 0.28f),
		GetRuntimeTuningValue(TEXT("TrapPulseIntervalBase"), 0.72f) - (MiniGameState->WaveIndex * GetRuntimeTuningValue(TEXT("TrapPulseIntervalPerWave"), 0.04f)));
	const int32 TrapVariant = FMath::RandRange(0, GetRuntimeTuningInt(TEXT("TrapVariantMax"), 2));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AT66MiniHazardTrap* Trap = World->SpawnActor<AT66MiniHazardTrap>(AT66MiniHazardTrap::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Trap->InitializeTrap(SpawnLocation, Radius, Damage, PulseInterval, Warmup, ActiveSeconds, TrapVariant);
		LiveTraps.Add(Trap);
	}
}

void AT66MiniGameMode::ReturnToShopIntermission()
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GameInstance);
	if (!RunState || !DataSubsystem || !FrontendState || !SaveSubsystem || !ActiveRun || !T66GameInstance)
	{
		return;
	}

	ActiveRun->bPendingShopIntermission = true;
	ActiveRun->bHasMidWaveSnapshot = false;
	ActiveRun->bHasPlayerLocation = false;
	ActiveRun->PlayerLocation = FVector::ZeroVector;
	ActiveRun->bBossSpawnedForWave = false;
	ActiveRun->BossTelegraphRemaining = 0.f;
	ActiveRun->PendingBossID = NAME_None;
	ActiveRun->PendingBossSpawnLocation = FVector::ZeroVector;
	ActiveRun->EnemySpawnAccumulator = 0.f;
	ActiveRun->InteractableSpawnAccumulator = 0.f;
	ActiveRun->TrapSpawnAccumulator = 0.f;
	ActiveRun->PostBossDelayRemaining = 0.f;
	ActiveRun->EnemySnapshots.Reset();
	ActiveRun->PickupSnapshots.Reset();
	ActiveRun->InteractableSnapshots.Reset();
	ActiveRun->TrapSnapshots.Reset();
	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->DeactivateTelegraph();
		ActiveBossTelegraphActor = nullptr;
	}
	FrontendState->SeedFromRunSave(ActiveRun);
	FrontendState->EnterIntermissionFlow(DataSubsystem);
	FrontendState->WriteIntermissionStateToRunSave(ActiveRun);
	SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);

	T66GameInstance->PendingFrontendScreen = ET66ScreenType::MiniShop;
	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
}

void AT66MiniGameMode::RecordClearedMiniStageProgression()
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const FT66MiniStageDefinition* StageDefinition = GetCurrentStageDefinition();
	const FT66MiniDifficultyDefinition* DifficultyDefinition = (ActiveRun && DataSubsystem)
		? DataSubsystem->FindDifficulty(ActiveRun->DifficultyID)
		: nullptr;
	const int32 ChadCouponsAwarded = StageDefinition && StageDefinition->ClearChadCoupons > 0
		? StageDefinition->ClearChadCoupons
		: (DifficultyDefinition
		? FMath::Max(0, DifficultyDefinition->StageClearChadCoupons)
		: 0);
	if (!SaveSubsystem || !DataSubsystem || !ActiveRun)
	{
		return;
	}

	if (StageDefinition)
	{
		ActiveRun->CurrentStageID = StageDefinition->StageID;
		ActiveRun->StageIndex = StageDefinition->StageIndex;
		ActiveRun->Gold += FMath::Max(0, StageDefinition->ClearGoldReward);
		ActiveRun->Materials += FMath::Max(0, StageDefinition->ClearMaterialReward);
	}
	else
	{
		ActiveRun->StageIndex = FMath::Max(1, ActiveRun->WaveIndex);
	}

	SaveSubsystem->RecordClearedMiniStage(ActiveRun->CompanionID, DataSubsystem);
	if (UT66AchievementsSubsystem* Achievements = GameInstance ? GameInstance->GetSubsystem<UT66AchievementsSubsystem>() : nullptr)
	{
		Achievements->AddChadCoupons(ChadCouponsAwarded);
	}
}

void AT66MiniGameMode::FinalizeRun(const bool bWasVictory, const FString& ResultLabel)
{
	if (bRunFinalized)
	{
		return;
	}

	bRunFinalized = true;

	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GameInstance ? GameInstance->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	UT66GameInstance* T66GameInstance = Cast<UT66GameInstance>(GameInstance);
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	AT66MiniPlayerPawn* PlayerPawn = T66MiniChooseAnchorPawn(GetWorld());
	if (!RunState || !FrontendState || !SaveSubsystem || !DataSubsystem || !MiniGameState || !T66GameInstance || !ActiveRun)
	{
		return;
	}

	if (BattleMusicComponent)
	{
		BattleMusicComponent->Stop();
	}
	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->DeactivateTelegraph();
		ActiveBossTelegraphActor = nullptr;
	}

	FT66MiniRunSummary Summary;
	Summary.bHasSummary = true;
	Summary.bWasVictory = bWasVictory;
	Summary.HeroID = ActiveRun->HeroID;
	Summary.CompanionID = ActiveRun->CompanionID;
	Summary.DifficultyID = ActiveRun->DifficultyID;
	Summary.WaveReached = FMath::Max(1, MiniGameState->WaveIndex);
	Summary.GoldCollected = PlayerPawn ? PlayerPawn->GetGold() : ActiveRun->Gold;
	Summary.MaterialsCollected = PlayerPawn ? PlayerPawn->GetMaterials() : ActiveRun->Materials;
	Summary.OwnedItemCount = ActiveRun->OwnedItemIDs.Num();
	Summary.EquippedIdolCount = ActiveRun->EquippedIdolIDs.Num();
	Summary.RunSeconds = ActiveRun->TotalRunSeconds;
	Summary.ResultLabel = ResultLabel;
	Summary.LastUpdatedUtc = FDateTime::UtcNow().ToIso8601();

	if (const FT66MiniHeroDefinition* HeroDefinition = DataSubsystem->FindHero(Summary.HeroID))
	{
		Summary.HeroDisplayName = HeroDefinition->DisplayName;
	}
	else
	{
		Summary.HeroDisplayName = Summary.HeroID.ToString();
	}

	if (const FT66MiniCompanionDefinition* CompanionDefinition = DataSubsystem->FindCompanion(Summary.CompanionID))
	{
		Summary.CompanionDisplayName = CompanionDefinition->DisplayName;
	}
	else
	{
		Summary.CompanionDisplayName = Summary.CompanionID.ToString();
	}

	if (const FT66MiniDifficultyDefinition* DifficultyDefinition = DataSubsystem->FindDifficulty(Summary.DifficultyID))
	{
		Summary.DifficultyDisplayName = DifficultyDefinition->DisplayName;
	}
	else
	{
		Summary.DifficultyDisplayName = Summary.DifficultyID.ToString();
	}

	FrontendState->SetLastRunSummary(Summary);
	FrontendState->ExitIntermissionFlow();
	SaveSubsystem->RecordRunSummary(Summary, DataSubsystem);
	if (UT66MiniLeaderboardSubsystem* MiniLeaderboardSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniLeaderboardSubsystem>() : nullptr)
	{
		MiniLeaderboardSubsystem->SubmitScore(
			Summary.DifficultyID,
			Summary.MaterialsCollected);
	}
	if (UT66BackendSubsystem* Backend = GameInstance ? GameInstance->GetSubsystem<UT66BackendSubsystem>() : nullptr)
	{
		const UT66MiniFrontendStateSubsystem* SubmitFrontendState = GameInstance ? GameInstance->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
		const UT66SteamHelper* SteamHelper = GameInstance ? GameInstance->GetSubsystem<UT66SteamHelper>() : nullptr;
		const bool bDailyRun = SubmitFrontendState && SubmitFrontendState->IsDailyRun();
		Backend->SubmitMinigameScore(
			SteamHelper ? SteamHelper->GetLocalDisplayName() : FString(TEXT("Player")),
			TEXT("mini"),
			bDailyRun ? TEXT("daily") : TEXT("alltime"),
			Summary.DifficultyID.ToString().ToLower(),
			FMath::Max(0, Summary.MaterialsCollected),
			bDailyRun ? SubmitFrontendState->GetDailyChallengeId() : FString(),
			bDailyRun ? SubmitFrontendState->GetDailySeed() : 0);
	}
	if (RunState->GetActiveSaveSlot() != INDEX_NONE)
	{
		SaveSubsystem->DeleteRunFromSlot(RunState->GetActiveSaveSlot());
	}
	RunState->ResetActiveRun();
	T66GameInstance->PendingFrontendScreen = ET66ScreenType::MiniRunSummary;
	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
}

void AT66MiniGameMode::TryApplySavedPawnState()
{
	if (bAppliedSavedPawnState)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!ActiveRun || !ActiveRun->bHasMidWaveSnapshot)
	{
		bAppliedSavedPawnState = true;
		return;
	}

	if (!ActiveRun->bHasPlayerLocation)
	{
		bAppliedSavedPawnState = true;
		return;
	}

	AT66MiniPlayerPawn* PlayerPawn = T66MiniChooseAnchorPawn(GetWorld());
	if (!PlayerPawn)
	{
		return;
	}

	PlayerPawn->SetActorLocation(ClampPointToArena(ActiveRun->PlayerLocation));
	bAppliedSavedPawnState = true;
}

void AT66MiniGameMode::SpawnCompanionActor()
{
	for (AT66MiniCompanionBase* CompanionActor : ActiveCompanionActors)
	{
		if (CompanionActor)
		{
			CompanionActor->Destroy();
		}
	}
	ActiveCompanionActors.Reset();

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	const TArray<AT66MiniPlayerPawn*> PlayerPawns = T66MiniGatherPlayerPawns(World, false);
	if (!World || !DataSubsystem || PlayerPawns.Num() == 0)
	{
		return;
	}

	if (MiniGameState && PlayerPawns.Num() > 0)
	{
		MiniGameState->CompanionID = PlayerPawns[0]->GetSelectedCompanionID();
	}

	for (AT66MiniPlayerPawn* PlayerPawn : PlayerPawns)
	{
		if (!PlayerPawn)
		{
			continue;
		}

		FName CompanionID = PlayerPawn->GetSelectedCompanionID();
		if (CompanionID.IsNone() && ActiveRun)
		{
			CompanionID = ActiveRun->CompanionID;
		}
		if ((CompanionID == NAME_None || (SaveSubsystem && !SaveSubsystem->IsCompanionUnlocked(CompanionID, DataSubsystem))) && SaveSubsystem)
		{
			CompanionID = SaveSubsystem->GetFirstUnlockedCompanionID(DataSubsystem);
		}

		const FT66MiniCompanionDefinition* CompanionDefinition = DataSubsystem->FindCompanion(CompanionID);
		if (!CompanionDefinition)
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AT66MiniCompanionBase* CompanionActor = World->SpawnActor<AT66MiniCompanionBase>(
			AT66MiniCompanionBase::StaticClass(),
			ClampPointToArena(PlayerPawn->GetActorLocation() + FVector(CompanionDefinition->FollowOffsetX, CompanionDefinition->FollowOffsetY, 0.f)),
			FRotator::ZeroRotator,
			SpawnParams))
		{
			CompanionActor->InitializeCompanion(*CompanionDefinition, PlayerPawn);
			ActiveCompanionActors.Add(CompanionActor);
		}
	}
}

void AT66MiniGameMode::RestoreTransientWaveState(const UT66MiniRunSaveGame* RunSave)
{
	if (!RunSave)
	{
		return;
	}

	bBossSpawnedForWave = RunSave->bBossSpawnedForWave;
	BossTelegraphRemaining = FMath::Max(0.f, RunSave->BossTelegraphRemaining);
	PendingBossID = RunSave->PendingBossID;
	PendingBossSpawnLocation = RunSave->PendingBossSpawnLocation;
	EnemySpawnAccumulator = FMath::Max(0.f, RunSave->EnemySpawnAccumulator);
	InteractableSpawnAccumulator = FMath::Max(0.f, RunSave->InteractableSpawnAccumulator);
	TrapSpawnAccumulator = FMath::Max(0.f, RunSave->TrapSpawnAccumulator);
	PostBossDelayRemaining = FMath::Max(0.f, RunSave->PostBossDelayRemaining);

	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->DeactivateTelegraph();
		ActiveBossTelegraphActor = nullptr;
	}

	if (BossTelegraphRemaining > 0.f && !PendingBossSpawnLocation.IsNearlyZero())
	{
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			ActiveBossTelegraphActor = VfxSubsystem->SpawnGroundTelegraph(GetWorld(), PendingBossSpawnLocation, GetRuntimeTuningValue(TEXT("BossTelegraphRadius"), 260.f), BossTelegraphRemaining, FLinearColor(0.98f, 0.26f, 0.20f, 0.38f));
		}
	}
}

void AT66MiniGameMode::RestoreWorldState(const UT66MiniRunSaveGame* RunSave)
{
	if (!RunSave)
	{
		return;
	}

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniVisualSubsystem* VisualSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr;
	if (!World || !DataSubsystem || !VisualSubsystem)
	{
		return;
	}

	LiveEnemies.Reset();
	LiveTraps.Reset();
	LiveInteractables.Reset();
	LivePickups.Reset();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FT66MiniEnemySnapshot& Snapshot : RunSave->EnemySnapshots)
	{
		ET66MiniEnemyBehaviorProfile BehaviorProfile = ET66MiniEnemyBehaviorProfile::Balanced;
		ET66MiniEnemyFamily EnemyFamily = Snapshot.bIsBoss ? ET66MiniEnemyFamily::Boss : ET66MiniEnemyFamily::Melee;
		FString VisualID = Snapshot.EnemyID.ToString();
		float FireIntervalSeconds = 1.8f;
		float ProjectileSpeed = 980.f;
		float ProjectileDamage = Snapshot.TouchDamage;
		float PreferredRange = 860.f;
		if (Snapshot.bIsBoss)
		{
			if (const FT66MiniBossDefinition* BossDefinition = DataSubsystem->FindBoss(Snapshot.EnemyID))
			{
				BehaviorProfile = BossDefinition->BehaviorProfile;
				EnemyFamily = BossDefinition->Family;
				VisualID = BossDefinition->VisualID;
				FireIntervalSeconds = BossDefinition->FireIntervalSeconds;
				ProjectileSpeed = BossDefinition->ProjectileSpeed;
				ProjectileDamage = BossDefinition->ProjectileDamage;
				PreferredRange = GetRuntimeTuningValue(TEXT("BossPreferredRange"), 960.f);
			}
		}
		else if (const FT66MiniEnemyDefinition* EnemyDefinition = DataSubsystem->FindEnemy(Snapshot.EnemyID))
		{
			BehaviorProfile = EnemyDefinition->BehaviorProfile;
			EnemyFamily = EnemyDefinition->Family;
			VisualID = EnemyDefinition->VisualID;
			FireIntervalSeconds = EnemyDefinition->FireIntervalSeconds;
			ProjectileSpeed = EnemyDefinition->ProjectileSpeed;
			ProjectileDamage = EnemyDefinition->ProjectileDamage;
			PreferredRange = EnemyDefinition->PreferredRange;
		}

		UTexture2D* Texture = Snapshot.bIsBoss ? VisualSubsystem->LoadBossTexture(VisualID) : VisualSubsystem->LoadEnemyTexture(VisualID);
		if (AT66MiniEnemyBase* Enemy = World->SpawnActor<AT66MiniEnemyBase>(AT66MiniEnemyBase::StaticClass(), ClampPointToArena(Snapshot.Location), FRotator::ZeroRotator, SpawnParams))
		{
			Enemy->InitializeEnemy(
				Snapshot.EnemyID,
				Snapshot.bIsBoss,
				Texture,
				Snapshot.MaxHealth,
				Snapshot.MoveSpeed,
				Snapshot.TouchDamage,
				Snapshot.MaterialDrop,
				Snapshot.ExperienceDrop,
				Snapshot.CurrentHealth,
				BehaviorProfile,
				EnemyFamily,
				FireIntervalSeconds,
				ProjectileSpeed,
				ProjectileDamage,
				PreferredRange);
			LiveEnemies.Add(Enemy);
		}
	}

	for (const FT66MiniPickupSnapshot& Snapshot : RunSave->PickupSnapshots)
	{
		if (AT66MiniPickup* Pickup = World->SpawnActor<AT66MiniPickup>(AT66MiniPickup::StaticClass(), ClampPointToArena(Snapshot.Location), FRotator::ZeroRotator, SpawnParams))
		{
			Pickup->InitializePickup(
				Snapshot.MaterialValue,
				Snapshot.ExperienceValue,
				Snapshot.HealValue,
				Snapshot.VisualID.IsEmpty() ? nullptr : VisualSubsystem->LoadInteractableTexture(Snapshot.VisualID),
				Snapshot.VisualID,
				Snapshot.LifetimeRemaining,
				Snapshot.GrantedItemID);
		}
	}

	for (const FT66MiniInteractableSnapshot& Snapshot : RunSave->InteractableSnapshots)
	{
		const FT66MiniInteractableDefinition* Definition = DataSubsystem->FindInteractable(FName(*Snapshot.VisualID));
		if (AT66MiniInteractable* Interactable = World->SpawnActor<AT66MiniInteractable>(AT66MiniInteractable::StaticClass(), ClampPointToArena(Snapshot.Location), FRotator::ZeroRotator, SpawnParams))
		{
			Interactable->InitializeInteractable(
				static_cast<ET66MiniInteractableType>(Snapshot.InteractableType),
				Snapshot.VisualID.IsEmpty() ? nullptr : VisualSubsystem->LoadInteractableTexture(Snapshot.VisualID),
				Snapshot.LifetimeRemaining,
				Snapshot.VisualID,
				Definition ? Definition->MaterialReward : 0,
				Definition ? Definition->GoldReward : 0,
				Definition ? Definition->ExperienceReward : 0.f,
				Definition ? Definition->HealAmount : 0.f,
				Definition ? Definition->bRequiresManualInteract : static_cast<ET66MiniInteractableType>(Snapshot.InteractableType) == ET66MiniInteractableType::QuickReviveMachine);
		}
	}

	for (const FT66MiniTrapSnapshot& Snapshot : RunSave->TrapSnapshots)
	{
		if (AT66MiniHazardTrap* Trap = World->SpawnActor<AT66MiniHazardTrap>(AT66MiniHazardTrap::StaticClass(), ClampPointToArena(Snapshot.Location), FRotator::ZeroRotator, SpawnParams))
		{
			const float ActiveSeconds = FMath::Max(0.2f, Snapshot.ActiveRemaining);
			const float WarmupSeconds = FMath::Max(0.f, Snapshot.WarmupRemaining);
			Trap->InitializeTrap(ClampPointToArena(Snapshot.Location), Snapshot.Radius, Snapshot.DamagePerPulse, Snapshot.PulseInterval, WarmupSeconds, ActiveSeconds, Snapshot.TrapVariant);
			LiveTraps.Add(Trap);
		}
	}
}

void AT66MiniGameMode::CaptureWorldState(UT66MiniRunSaveGame* RunSave) const
{
	if (!RunSave)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	RunSave->EnemySnapshots.Reset();
	RunSave->PickupSnapshots.Reset();
	RunSave->InteractableSnapshots.Reset();
	RunSave->TrapSnapshots.Reset();

	for (const AT66MiniEnemyBase* Enemy : LiveEnemies)
	{
		if (!Enemy || Enemy->IsEnemyDead())
		{
			continue;
		}

		FT66MiniEnemySnapshot& Snapshot = RunSave->EnemySnapshots.AddDefaulted_GetRef();
		Snapshot.EnemyID = Enemy->GetEnemyID();
		Snapshot.bIsBoss = Enemy->IsBossEnemy();
		Snapshot.Location = Enemy->GetActorLocation();
		Snapshot.CurrentHealth = Enemy->GetCurrentHealth();
		Snapshot.MaxHealth = Enemy->GetMaxHealth();
		Snapshot.MoveSpeed = Enemy->GetMoveSpeed();
		Snapshot.TouchDamage = Enemy->GetTouchDamage();
		Snapshot.MaterialDrop = Enemy->GetMaterialDrop();
		Snapshot.ExperienceDrop = Enemy->GetExperienceDrop();
	}

	for (const AT66MiniPickup* Pickup : LivePickups)
	{
		if (!Pickup)
		{
			continue;
		}

		FT66MiniPickupSnapshot& Snapshot = RunSave->PickupSnapshots.AddDefaulted_GetRef();
		Snapshot.Location = Pickup->GetActorLocation();
		Snapshot.VisualID = Pickup->GetVisualID();
		Snapshot.MaterialValue = Pickup->GetMaterialValue();
		Snapshot.ExperienceValue = Pickup->GetExperienceValue();
		Snapshot.HealValue = Pickup->GetHealValue();
		Snapshot.LifetimeRemaining = Pickup->GetLifetimeRemaining();
		Snapshot.GrantedItemID = Pickup->GetGrantedItemID();
	}

	for (const AT66MiniInteractable* Interactable : LiveInteractables)
	{
		if (!Interactable)
		{
			continue;
		}

		FT66MiniInteractableSnapshot& Snapshot = RunSave->InteractableSnapshots.AddDefaulted_GetRef();
		Snapshot.Location = Interactable->GetActorLocation();
		Snapshot.InteractableType = static_cast<uint8>(Interactable->GetInteractableType());
		Snapshot.VisualID = Interactable->GetVisualID();
		Snapshot.LifetimeRemaining = Interactable->GetLifetimeRemaining();
	}

	for (const AT66MiniHazardTrap* Trap : LiveTraps)
	{
		if (!Trap)
		{
			continue;
		}

		FT66MiniTrapSnapshot& Snapshot = RunSave->TrapSnapshots.AddDefaulted_GetRef();
		Snapshot.Location = Trap->GetActorLocation();
		Snapshot.Radius = Trap->GetRadius();
		Snapshot.DamagePerPulse = Trap->GetDamagePerPulse();
		Snapshot.PulseInterval = Trap->GetPulseInterval();
		Snapshot.WarmupRemaining = Trap->GetWarmupRemaining();
		Snapshot.ActiveRemaining = Trap->GetActiveRemaining();
		Snapshot.LifetimeRemaining = Trap->GetLifetimeRemaining();
		Snapshot.TrapVariant = Trap->GetTrapVariant();
	}
}

void AT66MiniGameMode::PersistActiveRunSnapshot(const bool bMarkMidWaveSnapshot)
{
	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!RunState || !SaveSubsystem || !ActiveRun || RunState->GetActiveSaveSlot() == INDEX_NONE)
	{
		return;
	}

	if (AT66MiniPlayerPawn* PlayerPawn = T66MiniChooseAnchorPawn(GetWorld()))
	{
		ActiveRun->PlayerLocation = PlayerPawn->GetActorLocation();
		ActiveRun->bHasPlayerLocation = bMarkMidWaveSnapshot;
		ActiveRun->CurrentHealth = PlayerPawn->GetCurrentHealth();
		ActiveRun->MaxHealth = PlayerPawn->GetMaxHealth();
		ActiveRun->Materials = PlayerPawn->GetMaterials();
		ActiveRun->Gold = PlayerPawn->GetGold();
		ActiveRun->Experience = PlayerPawn->GetExperience();
		ActiveRun->HeroLevel = PlayerPawn->GetHeroLevel();
		ActiveRun->UltimateCooldownRemaining = PlayerPawn->GetUltimateCooldownRemaining();
		ActiveRun->bEnduranceCheatUsedThisWave = PlayerPawn->HasEnduranceCheatUsedThisWave();
		ActiveRun->bQuickReviveReady = PlayerPawn->HasQuickReviveReady();
	}

	ActiveRun->bHasMidWaveSnapshot = bMarkMidWaveSnapshot;
	ActiveRun->BossTelegraphRemaining = FMath::Max(0.f, BossTelegraphRemaining);
	ActiveRun->PendingBossID = PendingBossID;
	ActiveRun->PendingBossSpawnLocation = PendingBossSpawnLocation;
	ActiveRun->TrapSpawnAccumulator = TrapSpawnAccumulator;
	CaptureWorldState(ActiveRun);
	SaveSubsystem->SaveRunToSlot(RunState->GetActiveSaveSlot(), ActiveRun);
}

void AT66MiniGameMode::UpdateLiveEnemyCache()
{
	for (int32 Index = LiveEnemies.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValid(LiveEnemies[Index]) || LiveEnemies[Index]->IsEnemyDead())
		{
			LiveEnemies.RemoveAtSwap(Index);
		}
	}
}

void AT66MiniGameMode::UpdateLivePlayerPawnCache()
{
	LivePlayerPawns.Reset();

	if (UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PlayerController = It->Get();
			AT66MiniPlayerPawn* PlayerPawn = PlayerController ? Cast<AT66MiniPlayerPawn>(PlayerController->GetPawn()) : nullptr;
			if (PlayerPawn)
			{
				LivePlayerPawns.AddUnique(PlayerPawn);
			}
		}
	}
}

void AT66MiniGameMode::UpdateLiveTrapCache()
{
	for (int32 Index = LiveTraps.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValid(LiveTraps[Index]))
		{
			LiveTraps.RemoveAtSwap(Index);
		}
	}
}

void AT66MiniGameMode::UpdateLiveInteractableCache()
{
	for (int32 Index = LiveInteractables.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValid(LiveInteractables[Index]))
		{
			LiveInteractables.RemoveAtSwap(Index);
		}
	}
}

void AT66MiniGameMode::UpdateLivePickupCache()
{
	for (int32 Index = LivePickups.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValid(LivePickups[Index]))
		{
			LivePickups.RemoveAtSwap(Index);
		}
	}
}

void AT66MiniGameMode::UpdateCombatTexts(const float DeltaSeconds)
{
	for (int32 Index = CombatTexts.Num() - 1; Index >= 0; --Index)
	{
		FT66MiniCombatTextEntry& Entry = CombatTexts[Index];
		Entry.RemainingTime -= DeltaSeconds;
		Entry.WorldLocation += Entry.Velocity * DeltaSeconds;
		Entry.Velocity *= 0.94f;
		if (Entry.RemainingTime <= 0.f)
		{
			CombatTexts.RemoveAtSwap(Index);
		}
	}
}

void AT66MiniGameMode::AddCombatText(const FVector& WorldLocation, const float Value, const FLinearColor& Color, const float Duration, const FString& Prefix)
{
	if (Value <= 0.f)
	{
		return;
	}

	FT66MiniCombatTextEntry& Entry = CombatTexts.AddDefaulted_GetRef();
	Entry.WorldLocation = WorldLocation;
	Entry.Label = Prefix.IsEmpty()
		? FString::Printf(TEXT("%.0f"), Value)
		: FString::Printf(TEXT("%s%.0f"), *Prefix, Value);
	Entry.Color = Color;
	Entry.RemainingTime = FMath::Max(0.2f, Duration);
	Entry.Velocity = FVector(FMath::FRandRange(-12.f, 12.f), FMath::FRandRange(-12.f, 12.f), FMath::FRandRange(120.f, 180.f));
}

bool AT66MiniGameMode::TryInteractNearest(AT66MiniPlayerPawn* PlayerPawn, const float MaxRange)
{
	if (!PlayerPawn)
	{
		return false;
	}

	AT66MiniInteractable* BestInteractable = nullptr;
	float BestDistanceSq = FMath::Square(MaxRange);
	for (AT66MiniInteractable* Candidate : LiveInteractables)
	{
		if (!Candidate || !Candidate->RequiresManualInteract())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(PlayerPawn->GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestInteractable = Candidate;
		}
	}

	if (BestInteractable)
	{
		return BestInteractable->TryInteract(PlayerPawn);
	}

	return false;
}
