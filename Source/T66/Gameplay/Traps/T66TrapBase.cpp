// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66TrapBase.h"

#include "Core/T66TrapSubsystem.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Components/SceneComponent.h"

AT66TrapBase::AT66TrapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void AT66TrapBase::BeginPlay()
{
	Super::BeginPlay();

	if (UT66TrapSubsystem* TrapSubsystem = UT66TrapSubsystem::Get(GetWorld()))
	{
		TrapSubsystem->RegisterTrap(this);
	}
}

void AT66TrapBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UT66TrapSubsystem* TrapSubsystem = UT66TrapSubsystem::Get(GetWorld()))
	{
		TrapSubsystem->UnregisterTrap(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66TrapBase::SetTrapEnabled(const bool bInEnabled)
{
	if (bTrapEnabled == bInEnabled)
	{
		return;
	}

	bTrapEnabled = bInEnabled;
	HandleTrapEnabledChanged();
}

bool AT66TrapBase::UsesTimedActivation() const
{
	return ActivationMode == ET66TrapActivationMode::Timed || ActivationMode == ET66TrapActivationMode::Hybrid;
}

bool AT66TrapBase::UsesTriggeredActivation() const
{
	return ActivationMode == ET66TrapActivationMode::Triggered || ActivationMode == ET66TrapActivationMode::Hybrid;
}

bool AT66TrapBase::TryTriggerTrap(AActor* TriggeringActor)
{
	if (!UsesTriggeredActivation() || bExternalTriggerLocked || !CanTriggerFromActor(TriggeringActor) || !CanAcceptExternalTrigger())
	{
		return false;
	}

	if (!HandleTrapTriggered(TriggeringActor))
	{
		return false;
	}

	bExternalTriggerLocked = true;
	return true;
}

void AT66TrapBase::ApplyProgressionScalars(const float InDamageScalar, const float InSpeedScalar)
{
	const float NewDamageScalar = FMath::Max(0.10f, InDamageScalar);
	const float NewSpeedScalar = FMath::Max(0.10f, InSpeedScalar);
	if (FMath::IsNearlyEqual(ProgressionDamageScalar, NewDamageScalar, KINDA_SMALL_NUMBER)
		&& FMath::IsNearlyEqual(ProgressionSpeedScalar, NewSpeedScalar, KINDA_SMALL_NUMBER))
	{
		return;
	}

	ProgressionDamageScalar = NewDamageScalar;
	ProgressionSpeedScalar = NewSpeedScalar;
	HandleProgressionScalarsChanged();
}

int32 AT66TrapBase::ScaleTrapDamage(const int32 BaseDamage) const
{
	return FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * FMath::Max(0.10f, ProgressionDamageScalar)));
}

float AT66TrapBase::ScaleTrapDuration(const float BaseSeconds) const
{
	if (BaseSeconds <= 0.0f)
	{
		return 0.0f;
	}

	return BaseSeconds / FMath::Max(0.10f, ProgressionSpeedScalar);
}

void AT66TrapBase::HandleTrapEnabledChanged()
{
	if (!bTrapEnabled)
	{
		bExternalTriggerLocked = false;
	}
}

void AT66TrapBase::HandleProgressionScalarsChanged()
{
}

bool AT66TrapBase::CanAcceptExternalTrigger() const
{
	return bTrapEnabled;
}

bool AT66TrapBase::HandleTrapTriggered(AActor* TriggeringActor)
{
	(void)TriggeringActor;
	return false;
}

bool AT66TrapBase::IsHeroTargetable(const AT66HeroBase* Hero) const
{
	if (!bTrapEnabled || !IsValid(Hero))
	{
		return false;
	}

	if (Hero->IsInSafeZone())
	{
		return false;
	}

	if (!IsActorOnCompatibleTowerFloor(Hero))
	{
		return false;
	}

	return true;
}

bool AT66TrapBase::IsEnemyTargetable(const AT66EnemyBase* Enemy) const
{
	if (!bTrapEnabled || !IsValid(Enemy) || Enemy->CurrentHP <= 0)
	{
		return false;
	}

	if (!IsActorOnCompatibleTowerFloor(Enemy))
	{
		return false;
	}

	return true;
}

bool AT66TrapBase::CanTriggerFromActor(const AActor* Actor) const
{
	if (!bTrapEnabled || !IsValid(Actor) || !IsActorOnCompatibleTowerFloor(Actor))
	{
		return false;
	}

	if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(Actor))
	{
		if (TriggerTargetMode == ET66TrapTriggerTarget::EnemiesOnly)
		{
			return false;
		}

		return !Hero->IsInSafeZone();
	}

	if (Cast<AT66EnemyBase>(Actor))
	{
		return TriggerTargetMode != ET66TrapTriggerTarget::HeroesOnly;
	}

	return false;
}

bool AT66TrapBase::IsActorOnCompatibleTowerFloor(const AActor* Actor) const
{
	if (TowerFloorNumber == INDEX_NONE)
	{
		return true;
	}

	const AT66GameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
	if (!GameMode || !Actor)
	{
		return false;
	}

	if (GameMode->GetTowerFloorIndexForLocation(Actor->GetActorLocation()) != TowerFloorNumber)
	{
		return false;
	}

	return true;
}
