// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TowerDescentHole.h"

#include "Components/BoxComponent.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"

AT66TowerDescentHole::AT66TowerDescentHole()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetBoxExtent(FVector(900.0f, 900.0f, 1200.0f));
	RootComponent = TriggerBox;
}

void AT66TowerDescentHole::InitializeHole(const int32 InFromFloorNumber, const int32 InToFloorNumber, const FVector& InTriggerExtent)
{
	FromFloorNumber = InFromFloorNumber;
	ToFloorNumber = InToFloorNumber;
	if (TriggerBox)
	{
		TriggerBox->SetBoxExtent(InTriggerExtent);
	}
}

void AT66TowerDescentHole::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AT66TowerDescentHole::OnTriggerBeginOverlap);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AT66TowerDescentHole::OnTriggerEndOverlap);
	}
}

void AT66TowerDescentHole::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero)
	{
		return;
	}

	TWeakObjectPtr<AActor> WeakActor(OtherActor);
	if (ActiveActors.Contains(WeakActor))
	{
		return;
	}

	ActiveActors.Add(WeakActor);

	if (AT66GameMode* GameMode = GetWorld() ? Cast<AT66GameMode>(GetWorld()->GetAuthGameMode()) : nullptr)
	{
		GameMode->HandleTowerDescentHoleTriggered(Hero, FromFloorNumber, ToFloorNumber);
	}
}

void AT66TowerDescentHole::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	ActiveActors.Remove(TWeakObjectPtr<AActor>(OtherActor));
}
