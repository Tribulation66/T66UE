// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66GamblerBoss.h"
#include "Gameplay/T66BossProjectile.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AT66GamblerBoss::AT66GamblerBoss()
{
	PrimaryActorTick.bCanEverTick = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = MoveSpeed;
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Align primitive mesh to ground when capsule is grounded:
	// capsule half-height~88, sphere half-height=50*4=200 => relative Z = 200 - 88 = 112.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 112.f));

	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		VisualMesh->SetStaticMesh(Sphere);
		VisualMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 4.0f)); // smaller than stage boss, larger than mobs
	}
	if (UMaterialInterface* ColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlaceholderColor.M_PlaceholderColor")))
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this))
		{
			Mat->SetVectorParameterValue(FName("Color"), FLinearColor(0.85f, 0.75f, 0.15f, 1.f)); // casino yellow
			VisualMesh->SetMaterial(0, Mat);
		}
	}
}

void AT66GamblerBoss::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;

	// Show boss bar immediately
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->SetBossActive(MaxHP);
			}
		}
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &AT66GamblerBoss::FireAtPlayer, FireIntervalSeconds, true, 0.35f);
	}
}

void AT66GamblerBoss::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentHP <= 0) return;
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn) return;

	FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Len = ToPlayer.Size();
	if (Len > 10.f)
	{
		ToPlayer /= Len;
		AddMovementInput(ToPlayer, 1.f);
	}
}

void AT66GamblerBoss::FireAtPlayer()
{
	if (CurrentHP <= 0) return;
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

bool AT66GamblerBoss::TakeDamageFromHeroHit(int32 DamageAmount)
{
	if (DamageAmount <= 0 || CurrentHP <= 0) return false;

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

void AT66GamblerBoss::Die()
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}

	if (UGameInstance* GI = World ? World->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			RunState->SetBossInactive();
			RunState->ResetGamblerAnger();
		}
	}

	Destroy();
}

