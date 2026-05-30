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
	if (MobID == FName(TEXT("CaveBat"))
		|| MobID == FName(TEXT("HiveWasp"))
		|| MobID == FName(TEXT("GhostRay"))
		|| MobID == FName(TEXT("SaucerDrone"))
		|| MobID == FName(TEXT("FireSkull"))
		|| MobID == FName(TEXT("Gargoyle"))
		|| MobID == FName(TEXT("CursedCrow"))
		|| MobID == FName(TEXT("WillOWisp"))
		|| MobID == FName(TEXT("GullDiver"))
		|| MobID == FName(TEXT("ReconOrb"))
		|| MobID == FName(TEXT("CinderWraith")))
	{
		return ET66EnemyFamily::Flying;
	}
	if (MobID == FName(TEXT("HexSlinger"))
		|| MobID == FName(TEXT("StoneSentinel"))
		|| MobID == FName(TEXT("BoneConjurer"))
		|| MobID == FName(TEXT("ThornImp"))
		|| MobID == FName(TEXT("ForestWraith"))
		|| MobID == FName(TEXT("MyconidDruid"))
		|| MobID == FName(TEXT("JellyHover"))
		|| MobID == FName(TEXT("CoralMortar"))
		|| MobID == FName(TEXT("BrineStrafer"))
		|| MobID == FName(TEXT("DrownedPriestess"))
		|| MobID == FName(TEXT("DroneGrunt"))
		|| MobID == FName(TEXT("PlasmaSpitter"))
		|| MobID == FName(TEXT("PlasmaSentinel"))
		|| MobID == FName(TEXT("CyberLich"))
		|| MobID == FName(TEXT("BrimstoneMortar"))
		|| MobID == FName(TEXT("PlagueCultist")))
	{
		return ET66EnemyFamily::Ranged;
	}
	if (MobID == FName(TEXT("RatPack"))
		|| MobID == FName(TEXT("MimicLure"))
		|| MobID == FName(TEXT("TuskerBoar"))
		|| MobID == FName(TEXT("SporeBomb"))
		|| MobID == FName(TEXT("ReefShark"))
		|| MobID == FName(TEXT("SeaMine"))
		|| MobID == FName(TEXT("RocketLeaper"))
		|| MobID == FName(TEXT("CrystalBomber"))
		|| MobID == FName(TEXT("PitImp"))
		|| MobID == FName(TEXT("Hellhound"))
		|| MobID == FName(TEXT("SinEater"))
		|| MobID == FName(TEXT("FamishedGhoul"))
		|| MobID == FName(TEXT("GoreStag")))
	{
		return ET66EnemyFamily::Rush;
	}
	if (MobID == FName(TEXT("RegularEnemy"))
		|| MobID == FName(TEXT("Slime"))
		|| MobID == FName(TEXT("BoneWalker"))
		|| MobID == FName(TEXT("TombSpider"))
		|| MobID == FName(TEXT("CryptWraith"))
		|| MobID == FName(TEXT("MushroomBrute"))
		|| MobID == FName(TEXT("TreantSapling"))
		|| MobID == FName(TEXT("TreantAncient"))
		|| MobID == FName(TEXT("VineStrangler"))
		|| MobID == FName(TEXT("CrabGuard"))
		|| MobID == FName(TEXT("DrownedSailor"))
		|| MobID == FName(TEXT("AnglerfishStalker"))
		|| MobID == FName(TEXT("CrystalCrawler"))
		|| MobID == FName(TEXT("MindSlug"))
		|| MobID == FName(TEXT("SandTunneler"))
		|| MobID == FName(TEXT("BoneKnight"))
		|| MobID == FName(TEXT("DemonSentinel"))
		|| MobID == FName(TEXT("HellWyrm"))
		|| MobID == FName(TEXT("Hammerjaw"))
		|| MobID == FName(TEXT("CarapaceBrute"))
		|| MobID == FName(TEXT("BrimstoneBrute")))
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
