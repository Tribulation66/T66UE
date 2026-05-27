// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66CommunityContentTypes.h"
#include "Core/T66DailyClimbTypes.h"

struct T66_API FT66RunModifierSnapshot
{
	int32 StartRandomItems = 0;
	int32 StartBonusGold = 0;
	float EnemyHealthMultiplier = 1.0f;
	float EnemyDamageMultiplier = 1.0f;
	float TrapDamageMultiplier = 1.0f;
	// Deprecated compatibility fields. Hero stat modifiers no longer apply at runtime.
	float HeroHealthMultiplier = 1.0f;
	float HeroDamageMultiplier = 1.0f;
	int32 HeroLuckFlat = 0;
	float EnemyLootBagCountMultiplier = 1.0f;

	void Sanitize();
	void Merge(const FT66RunModifierSnapshot& Other);
	bool HasAnyGameplayModifier() const;
};

struct T66_API FT66RunModifierCatalog
{
	static FT66RunModifierSnapshot FromDailyChallenge(const FT66DailyClimbChallengeData& Challenge);
	static FT66RunModifierSnapshot FromCommunityRules(const FT66CommunityRuleSet& Rules);
};
