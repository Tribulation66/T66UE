// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatHitZoneComponent.h"

UT66CombatHitZoneComponent::UT66CombatHitZoneComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SetGenerateOverlapEvents(true);
	SetCanEverAffectNavigation(false);
	SetHiddenInGame(true);
	InitSphereRadius(32.f);
}

FT66CombatTargetHandle UT66CombatHitZoneComponent::MakeTargetHandle() const
{
	FT66CombatTargetHandle Handle;
	if (!bTargetable)
	{
		return Handle;
	}

	Handle.Actor = GetOwner();
	Handle.HitComponent = const_cast<UT66CombatHitZoneComponent*>(this);
	Handle.HitZoneType = HitZoneType;
	Handle.HitZoneName = HitZoneName.IsNone() ? GetFName() : HitZoneName;
	Handle.AimPoint = GetAimPoint();
	return Handle;
}
