// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66SkillRatingSubsystem.generated.h"

/**
 * Skill Rating: summarizes combat pressure handling over time.
 *
 * Design (v1):
 * - Rating starts at 50 (clamped 0..100).
 * - During tracking (combat active), we evaluate non-overlapping 5s windows.
 * - At the end of each 5s window, apply a base delta from damage taken:
 *   0 hits: +1
 *   1 hit : -1
 *   2 hits: -5
 *   3 hits: -10
 *   4 hits: -20
 *   5+ hits: -50
 * - Then apply a small pressure bonus/penalty based on hit checks, dodges, and expected dodges in that window.
 *
 * Time is driven externally via TickSkillRating. Hit-check events are pushed in by RunState.
 */
UCLASS()
class T66_API UT66SkillRatingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 DefaultSkillRating = 50;
	static constexpr float WindowDurationSeconds = 5.f;

	/** Reset tracking for a brand new run (rating -> 50 and clears window counters). */
	void ResetForNewRun();

	/** Enable/disable tracking (typically active when stage timer is active). */
	void SetTrackingActive(bool bActive);

	bool IsTrackingActive() const { return bTrackingActive; }

	/** Fallback damage event input: call when damage was actually applied to the player. */
	void NotifyDamageTaken();

	/** Hit-check event input: call once for every incoming hit check that evaluated evasion. */
	void NotifyHitCheck(float EvasionChance01, bool bDodged, bool bDamageApplied);

	/** Advance the window timer (cheap; only applies a rule when >= 5s elapsed). */
	void TickSkillRating(float DeltaSeconds);

	/** Current Skill Rating (0..100). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetSkillRating0To100() const { return SkillRating0To100; }

private:
	void ResetWindowState();
	void ApplyWindowResultDetailed(int32 HitsInWindow, int32 HitChecksInWindow, int32 DodgesInWindow, float ExpectedDodgesInWindow);

	UPROPERTY()
	int32 SkillRating0To100 = DefaultSkillRating;

	UPROPERTY()
	bool bTrackingActive = false;

	UPROPERTY()
	float WindowSecondsAccum = 0.f;

	UPROPERTY()
	int32 HitsThisWindow = 0;

	UPROPERTY()
	int32 HitChecksThisWindow = 0;

	UPROPERTY()
	int32 DodgesThisWindow = 0;

	UPROPERTY()
	float ExpectedDodgesThisWindow = 0.f;
};

