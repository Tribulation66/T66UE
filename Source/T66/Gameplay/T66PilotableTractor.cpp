// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66PilotableTractor.h"

#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

AT66PilotableTractor::AT66PilotableTractor()
{
	PrimaryActorTick.bCanEverTick = true;

	TractorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TractorRoot"));
	SetRootComponent(TractorRoot);

	BodyCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyCollision"));
	BodyCollision->SetBoxExtent(FVector(180.f, 120.f, 95.f));
	BodyCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BodyCollision->SetCollisionObjectType(ECC_WorldDynamic);
	BodyCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	BodyCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BodyCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BodyCollision->SetCanEverAffectNavigation(false);
	BodyCollision->SetupAttachment(TractorRoot);
	BodyCollision->SetRelativeLocation(FVector(0.f, 0.f, 95.f));

	TriggerBox->SetupAttachment(TractorRoot);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetBoxExtent(FVector(260.f, 220.f, 180.f));
	TriggerBox->SetRelativeLocation(FVector(0.f, 0.f, 140.f));

	VisualMesh->SetupAttachment(TractorRoot);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SingleMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/World/Props/Tractor.Tractor")));

	PromptText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PromptText"));
	PromptText->SetupAttachment(TractorRoot);
	PromptText->SetHorizontalAlignment(EHTA_Center);
	PromptText->SetVerticalAlignment(EVRTA_TextCenter);
	PromptText->SetWorldSize(82.f);
	PromptText->SetTextRenderColor(FColor::White);
	PromptText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PromptText->SetHiddenInGame(true, true);
	PromptText->SetVisibility(false, true);

	RemainingPilotSeconds = TotalPilotSeconds;
	ApplyRarityVisuals();
}

bool AT66PilotableTractor::Interact(APlayerController* PC)
{
	AT66HeroBase* Hero = PC ? Cast<AT66HeroBase>(PC->GetPawn()) : nullptr;
	if (!Hero)
	{
		return false;
	}

	if (MountedHero.IsValid())
	{
		if (MountedHero.Get() != Hero)
		{
			return false;
		}

		DismountHero(false);
		return true;
	}

	if (RemainingPilotSeconds <= KINDA_SMALL_NUMBER)
	{
		ExpireTractor();
		return false;
	}

	return MountHero(Hero);
}

bool AT66PilotableTractor::IsMountedByHero(const AT66HeroBase* Hero) const
{
	return Hero && MountedHero.IsValid() && MountedHero.Get() == Hero;
}

void AT66PilotableTractor::SetDriveForwardInput(float Value)
{
	ForwardDriveInput = FMath::Clamp(Value, -1.f, 1.f);
}

void AT66PilotableTractor::SetDriveRightInput(float Value)
{
	RightDriveInput = FMath::Clamp(Value, -1.f, 1.f);
}

void AT66PilotableTractor::BeginPlay()
{
	Super::BeginPlay();

	RemainingPilotSeconds = FMath::Max(0.f, TotalPilotSeconds);
	TryApplyImportedMesh();
	FT66VisualUtil::SnapToGround(this, GetWorld());
	UpdatePromptPlacement();
	LastFrameLocation = GetActorLocation();
	UpdatePrompt();
}

void AT66PilotableTractor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MountedHero.IsValid())
	{
		TickMountedDriving(DeltaSeconds);
		RemainingPilotSeconds = FMath::Max(0.f, RemainingPilotSeconds - DeltaSeconds);
		HandleMountedEnemyMow(DeltaSeconds);
		if (RemainingPilotSeconds <= KINDA_SMALL_NUMBER)
		{
			ExpireTractor();
			return;
		}
	}
	else
	{
		CurrentVelocity = FVector::ZeroVector;
	}

	LastFrameLocation = GetActorLocation();
	UpdatePrompt();
}

void AT66PilotableTractor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AT66HeroBase* Hero = MountedHero.Get())
	{
		Hero->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		if (Hero->CombatComponent)
		{
			bool bShouldSuppressAutoAttack = false;
			if (AT66PlayerController* PC = Cast<AT66PlayerController>(Hero->GetController()))
			{
				bShouldSuppressAutoAttack = PC->IsHeroOneScopedUltActive();
			}
			Hero->CombatComponent->SetAutoAttackSuppressed(bShouldSuppressAutoAttack);
		}
		Hero->SetVehicleMounted(false, nullptr);
	}

	MountedHero.Reset();

	Super::EndPlay(EndPlayReason);
}

void AT66PilotableTractor::ApplyRarityVisuals()
{
	if (TryApplyImportedMesh())
	{
		UpdatePromptPlacement();
		return;
	}

	Super::ApplyRarityVisuals();
	UpdatePromptPlacement();
}

bool AT66PilotableTractor::MountHero(AT66HeroBase* Hero)
{
	if (!Hero || MountedHero.IsValid() || RemainingPilotSeconds <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	MountedHero = Hero;
	ForwardDriveInput = 0.f;
	RightDriveInput = 0.f;
	CurrentVelocity = FVector::ZeroVector;
	LastFrameLocation = GetActorLocation();

	if (Hero->CombatComponent)
	{
		Hero->CombatComponent->SetAutoAttackSuppressed(true);
		Hero->CombatComponent->ClearLockedTarget();
	}

	Hero->SetVehicleMounted(true, this, MountedHeroVisualOffset, MountedHeroVisualRotation);
	Hero->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	Hero->SetActorRelativeLocation(MountedHeroSeatOffset);
	Hero->SetActorRelativeRotation(FRotator::ZeroRotator);
	Hero->SetActorRotation(GetActorRotation());

	MowCheckAccumSeconds = MowCheckInterval;
	UpdatePrompt();
	return true;
}

void AT66PilotableTractor::DismountHero(bool bDestroyAfterExit)
{
	AT66HeroBase* Hero = MountedHero.Get();
	MountedHero.Reset();
	ForwardDriveInput = 0.f;
	RightDriveInput = 0.f;
	CurrentVelocity = FVector::ZeroVector;

	if (Hero)
	{
		const float CapsuleHalfHeight = Hero->GetCapsuleComponent() ? Hero->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.f;
		FVector ExitLoc = GetActorLocation() + GetActorRightVector() * 220.f + FVector(0.f, 0.f, CapsuleHalfHeight + 20.f);
		if (UWorld* World = GetWorld())
		{
			FHitResult Hit;
			const FVector TraceStart = ExitLoc + FVector(0.f, 0.f, 600.f);
			const FVector TraceEnd = ExitLoc - FVector(0.f, 0.f, 1600.f);
			if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic))
			{
				ExitLoc = Hit.ImpactPoint + FVector(0.f, 0.f, CapsuleHalfHeight + 5.f);
			}
		}

		Hero->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Hero->SetActorLocation(ExitLoc, false, nullptr, ETeleportType::TeleportPhysics);
		Hero->SetActorRotation(GetActorRotation());
		Hero->SetVehicleMounted(false, nullptr);

		if (Hero->CombatComponent)
		{
			bool bShouldSuppressAutoAttack = false;
			if (AT66PlayerController* PC = Cast<AT66PlayerController>(Hero->GetController()))
			{
				bShouldSuppressAutoAttack = PC->IsHeroOneScopedUltActive();
			}
			Hero->CombatComponent->SetAutoAttackSuppressed(bShouldSuppressAutoAttack);
		}
	}

	UpdatePrompt();

	if (bDestroyAfterExit)
	{
		bConsumed = true;
		Destroy();
	}
}

void AT66PilotableTractor::ExpireTractor()
{
	if (MountedHero.IsValid())
	{
		DismountHero(false);
	}

	bConsumed = true;
	Destroy();
}

void AT66PilotableTractor::TickMountedDriving(float DeltaSeconds)
{
	AT66HeroBase* Hero = MountedHero.Get();
	if (!Hero || DeltaSeconds <= KINDA_SMALL_NUMBER)
	{
		CurrentVelocity = FVector::ZeroVector;
		return;
	}

	AT66PlayerController* PC = Cast<AT66PlayerController>(Hero->GetController());
	if (!PC)
	{
		CurrentVelocity = FVector::ZeroVector;
		return;
	}
	(void)PC;

	const float TurnAmount = RightDriveInput * TurnSpeedDegreesPerSecond * DeltaSeconds;
	if (!FMath::IsNearlyZero(TurnAmount))
	{
		AddActorWorldRotation(FRotator(0.f, TurnAmount, 0.f));
	}

	if (!FMath::IsNearlyZero(ForwardDriveInput))
	{
		FHitResult SweepHit;
		const FVector Delta = GetActorForwardVector() * (DriveSpeed * ForwardDriveInput * DeltaSeconds);
		AddActorWorldOffset(Delta, true, &SweepHit, ETeleportType::None);
	}

	FT66VisualUtil::SnapToGround(this, GetWorld());

	CurrentVelocity = (GetActorLocation() - LastFrameLocation) / DeltaSeconds;
}

void AT66PilotableTractor::HandleMountedEnemyMow(float DeltaSeconds)
{
	AT66HeroBase* Hero = MountedHero.Get();
	UWorld* World = GetWorld();
	if (!Hero || !World || Hero->IsInSafeZone())
	{
		return;
	}

	if (CurrentVelocity.SizeSquared2D() < FMath::Square(MowMinSpeed))
	{
		return;
	}

	MowCheckAccumSeconds += DeltaSeconds;
	if (MowCheckAccumSeconds < MowCheckInterval)
	{
		return;
	}
	MowCheckAccumSeconds = 0.f;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(T66TractorMow), false, this);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Hero);

	World->OverlapMultiByChannel(
		Overlaps,
		GetActorLocation() + FVector(0.f, 0.f, 85.f),
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(MowKillRadius),
		Params);

	for (const FOverlapResult& Result : Overlaps)
	{
		AT66EnemyBase* Enemy = Cast<AT66EnemyBase>(Result.GetActor());
		if (!Enemy || Enemy->CurrentHP <= 0)
		{
			continue;
		}

		Enemy->TakeDamageFromHero(Enemy->CurrentHP + Enemy->MaxHP + 1, FName(TEXT("Tractor")));
	}
}

void AT66PilotableTractor::UpdatePrompt()
{
	if (!PromptText)
	{
		return;
	}

	AT66HeroBase* LocalHero = Cast<AT66HeroBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	const bool bMountedByLocalHero = LocalHero && IsMountedByHero(LocalHero);
	const bool bCanMount = !MountedHero.IsValid() && RemainingPilotSeconds > KINDA_SMALL_NUMBER;
	const bool bHeroInRange = LocalHero
		&& FVector::DistSquared2D(GetActorLocation(), LocalHero->GetActorLocation()) <= FMath::Square(PromptVisibleDistance);
	const bool bShowPrompt = bMountedByLocalHero || (bCanMount && bHeroInRange);

	PromptText->SetText(FText::FromString(BuildPromptString()));
	PromptText->SetHiddenInGame(!bShowPrompt, true);
	PromptText->SetVisibility(bShowPrompt, true);

	if (bShowPrompt)
	{
		UpdatePromptFacing();
	}
}

void AT66PilotableTractor::UpdatePromptFacing() const
{
	if (!PromptText)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	FVector CameraLocation = FVector::ZeroVector;
	FRotator CameraRotation = FRotator::ZeroRotator;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector ToCamera = CameraLocation - PromptText->GetComponentLocation();
	if (ToCamera.IsNearlyZero())
	{
		return;
	}

	const FRotator Facing = ToCamera.Rotation();
	PromptText->SetWorldRotation(FRotator(0.f, Facing.Yaw, 0.f));
}

void AT66PilotableTractor::UpdatePromptPlacement()
{
	if (!PromptText)
	{
		return;
	}

	const float CollisionHeight = BodyCollision ? (BodyCollision->GetUnscaledBoxExtent().Z * 2.f) : 180.f;
	const float VisualHeight = VisualMesh ? (VisualMesh->Bounds.BoxExtent.Z * 2.f) : 200.f;
	const float PromptHeight = FMath::Max(CollisionHeight, VisualHeight) + 140.f;
	PromptText->SetRelativeLocation(FVector(0.f, 0.f, PromptHeight));
}

FString AT66PilotableTractor::BuildPromptString() const
{
	if (RemainingPilotSeconds <= KINDA_SMALL_NUMBER)
	{
		return FString();
	}

	const int32 DisplaySeconds = FMath::Max(1, FMath::CeilToInt(RemainingPilotSeconds));
	if (MountedHero.IsValid())
	{
		return FString::Printf(TEXT("Press F to Exit (%ds)"), DisplaySeconds);
	}

	return FString::Printf(TEXT("Press F to Pilot (%ds)"), DisplaySeconds);
}
