// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66VendorBoss.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66GameInstance.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Data/T66DataTypes.h"
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
		CachedRunState->SetDOTDamageApplier([this](AActor* Target, int32 Damage, FName SourceIdolID)
		{
			ApplyDamageToActor(Target, Damage, NAME_None, SourceIdolID);
		});
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
	// Use the DataTable base range (hero-specific) instead of the C++ default.
	const float DataTableRange = CachedRunState->GetHeroBaseAttackRange();
	const float EffectiveBaseRange = (DataTableRange > 0.f) ? DataTableRange : BaseAttackRange;
	AttackRange = FMath::Clamp(EffectiveBaseRange * FMath::Max(0.1f, TotalScale), 200.f, 50000.f);

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

float UT66CombatComponent::GetAutoAttackCooldownProgress() const
{
	UWorld* World = GetWorld();
	if (!World || EffectiveFireIntervalSeconds <= 0.f) return 1.f;
	const float Now = static_cast<float>(World->GetTimeSeconds());
	const float Elapsed = Now - LastFireTime;
	return FMath::Clamp(Elapsed / EffectiveFireIntervalSeconds, 0.f, 1.f);
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

	LastFireTime = static_cast<float>(World->GetTimeSeconds());

	FVector MyLoc = OwnerActor->GetActorLocation();
	const float RangeSq = AttackRange * AttackRange;

	auto IsValidAutoTarget = [](AActor* A) -> bool
	{
		if (!A) return false;
		if (AT66EnemyBase* E = Cast<AT66EnemyBase>(A)) return E->CurrentHP > 0;
		if (AT66BossBase* B = Cast<AT66BossBase>(A)) return B->IsAwakened() && B->IsAlive();
		if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(A)) return GB->CurrentHP > 0;
		return false;
	};

	// Hero attack category (Pierce/Bounce/AOE/DOT) and data for Bounce/DOT params.
	ET66AttackCategory AttackCategory = ET66AttackCategory::AOE;
	FHeroData HeroDataForPrimary;
	bool bHaveHeroData = false;
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OwnerActor))
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(World->GetGameInstance()))
		{
			FHeroData HeroDataOut;
			if (GI->GetHeroData(Hero->HeroID, HeroDataOut))
			{
				AttackCategory = HeroDataOut.PrimaryCategory;
				HeroDataForPrimary = HeroDataOut;
				bHaveHeroData = true;
			}
		}
	}

	// --- Pierce (straight line): full range so enemies behind the first are hit; 10% damage reduction per pierced target. ---
	auto PerformPierce = [&](AActor* PrimaryTarget) -> bool
	{
		if (!PrimaryTarget) return false;

		const FVector TargetLoc = PrimaryTarget->GetActorLocation();
		const FVector Dir = (TargetLoc - MyLoc).GetSafeNormal();
		// Use full attack range so the line extends through the primary target and hits everyone behind.
		const float LineLength = AttackRange;
		const FVector EndPoint = MyLoc + Dir * LineLength;

		const float PierceRadius = 80.f * ProjectileScaleMultiplier;
		const float HalfLen = FMath::Max(1.f, (LineLength * 0.5f) - PierceRadius);
		const FVector MidPoint = MyLoc + Dir * (LineLength * 0.5f);
		const FQuat Rot = FQuat::FindBetween(FVector::UpVector, Dir);
		const FCollisionShape Cap = FCollisionShape::MakeCapsule(PierceRadius, HalfLen);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(Overlaps, MidPoint, Rot, ECC_Pawn, Cap, Params);

		// Collect all valid damageable actors in the line (primary + overlaps), then sort by distance for consistent pierce order.
		TArray<AActor*> InLine;
		InLine.Add(PrimaryTarget);
		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (A && A != PrimaryTarget && (Cast<AT66EnemyBase>(A) || Cast<AT66BossBase>(A) || Cast<AT66GamblerBoss>(A)))
			{
				InLine.AddUnique(A);
			}
		}
		// Sort by distance from hero so first in line = index 0 (100%), second = 90%, etc.
		InLine.Sort([&MyLoc](const AActor& A, const AActor& B)
		{
			return FVector::DistSquared(MyLoc, A.GetActorLocation()) < FVector::DistSquared(MyLoc, B.GetActorLocation());
		});

		// Apply damage with 10% reduction per enemy pierced (first = 100%, second = 90%, third = 80%, ...; floor at 10%).
		for (int32 i = 0; i < InLine.Num(); ++i)
		{
			const float Multiplier = FMath::Max(0.1f, 1.f - 0.1f * static_cast<float>(i));
			const int32 Damage = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * Multiplier));
			ApplyDamageToActor(InLine[i], Damage, NAME_None);
		}

		// VFX: hero primary is white.
		const FVector VFXEnd = TargetLoc + Dir * (LineLength * 0.5f);
		SpawnPierceVFX(TargetLoc, VFXEnd, FLinearColor::White);
		PlayShotSfx();
		return true;
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
		ApplyDamageToActor(PrimaryTarget, EffectiveDamagePerShot, NAME_None);

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
				ApplyDamageToActor(Hit, EffectiveDamagePerShot, NAME_None);
			}
		}

		// 3. VFX + SFX (hero primary = white)
		SpawnSlashVFX(SlashCenter, EffectiveSlashRadius, FLinearColor::White);
		PlayShotSfx();
		return true;
	};

	// --- Bounce: one projectile that bounces to nearest valid enemy each step. Each step uses AttackRange centered on the last hit enemy. ---
	auto PerformBounce = [&](AActor* PrimaryTarget) -> bool
	{
		if (!PrimaryTarget) return false;
		const int32 BounceCount = FMath::Max(1, bHaveHeroData ? HeroDataForPrimary.BaseBounceCount : 1);
		const float Falloff = FMath::Clamp(bHaveHeroData ? HeroDataForPrimary.FalloffPerHit : 0.f, 0.f, 0.95f);
		const float BounceRangeSq = AttackRange * AttackRange;
		const FVector PrimaryLoc = PrimaryTarget->GetActorLocation();
		TArray<AActor*> Chain;
		TArray<FVector> ChainPositions;
		Chain.Add(PrimaryTarget);
		ChainPositions.Add(MyLoc);
		ChainPositions.Add(PrimaryLoc);
		ApplyDamageToActor(PrimaryTarget, EffectiveDamagePerShot, NAME_None);
		FVector CurrentLoc = PrimaryLoc;
		TSet<AActor*> HitSet;
		HitSet.Add(PrimaryTarget);
		int32 BouncesLeft = BounceCount - 1;
		float DamageMult = 1.f - Falloff;
		while (BouncesLeft > 0)
		{
			AActor* Next = nullptr;
			float BestDistSq = BounceRangeSq;
			auto Consider = [&](AActor* A)
			{
				if (!A || HitSet.Contains(A)) return;
				const float DistSq = FVector::DistSquared(CurrentLoc, A->GetActorLocation());
				if (DistSq < BestDistSq && IsValidAutoTarget(A)) { BestDistSq = DistSq; Next = A; }
			};
			for (TActorIterator<AT66EnemyBase> It(World); It; ++It) Consider(Cast<AActor>(*It));
			for (TActorIterator<AT66BossBase> It(World); It; ++It) { AT66BossBase* B = *It; if (B && B->IsAwakened() && B->IsAlive()) Consider(Cast<AActor>(B)); }
			for (TActorIterator<AT66GamblerBoss> It(World); It; ++It) { AT66GamblerBoss* GB = *It; if (GB && GB->CurrentHP > 0) Consider(Cast<AActor>(GB)); }
			if (!Next) break;
			Chain.Add(Next);
			ChainPositions.Add(Next->GetActorLocation());
			HitSet.Add(Next);
			ApplyDamageToActor(Next, FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * DamageMult)), NAME_None);
			CurrentLoc = Next->GetActorLocation();
			DamageMult *= (1.f - Falloff);
			--BouncesLeft;
		}
		SpawnBounceVFX(ChainPositions, FLinearColor::White);
		PlayShotSfx();
		return true;
	};

	// --- DOT: one projectile to primary target — a bit of initial damage then DOT over time. ---
	auto PerformDOT = [&](AActor* PrimaryTarget) -> bool
	{
		if (!PrimaryTarget || !CachedRunState) return false;
		const float Duration = (bHaveHeroData && HeroDataForPrimary.DotDuration > 0.f) ? HeroDataForPrimary.DotDuration : 3.f;
		const float TickInterval = (bHaveHeroData && HeroDataForPrimary.DotTickInterval > 0.f) ? HeroDataForPrimary.DotTickInterval : 0.5f;
		const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
		const int32 InitialDamage = FMath::Max(1, EffectiveDamagePerShot / 2);
		const float DotTotalDamage = static_cast<float>(FMath::Max(1, EffectiveDamagePerShot - InitialDamage));
		const float DamagePerTick = DotTotalDamage / static_cast<float>(Ticks);
		ApplyDamageToActor(PrimaryTarget, InitialDamage, NAME_None);
		CachedRunState->ApplyDOT(PrimaryTarget, Duration, TickInterval, DamagePerTick, NAME_None);
		SpawnDOTVFX(PrimaryTarget->GetActorLocation(), Duration, 80.f, FLinearColor::White);
		PlayShotSfx();
		return true;
	};

	// Resolve primary target (locked > cached > closest).
	AActor* PrimaryTarget = nullptr;
	if (AActor* Locked = LockedTarget.Get())
	{
		const float DistSq = FVector::DistSquared(MyLoc, Locked->GetActorLocation());
		if (DistSq <= RangeSq && IsValidAutoTarget(Locked))
			PrimaryTarget = Locked;
		else
			ClearLockedTarget();
	}
	if (!PrimaryTarget)
	{
		AActor* Cached = CachedAutoTarget.Get();
		if (Cached)
		{
			const float DistSq = FVector::DistSquared(MyLoc, Cached->GetActorLocation());
			if (DistSq <= RangeSq && IsValidAutoTarget(Cached) && FiresSinceLastTargetRefresh < TargetRevalidateEveryNFires)
			{
				PrimaryTarget = Cached;
				++FiresSinceLastTargetRefresh;
			}
			else
				CachedAutoTarget.Reset();
		}
	}
	if (!PrimaryTarget)
	{
		AT66EnemyBase* ClosestEnemy = nullptr;
		AT66BossBase* ClosestBoss = nullptr;
		AT66GamblerBoss* ClosestGamblerBoss = nullptr;
		float ClosestDistSq = RangeSq;
		for (TActorIterator<AT66GamblerBoss> It(World); It; ++It)
		{
			AT66GamblerBoss* Boss = *It;
			if (!Boss || Boss->CurrentHP <= 0) continue;
			const float DistSq = FVector::DistSquared(MyLoc, Boss->GetActorLocation());
			if (DistSq < ClosestDistSq) { ClosestDistSq = DistSq; ClosestGamblerBoss = Boss; }
		}
		for (TActorIterator<AT66BossBase> It(World); It; ++It)
		{
			AT66BossBase* Boss = *It;
			if (!Boss || !Boss->IsAwakened() || !Boss->IsAlive()) continue;
			const float DistSq = FVector::DistSquared(MyLoc, Boss->GetActorLocation());
			if (DistSq < ClosestDistSq) { ClosestDistSq = DistSq; ClosestBoss = Boss; }
		}
		for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
		{
			AT66EnemyBase* Enemy = *It;
			if (!Enemy || Enemy->CurrentHP <= 0) continue;
			const float DistSq = FVector::DistSquared(MyLoc, Enemy->GetActorLocation());
			if (DistSq < ClosestDistSq) { ClosestDistSq = DistSq; ClosestEnemy = Enemy; }
		}
		AActor* Target = ClosestGamblerBoss ? Cast<AActor>(ClosestGamblerBoss) : (ClosestBoss ? Cast<AActor>(ClosestBoss) : Cast<AActor>(ClosestEnemy));
		if (Target) { CachedAutoTarget = Target; FiresSinceLastTargetRefresh = 0; PrimaryTarget = Target; }
	}

	// Hero primary attack (Pierce / Bounce / AOE / DOT; VFX white).
	if (PrimaryTarget)
	{
		switch (AttackCategory)
		{
		case ET66AttackCategory::Pierce: (void)PerformPierce(PrimaryTarget); break;
		case ET66AttackCategory::Bounce: (void)PerformBounce(PrimaryTarget); break;
		case ET66AttackCategory::AOE:   (void)PerformSlash(PrimaryTarget); break;
		case ET66AttackCategory::DOT:   (void)PerformDOT(PrimaryTarget); break;
		default: (void)PerformSlash(PrimaryTarget); break;
		}

		// Idol attacks: one per equipped idol, each with unique color.
		UT66GameInstance* GI = Cast<UT66GameInstance>(World->GetGameInstance());
		if (CachedRunState && GI)
		{
			const TArray<FName>& Idols = CachedRunState->GetEquippedIdols();
			const float IdolRange = AttackRange;
			// Bounce search radius = hero attack range, centered on the last hit enemy each step.
			const float BounceSearchRadius = IdolRange;

			for (int32 Slot = 0; Slot < Idols.Num(); ++Slot)
			{
				const FName IdolID = Idols[Slot];
				if (IdolID.IsNone()) continue;
				const int32 Level = CachedRunState->GetEquippedIdolLevelInSlot(Slot);
				FIdolData IdolData;
				if (!GI->GetIdolData(IdolID, IdolData)) continue;

				FLinearColor IdolColor = UT66RunStateSubsystem::GetIdolColor(IdolID);
				IdolColor.A = 1.f;
				const int32 IdolDamage = FMath::Max(1, FMath::RoundToInt(IdolData.GetDamageAtLevel(Level)));
				const FVector PrimaryLoc = PrimaryTarget->GetActorLocation();

				switch (IdolData.Category)
				{
				case ET66AttackCategory::Pierce:
				{
					const FVector Dir = (PrimaryLoc - MyLoc).GetSafeNormal();
					const float LineLength = IdolRange;
					const float PierceRadius = 60.f;
					const float HalfLen = FMath::Max(1.f, (LineLength * 0.5f) - PierceRadius);
					const FVector MidPoint = MyLoc + Dir * (LineLength * 0.5f);
					const FQuat Rot = FQuat::FindBetween(FVector::UpVector, Dir);
					FCollisionQueryParams Params;
					Params.AddIgnoredActor(OwnerActor);
					TArray<FOverlapResult> Overlaps;
					World->OverlapMultiByChannel(Overlaps, MidPoint, Rot, ECC_Pawn, FCollisionShape::MakeCapsule(PierceRadius, HalfLen), Params);
					TArray<AActor*> InLine;
					InLine.Add(PrimaryTarget);
					for (const FOverlapResult& O : Overlaps)
					{
						AActor* A = O.GetActor();
						if (A && A != PrimaryTarget && (Cast<AT66EnemyBase>(A) || Cast<AT66BossBase>(A) || Cast<AT66GamblerBoss>(A)))
							InLine.AddUnique(A);
					}
					InLine.Sort([&MyLoc](const AActor& a, const AActor& b) { return FVector::DistSquared(MyLoc, a.GetActorLocation()) < FVector::DistSquared(MyLoc, b.GetActorLocation()); });
					for (int32 i = 0; i < InLine.Num(); ++i)
					{
						const float Mult = FMath::Max(0.1f, 1.f - 0.1f * static_cast<float>(i));
						ApplyDamageToActor(InLine[i], FMath::Max(1, FMath::RoundToInt(IdolDamage * Mult)), NAME_None, IdolID);
					}
					SpawnPierceVFX(PrimaryLoc, PrimaryLoc + Dir * (LineLength * 0.5f), IdolColor);
					break;
				}
				case ET66AttackCategory::AOE:
				{
					const float Radius = FMath::Max(50.f, IdolData.GetPropertyAtLevel(Level));
					ApplyDamageToActor(PrimaryTarget, IdolDamage, NAME_None, IdolID);
					FCollisionQueryParams Params;
					Params.AddIgnoredActor(OwnerActor);
					Params.AddIgnoredActor(PrimaryTarget);
					TArray<FOverlapResult> Overlaps;
					World->OverlapMultiByChannel(Overlaps, PrimaryLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);
					for (const FOverlapResult& O : Overlaps)
					{
						if (AActor* Hit = O.GetActor())
							ApplyDamageToActor(Hit, IdolDamage, NAME_None, IdolID);
					}
					SpawnSlashVFX(PrimaryLoc, Radius, IdolColor);
					break;
				}
				case ET66AttackCategory::Bounce:
				{
					// BounceCount = number of jumps FROM the primary target to other enemies.
					// At level 1 (BaseProperty=1) the bolt bounces once (hits 1 extra enemy).
					const int32 BounceCount = FMath::Max(1, FMath::RoundToInt(IdolData.GetPropertyAtLevel(Level)));
					const float Falloff = FMath::Clamp(IdolData.FalloffPerHit, 0.f, 0.95f);
					const float BounceRangeSq = BounceSearchRadius * BounceSearchRadius;

					// Damage the primary target (auto-attack already hit it; idol adds extra damage).
					ApplyDamageToActor(PrimaryTarget, IdolDamage, NAME_None, IdolID);

					// Chain starts FROM the primary target (not from the hero).
					// VFX will only show enemy→enemy segments so it looks like a bolt bouncing off.
					TArray<FVector> ChainPositions;
					ChainPositions.Add(PrimaryLoc);

					FVector CurrentLoc = PrimaryLoc;
					TSet<AActor*> HitSet;
					HitSet.Add(PrimaryTarget);
					int32 BouncesLeft = BounceCount;
					float DamageMult = 1.f - Falloff;

					while (BouncesLeft > 0)
					{
						AActor* Next = nullptr;
						float BestDistSq = BounceRangeSq;
						auto Consider = [&](AActor* A)
						{
							if (!A || HitSet.Contains(A)) return;
							const float DistSq = FVector::DistSquared(CurrentLoc, A->GetActorLocation());
							if (DistSq < BestDistSq && IsValidAutoTarget(A)) { BestDistSq = DistSq; Next = A; }
						};
						for (TActorIterator<AT66EnemyBase> It(World); It; ++It) Consider(Cast<AActor>(*It));
						for (TActorIterator<AT66BossBase> It(World); It; ++It) { AT66BossBase* B = *It; if (B && B->IsAwakened() && B->IsAlive()) Consider(Cast<AActor>(B)); }
						for (TActorIterator<AT66GamblerBoss> It(World); It; ++It) { AT66GamblerBoss* GB = *It; if (GB && GB->CurrentHP > 0) Consider(Cast<AActor>(GB)); }
						if (!Next) break;
						ChainPositions.Add(Next->GetActorLocation());
						HitSet.Add(Next);
						ApplyDamageToActor(Next, FMath::Max(1, FMath::RoundToInt(IdolDamage * DamageMult)), NAME_None, IdolID);
						CurrentLoc = Next->GetActorLocation();
						DamageMult *= (1.f - Falloff);
						--BouncesLeft;
					}
					SpawnBounceVFX(ChainPositions, IdolColor);
					break;
				}
				case ET66AttackCategory::DOT:
				{
					const float Duration = FMath::Max(0.5f, IdolData.GetPropertyAtLevel(Level));
					const float TickInterval = FMath::Max(0.1f, IdolData.DotTickInterval);
					const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
					const float DamagePerTick = static_cast<float>(IdolDamage) / static_cast<float>(Ticks);
					CachedRunState->ApplyDOT(PrimaryTarget, Duration, TickInterval, DamagePerTick, IdolID);
					SpawnDOTVFX(PrimaryLoc, Duration, 80.f, IdolColor);
					break;
				}
				default:
					break;
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------
// ApplyDamageToActor — dispatches to the correct TakeDamage method per type.
// ---------------------------------------------------------------------------
void UT66CombatComponent::ApplyDamageToActor(AActor* Target, int32 DamageAmount, FName EventType, FName SourceID)
{
	if (!Target) return;
	const FName ResolvedSource = SourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : SourceID;

	if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
	{
		if (E->CurrentHP > 0)
		{
			E->TakeDamageFromHero(DamageAmount, ResolvedSource, EventType);
		}
	}
	else if (AT66GamblerBoss* GB = Cast<AT66GamblerBoss>(Target))
	{
		if (GB->CurrentHP > 0)
		{
			GB->TakeDamageFromHeroHit(DamageAmount, ResolvedSource, EventType);
		}
	}
	else if (AT66VendorBoss* VB = Cast<AT66VendorBoss>(Target))
	{
		if (VB->CurrentHP > 0)
		{
			VB->TakeDamageFromHeroHit(DamageAmount, ResolvedSource, EventType);
		}
	}
	else if (AT66BossBase* B = Cast<AT66BossBase>(Target))
	{
		if (B->IsAwakened() && B->IsAlive())
		{
			B->TakeDamageFromHeroHit(DamageAmount, ResolvedSource, EventType);
		}
	}
}

// ---------------------------------------------------------------------------
// SpawnSlashVFX — spawns Niagara particles in a tight arc (many, small, clumped).
// Uses VFX_Attack1; set User.Tint (Linear Color) in the Niagara system to drive color.
// ---------------------------------------------------------------------------
void UT66CombatComponent::SpawnSlashVFX(const FVector& Location, float Radius, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	if (!World || !CachedSlashVFXNiagara) return;

	const int32 NumParticles = 32;
	const float ArcAngleDeg = 180.f;
	const float StartAngleDeg = -ArcAngleDeg * 0.5f;
	// Tighter spread: use 0.45f of radius so particles clump in the blade shape.
	const float SpreadScale = 0.45f;
	const float ParticleScale = 0.4f;
	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < NumParticles; ++i)
	{
		const float T = (NumParticles > 1) ? (static_cast<float>(i) / static_cast<float>(NumParticles - 1)) : 0.5f;
		const float AngleRad = FMath::DegreesToRadians(StartAngleDeg + ArcAngleDeg * T);
		const FVector Offset(FMath::Cos(AngleRad) * Radius * SpreadScale, FMath::Sin(AngleRad) * Radius * SpreadScale, 0.f);
		const FVector SpawnLoc = Location + Offset;

		UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, CachedSlashVFXNiagara, SpawnLoc, FRotator::ZeroRotator,
			FVector(ParticleScale), true, true, ENCPoolMethod::None);
		if (NC)
		{
			NC->SetVariableVec4(FName(TEXT("User.Tint")), ColorVec);
		}
	}
}

// ---------------------------------------------------------------------------
// SpawnPierceVFX — spawns Niagara particles along a straight line (pierce attack).
// ---------------------------------------------------------------------------
void UT66CombatComponent::SpawnPierceVFX(const FVector& Start, const FVector& End, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	if (!World || !CachedSlashVFXNiagara) return;

	const int32 NumParticles = 28;
	const float ParticleScale = 0.35f;
	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);

	for (int32 i = 0; i < NumParticles; ++i)
	{
		const float T = (NumParticles > 1) ? (static_cast<float>(i) / static_cast<float>(NumParticles - 1)) : 0.5f;
		const FVector SpawnLoc = FMath::Lerp(Start, End, T);

		UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, CachedSlashVFXNiagara, SpawnLoc, FRotator::ZeroRotator,
			FVector(ParticleScale), true, true, ENCPoolMethod::None);
		if (NC)
		{
			NC->SetVariableVec4(FName(TEXT("User.Tint")), ColorVec);
		}
	}
}

void UT66CombatComponent::SpawnBounceVFX(const TArray<FVector>& ChainPositions, const FLinearColor& /*Color*/)
{
	UWorld* World = GetWorld();
	if (!World || !CachedSlashVFXNiagara || ChainPositions.Num() < 2) return;

	// All bounce idols use a bright-red bolt so the bounce is unmistakable.
	static const FVector4 RedTint(1.f, 0.1f, 0.1f, 1.f);

	const float TrailScale  = 0.6f;   // per-particle along the trail
	const float ImpactScale = 1.0f;   // burst at each contact point

	for (int32 i = 0; i < ChainPositions.Num() - 1; ++i)
	{
		const FVector Start = ChainPositions[i];
		const FVector End   = ChainPositions[i + 1];
		const FVector Dir   = (End - Start).GetSafeNormal();
		if (Dir.IsNearlyZero()) continue;
		const FRotator Rot  = Dir.Rotation();
		const float    Dist = FVector::Dist(Start, End);

		// Dense trail of red particles along the segment.
		const int32 Num = FMath::Max(10, FMath::RoundToInt(Dist / 30.f));
		for (int32 j = 0; j < Num; ++j)
		{
			const float T = (Num > 1) ? (static_cast<float>(j) / static_cast<float>(Num - 1)) : 0.5f;
			UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World, CachedSlashVFXNiagara, FMath::Lerp(Start, End, T), Rot,
				FVector(TrailScale), true, true, ENCPoolMethod::None);
			if (NC) NC->SetVariableVec4(FName(TEXT("User.Tint")), RedTint);
		}

		// Larger impact burst at the START of each segment (the contact/bounce point).
		UNiagaraComponent* Impact = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, CachedSlashVFXNiagara, Start, FRotator::ZeroRotator,
			FVector(ImpactScale), true, true, ENCPoolMethod::None);
		if (Impact) Impact->SetVariableVec4(FName(TEXT("User.Tint")), RedTint);
	}
}

void UT66CombatComponent::SpawnDOTVFX(const FVector& Location, float Duration, float Radius, const FLinearColor& Color)
{
	UWorld* World = GetWorld();
	if (!World || !CachedSlashVFXNiagara) return;

	const FVector4 ColorVec(Color.R, Color.G, Color.B, Color.A);
	const float ParticleScale = 0.25f;
	UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World, CachedSlashVFXNiagara, Location, FRotator::ZeroRotator,
		FVector(ParticleScale), true, true, ENCPoolMethod::None);
	if (NC)
	{
		NC->SetVariableVec4(FName(TEXT("User.Tint")), ColorVec);
		NC->SetAutoDestroy(true);
		if (AActor* Owner = NC->GetOwner())
			Owner->SetLifeSpan(FMath::Max(0.1f, Duration));
	}
}
