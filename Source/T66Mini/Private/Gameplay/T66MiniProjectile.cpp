// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniProjectile.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Gameplay/T66MiniEnemyBase.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Net/UnrealNetwork.h"

namespace
{
	FLinearColor T66MiniGetFollowUpTint(const ET66MiniProjectileBehavior Behavior, const FName IdolID)
	{
		if (IdolID == FName(TEXT("Idol_Electric")))
		{
			return FLinearColor(0.64f, 0.90f, 1.0f, 0.34f);
		}

		switch (Behavior)
		{
		case ET66MiniProjectileBehavior::Pierce:
			return FLinearColor(1.0f, 0.84f, 0.34f, 0.26f);

		case ET66MiniProjectileBehavior::Bounce:
			return FLinearColor(0.58f, 0.86f, 1.0f, 0.26f);

		case ET66MiniProjectileBehavior::AOE:
			return FLinearColor(1.0f, 0.58f, 0.24f, 0.28f);

		case ET66MiniProjectileBehavior::DOT:
			return FLinearColor(0.62f, 1.0f, 0.54f, 0.24f);

		default:
			return FLinearColor(1.f, 1.f, 1.f, 0.22f);
		}
	}

	FVector T66MiniGetFollowUpScale(const ET66MiniProjectileBehavior Behavior, const float Radius)
	{
		switch (Behavior)
		{
		case ET66MiniProjectileBehavior::AOE:
		{
			const float UniformScale = FMath::Clamp(Radius / 180.f, 0.46f, 1.18f);
			return FVector(UniformScale, UniformScale, 1.f);
		}

		case ET66MiniProjectileBehavior::DOT:
			return FVector(0.42f, 0.42f, 1.f);

		default:
			return FVector(0.36f, 0.36f, 1.f);
		}
	}

	void T66MiniCreditSuccessfulHit(AActor* OwnerActor, const float Damage)
	{
		if (Damage <= 0.f)
		{
			return;
		}

		if (AT66MiniPlayerPawn* PlayerPawn = Cast<AT66MiniPlayerPawn>(OwnerActor))
		{
			PlayerPawn->HandleSuccessfulHit(Damage);
		}
	}
}

AT66MiniProjectile::AT66MiniProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetupAttachment(SceneRoot);
	CollisionComponent->InitSphereRadius(26.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(SceneRoot);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
	SpriteComponent->SetRelativeScale3D(FVector(1.1f, 1.1f, 1.1f));
	SpriteComponent->SetHiddenInGame(false, true);
	SpriteComponent->SetVisibility(true, true);
}

void AT66MiniProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AT66MiniProjectile::HandleOverlap);
	}
	OnRep_ProjectileVisualState();
}

void AT66MiniProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66MiniProjectile, Behavior);
	DOREPLIFETIME(AT66MiniProjectile, IdolID);
	DOREPLIFETIME(AT66MiniProjectile, PrimaryTexture);
	DOREPLIFETIME(AT66MiniProjectile, FollowUpTexture);
}

void AT66MiniProjectile::InitializeProjectile(
	AActor* InOwnerActor,
	const FVector& InSpawnLocation,
	const FVector& InDirection,
	const ET66MiniProjectileBehavior InBehavior,
	const float InPrimaryDamage,
	const float InFollowUpDamage,
	const float InSpeed,
	const float InRadius,
	const int32 InRemainingHits,
	const int32 InRemainingBounces,
	const float InDotTickDamage,
	const float InDotTickInterval,
	const float InDotDuration,
	UTexture2D* InPrimaryTexture,
	UTexture2D* InFollowUpTexture,
	AActor* InInitialTarget,
	const FName InIdolID,
	const float InStunDuration)
{
	OwnerActor = InOwnerActor;
	SetActorLocation(InSpawnLocation);
	MoveDirection = InDirection.GetSafeNormal();
	Behavior = InBehavior;
	IdolID = InIdolID;
	PrimaryDamage = InPrimaryDamage;
	FollowUpDamage = InFollowUpDamage;
	Speed = InSpeed;
	Radius = InRadius;
	RemainingHits = FMath::Max(1, InRemainingHits);
	RemainingBounces = FMath::Max(0, InRemainingBounces);
	DotTickDamage = InDotTickDamage;
	DotTickInterval = InDotTickInterval;
	DotDuration = InDotDuration;
	StunDuration = InStunDuration;
	HomingTarget = InInitialTarget;
	PrimaryTexture = InPrimaryTexture;
	FollowUpTexture = InFollowUpTexture;
	bPrimaryHitResolved = false;

	CollisionComponent->SetSphereRadius(FMath::Clamp(InRadius * 0.18f, 18.f, 42.f));
	if (PrimaryTexture)
	{
		SpriteComponent->SetSprite(PrimaryTexture);
	}
	else if (FollowUpTexture)
	{
		SpriteComponent->SetSprite(FollowUpTexture);
	}
}

void AT66MiniProjectile::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	LifetimeRemaining -= DeltaSeconds;
	if (LifetimeRemaining <= 0.f)
	{
		if (Behavior == ET66MiniProjectileBehavior::AOE)
		{
			ExplodeAt(GetActorLocation());
		}

		Destroy();
		return;
	}

	if ((Behavior == ET66MiniProjectileBehavior::Bounce || Behavior == ET66MiniProjectileBehavior::DOT) && HomingTarget.IsValid())
	{
		const FVector ToTarget = HomingTarget->GetActorLocation() - GetActorLocation();
		if (!ToTarget.IsNearlyZero())
		{
			MoveDirection = ToTarget.GetSafeNormal();
		}
	}

	SetActorLocation(GetActorLocation() + (MoveDirection * Speed * DeltaSeconds));
}

void AT66MiniProjectile::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	const int32 OtherBodyIndex,
	const bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == OwnerActor.Get())
	{
		return;
	}

	AT66MiniEnemyBase* Enemy = Cast<AT66MiniEnemyBase>(OtherActor);
	if (!Enemy || Enemy->IsEnemyDead())
	{
		return;
	}

	if (HitActors.Contains(Enemy))
	{
		return;
	}

	HitActors.Add(Enemy);

	const bool bApplyPrimaryDamage = (Behavior == ET66MiniProjectileBehavior::Pierce) || !bPrimaryHitResolved;
	if (bApplyPrimaryDamage && PrimaryDamage > 0.f)
	{
		Enemy->ApplyDamage(PrimaryDamage);
		T66MiniCreditSuccessfulHit(OwnerActor.Get(), PrimaryDamage);
	}

	bPrimaryHitResolved = true;

	switch (Behavior)
	{
	case ET66MiniProjectileBehavior::BasicAttack:
		if (AT66MiniPlayerPawn* PlayerPawn = Cast<AT66MiniPlayerPawn>(OwnerActor.Get()))
		{
			PlayerPawn->HandleBasicAttackImpact(Enemy, Enemy->GetActorLocation() + FVector(0.f, 0.f, 8.f));
		}
		Destroy();
		break;

	case ET66MiniProjectileBehavior::Pierce:
		SpawnFollowUpPulse(Enemy->GetActorLocation() + FVector(0.f, 0.f, 6.f));
		if (!Enemy->IsEnemyDead() && FollowUpDamage > 0.f)
		{
			Enemy->ApplyDamage(FollowUpDamage);
			T66MiniCreditSuccessfulHit(OwnerActor.Get(), FollowUpDamage);
		}

		--RemainingHits;
		if (RemainingHits <= 0)
		{
			Destroy();
		}
		break;

	case ET66MiniProjectileBehavior::Bounce:
		SpawnFollowUpPulse(Enemy->GetActorLocation() + FVector(0.f, 0.f, 10.f));
		if (!Enemy->IsEnemyDead() && FollowUpDamage > 0.f)
		{
			Enemy->ApplyDamage(FollowUpDamage);
			T66MiniCreditSuccessfulHit(OwnerActor.Get(), FollowUpDamage);
		}

		if (!Enemy->IsEnemyDead() && StunDuration > 0.f)
		{
			Enemy->ApplyStun(StunDuration);
		}

		if (RemainingBounces > 0)
		{
			--RemainingBounces;
			if (AT66MiniEnemyBase* NextEnemy = FindNextBounceTarget(Enemy, Radius))
			{
				HomingTarget = NextEnemy;
				SetActorLocation(Enemy->GetActorLocation() + FVector(0.f, 0.f, 30.f));
				MoveDirection = (NextEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				if (FollowUpTexture)
				{
					SpriteComponent->SetSprite(FollowUpTexture);
				}
				return;
			}
		}

		Destroy();
		break;

	case ET66MiniProjectileBehavior::AOE:
		ExplodeAt(Enemy->GetActorLocation());
		Destroy();
		break;

	case ET66MiniProjectileBehavior::DOT:
		SpawnFollowUpPulse(Enemy->GetActorLocation() + FVector(0.f, 0.f, 8.f));
		if (!Enemy->IsEnemyDead() && FollowUpDamage > 0.f)
		{
			Enemy->ApplyDamage(FollowUpDamage);
			T66MiniCreditSuccessfulHit(OwnerActor.Get(), FollowUpDamage);
		}

		if (!Enemy->IsEnemyDead())
		{
			Enemy->ApplyDot(FMath::Max(DotTickDamage, FollowUpDamage * 0.35f), DotTickInterval, DotDuration);
		}

		Destroy();
		break;

	default:
		Destroy();
		break;
	}
}

AT66MiniEnemyBase* AT66MiniProjectile::FindNextBounceTarget(AActor* IgnoreActor, const float MaxRange) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AT66MiniEnemyBase* BestEnemy = nullptr;
	float BestDistanceSq = FMath::Square(MaxRange);
	for (TActorIterator<AT66MiniEnemyBase> It(World); It; ++It)
	{
		AT66MiniEnemyBase* Candidate = *It;
		if (!Candidate || Candidate == IgnoreActor || Candidate->IsEnemyDead() || HitActors.Contains(Candidate))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestEnemy = Candidate;
		}
	}

	return BestEnemy;
}

void AT66MiniProjectile::ExplodeAt(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SpawnFollowUpPulse(Location + FVector(0.f, 0.f, 6.f));

	for (TActorIterator<AT66MiniEnemyBase> It(World); It; ++It)
	{
		AT66MiniEnemyBase* Candidate = *It;
		if (!Candidate || Candidate->IsEnemyDead())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(Location, Candidate->GetActorLocation());
		if (DistanceSq <= FMath::Square(Radius))
		{
			Candidate->ApplyDamage(FollowUpDamage);
			T66MiniCreditSuccessfulHit(OwnerActor.Get(), FollowUpDamage);
		}
	}
}

void AT66MiniProjectile::SpawnFollowUpPulse(const FVector& Location) const
{
	UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr;
	if (!VfxSubsystem)
	{
		return;
	}

	const FVector Scale = T66MiniGetFollowUpScale(Behavior, Radius);
	const FLinearColor Tint = T66MiniGetFollowUpTint(Behavior, IdolID);
	if (FollowUpTexture)
	{
		VfxSubsystem->SpawnSpritePulse(GetWorld(), Location, Scale, Behavior == ET66MiniProjectileBehavior::AOE ? 0.20f : 0.14f, Tint, FollowUpTexture, Behavior == ET66MiniProjectileBehavior::AOE ? 0.94f : 0.70f);
		return;
	}

	VfxSubsystem->SpawnPulse(GetWorld(), Location, Scale, Behavior == ET66MiniProjectileBehavior::AOE ? 0.20f : 0.14f, Tint, Behavior == ET66MiniProjectileBehavior::AOE ? 0.94f : 0.70f);
}

void AT66MiniProjectile::OnRep_ProjectileVisualState()
{
	if (PrimaryTexture)
	{
		SpriteComponent->SetSprite(PrimaryTexture);
	}
	else if (FollowUpTexture)
	{
		SpriteComponent->SetSprite(FollowUpTexture);
	}
}
