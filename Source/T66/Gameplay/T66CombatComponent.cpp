// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66VendorBoss.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66DamageLogSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

UT66CombatComponent::UT66CombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Optional SFX asset (must be imported to a SoundWave/SoundCue .uasset).
	ShotSfx = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Audio/SFX/Shot.Shot")));
	// Niagara particles for slash (arc) and jump. Add User.Tint (Linear Color) in the system for color override.
	SlashVFXNiagara = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/VFX/VFX_Attack1.VFX_Attack1")));
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
		CachedRunState->DevCheatsChanged.AddDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
	}

	RecomputeFromRunState();

	// Preload optional shot SFX once (avoid sync loads in combat loop).
	CachedShotSfx = ShotSfx.LoadSynchronous();
	// Preload Niagara slash VFX.
	CachedSlashVFXNiagara = SlashVFXNiagara.LoadSynchronous();

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, EffectiveFireIntervalSeconds, true, EffectiveFireIntervalSeconds);
}

void UT66CombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}

	if (CachedRunState)
	{
		CachedRunState->InventoryChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->HeroProgressChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->SurvivalChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
		CachedRunState->DevCheatsChanged.RemoveDynamic(this, &UT66CombatComponent::HandleInventoryChanged);
	}

	Super::EndPlay(EndPlayReason);
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

	// Keep the hero's range ring in sync (attack range changes with Scale).
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(GetOwner()))
	{
		Hero->RefreshAttackRangeRing();
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

	// Per HUD/combat spec: Scale stat affects attack range (larger scale = larger range).
	AttackRange = FMath::Clamp(BaseAttackRange * FMath::Max(0.1f, TotalScale), 200.f, 50000.f);

	EffectiveFireIntervalSeconds = FMath::Clamp(BaseFireIntervalSeconds / FMath::Max(0.01f, TotalAttackSpeed), 0.05f, 10.f);
	EffectiveDamagePerShot = FMath::Clamp(FMath::RoundToInt(static_cast<float>(BaseDamagePerShot) * TotalDamage), 1, 999999);
	ProjectileScaleMultiplier = FMath::Clamp(TotalScale, 0.1f, 10.f);

	// Dev Power toggle: auto-attack does absurd damage.
	if (CachedRunState->GetDevPowerEnabled())
	{
		EffectiveDamagePerShot = 999999;
	}
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

	USoundBase* Sound = CachedShotSfx;
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

	auto ComputeSlashTint = [&]() -> FLinearColor
	{
		// Default: red.
		FLinearColor C(1.f, 0.2f, 0.2f, 1.f);
		if (!CachedRunState)
		{
			return C;
		}
		// Use first equipped idol as the slash tint.
		for (const FName& IdolID : CachedRunState->GetEquippedIdols())
		{
			if (!IdolID.IsNone())
			{
				C = UT66RunStateSubsystem::GetIdolColor(IdolID);
				C.A = 1.f;
				return C;
			}
		}
		return C;
	};

	// --- AoE Slash ---
	// Finds the primary target (same priority as before), then deals damage to it
	// plus any other enemies/bosses within SlashRadius centered on the target.
	auto PerformSlash = [&](AActor* PrimaryTarget) -> bool
	{
		if (!PrimaryTarget) return false;

		const FVector SlashCenter = PrimaryTarget->GetActorLocation();
		const float EffectiveSlashRadius = SlashRadius * ProjectileScaleMultiplier;

		// 1. Always damage the primary target.
		ApplyDamageToActor(PrimaryTarget, EffectiveDamagePerShot);

		// 2. Splash: overlap sphere around the target, hit nearby enemies/bosses.
		TArray<FOverlapResult> Overlaps;
		FCollisionShape Shape = FCollisionShape::MakeSphere(EffectiveSlashRadius);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);       // don't hit the hero
		Params.AddIgnoredActor(PrimaryTarget);     // already damaged above

		World->OverlapMultiByChannel(Overlaps, SlashCenter, FQuat::Identity,
			ECC_Pawn, Shape, Params);

		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Hit = Overlap.GetActor();
			if (Hit)
			{
				ApplyDamageToActor(Hit, EffectiveDamagePerShot);
			}
		}

		// 3. VFX + SFX
		SpawnSlashVFX(SlashCenter, EffectiveSlashRadius, ComputeSlashTint());
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
					(void)PerformSlash(Locked);
					return;
				}
				// Dead: clear lock.
				ClearLockedTarget();
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Locked))
			{
				if (B->IsAwakened() && B->IsAlive())
				{
					(void)PerformSlash(Locked);
					return;
				}
				ClearLockedTarget();
			}
			else if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(Locked))
			{
				if (GB->CurrentHP > 0)
				{
					(void)PerformSlash(Locked);
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
		(void)PerformSlash(Target);
	}
}

// ---------------------------------------------------------------------------
// ApplyDamageToActor — dispatches to the correct TakeDamage method per type.
// ---------------------------------------------------------------------------
void UT66CombatComponent::ApplyDamageToActor(AActor* Target, int32 DamageAmount)
{
	if (!Target) return;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_AutoAttack;

	if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
	{
		if (E->CurrentHP > 0)
		{
			E->TakeDamageFromHero(DamageAmount, SourceID);
		}
	}
	else if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(Target))
	{
		if (GB->CurrentHP > 0)
		{
			GB->TakeDamageFromHeroHit(DamageAmount, SourceID);
		}
	}
	else if (AT66VendorBoss* VB = Cast<AT66VendorBoss>(Target))
	{
		if (VB->CurrentHP > 0)
		{
			VB->TakeDamageFromHeroHit(DamageAmount, SourceID);
		}
	}
	else if (AT66BossBase* B = Cast<AT66BossBase>(Target))
	{
		if (B->IsAwakened() && B->IsAlive())
		{
			B->TakeDamageFromHeroHit(DamageAmount, SourceID);
		}
	}
}

// ---------------------------------------------------------------------------
// SpawnSlashVFX — spawns Niagara particles in an arc around the slash center.
// Uses VFX_Attack1; set User.Tint (Linear Color) in the Niagara system to drive color.
// ---------------------------------------------------------------------------
void UT66CombatComponent::SpawnSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	if (!World || !CachedSlashVFXNiagara) return;

	const int32 NumParticles = 10;
	const float ArcAngleDeg = 180.f;  // half circle
	const float StartAngleDeg = -ArcAngleDeg * 0.5f;
	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < NumParticles; ++i)
	{
		const float T = (NumParticles > 1) ? (static_cast<float>(i) / static_cast<float>(NumParticles - 1)) : 0.5f;
		const float AngleRad = FMath::DegreesToRadians(StartAngleDeg + ArcAngleDeg * T);
		const FVector Offset(FMath::Cos(AngleRad) * Radius, FMath::Sin(AngleRad) * Radius, 0.f);
		const FVector SpawnLoc = Location + Offset;

		UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, CachedSlashVFXNiagara, SpawnLoc, FRotator::ZeroRotator,
			FVector(1.f), true, true, ENCPoolMethod::None);
		if (NC)
		{
			NC->SetNiagaraVariableVec4(FString(TEXT("User.Tint")), ColorVec);
		}
	}
}
