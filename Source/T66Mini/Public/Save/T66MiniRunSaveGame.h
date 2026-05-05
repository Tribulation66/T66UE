// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "T66MiniRunSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FT66MiniEnemySnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FName EnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	bool bIsBoss = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float CurrentHealth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float MaxHealth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float MoveSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float TouchDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 MaterialDrop = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float ExperienceDrop = 0.f;
};

USTRUCT(BlueprintType)
struct FT66MiniPickupSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FString VisualID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 MaterialValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float ExperienceValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float HealValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float LifetimeRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FName GrantedItemID = NAME_None;
};

USTRUCT(BlueprintType)
struct FT66MiniInteractableSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	uint8 InteractableType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FString VisualID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float LifetimeRemaining = 0.f;
};

USTRUCT(BlueprintType)
struct FT66MiniTrapSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float Radius = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float DamagePerPulse = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float PulseInterval = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float WarmupRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float ActiveRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float LifetimeRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 TrapVariant = 0;
};

UCLASS()
class T66MINI_API UT66MiniRunSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 SaveSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FName CompanionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FName> EquippedIdolIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FName> OwnedItemIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FName> CurrentShopOfferIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FName> LockedShopOfferIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 WaveIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FName CurrentStageID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 StageIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 HeroLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 Materials = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float Experience = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float UltimateCooldownRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	bool bEnduranceCheatUsedThisWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	bool bQuickReviveReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float TotalRunSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 ShopRerollCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	bool bHasMidWaveSnapshot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	bool bPendingShopIntermission = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	bool bHasPlayerLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float WaveSecondsRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	bool bBossSpawnedForWave = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float BossTelegraphRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FName PendingBossID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FVector PendingBossSpawnLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float EnemySpawnAccumulator = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float InteractableSpawnAccumulator = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float PostBossDelayRemaining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float TrapSpawnAccumulator = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FT66MiniEnemySnapshot> EnemySnapshots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FT66MiniPickupSnapshot> PickupSnapshots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FT66MiniInteractableSnapshot> InteractableSnapshots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FT66MiniTrapSnapshot> TrapSnapshots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 CircusDebt = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	float CircusAnger01 = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	TArray<FName> CircusBuybackItemIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	int32 CircusVendorRerollCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mini")
	FString LastUpdatedUtc;
};
