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
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66ObstacleTrap, Log, All);

namespace
{
	// Fall Guys contract (2026-06-11): obstacle hits launch the capsule directly
	// (no ragdoll, no disable window). The ini LaunchXY/LaunchZ values are legacy
	// ragdoll-impulse magnitudes; these scales map them onto capsule velocities.
	static TAutoConsoleVariable<float> CVarT66ObstacleCapsuleLaunchScaleXY(
		TEXT("t66.Trap.ObstacleCapsuleLaunchScaleXY"),
		0.15f,
		TEXT("Scale mapping tuned obstacle LaunchXY impulse magnitudes onto horizontal capsule launch velocity."),
		ECVF_Default);

	static TAutoConsoleVariable<float> CVarT66ObstacleCapsuleLaunchScaleZ(
		TEXT("t66.Trap.ObstacleCapsuleLaunchScaleZ"),
		1.1f,
		TEXT("Scale mapping tuned obstacle LaunchZ impulse magnitudes onto vertical capsule launch velocity."),
		ECVF_Default);

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

	// ------------------------------------------------------------------
	// InflatableTraps01 kit: authored balloon meshes + pattern material
	// instances of the FriendSlop master. Every mesh keeps the engine
	// basic-shape native envelope (100uu) so the existing dimension-driven
	// component scale math is unchanged; the legacy basic shapes remain the
	// fallback when the kit assets are missing. Caches are rooted on first
	// load — function-local static TObjectPtr is invisible to the GC and
	// dangles after a world-teardown GC otherwise.
	// ------------------------------------------------------------------

	template <typename AssetType>
	static AssetType* T66LoadRootedTrapAsset(const TCHAR* ObjectPath, TObjectPtr<AssetType>& Cached)
	{
		if (!Cached)
		{
			Cached = LoadObject<AssetType>(nullptr, ObjectPath);
			if (Cached)
			{
				Cached->AddToRoot();
			}
		}
		return Cached.Get();
	}

#define T66_INFLATABLE_MESH_GETTER(FuncName, AssetName) \
	static UStaticMesh* FuncName() \
	{ \
		static TObjectPtr<UStaticMesh> Cached = nullptr; \
		return T66LoadRootedTrapAsset<UStaticMesh>( \
			TEXT("/Game/World/Traps/Inflatable/" AssetName "." AssetName), Cached); \
	}

#define T66_INFLATABLE_MATERIAL_GETTER(FuncName, AssetName) \
	static UMaterialInterface* FuncName() \
	{ \
		static TObjectPtr<UMaterialInterface> Cached = nullptr; \
		return T66LoadRootedTrapAsset<UMaterialInterface>( \
			TEXT("/Game/World/Traps/Inflatable/" AssetName "." AssetName), Cached); \
	}

	T66_INFLATABLE_MESH_GETTER(T66GetInflatableSweeperArmMesh, "SM_Inflatable_SweeperArm")
	T66_INFLATABLE_MESH_GETTER(T66GetInflatableHubMesh, "SM_Inflatable_Hub")
	T66_INFLATABLE_MESH_GETTER(T66GetInflatableBumperMesh, "SM_Inflatable_Bumper")
	T66_INFLATABLE_MESH_GETTER(T66GetInflatablePadMesh, "SM_Inflatable_Pad")
	T66_INFLATABLE_MESH_GETTER(T66GetInflatableMalletMesh, "SM_Inflatable_Mallet")
	T66_INFLATABLE_MESH_GETTER(T66GetInflatableTubeMesh, "SM_Inflatable_Tube")

	T66_INFLATABLE_MATERIAL_GETTER(T66GetInflatableStripesMaterial, "MI_Inflatable_StripesDiag")
	T66_INFLATABLE_MATERIAL_GETTER(T66GetInflatableBandsMaterial, "MI_Inflatable_BandsHoriz")
	T66_INFLATABLE_MATERIAL_GETTER(T66GetInflatableChevronsMaterial, "MI_Inflatable_Chevrons")
	T66_INFLATABLE_MATERIAL_GETTER(T66GetInflatableStarsMaterial, "MI_Inflatable_Stars")
	T66_INFLATABLE_MATERIAL_GETTER(T66GetInflatableDotsMaterial, "MI_Inflatable_Dots")

#undef T66_INFLATABLE_MESH_GETTER
#undef T66_INFLATABLE_MATERIAL_GETTER

	/** Assigns the inflatable mesh + pattern; returns false so callers can fall back. */
	static bool T66ApplyInflatableVisual(UStaticMeshComponent* MeshComponent, UStaticMesh* Mesh, UMaterialInterface* PatternMaterial)
	{
		if (!MeshComponent || !Mesh)
		{
			return false;
		}

		MeshComponent->SetStaticMesh(Mesh);
		if (PatternMaterial)
		{
			for (int32 SlotIndex = 0; SlotIndex < MeshComponent->GetNumMaterials(); ++SlotIndex)
			{
				MeshComponent->SetMaterial(SlotIndex, PatternMaterial);
			}
		}
		return true;
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

	// Fall Guys contract (2026-06-11, user-directed): obstacles LAUNCH the hero but
	// never ragdoll or disable him — no knockdown window, no jump-mash recovery
	// prompt. The tuned LaunchXY/LaunchZ values are ragdoll-impulse magnitudes, so
	// they map onto a capsule velocity through the CVar scales below.
	const FVector ImpulseVelocity = BuildLaunchVelocity(Hero, OtherComp, HitLocation);
	const FVector PlanarImpulse(ImpulseVelocity.X, ImpulseVelocity.Y, 0.0f);
	const float PlanarSpeed = FMath::Clamp(
		PlanarImpulse.Size() * FMath::Max(0.01f, CVarT66ObstacleCapsuleLaunchScaleXY.GetValueOnGameThread()),
		600.0f,
		2600.0f);
	const float VerticalSpeed = FMath::Clamp(
		FMath::Max(ImpulseVelocity.Z, 0.0f) * FMath::Max(0.01f, CVarT66ObstacleCapsuleLaunchScaleZ.GetValueOnGameThread()),
		650.0f,
		1600.0f);
	const FVector CapsuleLaunch = PlanarImpulse.GetSafeNormal2D() * PlanarSpeed + FVector::UpVector * VerticalSpeed;
	Hero->LaunchCharacter(CapsuleLaunch, true, true);

	LastReactionTimeSeconds = Now;
	UE_LOG(
		LogT66ObstacleTrap,
		Log,
		TEXT("[ObstacleTrapReaction] Trap=%s Type=%s Hero=%s Launch=%s Hit=%s Floor=%d Mode=CapsuleLaunch"),
		*GetName(),
		*GetTrapTypeID().ToString(),
		*Hero->GetName(),
		*CapsuleLaunch.ToCompactString(),
		*HitLocation.ToCompactString(),
		GetTowerFloorNumber());

	return true;
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
		// Inflatable balloon arm with diagonal stripes (spins into a barber pole);
		// the kit mesh shares the cube's 100uu envelope so the scale math is identical.
		if (!T66ApplyInflatableVisual(ArmMesh, T66GetInflatableSweeperArmMesh(), T66GetInflatableStripesMaterial()))
		{
			ArmMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
			ApplyMeshColor(ArmMesh, AccentColor);
		}
		ArmMesh->SetRelativeLocation(FVector(ArmLength * 0.5f, 0.f, 0.f));
		ArmMesh->SetRelativeScale3D(FVector(ArmLength / 100.f, ArmThickness / 100.f, ArmThickness / 100.f));
	}

	if (HubMesh)
	{
		if (!T66ApplyInflatableVisual(HubMesh, T66GetInflatableHubMesh(), T66GetInflatableBandsMaterial()))
		{
			HubMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCylinder());
			ApplyMeshColor(HubMesh, BaseColor);
		}
		HubMesh->SetRelativeLocation(FVector(0.f, 0.f, ArmHeight));
		HubMesh->SetRelativeScale3D(FVector(1.3f, 1.3f, 0.32f));
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
		// Hazard signature (FALLGUYS_MAP_ANALYSIS.md P9): every surface that PUNTS
		// the hero wears the diagonal stripes, so "stripes = launch" is one learned
		// rule across all obstacles.
		if (!T66ApplyInflatableVisual(BumperMesh, T66GetInflatableBumperMesh(), T66GetInflatableStripesMaterial()))
		{
			BumperMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeSphere());
			ApplyMeshColor(BumperMesh, AccentColor);
		}
		BumperMesh->SetRelativeLocation(FVector(0.f, 0.f, Height * 0.48f));
		BumperMesh->SetRelativeScale3D(FVector(Radius / 50.f, Radius / 50.f, Height / 100.f));
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
		// Hazard signature (P9): the strike cushion wears the diagonal stripes —
		// "stripes = launch" across all obstacles.
		if (!T66ApplyInflatableVisual(BumperMesh, T66GetInflatablePadMesh(), T66GetInflatableStripesMaterial()))
		{
			BumperMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
			ApplyMeshColor(BumperMesh, AccentColor);
		}
		BumperMesh->SetRelativeLocation(FVector(PlateThickness * 0.5f, 0.f, Height * 0.5f));
		BumperMesh->SetRelativeScale3D(FVector(PlateThickness / 100.f, Width / 100.f, Height / 100.f));
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
		if (!T66ApplyInflatableVisual(CeilingMountMesh, T66GetInflatableHubMesh(), T66GetInflatableDotsMaterial()))
		{
			CeilingMountMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCylinder());
			ApplyMeshColor(CeilingMountMesh, BaseColor);
		}
		CeilingMountMesh->SetRelativeLocation(FVector(0.f, 0.f, HangHeight + 26.f));
		CeilingMountMesh->SetRelativeScale3D(FVector(1.15f, 1.15f, 0.16f));
	}

	if (CableMesh)
	{
		// Segmented balloon tube for the hanging cable.
		if (!T66ApplyInflatableVisual(CableMesh, T66GetInflatableTubeMesh(), T66GetInflatableBandsMaterial()))
		{
			CableMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
			ApplyMeshColor(CableMesh, BaseColor);
		}
		CableMesh->SetRelativeLocation(FVector(0.f, 0.f, -HammerLength * 0.5f));
		CableMesh->SetRelativeScale3D(FVector(0.18f, 0.18f, HammerLength / 100.f));
	}

	if (HammerHeadMesh)
	{
		// Two-lobe balloon mallet head.
		// Hazard signature (P9): the mallet head wears the diagonal stripes —
		// "stripes = launch" across all obstacles.
		if (!T66ApplyInflatableVisual(HammerHeadMesh, T66GetInflatableMalletMesh(), T66GetInflatableStripesMaterial()))
		{
			HammerHeadMesh->SetStaticMesh(FT66VisualUtil::GetBasicShapeCube());
			ApplyMeshColor(HammerHeadMesh, AccentColor);
		}
		HammerHeadMesh->SetRelativeLocation(FVector(0.f, 0.f, -HammerLength));
		HammerHeadMesh->SetRelativeScale3D(FVector(HammerHeadSize / 100.f, HammerHeadSize * 0.62f / 100.f, HammerHeadSize * 0.72f / 100.f));
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
