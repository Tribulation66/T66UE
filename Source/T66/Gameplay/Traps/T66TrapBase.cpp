// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66TrapBase.h"

#include "Core/T66TrapSubsystem.h"
#include "Gameplay/T66GameMode.h"
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

	if (TowerFloorNumber != INDEX_NONE)
	{
		const AT66GameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
		if (GameMode && GameMode->GetTowerFloorIndexForLocation(Hero->GetActorLocation()) != TowerFloorNumber)
		{
			return false;
		}
	}

	return true;
}
