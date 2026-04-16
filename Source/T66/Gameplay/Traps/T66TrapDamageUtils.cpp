// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66TrapDamageUtils.h"

#include "Gameplay/Traps/T66TrapBase.h"

#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/GameInstance.h"

int32 FT66TrapDamageUtils::ResolveScaledDamage(const AT66TrapBase* Trap, const int32 BaseDamage)
{
	return Trap ? Trap->ScaleTrapDamage(BaseDamage) : FMath::Max(1, BaseDamage);
}

bool FT66TrapDamageUtils::ApplyTrapDamageToActor(AT66TrapBase* Trap, AActor* TargetActor, const int32 BaseDamage)
{
	if (!Trap || !TargetActor || BaseDamage <= 0)
	{
		return false;
	}

	const int32 DamageAmount = ResolveScaledDamage(Trap, BaseDamage);
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(TargetActor))
	{
		if (!Trap->CanDamageHeroes() || !Trap->CanAffectHero(Hero))
		{
			return false;
		}

		if (UGameInstance* GameInstance = Trap->GetWorld() ? Trap->GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->ApplyDamage(DamageAmount, Trap);
				return true;
			}
		}

		return false;
	}

	if (AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(TargetActor))
	{
		if (!Trap->CanDamageEnemies() || !Trap->CanAffectEnemy(Enemy))
		{
			return false;
		}

		return Enemy->TakeDamageFromEnvironment(DamageAmount, Trap);
	}

	return false;
}

int32 FT66TrapDamageUtils::ApplyTrapDamageToOverlaps(AT66TrapBase* Trap, UPrimitiveComponent* DamageZone, const int32 BaseDamage)
{
	if (!Trap || !DamageZone)
	{
		return 0;
	}

	TArray<AActor*> OverlappingActors;
	DamageZone->GetOverlappingActors(OverlappingActors);

	int32 DamagedActors = 0;
	for (AActor* OverlappingActor : OverlappingActors)
	{
		DamagedActors += ApplyTrapDamageToActor(Trap, OverlappingActor, BaseDamage) ? 1 : 0;
	}

	return DamagedActors;
}
