// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/Traps/T66ObstacleTrap.h"

#include "Gameplay/Physics/T66HeroPhysicsComponent.h"
#include "Gameplay/T66CombatDebugDraw.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66VisualUtil.h"
#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66ObstacleTrap, Log, All);

namespace
{
	static FVector FlattenedSafeNormal(const FVector& Vector, const FVector& Fallback)
	{
		FVector Flat = Vector;
		Flat.Z = 0.f;
		if (Flat.Normalize())
		{
			return Flat;
		}
		return Fallback.GetSafeNormal2D().IsNearlyZero() ? FVector::ForwardVector : Fallback.GetSafeNormal2D();
	}
}

AT66ObstacleTrapBase::AT66ObstacleTrapBase()
{
	TrapTypeID = FName(TEXT("ObstacleTrap"));
	TrapFamilyID = FName(TEXT("Obstacle"));
	bDamagesHeroes = false;
	bDamagesEnemies = false;
	TriggerTargetMode = ET66TrapTriggerTarget::HeroesOnly;
	PrimaryActorTick.bCanEverTick = false;
}

void AT66ObstacleTrapBase::BeginPlay()
{
	Super::BeginPlay();

	if (ReactionZone)
	{
		ReactionZone->OnComponentBeginOverlap.AddDynamic(this, &AT66ObstacleTrapBase::OnObstacleOverlap);
		ReactionZone->SetGenerateOverlapEvents(IsTrapEnabled());
	}

	UpdateObstacleVisuals();
}

void AT66ObstacleTrapBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateObstacleVisuals();
}

void AT66ObstacleTrapBase::HandleTrapEnabledChanged()
{
	Super::HandleTrapEnabledChanged();

	if (ReactionZone)
	{
		ReactionZone->SetGenerateOverlapEvents(IsTrapEnabled());
	}
}

void AT66ObstacleTrapBase::ConfigureReactionZone(UShapeComponent* InReactionZone)
{
	ReactionZone = InReactionZone;
	if (!ReactionZone)
	{
		return;
	}

	ReactionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ReactionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	ReactionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ReactionZone->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	ReactionZone->SetGenerateOverlapEvents(true);
	ReactionZone->SetHiddenInGame(true);
}

void AT66ObstacleTrapBase::OnObstacleOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherBodyIndex;
	(void)bFromSweep;

	const FVector HitLocation = SweepResult.ImpactPoint.IsNearlyZero()
		? (OtherComp ? OtherComp->GetComponentLocation() : GetActorLocation())
		: FVector(SweepResult.ImpactPoint);
	TryApplyObstacleReaction(OtherActor, OtherComp, HitLocation);
}

bool AT66ObstacleTrapBase::TryApplyObstacleReaction(AActor* OtherActor, const UPrimitiveComponent* OtherComp, const FVector& HitLocation)
{
	AT66HeroBase* Hero = Cast<AT66HeroBase>(OtherActor);
	if (!Hero || !CanAffectHero(Hero))
	{
		return false;
	}

	UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	if (Now - LastReactionTimeSeconds < FMath::Max(0.f, ReactionCooldownSeconds))
	{
		return false;
	}

	UT66HeroPhysicsComponent* HeroPhysics = Hero->GetHeroPhysicsComponent();
	if (!HeroPhysics)
	{
		return false;
	}

	const FVector LaunchVelocity = BuildLaunchVelocity(Hero, OtherComp, HitLocation);
	const bool bApplied = HeroPhysics->ApplyPhysicsReaction(LaunchVelocity, HitLocation, GetTrapTypeID());
	if (bApplied)
	{
		LastReactionTimeSeconds = Now;
		UE_LOG(
			LogT66ObstacleTrap,
			Log,
			TEXT("[ObstacleTrapReaction] Trap=%s Type=%s Hero=%s Launch=%s Hit=%s Floor=%d"),
			*GetName(),
			*GetTrapTypeID().ToString(),
			*Hero->GetName(),
			*LaunchVelocity.ToCompactString(),
			*HitLocation.ToCompactString(),
			GetTowerFloorNumber());
	}

	return bApplied;
}

FVector AT66ObstacleTrapBase::ResolveRadialLaunchDirection(const AT66HeroBase* Hero) const
{
	return FlattenedSafeNormal(Hero ? Hero->GetActorLocation() - GetActorLocation() : FVector::ZeroVector, GetActorForwardVector());
}

FVector AT66ObstacleTrapBase::BuildLaunchVelocity(const AT66HeroBase* Hero, const UPrimitiveComponent* HitComponent, const FVector& HitLocation) const
{
	(void)HitComponent;
	(void)HitLocation;
	return ResolveRadialLaunchDirection(Hero) * LaunchXY + FVector::UpVector * LaunchZ;
}

void AT66ObstacleTrapBase::ApplyMeshColor(UStaticMeshComponent* Mesh, const FLinearColor& Color) const
{
	if (Mesh)
	{
		FT66VisualUtil::ApplyT66Color(Mesh, const_cast<AT66ObstacleTrapBase*>(this), Color);
	}
}

void AT66ObstacleTrapBase::UpdateObstacleVisuals()
{
}

AT66SweeperArmTrap::AT66SweeperArmTrap()
{
	TrapTypeID = FName(TEXT("ObstacleSweeperArm"));
	TrapFamilyID = FName(TEXT("Obstacle"));
	LaunchXY = 11500.f;
	LaunchZ = 850.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	ArmPivot = CreateDefaultSubobject<USceneComponent>(TEXT("ArmPivot"));
	ArmPivot->SetupAttachment(SceneRoot);

	ArmMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArmMesh"));
	ArmMesh->SetupAttachment(ArmPivot);
	ArmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArmMesh->SetCastShadow(true);

	HubMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HubMesh"));
	HubMesh->SetupAttachment(SceneRoot);
	HubMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HubMesh->SetCastShadow(true);

	UBoxComponent* Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("ReactionZone"));
	Zone->SetupAttachment(ArmPivot);
	ConfigureReactionZone(Zone);
}

void AT66SweeperArmTrap::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(IsTrapEnabled());
}

void AT66SweeperArmTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!IsTrapEnabled() || !ArmPivot)
	{
		return;
	}

	const FRotator Rotation(0.f, ArmPivot->GetRelativeRotation().Yaw + RotationSpeedDegPerSecond * DeltaSeconds, 0.f);
	ArmPivot->SetRelativeRotation(Rotation);
}

void AT66SweeperArmTrap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateObstacleVisuals();
}

FVector AT66SweeperArmTrap::BuildLaunchVelocity(const AT66HeroBase* Hero, const UPrimitiveComponent* HitComponent, const FVector& HitLocation) const
{
	(void)HitComponent;
	(void)HitLocation;
	const FVector Radial = ResolveRadialLaunchDirection(Hero);
	const float RotationSign = RotationSpeedDegPerSecond >= 0.f ? 1.f : -1.f;
	const FVector Tangent = FVector::CrossProduct(FVector::UpVector, Radial).GetSafeNormal() * RotationSign;
	return FlattenedSafeNormal(Tangent, GetActorForwardVector()) * LaunchXY + FVector::UpVector * LaunchZ;
}

void AT66SweeperArmTrap::UpdateObstacleVisuals()
{
	if (ArmPivot)
	{
		ArmPivot->SetRelativeLocation(FVector(0.f, 0.f, ArmHeight));
	}

	if (ArmMesh)
	{
		ArmMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
		ArmMesh->SetRelativeLocation(FVector(ArmLength * 0.5f, 0.f, 0.f));
		ArmMesh->SetRelativeScale3D(FVector(ArmLength / 100.f, ArmThickness / 100.f, ArmThickness / 100.f));
		ApplyMeshColor(ArmMesh, AccentColor);
	}

	if (HubMesh)
	{
		HubMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCylinder());
		HubMesh->SetRelativeLocation(FVector(0.f, 0.f, ArmHeight));
		HubMesh->SetRelativeScale3D(FVector(1.3f, 1.3f, 0.32f));
		ApplyMeshColor(HubMesh, BaseColor);
	}

	if (UBoxComponent* Zone = Cast<UBoxComponent>(ReactionZone))
	{
		Zone->SetRelativeLocation(FVector(ArmLength * 0.5f, 0.f, 0.f));
		Zone->SetBoxExtent(FVector(ArmLength * 0.5f, ArmThickness * 0.72f, ArmThickness * 0.85f));
	}
}

AT66BumperTrap::AT66BumperTrap()
{
	TrapTypeID = FName(TEXT("ObstacleFloorBumper"));
	TrapFamilyID = FName(TEXT("Obstacle"));
	LaunchXY = 8500.f;
	LaunchZ = 1450.f;
	ReactionCooldownSeconds = 0.65f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	BumperMotionRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BumperMotionRoot"));
	BumperMotionRoot->SetupAttachment(SceneRoot);

	BumperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BumperMesh"));
	BumperMesh->SetupAttachment(BumperMotionRoot);
	BumperMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BumperMesh->SetCastShadow(true);

	UBoxComponent* Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("ReactionZone"));
	Zone->SetupAttachment(BumperMotionRoot);
	ConfigureReactionZone(Zone);
}

void AT66BumperTrap::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(IsTrapEnabled());
	UpdateBumperPose(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);
	UE_LOG(
		LogT66ObstacleTrap,
		Display,
		TEXT("[ObstacleBumperMotion] Type=%s Axis=Up Travel=%.1f Period=%.2f Phase=%.2f Location=%s"),
		*TrapTypeID.ToString(),
		TravelDistance,
		CyclePeriodSeconds,
		InitialPhaseSeconds,
		*GetActorLocation().ToCompactString());
}

void AT66BumperTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	(void)DeltaSeconds;
	if (!IsTrapEnabled())
	{
		return;
	}

	UpdateBumperPose(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);
}

void AT66BumperTrap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateObstacleVisuals();
	UpdateBumperPose(InitialPhaseSeconds);
}

void AT66BumperTrap::UpdateBumperPose(const float WorldSeconds)
{
	if (!BumperMotionRoot)
	{
		return;
	}

	const float Period = FMath::Max(0.20f, CyclePeriodSeconds);
	const float Phase = ((WorldSeconds + InitialPhaseSeconds) / Period) * 2.f * PI;
	const float TravelAlpha = 0.5f + 0.5f * FMath::Sin(Phase);
	BumperMotionRoot->SetRelativeLocation(FVector(0.f, 0.f, TravelAlpha * FMath::Max(0.f, TravelDistance)));
}

void AT66BumperTrap::UpdateObstacleVisuals()
{
	if (BumperMesh)
	{
		BumperMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeSphere());
		BumperMesh->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.48f));
		BumperMesh->SetRelativeScale3D(FVector(Radius / 50.f, Radius / 50.f, Height / 100.f));
		ApplyMeshColor(BumperMesh, AccentColor);
	}

	if (UBoxComponent* Zone = Cast<UBoxComponent>(ReactionZone))
	{
		Zone->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.48f));
		Zone->SetBoxExtent(FVector(Radius, Radius, Height * 0.55f));
	}
}

AT66WallBumperTrap::AT66WallBumperTrap()
{
	TrapTypeID = FName(TEXT("ObstacleWallBumper"));
	TrapFamilyID = FName(TEXT("Obstacle"));
	LaunchXY = 10500.f;
	LaunchZ = 650.f;
	ReactionCooldownSeconds = 0.65f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	BumperMotionRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BumperMotionRoot"));
	BumperMotionRoot->SetupAttachment(SceneRoot);

	WallMountMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMountMesh"));
	WallMountMesh->SetupAttachment(SceneRoot);
	WallMountMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WallMountMesh->SetCastShadow(true);

	BumperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BumperMesh"));
	BumperMesh->SetupAttachment(BumperMotionRoot);
	BumperMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BumperMesh->SetCastShadow(true);

	UBoxComponent* Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("ReactionZone"));
	Zone->SetupAttachment(BumperMotionRoot);
	ConfigureReactionZone(Zone);
}

void AT66WallBumperTrap::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(IsTrapEnabled());
	UpdateBumperPose(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);
	UE_LOG(
		LogT66ObstacleTrap,
		Display,
		TEXT("[ObstacleBumperMotion] Type=%s Axis=Forward Travel=%.1f Period=%.2f Phase=%.2f Location=%s"),
		*TrapTypeID.ToString(),
		TravelDistance,
		CyclePeriodSeconds,
		InitialPhaseSeconds,
		*GetActorLocation().ToCompactString());
}

void AT66WallBumperTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	(void)DeltaSeconds;
	if (!IsTrapEnabled())
	{
		return;
	}

	UpdateBumperPose(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);
}

void AT66WallBumperTrap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateObstacleVisuals();
	UpdateBumperPose(InitialPhaseSeconds);
}

FVector AT66WallBumperTrap::BuildLaunchVelocity(const AT66HeroBase* Hero, const UPrimitiveComponent* HitComponent, const FVector& HitLocation) const
{
	(void)Hero;
	(void)HitComponent;
	(void)HitLocation;
	return FlattenedSafeNormal(GetActorForwardVector(), FVector::ForwardVector) * LaunchXY + FVector::UpVector * LaunchZ;
}

void AT66WallBumperTrap::UpdateBumperPose(const float WorldSeconds)
{
	if (!BumperMotionRoot)
	{
		return;
	}

	const float Period = FMath::Max(0.20f, CyclePeriodSeconds);
	const float Phase = ((WorldSeconds + InitialPhaseSeconds) / Period) * 2.f * PI;
	const float TravelAlpha = 0.5f + 0.5f * FMath::Sin(Phase);
	BumperMotionRoot->SetRelativeLocation(FVector(TravelAlpha * FMath::Max(0.f, TravelDistance), 0.f, 0.f));
}

void AT66WallBumperTrap::UpdateObstacleVisuals()
{
	if (WallMountMesh)
	{
		WallMountMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
		WallMountMesh->SetRelativeLocation(FVector(-12.f, 0.f, Height * 0.5f));
		WallMountMesh->SetRelativeScale3D(FVector(0.24f, Width / 100.f, Height / 100.f));
		ApplyMeshColor(WallMountMesh, BaseColor);
	}

	if (BumperMesh)
	{
		BumperMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
		BumperMesh->SetRelativeLocation(FVector(PlateThickness * 0.5f, 0.f, Height * 0.5f));
		BumperMesh->SetRelativeScale3D(FVector(PlateThickness / 100.f, Width / 100.f, Height / 100.f));
		ApplyMeshColor(BumperMesh, AccentColor);
	}

	if (UBoxComponent* Zone = Cast<UBoxComponent>(ReactionZone))
	{
		Zone->SetRelativeLocation(FVector(PlateThickness * 0.5f, 0.f, Height * 0.5f));
		Zone->SetBoxExtent(FVector(PlateThickness * 0.60f, Width * 0.52f, Height * 0.52f));
	}
}

AT66LaunchPadTrap::AT66LaunchPadTrap()
{
	TrapTypeID = FName(TEXT("ObstacleLaunchPad"));
}

AT66CeilingHammerTrap::AT66CeilingHammerTrap()
{
	TrapTypeID = FName(TEXT("ObstacleCeilingHammer"));
	TrapFamilyID = FName(TEXT("Obstacle"));
	LaunchXY = 12000.f;
	LaunchZ = 900.f;
	ReactionCooldownSeconds = 0.70f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	HammerPivot = CreateDefaultSubobject<USceneComponent>(TEXT("HammerPivot"));
	HammerPivot->SetupAttachment(SceneRoot);

	CeilingMountMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CeilingMountMesh"));
	CeilingMountMesh->SetupAttachment(SceneRoot);
	CeilingMountMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CeilingMountMesh->SetCastShadow(true);

	CableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CableMesh"));
	CableMesh->SetupAttachment(HammerPivot);
	CableMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CableMesh->SetCastShadow(true);

	HammerHeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HammerHeadMesh"));
	HammerHeadMesh->SetupAttachment(HammerPivot);
	HammerHeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HammerHeadMesh->SetCastShadow(true);

	UBoxComponent* Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("ReactionZone"));
	Zone->SetupAttachment(HammerPivot);
	ConfigureReactionZone(Zone);
}

void AT66CeilingHammerTrap::BeginPlay()
{
	Super::BeginPlay();
	SetActorTickEnabled(IsTrapEnabled());
	UpdateHammerPose(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);
}

void AT66CeilingHammerTrap::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	(void)DeltaSeconds;
	if (!IsTrapEnabled())
	{
		return;
	}

	UpdateHammerPose(GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f);
}

void AT66CeilingHammerTrap::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateObstacleVisuals();
	UpdateHammerPose(InitialPhaseSeconds);
}

FVector AT66CeilingHammerTrap::BuildLaunchVelocity(const AT66HeroBase* Hero, const UPrimitiveComponent* HitComponent, const FVector& HitLocation) const
{
	(void)Hero;
	(void)HitComponent;
	(void)HitLocation;
	const FVector SwingDirection = FlattenedSafeNormal(GetActorRightVector() * LastSwingVelocitySign, GetActorRightVector());
	return SwingDirection * LaunchXY + FVector::UpVector * LaunchZ;
}

void AT66CeilingHammerTrap::UpdateObstacleVisuals()
{
	if (HammerPivot)
	{
		HammerPivot->SetRelativeLocation(FVector(0.f, 0.f, HangHeight));
	}

	if (CeilingMountMesh)
	{
		CeilingMountMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCylinder());
		CeilingMountMesh->SetRelativeLocation(FVector(0.f, 0.f, HangHeight + 26.f));
		CeilingMountMesh->SetRelativeScale3D(FVector(1.15f, 1.15f, 0.16f));
		ApplyMeshColor(CeilingMountMesh, BaseColor);
	}

	if (CableMesh)
	{
		CableMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
		CableMesh->SetRelativeLocation(FVector(0.f, 0.f, -HammerLength * 0.5f));
		CableMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, HammerLength / 100.f));
		ApplyMeshColor(CableMesh, BaseColor);
	}

	if (HammerHeadMesh)
	{
		HammerHeadMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
		HammerHeadMesh->SetRelativeLocation(FVector(0.f, 0.f, -HammerLength));
		HammerHeadMesh->SetRelativeScale3D(FVector(HammerHeadSize / 100.f, HammerHeadSize * 0.62f / 100.f, HammerHeadSize * 0.72f / 100.f));
		ApplyMeshColor(HammerHeadMesh, AccentColor);
	}

	if (UBoxComponent* Zone = Cast<UBoxComponent>(ReactionZone))
	{
		Zone->SetRelativeLocation(FVector(0.f, 0.f, -HammerLength));
		Zone->SetBoxExtent(FVector(HammerHeadSize * 0.62f, HammerHeadSize * 0.52f, HammerHeadSize * 0.52f));
	}
}

void AT66CeilingHammerTrap::UpdateHammerPose(const float WorldSeconds)
{
	if (!HammerPivot)
	{
		return;
	}

	const float Period = FMath::Max(0.20f, SwingPeriodSeconds);
	const float Phase = ((WorldSeconds + InitialPhaseSeconds) / Period) * 2.f * PI;
	const float Angle = FMath::Sin(Phase) * MaxSwingAngleDegrees;
	LastSwingVelocitySign = FMath::Cos(Phase) >= 0.f ? 1.f : -1.f;
	HammerPivot->SetRelativeRotation(FRotator(0.f, 0.f, Angle));
}
