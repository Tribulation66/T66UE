// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66HeroSpeedSubsystem.generated.h"

/**
 * Hero movement speed and animation state.
 * Speed ramps when moving; animation: alert when stopped, walk when moving, run after 1 second of walking.
 * Hero and companion both use GetMovementAnimState() so the companion always matches the hero's animation.
 */
UCLASS()
class T66_API UT66HeroSpeedSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Seconds of continuous walking before switching to run animation. */
	static constexpr float SecondsWalkingBeforeRun = 1.f;

	/** When moving, speed is at least this fraction of max (so base speed feels higher). */
	static constexpr float BaseSpeedFraction = 0.2f;

	/** Set movement params from hero data (call when hero spawns or initializes). */
	UFUNCTION(BlueprintCallable, Category = "T66|HeroSpeed")
	void SetParams(float InMaxSpeed, float AccelerationPercentPerSecond = 10.f);

	/** Call each frame from the hero: DeltaTime and whether the player has movement input. */
	UFUNCTION(BlueprintCallable, Category = "T66|HeroSpeed")
	void Update(float DeltaTime, bool bHasMovementInput);

	/** Current speed (0 to MaxSpeed). Used for hero MaxWalkSpeed. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	float GetCurrentSpeed() const { return CurrentSpeed; }

	/** Max speed for the current hero. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	float GetMaxSpeed() const { return MaxSpeed; }

	/**
	 * Animation state for hero and companion (both read this so companion always matches hero).
	 * 0 = Idle (alert), 1 = Walk, 2 = Run.
	 * Run starts after SecondsWalkingBeforeRun of continuous movement.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	int32 GetMovementAnimState() const;

private:
	UPROPERTY(Transient)
	float CurrentSpeed = 0.f;

	UPROPERTY(Transient)
	float MaxSpeed = 1200.f;

	/** Units per second added when moving. */
	UPROPERTY(Transient)
	float AccelerationPerSecond = 120.f;

	UPROPERTY(Transient)
	float DecelerationPerSecond = 120.f;

	/** Time spent moving this run (reset when stopped). After >= SecondsWalkingBeforeRun we use run anim. */
	UPROPERTY(Transient)
	float TimeWalkingSeconds = 0.f;
};
