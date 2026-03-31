// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroPlagueCloud.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

static UNiagaraSystem* LoadPixelVFX()
{
	static TObjectPtr<UNiagaraSystem> CachedSystem = nullptr;
	static TObjectPtr<UNiagaraSystem> CachedFallbackSystem = nullptr;
	if (!CachedSystem)
	{
		CachedSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
	}
	if (!CachedSystem && !CachedFallbackSystem)
	{
		CachedFallbackSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
	}
	return CachedSystem ? CachedSystem.Get() : CachedFallbackSystem.Get();
}

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

	InitialLifeSpan = 10.f;
}

void AT66HeroPlagueCloud::BeginPlay()
{
	Super::BeginPlay();

	if (DamageZone)
	{
		DamageZone->SetSphereRadius(Radius);
	}
	CachedPixelVFX = LoadPixelVFX();
}

void AT66HeroPlagueCloud::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CachedPixelVFX || !GetWorld()) return;

	VFXAccum += DeltaSeconds;
	static constexpr float SpawnInterval = 0.08f;
	if (VFXAccum < SpawnInterval) return;
	VFXAccum -= SpawnInterval;

	const FVector Origin = GetActorLocation();
	static constexpr int32 ParticlesPerBatch = 6;
	UT66PixelVFXSubsystem* PixelVFX = GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>();
	for (int32 i = 0; i < ParticlesPerBatch; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Dist = FMath::FRandRange(0.f, Radius);
		const float Z = FMath::FRandRange(30.f, 200.f);
		const FVector Loc(Origin.X + FMath::Cos(Angle) * Dist,
		                  Origin.Y + FMath::Sin(Angle) * Dist,
		                  Origin.Z + Z);

		const float G = FMath::FRandRange(0.5f, 0.85f);
		const FVector4 Tint(0.15f, G, 0.1f, 0.7f);
		if (PixelVFX)
		{
			PixelVFX->SpawnPixelAtLocation(
				Loc,
				FLinearColor(Tint.X, Tint.Y, Tint.Z, Tint.W),
				FVector2D(3.0f, 3.0f),
				ET66PixelVFXPriority::Low,
				FRotator::ZeroRotator,
				FVector(1.f),
				CachedPixelVFX);
		}
	}
}

void AT66HeroPlagueCloud::InitFromUltimate(int32 UltimateDamage)
{
	TotalDamagePerTarget = UltimateDamage;
	NumTicks = FMath::Max(1, FMath::RoundToInt(DurationSeconds / TickIntervalSeconds));
	DamagePerTick = static_cast<float>(TotalDamagePerTarget) / static_cast<float>(NumTicks);
	const int32 TickDamage = FMath::Max(1, FMath::RoundToInt(DamagePerTick));

	UWorld* World = GetWorld();
	if (World && DamageZone)
	{
		const TWeakObjectPtr<AT66HeroPlagueCloud> WeakThis(this);
		FTimerDelegate TickDelegate;
		TickDelegate.BindLambda([WeakThis, TickDamage]()
		{
			AT66HeroPlagueCloud* Cloud = WeakThis.Get();
			if (!IsValid(Cloud))
			{
				return;
			}

			USphereComponent* Zone = Cloud->DamageZone;
			if (!IsValid(Zone) || !IsValid(Cloud->GetWorld()))
			{
				return;
			}

			Zone->UpdateOverlaps();
			TArray<AActor*> Overlapping;
			Zone->GetOverlappingActors(Overlapping);
			const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;
			for (AActor* Actor : Overlapping)
			{
				if (!IsValid(Actor))
				{
					continue;
				}

				if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Actor))
				{
					if (IsValid(E) && E->CurrentHP > 0)
						E->TakeDamageFromHero(TickDamage, SourceID, NAME_None);
				}
				else if (AT66BossBase* B = Cast<AT66BossBase>(Actor))
				{
					if (IsValid(B) && B->IsAwakened() && B->IsAlive())
						B->TakeDamageFromHeroHit(TickDamage, SourceID, NAME_None);
				}
			}
		});
		World->GetTimerManager().SetTimer(TickTimerHandle, TickDelegate, TickIntervalSeconds, true, TickIntervalSeconds);
		World->GetTimerManager().SetTimer(DestroyTimerHandle, this, &AT66HeroPlagueCloud::DestroySelf, DurationSeconds, false);
	}
}

void AT66HeroPlagueCloud::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimerHandle);
		World->GetTimerManager().ClearTimer(DestroyTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66HeroPlagueCloud::ApplyTickDamage()
{
}

void AT66HeroPlagueCloud::DestroySelf()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimerHandle);
		World->GetTimerManager().ClearTimer(DestroyTimerHandle);
	}
	Destroy();
}
