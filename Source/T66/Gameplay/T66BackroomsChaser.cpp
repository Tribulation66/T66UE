// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66BackroomsChaser.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Data/T66DataTypes.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/T66GameMode.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Materials/MaterialInstanceDynamic.h"

AT66BackroomsChaser::AT66BackroomsChaser()
{
	PrimaryActorTick.bCanEverTick = true;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCapsuleRadius(56.f);
		Capsule->SetCapsuleHalfHeight(96.f);
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = MoveSpeed;
		Move->bOrientRotationToMovement = true;
	}

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, 62.f));
	VisualMesh->SetRelativeScale3D(FVector(2.1f, 2.1f, 2.1f));
	if (UStaticMesh* Sphere = FT66VisualUtil::GetBasicShapeSphere())
	{
		VisualMesh->SetStaticMesh(Sphere);
	}

	if (UMaterialInterface* ColorMat = FT66VisualUtil::GetFlatColorMaterial())
	{
		if (UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(ColorMat, this))
		{
			FT66VisualUtil::ConfigureFlatColorMaterial(Mat, FLinearColor(0.86f, 0.93f, 0.20f, 1.f));
			VisualMesh->SetMaterial(0, Mat);
		}
	}
}

void AT66BackroomsChaser::BeginPlay()
{
	Super::BeginPlay();

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AT66BackroomsChaser::OnCapsuleBeginOverlap);
	}

	LoadUniqueEnemyData();
	ApplyVisual();
}

void AT66BackroomsChaser::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bTouchTriggered)
	{
		return;
	}

	AT66GameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr;
	if (!GameMode || !GameMode->IsBackroomsChallengeActive())
	{
		return;
	}

	AT66HeroBase* Hero = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			Hero = Cast<AT66HeroBase>(PC->GetPawn());
		}
	}
	if (!Hero)
	{
		return;
	}

	FVector MoveTarget = FVector::ZeroVector;
	if (!GameMode->GetBackroomsChaserMoveTarget(GetActorLocation(), Hero->GetActorLocation(), MoveTarget))
	{
		MoveTarget = Hero->GetActorLocation();
	}

	FVector ToTarget = MoveTarget - GetActorLocation();
	ToTarget.Z = 0.f;
	if (!ToTarget.IsNearlyZero())
	{
		const FVector Direction = ToTarget.GetSafeNormal2D();
		const FVector Delta = Direction * MoveSpeed * FMath::Max(0.f, DeltaSeconds);
		FHitResult MoveHit;
		AddActorWorldOffset(Delta, true, &MoveHit, ETeleportType::None);
		SetActorRotation(Direction.Rotation());
	}

	if (FVector::DistSquared2D(GetActorLocation(), Hero->GetActorLocation()) <= FMath::Square(130.f))
	{
		TryTouchHero(Hero);
	}
}

void AT66BackroomsChaser::OnCapsuleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	TryTouchHero(Cast<AT66HeroBase>(OtherActor));
}

void AT66BackroomsChaser::LoadUniqueEnemyData()
{
	UT66GameInstance* GI = GetWorld() ? Cast<UT66GameInstance>(GetWorld()->GetGameInstance()) : nullptr;
	if (!GI)
	{
		return;
	}

	FUniqueEnemyData Data;
	if (!GI->GetUniqueEnemyData(UniqueEnemyID, Data))
	{
		return;
	}

	CharacterVisualID = Data.CharacterVisualID.IsNone() ? CharacterVisualID : Data.CharacterVisualID;
	MoveSpeed = FMath::Max(100.f, Data.MoveSpeed);
	TouchDamageHP = FMath::Max(1, Data.TouchDamageHP);
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = MoveSpeed;
	}
}

void AT66BackroomsChaser::ApplyVisual()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UT66CharacterVisualSubsystem* Visuals = GI ? GI->GetSubsystem<UT66CharacterVisualSubsystem>() : nullptr;
	if (!Visuals)
	{
		return;
	}

	Visuals->ApplyCharacterVisual(CharacterVisualID, GetMesh(), VisualMesh, true, false, false, VisualMesh);
}

void AT66BackroomsChaser::TryTouchHero(AT66HeroBase* Hero)
{
	if (bTouchTriggered || !Hero)
	{
		return;
	}

	if (AT66GameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AT66GameMode>() : nullptr)
	{
		bTouchTriggered = true;
		GameMode->HandleBackroomsChaserTouchedHero(this, Hero);
	}
}
