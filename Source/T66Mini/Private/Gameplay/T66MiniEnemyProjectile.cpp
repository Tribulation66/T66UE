// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66MiniEnemyProjectile.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Core/T66MiniVisualSubsystem.h"
#include "Core/T66MiniVFXSubsystem.h"
#include "Engine/Texture2D.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Gameplay/T66MiniPlayerPawn.h"
#include "Net/UnrealNetwork.h"

AT66MiniEnemyProjectile::AT66MiniEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(24.f);
	SetMinNetUpdateFrequency(12.f);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(22.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);

	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(CollisionComponent);
	SpriteComponent->SetRelativeLocation(FVector(0.f, 0.f, 28.f));
	SpriteComponent->SetRelativeScale3D(FVector(0.9f, 0.9f, 0.9f));
	SpriteComponent->SetHiddenInGame(false, true);
	SpriteComponent->SetVisibility(true, true);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = false;
	ProjectileMovementComponent->bShouldBounce = false;
}

void AT66MiniEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AT66MiniEnemyProjectile::HandleOverlap);
	}
	OnRep_ProjectileTexture();
}

void AT66MiniEnemyProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66MiniEnemyProjectile, ProjectileTexture);
}

void AT66MiniEnemyProjectile::InitializeProjectile(const FVector& SpawnLocation, const FVector& Direction, float InSpeed, float InDamage, UTexture2D* InTexture, float InLifetime)
{
	SetActorLocation(SpawnLocation);
	Damage = FMath::Max(1.f, InDamage);
	const float Speed = FMath::Max(200.f, InSpeed);
	SetLifeSpan(FMath::Max(0.2f, InLifetime));

	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->Velocity = Direction.GetSafeNormal() * Speed;
		ProjectileMovementComponent->InitialSpeed = Speed;
		ProjectileMovementComponent->MaxSpeed = Speed;
	}

	ProjectileTexture = InTexture;
	if (!ProjectileTexture)
	{
		if (UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr)
		{
			ProjectileTexture = VisualSubsystem->GetWhiteTexture();
		}
	}
	if (ProjectileTexture)
	{
		SpriteComponent->SetSprite(ProjectileTexture);
	}
}

void AT66MiniEnemyProjectile::HandleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	const int32 OtherBodyIndex,
	const bool bFromSweep,
	const FHitResult& SweepResult)
{
	AT66MiniPlayerPawn* PlayerPawn = Cast<AT66MiniPlayerPawn>(OtherActor);
	if (!PlayerPawn || !PlayerPawn->IsHeroAlive())
	{
		return;
	}

	PlayerPawn->ApplyDamage(Damage);
	if (UT66MiniVFXSubsystem* VfxSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVFXSubsystem>() : nullptr)
	{
		if (UT66MiniVisualSubsystem* VisualSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniVisualSubsystem>() : nullptr)
		{
			if (UTexture2D* ImpactTexture = VisualSubsystem->LoadEffectTexture(TEXT("EnemyProjectile_Impact")))
			{
				VfxSubsystem->SpawnSpritePulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 6.f), FVector(0.20f, 0.20f, 1.f), 0.12f, FLinearColor::White, ImpactTexture, 0.50f);
			}
			else
			{
				VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 6.f), FVector(0.14f, 0.14f, 1.f), 0.10f, FLinearColor(1.0f, 0.36f, 0.22f, 0.28f), 0.40f);
			}
		}
		else
		{
			VfxSubsystem->SpawnPulse(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 6.f), FVector(0.14f, 0.14f, 1.f), 0.10f, FLinearColor(1.0f, 0.36f, 0.22f, 0.28f), 0.40f);
		}
	}

	Destroy();
}

void AT66MiniEnemyProjectile::OnRep_ProjectileTexture()
{
	if (ProjectileTexture)
	{
		SpriteComponent->SetSprite(ProjectileTexture);
	}
}
