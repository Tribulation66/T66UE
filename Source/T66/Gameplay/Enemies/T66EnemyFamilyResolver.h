// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Enemies/T66EnemyFamilyTypes.h"
#include "Templates/SubclassOf.h"

class AT66EnemyBase;

class T66_API FT66EnemyFamilyResolver
{
public:
	static FName NormalizeMobID(FName MobID);
	static bool IsStageMobID(FName MobID);
	static ET66EnemyFamily ResolveFamily(FName MobID);
	static TSubclassOf<AT66EnemyBase> ResolveEnemyClass(FName MobID, TSubclassOf<AT66EnemyBase> FallbackClass = nullptr);
};
