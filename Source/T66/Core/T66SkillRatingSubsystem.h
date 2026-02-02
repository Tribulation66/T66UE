// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66SkillRatingSubsystem.generated.h"

/**
 * Skill Rating: measures "not taking damage" over time.
 *
 * Design (v0):
 * - Rating starts at 50 (clamped 0..100).
 * - During tracking (combat active), we evaluate non-overlapping 5s windows.
 * - At the end of each 5s window, apply:
 *   0 hits: +1
 *   1 hit : -1
 *   2 hits: -5
 *   3 hits: -10
 *   4 hits: -20
 *   5+ hits: -50
 *
 * Damage is an input event (NotifyDamageTaken). Time is driven externally via TickSkillRating or an engine timer.
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

	/** Damage event input: call when damage was actually applied to the player. */
	void NotifyDamageTaken();

	/** Advance the window timer (cheap; only applies a rule when >= 5s elapsed). */
	void TickSkillRating(float DeltaSeconds);

	/** Current Skill Rating (0..100). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RunState|Ratings")
	int32 GetSkillRating0To100() const { return SkillRating0To100; }

private:
	void ResetWindowState();
	void ApplyWindowResult(int32 HitsInWindow);

	UPROPERTY()
	int32 SkillRating0To100 = DefaultSkillRating;

	UPROPERTY()
	bool bTrackingActive = false;

	UPROPERTY()
	float WindowSecondsAccum = 0.f;

	UPROPERTY()
	int32 HitsThisWindow = 0;
};

