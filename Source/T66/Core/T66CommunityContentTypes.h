// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "T66CommunityContentTypes.generated.h"

UENUM(BlueprintType)
enum class ET66CommunityContentKind : uint8
{
	Challenge UMETA(DisplayName = "Challenge"),
	Mod UMETA(DisplayName = "Mod"),
};

UENUM(BlueprintType)
enum class ET66CommunityContentOrigin : uint8
{
	Official UMETA(DisplayName = "Official"),
	Community UMETA(DisplayName = "Community"),
	Draft UMETA(DisplayName = "Draft"),
};

namespace T66CommunityContentLimits
{
	static constexpr int32 MinStatBonus = -99;
	static constexpr int32 MaxStatBonus = 99;
	static constexpr int32 MaxStartLevelOverride = 99;
	static constexpr int32 MaxRequiredStageReached = 20;
	static constexpr int32 MaxRunTimeSeconds = 3600;
	static constexpr int32 MaxRewardChadCoupons = 9999;

	inline int32 ClampStatBonus(const int32 Value)
	{
		return FMath::Clamp(Value, MinStatBonus, MaxStatBonus);
	}

	inline int32 ClampStartLevelOverride(const int32 Value)
	{
		return FMath::Clamp(Value, 0, MaxStartLevelOverride);
	}

	inline int32 ClampRequiredStageReached(const int32 Value)
	{
		return FMath::Clamp(Value, 0, MaxRequiredStageReached);
	}

	inline int32 ClampRunTimeSeconds(const int32 Value)
	{
		return FMath::Clamp(Value, 0, MaxRunTimeSeconds);
	}

	inline int32 ClampRewardChadCoupons(const int32 Value)
	{
		return FMath::Clamp(Value, 0, MaxRewardChadCoupons);
	}
}

USTRUCT(BlueprintType)
struct T66_API FT66CommunityStatBonusBlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 AttackSpeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 AttackScale = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 Accuracy = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 Armor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 Evasion = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 Luck = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 Speed = 0;

	bool HasAnyBonus() const
	{
		return Damage != 0
			|| AttackSpeed != 0
			|| AttackScale != 0
			|| Accuracy != 0
			|| Armor != 0
			|| Evasion != 0
			|| Luck != 0
			|| Speed != 0;
	}

	void Sanitize()
	{
		Damage = T66CommunityContentLimits::ClampStatBonus(Damage);
		AttackSpeed = T66CommunityContentLimits::ClampStatBonus(AttackSpeed);
		AttackScale = T66CommunityContentLimits::ClampStatBonus(AttackScale);
		Accuracy = T66CommunityContentLimits::ClampStatBonus(Accuracy);
		Armor = T66CommunityContentLimits::ClampStatBonus(Armor);
		Evasion = T66CommunityContentLimits::ClampStatBonus(Evasion);
		Luck = T66CommunityContentLimits::ClampStatBonus(Luck);
		Speed = T66CommunityContentLimits::ClampStatBonus(Speed);
	}
};

USTRUCT(BlueprintType)
struct T66_API FT66CommunityRuleSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	// Deprecated: run-start hero levels no longer affect stats.
	int32 StartLevelOverride = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	// Deprecated: max hero stats no longer apply.
	bool bSetMaxHeroStats = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	// Deprecated: direct stat bonuses no longer apply; start items are the supported stat path.
	FT66CommunityStatBonusBlock BonusStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FName StartingItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	int32 StartRandomItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	int32 StartBonusGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	float EnemyHealthMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	float EnemyDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	float TrapDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	// Deprecated: hero health modifiers no longer apply; health changes should come from items.
	float HeroHealthMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	// Deprecated: hero damage modifiers no longer apply; damage changes should come from items.
	float HeroDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	// Deprecated: hero luck modifiers no longer apply; luck changes should come from items.
	int32 HeroLuckFlat = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community|Modifiers")
	float EnemyLootBagCountMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	ET66PassiveType PassiveOverride = ET66PassiveType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	ET66UltimateType UltimateOverride = ET66UltimateType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	bool bRequireFullClear = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	bool bRequireNoDamage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 RequiredStageReached = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 MaxRunTimeSeconds = 0;

	bool HasGameplayChanges() const
	{
		return !StartingItemId.IsNone()
			|| StartRandomItems > 0
			|| StartBonusGold > 0
			|| !FMath::IsNearlyEqual(EnemyHealthMultiplier, 1.0f)
			|| !FMath::IsNearlyEqual(EnemyDamageMultiplier, 1.0f)
			|| !FMath::IsNearlyEqual(TrapDamageMultiplier, 1.0f)
			|| !FMath::IsNearlyEqual(EnemyLootBagCountMultiplier, 1.0f)
			|| PassiveOverride != ET66PassiveType::None
			|| UltimateOverride != ET66UltimateType::None;
	}

	void Sanitize()
	{
		StartLevelOverride = T66CommunityContentLimits::ClampStartLevelOverride(StartLevelOverride);
		BonusStats.Sanitize();
		StartRandomItems = FMath::Clamp(StartRandomItems, 0, 20);
		StartBonusGold = FMath::Clamp(StartBonusGold, 0, 10000);
		EnemyHealthMultiplier = FMath::Clamp(EnemyHealthMultiplier, 0.1f, 10.0f);
		EnemyDamageMultiplier = FMath::Clamp(EnemyDamageMultiplier, 0.1f, 10.0f);
		TrapDamageMultiplier = FMath::Clamp(TrapDamageMultiplier, 0.1f, 10.0f);
		HeroHealthMultiplier = FMath::Clamp(HeroHealthMultiplier, 0.1f, 10.0f);
		HeroDamageMultiplier = FMath::Clamp(HeroDamageMultiplier, 0.1f, 10.0f);
		HeroLuckFlat = FMath::Clamp(HeroLuckFlat, -99, 99);
		EnemyLootBagCountMultiplier = FMath::Clamp(EnemyLootBagCountMultiplier, 0.0f, 20.0f);
		RequiredStageReached = T66CommunityContentLimits::ClampRequiredStageReached(RequiredStageReached);
		MaxRunTimeSeconds = T66CommunityContentLimits::ClampRunTimeSeconds(MaxRunTimeSeconds);
	}
};

USTRUCT(BlueprintType)
struct T66_API FT66CommunityContentEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FName LocalId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FString BackendId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	ET66CommunityContentKind Kind = ET66CommunityContentKind::Mod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	ET66CommunityContentOrigin Origin = ET66CommunityContentOrigin::Official;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FString AuthorDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FString AuthorAvatarUrl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FString ModerationStatus = TEXT("local");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 SuggestedRewardChadCoupons = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	int32 ApprovedRewardChadCoupons = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FString SubmissionStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FString ReviewNote;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FT66CommunityRuleSet Rules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	bool bLocallyAuthored = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Community")
	FDateTime UpdatedAt;

	bool IsDraft() const
	{
		return Origin == ET66CommunityContentOrigin::Draft;
	}

	bool IsCommunityApproved() const
	{
		return Origin == ET66CommunityContentOrigin::Community && ModerationStatus.Equals(TEXT("approved"), ESearchCase::IgnoreCase);
	}

	bool HasApprovedReward() const
	{
		if (Kind != ET66CommunityContentKind::Challenge || ApprovedRewardChadCoupons <= 0)
		{
			return false;
		}

		return Origin == ET66CommunityContentOrigin::Official || IsCommunityApproved();
	}

	void Sanitize()
	{
		Title = Title.TrimStartAndEnd();
		if (Title.IsEmpty())
		{
			Title = Kind == ET66CommunityContentKind::Mod ? TEXT("Untitled Mod") : TEXT("Untitled Challenge");
		}

		Description = Description.TrimStartAndEnd();
		if (Description.IsEmpty())
		{
			Description = TEXT("Describe what this ruleset changes.");
		}

		SuggestedRewardChadCoupons = T66CommunityContentLimits::ClampRewardChadCoupons(SuggestedRewardChadCoupons);
		ApprovedRewardChadCoupons = T66CommunityContentLimits::ClampRewardChadCoupons(ApprovedRewardChadCoupons);
		Rules.Sanitize();

		if (Origin == ET66CommunityContentOrigin::Draft && SubmissionStatus.IsEmpty())
		{
			SubmissionStatus = TEXT("Not submitted");
		}
	}
};
