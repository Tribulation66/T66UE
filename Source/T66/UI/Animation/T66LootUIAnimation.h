// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FT66ScrollingStripMotionTuning
{
	float FastTravelTileCount = 12.f;
	float OvershootTileFraction = 0.18f;
	float SlowdownTickTileCount = 6.f;
	float SkipTravelTileCount = 3.f;
};

struct FT66ScrollingStripMotionInput
{
	float ViewportWidth = 0.f;
	float StripTotalWidth = 0.f;
	float WinnerCenterX = 0.f;
	float ItemStride = 1.f;
	int32 WinnerIndex = INDEX_NONE;
	FT66ScrollingStripMotionTuning Tuning;
};

struct FT66ScrollingStripMotionPlan
{
	float SelectorPositionX = 0.f;
	float MaxVisibleOffset = 0.f;
	float FinalStripOffset = 0.f;
	float FastTravelEndOffset = 0.f;
	float OvershootOffset = 0.f;
	float RightContentAfterWinner = 0.f;
	int32 SlowdownStartIndex = INDEX_NONE;
	bool bHasRightContentForFinalView = false;
};

namespace T66LootUIAnimation
{
	T66_API float SafeRangeAlpha(float Value, float Start, float End);
	T66_API float InvertWeightedDecelProgress(float NormalizedValue);
	T66_API FT66ScrollingStripMotionPlan BuildScrollingStripMotionPlan(const FT66ScrollingStripMotionInput& Input);
	T66_API float ComputeStripBoundaryMarkerProgress(float BoundaryOffset, float StartOffset, float EndOffset, bool bWeightedDecel);
	T66_API float ComputeSkipStartOffset(float CurrentOffset, float FinalOffset, float ItemStride, float SkipTravelTileCount);
}
