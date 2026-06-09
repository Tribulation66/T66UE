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
	void SetHeroBaseSpeedStat(int32 InBaseSpeedStat);

	void SetMoveInputAxes(float ForwardValue, float RightValue);

	bool TryJump();
	void StopJumping();
	bool TryLeap();
	bool TryLeapInWorldDirection(const FVector& DesiredWorldDirection);
	UE_DEPRECATED(5.7, "Use TryLeap.")
	bool TryRollForward();
	UE_DEPRECATED(5.7, "Use TryLeapInWorldDirection.")
	bool TryDashInWorldDirection(const FVector& DesiredWorldDirection);

	bool HasMovementInput() const;
	float GetForwardInputValue() const { return CachedForwardInput; }
	float GetRightInputValue() const { return CachedRightInput; }

	const FT66HeroMovementTuning& GetMovementTuning() const { return MovementTuning; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	AT66HeroBase* ResolveHero() const;
	UCharacterMovementComponent* ResolveCharacterMovement() const;
	float ResolveCurrentMaxWalkSpeed() const;
	bool CanUseMovementAbilities() const;
	float ResolveLeapCooldownSeconds() const;
	void UpdateSurfaceBounce(float DeltaTime);
	bool WantsSurfaceBounce(const AT66HeroBase* Hero, const UCharacterMovementComponent* Movement) const;
	void UpdateSurfaceBounceAirborneState(AT66HeroBase* Hero, UCharacterMovementComponent* Movement, float Now);
	bool TryApplyGroundLandingSurfaceBounce(AT66HeroBase* Hero, UCharacterMovementComponent* Movement, float Now, float ImpactDownSpeed, float FallHeight);
	bool TryApplyWallSurfaceBounce(AT66HeroBase* Hero, UCharacterMovementComponent* Movement, float Now);
	FVector ResolveSurfaceBounceDirection(const AT66HeroBase* Hero, const UCharacterMovementComponent* Movement) const;
	void UpdateAnimationStateBridge() const;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	FT66HeroMovementTuning MovementTuning;

	UPROPERTY(Transient)
	TObjectPtr<AT66HeroBase> CachedHero = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66RunStateSubsystem> CachedRunState = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UT66HeroSpeedSubsystem> CachedHeroSpeedSubsystem = nullptr;

	float BaseWalkSpeed = 600.f;
	float CachedForwardInput = 0.f;
	float CachedRightInput = 0.f;
	float LastLeapTime = -9999.f;
	float LastGroundSurfaceBounceTime = -9999.f;
	float LastWallSurfaceBounceTime = -9999.f;
	float SurfaceBounceAirbornePeakZ = 0.f;
	float SurfaceBounceLastAirborneVelocityZ = 0.f;
	bool bSurfaceBounceWasAirborne = false;
};
