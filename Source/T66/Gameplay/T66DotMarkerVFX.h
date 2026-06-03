// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T66DotMarkerVFX.generated.h"

class UStaticMeshComponent;

/**
 * Temporary placeholder visual for the Hero 1 DOT weapon. Spawns a small ring of
 * sphere markers attached to the DOT target so they follow it for the DOT duration,
 * then self-destructs. The markers are created hidden and reveal one at a time on a
 * 0.5s cadence (index 0 immediately, 1 at +0.5s, 2 at +1.0s), and each revealed marker
 * disappears again after a short 0.3s visible pulse so only one placeholder dot shows
 * at a time (marker 0 visible 0.00-0.30s, marker 1 0.50-0.80s, marker 2 1.00-1.30s).
 * The 0.3s pulse is the current placeholder presentation; it is not a universal rule for
 * future DOT/final-art projectiles unless a future effect packet declares it. These
 * markers are
 * presentation-only applicator visuals: they carry NO damage, collision, or status
 * authority. The single authoritative DOT payload stays in
 * UT66RunStateSubsystem::ApplyDOT (one source, one payload).
 */
UCLASS()
class T66_API AT66DotMarkerVFX : public AActor
{
	GENERATED_BODY()

public:
	AT66DotMarkerVFX();

	/**
	 * Build MarkerCount sphere markers, attach this actor to FollowTarget so the markers
	 * track it for LifeSeconds, then self-destruct. Visual-only.
	 */
	void InitializeMarkers(AActor* FollowTarget, int32 MarkerCount, const FLinearColor& Color, float MarkerScale, float LifeSeconds);

	/** Number of markers actually spawned (proof hook). */
	int32 GetSpawnedMarkerCount() const { return Markers.Num(); }

	/** Seconds between each marker reveal in the staggered cadence. */
	static constexpr float MarkerRevealIntervalSeconds = 0.5f;

	/**
	 * Seconds each placeholder marker stays visible after its reveal before it disappears.
	 * Current placeholder pulse duration only; future DOT/final-art effects must opt in via
	 * their own effect packet rather than inheriting this value.
	 */
	static constexpr float MarkerVisibleDurationSeconds = 0.3f;

private:
	/** Make the marker at Index visible and log a proof line. Safe if Index is stale. */
	void RevealMarker(int32 Index);

	/** Hide the marker at Index after its visible pulse and log a proof line. Safe if Index is stale. */
	void HideMarker(int32 Index);

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> Markers;

	/** DOT target, kept weak for proof logging only (no authority, may outlive target). */
	TWeakObjectPtr<AActor> FollowTargetWeak;

	/** Marker count captured at init for proof logging. */
	int32 PlannedMarkerCount = 0;

	/** Reveal timer handles; cleared automatically when this actor is destroyed. */
	TArray<FTimerHandle> RevealTimerHandles;

	/** Hide timer handles; cleared automatically when this actor is destroyed. */
	TArray<FTimerHandle> HideTimerHandles;
};
