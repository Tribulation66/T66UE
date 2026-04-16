// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AActor;
class AT66TrapBase;
class UPrimitiveComponent;

class T66_API FT66TrapDamageUtils
{
public:
	static int32 ResolveScaledDamage(const AT66TrapBase* Trap, int32 BaseDamage);
	static bool ApplyTrapDamageToActor(AT66TrapBase* Trap, AActor* TargetActor, int32 BaseDamage);
	static int32 ApplyTrapDamageToOverlaps(AT66TrapBase* Trap, UPrimitiveComponent* DamageZone, int32 BaseDamage);
};
