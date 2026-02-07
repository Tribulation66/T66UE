// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66HeroSpeedSubsystem.generated.h"

/**
 * Hero movement speed and animation state.
 * Speed still ramps when moving (acceleration/deceleration unchanged).
 * Animation is binary: 0 = Idle (alert), 1 = Moving (run). No walk state.
 * Hero and companion both use GetMovementAnimState() so the companion always matches the hero.
 */
UCLASS()
class T66_API UT66HeroSpeedSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
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
	 * 0 = Idle (alert), 2 = Moving (run). No walk; speed/acceleration are unchanged but not tied to animation.
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
};
