// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Animation/T66LootUIAnimation.h"

namespace T66LootUIAnimation
{
	float SafeRangeAlpha(const float Value, const float Start, const float End)
	{
		const float Range = End - Start;
		return FMath::IsNearlyZero(Range) ? 1.f : FMath::Clamp((Value - Start) / Range, 0.f, 1.f);
	}

	float InvertWeightedDecelProgress(const float NormalizedValue)
	{
		// Inverse of ET66AnimationCurve::WeightedDecel: 1 - pow(1 - T, 4).
		const float Clamped = FMath::Clamp(NormalizedValue, 0.f, 1.f);
		return 1.f - FMath::Pow(1.f - Clamped, 0.25f);
	}

	FT66ScrollingStripMotionPlan BuildScrollingStripMotionPlan(const FT66ScrollingStripMotionInput& Input)
	{
		FT66ScrollingStripMotionPlan Plan;
		Plan.SelectorPositionX = FMath::Max(0.f, Input.ViewportWidth * 0.5f);
		Plan.MaxVisibleOffset = FMath::Max(0.f, Input.StripTotalWidth - Input.ViewportWidth);
		Plan.RightContentAfterWinner = FMath::Max(0.f, Input.StripTotalWidth - Input.WinnerCenterX);
		Plan.bHasRightContentForFinalView = Plan.RightContentAfterWinner >= Plan.SelectorPositionX;

		const float RawFinalOffset = FMath::Max(0.f, Input.WinnerCenterX - Plan.SelectorPositionX);
		Plan.FinalStripOffset = FMath::Clamp(RawFinalOffset, 0.f, Plan.MaxVisibleOffset);

		const float SafeStride = FMath::Max(1.f, Input.ItemStride);
		Plan.FastTravelEndOffset = FMath::Max(0.f, Plan.FinalStripOffset - (FMath::Max(0.f, Input.Tuning.FastTravelTileCount) * SafeStride));
		Plan.OvershootOffset = FMath::Clamp(
			Plan.FinalStripOffset + (FMath::Max(0.f, Input.Tuning.OvershootTileFraction) * SafeStride),
			0.f,
			Plan.MaxVisibleOffset);

		const int32 SlowdownTiles = FMath::Max(1, FMath::RoundToInt(Input.Tuning.SlowdownTickTileCount));
		Plan.SlowdownStartIndex = Input.WinnerIndex == INDEX_NONE ? INDEX_NONE : FMath::Max(0, Input.WinnerIndex - SlowdownTiles);
		return Plan;
	}

	float ComputeStripBoundaryMarkerProgress(
		const float BoundaryOffset,
		const float StartOffset,
		const float EndOffset,
		const bool bWeightedDecel)
	{
		float MarkerProgress = SafeRangeAlpha(BoundaryOffset, StartOffset, EndOffset);
		if (bWeightedDecel)
		{
			MarkerProgress = InvertWeightedDecelProgress(MarkerProgress);
		}
		return FMath::Clamp(MarkerProgress, 0.01f, 0.99f);
	}

	float ComputeSkipStartOffset(
		const float CurrentOffset,
		const float FinalOffset,
		const float ItemStride,
		const float SkipTravelTileCount)
	{
		const float SafeStride = FMath::Max(1.f, ItemStride);
		const float TravelDistance = FMath::Max(0.f, SkipTravelTileCount) * SafeStride;
		return FMath::Clamp(FMath::Max(CurrentOffset, FinalOffset - TravelDistance), 0.f, FinalOffset);
	}
}
