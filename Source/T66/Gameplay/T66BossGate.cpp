// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossGate.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GameMode.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPath.h"

namespace
{
	APawn* T66ResolveClosestBossGatePawn(const AActor* ContextActor)
	{
		const UWorld* World = ContextActor ? ContextActor->GetWorld() : nullptr;
		if (!World)
		{
			return nullptr;
		}

		const FVector Origin = ContextActor->GetActorLocation();
		APawn* BestPawn = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (!Pawn)
			{
				continue;
			}

			const float DistSq = FVector::DistSquared2D(Origin, Pawn->GetActorLocation());
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestPawn = Pawn;
			}
		}

		return BestPawn;
	}
}

AT66BossGate::AT66BossGate()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(60.f, 80.f, 180.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = TriggerBox;

	UStaticMesh* CylinderMesh = FT66VisualUtil::GetBasicShapeCylinder();
	const float PoleRadius = 40.f;
	const float PoleHeight = 320.f;
	const float PoleSpacing = 100.f;

	PoleLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleLeft"));
	PoleLeft->SetStaticMesh(CylinderMesh);
	PoleLeft->SetRelativeLocation(FVector(0.f, -PoleSpacing, PoleHeight * 0.5f));
	PoleLeft->SetRelativeScale3D(FVector(PoleRadius / 50.f, PoleRadius / 50.f, PoleHeight / 100.f));
	PoleLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PoleLeft->SetupAttachment(RootComponent);

	PoleRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleRight"));
	PoleRight->SetStaticMesh(CylinderMesh);
	PoleRight->SetRelativeLocation(FVector(0.f, PoleSpacing, PoleHeight * 0.5f));
	PoleRight->SetRelativeScale3D(FVector(PoleRadius / 50.f, PoleRadius / 50.f, PoleHeight / 100.f));
	PoleRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PoleRight->SetupAttachment(RootComponent);

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(RootComponent);
	GateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GateMesh->SetVisibility(false, true);

	GateMeshOverride = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Gates/BossGate_Pixal3D.BossGate_Pixal3D")));
}

void AT66BossGate::BeginPlay()
{
	Super::BeginPlay();
	ApplyImportedGateMesh();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AT66BossGate::OnBoxBeginOverlap);
}

void AT66BossGate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bTriggered) return;

	APawn* Pawn = T66ResolveClosestBossGatePawn(this);
	if (!Pawn) return;

	const float Dist2D = FVector::Dist2D(Pawn->GetActorLocation(), GetActorLocation());
	if (Dist2D <= TriggerDistance2D)
	{
		TryTriggerForActor(Pawn);
	}
}

void AT66BossGate::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryTriggerForActor(OtherActor);
}

void AT66BossGate::ApplyImportedGateMesh()
{
	if (!GateMesh || GateMeshOverride.IsNull())
	{
		return;
	}

	if (UStaticMesh* ImportedMesh = GateMeshOverride.LoadSynchronous())
	{
		GateMesh->EmptyOverrideMaterials();
		GateMesh->SetStaticMesh(ImportedMesh);
		GateMesh->SetRelativeScale3D(FVector::OneVector);
		GateMesh->SetRelativeRotation(FRotator::ZeroRotator);
		FT66VisualUtil::GroundMeshToActorOrigin(GateMesh, ImportedMesh);
		GateMesh->SetVisibility(true, true);
		GateMesh->SetHiddenInGame(false, true);

		if (PoleLeft)
		{
			PoleLeft->SetVisibility(false, true);
			PoleLeft->SetHiddenInGame(true, true);
		}
		if (PoleRight)
		{
			PoleRight->SetVisibility(false, true);
			PoleRight->SetHiddenInGame(true, true);
		}
	}
}

void AT66BossGate::TryTriggerForActor(AActor* OtherActor)
{
	if (bTriggered || !OtherActor) return;
	if (!Cast<AT66HeroBase>(OtherActor)) return;

	bTriggered = true;
	SetActorTickEnabled(false);

	// Awaken the stage boss immediately once the player passes the boss pillars.
	UWorld* World = GetWorld();
	if (!World) return;
	if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
	{
		for (const TWeakObjectPtr<AT66BossBase>& WeakBoss : Registry->GetBosses())
		{
			if (AT66BossBase* Boss = WeakBoss.Get())
			{
				Boss->ForceAwaken();
			}
		}
	}

	if (AT66GameMode* GameMode = World->GetAuthGameMode<AT66GameMode>())
	{
		GameMode->SetEnemyDirectorSpawningPaused(true);
	}
}

