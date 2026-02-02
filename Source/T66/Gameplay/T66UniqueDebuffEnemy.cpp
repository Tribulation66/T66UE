// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66UniqueDebuffEnemy.h"

#include "Gameplay/T66UniqueDebuffProjectile.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Core/T66RunStateSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
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

	// Visuals are stage-driven in BeginPlay so each stage's Unique enemy looks different.
	if (VisualMesh)
	{
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 40.f)); // float above capsule center
	}
}

void AT66UniqueDebuffEnemy::BeginPlay()
{
	// Stage-driven tuning + visuals (so each stage has its own unique enemy flavor).
	int32 StageNum = 1;
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
		{
			StageNum = RunState->GetCurrentStage();
		}
	}
	StageNum = FMath::Clamp(StageNum, 1, 66);

	// Stage 1/2: replace placeholder with real assets (walk anim for stage 1; static for stage 2).
	// NOTE: We set CharacterVisualID BEFORE Super::BeginPlay so base class can apply it immediately.
	if (StageNum == 1)
	{
		CharacterVisualID = FName(TEXT("Unique_Stage01"));
	}
	else
	{
		// Keep placeholder default for other stages (and stage 2 uses a static mesh on VisualMesh).
		CharacterVisualID = NAME_None;
	}

	Super::BeginPlay();

	// Scale stats up as stages increase.
	MaxHP = FMath::Clamp(60 + (StageNum * 3), 60, 9999);
	CurrentHP = MaxHP;
	FireIntervalSeconds = FMath::Clamp(1.40f - (static_cast<float>(StageNum) * 0.006f), 0.55f, 1.40f);
	LifetimeSeconds = FMath::Clamp(10.f + (static_cast<float>(StageNum) * 0.12f), 10.f, 22.f);

	// Visuals:
	// - Stage 1 uses imported skeletal mesh via DT_CharacterVisuals (handled by Super::BeginPlay).
	// - Stage 2 uses a static mesh (no animation) on the placeholder VisualMesh.
	// - All other stages keep the existing placeholder "stage-driven" look.
	if (StageNum == 2)
	{
		if (VisualMesh)
		{
			if (UStaticMesh* SM = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Characters/Enemies/Stages/Unique_Stage02/SM_Unique_Stage02.SM_Unique_Stage02")))
			{
				VisualMesh->SetStaticMesh(SM);
				VisualMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 0.85f));

				// Same root cause as some skeletal meshes: FBX pivot can be far from geometry.
				// Recenter the static mesh so it appears at the actor location.
				const FBoxSphereBounds B = SM->GetBounds();
				const FVector Scale = VisualMesh->GetRelativeScale3D();
				const FVector LocalOrigin(B.Origin.X * Scale.X, B.Origin.Y * Scale.Y, B.Origin.Z * Scale.Z);
				const FVector RotatedOrigin = VisualMesh->GetRelativeRotation().RotateVector(LocalOrigin);
				VisualMesh->AddRelativeLocation(-RotatedOrigin);
				// Keep default location offset (hover) and do not tint (use authored materials/textures).
			}
		}
	}
	else if (StageNum != 1)
	{
		UStaticMesh* Shape = nullptr;
		switch (StageNum % 4)
		{
			case 1: Shape = FT66VisualUtil::GetBasicShapeSphere(); break;
			case 2: Shape = FT66VisualUtil::GetBasicShapeCube(); break;
			case 3: Shape = FT66VisualUtil::GetBasicShapeCylinder(); break;
			default: Shape = FT66VisualUtil::GetBasicShapeCone(); break;
		}
		if (Shape)
		{
			VisualMesh->SetStaticMesh(Shape);
		}

		const float Hue = FMath::Fmod(static_cast<float>(StageNum) * 41.f, 360.f);
		FLinearColor C = FLinearColor::MakeFromHSV8(static_cast<uint8>(Hue / 360.f * 255.f), 220, 245);
		C.A = 1.f;
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, C);

		// Slight size ramp over the run.
		const float S = FMath::Lerp(0.70f, 1.05f, static_cast<float>(StageNum - 1) / 65.f);
		VisualMesh->SetRelativeScale3D(FVector(S, S, S));
	}

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

