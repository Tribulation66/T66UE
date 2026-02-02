// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66SkillRatingSubsystem.h"

namespace
{
	static int32 T66_SkillDeltaForHitsInWindow(int32 Hits)
	{
		const int32 H = FMath::Max(0, Hits);
		switch (H)
		{
		case 0:  return +1;
		case 1:  return -1;
		case 2:  return -5;
		case 3:  return -10;
		case 4:  return -20;
		default: return -50; // 5+
		}
	}
}

void UT66SkillRatingSubsystem::ResetWindowState()
{
	WindowSecondsAccum = 0.f;
	HitsThisWindow = 0;
}

void UT66SkillRatingSubsystem::ResetForNewRun()
{
	SkillRating0To100 = DefaultSkillRating;
	bTrackingActive = false;
	ResetWindowState();
}

void UT66SkillRatingSubsystem::SetTrackingActive(bool bActive)
{
	if (bTrackingActive == bActive) return;
	bTrackingActive = bActive;

	// Policy: when entering/leaving tracking, reset the current window so start-area downtime
	// can never "complete" a window (and to keep windows aligned to combat-only time).
	ResetWindowState();
}

void UT66SkillRatingSubsystem::NotifyDamageTaken()
{
	if (!bTrackingActive) return;
	HitsThisWindow = FMath::Clamp(HitsThisWindow + 1, 0, 1000000);
}

void UT66SkillRatingSubsystem::ApplyWindowResult(int32 HitsInWindow)
{
	const int32 Delta = T66_SkillDeltaForHitsInWindow(HitsInWindow);
	SkillRating0To100 = FMath::Clamp(SkillRating0To100 + Delta, 0, 100);
}

void UT66SkillRatingSubsystem::TickSkillRating(float DeltaSeconds)
{
	if (!bTrackingActive) return;
	if (DeltaSeconds <= 0.f) return;

	WindowSecondsAccum += DeltaSeconds;
	while (WindowSecondsAccum >= WindowDurationSeconds)
	{
		ApplyWindowResult(HitsThisWindow);
		WindowSecondsAccum -= WindowDurationSeconds;
		HitsThisWindow = 0;
	}
}

