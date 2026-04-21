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
