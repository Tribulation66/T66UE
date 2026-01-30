// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66Rarity.h"

ET66Rarity FT66RarityUtil::RollDefaultRarity(FRandomStream& Rng)
{
	// Probabilities (sum=1.0):
	// - Black: 0.70
	// - Red:   0.20
	// - Yellow:0.09
	// - White: 0.01
	const float Roll = Rng.FRand();
	if (Roll < 0.70f) return ET66Rarity::Black;
	if (Roll < 0.90f) return ET66Rarity::Red;
	if (Roll < 0.99f) return ET66Rarity::Yellow;
	return ET66Rarity::White;
}

FLinearColor FT66RarityUtil::GetRarityColor(ET66Rarity R)
{
	switch (R)
	{
	case ET66Rarity::Black:  return FLinearColor(0.10f, 0.10f, 0.12f, 1.f);
	case ET66Rarity::Red:    return FLinearColor(0.90f, 0.20f, 0.20f, 1.f);
	case ET66Rarity::Yellow: return FLinearColor(0.95f, 0.80f, 0.15f, 1.f);
	case ET66Rarity::White:  return FLinearColor(0.92f, 0.92f, 0.96f, 1.f);
	default:                 return FLinearColor::White;
	}
}

FLinearColor FT66RarityUtil::GetTierColor(int32 TierIndex)
{
	switch (FMath::Max(0, TierIndex))
	{
	case 0: return FLinearColor(0.90f, 0.20f, 0.20f, 1.f); // red
	case 1: return FLinearColor(0.20f, 0.45f, 0.95f, 1.f); // blue
	case 2: return FLinearColor(0.20f, 0.85f, 0.35f, 1.f); // green
	case 3: return FLinearColor(0.65f, 0.25f, 0.90f, 1.f); // purple
	case 4: return FLinearColor(0.95f, 0.80f, 0.15f, 1.f); // gold
	default: return FLinearColor(0.20f, 0.85f, 0.85f, 1.f); // cyan fallback
	}
}

void FT66RarityUtil::ComputeTierAndCount5(int32 Total, int32& OutTierIndex, int32& OutCount1To5)
{
	if (Total <= 0)
	{
		OutTierIndex = 0;
		OutCount1To5 = 0;
		return;
	}
	OutTierIndex = (Total - 1) / 5;
	OutCount1To5 = ((Total - 1) % 5) + 1;
}

