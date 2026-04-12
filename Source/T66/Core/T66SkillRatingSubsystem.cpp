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

	static int32 T66_SkillPressureDeltaForWindow(int32 HitChecks, int32 Hits, int32 Dodges, float ExpectedDodges)
	{
		if (HitChecks < 4)
		{
			return 0;
		}

		int32 Delta = 0;
		if (Hits <= 0)
		{
			Delta += 1;
		}

		const float ExcessDodges = static_cast<float>(Dodges) - FMath::Max(0.f, ExpectedDodges);
		if (ExcessDodges >= 2.5f)
		{
			Delta += 2;
		}
		else if (ExcessDodges >= 1.25f)
		{
			Delta += 1;
		}
		else if (ExpectedDodges >= 1.f && ExcessDodges <= -2.5f)
		{
			Delta -= 2;
		}
		else if (ExpectedDodges >= 1.f && ExcessDodges <= -1.25f)
		{
			Delta -= 1;
		}

		return Delta;
	}
}

void UT66SkillRatingSubsystem::ResetWindowState()
{
	WindowSecondsAccum = 0.f;
	HitsThisWindow = 0;
	HitChecksThisWindow = 0;
	DodgesThisWindow = 0;
	ExpectedDodgesThisWindow = 0.f;
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

void UT66SkillRatingSubsystem::NotifyHitCheck(float EvasionChance01, bool bDodged, bool bDamageApplied)
{
	if (!bTrackingActive) return;

	HitChecksThisWindow = FMath::Clamp(HitChecksThisWindow + 1, 0, 1000000);
	ExpectedDodgesThisWindow = FMath::Clamp(ExpectedDodgesThisWindow + FMath::Clamp(EvasionChance01, 0.f, 1.f), 0.f, 1000000.f);
	if (bDodged)
	{
		DodgesThisWindow = FMath::Clamp(DodgesThisWindow + 1, 0, 1000000);
	}
	if (bDamageApplied)
	{
		HitsThisWindow = FMath::Clamp(HitsThisWindow + 1, 0, 1000000);
	}
}

void UT66SkillRatingSubsystem::ApplyWindowResultDetailed(int32 HitsInWindow, int32 HitChecksInWindow, int32 DodgesInWindow, float ExpectedDodgesInWindow)
{
	const int32 Delta = T66_SkillDeltaForHitsInWindow(HitsInWindow)
		+ T66_SkillPressureDeltaForWindow(HitChecksInWindow, HitsInWindow, DodgesInWindow, ExpectedDodgesInWindow);
	SkillRating0To100 = FMath::Clamp(SkillRating0To100 + Delta, 0, 100);
}

void UT66SkillRatingSubsystem::TickSkillRating(float DeltaSeconds)
{
	if (!bTrackingActive) return;
	if (DeltaSeconds <= 0.f) return;

	WindowSecondsAccum += DeltaSeconds;
	while (WindowSecondsAccum >= WindowDurationSeconds)
	{
		ApplyWindowResultDetailed(HitsThisWindow, HitChecksThisWindow, DodgesThisWindow, ExpectedDodgesThisWindow);
		WindowSecondsAccum -= WindowDurationSeconds;
		HitsThisWindow = 0;
		HitChecksThisWindow = 0;
		DodgesThisWindow = 0;
		ExpectedDodgesThisWindow = 0.f;
	}
}

