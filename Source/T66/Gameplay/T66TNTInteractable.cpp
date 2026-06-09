#include "Gameplay/T66TNTInteractable.h"

#include "Components/StaticMeshComponent.h"
#include "Core/T66InteractionPromptSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "EngineUtils.h"
#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66MobBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66TNTInteractable, Log, All);

namespace
{
	const FName TNTExplosionDelivery(TEXT("TNTExplosion"));
	const FName TNTDamageSource(TEXT("TNTExplosion"));
}

AT66TNTInteractable::AT66TNTInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	FuseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FuseMesh"));
	FuseMesh->SetupAttachment(RootComponent);
	FuseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FuseMesh->SetCanEverAffectNavigation(false);

	Rarity = ET66Rarity::Red;

	ApplyRarityVisuals();
}

bool AT66TNTInteractable::Interact(APlayerController* PlayerController)
{
	if (!PlayerController || bConsumed || bFuseLit || !GetWorld())
	{
		return false;
	}

	bFuseLit = true;
	bConsumed = true;
	UpdateFuseVisuals();
	RefreshInteractionPrompt();

	const float ClampedFuseSeconds = FMath::Max(0.10f, FuseSeconds);
	GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AT66TNTInteractable::Explode, ClampedFuseSeconds, false);

	UE_LOG(
		LogT66TNTInteractable,
		Display,
		TEXT("[TNT] FuseLit Actor=%s FuseSeconds=%.2f Radius=%.1f DamageHP=%d Location=%s"),
		*GetNameSafe(this),
		ClampedFuseSeconds,
		ExplosionRadius,
		DamageHP,
		*GetActorLocation().ToString());

	return true;
}

void AT66TNTInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FuseTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66TNTInteractable::ApplyRarityVisuals()
{
	if (VisualMesh)
	{
		VisualMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
		VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 95.0f));
		VisualMesh->SetRelativeScale3D(FVector(1.20f, 1.20f, 1.10f));
		const FLinearColor BodyColor = bFuseLit
			? FLinearColor(1.0f, 0.26f, 0.05f, 1.0f)
			: FLinearColor(0.84f, 0.02f, 0.02f, 1.0f);
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, BodyColor);
	}

	if (FuseMesh)
	{
		FuseMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
		FuseMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 170.0f));
		FuseMesh->SetRelativeScale3D(bFuseLit ? FVector(0.30f, 0.30f, 0.30f) : FVector(0.22f, 0.22f, 0.22f));
		const FLinearColor FuseColor = bFuseLit
			? FLinearColor(1.0f, 0.82f, 0.10f, 1.0f)
			: FLinearColor(0.04f, 0.03f, 0.03f, 1.0f);
		FT66VisualUtil::ApplyT66Color(FuseMesh, this, FuseColor);
	}
}

FText AT66TNTInteractable::BuildInteractionPromptText() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UT66InteractionPromptSubsystem* PromptSubsystem = GameInstance->GetSubsystem<UT66InteractionPromptSubsystem>())
		{
			return PromptSubsystem->BuildCustomPromptText(NSLOCTEXT("T66.TNTInteractable", "LightFuseVerb", "light fuse"));
		}
	}

	return NSLOCTEXT("T66.TNTInteractable", "LightFuseFallback", "Press F to light fuse");
}

FText AT66TNTInteractable::BuildInteractionPromptTargetName() const
{
	return NSLOCTEXT("T66.TNTInteractable", "TargetName", "TNT");
}

float AT66TNTInteractable::GetInteractionPromptVerticalPadding() const
{
	return 160.0f;
}

FVector AT66TNTInteractable::GetMinimumInteractionExtent() const
{
	return FVector(260.0f, 260.0f, 220.0f);
}

FVector AT66TNTInteractable::GetInteractionBoundsPadding() const
{
	return FVector(100.0f, 100.0f, 70.0f);
}

void AT66TNTInteractable::Explode()
{
	if (!GetWorld())
	{
		return;
	}

	int32 HeroesDamaged = 0;
	int32 EnemiesDamaged = 0;
	int32 MobsDamaged = 0;
	int32 BossesDamaged = 0;
	ApplyExplosionDamage(HeroesDamaged, EnemiesDamaged, MobsDamaged, BossesDamaged);

	const FVector Origin = GetActorLocation();
	T66CombatDebugDraw::DrawDamageSphere(GetWorld(), Origin, ExplosionRadius, TEXT("TNT Explosion"), true);
	UT66CombatComponent::SpawnDeathBurstAtLocation(GetWorld(), Origin, 36, FMath::Clamp(ExplosionRadius * 0.35f, 140.0f, 280.0f));

	UE_LOG(
		LogT66TNTInteractable,
		Display,
		TEXT("[TNTExplosion] Actor=%s DamageHP=%d Radius=%.1f Heroes=%d Enemies=%d Mobs=%d Bosses=%d Location=%s"),
		*GetNameSafe(this),
		DamageHP,
		ExplosionRadius,
		HeroesDamaged,
		EnemiesDamaged,
		MobsDamaged,
		BossesDamaged,
		*Origin.ToString());

	if (bShowcaseReusable)
	{
		bFuseLit = false;
		bConsumed = false;
		UpdateFuseVisuals();
		RefreshInteractionPrompt();
		return;
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	Destroy();
}

void AT66TNTInteractable::ApplyExplosionDamage(int32& OutHeroesDamaged, int32& OutEnemiesDamaged, int32& OutMobsDamaged, int32& OutBossesDamaged)
{
	OutHeroesDamaged = 0;
	OutEnemiesDamaged = 0;
	OutMobsDamaged = 0;
	OutBossesDamaged = 0;

	UWorld* World = GetWorld();
	if (!World || DamageHP <= 0 || ExplosionRadius <= 0.0f)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const float RadiusSq = FMath::Square(ExplosionRadius);

	bool bAppliedHeroDamage = false;
	for (TActorIterator<AT66HeroBase> It(World); It; ++It)
	{
		AT66HeroBase* Hero = *It;
		if (!Hero || bAppliedHeroDamage || !IsActorInExplosionRadius(Hero, Origin, RadiusSq))
		{
			continue;
		}

		if (UT66RunStateSubsystem* RunState = World->GetGameInstance()
			? World->GetGameInstance()->GetSubsystem<UT66RunStateSubsystem>()
			: nullptr)
		{
			if (RunState->ApplyDamage(DamageHP, this, TNTExplosionDelivery, this))
			{
				++OutHeroesDamaged;
			}
			bAppliedHeroDamage = true;
		}
	}

	for (TActorIterator<AT66EnemyBase> It(World); It; ++It)
	{
		AT66EnemyBase* Enemy = *It;
		if (!Enemy || Enemy->CurrentHP <= 0 || !IsActorInExplosionRadius(Enemy, Origin, RadiusSq))
		{
			continue;
		}

		const int32 PreviousHP = Enemy->CurrentHP;
		Enemy->TakeDamageFromEnvironment(DamageHP, this, TNTExplosionDelivery);
		if (Enemy->CurrentHP < PreviousHP)
		{
			++OutEnemiesDamaged;
		}
	}

	for (TActorIterator<AT66MobBase> It(World); It; ++It)
	{
		AT66MobBase* Mob = *It;
		if (!Mob || !Mob->IsAliveAndActive() || !IsActorInExplosionRadius(Mob, Origin, RadiusSq))
		{
			continue;
		}

		const float PreviousHP = Mob->GetCurrentHP();
		const FT66CombatTargetHandle TargetHandle = Mob->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Body);
		Mob->TakeDamageFromHeroHitZone(DamageHP, TargetHandle, TNTDamageSource, TNTExplosionDelivery);
		if (Mob->GetCurrentHP() < PreviousHP)
		{
			++OutMobsDamaged;
		}
	}

	if (bDamageBosses)
	{
		for (TActorIterator<AT66BossBase> It(World); It; ++It)
		{
			AT66BossBase* Boss = *It;
			if (!Boss || !Boss->IsAwakened() || !Boss->IsAlive() || !IsActorInExplosionRadius(Boss, Origin, RadiusSq))
			{
				continue;
			}

			const int32 PreviousHP = Boss->CurrentHP;
			const FT66CombatTargetHandle TargetHandle = Boss->ResolveCombatTargetHandle(nullptr, ET66HitZoneType::Core);
			Boss->TakeDamageFromHeroHitZone(DamageHP, TargetHandle, TNTDamageSource, TNTExplosionDelivery);
			if (Boss->CurrentHP < PreviousHP)
			{
				++OutBossesDamaged;
			}
		}
	}
}

bool AT66TNTInteractable::IsActorInExplosionRadius(const AActor* Actor, const FVector& Origin, float RadiusSq) const
{
	if (!Actor)
	{
		return false;
	}

	return FVector::DistSquared(Actor->GetActorLocation(), Origin) <= RadiusSq;
}

void AT66TNTInteractable::UpdateFuseVisuals()
{
	ApplyRarityVisuals();
}
