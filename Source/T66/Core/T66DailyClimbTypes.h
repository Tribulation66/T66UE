// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "T66DailyClimbTypes.generated.h"

UENUM(BlueprintType)
enum class ET66DailyClimbRuleType : uint8
{
	Unknown UMETA(DisplayName = "Unknown"),
	StartRandomItems UMETA(DisplayName = "Start Random Items"),
	StartBonusGold UMETA(DisplayName = "Start Bonus Gold"),
	EnemyHpMultiplier UMETA(DisplayName = "Enemy HP Multiplier"),
	EnemyLootBagCountMultiplier UMETA(DisplayName = "Enemy Loot Bag Count Multiplier")
};

USTRUCT(BlueprintType)
struct T66_API FT66DailyClimbRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	ET66DailyClimbRuleType Type = ET66DailyClimbRuleType::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FString Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	float FloatValue = 0.0f;
};

USTRUCT(BlueprintType)
struct T66_API FT66DailyClimbChallengeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FString ChallengeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FString ChallengeDateUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FName HeroID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	ET66Difficulty Difficulty = ET66Difficulty::Easy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	int32 RunSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	int32 CouponReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FString LeaderboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FString AttemptId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	FString AttemptState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Climb")
	TArray<FT66DailyClimbRule> Rules;

	bool IsValid() const
	{
		return !ChallengeId.IsEmpty() && !HeroID.IsNone();
	}

	bool HasStartedAttempt() const
	{
		return AttemptState.Equals(TEXT("started"), ESearchCase::IgnoreCase)
			|| AttemptState.Equals(TEXT("submitting"), ESearchCase::IgnoreCase)
			|| AttemptState.Equals(TEXT("completed"), ESearchCase::IgnoreCase);
	}

	bool HasCompletedAttempt() const
	{
		return AttemptState.Equals(TEXT("completed"), ESearchCase::IgnoreCase);
	}

	int32 FindIntRuleValue(const ET66DailyClimbRuleType RuleType, const int32 DefaultValue = 0) const
	{
		for (const FT66DailyClimbRule& Rule : Rules)
		{
			if (Rule.Type == RuleType)
			{
				return Rule.IntValue;
			}
		}

		return DefaultValue;
	}

	float FindFloatRuleValue(const ET66DailyClimbRuleType RuleType, const float DefaultValue = 0.0f) const
	{
		for (const FT66DailyClimbRule& Rule : Rules)
		{
			if (Rule.Type == RuleType)
			{
				return Rule.FloatValue;
			}
		}

		return DefaultValue;
	}
};
