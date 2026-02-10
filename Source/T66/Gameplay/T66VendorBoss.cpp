// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66VendorBoss.h"
#include "Gameplay/T66BossProjectile.h"
#include "Gameplay/T66VendorNPC.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AT66VendorBoss::AT66VendorBoss()
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
	// capsule half-height~88, cube half-height=50*4=200 => relative Z = 200 - 88 = 112.
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 112.f));

	if (UStaticMesh* Cube = FT66VisualUtil::GetBasicShapeCube())
	{
		VisualMesh->SetStaticMesh(Cube);
		VisualMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 4.0f));
	}
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.08f, 0.08f, 0.10f, 1.f));

	// Prepare built-in SkeletalMeshComponent for imported models.
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Skel->SetVisibility(false, true);
	}
}

void AT66VendorBoss::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;

	// Vendor boss must use the same mesh as Vendor NPC (per rule).
	// We use a dedicated VisualID so the boss can have a capsule-friendly transform,
	// while still referencing the same SkeletalMesh asset as the Vendor NPC.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const bool bApplied = Visuals->ApplyCharacterVisual(FName(TEXT("VendorBoss")), GetMesh(), VisualMesh, true);
			if (!bApplied && GetMesh())
			{
				GetMesh()->SetVisibility(false, true);
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
			{
				RunState->SetBossActiveWithId(FName(TEXT("VendorBoss")), MaxHP);
			}
		}
		World->GetTimerManager().SetTimer(FireTimerHandle, this, &AT66VendorBoss::FireAtPlayer, FireIntervalSeconds, true, 0.25f);
	}
}

void AT66VendorBoss::Tick(float DeltaSeconds)
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

void AT66VendorBoss::FireAtPlayer()
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

bool AT66VendorBoss::TakeDamageFromHeroHit(int32 DamageAmount, FName DamageSourceID)
{
	if (DamageAmount <= 0 || CurrentHP <= 0) return false;
	const FName SourceID = DamageSourceID.IsNone() ? UT66DamageLogSubsystem::SourceID_AutoAttack : DamageSourceID;
	UWorld* World = GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		DamageLog->RecordDamageDealt(SourceID, DamageAmount);
	}
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

void AT66VendorBoss::Die()
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
			RunState->ResetVendorAnger();
		}
	}

	// Respawn Vendor NPC so the house becomes interactable again this stage.
	if (World)
	{
		FActorSpawnParameters P;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AT66VendorNPC>(AT66VendorNPC::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, P);
	}

	Destroy();
}

