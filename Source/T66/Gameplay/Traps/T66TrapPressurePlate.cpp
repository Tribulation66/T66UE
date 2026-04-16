// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66TrapPressurePlate.h"

#include "Gameplay/Traps/T66TrapBase.h"

#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "TimerManager.h"

AT66TrapPressurePlate::AT66TrapPressurePlate()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	TriggerZone->SetupAttachment(SceneRoot);
	TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerZone->SetGenerateOverlapEvents(true);

	PlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
	PlateMesh->SetupAttachment(SceneRoot);
	PlateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlateMesh->SetCastShadow(false);
}

void AT66TrapPressurePlate::AddLinkedTrap(AT66TrapBase* Trap)
{
	if (!Trap)
	{
		return;
	}

	LinkedTraps.AddUnique(Trap);
}

void AT66TrapPressurePlate::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateVisuals();
}

void AT66TrapPressurePlate::BeginPlay()
{
	Super::BeginPlay();

	UpdateVisuals();
	if (TriggerZone)
	{
		TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &AT66TrapPressurePlate::HandleTriggerZoneBeginOverlap);
		TriggerZone->OnComponentEndOverlap.AddDynamic(this, &AT66TrapPressurePlate::HandleTriggerZoneEndOverlap);
	}
}

void AT66TrapPressurePlate::HandleTriggerZoneBeginOverlap(
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

	if (!CanTriggerFromActor(OtherActor))
	{
		return;
	}

	++OverlappingTriggerActorCount;
	if (!bArmed)
	{
		UpdateVisuals();
		return;
	}

	bArmed = false;
	UpdateVisuals();
	TriggerLinkedTraps(OtherActor);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RearmTimerHandle);
		World->GetTimerManager().SetTimer(
			RearmTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (OverlappingTriggerActorCount <= 0)
				{
					bArmed = true;
					UpdateVisuals();
				}
			}),
			FMath::Max(RearmDelaySeconds, KINDA_SMALL_NUMBER),
			false);
	}
}

void AT66TrapPressurePlate::HandleTriggerZoneEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherActor;
	(void)OtherComp;
	(void)OtherBodyIndex;

	OverlappingTriggerActorCount = FMath::Max(0, OverlappingTriggerActorCount - 1);
	if (OverlappingTriggerActorCount <= 0 && !bArmed)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RearmTimerHandle);
			World->GetTimerManager().SetTimer(
				RearmTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					if (OverlappingTriggerActorCount <= 0)
					{
						bArmed = true;
						UpdateVisuals();
					}
				}),
				FMath::Max(RearmDelaySeconds, KINDA_SMALL_NUMBER),
				false);
		}
		else
		{
			bArmed = true;
			UpdateVisuals();
		}
	}
}

void AT66TrapPressurePlate::UpdateVisuals()
{
	if (TriggerZone)
	{
		TriggerZone->SetBoxExtent(TriggerExtents);
	}

	if (!PlateMesh)
	{
		return;
	}

	if (!PlateMesh->GetStaticMesh())
	{
		if (UStaticMesh* CylinderMesh = FT66VisualUtil::GetBasicShapeCylinder())
		{
			PlateMesh->SetStaticMesh(CylinderMesh);
		}
	}

	PlateMesh->SetRelativeLocation(FVector(0.f, 0.f, -TriggerExtents.Z + 6.f));
	PlateMesh->SetRelativeScale3D(FVector(TriggerExtents.X / 50.f, TriggerExtents.Y / 50.f, 0.06f));
	FT66VisualUtil::ApplyT66Color(PlateMesh, this, bArmed ? IdleColor : PressedColor);
}

bool AT66TrapPressurePlate::CanTriggerFromActor(const AActor* Actor) const
{
	if (!Actor || !IsActorOnCompatibleTowerFloor(Actor))
	{
		return false;
	}

	if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(Actor))
	{
		if (TriggerTargetMode == ET66TrapTriggerTarget::EnemiesOnly)
		{
			return false;
		}

		return !Hero->IsInSafeZone();
	}

	if (Cast<AT66EnemyBase>(Actor))
	{
		return TriggerTargetMode != ET66TrapTriggerTarget::HeroesOnly;
	}

	return false;
}

bool AT66TrapPressurePlate::IsActorOnCompatibleTowerFloor(const AActor* Actor) const
{
	if (TowerFloorNumber == INDEX_NONE)
	{
		return true;
	}

	const AT66GameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
	return GameMode && Actor && GameMode->GetTowerFloorIndexForLocation(Actor->GetActorLocation()) == TowerFloorNumber;
}

void AT66TrapPressurePlate::TriggerLinkedTraps(AActor* TriggeringActor)
{
	for (int32 Index = LinkedTraps.Num() - 1; Index >= 0; --Index)
	{
		AT66TrapBase* Trap = LinkedTraps[Index].Get();
		if (!Trap)
		{
			LinkedTraps.RemoveAtSwap(Index, 1, EAllowShrinking::No);
			continue;
		}

		Trap->TryTriggerTrap(TriggeringActor);
	}
}
