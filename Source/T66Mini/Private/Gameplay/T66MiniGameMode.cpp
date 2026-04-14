// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniGameMode.h"

#include "Components/AudioComponent.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "EngineUtils.h"
#include "Gameplay/T66MiniArena.h"
#include "Gameplay/T66MiniCompanionBase.h"
#include "Gameplay/T66MiniEnemyBase.h"
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
}

AT66MiniGameMode::AT66MiniGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AT66MiniPlayerPawn::StaticClass();
	PlayerControllerClass = AT66MiniPlayerController::StaticClass();
	GameStateClass = AT66MiniGameState::StaticClass();
	HUDClass = AT66MiniBattleHUD::StaticClass();
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
	}

	if (USoundBase* BattleMusic = T66MiniLoadBattleMusic())
	{
		BattleMusicComponent = UGameplayStatics::SpawnSound2D(this, BattleMusic, 1.0f, 1.0f, 0.0f, nullptr, false);
		RefreshAudioMix();
	}

	SpawnArenaAndPositionPlayer();
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

	RefreshAudioMix();

	UGameInstance* GameInstance = GetGameInstance();
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!RunState || !DataSubsystem || !MiniGameState || !ActiveRun)
	{
		return;
	}

	TryApplySavedPawnState();
	UpdateLiveEnemyCache();

	if (!bRunCompleted)
	{
		if (PostBossDelayRemaining > 0.f)
		{
			PostBossDelayRemaining -= DeltaSeconds;
			if (PostBossDelayRemaining <= 0.f)
			{
				RecordClearedMiniStageProgression();
				if (MiniGameState->WaveIndex >= 5)
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

			float SpawnInterval = WaveDefinition ? WaveDefinition->SpawnInterval : 1.2f;
			if (DifficultyDefinition)
			{
				SpawnInterval = SpawnInterval / FMath::Max(0.65f, DifficultyDefinition->SpawnRateScalar);
			}
			SpawnInterval = FMath::Max(0.28f, SpawnInterval);

			while (EnemySpawnAccumulator >= SpawnInterval && MiniGameState->WaveSecondsRemaining > 0.f)
			{
				EnemySpawnAccumulator -= SpawnInterval;
				SpawnWaveEnemy();
			}

			const float InteractableInterval = WaveDefinition
				? WaveDefinition->InteractableInterval
				: (DifficultyDefinition ? DifficultyDefinition->InteractableInterval : 18.f);
			if (InteractableSpawnAccumulator >= InteractableInterval)
			{
				InteractableSpawnAccumulator = 0.f;
				SpawnRandomInteractable();
			}

			if (MiniGameState->WaveSecondsRemaining <= 0.f)
			{
				BeginBossSpawnTelegraph();
			}
		}
		else if (LiveEnemies.Num() == 0 && PostBossDelayRemaining <= 0.f)
		{
			PostBossDelayRemaining = 2.0f;
		}
	}

	ActiveRun->WaveIndex = MiniGameState->WaveIndex;
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
	ActiveRun->TotalRunSeconds += DeltaSeconds;

	if (AT66MiniPlayerPawn* PlayerPawn = GetWorld() ? Cast<AT66MiniPlayerPawn>(GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr) : nullptr)
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
	}

	AutosaveAccumulator += DeltaSeconds;
	if (AutosaveAccumulator >= 1.0f)
	{
		PersistActiveRunSnapshot(true);
		AutosaveAccumulator = 0.f;
	}
}

void AT66MiniGameMode::RefreshAudioMix()
{
	if (!BattleMusicComponent)
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const float EffectiveMusicVolume = 0.40f * T66MiniResolveMasterVolume(GameInstance) * T66MiniResolveMusicVolume(GameInstance);
	BattleMusicComponent->SetVolumeMultiplier(FMath::Clamp(EffectiveMusicVolume, 0.f, 1.f));
}

void AT66MiniGameMode::HandleEnemyKilled(AT66MiniEnemyBase* Enemy)
{
	if (Enemy)
	{
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			VfxSubsystem->SpawnPulse(GetWorld(), Enemy->GetActorLocation() + FVector(0.f, 0.f, 8.f), Enemy->IsBossEnemy() ? FVector(0.62f, 0.62f, 1.f) : FVector(0.24f, 0.24f, 1.f), Enemy->IsBossEnemy() ? 0.22f : 0.12f, Enemy->IsBossEnemy() ? FLinearColor(1.0f, 0.56f, 0.24f, 0.34f) : FLinearColor(0.98f, 0.44f, 0.22f, 0.18f), Enemy->IsBossEnemy() ? 0.95f : 0.55f);
		}
	}

	LiveEnemies.Remove(Enemy);
}

void AT66MiniGameMode::HandlePlayerDefeated()
{
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

	if (APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PlayerController->GetPawn())
		{
			PlayerPawn->SetActorLocation(ArenaOrigin + FVector(0.f, 0.f, 20.f));
		}
	}
}

void AT66MiniGameMode::StartWave(const int32 WaveIndex, const bool bResetTimer)
{
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	if (!MiniGameState)
	{
		return;
	}

	MiniGameState->WaveIndex = FMath::Clamp(WaveIndex, 1, 5);
	if (bResetTimer)
	{
		if (const FT66MiniWaveDefinition* WaveDefinition = GetWaveDefinition())
		{
			MiniGameState->WaveSecondsRemaining = WaveDefinition->DurationSeconds;
		}
		else
		{
			MiniGameState->WaveSecondsRemaining = 180.f;
		}
	}

	EnemySpawnAccumulator = 0.f;
	InteractableSpawnAccumulator = 0.f;
	PostBossDelayRemaining = 0.f;
	BossTelegraphRemaining = 0.f;
	PendingBossID = NAME_None;
	PendingBossSpawnLocation = FVector::ZeroVector;
	bBossSpawnedForWave = false;
}

const FT66MiniWaveDefinition* AT66MiniGameMode::GetWaveDefinition() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	const AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	if (!DataSubsystem || !MiniGameState)
	{
		return nullptr;
	}

	return DataSubsystem->FindWave(MiniGameState->DifficultyID, MiniGameState->WaveIndex);
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
	AT66MiniPlayerPawn* PlayerPawn = World ? Cast<AT66MiniPlayerPawn>(World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr) : nullptr;
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
	const float ProgressScalar = 1.0f + ((MiniGameState->WaveIndex - 1) * 0.16f);

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
			EnemyDefinition->BehaviorProfile);
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
	AT66MiniPlayerPawn* PlayerPawn = World ? Cast<AT66MiniPlayerPawn>(World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr) : nullptr;
	if (!World || !DataSubsystem || !WaveDefinition || !PlayerPawn)
	{
		SpawnBossEnemy();
		bBossSpawnedForWave = true;
		return;
	}

	PendingBossID = WaveDefinition->BossID;
	PendingBossSpawnLocation = ClampPointToArena(PlayerPawn->GetActorLocation() + FVector(ArenaHalfExtent * 0.72f, 0.f, 0.f));

	const FT66MiniBossDefinition* BossDefinition = DataSubsystem->FindBoss(PendingBossID);
	BossTelegraphRemaining = BossDefinition ? BossDefinition->TelegraphSeconds : 1.2f;
	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->Destroy();
		ActiveBossTelegraphActor = nullptr;
	}
	if (UT66MiniVFXSubsystem* VfxSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		ActiveBossTelegraphActor = VfxSubsystem->SpawnGroundTelegraph(World, PendingBossSpawnLocation, 260.f, BossTelegraphRemaining, FLinearColor(0.98f, 0.26f, 0.20f, 0.38f));
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

	const FName BossID = PendingBossID.IsNone() ? (GetWaveDefinition() ? GetWaveDefinition()->BossID : NAME_None) : PendingBossID;
	const FT66MiniBossDefinition* BossDefinition = DataSubsystem->FindBoss(BossID);
	if (!BossDefinition)
	{
		return;
	}

	const FVector SpawnLocation = PendingBossSpawnLocation.IsNearlyZero()
		? ClampPointToArena(ArenaOrigin + FVector(ArenaHalfExtent * 0.7f, 0.f, 0.f))
		: PendingBossSpawnLocation;
	const float DifficultyScalar = DifficultyDefinition ? DifficultyDefinition->BossScalar : 1.0f;
	const float WaveScalar = 1.0f + (MiniGameState->WaveIndex * 0.14f);

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
			BossDefinition->BehaviorProfile);
		LiveEnemies.Add(Boss);
	}

	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->Destroy();
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
			PickedDefinition->HealAmount);
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
	ActiveRun->PostBossDelayRemaining = 0.f;
	ActiveRun->EnemySnapshots.Reset();
	ActiveRun->PickupSnapshots.Reset();
	ActiveRun->InteractableSnapshots.Reset();
	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->Destroy();
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
	const UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	if (!SaveSubsystem || !DataSubsystem || !ActiveRun)
	{
		return;
	}

	SaveSubsystem->RecordClearedMiniStage(ActiveRun->CompanionID, DataSubsystem);
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
	AT66MiniPlayerPawn* PlayerPawn = GetWorld() ? Cast<AT66MiniPlayerPawn>(GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr) : nullptr;
	if (!RunState || !FrontendState || !SaveSubsystem || !DataSubsystem || !MiniGameState || !T66GameInstance || !ActiveRun)
	{
		return;
	}

	FT66MiniRunSummary Summary;
	Summary.bHasSummary = true;
	Summary.bWasVictory = bWasVictory;
	Summary.HeroID = ActiveRun->HeroID;
	Summary.CompanionID = ActiveRun->CompanionID;
	Summary.DifficultyID = ActiveRun->DifficultyID;
	Summary.WaveReached = FMath::Max(1, MiniGameState->WaveIndex);
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
	if (RunState->GetActiveSaveSlot() != INDEX_NONE)
	{
		SaveSubsystem->DeleteRunFromSlot(RunState->GetActiveSaveSlot());
	}
	if (BattleMusicComponent)
	{
		BattleMusicComponent->Stop();
	}
	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->Destroy();
		ActiveBossTelegraphActor = nullptr;
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
	if (!ActiveRun || !ActiveRun->bHasMidWaveSnapshot || !ActiveRun->bHasPlayerLocation)
	{
		bAppliedSavedPawnState = true;
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!PlayerPawn)
	{
		return;
	}

	PlayerPawn->SetActorLocation(ClampPointToArena(ActiveRun->PlayerLocation));
	bAppliedSavedPawnState = true;
}

void AT66MiniGameMode::SpawnCompanionActor()
{
	if (ActiveCompanionActor)
	{
		ActiveCompanionActor->Destroy();
		ActiveCompanionActor = nullptr;
	}

	UWorld* World = GetWorld();
	UGameInstance* GameInstance = GetGameInstance();
	AT66MiniPlayerPawn* PlayerPawn = World ? Cast<AT66MiniPlayerPawn>(World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr) : nullptr;
	UT66MiniRunStateSubsystem* RunState = GameInstance ? GameInstance->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniDataSubsystem* DataSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	AT66MiniGameState* MiniGameState = GetGameState<AT66MiniGameState>();
	UT66MiniRunSaveGame* ActiveRun = RunState ? RunState->GetActiveRun() : nullptr;
	FName CompanionID = ActiveRun ? ActiveRun->CompanionID : NAME_None;
	if ((CompanionID == NAME_None || (SaveSubsystem && !SaveSubsystem->IsCompanionUnlocked(CompanionID, DataSubsystem))) && SaveSubsystem && DataSubsystem)
	{
		CompanionID = SaveSubsystem->GetFirstUnlockedCompanionID(DataSubsystem);
		if (ActiveRun)
		{
			ActiveRun->CompanionID = CompanionID;
		}
	}

	if (MiniGameState)
	{
		MiniGameState->CompanionID = CompanionID;
	}

	if (!World || !PlayerPawn || !DataSubsystem || CompanionID == NAME_None)
	{
		return;
	}

	const FT66MiniCompanionDefinition* CompanionDefinition = DataSubsystem->FindCompanion(CompanionID);
	if (!CompanionDefinition)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActiveCompanionActor = World->SpawnActor<AT66MiniCompanionBase>(
		AT66MiniCompanionBase::StaticClass(),
		ClampPointToArena(PlayerPawn->GetActorLocation() + FVector(CompanionDefinition->FollowOffsetX, CompanionDefinition->FollowOffsetY, 0.f)),
		FRotator::ZeroRotator,
		SpawnParams);
	if (ActiveCompanionActor)
	{
		ActiveCompanionActor->InitializeCompanion(*CompanionDefinition, PlayerPawn);
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
	PostBossDelayRemaining = FMath::Max(0.f, RunSave->PostBossDelayRemaining);

	if (ActiveBossTelegraphActor)
	{
		ActiveBossTelegraphActor->Destroy();
		ActiveBossTelegraphActor = nullptr;
	}

	if (BossTelegraphRemaining > 0.f && !PendingBossSpawnLocation.IsNearlyZero())
	{
		if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
		{
			ActiveBossTelegraphActor = VfxSubsystem->SpawnGroundTelegraph(GetWorld(), PendingBossSpawnLocation, 260.f, BossTelegraphRemaining, FLinearColor(0.98f, 0.26f, 0.20f, 0.38f));
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FT66MiniEnemySnapshot& Snapshot : RunSave->EnemySnapshots)
	{
		ET66MiniEnemyBehaviorProfile BehaviorProfile = ET66MiniEnemyBehaviorProfile::Balanced;
		FString VisualID = Snapshot.EnemyID.ToString();
		if (Snapshot.bIsBoss)
		{
			if (const FT66MiniBossDefinition* BossDefinition = DataSubsystem->FindBoss(Snapshot.EnemyID))
			{
				BehaviorProfile = BossDefinition->BehaviorProfile;
				VisualID = BossDefinition->VisualID;
			}
		}
		else if (const FT66MiniEnemyDefinition* EnemyDefinition = DataSubsystem->FindEnemy(Snapshot.EnemyID))
		{
			BehaviorProfile = EnemyDefinition->BehaviorProfile;
			VisualID = EnemyDefinition->VisualID;
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
				BehaviorProfile);
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
				Definition ? Definition->HealAmount : 0.f);
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

	for (TActorIterator<AT66MiniEnemyBase> It(World); It; ++It)
	{
		const AT66MiniEnemyBase* Enemy = *It;
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

	for (TActorIterator<AT66MiniPickup> It(World); It; ++It)
	{
		const AT66MiniPickup* Pickup = *It;
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

	for (TActorIterator<AT66MiniInteractable> It(World); It; ++It)
	{
		const AT66MiniInteractable* Interactable = *It;
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

	if (AT66MiniPlayerPawn* PlayerPawn = GetWorld() ? Cast<AT66MiniPlayerPawn>(GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr) : nullptr)
	{
		ActiveRun->PlayerLocation = PlayerPawn->GetActorLocation();
		ActiveRun->bHasPlayerLocation = bMarkMidWaveSnapshot;
		ActiveRun->CurrentHealth = PlayerPawn->GetCurrentHealth();
		ActiveRun->MaxHealth = PlayerPawn->GetMaxHealth();
		ActiveRun->Materials = PlayerPawn->GetMaterials();
		ActiveRun->Gold = PlayerPawn->GetGold();
		ActiveRun->Experience = PlayerPawn->GetExperience();
		ActiveRun->HeroLevel = PlayerPawn->GetHeroLevel();
	}

	ActiveRun->bHasMidWaveSnapshot = bMarkMidWaveSnapshot;
	ActiveRun->BossTelegraphRemaining = FMath::Max(0.f, BossTelegraphRemaining);
	ActiveRun->PendingBossID = PendingBossID;
	ActiveRun->PendingBossSpawnLocation = PendingBossSpawnLocation;
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
