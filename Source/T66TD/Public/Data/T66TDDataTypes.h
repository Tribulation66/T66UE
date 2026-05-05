// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T66TDDataTypes.generated.h"

USTRUCT(BlueprintType)
struct T66TD_API FT66TDHeroDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString RoleLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString Signature;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString UltimateLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString PassiveLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FLinearColor PlaceholderColor = FLinearColor(0.26f, 0.30f, 0.36f, 1.0f);
};

USTRUCT(BlueprintType)
struct T66TD_API FT66TDHeroCombatDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 Cost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float Damage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float Range = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float FireInterval = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 ChainBounces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float ChainRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float SplashRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float DotDamagePerSecond = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float DotDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float SlowMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float SlowDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float BossDamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float FlatArmorPierce = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float ShieldDamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	bool bCanTargetHidden = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	bool bPrioritizeBoss = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 VolleyShots = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 BonusMaterialsOnKill = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString CombatLabel;
};

USTRUCT(BlueprintType)
struct T66TD_API FT66TDEnemyArchetypeDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName EnemyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString VisualID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float BaseHealth = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float BaseSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 LeakDamage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 Bounty = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float Radius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FLinearColor Tint = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct T66TD_API FT66TDBattleTuningEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName TuningKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float Value = 0.f;
};

USTRUCT(BlueprintType)
struct T66TD_API FT66TDThemeModifierRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString ThemeLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString BossVisualID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 BossModifierMask = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 HiddenMinWave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 ShieldMinWave = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	bool bEnableRegen = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	bool bShieldGoat = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float ArmorChanceBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float HiddenChanceBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float RegenChanceBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float ShieldChanceBonus = 0.f;
};

USTRUCT(BlueprintType)
struct T66TD_API FT66TDDifficultyDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FLinearColor AccentColor = FLinearColor(0.32f, 0.44f, 0.68f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 MapCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float EnemyHealthScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float EnemySpeedScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float BossScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	float RewardScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 StageClearChadCoupons = 0;
};

USTRUCT(BlueprintType)
struct T66TD_API FT66TDMapDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName MapID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString ThemeLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString BackgroundRelativePath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 LaneCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 BuildPadCount = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 BossWave = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString EnemyNotes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	TArray<FName> FeaturedHeroIDs;
};

USTRUCT(BlueprintType)
struct T66TD_API FT66TDStageDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName StageID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName DifficultyID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 StageIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName MapID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 BossWave = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 StartingGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 StartingMaterials = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 ClearGoldReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 ClearMaterialReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	int32 ClearChadCoupons = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TD")
	FName NextStageID = NAME_None;
};

struct T66TD_API FT66TDMapPadDefinition
{
	FVector2D PositionNormalized = FVector2D::ZeroVector;
	float RadiusNormalized = 0.01f;
};

struct T66TD_API FT66TDMapLayoutDefinition
{
	FName MapID = NAME_None;
	TArray<TArray<FVector2D>> Paths;
	TArray<FT66TDMapPadDefinition> Pads;
};
