// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66UniqueDebuffEnemy.h"

#include "Gameplay/T66UniqueDebuffProjectile.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AT66UniqueDebuffEnemy::AT66UniqueDebuffEnemy()
{
	// Unique enemies intentionally keep their placeholder look (no imported mesh).
	CharacterVisualID = NAME_None;

	// Unique enemies do not drop loot or XP in the Bible.
	bDropsLoot = false;
	XPValue = 0;

	MaxHP = 60;
	CurrentHP = MaxHP;
	TouchDamageHearts = 1;
	PointValue = 0;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = 420.f;
		Move->SetMovementMode(MOVE_Flying);
	}

	// Visual: make it clearly "different" (purple).
	FT66VisualUtil::ApplyT66Color(VisualMesh, this, FLinearColor(0.65f, 0.20f, 0.90f, 1.f));
	if (VisualMesh)
	{
		if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
		{
			VisualMesh->SetStaticMesh(Sphere);
		}
		VisualMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 40.f)); // float above capsule center
	}
}

void AT66UniqueDebuffEnemy::BeginPlay()
{
	Super::BeginPlay();
	LifeRemaining = LifetimeSeconds;

	// Lift to hover height (spawn loc is at capsule half-height; we add hover offset).
	SetActorLocation(GetActorLocation() + FVector(0.f, 0.f, HoverHeight));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FireTimer, this, &AT66UniqueDebuffEnemy::FireAtPlayer, FireIntervalSeconds, true, 0.5f);
	}
}

void AT66UniqueDebuffEnemy::Tick(float DeltaSeconds)
{
	// Keep base enemy behavior (safe-zone avoidance + chase), but allow flying.
	Super::Tick(DeltaSeconds);

	LifeRemaining = FMath::Max(0.f, LifeRemaining - DeltaSeconds);
	if (LifeRemaining <= 0.f)
	{
		Destroy();
	}
}

void AT66UniqueDebuffEnemy::FireAtPlayer()
{
	if (CurrentHP <= 0) return;
	UWorld* World = GetWorld();
	if (!World) return;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	AT66HeroBase* Hero = Cast<AT66HeroBase>(PlayerPawn);
	if (!Hero) return;
	if (Hero->IsInSafeZone()) return;

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 40.f);
	const FVector Target = Hero->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	FVector Dir = (Target - Start);
	if (Dir.IsNearlyZero()) return;
	Dir.Normalize();

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform SpawnTM(Dir.Rotation(), Start);
	AT66UniqueDebuffProjectile* Proj = World->SpawnActorDeferred<AT66UniqueDebuffProjectile>(AT66UniqueDebuffProjectile::StaticClass(), SpawnTM, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Proj) return;

	// Randomize effect per shot.
	const float Roll = FMath::FRand();
	if (Roll < 0.34f) Proj->EffectType = ET66HeroStatusEffectType::Burn;
	else if (Roll < 0.67f) Proj->EffectType = ET66HeroStatusEffectType::Chill;
	else Proj->EffectType = ET66HeroStatusEffectType::Curse;
	Proj->EffectDurationSeconds = 4.5f;

	UGameplayStatics::FinishSpawningActor(Proj, SpawnTM);

	if (Proj->ProjectileMovement)
	{
		Proj->ProjectileMovement->Velocity = Dir * Proj->ProjectileMovement->InitialSpeed;
	}
	Proj->SetActorRotation(Dir.Rotation());
}

