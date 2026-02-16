// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroPlagueCloud.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

AT66HeroPlagueCloud::AT66HeroPlagueCloud()
{
	PrimaryActorTick.bCanEverTick = true;

	DamageZone = CreateDefaultSubobject<USphereComponent>(TEXT("DamageZone"));
	DamageZone->SetSphereRadius(400.f);
	DamageZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageZone->SetGenerateOverlapEvents(true);
	RootComponent = DamageZone;

	CloudMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CloudMesh"));
	CloudMesh->SetupAttachment(RootComponent);
	CloudMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cyl = FT66VisualUtil::GetBasicShapeCylinder())
		CloudMesh->SetStaticMesh(Cyl);

	InitialLifeSpan = 10.f;
}

void AT66HeroPlagueCloud::BeginPlay()
{
	Super::BeginPlay();

	DamageZone->SetSphereRadius(Radius);

	const float DiscScale = (Radius * 2.f) / 100.f;
	CloudMesh->SetRelativeScale3D(FVector(DiscScale, DiscScale, 4.f));
	CloudMesh->SetRelativeLocation(FVector(0.f, 0.f, 150.f));

	if (UMaterialInstanceDynamic* Mat = CloudMesh->CreateAndSetMaterialInstanceDynamic(0))
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.2f, 0.7f, 0.2f, 0.5f));

	// Timers started in InitFromUltimate so damage uses the correct UltimateDamage
}

void AT66HeroPlagueCloud::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AT66HeroPlagueCloud::InitFromUltimate(int32 UltimateDamage)
{
	TotalDamagePerTarget = UltimateDamage;
	NumTicks = FMath::Max(1, FMath::RoundToInt(DurationSeconds / TickIntervalSeconds));
	DamagePerTick = static_cast<float>(TotalDamagePerTarget) / static_cast<float>(NumTicks);
	const int32 TickDamage = FMath::Max(1, FMath::RoundToInt(DamagePerTick));

	UWorld* World = GetWorld();
	if (World)
	{
		FTimerDelegate TickDelegate;
		TickDelegate.BindLambda([this, TickDamage]()
		{
			DamageZone->UpdateOverlaps();
			TArray<AActor*> Overlapping;
			DamageZone->GetOverlappingActors(Overlapping);
			const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
			for (AActor* Actor : Overlapping)
			{
				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Actor))
				{
					if (E->CurrentHP > 0)
						E->TakeDamageFromHero(TickDamage, SourceID, NAME_None);
				}
				else if (AT66BossBase* B = Cast<AT66BossBase>(Actor))
				{
					if (B->IsAwakened() && B->IsAlive())
						B->TakeDamageFromHeroHit(TickDamage, SourceID, NAME_None);
				}
			}
		});
		World->GetTimerManager().SetTimer(TickTimerHandle, TickDelegate, TickIntervalSeconds, true, TickIntervalSeconds);
		World->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AT66HeroPlagueCloud::DestroySelf, DurationSeconds, false);
	}
}

void AT66HeroPlagueCloud::ApplyTickDamage()
{
	// Handled in timer lambda in InitFromUltimate
}

void AT66HeroPlagueCloud::DestroySelf()
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(TickTimerHandle);
	Destroy();
}
