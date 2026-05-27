// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66HeroSpeedSubsystem.generated.h"

/**
 * Hero movement speed and movement-state signaling.
 * No acceleration: speed is always the resolved movement speed when moving, 0 when idle.
 * Movement animation state: 0 = Idle, 1 = Walk, 2 = Jump, 3 = Roll.
 * Companions mirror this state for their own visuals.
 */
UCLASS()
class T66_API UT66HeroSpeedSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Set the resolved movement speed signal (call when hero movement speed changes). */
	UFUNCTION(BlueprintCallable, Category = "T66|HeroSpeed")
	void SetParams(float InMovementSpeed, float AccelerationPercentPerSecond = 10.f);

	/** Update the current speed signal. Animation state is published separately by SetMovementAnimState. */
	UFUNCTION(BlueprintCallable, Category = "T66|HeroSpeed")
	void Update(float DeltaTime, bool bHasMovementInput);

	/** Current speed signal (resolved movement speed when moving, 0 when idle). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	float GetCurrentSpeed() const { return CurrentSpeed; }

	/** Resolved movement speed for the current hero. Kept under the legacy API name for compatibility. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	float GetMaxSpeed() const { return MaxSpeed; }

	/** Movement state for hero/companion visuals. 0 = Idle, 1 = Walk, 2 = Jump, 3 = Roll. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "T66|HeroSpeed")
	int32 GetMovementAnimState() const;

	UFUNCTION(BlueprintCallable, Category = "T66|HeroSpeed")
	void SetMovementAnimState(int32 InState);

private:
	UPROPERTY(Transient)
	float CurrentSpeed = 0.f;

	UPROPERTY(Transient)
	float MaxSpeed = 2400.f;

	UPROPERTY(Transient)
	int32 MovementAnimState = 0;
};
