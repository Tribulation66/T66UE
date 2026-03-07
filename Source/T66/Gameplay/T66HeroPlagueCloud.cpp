// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroPlagueCloud.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

static UNiagaraSystem* LoadPixelVFX()
{
	UNiagaraSystem* Sys = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
	if (!Sys)
		Sys = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
	return Sys;
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

	DamageZone->SetSphereRadius(Radius);
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

		UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), CachedPixelVFX, Loc, FRotator::ZeroRotator,
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
		if (NC)
		{
			NC->SetVariableVec4(FName(TEXT("User.Tint")), Tint);
			NC->SetVariableVec2(FName(TEXT("User.SpriteSize")), FVector2D(3.0, 3.0));
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
}

void AT66HeroPlagueCloud::DestroySelf()
{
	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(TickTimerHandle);
	Destroy();
}
