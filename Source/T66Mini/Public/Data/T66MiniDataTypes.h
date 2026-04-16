// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "T66MiniDataTypes.generated.h"

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniHeroDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString PrimaryCategory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FLinearColor PlaceholderColor = FLinearColor(0.24f, 0.28f, 0.36f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString PortraitPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString PortraitFullPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseAttackSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseArmor = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseLuck = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	ET66UltimateType UltimateType = ET66UltimateType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	ET66PassiveType PassiveType = ET66PassiveType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	bool bUnlockedByDefault = true;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniIdolDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName IdolID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString IconPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 MaxLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float DamagePerLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseProperty = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float PropertyPerLevel = 0.f;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniCompanionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName CompanionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString VisualID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FLinearColor PlaceholderColor = FLinearColor(0.48f, 0.38f, 0.22f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseHealingPerSecond = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 UnlockStageRequirement = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float FollowOffsetX = -145.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float FollowOffsetY = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	bool bUnlockedByDefault = true;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniDifficultyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FLinearColor AccentColor = FLinearColor(0.34f, 0.44f, 0.60f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float HealthScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float DamageScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float SpeedScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float SpawnRateScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BossScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float InteractableInterval = 18.0f;
};

UENUM(BlueprintType)
enum class ET66MiniEnemyBehaviorProfile : uint8
{
	Balanced UMETA(DisplayName = "Balanced"),
	HumanoidBalanced UMETA(DisplayName = "Humanoid Balanced"),
	RoostPress UMETA(DisplayName = "Roost Press"),
	CowBruiser UMETA(DisplayName = "Cow Bruiser"),
	GoatCharger UMETA(DisplayName = "Goat Charger"),
	PigEnrage UMETA(DisplayName = "Pig Enrage"),
	Sharpshooter UMETA(DisplayName = "Sharpshooter"),
	Juggernaut UMETA(DisplayName = "Juggernaut"),
	Duelist UMETA(DisplayName = "Duelist")
};

UENUM(BlueprintType)
enum class ET66MiniEnemyFamily : uint8
{
	Melee UMETA(DisplayName = "Melee"),
	Ranged UMETA(DisplayName = "Ranged"),
	Rushing UMETA(DisplayName = "Rushing"),
	Boss UMETA(DisplayName = "Boss")
};

UENUM(BlueprintType)
enum class ET66MiniInteractableType : uint8
{
	TreasureChest UMETA(DisplayName = "Treasure Chest"),
	Fountain UMETA(DisplayName = "Fountain"),
	LootCrate UMETA(DisplayName = "Loot Crate"),
	QuickReviveMachine UMETA(DisplayName = "Quick Revive Machine")
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniEnemyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName EnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString VisualID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	ET66MiniEnemyBehaviorProfile BehaviorProfile = ET66MiniEnemyBehaviorProfile::Balanced;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	ET66MiniEnemyFamily Family = ET66MiniEnemyFamily::Melee;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseHealth = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseTouchDamage = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 BaseMaterials = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float BaseExperience = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float FireIntervalSeconds = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float ProjectileSpeed = 920.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float ProjectileDamage = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float PreferredRange = 860.0f;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniBossDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName BossID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString VisualID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	ET66MiniEnemyBehaviorProfile BehaviorProfile = ET66MiniEnemyBehaviorProfile::Balanced;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	ET66MiniEnemyFamily Family = ET66MiniEnemyFamily::Boss;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float MaxHealth = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float MoveSpeed = 320.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float TouchDamage = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 MaterialReward = 24;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float ExperienceReward = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float TelegraphSeconds = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float FireIntervalSeconds = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float ProjectileSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float ProjectileDamage = 18.0f;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniWaveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 WaveIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float DurationSeconds = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName BossID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	TArray<FName> EnemyIDs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float SpawnInterval = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float InteractableInterval = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float EnemyHealthScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float EnemyDamageScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float EnemySpeedScalar = 1.0f;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniInteractableDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName InteractableID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	ET66MiniInteractableType Type = ET66MiniInteractableType::TreasureChest;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString VisualID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float LifetimeSeconds = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float SpawnWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 MaterialReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 GoldReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float ExperienceReward = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float HealAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	bool bRequiresManualInteract = false;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniItemDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString IconPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString PrimaryStatType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString SecondaryStatType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 BaseBuyGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 BaseSellGold = 0;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniSaveSlotSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString HeroDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString DifficultyDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 WaveIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	bool bHasMidWaveSnapshot = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString LastUpdatedUtc;
};

USTRUCT(BlueprintType)
struct T66MINI_API FT66MiniRunSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	bool bHasSummary = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	bool bWasVictory = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString HeroDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName CompanionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString CompanionDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString DifficultyDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 WaveReached = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 MaterialsCollected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 OwnedItemCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	int32 EquippedIdolCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	float RunSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString ResultLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mini")
	FString LastUpdatedUtc;
};
