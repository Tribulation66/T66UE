// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MobManagerSubsystem.h"

#include "Components/CapsuleComponent.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66ActorRegistrySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunStateSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Gameplay/T66CombatHitZoneComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobBase.h"
#include "HAL/IConsoleManager.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66MobManager, Log, All);
DEFINE_LOG_CATEGORY(LogT66RangedDiagnostics);

namespace
{
FString GetMobIDForLog(const AT66MobBase* Mob)
{
	if (!Mob || Mob->MobID.IsNone())
	{
		return TEXT("unset");
	}

	return Mob->MobID.ToString();
}

constexpr float T66MobArrivalThreshold = 100.f;
constexpr float T66MobMeleeChaseSpeed = 350.f;
constexpr float T66MobTouchDamageCooldown = 0.5f;
constexpr float T66MobTouchDamageContactTolerance = 12.f;
constexpr float T66MobSafeZoneRepelStopDistance = 1450.f;
constexpr float T66MobSafeZoneRepelSpeedScale = 0.95f;
const FName T66MobTestActorTag(TEXT("T66_TestMob"));
const FVector T66MobPoolParkingLocation(0.f, 0.f, -50000.f);
const FName T66ManagedMobVATClip_Idle(TEXT("Idle"));
const FName T66ManagedMobVATClip_Move(TEXT("Move"));
const FName T66ManagedMobVATClip_AttackCue(TEXT("AttackCue"));
const FName T66ManagedMobVATClip_HitReact(TEXT("HitReact"));
const FName T66ManagedMobVATClip_Death(TEXT("Death"));

static TAutoConsoleVariable<int32> CVarT66MobManagerTickProfileEnabled(
	TEXT("T66.Mob.ManagerTickProfileEnabled"),
	0,
	TEXT("Non-shipping diagnostics: when 1, UT66MobManagerSubsystem records grouped per-family tick timing and emits VeryVerbose summaries every 60 frames. Default 0 has no cycle-counter sampling."),
#if UE_BUILD_SHIPPING
	ECVF_ReadOnly
#else
	ECVF_Default
#endif
);

static TAutoConsoleVariable<int32> CVarT66RangedDiagnosticLogging(
	TEXT("T66.Ranged.DiagnosticLogging"),
	0,
	TEXT("Non-shipping diagnostic: when 1, tracks aggregate Ranged decision counters and emits one terminal [RangedDecisionSummary] line. Default 0 has no hot-path tracking or logging."),
#if UE_BUILD_SHIPPING
	ECVF_ReadOnly
#else
	ECVF_Default
#endif
);

struct FT66MobManagerProfileWindow
{
	uint64 TotalMobCycles = 0;
	uint64 MeleeMobCycles = 0;
	uint64 RushMobCycles = 0;
	uint64 FlyingMobCycles = 0;
	uint64 RangedMobCycles = 0;
	uint64 PoolAcquireCycles = 0;
	uint64 PoolReleaseCycles = 0;
	int32 TotalMobSamples = 0;
	int32 MeleeMobSamples = 0;
	int32 RushMobSamples = 0;
	int32 FlyingMobSamples = 0;
	int32 RangedMobSamples = 0;
	int32 PoolAcquireSamples = 0;
	int32 PoolReleaseSamples = 0;

	void Reset()
	{
		*this = FT66MobManagerProfileWindow{};
	}
};

FT66MobManagerProfileWindow GManagerProfileWindow;

bool IsManagerTickProfileEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarT66MobManagerTickProfileEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

ET66RangedDiagnosticPath ResolveRangedDiagnosticPathFromOwner(const AActor* Owner, FName& OutMobID, FString& OutOwnerClass)
{
	OutMobID = NAME_None;
	OutOwnerClass = Owner && Owner->GetClass() ? Owner->GetClass()->GetName() : FString(TEXT("None"));
	if (const AT66MobBase* Mob = Cast<AT66MobBase>(Owner))
	{
		OutMobID = Mob->MobID;
		return Mob->GetEnemyFamily() == ET66EnemyFamily::Ranged
			? ET66RangedDiagnosticPath::Lightweight
			: ET66RangedDiagnosticPath::Unknown;
	}
	if (const AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Owner))
	{
		OutMobID = Enemy->MobID;
		return Enemy->EnemyFamily == ET66EnemyFamily::Ranged
			? ET66RangedDiagnosticPath::Rich
			: ET66RangedDiagnosticPath::Unknown;
	}
	return ET66RangedDiagnosticPath::Unknown;
}

void IncrementPathCounter(const bool bLightweight, int32& RichCounter, int32& LightweightCounter)
{
	if (bLightweight)
	{
		++LightweightCounter;
	}
	else
	{
		++RichCounter;
	}
}

FT66RouteAttributionFamilyCounters& GetRouteFamilyCounters(FT66RouteAttributionDiagnostics& Diagnostics, const ET66EnemyFamily Family)
{
	switch (Family)
	{
	case ET66EnemyFamily::Melee:
		return Diagnostics.Melee;
	case ET66EnemyFamily::Rush:
		return Diagnostics.Rush;
	case ET66EnemyFamily::Flying:
		return Diagnostics.Flying;
	case ET66EnemyFamily::Ranged:
		return Diagnostics.Ranged;
	case ET66EnemyFamily::Special:
	default:
		return Diagnostics.SpecialUnknown;
	}
}

enum class ET66RangedLosBlockerBucket : uint8
{
	WorldStatic,
	WorldDynamic,
	RichEnemy,
	LightweightMob,
	OtherPawn,
	Unknown
};

ET66RangedLosBlockerBucket ClassifyRangedLosBlocker(const AActor* BlockerActor, const UPrimitiveComponent* BlockerComponent)
{
	if (!BlockerActor)
	{
		if (!BlockerComponent)
		{
			return ET66RangedLosBlockerBucket::Unknown;
		}

		switch (BlockerComponent->GetCollisionObjectType())
		{
		case ECC_WorldStatic:
			return ET66RangedLosBlockerBucket::WorldStatic;
		case ECC_WorldDynamic:
			return ET66RangedLosBlockerBucket::WorldDynamic;
		case ECC_Pawn:
			return ET66RangedLosBlockerBucket::OtherPawn;
		default:
			return ET66RangedLosBlockerBucket::Unknown;
		}
	}

	if (Cast<AT66EnemyBase>(BlockerActor))
	{
		return ET66RangedLosBlockerBucket::RichEnemy;
	}
	if (Cast<AT66MobBase>(BlockerActor))
	{
		return ET66RangedLosBlockerBucket::LightweightMob;
	}
	if (Cast<APawn>(BlockerActor))
	{
		return ET66RangedLosBlockerBucket::OtherPawn;
	}

	if (BlockerComponent)
	{
		switch (BlockerComponent->GetCollisionObjectType())
		{
		case ECC_WorldStatic:
			return ET66RangedLosBlockerBucket::WorldStatic;
		case ECC_WorldDynamic:
			return ET66RangedLosBlockerBucket::WorldDynamic;
		case ECC_Pawn:
			return ET66RangedLosBlockerBucket::OtherPawn;
		default:
			break;
		}
	}

	return ET66RangedLosBlockerBucket::Unknown;
}

void IncrementLosBlockerBucket(FT66RangedPressureDiagnostics& Diagnostics, const bool bLightweight, const ET66RangedLosBlockerBucket Bucket)
{
	switch (Bucket)
	{
	case ET66RangedLosBlockerBucket::WorldStatic:
		IncrementPathCounter(bLightweight, Diagnostics.RichLosBlockerWorldStatic, Diagnostics.LightweightLosBlockerWorldStatic);
		break;
	case ET66RangedLosBlockerBucket::WorldDynamic:
		IncrementPathCounter(bLightweight, Diagnostics.RichLosBlockerWorldDynamic, Diagnostics.LightweightLosBlockerWorldDynamic);
		break;
	case ET66RangedLosBlockerBucket::RichEnemy:
		IncrementPathCounter(bLightweight, Diagnostics.RichLosBlockerRichEnemy, Diagnostics.LightweightLosBlockerRichEnemy);
		break;
	case ET66RangedLosBlockerBucket::LightweightMob:
		IncrementPathCounter(bLightweight, Diagnostics.RichLosBlockerLightweightMob, Diagnostics.LightweightLosBlockerLightweightMob);
		break;
	case ET66RangedLosBlockerBucket::OtherPawn:
		IncrementPathCounter(bLightweight, Diagnostics.RichLosBlockerOtherPawn, Diagnostics.LightweightLosBlockerOtherPawn);
		break;
	case ET66RangedLosBlockerBucket::Unknown:
	default:
		IncrementPathCounter(bLightweight, Diagnostics.RichLosBlockerUnknown, Diagnostics.LightweightLosBlockerUnknown);
		break;
	}
}

double CyclesToMicroseconds(const uint64 Cycles)
{
	return FPlatformTime::ToMilliseconds64(Cycles) * 1000.0;
}

void AccumulateMobTickProfile(
	const bool bProfileEnabled,
	const uint64 StartCycles,
	const ET66EnemyFamily Family)
{
	if (!bProfileEnabled || StartCycles == 0)
	{
		return;
	}

	const uint64 ElapsedCycles = FPlatformTime::Cycles64() - StartCycles;
	GManagerProfileWindow.TotalMobCycles += ElapsedCycles;
	++GManagerProfileWindow.TotalMobSamples;

	if (Family == ET66EnemyFamily::Melee)
	{
		GManagerProfileWindow.MeleeMobCycles += ElapsedCycles;
		++GManagerProfileWindow.MeleeMobSamples;
	}
	else if (Family == ET66EnemyFamily::Rush)
	{
		GManagerProfileWindow.RushMobCycles += ElapsedCycles;
		++GManagerProfileWindow.RushMobSamples;
	}
	else if (Family == ET66EnemyFamily::Flying)
	{
		GManagerProfileWindow.FlyingMobCycles += ElapsedCycles;
		++GManagerProfileWindow.FlyingMobSamples;
	}
	else if (Family == ET66EnemyFamily::Ranged)
	{
		GManagerProfileWindow.RangedMobCycles += ElapsedCycles;
		++GManagerProfileWindow.RangedMobSamples;
	}
}

void AccumulatePoolAcquireProfile(const bool bProfileEnabled, const uint64 StartCycles)
{
	if (!bProfileEnabled || StartCycles == 0)
	{
		return;
	}

	GManagerProfileWindow.PoolAcquireCycles += FPlatformTime::Cycles64() - StartCycles;
	++GManagerProfileWindow.PoolAcquireSamples;
}

void AccumulatePoolReleaseProfile(const bool bProfileEnabled, const uint64 StartCycles)
{
	if (!bProfileEnabled || StartCycles == 0)
	{
		return;
	}

	GManagerProfileWindow.PoolReleaseCycles += FPlatformTime::Cycles64() - StartCycles;
	++GManagerProfileWindow.PoolReleaseSamples;
}

float GetFamilyChaseSpeed(const AT66MobBase* Mob)
{
	if (!Mob)
	{
		return 0.f;
	}
	if (Mob->ChaseSpeed > 0.f)
	{
		return Mob->ChaseSpeed;
	}

	switch (Mob->GetEnemyFamily())
	{
	case ET66EnemyFamily::Flying:
		return 430.f;
	case ET66EnemyFamily::Ranged:
		return 320.f;
	case ET66EnemyFamily::Rush:
		return 330.f;
	case ET66EnemyFamily::Melee:
	default:
		return T66MobMeleeChaseSpeed;
	}
}

AT66HeroBase* ResolveLocalHero(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (const APlayerController* PlayerController = It->Get())
		{
			if (AT66HeroBase* Hero = Cast<AT66HeroBase>(PlayerController->GetPawn()))
			{
				return Hero;
			}
		}
	}

	return nullptr;
}

float ResolveRunStateHeroHP(UWorld* World)
{
	if (!World)
	{
		return -1.f;
	}

	if (const UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (const UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
		{
			return RunState->GetCurrentHP();
		}
	}

	return -1.f;
}

void TickStatusTimer(float& RemainingSeconds, float& DurationSeconds, float DeltaTime)
{
	if (RemainingSeconds <= 0.f)
	{
		return;
	}

	RemainingSeconds = FMath::Max(0.f, RemainingSeconds - DeltaTime);
	if (RemainingSeconds <= 0.f)
	{
		DurationSeconds = 0.f;
	}
}

bool IsChaseBlockedByStatus(const AT66MobBase* Mob)
{
	if (!Mob)
	{
		return true;
	}

	if (Mob->FreezeSecondsRemaining > 0.f)
	{
		return true;
	}
	if (Mob->StunSecondsRemaining > 0.f)
	{
		return true;
	}
	if (Mob->RootSecondsRemaining > 0.f)
	{
		return true;
	}
	if (Mob->KnockbackSecondsRemaining > 0.f)
	{
		return true;
	}

	return false;
}

void ApplyMobKnockback(AT66MobBase* Mob, const float DeltaTime)
{
	if (!Mob || Mob->KnockbackSecondsRemaining <= 0.f)
	{
		return;
	}

	const FVector MobLocation = Mob->GetActorLocation();
	FVector NewLocation = MobLocation + Mob->KnockbackVelocity * DeltaTime;
	NewLocation.Z = MobLocation.Z;
	Mob->StoredVelocity = Mob->KnockbackVelocity;
	Mob->SetActorLocation(NewLocation, false);

	FVector Facing = Mob->KnockbackVelocity;
	Facing.Z = 0.f;
	if (!Facing.IsNearlyZero())
	{
		Mob->SetActorRotation(Facing.GetSafeNormal2D().Rotation());
	}
}

void ApplyMobPlanarMovement(
	AT66MobBase* Mob,
	const FVector& Direction,
	const float Speed,
	const float DeltaTime,
	const FVector* StopBeforeLocation = nullptr,
	const float StopDistance = 0.f)
{
	if (!Mob)
	{
		return;
	}

	const FVector MobLocation = Mob->GetActorLocation();
	Mob->StoredVelocity = Direction * FMath::Max(0.f, Speed);

	FVector NewLocation = MobLocation + Mob->StoredVelocity * DeltaTime;
	NewLocation.Z = MobLocation.Z;
	if (StopBeforeLocation && StopDistance > 0.f)
	{
		FVector ToStopLocation = *StopBeforeLocation - MobLocation;
		ToStopLocation.Z = 0.f;
		if (FVector::DotProduct(Direction, ToStopLocation) > 0.f)
		{
			FVector ProposedFromStop = NewLocation - *StopBeforeLocation;
			ProposedFromStop.Z = 0.f;
			if (ProposedFromStop.SizeSquared2D() < FMath::Square(StopDistance))
			{
				NewLocation = *StopBeforeLocation - Direction * StopDistance;
				NewLocation.Z = MobLocation.Z;
				Mob->StoredVelocity = (NewLocation - MobLocation) / FMath::Max(DeltaTime, KINDA_SMALL_NUMBER);
			}
		}
	}
	Mob->SetActorLocation(NewLocation, false);

	if (!Mob->StoredVelocity.IsNearlyZero())
	{
		Mob->SetActorRotation(Direction.Rotation());
	}
}

bool TickRushMovementIfNeeded(AT66MobBase* Mob, const FVector& HeroLocation, const float DeltaTime)
{
	if (!Mob || Mob->GetEnemyFamily() != ET66EnemyFamily::Rush)
	{
		return false;
	}

	const FVector MobLocation = Mob->GetActorLocation();
	FVector ToHero = HeroLocation - MobLocation;
	ToHero.Z = 0.f;
	const float DistanceToHero = ToHero.Size2D();
	const bool bHasDirection = ToHero.Normalize();

	if (Mob->bIsRushing)
	{
		Mob->RushSecondsRemaining = FMath::Max(0.f, Mob->RushSecondsRemaining - DeltaTime);

		FVector Direction = Mob->RushDirection;
		Direction.Z = 0.f;
		if (!Direction.Normalize())
		{
			Mob->bIsRushing = false;
			Mob->RushSecondsRemaining = 0.f;
			Mob->RushCooldownRemaining = Mob->RushIntervalSeconds;
			Mob->StoredVelocity = FVector::ZeroVector;
			return true;
		}

		ApplyMobPlanarMovement(Mob, Direction, GetFamilyChaseSpeed(Mob) * Mob->RushSpeedMultiplier, DeltaTime, &HeroLocation, T66MobArrivalThreshold);
		if (Mob->RushSecondsRemaining <= 0.f)
		{
			Mob->bIsRushing = false;
			Mob->RushCooldownRemaining = Mob->RushIntervalSeconds;
		}
		return true;
	}

	Mob->RushCooldownRemaining = FMath::Max(0.f, Mob->RushCooldownRemaining - DeltaTime);
	if (Mob->RushCooldownRemaining <= 0.f && bHasDirection && DistanceToHero <= Mob->RushTriggerDistance)
	{
		Mob->bIsRushing = true;
		Mob->RushSecondsRemaining = Mob->RushDurationSeconds;
		Mob->RushDirection = ToHero;
		ApplyMobPlanarMovement(Mob, ToHero, GetFamilyChaseSpeed(Mob) * Mob->RushSpeedMultiplier, DeltaTime, &HeroLocation, T66MobArrivalThreshold);
		return true;
	}

	return false;
}

bool TickFlyingMovementIfNeeded(AT66MobBase* Mob, const FVector& HeroLocation, const float DeltaTime)
{
	if (!Mob || Mob->GetEnemyFamily() != ET66EnemyFamily::Flying)
	{
		return false;
	}

	if (Mob->HoverAnchorZ <= KINDA_SMALL_NUMBER)
	{
		Mob->HoverAnchorZ = Mob->GetActorLocation().Z + Mob->HoverHeight;
	}

	const FVector MobLocation = Mob->GetActorLocation();
	FVector ToHero = HeroLocation - MobLocation;
	ToHero.Z = 0.f;

	FVector NewLocation = MobLocation;
	Mob->HoverBobTime += DeltaTime;
	const float DesiredZ = Mob->HoverAnchorZ + (FMath::Sin(Mob->HoverBobTime * Mob->HoverBobFrequency) * Mob->HoverBobAmplitude);
	NewLocation.Z = FMath::FInterpTo(MobLocation.Z, DesiredZ, DeltaTime, 6.f);

	const float DistanceToHero = ToHero.Size2D();
	if (DistanceToHero >= T66MobArrivalThreshold)
	{
		const FVector Direction = ToHero.GetSafeNormal2D();
		const float SlowFactor = FMath::Clamp(1.f - Mob->SlowStrength, 0.f, 1.f);
		const float ChaseSpeed = GetFamilyChaseSpeed(Mob) * SlowFactor;
		Mob->StoredVelocity = Direction * FMath::Max(0.f, ChaseSpeed);
		NewLocation.X += Mob->StoredVelocity.X * DeltaTime;
		NewLocation.Y += Mob->StoredVelocity.Y * DeltaTime;
		Mob->SetActorRotation(Direction.Rotation());
	}
	else
	{
		Mob->StoredVelocity = FVector::ZeroVector;
	}

	Mob->SetActorLocation(NewLocation, false);
	return true;
}

bool TickRangedMovementIfNeeded(UT66MobManagerSubsystem* Manager, AT66MobBase* Mob, const AT66HeroBase* Hero, const FVector& HeroLocation, const float DeltaTime)
{
	if (!Mob || Mob->GetEnemyFamily() != ET66EnemyFamily::Ranged)
	{
		return false;
	}

	Mob->FireCooldownRemaining = FMath::Max(0.f, Mob->FireCooldownRemaining - DeltaTime);

	const FVector MobLocation = Mob->GetActorLocation();
	FVector ToHero = HeroLocation - MobLocation;
	ToHero.Z = 0.f;
	const float DistanceToHero = ToHero.Size2D();
	if (Manager)
	{
		Manager->RecordRangedFireAttempt(true, Mob->MobID, DistanceToHero);
	}
	const bool bHasDirection = ToHero.Normalize();
	const float SlowFactor = FMath::Clamp(1.f - Mob->SlowStrength, 0.f, 1.f);
	const float MoveSpeed = GetFamilyChaseSpeed(Mob) * SlowFactor;

	if (bHasDirection && DistanceToHero < Mob->DesiredMinRange)
	{
		ApplyMobPlanarMovement(Mob, -ToHero, MoveSpeed, DeltaTime);
	}
	else if (bHasDirection && DistanceToHero > Mob->DesiredMaxRange)
	{
		ApplyMobPlanarMovement(Mob, ToHero, MoveSpeed, DeltaTime);
	}
	else
	{
		Mob->StoredVelocity = FVector::ZeroVector;
	}

	if (Mob->FireCooldownRemaining <= 0.f)
	{
		if (Hero && Hero->IsInSafeZone())
		{
			if (Manager)
			{
				Manager->RecordRangedFireSkippedSafeZone(true, Mob->MobID, DistanceToHero);
			}
			return true;
		}

		const float MaxFireRange = FMath::Max(Mob->DesiredMaxRange, 1.f) + FMath::Max(0.f, Mob->FireRangeGrace);
		if (DistanceToHero > MaxFireRange)
		{
			if (Manager)
			{
				Manager->RecordRangedFireSkippedOutOfRange(true, Mob->MobID, DistanceToHero, MaxFireRange);
			}
			Mob->FireCooldownRemaining = 0.25f;
		}
		else
		{
			if (Manager)
			{
				Manager->RecordRangedDistancePassed(true, Mob->MobID, DistanceToHero, Mob->DesiredMinRange, Mob->DesiredMaxRange, MaxFireRange);
			}
			if (Mob->TryFireProjectileAtHero(Hero))
			{
				Mob->FireCooldownRemaining = Mob->FireCooldownDuration;
			}
			else
			{
				Mob->FireCooldownRemaining = 0.25f;
			}
		}
	}
	else if (Manager)
	{
		Manager->RecordRangedCooldownBlocked(true, Mob->MobID, DistanceToHero, Mob->FireCooldownRemaining);
	}

	return true;
}

bool TickSafeZoneRepelMovementIfNeeded(UT66MobManagerSubsystem* Manager, AT66MobBase* Mob, const AT66HeroBase* Hero, const FVector& HeroLocation, const float DeltaTime)
{
	if (!Mob || !Hero || !Hero->IsInSafeZone())
	{
		return false;
	}

	const FVector MobLocation = Mob->GetActorLocation();
	FVector AwayFromHero = MobLocation - HeroLocation;
	AwayFromHero.Z = 0.f;
	const float DistanceToHero = AwayFromHero.Size2D();
	const bool bHasDirection = AwayFromHero.Normalize();

	if (Mob->GetEnemyFamily() == ET66EnemyFamily::Ranged && Manager)
	{
		Manager->RecordRangedFireAttempt(true, Mob->MobID, DistanceToHero);
		Manager->RecordRangedFireSkippedSafeZone(true, Mob->MobID, DistanceToHero);
	}

	Mob->bIsRushing = false;
	Mob->RushSecondsRemaining = 0.f;

	if (!bHasDirection || DistanceToHero >= T66MobSafeZoneRepelStopDistance)
	{
		Mob->StoredVelocity = FVector::ZeroVector;
		return true;
	}

	const float SlowFactor = FMath::Clamp(1.f - Mob->SlowStrength, 0.f, 1.f);
	const float RepelSpeed = GetFamilyChaseSpeed(Mob) * SlowFactor * T66MobSafeZoneRepelSpeedScale;
	ApplyMobPlanarMovement(Mob, AwayFromHero, RepelSpeed, DeltaTime);
	return true;
}

void SetMobRuntimeActive(AT66MobBase* Mob, const bool bActive)
{
	if (!Mob)
	{
		return;
	}

	Mob->SetActorHiddenInGame(!bActive);
	Mob->SetActorEnableCollision(bActive);
	Mob->SetActorTickEnabled(false);

	if (UCapsuleComponent* Capsule = Mob->CapsuleComponent)
	{
		Capsule->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		Capsule->SetGenerateOverlapEvents(bActive);
	}

	if (UT66CombatHitZoneComponent* BodyHitZone = Mob->BodyHitZone)
	{
		BodyHitZone->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		BodyHitZone->SetGenerateOverlapEvents(bActive);
		BodyHitZone->bTargetable = bActive;
	}

	if (UT66CombatHitZoneComponent* HeadHitZone = Mob->HeadHitZone)
	{
		HeadHitZone->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		HeadHitZone->SetGenerateOverlapEvents(bActive);
		HeadHitZone->bTargetable = bActive;
	}

	if (UStaticMeshComponent* VisualMesh = Mob->VisualMesh)
	{
		VisualMesh->SetHiddenInGame(!bActive, true);
		VisualMesh->SetVisibility(bActive, true);
	}

	Mob->HideLockIndicator();
}

void ApplyMobTouchDamageIfNeeded(AT66MobBase* Mob, AT66HeroBase* Hero, const float DeltaTime)
{
	if (!Mob)
	{
		return;
	}

	Mob->TouchDamageCooldownSeconds = FMath::Max(0.f, Mob->TouchDamageCooldownSeconds - DeltaTime);

	if (!Hero)
	{
		Mob->bIsTouchingHero = false;
		return;
	}

	const UCapsuleComponent* MobCapsule = Mob->CapsuleComponent;
	const UCapsuleComponent* HeroCapsule = Hero->GetCapsuleComponent();
	if (!MobCapsule || !HeroCapsule)
	{
		Mob->bIsTouchingHero = false;
		return;
	}

	const FVector MobLocation = MobCapsule->GetComponentLocation();
	const FVector HeroLocation = HeroCapsule->GetComponentLocation();
	const float TouchRadius = MobCapsule->GetScaledCapsuleRadius()
		+ HeroCapsule->GetScaledCapsuleRadius()
		+ T66MobTouchDamageContactTolerance;
	const float MaxVerticalSeparation = MobCapsule->GetScaledCapsuleHalfHeight()
		+ HeroCapsule->GetScaledCapsuleHalfHeight()
		+ T66MobTouchDamageContactTolerance;
	const bool bInTouchRange =
		FVector::DistSquared2D(MobLocation, HeroLocation) <= FMath::Square(TouchRadius)
		&& FMath::Abs(MobLocation.Z - HeroLocation.Z) <= MaxVerticalSeparation;
	if (Mob->bIsTouchingHero)
	{
		if (bInTouchRange)
		{
			return;
		}

		Mob->bIsTouchingHero = false;
		return;
	}

	if (!bInTouchRange)
	{
		return;
	}

	Mob->bIsTouchingHero = true;
	if (Hero->IsVehicleMounted() || Hero->IsInSafeZone() || Mob->TouchDamageHearts <= 0 || Mob->TouchDamageCooldownSeconds > 0.f)
	{
		return;
	}

	if (UWorld* World = Mob->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
			{
				const int32 DamageHP = FMath::Max(1, Mob->TouchDamageHearts) * 20;
				if (RunState->ApplyDamage(DamageHP, Mob, FName(TEXT("EnemyTouch")), Mob))
				{
					Mob->TouchDamageCooldownSeconds = T66MobTouchDamageCooldown;
					UE_LOG(LogT66MobManager, VeryVerbose, TEXT("MobTouchDamage mob=%s MobID=%s damageHP=%d hero=%s cooldown=%.2f"),
						*GetNameSafe(Mob),
						*GetMobIDForLog(Mob),
						DamageHP,
						*GetNameSafe(Hero),
						T66MobTouchDamageCooldown);
				}
			}
		}
	}
}

bool IsTestMob(const AT66MobBase* Mob)
{
	return Mob && (Mob->ActorHasTag(T66MobTestActorTag) || Mob->MobID.ToString().StartsWith(TEXT("TestMob_")));
}

FString BuildStatusFlagsForLog(const AT66MobBase* Mob)
{
	if (!Mob)
	{
		return TEXT("None");
	}

	TArray<FString> Flags;
	if (Mob->FreezeSecondsRemaining > 0.f)
	{
		Flags.Add(FString::Printf(TEXT("Freeze=%.2f"), Mob->FreezeSecondsRemaining));
	}
	if (Mob->StunSecondsRemaining > 0.f)
	{
		Flags.Add(FString::Printf(TEXT("Stun=%.2f"), Mob->StunSecondsRemaining));
	}
	if (Mob->RootSecondsRemaining > 0.f)
	{
		Flags.Add(FString::Printf(TEXT("Root=%.2f"), Mob->RootSecondsRemaining));
	}
	if (Mob->KnockbackSecondsRemaining > 0.f)
	{
		Flags.Add(FString::Printf(TEXT("Knockback=%.2f"), Mob->KnockbackSecondsRemaining));
	}
	if (Mob->SlowSecondsRemaining > 0.f)
	{
		Flags.Add(FString::Printf(TEXT("Slow=%.2f/%.2f"), Mob->SlowSecondsRemaining, Mob->SlowStrength));
	}
	if (Mob->GetEnemyFamily() == ET66EnemyFamily::Rush)
	{
		if (Mob->bIsRushing)
		{
			Flags.Add(FString::Printf(TEXT("Rush=%.2f"), Mob->RushSecondsRemaining));
		}
		else if (Mob->RushCooldownRemaining > 0.f)
		{
			Flags.Add(FString::Printf(TEXT("RushCooldown=%.2f"), Mob->RushCooldownRemaining));
		}
	}
	if (Mob->GetEnemyFamily() == ET66EnemyFamily::Flying)
	{
		Flags.Add(FString::Printf(TEXT("HoverZ=%.1f"), Mob->HoverAnchorZ));
	}
	if (Mob->GetEnemyFamily() == ET66EnemyFamily::Ranged)
	{
		Flags.Add(FString::Printf(TEXT("FireCooldown=%.2f"), Mob->FireCooldownRemaining));
	}

	return Flags.IsEmpty() ? TEXT("None") : FString::Join(Flags, TEXT("|"));
}

#if !UE_BUILD_SHIPPING
UT66MobManagerSubsystem* GetMobManagerForCommand(UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogT66MobManager, Warning, TEXT("T66.Mob command failed: no world."));
		return nullptr;
	}

	UT66MobManagerSubsystem* Manager = World->GetSubsystem<UT66MobManagerSubsystem>();
	if (!Manager)
	{
		UE_LOG(LogT66MobManager, Warning, TEXT("T66.Mob command failed: no UT66MobManagerSubsystem in world=%s."), *GetNameSafe(World));
	}
	return Manager;
}

FVector ResolveSpawnTestLocation(UWorld* World)
{
	if (!World)
	{
		return FVector::ZeroVector;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (const APlayerController* PlayerController = It->Get())
		{
			if (const APawn* Pawn = PlayerController->GetPawn())
			{
				return Pawn->GetActorLocation() + FVector(300.f, 0.f, 0.f);
			}
		}
	}

	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.SpawnTest found no local hero pawn; spawning at world origin."));
	return FVector::ZeroVector;
}

void ResolveMobSpawnDefaults(UWorld* World, int32& OutStageNum, float& OutDifficultyScalar, float& OutEnemyProgressionScalar, float& OutFinaleScalar)
{
	OutStageNum = 1;
	OutDifficultyScalar = 1.f;
	OutEnemyProgressionScalar = 1.f;
	OutFinaleScalar = 1.f;

	if (!World)
	{
		return;
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (UT66RunStateSubsystem* RunState = GameInstance->GetSubsystem<UT66RunStateSubsystem>())
		{
			OutStageNum = FMath::Max(1, RunState->GetCurrentStage());
			OutDifficultyScalar = RunState->GetDifficultyScalar();
			OutFinaleScalar = RunState->GetFinalSurvivalEnemyScalar();
		}
	}
}

AT66MobBase* SpawnConfiguredTestMob(UWorld* World, const FName MobID, const FVector& SpawnLocation, const ET66EnemyFamily Family = ET66EnemyFamily::Melee)
{
	if (!World)
	{
		return nullptr;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	AT66MobBase* Mob = World->SpawnActorDeferred<AT66MobBase>(AT66MobBase::StaticClass(), SpawnTransform);
	if (!Mob)
	{
		UE_LOG(LogT66MobManager, Warning, TEXT("T66.Mob.SpawnTest failed to spawn AT66MobBase at %s."), *SpawnLocation.ToCompactString());
		return nullptr;
	}

	Mob->Tags.AddUnique(T66MobTestActorTag);
	Mob->MobID = MobID;
	Mob->CharacterVisualID = MobID;
	Mob->LifecycleState = ET66MobLifecycleState::Active;
	Mob->FinishSpawning(SpawnTransform);

	int32 StageNum = 1;
	float DifficultyScalar = 1.f;
	float EnemyProgressionScalar = 1.f;
	float FinaleScalar = 1.f;
	ResolveMobSpawnDefaults(World, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar);
	Mob->ConfigureAsMob(MobID, Family, NAME_None, StageNum, DifficultyScalar, EnemyProgressionScalar, FinaleScalar, false);
	return Mob;
}

void SpawnTestMobCommand(const TArray<FString>& Args, UWorld* World)
{
	UT66MobManagerSubsystem* Manager = GetMobManagerForCommand(World);
	if (!Manager || !World)
	{
		return;
	}

	const FName RequestedMobID = Args.IsValidIndex(0) && !Args[0].TrimStartAndEnd().IsEmpty()
		? FName(*Args[0].TrimStartAndEnd())
		: FName(TEXT("Slime"));
	const FVector SpawnLocation = ResolveSpawnTestLocation(World);
	AT66MobBase* Mob = SpawnConfiguredTestMob(World, RequestedMobID, SpawnLocation);
	if (!Mob)
	{
		return;
	}

	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.SpawnTest spawned configured mob=%s MobID=%s HP=%.1f speed=%.1f touchHearts=%d location=%s ActiveMobs.Num()=%d"),
		*GetNameSafe(Mob),
		*Mob->MobID.ToString(),
		Mob->MaxHP,
		Mob->ChaseSpeed,
		Mob->TouchDamageHearts,
		*SpawnLocation.ToCompactString(),
		Manager->GetActiveMobs().Num());
}

void SpawnTestRushCommand(const TArray<FString>& Args, UWorld* World)
{
	UT66MobManagerSubsystem* Manager = GetMobManagerForCommand(World);
	if (!Manager || !World)
	{
		return;
	}

	const FName RequestedMobID = Args.IsValidIndex(0) && !Args[0].TrimStartAndEnd().IsEmpty()
		? FName(*Args[0].TrimStartAndEnd())
		: FName(TEXT("RatPack"));
	const FVector SpawnLocation = ResolveSpawnTestLocation(World);
	AT66MobBase* Mob = SpawnConfiguredTestMob(World, RequestedMobID, SpawnLocation, ET66EnemyFamily::Rush);
	if (!Mob)
	{
		return;
	}

	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.SpawnTestRush spawned rush mob=%s MobID=%s HP=%.1f speed=%.1f rushDuration=%.2f rushMultiplier=%.2f rushCooldown=%.2f location=%s ActiveMobs.Num()=%d"),
		*GetNameSafe(Mob),
		*Mob->MobID.ToString(),
		Mob->MaxHP,
		Mob->ChaseSpeed,
		Mob->RushDurationSeconds,
		Mob->RushSpeedMultiplier,
		Mob->RushCooldownRemaining,
		*SpawnLocation.ToCompactString(),
		Manager->GetActiveMobs().Num());
}

void SpawnTestFlyingCommand(const TArray<FString>& Args, UWorld* World)
{
	UT66MobManagerSubsystem* Manager = GetMobManagerForCommand(World);
	if (!Manager || !World)
	{
		return;
	}

	const FName RequestedMobID = Args.IsValidIndex(0) && !Args[0].TrimStartAndEnd().IsEmpty()
		? FName(*Args[0].TrimStartAndEnd())
		: FName(TEXT("CaveBat"));
	const FVector SpawnLocation = ResolveSpawnTestLocation(World);
	AT66MobBase* Mob = SpawnConfiguredTestMob(World, RequestedMobID, SpawnLocation, ET66EnemyFamily::Flying);
	if (!Mob)
	{
		return;
	}

	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.SpawnTestFlying spawned flying mob=%s MobID=%s HP=%.1f speed=%.1f hoverAnchorZ=%.1f hoverAmplitude=%.1f hoverFrequency=%.2f location=%s ActiveMobs.Num()=%d"),
		*GetNameSafe(Mob),
		*Mob->MobID.ToString(),
		Mob->MaxHP,
		Mob->ChaseSpeed,
		Mob->HoverAnchorZ,
		Mob->HoverBobAmplitude,
		Mob->HoverBobFrequency,
		*SpawnLocation.ToCompactString(),
		Manager->GetActiveMobs().Num());
}

void SpawnTestRangedCommand(const TArray<FString>& Args, UWorld* World)
{
	UT66MobManagerSubsystem* Manager = GetMobManagerForCommand(World);
	if (!Manager || !World)
	{
		return;
	}

	const FName RequestedMobID = Args.IsValidIndex(0) && !Args[0].TrimStartAndEnd().IsEmpty()
		? FName(*Args[0].TrimStartAndEnd())
		: FName(TEXT("HexSlinger"));
	FVector SpawnLocation = ResolveSpawnTestLocation(World);
	if (AT66HeroBase* Hero = ResolveLocalHero(World))
	{
		SpawnLocation = Hero->GetActorLocation() + Hero->GetActorForwardVector().GetSafeNormal2D() * 1000.f;
	}
	AT66MobBase* Mob = SpawnConfiguredTestMob(World, RequestedMobID, SpawnLocation, ET66EnemyFamily::Ranged);
	if (!Mob)
	{
		return;
	}

	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.SpawnTestRanged spawned ranged mob=%s MobID=%s HP=%.1f speed=%.1f minRange=%.1f maxRange=%.1f fireCooldown=%.2f projectileHeight=%.1f location=%s ActiveMobs.Num()=%d"),
		*GetNameSafe(Mob),
		*Mob->MobID.ToString(),
		Mob->MaxHP,
		Mob->ChaseSpeed,
		Mob->DesiredMinRange,
		Mob->DesiredMaxRange,
		Mob->FireCooldownDuration,
		Mob->ProjectileSpawnHeight,
		*SpawnLocation.ToCompactString(),
		Manager->GetActiveMobs().Num());
}

void SpawnTestRosterCommand(const TArray<FString>& Args, UWorld* World)
{
	(void)Args;

	UT66MobManagerSubsystem* Manager = GetMobManagerForCommand(World);
	if (!Manager || !World)
	{
		return;
	}

	const FName Roster[] =
	{
		FName(TEXT("Slime")),
		FName(TEXT("CaveBat")),
		FName(TEXT("BoneWalker")),
		FName(TEXT("RatPack")),
		FName(TEXT("TombSpider")),
		FName(TEXT("HexSlinger")),
		FName(TEXT("StoneSentinel")),
		FName(TEXT("MimicLure")),
		FName(TEXT("BoneConjurer")),
		FName(TEXT("CryptWraith"))
	};

	FVector Origin = ResolveSpawnTestLocation(World);
	FVector Right = FVector::RightVector;
	if (AT66HeroBase* Hero = ResolveLocalHero(World))
	{
		const FVector Forward = Hero->GetActorForwardVector().GetSafeNormal2D();
		Right = Hero->GetActorRightVector().GetSafeNormal2D();
		Origin = Hero->GetActorLocation() + Forward * 420.f - Right * 900.f;
	}

	int32 SpawnedCount = 0;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Roster); ++Index)
	{
		const FVector SpawnLocation = Origin + Right * static_cast<float>(Index * 200);
		if (AT66MobBase* Mob = SpawnConfiguredTestMob(World, Roster[Index], SpawnLocation))
		{
			Mob->ForceMobVertexAnimationClipForAutomation(FName(TEXT("Move")), 30.f);
			++SpawnedCount;
		}
	}

	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.SpawnTestRoster spawned %d configured VAT test mob(s). ActiveMobs.Num()=%d"),
		SpawnedCount,
		Manager->GetActiveMobs().Num());
}

void DespawnAllTestMobsCommand(const TArray<FString>& Args, UWorld* World)
{
	(void)Args;

	UT66MobManagerSubsystem* Manager = GetMobManagerForCommand(World);
	if (!Manager)
	{
		return;
	}

	TArray<TWeakObjectPtr<AT66MobBase>> MobsToInspect = Manager->GetActiveMobs();
	int32 DestroyedCount = 0;
	for (const TWeakObjectPtr<AT66MobBase>& WeakMob : MobsToInspect)
	{
		AT66MobBase* Mob = WeakMob.Get();
		if (!IsTestMob(Mob))
		{
			continue;
		}

		Mob->Destroy();
		++DestroyedCount;
	}

	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.DespawnAllTest destroyed %d test mob(s). ActiveMobs.Num()=%d"),
		DestroyedCount,
		Manager->GetActiveMobs().Num());
}

float ParseFloatArg(const TArray<FString>& Args, int32 Index, float DefaultValue)
{
	if (!Args.IsValidIndex(Index))
	{
		return DefaultValue;
	}

	return FCString::Atof(*Args[Index]);
}

template <typename ApplyStatusFuncType>
int32 ApplyStatusToTestMobs(UWorld* World, ApplyStatusFuncType ApplyStatus)
{
	UT66MobManagerSubsystem* Manager = GetMobManagerForCommand(World);
	if (!Manager)
	{
		return 0;
	}

	int32 AffectedCount = 0;
	for (const TWeakObjectPtr<AT66MobBase>& WeakMob : Manager->GetActiveMobs())
	{
		AT66MobBase* Mob = WeakMob.Get();
		if (!IsTestMob(Mob))
		{
			continue;
		}

		ApplyStatus(Mob);
		++AffectedCount;
	}

	return AffectedCount;
}

void TestStunCommand(const TArray<FString>& Args, UWorld* World)
{
	const float Seconds = FMath::Max(0.f, ParseFloatArg(Args, 0, 2.f));
	const int32 AffectedCount = ApplyStatusToTestMobs(World, [Seconds](AT66MobBase* Mob)
	{
		Mob->ApplyStun(Seconds);
	});
	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.TestStun seconds=%.2f affected=%d"), Seconds, AffectedCount);
}

void TestSlowCommand(const TArray<FString>& Args, UWorld* World)
{
	const float Seconds = FMath::Max(0.f, ParseFloatArg(Args, 0, 3.f));
	const float Strength = FMath::Clamp(ParseFloatArg(Args, 1, 0.5f), 0.f, 1.f);
	const int32 AffectedCount = ApplyStatusToTestMobs(World, [Seconds, Strength](AT66MobBase* Mob)
	{
		Mob->ApplySlow(Strength, Seconds);
	});
	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.TestSlow seconds=%.2f strength=%.2f affected=%d"), Seconds, Strength, AffectedCount);
}

void TestRootCommand(const TArray<FString>& Args, UWorld* World)
{
	const float Seconds = FMath::Max(0.f, ParseFloatArg(Args, 0, 2.f));
	const int32 AffectedCount = ApplyStatusToTestMobs(World, [Seconds](AT66MobBase* Mob)
	{
		Mob->ApplyRoot(Seconds);
	});
	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.TestRoot seconds=%.2f affected=%d"), Seconds, AffectedCount);
}

void TestFreezeCommand(const TArray<FString>& Args, UWorld* World)
{
	const float Seconds = FMath::Max(0.f, ParseFloatArg(Args, 0, 2.f));
	const int32 AffectedCount = ApplyStatusToTestMobs(World, [Seconds](AT66MobBase* Mob)
	{
		Mob->ApplyFreeze(Seconds);
	});
	UE_LOG(LogT66MobManager, Display, TEXT("T66.Mob.TestFreeze seconds=%.2f affected=%d"), Seconds, AffectedCount);
}

static FAutoConsoleCommandWithWorldAndArgs T66MobSpawnTestCommand(
	TEXT("T66.Mob.SpawnTest"),
	TEXT("Spawns one configured diagnostic AT66MobBase near the local hero. Usage: T66.Mob.SpawnTest [MobID=Slime]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SpawnTestMobCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobSpawnTestRosterCommand(
	TEXT("T66.Mob.SpawnTestRoster"),
	TEXT("Spawns the 10 Dungeon VAT mob visuals as configured diagnostic AT66MobBase actors."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SpawnTestRosterCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobSpawnTestRushCommand(
	TEXT("T66.Mob.SpawnTestRush"),
	TEXT("Spawns one configured Rush diagnostic AT66MobBase near the local hero. Usage: T66.Mob.SpawnTestRush [MobID=RatPack]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SpawnTestRushCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobSpawnTestFlyingCommand(
	TEXT("T66.Mob.SpawnTestFlying"),
	TEXT("Spawns one configured Flying diagnostic AT66MobBase near the local hero. Usage: T66.Mob.SpawnTestFlying [MobID=CaveBat]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SpawnTestFlyingCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobSpawnTestRangedCommand(
	TEXT("T66.Mob.SpawnTestRanged"),
	TEXT("Spawns one configured Ranged diagnostic AT66MobBase near the local hero. Usage: T66.Mob.SpawnTestRanged [MobID=HexSlinger]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SpawnTestRangedCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobDespawnAllTestCommand(
	TEXT("T66.Mob.DespawnAllTest"),
	TEXT("Destroys diagnostic AT66MobBase instances whose MobID starts with TestMob_."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&DespawnAllTestMobsCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobTestStunCommand(
	TEXT("T66.Mob.TestStun"),
	TEXT("Applies stun to active TestMob_* mobs. Usage: T66.Mob.TestStun [Seconds=2.0]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&TestStunCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobTestSlowCommand(
	TEXT("T66.Mob.TestSlow"),
	TEXT("Applies slow to active TestMob_* mobs. Usage: T66.Mob.TestSlow [Seconds=3.0] [Strength=0.5]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&TestSlowCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobTestRootCommand(
	TEXT("T66.Mob.TestRoot"),
	TEXT("Applies root to active TestMob_* mobs. Usage: T66.Mob.TestRoot [Seconds=2.0]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&TestRootCommand));

static FAutoConsoleCommandWithWorldAndArgs T66MobTestFreezeCommand(
	TEXT("T66.Mob.TestFreeze"),
	TEXT("Applies freeze to active TestMob_* mobs. Usage: T66.Mob.TestFreeze [Seconds=2.0]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&TestFreezeCommand));
#endif
}

void FT66RangedPressureDiagnostics::Reset(const FString& Reason)
{
	const int32 NextResetCount = ResetCount + 1;
	*this = FT66RangedPressureDiagnostics{};
	ResetCount = NextResetCount;
	LastResetReason = Reason;
}

void FT66RouteAttributionDiagnostics::Reset(const FString& Reason)
{
	const int32 NextResetCount = ResetCount + 1;
	*this = FT66RouteAttributionDiagnostics{};
	ResetCount = NextResetCount;
	LastResetReason = Reason;
}

void FT66MobVertexAnimationRuntimeState::Reset()
{
	Mob.Reset();
	Row = FT66MobVertexAnimationRow{};
	Material = nullptr;
	Clip = NAME_None;
	ClipTime = 0.f;
	PlayRate = 1.f;
	OverrideSecondsRemaining = 0.f;
	bUsingVertexAnimation = false;
	CustomDataFrame = 0.f;
	CustomDataStartFrame = 0.f;
	CustomDataEndFrame = 0.f;
	CustomDataClipIndex = 0.f;
	CustomDataPlayRate = 1.f;
	CustomDataFlags = 0.f;
}

int32 FT66RouteAttributionDiagnostics::CalculateCounterMismatch() const
{
	const int32 FamilyTotal = Melee.Total
		+ Rush.Total
		+ Flying.Total
		+ Ranged.Total
		+ SpecialUnknown.Total;
	return TotalObservedSpawns - FamilyTotal;
}

#if !UE_BUILD_SHIPPING
bool UT66MobManagerSubsystem::AutomationApplyMobTouchDamageForTest(AT66MobBase* Mob, AT66HeroBase* Hero, const float DeltaTime)
{
	UT66RunStateSubsystem* RunState = Hero && Hero->GetGameInstance()
		? Hero->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>()
		: nullptr;
	const float HPBefore = RunState ? RunState->GetCurrentHP() : -1.f;
	ApplyMobTouchDamageIfNeeded(Mob, Hero, DeltaTime);
	const float HPAfter = RunState ? RunState->GetCurrentHP() : -1.f;
	return HPBefore >= 0.f && HPAfter >= 0.f && HPAfter < HPBefore;
}
#endif

void UT66MobManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetRangedPressureDiagnostics(TEXT("Initialize"));
	UE_LOG(LogT66MobManager, VeryVerbose, TEXT("UT66MobManagerSubsystem initialized for world=%s"), *GetNameSafe(GetWorld()));
}

void UT66MobManagerSubsystem::Deinitialize()
{
	UE_LOG(LogT66MobManager, VeryVerbose, TEXT("UT66MobManagerSubsystem deinitialized for world=%s activeMobs=%d inactiveMobs=%d"),
		*GetNameSafe(GetWorld()),
		ActiveMobs.Num(),
		InactiveMobs.Num());
	ActiveMobs.Reset();
	InactiveMobs.Reset();
	ActiveMobVertexAnimationStates.Reset();
	Super::Deinitialize();
}

bool UT66MobManagerSubsystem::IsRangedDiagnosticLoggingEnabled()
{
#if !UE_BUILD_SHIPPING
	if (!IsInGameThread())
	{
		ensureMsgf(false, TEXT("UT66MobManagerSubsystem::IsRangedDiagnosticLoggingEnabled must run on the game thread."));
		return false;
	}
	return CVarT66RangedDiagnosticLogging.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool UT66MobManagerSubsystem::ShouldTrackRangedDiagnostics(const TCHAR* FunctionName) const
{
#if !UE_BUILD_SHIPPING
	if (!IsInGameThread())
	{
		ensureMsgf(false, TEXT("UT66MobManagerSubsystem::%s must run on the game thread for aggregate Ranged diagnostics."), FunctionName ? FunctionName : TEXT("Unknown"));
		return false;
	}
	return CVarT66RangedDiagnosticLogging.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

int32 UT66MobManagerSubsystem::FindMobVertexAnimationStateIndex(const AT66MobBase* Mob) const
{
	if (!Mob)
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < ActiveMobVertexAnimationStates.Num(); ++Index)
	{
		if (ActiveMobVertexAnimationStates[Index].Mob.Get() == Mob)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

FT66MobVertexAnimationRuntimeState* UT66MobManagerSubsystem::FindMobVertexAnimationState(AT66MobBase* Mob)
{
	const int32 StateIndex = FindMobVertexAnimationStateIndex(Mob);
	return StateIndex != INDEX_NONE ? &ActiveMobVertexAnimationStates[StateIndex] : nullptr;
}

const FT66MobVertexAnimationRuntimeState* UT66MobManagerSubsystem::FindMobVertexAnimationState(const AT66MobBase* Mob) const
{
	const int32 StateIndex = FindMobVertexAnimationStateIndex(Mob);
	return StateIndex != INDEX_NONE ? &ActiveMobVertexAnimationStates[StateIndex] : nullptr;
}

float UT66MobManagerSubsystem::ResolveMobVertexAnimationClipIndex(const FName ClipName) const
{
	if (ClipName == T66ManagedMobVATClip_Move)
	{
		return 1.f;
	}
	if (ClipName == T66ManagedMobVATClip_AttackCue)
	{
		return 2.f;
	}
	if (ClipName == T66ManagedMobVATClip_HitReact)
	{
		return 3.f;
	}
	if (ClipName == T66ManagedMobVATClip_Death)
	{
		return 4.f;
	}
	return 0.f;
}

bool UT66MobManagerSubsystem::GetMobVertexAnimationClipRange(const FT66MobVertexAnimationRuntimeState& State, const FName ClipName, int32& OutStartFrame, int32& OutEndFrame, float& OutPlayRate) const
{
	if (ClipName == T66ManagedMobVATClip_Move)
	{
		OutStartFrame = State.Row.MoveStartFrame;
		OutEndFrame = State.Row.MoveEndFrame;
		OutPlayRate = State.Row.MovePlayRate;
	}
	else if (ClipName == T66ManagedMobVATClip_AttackCue)
	{
		OutStartFrame = State.Row.AttackCueStartFrame;
		OutEndFrame = State.Row.AttackCueEndFrame;
		OutPlayRate = State.Row.AttackCuePlayRate;
	}
	else if (ClipName == T66ManagedMobVATClip_HitReact)
	{
		OutStartFrame = State.Row.HitReactStartFrame;
		OutEndFrame = State.Row.HitReactEndFrame;
		OutPlayRate = State.Row.HitReactPlayRate;
	}
	else if (ClipName == T66ManagedMobVATClip_Death)
	{
		OutStartFrame = State.Row.DeathStartFrame;
		OutEndFrame = State.Row.DeathEndFrame;
		OutPlayRate = State.Row.DeathPlayRate;
	}
	else
	{
		OutStartFrame = State.Row.IdleStartFrame;
		OutEndFrame = State.Row.IdleEndFrame;
		OutPlayRate = State.Row.IdlePlayRate;
	}

	OutStartFrame = FMath::Max(0, OutStartFrame);
	OutEndFrame = FMath::Max(OutStartFrame, OutEndFrame);
	OutPlayRate = FMath::Max(0.01f, OutPlayRate);
	return State.Row.SampleRate > 0.f
		&& State.Row.RowsPerFrame > 0
		&& OutEndFrame >= OutStartFrame;
}

void UT66MobManagerSubsystem::UpdateMobVertexAnimationCustomData(FT66MobVertexAnimationRuntimeState& State, const int32 Frame, const int32 StartFrame, const int32 EndFrame, const float PlayRate) const
{
	State.CustomDataFrame = static_cast<float>(Frame);
	State.CustomDataStartFrame = static_cast<float>(StartFrame);
	State.CustomDataEndFrame = static_cast<float>(EndFrame);
	State.CustomDataClipIndex = ResolveMobVertexAnimationClipIndex(State.Clip);
	State.CustomDataPlayRate = PlayRate;
	State.CustomDataFlags = State.bUsingVertexAnimation ? 1.f : 0.f;
}

void UT66MobManagerSubsystem::ConfigureMobVertexAnimationState(AT66MobBase* Mob, const FT66MobVertexAnimationRow& Row, UMaterialInstanceDynamic* Material)
{
	if (!Mob || !Material)
	{
		ClearMobVertexAnimationState(Mob);
		return;
	}

	ActiveMobVertexAnimationStates.RemoveAllSwap([](const FT66MobVertexAnimationRuntimeState& State)
	{
		return !State.Mob.IsValid();
	}, EAllowShrinking::No);

	int32 StateIndex = FindMobVertexAnimationStateIndex(Mob);
	if (StateIndex == INDEX_NONE)
	{
		StateIndex = ActiveMobVertexAnimationStates.AddDefaulted();
	}

	FT66MobVertexAnimationRuntimeState& State = ActiveMobVertexAnimationStates[StateIndex];
	State.Reset();
	State.Mob = Mob;
	State.Row = Row;
	State.Material = Material;
	State.bUsingVertexAnimation = true;
	SetMobVertexAnimationClip(Mob, T66ManagedMobVATClip_Idle);
}

void UT66MobManagerSubsystem::ClearMobVertexAnimationState(AT66MobBase* Mob)
{
	ActiveMobVertexAnimationStates.RemoveAllSwap([Mob](const FT66MobVertexAnimationRuntimeState& State)
	{
		return !State.Mob.IsValid() || (Mob && State.Mob.Get() == Mob);
	}, EAllowShrinking::No);
}

void UT66MobManagerSubsystem::SetMobVertexAnimationClip(AT66MobBase* Mob, const FName ClipName, const float OverrideSeconds)
{
	FT66MobVertexAnimationRuntimeState* State = FindMobVertexAnimationState(Mob);
	if (!State || !State->bUsingVertexAnimation || !State->Material)
	{
		return;
	}

	int32 StartFrame = 0;
	int32 EndFrame = 0;
	float ResolvedPlayRate = 1.f;
	if (!GetMobVertexAnimationClipRange(*State, ClipName, StartFrame, EndFrame, ResolvedPlayRate))
	{
		return;
	}

	if (State->Clip != ClipName)
	{
		State->Clip = ClipName;
		State->ClipTime = 0.f;
		State->Material->SetScalarParameterValue(TEXT("StartFrame"), static_cast<float>(StartFrame));
		State->Material->SetScalarParameterValue(TEXT("EndFrame"), static_cast<float>(EndFrame));
	}

	State->PlayRate = ResolvedPlayRate;
	State->OverrideSecondsRemaining = FMath::Max(State->OverrideSecondsRemaining, OverrideSeconds);
	UpdateMobVertexAnimationCustomData(*State, StartFrame, StartFrame, EndFrame, ResolvedPlayRate);
}

void UT66MobManagerSubsystem::TickMobVertexAnimationState(AT66MobBase* Mob, const float DeltaSeconds)
{
	FT66MobVertexAnimationRuntimeState* State = FindMobVertexAnimationState(Mob);
	if (!Mob || !State || !State->bUsingVertexAnimation || !State->Material)
	{
		return;
	}

	if (State->OverrideSecondsRemaining > 0.f)
	{
		State->OverrideSecondsRemaining = FMath::Max(0.f, State->OverrideSecondsRemaining - DeltaSeconds);
	}
	else
	{
		const FName DesiredClip = Mob->StoredVelocity.SizeSquared2D() > FMath::Square(10.f)
			? T66ManagedMobVATClip_Move
			: T66ManagedMobVATClip_Idle;
		if (State->Clip != DesiredClip)
		{
			SetMobVertexAnimationClip(Mob, DesiredClip);
			State = FindMobVertexAnimationState(Mob);
			if (!State || !State->bUsingVertexAnimation || !State->Material)
			{
				return;
			}
		}
	}

	const FName ActiveClip = State->Clip.IsNone() ? T66ManagedMobVATClip_Idle : State->Clip;
	int32 StartFrame = 0;
	int32 EndFrame = 0;
	float ResolvedPlayRate = 1.f;
	if (!GetMobVertexAnimationClipRange(*State, ActiveClip, StartFrame, EndFrame, ResolvedPlayRate))
	{
		return;
	}

	State->ClipTime += DeltaSeconds;
	State->PlayRate = ResolvedPlayRate;
	const int32 FrameCount = FMath::Max(1, EndFrame - StartFrame + 1);
	const int32 ClipFrameOffset = FMath::FloorToInt(State->ClipTime * State->Row.SampleRate * ResolvedPlayRate) % FrameCount;
	const int32 CurrentFrame = StartFrame + ClipFrameOffset;
	State->Material->SetScalarParameterValue(TEXT("Frame"), static_cast<float>(CurrentFrame));
	UpdateMobVertexAnimationCustomData(*State, CurrentFrame, StartFrame, EndFrame, ResolvedPlayRate);
}

bool UT66MobManagerSubsystem::IsMobUsingVertexAnimation(const AT66MobBase* Mob) const
{
	const FT66MobVertexAnimationRuntimeState* State = FindMobVertexAnimationState(Mob);
	return State && State->bUsingVertexAnimation && State->Material;
}

#if !UE_BUILD_SHIPPING
bool UT66MobManagerSubsystem::RunMobTickVatRuntimeProof()
{
	AT66MobBase* FamilyMobs[4] = { nullptr, nullptr, nullptr, nullptr };
	auto FamilyToIndex = [](const ET66EnemyFamily Family) -> int32
	{
		switch (Family)
		{
		case ET66EnemyFamily::Melee: return 0;
		case ET66EnemyFamily::Rush: return 1;
		case ET66EnemyFamily::Flying: return 2;
		case ET66EnemyFamily::Ranged: return 3;
		default: return INDEX_NONE;
		}
	};

	int32 ActiveMobCount = 0;
	int32 ActorTickEnabledCount = 0;
	int32 ComponentTickEnabledCount = 0;
	int32 ComponentTickRegisteredCount = 0;
	for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ActiveMobs)
	{
		AT66MobBase* Mob = WeakMob.Get();
		if (!Mob || Mob->LifecycleState != ET66MobLifecycleState::Active)
		{
			continue;
		}

		++ActiveMobCount;
		if (Mob->IsActorTickEnabled())
		{
			++ActorTickEnabledCount;
		}

		TArray<UActorComponent*> Components;
		Mob->GetComponents(Components);
		for (UActorComponent* Component : Components)
		{
			if (!Component)
			{
				continue;
			}
			if (Component->IsComponentTickEnabled())
			{
				++ComponentTickEnabledCount;
			}
			if (Component->PrimaryComponentTick.IsTickFunctionRegistered())
			{
				++ComponentTickRegisteredCount;
			}
		}

		const int32 FamilyIndex = FamilyToIndex(Mob->GetEnemyFamily());
		if (FamilyIndex != INDEX_NONE && !FamilyMobs[FamilyIndex] && IsMobUsingVertexAnimation(Mob))
		{
			FamilyMobs[FamilyIndex] = Mob;
		}
	}

	int32 FamiliesPresent = 0;
	int32 ClipSamplesWithFrameChange = 0;
	int32 ClipSamplesTotal = 0;
	int32 PoolReuseResetChecks = 0;
	int32 PoolReuseResetPasses = 0;
	static const FName ProofClips[] =
	{
		T66ManagedMobVATClip_Idle,
		T66ManagedMobVATClip_Move,
		T66ManagedMobVATClip_AttackCue,
		T66ManagedMobVATClip_Death
	};

	for (AT66MobBase* Mob : FamilyMobs)
	{
		if (!Mob)
		{
			continue;
		}
		++FamiliesPresent;

		for (const FName ClipName : ProofClips)
		{
			FT66MobVertexAnimationRuntimeState* State = FindMobVertexAnimationState(Mob);
			if (!State || !State->bUsingVertexAnimation || !State->Material)
			{
				continue;
			}

			SetMobVertexAnimationClip(Mob, ClipName, 1.0f);
			State = FindMobVertexAnimationState(Mob);
			if (!State)
			{
				continue;
			}
			const float Frame0 = State->CustomDataFrame;
			TickMobVertexAnimationState(Mob, 0.12f);
			State = FindMobVertexAnimationState(Mob);
			const float Frame1 = State ? State->CustomDataFrame : Frame0;
			TickMobVertexAnimationState(Mob, 0.12f);
			State = FindMobVertexAnimationState(Mob);
			const float Frame2 = State ? State->CustomDataFrame : Frame1;
			++ClipSamplesTotal;
			if (!FMath::IsNearlyEqual(Frame0, Frame1) || !FMath::IsNearlyEqual(Frame1, Frame2))
			{
				++ClipSamplesWithFrameChange;
			}
		}

		if (FT66MobVertexAnimationRuntimeState* State = FindMobVertexAnimationState(Mob))
		{
			++PoolReuseResetChecks;
			const FT66MobVertexAnimationRow ExistingRow = State->Row;
			UMaterialInstanceDynamic* ExistingMaterial = State->Material;
			ConfigureMobVertexAnimationState(Mob, ExistingRow, ExistingMaterial);
			State = FindMobVertexAnimationState(Mob);
			if (State
				&& State->bUsingVertexAnimation
				&& State->Clip == T66ManagedMobVATClip_Idle
				&& FMath::IsNearlyZero(State->ClipTime)
				&& FMath::IsNearlyEqual(State->CustomDataFrame, State->CustomDataStartFrame))
			{
				++PoolReuseResetPasses;
			}
		}
	}

	const bool bPass = ActiveMobCount > 0
		&& FamiliesPresent == 4
		&& ActorTickEnabledCount == 0
		&& ComponentTickEnabledCount == 0
		&& ComponentTickRegisteredCount == 0
		&& ClipSamplesTotal == 16
		&& ClipSamplesWithFrameChange == ClipSamplesTotal
		&& PoolReuseResetChecks == 4
		&& PoolReuseResetPasses == PoolReuseResetChecks;

	UE_LOG(
		LogT66MobManager,
		Log,
		TEXT("[MobTickVatRuntimeProofSummary] Terminal=1 ActiveMobs=%d FamiliesPresent=%d ActorTickEnabled=%d ComponentTickEnabled=%d ComponentTickRegistered=%d ClipSamples=%d ClipSamplesWithFrameChange=%d PoolReuseResetChecks=%d PoolReuseResetPasses=%d ClassMap=Melee/Rush/Flying/Ranged:AT66MobBase DataSource=Enemies.csv+T66EnemyDirectorBasicRouting Pass=%d"),
		ActiveMobCount,
		FamiliesPresent,
		ActorTickEnabledCount,
		ComponentTickEnabledCount,
		ComponentTickRegisteredCount,
		ClipSamplesTotal,
		ClipSamplesWithFrameChange,
		PoolReuseResetChecks,
		PoolReuseResetPasses,
		bPass ? 1 : 0);

	return bPass;
}
#endif

void UT66MobManagerSubsystem::ResetRangedPressureDiagnostics(const TCHAR* Reason)
{
	const FString ResetReason = Reason ? FString(Reason) : FString(TEXT("Unknown"));
	RangedDiagnostics.Reset(ResetReason);
	RouteAttributionDiagnostics.Reset(ResetReason);
	bRangedDiagnosticTerminalSummaryEmitted = false;
}

void UT66MobManagerSubsystem::RecordRangedMobSpawn(const bool bLightweight, const FName MobID)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedMobSpawn")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichRangedSpawns, RangedDiagnostics.LightweightRangedSpawns);
	(void)MobID;
}

void UT66MobManagerSubsystem::RecordRangedFireAttempt(const bool bLightweight, const FName MobID, const float Dist2D)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedFireAttempt")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichFireAttempts, RangedDiagnostics.LightweightFireAttempts);
	(void)MobID;
	(void)Dist2D;
}

void UT66MobManagerSubsystem::RecordRangedStatusBlocked(const bool bLightweight, const FName MobID, const float Dist2D)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedStatusBlocked")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichStatusBlocked, RangedDiagnostics.LightweightStatusBlocked);
	(void)MobID;
	(void)Dist2D;
}

void UT66MobManagerSubsystem::RecordRangedCooldownBlocked(const bool bLightweight, const FName MobID, const float Dist2D, const float CooldownRemaining)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedCooldownBlocked")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichCooldownBlocked, RangedDiagnostics.LightweightCooldownBlocked);
	(void)MobID;
	(void)Dist2D;
	(void)CooldownRemaining;
}

void UT66MobManagerSubsystem::RecordRangedDistancePassed(const bool bLightweight, const FName MobID, const float Dist2D, const float DesiredMinRange, const float DesiredMaxRange, const float MaxFireRange)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedDistancePassed")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichDistancePass, RangedDiagnostics.LightweightDistancePass);
	(void)MobID;
	(void)Dist2D;
	(void)DesiredMinRange;
	(void)DesiredMaxRange;
	(void)MaxFireRange;
}

void UT66MobManagerSubsystem::RecordRangedFireSkippedSafeZone(const bool bLightweight, const FName MobID, const float Dist2D)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedFireSkippedSafeZone")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichSafeZoneSkips, RangedDiagnostics.LightweightSafeZoneSkips);
	(void)MobID;
	(void)Dist2D;
}

void UT66MobManagerSubsystem::RecordRangedFireSkippedOutOfRange(const bool bLightweight, const FName MobID, const float Dist2D, const float MaxFireRange)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedFireSkippedOutOfRange")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichOutOfRangeSkips, RangedDiagnostics.LightweightOutOfRangeSkips);
	(void)MobID;
	(void)Dist2D;
	(void)MaxFireRange;
}

void UT66MobManagerSubsystem::RecordRangedLosBlocked(const bool bLightweight, const FName MobID, const float Dist2D, const AActor* BlockerActor, const UPrimitiveComponent* BlockerComponent)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedLosBlocked")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichLosBlocked, RangedDiagnostics.LightweightLosBlocked);
	IncrementLosBlockerBucket(RangedDiagnostics, bLightweight, ClassifyRangedLosBlocker(BlockerActor, BlockerComponent));
	(void)MobID;
	(void)Dist2D;
}

void UT66MobManagerSubsystem::RecordRangedLosPassed(const bool bLightweight, const FName MobID, const float Dist2D)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedLosPassed")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichLosPass, RangedDiagnostics.LightweightLosPass);
	(void)MobID;
	(void)Dist2D;
}

void UT66MobManagerSubsystem::RecordRangedDispatchReached(const bool bLightweight, const FName MobID, const float Dist2D)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedDispatchReached")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichDispatchReached, RangedDiagnostics.LightweightDispatchReached);
	(void)MobID;
	(void)Dist2D;
}

void UT66MobManagerSubsystem::RecordRangedProjectileSpawned(const bool bLightweight, const FName MobID)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedProjectileSpawned")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichProjectileSpawned, RangedDiagnostics.LightweightProjectileSpawned);
	(void)MobID;
}

void UT66MobManagerSubsystem::RecordRangedProjectileSpawnFailed(const bool bLightweight, const FName MobID)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedProjectileSpawnFailed")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichProjectileSpawnFailed, RangedDiagnostics.LightweightProjectileSpawnFailed);
	(void)MobID;
}

void UT66MobManagerSubsystem::RecordRangedZeroDirectionShot(const bool bLightweight, const FName MobID)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRangedZeroDirectionShot")))
	{
		return;
	}
	IncrementPathCounter(bLightweight, RangedDiagnostics.RichZeroDirectionShots, RangedDiagnostics.LightweightZeroDirectionShots);
	(void)MobID;
}

void UT66MobManagerSubsystem::RecordEnemyProjectileOwnerIgnored(const AActor* Projectile, const AActor* Owner)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordEnemyProjectileOwnerIgnored")))
	{
		return;
	}
	++RangedDiagnostics.ProjectileOwnerIgnored;
	(void)Projectile;
	(void)Owner;
}

void UT66MobManagerSubsystem::RecordEnemyProjectileNonHeroImpact(const AActor* Projectile, const AActor* Owner, const AActor* OtherActor)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordEnemyProjectileNonHeroImpact")))
	{
		return;
	}
	++RangedDiagnostics.ProjectileNonHeroImpacts;
	(void)Projectile;
	(void)Owner;
	(void)OtherActor;
}

void UT66MobManagerSubsystem::RecordEnemyProjectileHeroHurtboxReject(const AActor* Projectile, const AActor* Owner, const AT66HeroBase* Hero)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordEnemyProjectileHeroHurtboxReject")))
	{
		return;
	}
	++RangedDiagnostics.ProjectileHeroHurtboxRejects;
	(void)Projectile;
	(void)Owner;
	(void)Hero;
}

void UT66MobManagerSubsystem::RecordEnemyProjectileSafeZoneReject(const AActor* Projectile, const AActor* Owner, const AT66HeroBase* Hero)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordEnemyProjectileSafeZoneReject")))
	{
		return;
	}
	++RangedDiagnostics.ProjectileSafeZoneRejects;
	(void)Projectile;
	(void)Owner;
	(void)Hero;
}

void UT66MobManagerSubsystem::RecordEnemyProjectileHeroHit(const AActor* Projectile, const AActor* Owner, const AT66HeroBase* Hero, const int32 DamageHP)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordEnemyProjectileHeroHit")))
	{
		return;
	}
	FName OwnerMobID = NAME_None;
	FString OwnerClass;
	const ET66RangedDiagnosticPath Path = ResolveRangedDiagnosticPathFromOwner(Owner, OwnerMobID, OwnerClass);
	++RangedDiagnostics.ProjectileHeroHits;
	if (Path == ET66RangedDiagnosticPath::Rich)
	{
		++RangedDiagnostics.RichProjectileHeroHits;
	}
	else if (Path == ET66RangedDiagnosticPath::Lightweight)
	{
		++RangedDiagnostics.LightweightProjectileHeroHits;
	}
	else
	{
		++RangedDiagnostics.UnknownProjectileHeroHits;
	}

	RangedDiagnostics.ProjectileDamageHP += FMath::Max(0, DamageHP);
	if (const UWorld* World = GetWorld())
	{
		const float WorldTime = World->GetTimeSeconds();
		if (RangedDiagnostics.FirstHeroProjectileHitWorldTime < 0.f)
		{
			RangedDiagnostics.FirstHeroProjectileHitWorldTime = WorldTime;
		}
		RangedDiagnostics.LastHeroProjectileHitWorldTime = WorldTime;
	}
	RangedDiagnostics.LastHeroHP = Hero ? ResolveRunStateHeroHP(GetWorld()) : -1.f;
	RangedDiagnostics.LastHeroHitMobID = OwnerMobID;
	RangedDiagnostics.LastHeroHitOwnerName = GetNameSafe(Owner);
	RangedDiagnostics.LastHeroHitOwnerClass = OwnerClass;
	(void)Projectile;
}

void UT66MobManagerSubsystem::RecordManagedEnemyProjectileOwnerIgnored(const AActor* Owner, const FName OwnerMobID, const bool bLightweight)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordManagedEnemyProjectileOwnerIgnored")))
	{
		return;
	}
	++RangedDiagnostics.ProjectileOwnerIgnored;
	(void)Owner;
	(void)OwnerMobID;
	(void)bLightweight;
}

void UT66MobManagerSubsystem::RecordManagedEnemyProjectileWorldImpact(const AActor* Owner, const FName OwnerMobID, const bool bLightweight, const AActor* HitActor)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordManagedEnemyProjectileWorldImpact")))
	{
		return;
	}
	++RangedDiagnostics.ProjectileNonHeroImpacts;
	(void)Owner;
	(void)OwnerMobID;
	(void)bLightweight;
	(void)HitActor;
}

void UT66MobManagerSubsystem::RecordManagedEnemyProjectileSafeZoneReject(const AActor* Owner, const FName OwnerMobID, const bool bLightweight, const AT66HeroBase* Hero)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordManagedEnemyProjectileSafeZoneReject")))
	{
		return;
	}
	++RangedDiagnostics.ProjectileSafeZoneRejects;
	(void)Owner;
	(void)OwnerMobID;
	(void)bLightweight;
	(void)Hero;
}

void UT66MobManagerSubsystem::RecordManagedEnemyProjectileHeroHit(const AActor* Owner, const FName OwnerMobID, const bool bLightweight, const AT66HeroBase* Hero, const int32 DamageHP)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordManagedEnemyProjectileHeroHit")))
	{
		return;
	}
	++RangedDiagnostics.ProjectileHeroHits;
	if (bLightweight)
	{
		++RangedDiagnostics.LightweightProjectileHeroHits;
	}
	else
	{
		++RangedDiagnostics.RichProjectileHeroHits;
	}

	RangedDiagnostics.ProjectileDamageHP += FMath::Max(0, DamageHP);
	if (const UWorld* World = GetWorld())
	{
		const float WorldTime = World->GetTimeSeconds();
		if (RangedDiagnostics.FirstHeroProjectileHitWorldTime < 0.f)
		{
			RangedDiagnostics.FirstHeroProjectileHitWorldTime = WorldTime;
		}
		RangedDiagnostics.LastHeroProjectileHitWorldTime = WorldTime;
	}
	RangedDiagnostics.LastHeroHP = Hero ? ResolveRunStateHeroHP(GetWorld()) : -1.f;
	RangedDiagnostics.LastHeroHitMobID = OwnerMobID;
	RangedDiagnostics.LastHeroHitOwnerName = Owner ? GetNameSafe(Owner) : OwnerMobID.ToString();
	RangedDiagnostics.LastHeroHitOwnerClass = Owner && Owner->GetClass() ? Owner->GetClass()->GetName() : FString(TEXT("InvalidSource"));
}

void UT66MobManagerSubsystem::RecordRouteAttribution(
	const ET66EnemyFamily Family,
	const ET66RouteAttributionReason Reason,
	const ET66RouteAttributionChannel Channel)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordRouteAttribution")))
	{
		return;
	}

	FT66RouteAttributionFamilyCounters& FamilyCounters = GetRouteFamilyCounters(RouteAttributionDiagnostics, Family);
	++RouteAttributionDiagnostics.TotalObservedSpawns;
	++FamilyCounters.Total;

	if (Channel == ET66RouteAttributionChannel::NonDirector)
	{
		++RouteAttributionDiagnostics.NonDirectorObservedSpawns;
	}
	else
	{
		++RouteAttributionDiagnostics.DirectorObservedSpawns;
		if (Channel == ET66RouteAttributionChannel::InitialPopulation)
		{
			++RouteAttributionDiagnostics.InitialPopulationSpawns;
		}
		else
		{
			++RouteAttributionDiagnostics.RuntimeTrickleSpawns;
		}
	}

	switch (Reason)
	{
	case ET66RouteAttributionReason::RoutedLightweight_BasicFamily:
		++FamilyCounters.RoutedLightweightBasic;
		break;
	case ET66RouteAttributionReason::RoutedRich_SpecialOrMiniBoss:
		++FamilyCounters.RoutedRichSpecialOrMiniBoss;
		++RouteAttributionDiagnostics.SpecialSlots;
		break;
	case ET66RouteAttributionReason::RoutedRich_MiniBossPromotion:
		++FamilyCounters.RoutedRichMiniBossPromotion;
		++RouteAttributionDiagnostics.MiniBossPromotionSlots;
		break;
	case ET66RouteAttributionReason::RoutedRich_FamilyLookupFailed:
		++FamilyCounters.RoutedRichFamilyLookupFailed;
		break;
	case ET66RouteAttributionReason::RoutedRich_FallbackBranch:
		++FamilyCounters.RoutedRichFallbackBranch;
		break;
	case ET66RouteAttributionReason::RoutedRich_NonDirectorPath:
		++FamilyCounters.RoutedRichNonDirectorPath;
		break;
	default:
		++FamilyCounters.RoutedRichFamilyLookupFailed;
		break;
	}
}

void UT66MobManagerSubsystem::RecordLightweightAcquireFailed(const ET66EnemyFamily Family, const ET66RouteAttributionChannel Channel)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordLightweightAcquireFailed")))
	{
		return;
	}
	++RouteAttributionDiagnostics.LightweightAcquireFailed;
	(void)Family;
	(void)Channel;
}

void UT66MobManagerSubsystem::RecordBossOrGuardianRouteAttribution(const ET66EnemyFamily Family)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("RecordBossOrGuardianRouteAttribution")))
	{
		return;
	}
	++RouteAttributionDiagnostics.BossOrGuardianObserved;
	RecordRouteAttribution(Family, ET66RouteAttributionReason::RoutedRich_NonDirectorPath, ET66RouteAttributionChannel::NonDirector);
}

void UT66MobManagerSubsystem::EmitRangedPressureSummary(const TCHAR* Reason, const bool bTerminal)
{
	if (!ShouldTrackRangedDiagnostics(TEXT("EmitRangedPressureSummary")))
	{
		return;
	}
	if (bTerminal && bRangedDiagnosticTerminalSummaryEmitted)
	{
		return;
	}
	if (bTerminal)
	{
		bRangedDiagnosticTerminalSummaryEmitted = true;
	}

	UWorld* World = GetWorld();
	const AT66HeroBase* Hero = ResolveLocalHero(World);
	const float HeroHP = Hero ? ResolveRunStateHeroHP(World) : -1.f;
	const float WorldTime = World ? World->GetTimeSeconds() : -1.f;
	UE_LOG(
		LogT66MobManager,
		Log,
		TEXT("[RangedDecisionSummary] Reason=%s Terminal=%d WorldTime=%.2f HeroHP=%.1f ResetCount=%d ResetReason=%s RichSpawns=%d LightweightSpawns=%d RichFireAttempts=%d LightweightFireAttempts=%d RichCooldownBlocked=%d LightweightCooldownBlocked=%d RichStatusBlocked=%d LightweightStatusBlocked=%d RichSafeZoneBlocked=%d LightweightSafeZoneBlocked=%d RichDistanceBlocked=%d LightweightDistanceBlocked=%d RichDistancePassed=%d LightweightDistancePassed=%d RichLOSBlocked=%d LightweightLOSBlocked=%d RichLOSBlockerWorldStatic=%d LightweightLOSBlockerWorldStatic=%d RichLOSBlockerWorldDynamic=%d LightweightLOSBlockerWorldDynamic=%d RichLOSBlockerRichEnemy=%d LightweightLOSBlockerRichEnemy=%d RichLOSBlockerLightweightMob=%d LightweightLOSBlockerLightweightMob=%d RichLOSBlockerOtherPawn=%d LightweightLOSBlockerOtherPawn=%d RichLOSBlockerUnknown=%d LightweightLOSBlockerUnknown=%d RichLOSPassed=%d LightweightLOSPassed=%d RichProjectilesDispatched=%d LightweightProjectilesDispatched=%d RichProjectilesSpawned=%d LightweightProjectilesSpawned=%d RichSpawnFailed=%d LightweightSpawnFailed=%d RichZeroDirection=%d LightweightZeroDirection=%d ProjectileOwnerIgnored=%d ProjectileNonHeroImpacts=%d ProjectileHeroHurtboxRejects=%d ProjectileSafeZoneRejects=%d ProjectilesHitHero=%d RichProjectilesHitHero=%d LightweightProjectilesHitHero=%d UnknownProjectilesHitHero=%d ProjectileDamageHP=%d FirstHeroHitTime=%.2f LastHeroHitTime=%.2f LastHitMobID=%s LastHitOwner=%s LastHitOwnerClass=%s LastHeroHPBeforeHit=%.1f"),
		Reason ? Reason : TEXT("Unknown"),
		bTerminal ? 1 : 0,
		WorldTime,
		HeroHP,
		RangedDiagnostics.ResetCount,
		*RangedDiagnostics.LastResetReason,
		RangedDiagnostics.RichRangedSpawns,
		RangedDiagnostics.LightweightRangedSpawns,
		RangedDiagnostics.RichFireAttempts,
		RangedDiagnostics.LightweightFireAttempts,
		RangedDiagnostics.RichCooldownBlocked,
		RangedDiagnostics.LightweightCooldownBlocked,
		RangedDiagnostics.RichStatusBlocked,
		RangedDiagnostics.LightweightStatusBlocked,
		RangedDiagnostics.RichSafeZoneSkips,
		RangedDiagnostics.LightweightSafeZoneSkips,
		RangedDiagnostics.RichOutOfRangeSkips,
		RangedDiagnostics.LightweightOutOfRangeSkips,
		RangedDiagnostics.RichDistancePass,
		RangedDiagnostics.LightweightDistancePass,
		RangedDiagnostics.RichLosBlocked,
		RangedDiagnostics.LightweightLosBlocked,
		RangedDiagnostics.RichLosBlockerWorldStatic,
		RangedDiagnostics.LightweightLosBlockerWorldStatic,
		RangedDiagnostics.RichLosBlockerWorldDynamic,
		RangedDiagnostics.LightweightLosBlockerWorldDynamic,
		RangedDiagnostics.RichLosBlockerRichEnemy,
		RangedDiagnostics.LightweightLosBlockerRichEnemy,
		RangedDiagnostics.RichLosBlockerLightweightMob,
		RangedDiagnostics.LightweightLosBlockerLightweightMob,
		RangedDiagnostics.RichLosBlockerOtherPawn,
		RangedDiagnostics.LightweightLosBlockerOtherPawn,
		RangedDiagnostics.RichLosBlockerUnknown,
		RangedDiagnostics.LightweightLosBlockerUnknown,
		RangedDiagnostics.RichLosPass,
		RangedDiagnostics.LightweightLosPass,
		RangedDiagnostics.RichDispatchReached,
		RangedDiagnostics.LightweightDispatchReached,
		RangedDiagnostics.RichProjectileSpawned,
		RangedDiagnostics.LightweightProjectileSpawned,
		RangedDiagnostics.RichProjectileSpawnFailed,
		RangedDiagnostics.LightweightProjectileSpawnFailed,
		RangedDiagnostics.RichZeroDirectionShots,
		RangedDiagnostics.LightweightZeroDirectionShots,
		RangedDiagnostics.ProjectileOwnerIgnored,
		RangedDiagnostics.ProjectileNonHeroImpacts,
		RangedDiagnostics.ProjectileHeroHurtboxRejects,
		RangedDiagnostics.ProjectileSafeZoneRejects,
		RangedDiagnostics.ProjectileHeroHits,
		RangedDiagnostics.RichProjectileHeroHits,
		RangedDiagnostics.LightweightProjectileHeroHits,
		RangedDiagnostics.UnknownProjectileHeroHits,
		RangedDiagnostics.ProjectileDamageHP,
		RangedDiagnostics.FirstHeroProjectileHitWorldTime,
		RangedDiagnostics.LastHeroProjectileHitWorldTime,
		RangedDiagnostics.LastHeroHitMobID.IsNone() ? TEXT("None") : *RangedDiagnostics.LastHeroHitMobID.ToString(),
		RangedDiagnostics.LastHeroHitOwnerName.IsEmpty() ? TEXT("None") : *RangedDiagnostics.LastHeroHitOwnerName,
		RangedDiagnostics.LastHeroHitOwnerClass.IsEmpty() ? TEXT("None") : *RangedDiagnostics.LastHeroHitOwnerClass,
		RangedDiagnostics.LastHeroHP);

	const FT66RouteAttributionFamilyCounters& Melee = RouteAttributionDiagnostics.Melee;
	const FT66RouteAttributionFamilyCounters& Rush = RouteAttributionDiagnostics.Rush;
	const FT66RouteAttributionFamilyCounters& Flying = RouteAttributionDiagnostics.Flying;
	const FT66RouteAttributionFamilyCounters& Ranged = RouteAttributionDiagnostics.Ranged;
	const FT66RouteAttributionFamilyCounters& SpecialUnknown = RouteAttributionDiagnostics.SpecialUnknown;
	UE_LOG(
		LogT66MobManager,
		Log,
		TEXT("[RouteAttributionSummary] Reason=%s Terminal=%d WorldTime=%.2f ResetCount=%d ResetReason=%s TotalObservedSpawns=%d DirectorObservedSpawns=%d InitialPopulationSpawns=%d RuntimeTrickleSpawns=%d NonDirectorObservedSpawns=%d CounterMismatch=%d LightweightAcquireFailed=%d MiniBossPromotionSlots=%d SpecialSlots=%d BossOrGuardianObserved=%d MeleeTotal=%d MeleeRoutedLightweightBasic=%d MeleeRoutedRichSpecialOrMiniBoss=%d MeleeRoutedRichMiniBossPromotion=%d MeleeRoutedRichFamilyLookupFailed=%d MeleeRoutedRichFallbackBranch=%d MeleeRoutedRichNonDirectorPath=%d RushTotal=%d RushRoutedLightweightBasic=%d RushRoutedRichSpecialOrMiniBoss=%d RushRoutedRichMiniBossPromotion=%d RushRoutedRichFamilyLookupFailed=%d RushRoutedRichFallbackBranch=%d RushRoutedRichNonDirectorPath=%d FlyingTotal=%d FlyingRoutedLightweightBasic=%d FlyingRoutedRichSpecialOrMiniBoss=%d FlyingRoutedRichMiniBossPromotion=%d FlyingRoutedRichFamilyLookupFailed=%d FlyingRoutedRichFallbackBranch=%d FlyingRoutedRichNonDirectorPath=%d RangedTotal=%d RangedRoutedLightweightBasic=%d RangedRoutedRichSpecialOrMiniBoss=%d RangedRoutedRichMiniBossPromotion=%d RangedRoutedRichFamilyLookupFailed=%d RangedRoutedRichFallbackBranch=%d RangedRoutedRichNonDirectorPath=%d SpecialUnknownTotal=%d SpecialUnknownRoutedRichSpecialOrMiniBoss=%d SpecialUnknownRoutedRichFamilyLookupFailed=%d SpecialUnknownRoutedRichFallbackBranch=%d SpecialUnknownRoutedRichNonDirectorPath=%d"),
		Reason ? Reason : TEXT("Unknown"),
		bTerminal ? 1 : 0,
		WorldTime,
		RouteAttributionDiagnostics.ResetCount,
		*RouteAttributionDiagnostics.LastResetReason,
		RouteAttributionDiagnostics.TotalObservedSpawns,
		RouteAttributionDiagnostics.DirectorObservedSpawns,
		RouteAttributionDiagnostics.InitialPopulationSpawns,
		RouteAttributionDiagnostics.RuntimeTrickleSpawns,
		RouteAttributionDiagnostics.NonDirectorObservedSpawns,
		RouteAttributionDiagnostics.CalculateCounterMismatch(),
		RouteAttributionDiagnostics.LightweightAcquireFailed,
		RouteAttributionDiagnostics.MiniBossPromotionSlots,
		RouteAttributionDiagnostics.SpecialSlots,
		RouteAttributionDiagnostics.BossOrGuardianObserved,
		Melee.Total,
		Melee.RoutedLightweightBasic,
		Melee.RoutedRichSpecialOrMiniBoss,
		Melee.RoutedRichMiniBossPromotion,
		Melee.RoutedRichFamilyLookupFailed,
		Melee.RoutedRichFallbackBranch,
		Melee.RoutedRichNonDirectorPath,
		Rush.Total,
		Rush.RoutedLightweightBasic,
		Rush.RoutedRichSpecialOrMiniBoss,
		Rush.RoutedRichMiniBossPromotion,
		Rush.RoutedRichFamilyLookupFailed,
		Rush.RoutedRichFallbackBranch,
		Rush.RoutedRichNonDirectorPath,
		Flying.Total,
		Flying.RoutedLightweightBasic,
		Flying.RoutedRichSpecialOrMiniBoss,
		Flying.RoutedRichMiniBossPromotion,
		Flying.RoutedRichFamilyLookupFailed,
		Flying.RoutedRichFallbackBranch,
		Flying.RoutedRichNonDirectorPath,
		Ranged.Total,
		Ranged.RoutedLightweightBasic,
		Ranged.RoutedRichSpecialOrMiniBoss,
		Ranged.RoutedRichMiniBossPromotion,
		Ranged.RoutedRichFamilyLookupFailed,
		Ranged.RoutedRichFallbackBranch,
		Ranged.RoutedRichNonDirectorPath,
		SpecialUnknown.Total,
		SpecialUnknown.RoutedRichSpecialOrMiniBoss,
		SpecialUnknown.RoutedRichFamilyLookupFailed,
		SpecialUnknown.RoutedRichFallbackBranch,
		SpecialUnknown.RoutedRichNonDirectorPath);
}

AT66MobBase* UT66MobManagerSubsystem::AcquireMob(UClass* MobClass, const FTransform& SpawnTransform, bool* bOutRequiresFinishSpawning)
{
	const bool bProfileEnabled = IsManagerTickProfileEnabled();
	const uint64 ProfileStartCycles = bProfileEnabled ? FPlatformTime::Cycles64() : 0;
	UWorld* World = GetWorld();
	if (!World)
	{
		AccumulatePoolAcquireProfile(bProfileEnabled, ProfileStartCycles);
		return nullptr;
	}

	if (bOutRequiresFinishSpawning)
	{
		*bOutRequiresFinishSpawning = false;
	}

	UClass* ResolvedClass = MobClass && MobClass->IsChildOf(AT66MobBase::StaticClass())
		? MobClass
		: AT66MobBase::StaticClass();

	InactiveMobs.RemoveAllSwap([](const TWeakObjectPtr<AT66MobBase>& WeakMob)
	{
		return !WeakMob.IsValid();
	}, EAllowShrinking::No);

	for (int32 Index = InactiveMobs.Num() - 1; Index >= 0; --Index)
	{
		AT66MobBase* Mob = InactiveMobs[Index].Get();
		if (!Mob || Mob->GetClass() != ResolvedClass)
		{
			continue;
		}

		InactiveMobs.RemoveAtSwap(Index, 1, EAllowShrinking::No);
		Mob->ResetForReuse();
		Mob->SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
		SetMobRuntimeActive(Mob, true);
		RegisterMob(Mob);
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->RegisterMob(Mob);
		}

		++PoolReuseAcquireCount;
		UE_LOG(LogT66MobManager, VeryVerbose, TEXT("AcquireMob reused mob=%s class=%s ActiveMobs.Num()=%d InactiveMobs.Num()=%d reuseCount=%d"),
			*GetNameSafe(Mob),
			*GetNameSafe(ResolvedClass),
			ActiveMobs.Num(),
			InactiveMobs.Num(),
			PoolReuseAcquireCount);
		AccumulatePoolAcquireProfile(bProfileEnabled, ProfileStartCycles);
		return Mob;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AT66MobBase* SpawnedMob = World->SpawnActorDeferred<AT66MobBase>(
		ResolvedClass,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!SpawnedMob)
	{
		AccumulatePoolAcquireProfile(bProfileEnabled, ProfileStartCycles);
		return nullptr;
	}

	if (bOutRequiresFinishSpawning)
	{
		*bOutRequiresFinishSpawning = true;
	}
	++PoolFreshSpawnCount;
	UE_LOG(LogT66MobManager, VeryVerbose, TEXT("AcquireMob fresh mob=%s class=%s ActiveMobs.Num()=%d InactiveMobs.Num()=%d freshCount=%d"),
		*GetNameSafe(SpawnedMob),
		*GetNameSafe(ResolvedClass),
		ActiveMobs.Num(),
		InactiveMobs.Num(),
		PoolFreshSpawnCount);
	AccumulatePoolAcquireProfile(bProfileEnabled, ProfileStartCycles);
	return SpawnedMob;
}

void UT66MobManagerSubsystem::ReleaseMob(AT66MobBase* Mob)
{
	const bool bProfileEnabled = IsManagerTickProfileEnabled();
	const uint64 ProfileStartCycles = bProfileEnabled ? FPlatformTime::Cycles64() : 0;
	if (!IsValid(Mob))
	{
		AccumulatePoolReleaseProfile(bProfileEnabled, ProfileStartCycles);
		return;
	}

	++PoolReleaseCount;
	UnregisterMob(Mob);
	if (UWorld* World = GetWorld())
	{
		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			Registry->UnregisterMob(Mob);
		}
	}

	Mob->OwningDirector = nullptr;
	Mob->StoredVelocity = FVector::ZeroVector;
	Mob->KnockbackVelocity = FVector::ZeroVector;
	Mob->bIsTouchingHero = false;
	Mob->TouchDamageCooldownSeconds = 0.f;
	Mob->LifecycleState = ET66MobLifecycleState::Pooled;
	SetMobRuntimeActive(Mob, false);

	if (InactiveMobs.Num() < MaxInactiveMobs)
	{
		Mob->SetActorLocation(T66MobPoolParkingLocation, false, nullptr, ETeleportType::TeleportPhysics);
		InactiveMobs.AddUnique(Mob);
		PeakInactiveMobCount = FMath::Max(PeakInactiveMobCount, InactiveMobs.Num());
		UE_LOG(LogT66MobManager, VeryVerbose, TEXT("ReleaseMob pooled mob=%s MobID=%s ActiveMobs.Num()=%d InactiveMobs.Num()=%d releaseCount=%d"),
			*GetNameSafe(Mob),
			*GetMobIDForLog(Mob),
			ActiveMobs.Num(),
			InactiveMobs.Num(),
			PoolReleaseCount);
		AccumulatePoolReleaseProfile(bProfileEnabled, ProfileStartCycles);
		return;
	}

	++PoolOverflowDestroyCount;
	UE_LOG(LogT66MobManager, VeryVerbose, TEXT("ReleaseMob overflow destroy mob=%s MobID=%s maxPool=%d overflowCount=%d"),
		*GetNameSafe(Mob),
		*GetMobIDForLog(Mob),
		MaxInactiveMobs,
		PoolOverflowDestroyCount);
	Mob->Destroy();
	AccumulatePoolReleaseProfile(bProfileEnabled, ProfileStartCycles);
}

void UT66MobManagerSubsystem::RegisterMob(AT66MobBase* Mob)
{
	if (!Mob)
	{
		return;
	}

	ActiveMobs.RemoveAllSwap([](const TWeakObjectPtr<AT66MobBase>& WeakMob)
	{
		return !WeakMob.IsValid();
	}, EAllowShrinking::No);
	ActiveMobVertexAnimationStates.RemoveAllSwap([](const FT66MobVertexAnimationRuntimeState& State)
	{
		return !State.Mob.IsValid();
	}, EAllowShrinking::No);

	ActiveMobs.AddUnique(Mob);

	UE_LOG(LogT66MobManager, VeryVerbose, TEXT("RegisterMob mob=%s MobID=%s ActiveMobs.Num()=%d"),
		*GetNameSafe(Mob),
		*GetMobIDForLog(Mob),
		ActiveMobs.Num());
}

void UT66MobManagerSubsystem::UnregisterMob(AT66MobBase* Mob)
{
	if (!Mob)
	{
		return;
	}

	ActiveMobs.RemoveAllSwap([Mob](const TWeakObjectPtr<AT66MobBase>& WeakMob)
	{
		return !WeakMob.IsValid() || WeakMob.Get() == Mob;
	}, EAllowShrinking::No);
	ClearMobVertexAnimationState(Mob);

	UE_LOG(LogT66MobManager, VeryVerbose, TEXT("UnregisterMob mob=%s MobID=%s ActiveMobs.Num()=%d"),
		*GetNameSafe(Mob),
		*GetMobIDForLog(Mob),
		ActiveMobs.Num());
}

void UT66MobManagerSubsystem::NotifyMobDying(AT66MobBase* Mob)
{
	if (!Mob)
	{
		return;
	}

	Mob->LifecycleState = ET66MobLifecycleState::Dying;
	UE_LOG(LogT66MobManager, Verbose, TEXT("NotifyMobDying mob=%s MobID=%s finalLocation=%s"),
		*GetNameSafe(Mob),
		*GetMobIDForLog(Mob),
		*Mob->GetActorLocation().ToCompactString());

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UT66MobManagerSubsystem> WeakThis(this);
		TWeakObjectPtr<AT66MobBase> WeakMob(Mob);
		FTimerDelegate ReleaseDelegate = FTimerDelegate::CreateLambda([WeakThis, WeakMob]()
		{
			if (UT66MobManagerSubsystem* Manager = WeakThis.Get())
			{
				if (AT66MobBase* MobToRelease = WeakMob.Get())
				{
					Manager->ReleaseMob(MobToRelease);
				}
			}
		});
		World->GetTimerManager().SetTimerForNextTick(ReleaseDelegate);
	}
}

void UT66MobManagerSubsystem::SetBackroomsGameplayPaused(const bool bPaused)
{
	if (bBackroomsGameplayPaused == bPaused)
	{
		return;
	}

	bBackroomsGameplayPaused = bPaused;
	if (bBackroomsGameplayPaused)
	{
		for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ActiveMobs)
		{
			if (AT66MobBase* Mob = WeakMob.Get())
			{
				Mob->StoredVelocity = FVector::ZeroVector;
				Mob->KnockbackVelocity = FVector::ZeroVector;
				Mob->bIsTouchingHero = false;
			}
		}
	}
}

void UT66MobManagerSubsystem::Tick(float DeltaTime)
{
	if (bBackroomsGameplayPaused)
	{
		for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ActiveMobs)
		{
			if (AT66MobBase* Mob = WeakMob.Get())
			{
				Mob->StoredVelocity = FVector::ZeroVector;
				Mob->KnockbackVelocity = FVector::ZeroVector;
				Mob->bIsTouchingHero = false;
			}
		}
		return;
	}

	const bool bProfileEnabled = IsManagerTickProfileEnabled();
	AT66HeroBase* Hero = ResolveLocalHero(GetWorld());
	const FVector HeroLocation = Hero ? Hero->GetActorLocation() : FVector::ZeroVector;

	for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ActiveMobs)
	{
		AT66MobBase* Mob = WeakMob.Get();
		if (!Mob)
		{
			continue;
		}

		const ET66EnemyFamily ProfileFamily = Mob->GetEnemyFamily();
		const uint64 ProfileStartCycles = bProfileEnabled ? FPlatformTime::Cycles64() : 0;

		if (Mob->LifecycleState == ET66MobLifecycleState::Dying)
		{
			Mob->StoredVelocity = FVector::ZeroVector;
			TickMobVertexAnimationState(Mob, DeltaTime);
			AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
			continue;
		}

		TickStatusTimer(Mob->StunSecondsRemaining, Mob->StunDurationSeconds, DeltaTime);
		TickStatusTimer(Mob->RootSecondsRemaining, Mob->RootDurationSeconds, DeltaTime);
		TickStatusTimer(Mob->FreezeSecondsRemaining, Mob->FreezeDurationSeconds, DeltaTime);
		TickStatusTimer(Mob->KnockbackSecondsRemaining, Mob->KnockbackDurationSeconds, DeltaTime);
		TickStatusTimer(Mob->SlowSecondsRemaining, Mob->SlowDurationSeconds, DeltaTime);
		if (Mob->SlowSecondsRemaining <= 0.f)
		{
			Mob->SlowStrength = 0.f;
			Mob->SlowMultiplier = 1.f;
		}

		ApplyMobTouchDamageIfNeeded(Mob, Hero, DeltaTime);

		if (Mob->KnockbackSecondsRemaining > 0.f)
		{
			ApplyMobKnockback(Mob, DeltaTime);
			TickMobVertexAnimationState(Mob, DeltaTime);
			AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
			continue;
		}
		Mob->KnockbackVelocity = FVector::ZeroVector;

		if (!Hero || IsChaseBlockedByStatus(Mob))
		{
			if (Hero && ProfileFamily == ET66EnemyFamily::Ranged)
			{
				const float DistanceToHero = FVector::Dist2D(Mob->GetActorLocation(), HeroLocation);
				RecordRangedFireAttempt(true, Mob->MobID, DistanceToHero);
				RecordRangedStatusBlocked(true, Mob->MobID, DistanceToHero);
			}
			Mob->StoredVelocity = FVector::ZeroVector;
			TickMobVertexAnimationState(Mob, DeltaTime);
			AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
			continue;
		}

		if (TickSafeZoneRepelMovementIfNeeded(this, Mob, Hero, HeroLocation, DeltaTime))
		{
			TickMobVertexAnimationState(Mob, DeltaTime);
			AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
			continue;
		}

		if (TickRushMovementIfNeeded(Mob, HeroLocation, DeltaTime))
		{
			TickMobVertexAnimationState(Mob, DeltaTime);
			AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
			continue;
		}

		if (TickFlyingMovementIfNeeded(Mob, HeroLocation, DeltaTime))
		{
			TickMobVertexAnimationState(Mob, DeltaTime);
			AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
			continue;
		}

		if (TickRangedMovementIfNeeded(this, Mob, Hero, HeroLocation, DeltaTime))
		{
			TickMobVertexAnimationState(Mob, DeltaTime);
			AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
			continue;
		}

		const FVector MobLocation = Mob->GetActorLocation();
		FVector ToHero = HeroLocation - MobLocation;
		ToHero.Z = 0.f;
		const float DistanceToHero = ToHero.Size2D();
		if (DistanceToHero < T66MobArrivalThreshold)
		{
			Mob->StoredVelocity = FVector::ZeroVector;
			TickMobVertexAnimationState(Mob, DeltaTime);
			AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
			continue;
		}

		const FVector Direction = ToHero.GetSafeNormal2D();
		const float SlowFactor = FMath::Clamp(1.f - Mob->SlowStrength, 0.f, 1.f);
		const float ChaseSpeed = GetFamilyChaseSpeed(Mob) * SlowFactor;
		ApplyMobPlanarMovement(Mob, Direction, ChaseSpeed, DeltaTime);

		TickMobVertexAnimationState(Mob, DeltaTime);
		AccumulateMobTickProfile(bProfileEnabled, ProfileStartCycles, ProfileFamily);
	}

	++TickFrameCounter;
#if !UE_BUILD_SHIPPING
	if (!bMobTickVatRuntimeProofEmitted && FParse::Param(FCommandLine::Get(), TEXT("T66MobTickVatRuntimeProof")))
	{
		const UWorld* World = GetWorld();
		const bool bReadyForProof = ActiveMobs.Num() >= 16
			&& (!World || World->GetTimeSeconds() >= 3.0f || TickFrameCounter >= 180ull);
		if (bReadyForProof)
		{
			bMobTickVatRuntimeProofEmitted = true;
			const bool bPass = RunMobTickVatRuntimeProof();
			FPlatformMisc::RequestExitWithStatus(false, bPass ? 0 : 1, TEXT("MobTickVatRuntimeProofComplete"));
			return;
		}
	}
#endif
	if ((TickFrameCounter % 60ull) == 0ull)
	{
		if (bProfileEnabled)
		{
			const double TotalMobUs = CyclesToMicroseconds(GManagerProfileWindow.TotalMobCycles);
			const double MeleeMobUs = CyclesToMicroseconds(GManagerProfileWindow.MeleeMobCycles);
			const double RushMobUs = CyclesToMicroseconds(GManagerProfileWindow.RushMobCycles);
			const double FlyingMobUs = CyclesToMicroseconds(GManagerProfileWindow.FlyingMobCycles);
			const double RangedMobUs = CyclesToMicroseconds(GManagerProfileWindow.RangedMobCycles);
			const double PoolAcquireUs = CyclesToMicroseconds(GManagerProfileWindow.PoolAcquireCycles);
			const double PoolReleaseUs = CyclesToMicroseconds(GManagerProfileWindow.PoolReleaseCycles);
			const double MeleePerMobUs = GManagerProfileWindow.MeleeMobSamples > 0
				? MeleeMobUs / static_cast<double>(GManagerProfileWindow.MeleeMobSamples)
				: 0.0;
			const double RushPerMobUs = GManagerProfileWindow.RushMobSamples > 0
				? RushMobUs / static_cast<double>(GManagerProfileWindow.RushMobSamples)
				: 0.0;
			const double FlyingPerMobUs = GManagerProfileWindow.FlyingMobSamples > 0
				? FlyingMobUs / static_cast<double>(GManagerProfileWindow.FlyingMobSamples)
				: 0.0;
			const double RangedPerMobUs = GManagerProfileWindow.RangedMobSamples > 0
				? RangedMobUs / static_cast<double>(GManagerProfileWindow.RangedMobSamples)
				: 0.0;
			const double TotalPerMobUs = GManagerProfileWindow.TotalMobSamples > 0
				? TotalMobUs / static_cast<double>(GManagerProfileWindow.TotalMobSamples)
				: 0.0;
			const double PoolAcquirePerCallUs = GManagerProfileWindow.PoolAcquireSamples > 0
				? PoolAcquireUs / static_cast<double>(GManagerProfileWindow.PoolAcquireSamples)
				: 0.0;
			const double PoolReleasePerCallUs = GManagerProfileWindow.PoolReleaseSamples > 0
				? PoolReleaseUs / static_cast<double>(GManagerProfileWindow.PoolReleaseSamples)
				: 0.0;

			UE_LOG(
				LogT66MobManager,
				VeryVerbose,
				TEXT("[MobManagerProfile] frames=60 totalMobUs=%.3f totalMobSamples=%d totalPerMobUs=%.6f meleeUs=%.3f meleeSamples=%d meleePerMobUs=%.6f rushUs=%.3f rushSamples=%d rushPerMobUs=%.6f flyingUs=%.3f flyingSamples=%d flyingPerMobUs=%.6f rangedUs=%.3f rangedSamples=%d rangedPerMobUs=%.6f poolAcquireUs=%.3f poolAcquireSamples=%d poolAcquirePerCallUs=%.6f poolReleaseUs=%.3f poolReleaseSamples=%d poolReleasePerCallUs=%.6f activeMobs=%d"),
				TotalMobUs,
				GManagerProfileWindow.TotalMobSamples,
				TotalPerMobUs,
				MeleeMobUs,
				GManagerProfileWindow.MeleeMobSamples,
				MeleePerMobUs,
				RushMobUs,
				GManagerProfileWindow.RushMobSamples,
				RushPerMobUs,
				FlyingMobUs,
				GManagerProfileWindow.FlyingMobSamples,
				FlyingPerMobUs,
				RangedMobUs,
				GManagerProfileWindow.RangedMobSamples,
				RangedPerMobUs,
				PoolAcquireUs,
				GManagerProfileWindow.PoolAcquireSamples,
				PoolAcquirePerCallUs,
				PoolReleaseUs,
				GManagerProfileWindow.PoolReleaseSamples,
				PoolReleasePerCallUs,
				ActiveMobs.Num());
			GManagerProfileWindow.Reset();
		}

		const AT66MobBase* FirstMob = nullptr;
		for (const TWeakObjectPtr<AT66MobBase>& WeakMob : ActiveMobs)
		{
			if (const AT66MobBase* Mob = WeakMob.Get())
			{
				FirstMob = Mob;
				break;
			}
		}

		if (FirstMob)
		{
			UE_LOG(LogT66MobManager, VeryVerbose, TEXT("Tick delta=%.4f ActiveMobs.Num()=%d sampleMob=%s loc=%s velocity=%s status=%s"),
				DeltaTime,
				ActiveMobs.Num(),
				*GetMobIDForLog(FirstMob),
				*FirstMob->GetActorLocation().ToCompactString(),
				*FirstMob->StoredVelocity.ToCompactString(),
				*BuildStatusFlagsForLog(FirstMob));
		}
		else
		{
			UE_LOG(LogT66MobManager, VeryVerbose, TEXT("Tick delta=%.4f ActiveMobs.Num()=%d"), DeltaTime, ActiveMobs.Num());
		}
	}
}

TStatId UT66MobManagerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UT66MobManagerSubsystem, STATGROUP_Tickables);
}

bool UT66MobManagerSubsystem::IsTickable() const
{
	return !ActiveMobs.IsEmpty();
}

bool UT66MobManagerSubsystem::IsTickableWhenPaused() const
{
	return false;
}

bool UT66MobManagerSubsystem::IsTickableInEditor() const
{
	return false;
}
