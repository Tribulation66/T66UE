// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Movement/T66HeroMovementComponent.h"

#include "Gameplay/T66HeroBase.h"
#include "Core/T66HeroSpeedSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"

UT66HeroMovementComponent::UT66HeroMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UT66HeroMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedHero = Cast<AT66HeroBase>(GetOwner());
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			CachedRunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			CachedHeroSpeedSubsystem = GI->GetSubsystem<UT66HeroSpeedSubsystem>();
		}
	}

	ApplyDefaultMovementConfig();
	RefreshWalkSpeedFromRunState();
	UpdateAnimationStateBridge();
}

AT66HeroBase* UT66HeroMovementComponent::ResolveHero() const
{
	return CachedHero ? CachedHero.Get() : Cast<AT66HeroBase>(GetOwner());
}

UCharacterMovementComponent* UT66HeroMovementComponent::ResolveCharacterMovement() const
{
	if (AT66HeroBase* Hero = ResolveHero())
	{
		return Hero->GetCharacterMovement();
	}
	return nullptr;
}

float UT66HeroMovementComponent::ResolveCurrentMaxWalkSpeed() const
{
	if (const UCharacterMovementComponent* Movement = ResolveCharacterMovement())
	{
		return FMath::Max(200.f, Movement->MaxWalkSpeed);
	}

	return FMath::Max(200.f, BaseWalkSpeed);
}

void UT66HeroMovementComponent::ApplyDefaultMovementConfig()
{
	if (AT66HeroBase* Hero = ResolveHero())
	{
		Hero->JumpMaxCount = 1;
		Hero->JumpMaxHoldTime = MovementTuning.JumpMaxHoldTime;
	}

	if (UCharacterMovementComponent* Movement = ResolveCharacterMovement())
	{
		Movement->MaxWalkSpeed = BaseWalkSpeed;
		Movement->MaxAcceleration = MovementTuning.MaxAcceleration;
		Movement->BrakingDecelerationWalking = MovementTuning.BrakingDecelerationWalking;
		Movement->GroundFriction = MovementTuning.GroundFriction;
		Movement->bUseSeparateBrakingFriction = MovementTuning.bUseSeparateBrakingFriction;
		Movement->BrakingFriction = MovementTuning.BrakingFriction;
		Movement->BrakingFrictionFactor = MovementTuning.BrakingFrictionFactor;
		Movement->JumpZVelocity = MovementTuning.JumpZVelocity;
		Movement->AirControl = MovementTuning.AirControl;
		Movement->GravityScale = MovementTuning.GravityScale;
		Movement->FallingLateralFriction = MovementTuning.FallingLateralFriction;
		Movement->BrakingDecelerationFalling = MovementTuning.BrakingDecelerationFalling;
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.f, MovementTuning.RotationRateYaw, 0.f);
	}
}

void UT66HeroMovementComponent::RefreshWalkSpeedFromRunState()
{
	UCharacterMovementComponent* Movement = ResolveCharacterMovement();
	if (!Movement)
	{
		return;
	}

	float Multiplier = 1.f;
	if (!CachedRunState)
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				CachedRunState = GI->GetSubsystem<UT66RunStateSubsystem>();
			}
		}
	}

	if (CachedRunState)
	{
		Multiplier =
			CachedRunState->GetHeroMoveSpeedMultiplier() *
			CachedRunState->GetItemMoveSpeedMultiplier() *
			CachedRunState->GetMovementSpeedSecondaryMultiplier() *
			CachedRunState->GetLastStandMoveSpeedMultiplier() *
			CachedRunState->GetStageMoveSpeedMultiplier() *
			CachedRunState->GetStatusMoveSpeedMultiplier();
	}

	Movement->MaxWalkSpeed = FMath::Clamp(BaseWalkSpeed * Multiplier, 200.f, 10000.f);
}

void UT66HeroMovementComponent::SetHeroBaseWalkSpeed(const float InBaseWalkSpeed)
{
	BaseWalkSpeed = FMath::Max(200.f, InBaseWalkSpeed);

	if (CachedHeroSpeedSubsystem)
	{
		CachedHeroSpeedSubsystem->SetParams(BaseWalkSpeed);
	}

	RefreshWalkSpeedFromRunState();
}

void UT66HeroMovementComponent::SetMoveInputAxes(const float ForwardValue, const float RightValue)
{
	CachedForwardInput = ForwardValue;
	CachedRightInput = RightValue;
	UpdateAnimationStateBridge();

	if (bDashModifierHeld && !bDashConsumedThisHold)
	{
		TryConsumeHeldDash();
	}
}

void UT66HeroMovementComponent::SetDashModifierHeld(const bool bHeld)
{
	if (bDashModifierHeld == bHeld)
	{
		return;
	}

	bDashModifierHeld = bHeld;
	if (!bDashModifierHeld)
	{
		bDashConsumedThisHold = false;
		return;
	}

	TryConsumeHeldDash();
}

bool UT66HeroMovementComponent::TryJump()
{
	AT66HeroBase* Hero = ResolveHero();
	if (!Hero || !CanUseMovementAbilities() || !Hero->CanJump())
	{
		return false;
	}

	Hero->Jump();
	return true;
}

void UT66HeroMovementComponent::StopJumping()
{
	if (AT66HeroBase* Hero = ResolveHero())
	{
		Hero->StopJumping();
	}
}

bool UT66HeroMovementComponent::HasMovementInput() const
{
	return FMath::Abs(CachedForwardInput) > 0.1f || FMath::Abs(CachedRightInput) > 0.1f;
}

ET66DashDirection UT66HeroMovementComponent::GetCurrentDashDirection() const
{
	const FVector2D Input(CachedForwardInput, CachedRightInput);
	if (Input.SizeSquared() <= FMath::Square(0.25f))
	{
		return ET66DashDirection::None;
	}

	const float AngleDegrees = FMath::RadiansToDegrees(FMath::Atan2(Input.Y, Input.X));
	const int32 Octant = ((FMath::RoundToInt(AngleDegrees / 45.f) % 8) + 8) % 8;
	switch (Octant)
	{
	case 0: return ET66DashDirection::North;
	case 1: return ET66DashDirection::NorthEast;
	case 2: return ET66DashDirection::East;
	case 3: return ET66DashDirection::SouthEast;
	case 4: return ET66DashDirection::South;
	case 5: return ET66DashDirection::SouthWest;
	case 6: return ET66DashDirection::West;
	case 7: return ET66DashDirection::NorthWest;
	default: return ET66DashDirection::None;
	}
}

FVector UT66HeroMovementComponent::GetWorldMoveDirectionFromAxes() const
{
	if (!HasMovementInput())
	{
		return FVector::ZeroVector;
	}

	const AT66HeroBase* Hero = ResolveHero();
	const AController* Controller = Hero ? Hero->GetController() : nullptr;
	const FRotator ControlRot = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
	const FRotator FlatRot(0.f, ControlRot.Yaw, 0.f);
	const FVector ForwardDir = FRotationMatrix(FlatRot).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(FlatRot).GetUnitAxis(EAxis::Y);

	FVector MoveDir = (ForwardDir * CachedForwardInput) + (RightDir * CachedRightInput);
	MoveDir.Z = 0.f;
	MoveDir.Normalize();
	return MoveDir;
}

FVector UT66HeroMovementComponent::GetQuantizedWorldDashDirection() const
{
	const FVector WorldMoveDir = GetWorldMoveDirectionFromAxes();
	if (WorldMoveDir.IsNearlyZero())
	{
		return FVector::ZeroVector;
	}

	const float WorldYaw = FMath::RadiansToDegrees(FMath::Atan2(WorldMoveDir.Y, WorldMoveDir.X));
	const float QuantizedYaw = FMath::RoundToFloat(WorldYaw / 45.f) * 45.f;
	return FRotator(0.f, QuantizedYaw, 0.f).Vector();
}

bool UT66HeroMovementComponent::CanUseMovementAbilities() const
{
	const AT66HeroBase* Hero = ResolveHero();
	if (!Hero)
	{
		return false;
	}

	return !Hero->IsPreviewMode()
		&& !Hero->IsVehicleMounted()
		&& !Hero->IsQuickReviveDowned();
}

float UT66HeroMovementComponent::ResolveDashCooldownSeconds() const
{
	float Cooldown = MovementTuning.DashCooldownSeconds;
	if (CachedRunState)
	{
		Cooldown *= CachedRunState->GetDashCooldownMultiplier();
	}

	return FMath::Clamp(Cooldown, 0.05f, 10.f);
}

void UT66HeroMovementComponent::UpdateAnimationStateBridge() const
{
	if (CachedHeroSpeedSubsystem)
	{
		CachedHeroSpeedSubsystem->Update(0.f, HasMovementInput());
	}
}

void UT66HeroMovementComponent::TryConsumeHeldDash()
{
	if (!bDashModifierHeld || bDashConsumedThisHold)
	{
		return;
	}

	const FVector DashDirection = GetQuantizedWorldDashDirection();
	if (DashDirection.IsNearlyZero())
	{
		return;
	}

	if (TryDashInWorldDirection(DashDirection))
	{
		bDashConsumedThisHold = true;
	}
}

bool UT66HeroMovementComponent::TryDashInWorldDirection(const FVector& DesiredWorldDirection)
{
	AT66HeroBase* Hero = ResolveHero();
	UWorld* World = GetWorld();
	if (!Hero || !World || !CanUseMovementAbilities())
	{
		return false;
	}

	FVector DashDirection = DesiredWorldDirection;
	DashDirection.Z = 0.f;
	if (!DashDirection.Normalize())
	{
		return false;
	}

	const float Now = static_cast<float>(World->GetTimeSeconds());
	if (Now - LastDashTime < ResolveDashCooldownSeconds())
	{
		return false;
	}

	const float DashStrength = FMath::Max(
		MovementTuning.DashStrength,
		ResolveCurrentMaxWalkSpeed() * MovementTuning.DashSpeedMultiplierOverWalkSpeed);

	LastDashTime = Now;
	Hero->LaunchCharacter(DashDirection * DashStrength, true, true);
	return true;
}
