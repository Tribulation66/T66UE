// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66GameInstance.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Data/T66DataTypes.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66FloatingCombatTextSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
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

// ---------------------------------------------------------------------------
// BeginPlay — set up delegates, range sphere, fire timer.
// ---------------------------------------------------------------------------
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

	// --- Create the range detection sphere ---
	AActor* Owner = GetOwner();
	if (Owner)
	{
		RangeSphere = NewObject<USphereComponent>(Owner, TEXT("CombatRangeSphere"));
		if (RangeSphere)
		{
			RangeSphere->SetupAttachment(Owner->GetRootComponent());
			RangeSphere->SetSphereRadius(AttackRange);
			RangeSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
			RangeSphere->SetGenerateOverlapEvents(true);
			RangeSphere->SetHiddenInGame(true);
			RangeSphere->SetCanEverAffectNavigation(false);
			// No physics response — purely a query volume.
			RangeSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
			RangeSphere->RegisterComponent();

			RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UT66CombatComponent::OnRangeBeginOverlap);
			RangeSphere->OnComponentEndOverlap.AddDynamic(this, &UT66CombatComponent::OnRangeEndOverlap);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this, &UT66CombatComponent::TryFire, EffectiveFireIntervalSeconds, true, EffectiveFireIntervalSeconds);

	UE_LOG(LogTemp, Log, TEXT("[GOLD] CombatComponent: initialized — overlap sphere (radius=%.0f), VFX pooling (AutoRelease), timer-based fire (%.2fs)"),
		AttackRange, EffectiveFireIntervalSeconds);
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

	// Clean up the sphere.
	if (RangeSphere)
	{
		RangeSphere->DestroyComponent();
		RangeSphere = nullptr;
	}
	EnemiesInRange.Empty();

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Overlap callbacks — maintain EnemiesInRange.
// ---------------------------------------------------------------------------
void UT66CombatComponent::OnRangeBeginOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/, bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
	if (!OtherActor) return;

	// Only track enemies and bosses (Gambler/Vendor are AT66BossBase).
	if (Cast<AT66EnemyBase>(OtherActor) || Cast<AT66BossBase>(OtherActor))
	{
		EnemiesInRange.AddUnique(OtherActor);
	}
}

void UT66CombatComponent::OnRangeEndOverlap(UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/)
{
	if (!OtherActor) return;
	EnemiesInRange.RemoveAll([OtherActor](const TWeakObjectPtr<AActor>& Weak) { return !Weak.IsValid() || Weak.Get() == OtherActor; });
}

// ---------------------------------------------------------------------------
// IsValidAutoTarget — returns true if the actor is alive and targetable.
// ---------------------------------------------------------------------------
bool UT66CombatComponent::IsValidAutoTarget(AActor* A)
{
	if (!A) return false;
	if (AT66EnemyBase* E = Cast<AT66EnemyBase>(A)) return E->CurrentHP > 0;
	if (AT66BossBase* B = Cast<AT66BossBase>(A)) return B->IsAwakened() && B->IsAlive();
	return false;
}

// ---------------------------------------------------------------------------
// FindClosestEnemyInRange — walks EnemiesInRange (small list) instead of all
// world actors.  Returns nullptr if nothing valid is in range.
// ---------------------------------------------------------------------------
AActor* UT66CombatComponent::FindClosestEnemyInRange(const FVector& FromLocation, float MaxRangeSq,
	const TSet<AActor*>* ExcludeSet) const
{
	AActor* Best = nullptr;
	float BestDistSq = MaxRangeSq;

	for (const TWeakObjectPtr<AActor>& Weak : EnemiesInRange)
	{
		AActor* A = Weak.Get();
		if (!A) continue;
		if (ExcludeSet && ExcludeSet->Contains(A)) continue;
		if (!IsValidAutoTarget(A)) continue;
		const float DistSq = FVector::DistSquared(FromLocation, A->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = A;
		}
	}
	return Best;
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

	// Update the detection sphere to match the new attack range.
	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
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

	const float HeroAttackSpeedMult = CachedRunState->GetHeroAttackSpeedMultiplier() * CachedRunState->GetLastStandAttackSpeedMultiplier() * CachedRunState->GetRallyAttackSpeedMultiplier();
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

	// Keep the detection sphere in sync whenever stats are recomputed.
	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
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

// ---------------------------------------------------------------------------
// TryFire — the hero auto-attack heartbeat.
// Target finding now walks the small EnemiesInRange list (maintained by sphere
// overlap events) instead of doing 3x TActorIterator over the entire world.
// ---------------------------------------------------------------------------
void UT66CombatComponent::TryFire()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FLagScopedScope LagScope(World, TEXT("CombatComponent::TryFire"));

	// Safe zone rule: if hero is inside any NPC safe bubble, do not fire.
	if (AT66HeroBase* Hero = Cast<AT66HeroBase>(OwnerActor))
	{
		if (Hero->IsInSafeZone())
		{
			return;
		}
	}

	LastFireTime = static_cast<float>(World->GetTimeSeconds());

	// Crit: roll per hit; multiply damage and pass EventType_Crit for floating text.
	auto ResolveCrit = [this](int32 BaseDamage) -> TPair<int32, FName>
	{
		if (!CachedRunState) return { BaseDamage, NAME_None };
		const float CritChance = CachedRunState->GetCritChance01();
		const bool bCrit = (CritChance > 0.f && FMath::FRand() < CritChance);
		if (bCrit)
		{
			const float Mult = CachedRunState->GetCritDamageMultiplier();
			return { FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult)), UT66FloatingCombatTextSubsystem::EventType_Crit };
		}
		return { BaseDamage, NAME_None };
	};

	FVector MyLoc = OwnerActor->GetActorLocation();

	// Close/Long range damage: multiply base damage by range-based multiplier (close = 0–10% of range, long = 90–100%).
	// OutRangeEvent set when in close/long zone and multiplier != 1 (for floating text on hero).
	auto GetRangeMultipliedDamage = [this, MyLoc](int32 BaseDamage, AActor* Target, FName* OutRangeEvent = nullptr) -> int32
	{
		if (!CachedRunState || !Target) return BaseDamage;
		const float Dist = FVector::Dist(MyLoc, Target->GetActorLocation());
		const float CloseThresh = CachedRunState->GetCloseRangeThreshold();
		const float LongThresh = CachedRunState->GetLongRangeThreshold();
		if (Dist <= CloseThresh)
		{
			const float Mult = CachedRunState->GetCloseRangeDamageMultiplier();
			if (OutRangeEvent && Mult != 1.f) *OutRangeEvent = UT66FloatingCombatTextSubsystem::EventType_CloseRange;
			return FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult));
		}
		if (Dist >= LongThresh)
		{
			const float Mult = CachedRunState->GetLongRangeDamageMultiplier();
			if (OutRangeEvent && Mult != 1.f) *OutRangeEvent = UT66FloatingCombatTextSubsystem::EventType_LongRange;
			return FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseDamage) * Mult));
		}
		return BaseDamage;
	};

	const float RangeSq = AttackRange * AttackRange;

	// Purge stale weak pointers (destroyed actors that didn't fire EndOverlap).
	EnemiesInRange.RemoveAll([](const TWeakObjectPtr<AActor>& Weak) { return !Weak.IsValid(); });

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
	auto PerformPierce = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget) return false;

		const FVector TargetLoc = PrimaryTarget->GetActorLocation();
		const FVector Dir = (TargetLoc - MyLoc).GetSafeNormal();
		const float LineLength = AttackRange;
		const float PierceRadius = 80.f * ProjectileScaleMultiplier;
		const float HalfLen = FMath::Max(1.f, (LineLength * 0.5f) - PierceRadius);
		const FVector MidPoint = MyLoc + Dir * (LineLength * 0.5f);
		const FQuat Rot = FQuat::FindBetween(FVector::UpVector, Dir);
		const FCollisionShape Cap = FCollisionShape::MakeCapsule(PierceRadius, HalfLen);

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);

		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(Overlaps, MidPoint, Rot, ECC_Pawn, Cap, Params);

		TArray<AActor*> InLine;
		InLine.Add(PrimaryTarget);
		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (A && A != PrimaryTarget && (Cast<AT66EnemyBase>(A) || Cast<AT66BossBase>(A)))
				InLine.AddUnique(A);
		}
		InLine.Sort([&MyLoc](const AActor& A, const AActor& B)
		{
			return FVector::DistSquared(MyLoc, A.GetActorLocation()) < FVector::DistSquared(MyLoc, B.GetActorLocation());
		});

		for (int32 i = 0; i < InLine.Num(); ++i)
		{
			const float Multiplier = FMath::Max(0.1f, 1.f - 0.1f * static_cast<float>(i));
			const float PrimaryMult = (i == 0) ? PrimaryDamageMult : 1.f;
			const int32 BaseDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * Multiplier * PrimaryMult));
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(BaseDmg, InLine[i], &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(InLine[i], Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}

		const FVector VFXEnd = TargetLoc + Dir * (LineLength * 0.5f);
		SpawnPierceVFX(TargetLoc, VFXEnd, FLinearColor::White);
		PlayShotSfx();
		return true;
	};

	// --- AoE Slash ---
	auto PerformSlash = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget) return false;

		const FVector SlashCenter = PrimaryTarget->GetActorLocation();
		const float EffectiveSlashRadius = SlashRadius * ProjectileScaleMultiplier;

		TArray<FOverlapResult> Overlaps;
		FCollisionShape Shape = FCollisionShape::MakeSphere(EffectiveSlashRadius);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(OwnerActor);
		Params.AddIgnoredActor(PrimaryTarget);
		World->OverlapMultiByChannel(Overlaps, SlashCenter, FQuat::Identity, ECC_Pawn, Shape, Params);

		const int32 HitCount = 1 + Overlaps.Num();
		float ArcaneMult = 1.f;
		if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::ArcaneAmplification)
		{
			if (HitCount >= 5) ArcaneMult = 1.35f;
			else if (HitCount >= 3) ArcaneMult = 1.2f;
		}

		const int32 PrimaryDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult * ArcaneMult));
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(PrimaryDmg, PrimaryTarget, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}

		const int32 SplashDmg = FMath::Max(1, FMath::RoundToInt(static_cast<float>(EffectiveDamagePerShot) * ArcaneMult));
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Hit = Overlap.GetActor();
			if (Hit)
			{
				FName RangeEvent;
				const int32 RangeDmg = GetRangeMultipliedDamage(SplashDmg, Hit, &RangeEvent);
				const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
				ApplyDamageToActor(Hit, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
			}
		}

		SpawnSlashVFX(SlashCenter, EffectiveSlashRadius, FLinearColor::White);
		PlayShotSfx();
		return true;
	};

	// --- Bounce ---
	auto PerformBounce = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
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
		const int32 PrimaryDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult));
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(PrimaryDmg, PrimaryTarget, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}
		FVector CurrentLoc = PrimaryLoc;
		TSet<AActor*> HitSet;
		HitSet.Add(PrimaryTarget);
		int32 BouncesLeft = BounceCount - 1;
		float DamageMult = 1.f - Falloff;
		while (BouncesLeft > 0)
		{
			AActor* Next = FindClosestEnemyInRange(CurrentLoc, BounceRangeSq, &HitSet);
			if (!Next) break;
			Chain.Add(Next);
			ChainPositions.Add(Next->GetActorLocation());
			HitSet.Add(Next);
			const int32 BounceDmg = FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * DamageMult));
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(BounceDmg, Next, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(Next, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
			CurrentLoc = Next->GetActorLocation();
			DamageMult *= (1.f - Falloff);
			--BouncesLeft;
		}
		SpawnBounceVFX(ChainPositions, FLinearColor::White);
		PlayShotSfx();
		return true;
	};

	// --- DOT ---
	auto PerformDOT = [&](AActor* PrimaryTarget, float PrimaryDamageMult) -> bool
	{
		if (!PrimaryTarget || !CachedRunState) return false;
		const float Duration = (bHaveHeroData && HeroDataForPrimary.DotDuration > 0.f) ? HeroDataForPrimary.DotDuration : 3.f;
		const float TickInterval = (bHaveHeroData && HeroDataForPrimary.DotTickInterval > 0.f) ? HeroDataForPrimary.DotTickInterval : 0.5f;
		const int32 Ticks = FMath::Max(1, FMath::RoundToInt(Duration / TickInterval));
		const int32 InitialDamage = FMath::Max(1, FMath::RoundToInt(static_cast<float>(EffectiveDamagePerShot) * 0.5f * PrimaryDamageMult));
		const float DotTotalDamage = static_cast<float>(FMath::Max(1, FMath::RoundToInt(EffectiveDamagePerShot * PrimaryDamageMult) - InitialDamage));
		const float DamagePerTick = DotTotalDamage / static_cast<float>(Ticks);
		{
			FName RangeEvent;
			const int32 RangeDmg = GetRangeMultipliedDamage(InitialDamage, PrimaryTarget, &RangeEvent);
			const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
			ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, NAME_None, RangeEvent);
		}
		CachedRunState->ApplyDOT(PrimaryTarget, Duration, TickInterval, DamagePerTick, NAME_None);
		SpawnDOTVFX(PrimaryTarget->GetActorLocation(), Duration, 80.f, FLinearColor::White);
		PlayShotSfx();
		return true;
	};

	// ---------------------------------------------------------------------------
	// Resolve primary target: locked > closest in EnemiesInRange.
	// No TActorIterator — just walk the small overlap list.
	// ---------------------------------------------------------------------------
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
		PrimaryTarget = FindClosestEnemyInRange(MyLoc, RangeSq);
	}

	// Marksman's Focus (Archer): consecutive hits on same target stack +8% damage (max 5).
	float PrimaryDamageMultiplier = 1.f;
	if (CachedRunState && CachedRunState->GetPassiveType() == ET66PassiveType::MarksmanFocus && PrimaryTarget)
	{
		if (PrimaryTarget == LastMarksmanTarget.Get())
			MarksmanStacks = FMath::Min(5, MarksmanStacks + 1);
		else
			MarksmanStacks = 1;
		LastMarksmanTarget = PrimaryTarget;
		PrimaryDamageMultiplier = 1.f + 0.08f * static_cast<float>(MarksmanStacks);
	}

	// Hero primary attack (Pierce / Bounce / AOE / DOT; VFX white).
	if (PrimaryTarget)
	{
		switch (AttackCategory)
		{
		case ET66AttackCategory::Pierce: (void)PerformPierce(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::Bounce: (void)PerformBounce(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::AOE:   (void)PerformSlash(PrimaryTarget, PrimaryDamageMultiplier); break;
		case ET66AttackCategory::DOT:   (void)PerformDOT(PrimaryTarget, PrimaryDamageMultiplier); break;
		default: (void)PerformSlash(PrimaryTarget, PrimaryDamageMultiplier); break;
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
						if (A && A != PrimaryTarget && (Cast<AT66EnemyBase>(A) || Cast<AT66BossBase>(A)))
							InLine.AddUnique(A);
					}
					InLine.Sort([&MyLoc](const AActor& a, const AActor& b) { return FVector::DistSquared(MyLoc, a.GetActorLocation()) < FVector::DistSquared(MyLoc, b.GetActorLocation()); });
					for (int32 i = 0; i < InLine.Num(); ++i)
					{
						const float Mult = FMath::Max(0.1f, 1.f - 0.1f * static_cast<float>(i));
						const int32 Dmg = FMath::Max(1, FMath::RoundToInt(IdolDamage * Mult));
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(Dmg, InLine[i], &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(InLine[i], Resolved.Key, Resolved.Value, IdolID, RangeEvent);
					}
					SpawnPierceVFX(PrimaryLoc, PrimaryLoc + Dir * (LineLength * 0.5f), IdolColor);
					break;
				}
				case ET66AttackCategory::AOE:
				{
					const float Radius = FMath::Max(50.f, IdolData.GetPropertyAtLevel(Level));
					{
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, PrimaryTarget, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
					}
					FCollisionQueryParams Params;
					Params.AddIgnoredActor(OwnerActor);
					Params.AddIgnoredActor(PrimaryTarget);
					TArray<FOverlapResult> Overlaps;
					World->OverlapMultiByChannel(Overlaps, PrimaryLoc, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);
					for (const FOverlapResult& O : Overlaps)
					{
						if (AActor* Hit = O.GetActor())
						{
							FName RangeEvent;
							const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, Hit, &RangeEvent);
							const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
							ApplyDamageToActor(Hit, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
						}
					}
					SpawnSlashVFX(PrimaryLoc, Radius, IdolColor);
					break;
				}
				case ET66AttackCategory::Bounce:
				{
					// BounceCount = number of jumps FROM the primary target to other enemies.
					// At level 1 (BaseProperty=1) the bolt bounces once (hits 1 extra enemy).
					const int32 BounceCount = FMath::Max(1, FMath::RoundToInt(IdolData.GetPropertyAtLevel(Level)));
					const float IdolFalloff = FMath::Clamp(IdolData.FalloffPerHit, 0.f, 0.95f);
					const float BounceRangeSq = BounceSearchRadius * BounceSearchRadius;

					// Damage the primary target (auto-attack already hit it; idol adds extra damage).
					{
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(IdolDamage, PrimaryTarget, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(PrimaryTarget, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
					}

					// Chain starts FROM the primary target (not from the hero).
					// VFX will only show enemy->enemy segments so it looks like a bolt bouncing off.
					TArray<FVector> ChainPositions;
					ChainPositions.Add(PrimaryLoc);

					FVector CurrentLoc = PrimaryLoc;
					TSet<AActor*> HitSet;
					HitSet.Add(PrimaryTarget);
					int32 BouncesLeft = BounceCount;
					float IdolDamageMult = 1.f - IdolFalloff;

					while (BouncesLeft > 0)
					{
						AActor* Next = FindClosestEnemyInRange(CurrentLoc, BounceRangeSq, &HitSet);
						if (!Next) break;
						ChainPositions.Add(Next->GetActorLocation());
						HitSet.Add(Next);
						const int32 BounceDmg = FMath::Max(1, FMath::RoundToInt(IdolDamage * IdolDamageMult));
						FName RangeEvent;
						const int32 RangeDmg = GetRangeMultipliedDamage(BounceDmg, Next, &RangeEvent);
						const TPair<int32, FName> Resolved = ResolveCrit(RangeDmg);
						ApplyDamageToActor(Next, Resolved.Key, Resolved.Value, IdolID, RangeEvent);
						CurrentLoc = Next->GetActorLocation();
						IdolDamageMult *= (1.f - IdolFalloff);
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
void UT66CombatComponent::ApplyDamageToActor(AActor* Target, int32 DamageAmount, FName EventType, FName SourceID, FName RangeEventForHero)
{
	if (!Target) return;
	// Toxin Stacking (Rogue): enemies with active DOT take +15% damage from all sources.
	if (CachedRunState)
	{
		const float ToxinMult = CachedRunState->GetToxinStackingDamageMultiplier(Target);
		if (ToxinMult > 1.f)
			DamageAmount = FMath::Max(1, FMath::RoundToInt(static_cast<float>(DamageAmount) * ToxinMult));
	}
	const FName ResolvedSource = SourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : SourceID;
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66FloatingCombatTextSubsystem* FloatingText = GI ? GI->GetSubsystem<UT66FloatingCombatTextSubsystem>() : nullptr;
	AActor* Hero = GetOwner();

	if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
	{
		if (E->CurrentHP > 0)
		{
			E->TakeDamageFromHero(DamageAmount, ResolvedSource, EventType);
		}
	}
	else if (AT66BossBase* B = Cast<AT66BossBase>(Target))
	{
		if (B->IsAwakened() && B->IsAlive())
		{
			B->TakeDamageFromHeroHit(DamageAmount, ResolvedSource, EventType);
		}
	}

	// Life steal: % chance per hit; when it procs, heal 10% of damage dealt.
	if (DamageAmount > 0 && CachedRunState)
	{
		const float LsChance = FMath::Clamp(CachedRunState->GetLifeStealFraction(), 0.f, 1.f);
		if (LsChance > 0.f && FMath::FRand() < LsChance)
		{
			CachedRunState->HealHP(static_cast<float>(DamageAmount) * 0.1f);
			if (FloatingText && Hero) FloatingText->ShowStatusEvent(Hero, UT66FloatingCombatTextSubsystem::EventType_LifeSteal);
		}
	}

	// Invisibility: chance to confuse (apply confusion) the enemy we hit.
	if (Target && DamageAmount > 0 && CachedRunState)
	{
		const float InvisChance = CachedRunState->GetInvisibilityChance01();
		if (InvisChance > 0.f && FMath::FRand() < InvisChance)
		{
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
			{
				E->ApplyConfusion(3.f);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Confusion);
				if (FloatingText && Hero) FloatingText->ShowStatusEvent(Hero, UT66FloatingCombatTextSubsystem::EventType_Invisibility);
			}
		}
	}

	// Taunt: on hit, apply armor debuff to the target (enemy or boss) using Taunt stat.
	if (Target && DamageAmount > 0 && CachedRunState)
	{
		const float Aggro = CachedRunState->GetAggroMultiplier();
		if (Aggro > 1.f)
		{
			const float ReductionAmount = FMath::Min(0.5f, Aggro - 1.f);
			const float DurationSeconds = 3.f;
			if (AT66EnemyBase* E = Cast<AT66EnemyBase>(Target))
			{
				E->ApplyArmorDebuff(ReductionAmount, DurationSeconds);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Taunt);
			}
			else if (AT66BossBase* B = Cast<AT66BossBase>(Target))
			{
				B->ApplyArmorDebuff(ReductionAmount, DurationSeconds);
				if (FloatingText) FloatingText->ShowStatusEvent(Target, UT66FloatingCombatTextSubsystem::EventType_Taunt);
			}
		}
	}

	// Close/Long range bonus: show on hero when applicable.
	if (FloatingText && Hero && !RangeEventForHero.IsNone())
	{
		FloatingText->ShowStatusEvent(Hero, RangeEventForHero);
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
			FVector(ParticleScale), true, true, ENCPoolMethod::AutoRelease);
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
			FVector(ParticleScale), true, true, ENCPoolMethod::AutoRelease);
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
		const FVector ChainStart = ChainPositions[i];
		const FVector ChainEnd   = ChainPositions[i + 1];
		const FVector Dir   = (ChainEnd - ChainStart).GetSafeNormal();
		if (Dir.IsNearlyZero()) continue;
		const FRotator Rot  = Dir.Rotation();
		const float    Dist = FVector::Dist(ChainStart, ChainEnd);

		// Dense trail of red particles along the segment.
		const int32 Num = FMath::Max(10, FMath::RoundToInt(Dist / 30.f));
		for (int32 j = 0; j < Num; ++j)
		{
			const float T = (Num > 1) ? (static_cast<float>(j) / static_cast<float>(Num - 1)) : 0.5f;
			UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World, CachedSlashVFXNiagara, FMath::Lerp(ChainStart, ChainEnd, T), Rot,
				FVector(TrailScale), true, true, ENCPoolMethod::AutoRelease);
			if (NC) NC->SetVariableVec4(FName(TEXT("User.Tint")), RedTint);
		}

		// Larger impact burst at the START of each segment (the contact/bounce point).
		UNiagaraComponent* Impact = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, CachedSlashVFXNiagara, ChainStart, FRotator::ZeroRotator,
			FVector(ImpactScale), true, true, ENCPoolMethod::AutoRelease);
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
		FVector(ParticleScale), true, true, ENCPoolMethod::AutoRelease);
	if (NC)
	{
		NC->SetVariableVec4(FName(TEXT("User.Tint")), ColorVec);
		NC->SetAutoDestroy(true);
		if (AActor* Owner = NC->GetOwner())
			Owner->SetLifeSpan(FMath::Max(0.1f, Duration));
	}
}

void UT66CombatComponent::PerformUltimateSpearStorm(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const FRotator HeroRot = OwnerActor->GetActorRotation();
	const FVector Forward = HeroRot.Vector();
	const float LineLength = 1200.f;
	const int32 NumRays = 10;
	const float TubeRadius = 100.f;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(LineLength), Params);

	TSet<AActor*> AlreadyHit;
	for (int32 i = 0; i < NumRays; ++i)
	{
		const float AngleDeg = (360.f / NumRays) * i;
		const FVector Dir = Forward.RotateAngleAxis(AngleDeg, FVector::UpVector);
		const FVector Start = HeroLoc;
		const FVector End = HeroLoc + Dir * LineLength;

		for (const FOverlapResult& O : Overlaps)
		{
			AActor* A = O.GetActor();
			if (!A || !IsValidAutoTarget(A) || AlreadyHit.Contains(A)) continue;
			FVector Pt = A->GetActorLocation();
			const float DistSq = FMath::PointDistToSegmentSquared(Pt, Start, End);
			if (DistSq <= TubeRadius * TubeRadius)
			{
				AlreadyHit.Add(A);
				ApplyDamageToActor(A, UltimateDamage, NAME_None, SourceID);
			}
		}
		if (CachedSlashVFXNiagara)
			SpawnPierceVFX(Start, End, FLinearColor(0.9f, 0.85f, 0.3f, 1.f));
	}
}

void UT66CombatComponent::PerformUltimateChainLightning(int32 UltimateDamage)
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return;

	const FVector HeroLoc = OwnerActor->GetActorLocation();
	const float ChainRange = 1500.f;
	const float ChainRangeSq = ChainRange * ChainRange;
	const FName SourceID = UT66DamageLogSubsystem::SourceID_Ultimate;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	World->OverlapMultiByChannel(Overlaps, HeroLoc, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(ChainRange), Params);

	TArray<AActor*> ValidTargets;
	for (const FOverlapResult& O : Overlaps)
	{
		AActor* A = O.GetActor();
		if (A && IsValidAutoTarget(A))
			ValidTargets.Add(A);
	}

	TArray<FVector> ChainPositions;
	ChainPositions.Add(HeroLoc);
	FVector CurrentLoc = HeroLoc;
	TSet<AActor*> HitSet;

	while (ValidTargets.Num() > 0)
	{
		AActor* Nearest = nullptr;
		float BestDistSq = ChainRangeSq;
		for (AActor* A : ValidTargets)
		{
			if (HitSet.Contains(A)) continue;
			const float DistSq = FVector::DistSquared(CurrentLoc, A->GetActorLocation());
			if (DistSq < BestDistSq) { BestDistSq = DistSq; Nearest = A; }
		}
		if (!Nearest) break;
		HitSet.Add(Nearest);
		ValidTargets.Remove(Nearest);
		ApplyDamageToActor(Nearest, UltimateDamage, NAME_None, SourceID);
		CurrentLoc = Nearest->GetActorLocation();
		ChainPositions.Add(CurrentLoc);
	}

	if (ChainPositions.Num() >= 2 && CachedSlashVFXNiagara)
		SpawnBounceVFX(ChainPositions, FLinearColor(1.f, 0.85f, 0.2f, 1.f));
}
