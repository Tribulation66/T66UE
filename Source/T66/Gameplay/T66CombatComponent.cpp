// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66HeroProjectile.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

UT66CombatComponent::UT66CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Optional SFX asset (must be imported to a SoundWave/SoundCue .uasset).
	ShotSfx = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/SFX/Shot.Shot")));
}

void UT66CombatComponent::SetLockedTarget(AActor* InTarget)
{
	LockedTarget = InTarget;
}

void UT66CombatComponent::ClearLockedTarget()
{
	LockedTarget.Reset();
}

void UT66CombatComponent::BeginPlay()
{
	Super::BeginPlay();

	BaseAttackRange = AttackRange;
	BaseFireIntervalSeconds = FireIntervalSeconds;
	BaseDamagePerShot = DamagePerShot;

	CachedRunState = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (CachedRunState)
	{
		CachedRunState->InventoryChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->HeroProgressChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->SurvivalChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
	}

	RecomputeFromRunState();
	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, EffectiveFireIntervalSeconds, true, EffectiveFireIntervalSeconds);
}

void UT66CombatComponent::HandleInventoryChanged()
{
	const float OldInterval = EffectiveFireIntervalSeconds;
	RecomputeFromRunState();
	if (!FMath::IsNearlyEqual(OldInterval, EffectiveFireIntervalSeconds, 0.001f))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FireTimerHandle);
			World->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, EffectiveFireIntervalSeconds, true, EffectiveFireIntervalSeconds);
		}
	}
}

void UT66CombatComponent::RecomputeFromRunState()
{
	AttackRange = BaseAttackRange;
	EffectiveFireIntervalSeconds = BaseFireIntervalSeconds;
	EffectiveDamagePerShot = BaseDamagePerShot;
	ProjectileScaleMultiplier = 1.f;

	if (!CachedRunState)
	{
		return;
	}

	const float AttackSpeedMult = CachedRunState->GetItemAttackSpeedMultiplier();
	const float DamageMult = CachedRunState->GetItemDamageMultiplier();
	const float ScaleMult = CachedRunState->GetItemScaleMultiplier();

	const float HeroAttackSpeedMult = CachedRunState->GetHeroAttackSpeedMultiplier() * CachedRunState->GetLastStandAttackSpeedMultiplier();
	const float HeroDamageMult = CachedRunState->GetHeroDamageMultiplier();
	const float HeroScaleMult = CachedRunState->GetHeroScaleMultiplier();

	const float TotalAttackSpeed = AttackSpeedMult * HeroAttackSpeedMult;
	const float TotalDamage = DamageMult * HeroDamageMult;
	const float TotalScale = ScaleMult * HeroScaleMult;

	EffectiveFireIntervalSeconds = FMath::Clamp(BaseFireIntervalSeconds / FMath::Max(0.01f, TotalAttackSpeed), 0.05f, 10.f);
	EffectiveDamagePerShot = FMath::Clamp(FMath::RoundToInt(static_cast<float>(BaseDamagePerShot) * TotalDamage), 1, 999999);
	ProjectileScaleMultiplier = FMath::Clamp(TotalScale, 0.1f, 10.f);
}

void UT66CombatComponent::PlayShotSfx()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Apply user SFX volume even if SoundClass assets aren't set up yet.
	UGameInstance* GI = World->GetGameInstance();
	UT66PlayerSettingsSubsystem* PS = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	const float MasterVol = PS ? FMath::Clamp(PS->GetMasterVolume(), 0.f, 1.f) : 1.f;
	const float SfxVol = PS ? FMath::Clamp(PS->GetSfxVolume(), 0.f, 1.f) : 1.f;
	const float EffectiveSfx = MasterVol * SfxVol;
	if (EffectiveSfx <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	USoundBase* Sound = ShotSfx.LoadSynchronous();
	if (!Sound)
	{
		if (!bShotSfxWarnedMissing)
		{
			bShotSfxWarnedMissing = true;
			UE_LOG(LogTemp, Warning, TEXT("Shot SFX not found. Import your Shot audio into UE so '/Game/Audio/SFX/Shot' exists (SoundWave/SoundCue)."));
		}
		return;
	}

	// Auto attack is effectively UI-ish for now; play 2D so it always reads clearly.
	UGameplayStatics::PlaySound2D(World, Sound, EffectiveSfx);
}

void UT66CombatComponent::TryFire()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	// Safe zone rule: if hero is inside any NPC safe bubble, do not fire.
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OwnerActor))
	{
		if (Hero->IsInSafeZone())
		{
			return;
		}
	}

	UWorld* World = GetWorld();
	if (!World) return;

	FVector MyLoc = OwnerActor->GetActorLocation();
	const float RangeSq = AttackRange * AttackRange;

	auto SpawnProjectile = [&](const FVector& TargetLoc) -> bool
	{
		FVector SpawnLoc = MyLoc + FVector(0.f, 0.f, 50.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerActor;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AT66HeroProjectile* Proj = World->SpawnActor<AT66HeroProjectile>(AT66HeroProjectile::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (!Proj)
		{
			return false;
		}
		Proj->Damage = EffectiveDamagePerShot;
		Proj->SetScaleMultiplier(ProjectileScaleMultiplier);
		Proj->SetTargetLocation(TargetLoc);
		PlayShotSfx();
		return true;
	};

	// If we have a valid locked target in range and alive, prefer it.
	if (AActor* Locked = LockedTarget.Get())
	{
		const float DistSq = FVector::DistSquared(MyLoc, Locked->GetActorLocation());
		if (DistSq <= RangeSq)
		{
			// Accept enemies/bosses only if alive.
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Locked))
			{
				if (E->CurrentHP > 0)
				{
					(void)SpawnProjectile(Locked->GetActorLocation());
					return;
				}
				// Dead: clear lock.
				ClearLockedTarget();
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Locked))
			{
				if (B->IsAwakened() && B->IsAlive())
				{
					(void)SpawnProjectile(Locked->GetActorLocation());
					return;
				}
				ClearLockedTarget();
			}
			else if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(Locked))
			{
				if (GB->CurrentHP > 0)
				{
					(void)SpawnProjectile(Locked->GetActorLocation());
					return;
				}
				ClearLockedTarget();
			}
		}
	}

	AT66EnemyBase* ClosestEnemy = nullptr;
	AT66BossBase* ClosestBoss = nullptr;
	AT66GamblerBoss* ClosestGamblerBoss = nullptr;
	float ClosestDistSq = RangeSq;

	// Prefer Gambler Boss if present
	for (TActorIterator<AT66GamblerBoss> It(World); It; ++It)
	{
		AT66GamblerBoss* Boss = *It;
		if (!Boss || Boss->CurrentHP <= 0) continue;
		const float DistSq = FVector::DistSquared(MyLoc, Boss->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestGamblerBoss = Boss;
		}
	}

	// Prefer awakened boss if present
	for (TActorIterator<AT66BossBase> It(World); It; ++It)
	{
		AT66BossBase* Boss = *It;
		if (!Boss || !Boss->IsAwakened() || !Boss->IsAlive()) continue;
		float DistSq = FVector::DistSquared(MyLoc, Boss->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestBoss = Boss;
		}
	}

	// Otherwise target nearest enemy
	for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
	{
		AT66EnemyBase* Enemy = *It;
		if (!Enemy || Enemy->CurrentHP <= 0) continue;
		float DistSq = FVector::DistSquared(MyLoc, Enemy->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestEnemy = Enemy;
		}
	}

	AActor* Target = ClosestGamblerBoss ? Cast<AActor>(ClosestGamblerBoss)
		: (ClosestBoss ? Cast<AActor>(ClosestBoss) : Cast<AActor>(ClosestEnemy));
	if (Target)
	{
		(void)SpawnProjectile(Target->GetActorLocation());
	}
}
