// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossGroundAOE.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AT66BossGroundAOE::AT66BossGroundAOE()
{
	PrimaryActorTick.bCanEverTick = true;

	DamageZone = CreateDefaultSubobject<USphereComponent>(TEXT("DamageZone"));
	DamageZone->SetSphereRadius(300.f);
	DamageZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageZone->SetGenerateOverlapEvents(false);
	RootComponent = DamageZone;

	// Warning disc: flat cylinder on the ground (red, semi-transparent)
	WarningDisc = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WarningDisc"));
	WarningDisc->SetupAttachment(RootComponent);
	WarningDisc->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cyl = FT66VisualUtil::GetBasicShapeCylinder())
	{
		WarningDisc->SetStaticMesh(Cyl);
	}

	// Impact pillar: tall cylinder (red, fully opaque), hidden initially
	ImpactPillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ImpactPillar"));
	ImpactPillar->SetupAttachment(RootComponent);
	ImpactPillar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ImpactPillar->SetVisibility(false);
	if (UStaticMesh* Cyl = FT66VisualUtil::GetBasicShapeCylinder())
	{
		ImpactPillar->SetStaticMesh(Cyl);
	}

	InitialLifeSpan = 8.f;
}

void AT66BossGroundAOE::BeginPlay()
{
	Super::BeginPlay();

	// Scale the damage zone sphere to match Radius
	DamageZone->SetSphereRadius(Radius);

	// Scale the warning disc: UE cylinder is 100 units diameter, 100 units tall.
	// We want diameter = Radius*2, height = very thin (5 units).
	const float DiscScale = (Radius * 2.f) / 100.f;
	WarningDisc->SetRelativeScale3D(FVector(DiscScale, DiscScale, 0.05f));
	WarningDisc->SetRelativeLocation(FVector(0.f, 0.f, 3.f));

	if (UMaterialInstanceDynamic* Mat = WarningDisc->CreateAndSetMaterialInstanceDynamic(0))
	{
		FLinearColor DiscColor = bDamageEnemies ? FLinearColor(0.2f, 0.3f, 0.9f, 0.45f) : FLinearColor(0.9f, 0.1f, 0.05f, 0.45f);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), DiscColor);
	}

	// Scale the impact pillar: same diameter, 600 units tall
	static constexpr float PillarHeight = 600.f;
	const float PillarScaleZ = PillarHeight / 100.f;
	ImpactPillar->SetRelativeScale3D(FVector(DiscScale, DiscScale, PillarScaleZ));
	ImpactPillar->SetRelativeLocation(FVector(0.f, 0.f, PillarHeight * 0.5f));

	if (UMaterialInstanceDynamic* Mat = ImpactPillar->CreateAndSetMaterialInstanceDynamic(0))
	{
		FLinearColor PillarColor = bDamageEnemies ? FLinearColor(0.4f, 0.2f, 0.9f, 0.8f) : FLinearColor(1.f, 0.15f, 0.05f, 0.8f);
		Mat->SetVectorParameterValue(TEXT("BaseColor"), PillarColor);
	}

	WarningElapsed = 0.f;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(ActivateTimerHandle, this, &AT66BossGroundAOE::ActivateDamage, WarningDurationSeconds, false);
	}
}

void AT66BossGroundAOE::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bDamageActivated) return;

	// Pulse the warning disc scale to telegraph urgency
	WarningElapsed += DeltaSeconds;
	const float T = WarningElapsed / FMath::Max(0.1f, WarningDurationSeconds);
	const float PulseFreq = FMath::Lerp(2.f, 8.f, FMath::Clamp(T, 0.f, 1.f));
	const float PulseScale = 1.f + 0.06f * FMath::Sin(WarningElapsed * PulseFreq * PI);
	const float DiscScale = (Radius * 2.f) / 100.f;
	WarningDisc->SetRelativeScale3D(FVector(DiscScale * PulseScale, DiscScale * PulseScale, 0.05f));
}

void AT66BossGroundAOE::ActivateDamage()
{
	bDamageActivated = true;

	// Enable overlap events and force an update so we detect actors already inside
	DamageZone->SetGenerateOverlapEvents(true);
	DamageZone->UpdateOverlaps();

	const FName UltimateSourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	if (bDamageEnemies)
	{
		TArray<AActor*> Overlapping;
		DamageZone->GetOverlappingActors(Overlapping);
		for (AActor* Actor : Overlapping)
		{
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Actor))
			{
				if (E->CurrentHP > 0)
					E->TakeDamageFromHero(DamageHP, UltimateSourceID, NAME_None);
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Actor))
			{
				if (B->IsAwakened() && B->IsAlive())
					B->TakeDamageFromHeroHit(DamageHP, UltimateSourceID, NAME_None);
			}
		}
	}
	else
	{
		TArray<AActor*> Overlapping;
		DamageZone->GetOverlappingActors(Overlapping, AT66HeroBase::StaticClass());
		for (AActor* Actor : Overlapping)
		{
			AT66HeroBase* Hero = Cast<AT66HeroBase>(Actor);
			if (!Hero) continue;
			if (Hero->IsInSafeZone()) continue;

			UWorld* World = GetWorld();
			UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
			UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
			if (RunState)
				RunState->ApplyDamage(DamageHP, GetOwner());
		}
	}

	// Disable further overlap events
	DamageZone->SetGenerateOverlapEvents(false);

	// Visual swap: hide disc, show pillar
	WarningDisc->SetVisibility(false);
	ImpactPillar->SetVisibility(true);

	// Stop ticking (no more pulse needed)
	SetActorTickEnabled(false);

	// Schedule cleanup
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AT66BossGroundAOE::DestroySelf, PillarLingerSeconds, false);
	}
}

void AT66BossGroundAOE::DestroySelf()
{
	Destroy();
}
