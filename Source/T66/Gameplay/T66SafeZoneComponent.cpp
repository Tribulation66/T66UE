// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66SafeZoneComponent.h"

#include "Core/T66ActorRegistrySubsystem.h"
#include "Gameplay/T66HeroBase.h"
#include "Engine/World.h"

UT66SafeZoneComponent::UT66SafeZoneComponent()
{
	InitSphereRadius(650.f);
}

void UT66SafeZoneComponent::ConfigureSafeZone(const float InRadius)
{
	SetSphereRadius(FMath::Max(1.f, InRadius));
}

void UT66SafeZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetGenerateOverlapEvents(true);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	OnComponentBeginOverlap.AddDynamic(this, &UT66SafeZoneComponent::HandleSafeZoneBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UT66SafeZoneComponent::HandleSafeZoneEndOverlap);

	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterSafeZone(this);
		}
	}
}

void UT66SafeZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AT66HeroBase* Hero = OverlappingHero.Get())
	{
		Hero->AddSafeZoneOverlap(-1);
		OverlappingHero.Reset();
	}

	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterSafeZone(this);
		}
	}

	OnComponentBeginOverlap.RemoveDynamic(this, &UT66SafeZoneComponent::HandleSafeZoneBeginOverlap);
	OnComponentEndOverlap.RemoveDynamic(this, &UT66SafeZoneComponent::HandleSafeZoneEndOverlap);

	Super::EndPlay(EndPlayReason);
}

void UT66SafeZoneComponent::HandleSafeZoneBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	AT66HeroBase* Hero = ResolveHero(OtherActor);
	if (!Hero || OverlappingHero.IsValid())
	{
		return;
	}

	OverlappingHero = Hero;
	Hero->AddSafeZoneOverlap(+1);
}

void UT66SafeZoneComponent::HandleSafeZoneEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;

	AT66HeroBase* Hero = ResolveHero(OtherActor);
	if (!Hero || OverlappingHero.Get() != Hero)
	{
		return;
	}

	OverlappingHero.Reset();
	Hero->AddSafeZoneOverlap(-1);
}

AT66HeroBase* UT66SafeZoneComponent::ResolveHero(AActor* OtherActor) const
{
	return Cast<AT66HeroBase>(OtherActor);
}
