// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66HeroSpeedSubsystem.generated.h"

/**
 * Hero movement speed and movement-state signaling.
 * No acceleration: speed is always MaxSpeed when moving, 0 when idle.
 * State is binary: 0 = Idle, 1 = Moving.
 * Heroes decide whether Moving means walk or jump; companions reuse it for their own visuals.
 */
UCLASS()
class T66_API UT66HeroSpeedSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Set movement params from hero data (call when hero spawns or initializes). */
	UFUNCTION(BlueprintCallable, Category = "T66|HeroSpeed")
	void SetParams(float InMaxSpeed, float AccelerationPercentPerSecond = 10.f);

	/** Call each frame from the hero: DeltaTime and whether the player has movement input. */
	UFUNCTION(BlueprintCallable, Category = "T66|HeroSpeed")
	void Update(float DeltaTime, bool bHasMovementInput);

	/** Current speed (MaxSpeed when moving, 0 when idle). Used for hero MaxWalkSpeed. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	float GetCurrentSpeed() const { return CurrentSpeed; }

	/** Max speed for the current hero. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	float GetMaxSpeed() const { return MaxSpeed; }

	/** Movement state for hero/companion visuals. 0 = Idle, 1 = Moving. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	int32 GetMovementAnimState() const;

private:
	UPROPERTY(Transient)
	float CurrentSpeed = 0.f;

	UPROPERTY(Transient)
	float MaxSpeed = 1200.f;

	UPROPERTY(Transient)
	bool bLastHasMovementInput = false;
};
