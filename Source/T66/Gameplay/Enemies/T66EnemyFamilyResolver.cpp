// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/T66EnemyFamilyResolver.h"

#include "Gameplay/Enemies/T66FlyingEnemy.h"
#include "Gameplay/Enemies/T66MeleeEnemy.h"
#include "Gameplay/Enemies/T66RangedEnemy.h"
#include "Gameplay/Enemies/T66RushEnemy.h"

FName FT66EnemyFamilyResolver::NormalizeMobID(const FName MobID)
{
	return MobID;
}

bool FT66EnemyFamilyResolver::IsStageMobID(FName MobID)
{
	MobID = NormalizeMobID(MobID);
	return ResolveFamily(MobID) != ET66EnemyFamily::Special;
}

ET66EnemyFamily FT66EnemyFamilyResolver::ResolveFamily(FName MobID)
{
	MobID = NormalizeMobID(MobID);
	if (MobID == FName(TEXT("Dungeon_Bat"))
		|| MobID == FName(TEXT("Forest_Wasp"))
		|| MobID == FName(TEXT("Ocean_GhostRay"))
		|| MobID == FName(TEXT("Martian_SaucerDrone"))
		|| MobID == FName(TEXT("Hell_Gargoyle")))
	{
		return ET66EnemyFamily::Flying;
	}
	if (MobID == FName(TEXT("Dungeon_WebSpider"))
		|| MobID == FName(TEXT("Forest_ThornImp"))
		|| MobID == FName(TEXT("Ocean_Jellyfish"))
		|| MobID == FName(TEXT("Martian_PlasmaSpitter"))
		|| MobID == FName(TEXT("Hell_FireSkull")))
	{
		return ET66EnemyFamily::Ranged;
	}
	if (MobID == FName(TEXT("Dungeon_RabidRat"))
		|| MobID == FName(TEXT("Forest_Boar"))
		|| MobID == FName(TEXT("Ocean_SharkPup"))
		|| MobID == FName(TEXT("Martian_RocketLeaper"))
		|| MobID == FName(TEXT("Hellhound")))
	{
		return ET66EnemyFamily::Rush;
	}
	if (MobID == FName(TEXT("RegularEnemy"))
		|| MobID == FName(TEXT("Dungeon_Slime"))
		|| MobID == FName(TEXT("Dungeon_Skeleton"))
		|| MobID == FName(TEXT("Forest_MushroomBrute"))
		|| MobID == FName(TEXT("Forest_TreantSapling"))
		|| MobID == FName(TEXT("Ocean_CrabGuard"))
		|| MobID == FName(TEXT("Ocean_DrownedSailor"))
		|| MobID == FName(TEXT("Martian_DroneGrunt"))
		|| MobID == FName(TEXT("Martian_CrystalCrawler"))
		|| MobID == FName(TEXT("Hell_Imp"))
		|| MobID == FName(TEXT("Hell_BoneKnight")))
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
