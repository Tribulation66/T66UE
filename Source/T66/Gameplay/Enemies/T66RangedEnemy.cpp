// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Enemies/T66RangedEnemy.h"

#include "Core/T66ActorRegistrySubsystem.h"
#include "Gameplay/Enemies/Projectiles/T66EnemyProjectileBase.h"
#include "Gameplay/Enemies/Projectiles/T66SpitProjectile.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobManagerSubsystem.h"
#include "Gameplay/T66ProjectileManagerSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CoreGlobals.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66RangedEnemy, Log, All);

namespace
{
struct FT66RichEnemyIgnoreCache
{
	TWeakObjectPtr<UWorld> World;
	uint64 FrameNumber = MAX_uint64;
	TArray<TWeakObjectPtr<AActor>> Actors;
};

FT66RichEnemyIgnoreCache GT66RichEnemyIgnoreCache;

void AddCachedRichEnemyIgnoreActors(UWorld* World, FCollisionQueryParams& Params)
{
	if (!World)
	{
		return;
	}

	if (GT66RichEnemyIgnoreCache.World.Get() != World || GT66RichEnemyIgnoreCache.FrameNumber != GFrameCounter)
	{
		GT66RichEnemyIgnoreCache.World = World;
		GT66RichEnemyIgnoreCache.FrameNumber = GFrameCounter;
		GT66RichEnemyIgnoreCache.Actors.Reset();

		if (UT66ActorRegistrySubsystem* Registry = World->GetSubsystem<UT66ActorRegistrySubsystem>())
		{
			for (const TWeakObjectPtr<AT66EnemyBase>& WeakEnemy : Registry->GetEnemies())
			{
				if (AActor* Enemy = WeakEnemy.Get())
				{
					GT66RichEnemyIgnoreCache.Actors.Add(Enemy);
				}
			}
		}
	}

	for (const TWeakObjectPtr<AActor>& WeakActor : GT66RichEnemyIgnoreCache.Actors)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Params.AddIgnoredActor(Actor);
		}
	}
}
}

AT66RangedEnemy::AT66RangedEnemy()
{
	EnemyFamily = ET66EnemyFamily::Ranged;
	ProjectileClass = AT66SpitProjectile::StaticClass();
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 160.f;
	}
}

void AT66RangedEnemy::ResetFamilyState()
{
	FireCooldownRemaining = 0.6f;
}

void AT66RangedEnemy::TickFamilyBehavior(APawn* PlayerPawn, const float DeltaSeconds, const float Dist2DToPlayer, const bool bShouldRunAwayFromPlayer)
{
	if (!PlayerPawn)
	{
		return;
	}

	FireCooldownRemaining = FMath::Max(0.f, FireCooldownRemaining - DeltaSeconds);

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const bool bHasDirection = ToPlayer.Normalize();

	if (bShouldRunAwayFromPlayer)
	{
		if (bHasDirection)
		{
			AddMovementInput(-ToPlayer, 1.f);
		}
		return;
	}

	if (Dist2DToPlayer < DesiredMinRange)
	{
		if (bHasDirection)
		{
			AddMovementInput(-ToPlayer, 1.f);
		}
	}
	else if (Dist2DToPlayer > DesiredMaxRange)
	{
		if (bHasDirection)
		{
			AddMovementInput(ToPlayer, 1.f);
		}
	}
	else if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
	}

	if (FireCooldownRemaining <= 0.f)
	{
		UT66MobManagerSubsystem* MobManager = GetWorld() ? GetWorld()->GetSubsystem<UT66MobManagerSubsystem>() : nullptr;
		if (const AT66HeroBase* Hero = Cast<AT66HeroBase>(PlayerPawn))
		{
			if (Hero->IsInSafeZone())
			{
				if (MobManager)
				{
					MobManager->RecordRangedFireSkippedSafeZone(false, MobID, Dist2DToPlayer);
				}
				return;
			}
		}

		const float MaxFireRange = FMath::Max(DesiredMaxRange, 1.f) + FMath::Max(0.f, FireRangeGrace);
		if (Dist2DToPlayer > MaxFireRange)
		{
			if (MobManager)
			{
				MobManager->RecordRangedFireSkippedOutOfRange(false, MobID, Dist2DToPlayer, MaxFireRange);
			}
			FireCooldownRemaining = 0.25f;
		}
		else
		{
			if (MobManager)
			{
				MobManager->RecordRangedDistancePassed(false, MobID, Dist2DToPlayer, DesiredMinRange, DesiredMaxRange, MaxFireRange);
			}
			if (FireProjectileAtPlayer(PlayerPawn))
			{
				FireCooldownRemaining = FireIntervalSeconds;
			}
			else
			{
				FireCooldownRemaining = 0.25f;
			}
		}
	}
	else if (UT66MobManagerSubsystem* MobManager = GetWorld() ? GetWorld()->GetSubsystem<UT66MobManagerSubsystem>() : nullptr)
	{
		MobManager->RecordRangedCooldownBlocked(false, MobID, Dist2DToPlayer, FireCooldownRemaining);
	}
}

bool AT66RangedEnemy::FireProjectileAtPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	UT66MobManagerSubsystem* MobManager = World->GetSubsystem<UT66MobManagerSubsystem>();

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, ProjectileSpawnHeight);
	const FVector Target = PlayerPawn->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const float Dist2D = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());
	FString LOSBlockerName;
	const AActor* LOSBlockerActor = nullptr;
	const UPrimitiveComponent* LOSBlockerComponent = nullptr;
	if (!HasProjectileLineOfSightToPlayer(PlayerPawn, Start, Target, LOSBlockerName, LOSBlockerActor, LOSBlockerComponent))
	{
		if (MobManager)
		{
			MobManager->RecordRangedLosBlocked(false, MobID, Dist2D, LOSBlockerActor, LOSBlockerComponent);
		}
		UE_LOG(
			LogT66RangedEnemy,
			VeryVerbose,
			TEXT("[EnemyRange] BlockedShot Enemy=%s MobID=%s Player=%s Dist2D=%.1f Start=%s Target=%s Blocker=%s"),
			*GetName(),
			*MobID.ToString(),
			*PlayerPawn->GetName(),
			Dist2D,
			*Start.ToCompactString(),
			*Target.ToCompactString(),
			*LOSBlockerName);
		return false;
	}
	if (MobManager)
	{
		MobManager->RecordRangedLosPassed(false, MobID, Dist2D);
	}

	FVector ShotDirection = Target - Start;
	if (!ShotDirection.Normalize())
	{
		if (MobManager)
		{
			MobManager->RecordRangedZeroDirectionShot(false, MobID);
		}
		return false;
	}
	if (MobManager)
	{
		MobManager->RecordRangedDispatchReached(false, MobID, Dist2D);
	}
	UT66ProjectileManagerSubsystem* ProjectileManager = World->GetSubsystem<UT66ProjectileManagerSubsystem>();
	if (ProjectileManager)
	{
		FT66ManagedProjectileFireParams FireParams;
		FireParams.SourceActor = this;
		FireParams.SourceID = MobID;
		FireParams.Origin = Start;
		FireParams.Direction = ShotDirection;
		FireParams.Speed = 2400.f;
		FireParams.Damage = 20.f;
		FireParams.Radius = 18.f;
		FireParams.Lifetime = 4.f;
		FireParams.ProjectileTypeIndex = UT66ProjectileManagerSubsystem::EnemySpitProjectileTypeIndex;
		FireParams.Delivery = ET66ManagedProjectileDelivery::EnemyProjectile;
		FireParams.AttackCategory = ProjectileCategory;
		FireParams.VisualProfileID = ProjectileVisualProfileID.IsNone()
			? UT66ProjectileManagerSubsystem::DefaultEnemySpitVisualProfileID()
			: ProjectileVisualProfileID;
		FireParams.ProjectileMesh = ProjectileMesh;
		FireParams.ProjectileMeshScale = ProjectileMeshScale;
		if (ProjectileManager->FireManagedProjectile(FireParams))
		{
			if (MobManager)
			{
				MobManager->RecordRangedProjectileSpawned(false, MobID);
			}
			UE_LOG(
				LogT66RangedEnemy,
				VeryVerbose,
				TEXT("[EnemyRange] FiredShot Enemy=%s MobID=%s Projectile=ManagedEnemySpit Category=%d Profile=%s Mesh=%s Player=%s Dist2D=%.1f Start=%s Target=%s"),
				*GetName(),
				*MobID.ToString(),
				static_cast<int32>(ProjectileCategory),
				*FireParams.VisualProfileID.ToString(),
				*ProjectileMesh.ToSoftObjectPath().ToString(),
				*PlayerPawn->GetName(),
				Dist2D,
				*Start.ToCompactString(),
				*Target.ToCompactString());
			return true;
		}
	}

	if (MobManager)
	{
		MobManager->RecordRangedProjectileSpawnFailed(false, MobID);
	}
	return false;
}

bool AT66RangedEnemy::HasProjectileLineOfSightToPlayer(const APawn* PlayerPawn, const FVector& Start, const FVector& End, FString& OutBlockerName, const AActor*& OutBlockerActor, const UPrimitiveComponent*& OutBlockerComponent) const
{
	OutBlockerName = TEXT("None");
	OutBlockerActor = nullptr;
	OutBlockerComponent = nullptr;
	UWorld* World = GetWorld();
	if (!World || !PlayerPawn)
	{
		OutBlockerName = TEXT("NoWorldOrPlayer");
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66RangedEnemyProjectileLOS), false, this);
	AddCachedRichEnemyIgnoreActors(World, Params);
	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return true;
	}

	const AActor* HitActor = Hit.GetActor();
	if (HitActor == PlayerPawn)
	{
		return true;
	}

	OutBlockerActor = HitActor;
	OutBlockerComponent = Hit.GetComponent();
	OutBlockerName = HitActor
		? FString::Printf(TEXT("%s/%s"), *HitActor->GetName(), HitActor->GetClass() ? *HitActor->GetClass()->GetName() : TEXT("None"))
		: FString(TEXT("WorldStatic"));
	return false;
}
