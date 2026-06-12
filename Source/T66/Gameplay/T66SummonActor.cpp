// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66SummonActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/T66VisualUtil.h"

AT66SummonActor::AT66SummonActor()
{
	PrimaryActorTick.bCanEverTick = true;

	ContactSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ContactSphere"));
	SetRootComponent(ContactSphere);
	ContactSphere->InitSphereRadius(ContactRadius);
	ContactSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ContactSphere->SetCollisionObjectType(ECC_WorldDynamic);
	ContactSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ContactSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ContactSphere->SetGenerateOverlapEvents(true);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(ContactSphere);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetGenerateOverlapEvents(false);
}

void AT66SummonActor::BeginPlay()
{
	Super::BeginPlay();

	if (VisualMesh)
	{
		VisualMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeSphere());
		VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, ContactRadius * 0.35f));
		VisualMesh->SetWorldScale3D(FVector(FMath::Max(0.08f, ContactRadius / 50.f)));
	}

	if (ContactSphere)
	{
		ContactSphere->OnComponentBeginOverlap.AddDynamic(this, &AT66SummonActor::OnContactBeginOverlap);
	}
}

void AT66SummonActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwnerCombat)
	{
		OwnerCombat->NotifySummonDestroyed(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66SummonActor::InitializeSummon(
	UT66CombatComponent* InOwnerCombat,
	const FName InIdolID,
	const ET66IdolElement InElement,
	const ET66ItemRarity InRarity,
	const int32 InDamage,
	const float InStatusChance,
	const float InStatusDuration,
	const float InMoveSpeed,
	const float InContactRadius,
	const float InLifetimeSeconds,
	const int32 InMaxHits,
	const float InBounceSpeed,
	const float InSearchRange,
	AActor* InInitialTarget)
{
	OwnerCombat = InOwnerCombat;
	IdolID = InIdolID;
	Element = InElement;
	Rarity = InRarity;
	Damage = FMath::Max(1, InDamage);
	StatusChance = FMath::Clamp(InStatusChance, 0.f, 1.f);
	StatusDuration = FMath::Max(0.f, InStatusDuration);
	MoveSpeed = FMath::Max(1.f, InMoveSpeed);
	ContactRadius = FMath::Max(10.f, InContactRadius);
	LifetimeSeconds = FMath::Max(0.1f, InLifetimeSeconds);
	MaxHits = FMath::Max(1, InMaxHits);
	BounceSpeed = FMath::Max(1.f, InBounceSpeed);
	SearchRange = FMath::Max(100.f, InSearchRange);
	CurrentTarget = InInitialTarget;

	if (ContactSphere)
	{
		ContactSphere->SetSphereRadius(ContactRadius);
	}

	if (VisualMesh)
	{
		VisualMesh->SetWorldScale3D(FVector(FMath::Max(0.08f, ContactRadius / 50.f)));
		FT66VisualUtil::ApplyT66Color(VisualMesh, this, UT66CombatComponent::GetIdolElementColor(Element));
	}

	if (!CurrentTarget.IsValid() || !OwnerCombat || !OwnerCombat->IsActorValidSummonTarget(CurrentTarget.Get()))
	{
		RefreshTarget();
	}
}

void AT66SummonActor::ApplyVisualMeshOverride(UStaticMesh* InMesh, const float InMeshScale)
{
	if (!VisualMesh || !InMesh)
	{
		return;
	}

	VisualMesh->SetStaticMesh(InMesh);
	VisualMesh->EmptyOverrideMaterials();
	VisualMesh->SetRelativeLocation(FVector::ZeroVector);
	VisualMesh->SetRelativeRotation(FRotator::ZeroRotator);
	VisualMesh->SetRelativeScale3D(FVector(FMath::Max(0.05f, InMeshScale)));
	VisualMesh->SetVisibility(true, true);
	VisualMesh->SetHiddenInGame(false, true);
	VisualMesh->SetRenderInMainPass(true);
}

void AT66SummonActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AgeSeconds += DeltaSeconds;
	LastHitCooldownSeconds = FMath::Max(0.f, LastHitCooldownSeconds - DeltaSeconds);
	if (AgeSeconds >= LifetimeSeconds || HitCount >= MaxHits)
	{
		Destroy();
		return;
	}

	if (!OwnerCombat)
	{
		Destroy();
		return;
	}

	FVector MoveDelta = FVector::ZeroVector;
	if (BounceSecondsRemaining > 0.f)
	{
		BounceSecondsRemaining = FMath::Max(0.f, BounceSecondsRemaining - DeltaSeconds);
		MoveDelta = BounceVelocity * DeltaSeconds;
	}
	else
	{
		if (!CurrentTarget.IsValid() || !OwnerCombat->IsActorValidSummonTarget(CurrentTarget.Get()))
		{
			RefreshTarget();
		}

		if (AActor* Target = CurrentTarget.Get())
		{
			const FVector ToTarget = OwnerCombat->GetSummonTargetAimPoint(Target) - GetActorLocation();
			const FVector Direction = ToTarget.GetSafeNormal2D();
			MoveDelta = Direction * MoveSpeed * DeltaSeconds;
			MoveDelta.Z = FMath::Sin(AgeSeconds * 9.f) * 16.f * DeltaSeconds;
		}
	}

	if (!MoveDelta.IsNearlyZero())
	{
		FHitResult SweepHit;
		SetActorLocation(GetActorLocation() + MoveDelta, true, &SweepHit);
		if (SweepHit.bBlockingHit)
		{
			BounceVelocity = FVector::VectorPlaneProject(-MoveDelta.GetSafeNormal() * BounceSpeed, FVector::UpVector);
			BounceSecondsRemaining = 0.15f;
		}
	}
}

void AT66SummonActor::OnContactBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OwnerCombat || !OtherActor || OtherActor == GetOwner() || OtherActor == this)
	{
		return;
	}

	if (LastHitCooldownSeconds > 0.f && LastHitTarget.Get() == OtherActor)
	{
		return;
	}

	if (OwnerCombat->HandleSummonContact(this, OtherActor, IdolID, Element, Damage, StatusChance, StatusDuration))
	{
		++HitCount;
		LastHitTarget = OtherActor;
		LastHitCooldownSeconds = 0.35f;
		BounceAwayFrom(OtherActor);
	}
}

void AT66SummonActor::RefreshTarget()
{
	CurrentTarget = OwnerCombat ? OwnerCombat->ResolveSummonTargetFromLocation(GetActorLocation(), SearchRange) : nullptr;
}

void AT66SummonActor::BounceAwayFrom(AActor* HitActor)
{
	const FVector Away = HitActor
		? FVector::VectorPlaneProject(GetActorLocation() - HitActor->GetActorLocation(), FVector::UpVector).GetSafeNormal()
		: -GetActorForwardVector();
	BounceVelocity = (Away.IsNearlyZero() ? -GetActorForwardVector() : Away) * BounceSpeed;
	BounceVelocity.Z = 120.f;
	BounceSecondsRemaining = 0.25f;
	RefreshTarget();
}
