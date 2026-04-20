// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Movement/T66HeroMovementTypes.h"
#include "T66HeroMovementComponent.generated.h"

class AT66HeroBase;
class UCharacterMovementComponent;
class UT66HeroSpeedSubsystem;
class UT66RunStateSubsystem;

UCLASS(ClassGroup=(Movement), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class T66_API UT66HeroMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UT66HeroMovementComponent();

	void ApplyDefaultMovementConfig();
	void RefreshWalkSpeedFromRunState();
	void SetHeroBaseWalkSpeed(float InBaseWalkSpeed);

	void SetMoveInputAxes(float ForwardValue, float RightValue);
	void SetDashModifierHeld(bool bHeld);

	bool TryJump();
	void StopJumping();
	bool TryDashInWorldDirection(const FVector& DesiredWorldDirection);

	bool HasMovementInput() const;
	float GetForwardInputValue() const { return CachedForwardInput; }
	float GetRightInputValue() const { return CachedRightInput; }
	ET66DashDirection GetCurrentDashDirection() const;

	const FT66HeroMovementTuning& GetMovementTuning() const { return MovementTuning; }

protected:
	virtual void BeginPlay() override;

private:
	AT66HeroBase* ResolveHero() const;
	UCharacterMovementComponent* ResolveCharacterMovement() const;
	float ResolveCurrentMaxWalkSpeed() const;
	FVector GetWorldMoveDirectionFromAxes() const;
	FVector GetQuantizedWorldDashDirection() const;
	bool CanUseMovementAbilities() const;
	float ResolveDashCooldownSeconds() const;
	void UpdateAnimationStateBridge() const;
	void TryConsumeHeldDash();

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	FT66HeroMovementTuning MovementTuning;

	UPROPERTY(Transient)
	TObjectPtr<AT66HeroBase> CachedHero = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66RunStateSubsystem> CachedRunState = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66HeroSpeedSubsystem> CachedHeroSpeedSubsystem = nullptr;

	float BaseWalkSpeed = 1800.f;
	float CachedForwardInput = 0.f;
	float CachedRightInput = 0.f;
	float LastDashTime = -9999.f;
	bool bDashModifierHeld = false;
	bool bDashConsumedThisHold = false;
};
