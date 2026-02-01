// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BossBase.h"
#include "Gameplay/T66BossProjectile.h"
#include "Gameplay/T66GameMode.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "AIController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AT66BossBase::AT66BossBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Needed so AddMovementInput works (it requires a Controller when bForce=false).
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 350.f;
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Align primitive mesh to ground when capsule is grounded:
	// capsule half-height~88, sphere half-height=50*6=300 => relative Z = 300 - 88 = 212.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 212.f));

	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(6.f, 6.f, 6.f)); // very large sphere
	}
	if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Mat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.9f, 0.05f, 0.05f, 1.f));
	}

	// Prepare built-in SkeletalMeshComponent for imported models.
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Skel->SetVisibility(false, true); // shown only when a character visual mapping exists
	}
}

void AT66BossBase::InitializeBoss(const FBossData& BossData)
{
	BossID = BossData.BossID;
	// Standardized HP baseline: bosses are 1000+ HP.
	MaxHP = FMath::Max(1000, BossData.MaxHP);
	AwakenDistance = BossData.AwakenDistance;
	FireIntervalSeconds = BossData.FireIntervalSeconds;
	ProjectileSpeed = BossData.ProjectileSpeed;
	ProjectileDamageHearts = BossData.ProjectileDamageHearts;
	PointValue = FMath::Max(0, BossData.PointValue);

	// Conservative default if DT doesn't specify a score: tie to HP scale.
	if (PointValue <= 0)
	{
		PointValue = FMath::Clamp(MaxHP / 10, 100, 5000);
	}

	BaseMaxHP = MaxHP;
	BaseProjectileDamageHearts = ProjectileDamageHearts;
	bBaseTuningInitialized = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = BossData.MoveSpeed;
	}
	if (VisualMesh)
	{
		if (UMaterialInstanceDynamic* Mat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			Mat->SetVectorParameterValue(TEXT("BaseColor"), BossData.PlaceholderColor);
		}
	}

	// Imported model: map BossID -> mesh (DT_CharacterVisuals).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const bool bApplied = Visuals->ApplyCharacterVisual(BossID, GetMesh(), VisualMesh, true);
			if (!bApplied && GetMesh())
			{
				GetMesh()->SetVisibility(false, true);
			}
		}
	}

	// Apply current run difficulty (boss is usually dormant until awaken).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			ApplyDifficultyScalar(RunState->GetDifficultyScalar());
		}
	}
}

void AT66BossBase::ApplyDifficultyScalar(float Scalar)
{
	if (!bBaseTuningInitialized)
	{
		BaseMaxHP = MaxHP;
		BaseProjectileDamageHearts = ProjectileDamageHearts;
		bBaseTuningInitialized = true;
	}

	const float ClampedScalar = FMath::Clamp(Scalar, 1.0f, 99.0f);
	MaxHP = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseMaxHP) * ClampedScalar));
	ProjectileDamageHearts = FMath::Max(1, FMath::RoundToInt(static_cast<float>(BaseProjectileDamageHearts) * ClampedScalar));

	// If boss is active, rescale the RunState boss bar max HP while preserving percent.
	if (bAwakened)
	{
		if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->RescaleBossMaxHPPreservePercent(MaxHP);
				CurrentHP = RunState->GetBossCurrentHP();
			}
		}
	}
}

void AT66BossBase::BeginPlay()
{
	Super::BeginPlay();
	bAwakened = false;
	CurrentHP = 0;
}

void AT66BossBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	const FVector MyLoc = GetActorLocation();
	const FVector PlayerLoc = PlayerPawn->GetActorLocation();

	if (!bAwakened)
	{
		if (FVector::DistSquared(MyLoc, PlayerLoc) <= AwakenDistance * AwakenDistance)
		{
			Awaken();
		}
		return;
	}

	// Chase player
	FVector ToPlayer = PlayerLoc - MyLoc;
	ToPlayer.Z = 0.f;
	const float Len = ToPlayer.Size();
	if (Len > 10.f)
	{
		ToPlayer /= Len;
		AddMovementInput(ToPlayer, 1.f);
	}
}

void AT66BossBase::Awaken()
{
	if (bAwakened) return;
	bAwakened = true;
	CurrentHP = MaxHP;

	// Show boss bar (100/100) when awakened
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->SetBossActiveWithId(BossID, MaxHP);
	}

	// Start firing projectiles
	if (World)
	{
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &AT66BossBase::FireAtPlayer, FireIntervalSeconds, true, 0.25f);
	}
}

void AT66BossBase::FireAtPlayer()
{
	if (!bAwakened || CurrentHP <= 0) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	const FVector SpawnLoc = GetActorLocation() + FVector(0.f, 0.f, 80.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AT66BossProjectile* Proj = World->SpawnActor<AT66BossProjectile>(AT66BossProjectile::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Proj)
	{
		Proj->DamageHearts = ProjectileDamageHearts;
		Proj->SetTargetLocation(PlayerPawn->GetActorLocation(), ProjectileSpeed);
	}
}

bool AT66BossBase::TakeDamageFromHeroHit(int32 DamageAmount)
{
	if (!bAwakened || CurrentHP <= 0) return false;
	if (DamageAmount <= 0) return false;

	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		const bool bDied = RunState->ApplyBossDamage(DamageAmount);
		CurrentHP = RunState->GetBossCurrentHP();
		if (bDied)
		{
			Die();
			return true;
		}
	}

	if (CurrentHP <= 0)
	{
		Die();
		return true;
	}

	return false;
}

void AT66BossBase::Die()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}

	// Delegate death handling to GameMode (normal vs Coliseum).
	if (AT66GameMode* GM = World ? World->GetAuthGameMode<AT66GameMode>() : nullptr)
	{
		GM->HandleBossDefeated(this);
	}

	Destroy();
}

