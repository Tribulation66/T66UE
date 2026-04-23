// Copyright Tribulation 66. All Rights Reserved.

#include "Core/Backend/T66BackendDailyClimbJson.h"
#include "Dom/JsonObject.h"

namespace
{
	ET66DailyClimbRuleType T66ParseDailyClimbRuleType(const FString& Type)
	{
		if (Type.Equals(TEXT("start_random_items"), ESearchCase::IgnoreCase))
		{
			return ET66DailyClimbRuleType::StartRandomItems;
		}
		if (Type.Equals(TEXT("start_bonus_gold"), ESearchCase::IgnoreCase))
		{
			return ET66DailyClimbRuleType::StartBonusGold;
		}
		if (Type.Equals(TEXT("enemy_hp_multiplier"), ESearchCase::IgnoreCase))
		{
			return ET66DailyClimbRuleType::EnemyHpMultiplier;
		}
		if (Type.Equals(TEXT("enemy_loot_bag_count_multiplier"), ESearchCase::IgnoreCase))
		{
			return ET66DailyClimbRuleType::EnemyLootBagCountMultiplier;
		}

		return ET66DailyClimbRuleType::Unknown;
	}

	ET66Difficulty T66ParseDailyClimbDifficulty(const FString& Difficulty)
	{
		if (Difficulty.Equals(TEXT("medium"), ESearchCase::IgnoreCase)) return ET66Difficulty::Medium;
		if (Difficulty.Equals(TEXT("hard"), ESearchCase::IgnoreCase)) return ET66Difficulty::Hard;
		if (Difficulty.Equals(TEXT("veryhard"), ESearchCase::IgnoreCase)) return ET66Difficulty::VeryHard;
		if (Difficulty.Equals(TEXT("impossible"), ESearchCase::IgnoreCase)) return ET66Difficulty::Impossible;
		return ET66Difficulty::Easy;
	}
}

bool T66BackendDailyClimbJson::ParseChallengeData(const TSharedPtr<FJsonObject>& Json, FT66DailyClimbChallengeData& OutChallenge)
{
	if (!Json.IsValid())
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* ChallengeObjectPtr = nullptr;
	const TSharedPtr<FJsonObject> ChallengeJson =
		(Json->TryGetObjectField(TEXT("challenge"), ChallengeObjectPtr) && ChallengeObjectPtr && (*ChallengeObjectPtr).IsValid())
		? *ChallengeObjectPtr
		: Json;

	if (!ChallengeJson.IsValid())
	{
		return false;
	}

	FT66DailyClimbChallengeData ParsedChallenge;
	ChallengeJson->TryGetStringField(TEXT("challenge_id"), ParsedChallenge.ChallengeId);
	ChallengeJson->TryGetStringField(TEXT("challenge_date_utc"), ParsedChallenge.ChallengeDateUtc);
	ChallengeJson->TryGetStringField(TEXT("title"), ParsedChallenge.Title);
	ChallengeJson->TryGetStringField(TEXT("leaderboard_key"), ParsedChallenge.LeaderboardKey);
	ChallengeJson->TryGetStringField(TEXT("attempt_id"), ParsedChallenge.AttemptId);
	ChallengeJson->TryGetStringField(TEXT("attempt_state"), ParsedChallenge.AttemptState);

	FString HeroIdString;
	if (ChallengeJson->TryGetStringField(TEXT("hero_id"), HeroIdString) && !HeroIdString.IsEmpty())
	{
		ParsedChallenge.HeroID = FName(*HeroIdString);
	}

	FString DifficultyString;
	if (ChallengeJson->TryGetStringField(TEXT("difficulty"), DifficultyString))
	{
		ParsedChallenge.Difficulty = T66ParseDailyClimbDifficulty(DifficultyString);
	}

	double RunSeedValue = 0.0;
	if (ChallengeJson->TryGetNumberField(TEXT("run_seed"), RunSeedValue))
	{
		ParsedChallenge.RunSeed = static_cast<int32>(RunSeedValue);
	}

	double CouponRewardValue = 0.0;
	if (ChallengeJson->TryGetNumberField(TEXT("coupon_reward"), CouponRewardValue))
	{
		ParsedChallenge.CouponReward = static_cast<int32>(CouponRewardValue);
	}

	const TArray<TSharedPtr<FJsonValue>>* RulesArray = nullptr;
	if (ChallengeJson->TryGetArrayField(TEXT("rules"), RulesArray) && RulesArray)
	{
		for (const TSharedPtr<FJsonValue>& RuleValue : *RulesArray)
		{
			const TSharedPtr<FJsonObject>* RuleObject = nullptr;
			if (!RuleValue.IsValid() || !RuleValue->TryGetObject(RuleObject) || !RuleObject || !(*RuleObject).IsValid())
			{
				continue;
			}

			FT66DailyClimbRule& Rule = ParsedChallenge.Rules.AddDefaulted_GetRef();
			FString RuleTypeString;
			(*RuleObject)->TryGetStringField(TEXT("type"), RuleTypeString);
			Rule.Type = T66ParseDailyClimbRuleType(RuleTypeString);
			(*RuleObject)->TryGetStringField(TEXT("label"), Rule.Label);
			(*RuleObject)->TryGetStringField(TEXT("description"), Rule.Description);

			double IntValue = 0.0;
			if ((*RuleObject)->TryGetNumberField(TEXT("intValue"), IntValue))
			{
				Rule.IntValue = static_cast<int32>(IntValue);
			}

			double FloatValue = 0.0;
			if ((*RuleObject)->TryGetNumberField(TEXT("floatValue"), FloatValue))
			{
				Rule.FloatValue = static_cast<float>(FloatValue);
			}
		}
	}

	if (!ParsedChallenge.IsValid())
	{
		return false;
	}

	OutChallenge = MoveTemp(ParsedChallenge);
	return true;
}
