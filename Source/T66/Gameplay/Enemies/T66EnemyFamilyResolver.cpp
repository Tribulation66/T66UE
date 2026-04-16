// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/T66EnemyFamilyResolver.h"

#include "Gameplay/Enemies/T66FlyingEnemy.h"
#include "Gameplay/Enemies/T66MeleeEnemy.h"
#include "Gameplay/Enemies/T66RangedEnemy.h"
#include "Gameplay/Enemies/T66RushEnemy.h"

FName FT66EnemyFamilyResolver::NormalizeMobID(const FName MobID)
{
	if (MobID == FName(TEXT("Rooster")))
	{
		return FName(TEXT("Roost"));
	}

	return MobID;
}

bool FT66EnemyFamilyResolver::IsStageMobID(FName MobID)
{
	MobID = NormalizeMobID(MobID);
	return MobID == FName(TEXT("Cow"))
		|| MobID == FName(TEXT("Pig"))
		|| MobID == FName(TEXT("Goat"))
		|| MobID == FName(TEXT("Roost"));
}

ET66EnemyFamily FT66EnemyFamilyResolver::ResolveFamily(FName MobID)
{
	MobID = NormalizeMobID(MobID);
	if (MobID == FName(TEXT("Pig")))
	{
		return ET66EnemyFamily::Rush;
	}
	if (MobID == FName(TEXT("Goat")))
	{
		return ET66EnemyFamily::Ranged;
	}
	if (MobID == FName(TEXT("Roost")))
	{
		return ET66EnemyFamily::Flying;
	}
	if (MobID == FName(TEXT("Cow")) || MobID == FName(TEXT("RegularEnemy")))
	{
		return ET66EnemyFamily::Melee;
	}

	return ET66EnemyFamily::Special;
}

TSubclassOf<AT66EnemyBase> FT66EnemyFamilyResolver::ResolveEnemyClass(FName MobID, TSubclassOf<AT66EnemyBase> FallbackClass)
{
	switch (ResolveFamily(MobID))
	{
	case ET66EnemyFamily::Flying:
		return AT66FlyingEnemy::StaticClass();
	case ET66EnemyFamily::Ranged:
		return AT66RangedEnemy::StaticClass();
	case ET66EnemyFamily::Rush:
		return AT66RushEnemy::StaticClass();
	case ET66EnemyFamily::Melee:
		return AT66MeleeEnemy::StaticClass();
	case ET66EnemyFamily::Special:
	default:
		break;
	}

	if (FallbackClass)
	{
		return FallbackClass;
	}

	return AT66MeleeEnemy::StaticClass();
}
