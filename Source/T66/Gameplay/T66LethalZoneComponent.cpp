// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66LethalZoneComponent.h"

#include "Core/T66RunStateSubsystem.h"
#include "Gameplay/T66HeroBase.h"

UT66LethalZoneComponent::UT66LethalZoneComponent()
{
	InitSphereRadius(450.f);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetGenerateOverlapEvents(true);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void UT66LethalZoneComponent::ConfigureLethalZone(const float InRadius, const FName InDeliveryMethod)
{
	SetSphereRadius(FMath::Max(1.f, InRadius));
	DeliveryMethod = InDeliveryMethod.IsNone() ? FName(TEXT("LethalZone")) : InDeliveryMethod;
}

void UT66LethalZoneComponent::BeginPlay()
{
	Super::BeginPlay();
	OnComponentBeginOverlap.AddDynamic(this, &UT66LethalZoneComponent::HandleLethalZoneBeginOverlap);
}

void UT66LethalZoneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnComponentBeginOverlap.RemoveDynamic(this, &UT66LethalZoneComponent::HandleLethalZoneBeginOverlap);
	Super::EndPlay(EndPlayReason);
}

void UT66LethalZoneComponent::HandleLethalZoneBeginOverlap(
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

	if (!Cast<AT66HeroBase>(OtherActor))
	{
		return;
	}

	const UWorld* World = GetWorld();
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (!RunState)
	{
		return;
	}

	RunState->KillPlayer(DeliveryMethod);
}
