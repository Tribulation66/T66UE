// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossGroundAOE.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Core/T66PixelVFXSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"

static UNiagaraSystem* LoadPixelVFX_AOE()
{
	UNiagaraSystem* Sys = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_PixelParticle.NS_PixelParticle"));
	if (!Sys)
		Sys = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1"));
	return Sys;
}

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

	InitialLifeSpan = 8.f;
}

void AT66BossGroundAOE::BeginPlay()
{
	Super::BeginPlay();
	DamageZone->SetSphereRadius(Radius);
	CachedPixelVFX = LoadPixelVFX_AOE();
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

	if (!CachedPixelVFX || !GetWorld()) return;

	VFXAccum += DeltaSeconds;

	if (!bDamageActivated)
	{
		WarningElapsed += DeltaSeconds;

		static constexpr float RingSpawnInterval = 0.06f;
		if (VFXAccum < RingSpawnInterval) return;
		VFXAccum -= RingSpawnInterval;

		const FVector Origin = GetActorLocation();
		const float T = WarningElapsed / FMath::Max(0.1f, WarningDurationSeconds);
		const float PulseFreq = FMath::Lerp(2.f, 8.f, FMath::Clamp(T, 0.f, 1.f));
		const float PulseScale = 1.f + 0.1f * FMath::Sin(WarningElapsed * PulseFreq * PI);
		const float EffectiveRadius = Radius * PulseScale;

		const FLinearColor BaseColor = bDamageEnemies
			? FLinearColor(0.2f, 0.3f, 0.9f, 0.6f)
			: FLinearColor(0.9f, 0.1f, 0.05f, 0.6f);
		UT66PixelVFXSubsystem* PixelVFX = GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>();

		static constexpr int32 RingParticles = 8;
		for (int32 i = 0; i < RingParticles; ++i)
		{
			const float Angle = (static_cast<float>(i) / static_cast<float>(RingParticles)) * 2.f * PI
				+ WarningElapsed * 2.f;
			const FVector Loc(
				Origin.X + FMath::Cos(Angle) * EffectiveRadius,
				Origin.Y + FMath::Sin(Angle) * EffectiveRadius,
				Origin.Z + 5.f);

			const FVector4 Tint(BaseColor.R, BaseColor.G, BaseColor.B, BaseColor.A);
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
	else
	{
		static constexpr float PillarSpawnInterval = 0.05f;
		if (VFXAccum < PillarSpawnInterval) return;
		VFXAccum -= PillarSpawnInterval;

		const FVector Origin = GetActorLocation();
		const FLinearColor PillarColor = bDamageEnemies
			? FLinearColor(0.4f, 0.2f, 0.9f, 0.9f)
			: FLinearColor(1.f, 0.15f, 0.05f, 0.9f);
		UT66PixelVFXSubsystem* PixelVFX = GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>();

		static constexpr int32 PillarParticles = 10;
		for (int32 i = 0; i < PillarParticles; ++i)
		{
			const float Angle = FMath::FRandRange(0.f, 2.f * PI);
			const float Dist = FMath::FRandRange(0.f, Radius * 0.7f);
			const float Z = FMath::FRandRange(0.f, 600.f);
			const FVector Loc(
				Origin.X + FMath::Cos(Angle) * Dist,
				Origin.Y + FMath::Sin(Angle) * Dist,
				Origin.Z + Z);

			const FVector4 Tint(PillarColor.R, PillarColor.G, PillarColor.B, PillarColor.A);
			if (PixelVFX)
			{
				PixelVFX->SpawnPixelAtLocation(
					Loc,
					FLinearColor(Tint.X, Tint.Y, Tint.Z, Tint.W),
					FVector2D(4.0f, 4.0f),
					ET66PixelVFXPriority::Low,
					FRotator::ZeroRotator,
					FVector(1.f),
					CachedPixelVFX);
			}
		}
	}
}

void AT66BossGroundAOE::ActivateDamage()
{
	bDamageActivated = true;
	VFXAccum = 0.f;

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

	DamageZone->SetGenerateOverlapEvents(false);

	// Spawn a big burst of particles at impact
	if (CachedPixelVFX && GetWorld())
	{
		const FVector Origin = GetActorLocation();
		const FLinearColor PillarColor = bDamageEnemies
			? FLinearColor(0.4f, 0.2f, 0.9f, 1.f)
			: FLinearColor(1.f, 0.15f, 0.05f, 1.f);
		UT66PixelVFXSubsystem* PixelVFX = GetWorld()->GetSubsystem<UT66PixelVFXSubsystem>();

		for (int32 i = 0; i < 30; ++i)
		{
			const float Angle = FMath::FRandRange(0.f, 2.f * PI);
			const float Dist = FMath::FRandRange(0.f, Radius);
			const float Z = FMath::FRandRange(0.f, 600.f);
			const FVector Loc(
				Origin.X + FMath::Cos(Angle) * Dist,
				Origin.Y + FMath::Sin(Angle) * Dist,
				Origin.Z + Z);
			if (PixelVFX)
			{
				PixelVFX->SpawnPixelAtLocation(
					Loc,
					FLinearColor(PillarColor.R, PillarColor.G, PillarColor.B, 1.f),
					FVector2D(4.0f, 4.0f),
					ET66PixelVFXPriority::Medium,
					FRotator::ZeroRotator,
					FVector(1.f),
					CachedPixelVFX);
			}
		}
	}

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
