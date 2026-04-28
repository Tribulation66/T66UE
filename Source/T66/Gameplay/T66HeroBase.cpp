// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66EnemyBase.h"
#include "Gameplay/T66PilotableTractor.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Gameplay/T66VisualUtil.h"
#include "Gameplay/T66CombatComponent.h"
#include "Gameplay/Movement/T66HeroMovementComponent.h"
#include "Core/T66CharacterVisualSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/T66HeroCooldownBarWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Engine/EngineTypes.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Hero, Log, All);

namespace
{
	static constexpr float T66HeroCapsuleRadiusUU = 34.0f;
	static constexpr float T66HeroHeightTypeAUU = 200.0f;
	static constexpr float T66HeroHeightTypeBUU = 180.0f;
	static constexpr float T66HeroBaselineVisualHeightUU = 176.0f;
	static constexpr float T66HeroDefaultCameraArmLengthUU = 1440.0f;
	static TAutoConsoleVariable<int32> CVarT66HeroOcclusionRevealEnabled(
		TEXT("T66.Camera.EnableHeroOcclusionReveal"),
		0,
		TEXT("0 disables hero occlusion reveal, 1 outlines/reveals the local hero when fixed-distance camera geometry occludes them."),
		ECVF_Default);
	static const TCHAR* T66HeroOcclusionMaterialPath = TEXT("/Game/Materials/PostProcess/M_HeroOcclusionReveal.M_HeroOcclusionReveal");
	static const FName T66HeroRevealColorParameter(TEXT("RevealColor"));
	static const FName T66HeroRevealOpacityParameter(TEXT("RevealOpacity"));
	static const FName T66HeroRevealDepthBiasParameter(TEXT("DepthBias"));
	static const FName T66HeroRevealDepthScaleParameter(TEXT("DepthScale"));
	static const FName T66HeroRevealStencilValueParameter(TEXT("HeroStencilValue"));
	static constexpr int32 T66HeroOcclusionStencilValue = 17;
	static constexpr float T66HeroOcclusionPostProcessPriority = 6000.0f;
}

AT66HeroBase::AT66HeroBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// ========== Capsule Setup ==========
	GetCapsuleComponent()->SetCapsuleHalfHeight(T66HeroHeightTypeAUU * 0.5f);
	GetCapsuleComponent()->SetCapsuleRadius(T66HeroCapsuleRadiusUU);

	// ========== Camera Setup (Third-Person with Mouse Look) ==========
	
	// Create spring arm (camera boom)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = T66HeroDefaultCameraArmLengthUU;
	CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 60.f)); // Offset up from center
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller (mouse look)
	CameraBoom->bDoCollisionTest = false; // Keep distance stable in dense procedural dungeons.
	CameraBoom->ProbeSize = 12.f;
	CameraBoom->ProbeChannel = ECC_Camera;
	
	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // Camera doesn't rotate independently

	// Hide skeletal mesh until ApplyCharacterVisual applies the correct animation,
	// preventing the default rest pose (face-down) from flashing for a frame.
	if (GetMesh())
	{
		GetMesh()->SetVisibility(false, true);
	}

	// ========== Placeholder Mesh Setup ==========
	
	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	// Align primitive mesh to ground when capsule is grounded:
	// capsule half-height=88, cube/cylinder half-height=50 => relative Z = 50 - 88 = -38.
	PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, -38.f));
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Capsule handles collision

	// Cache mesh assets in constructor
	CacheMeshAssets();
	ApplyBodyTypeDimensions(false);

	// ========== Combat range ring (visual) ==========
	AttackRangeRingISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("AttackRangeRingISM"));
	AttackRangeRingISM->SetupAttachment(RootComponent);
	AttackRangeRingISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackRangeRingISM->SetGenerateOverlapEvents(false);
	AttackRangeRingISM->CastShadow = false;
	AttackRangeRingISM->bReceivesDecals = false;
	AttackRangeRingISM->SetHiddenInGame(true, true);
	AttackRangeRingISM->SetVisibility(false, true);

	// Use simple cube segments to avoid a filled disk that "whites out" the floor.
	if (CubeMesh)
	{
		AttackRangeRingISM->SetStaticMesh(CubeMesh);
	}
	else if (CylinderMesh)
	{
		AttackRangeRingISM->SetStaticMesh(CylinderMesh);
	}

	// Keep the range ring on a stable project material path; editor-only engine debug materials are noisy in commandlets.
	UMaterialInterface* RingMat = FT66VisualUtil::GetFlatColorMaterial();
	if (!RingMat)
	{
		RingMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (RingMat)
	{
		AttackRangeRingISM->SetMaterial(0, RingMat);
	}

	// Sit on the ground plane under the capsule.
	AttackRangeRingISM->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	// Close range ring (10% of attack range) and long range ring (90%) — same mesh/material as outer ring.
	CloseRangeRingISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CloseRangeRingISM"));
	CloseRangeRingISM->SetupAttachment(RootComponent);
	CloseRangeRingISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CloseRangeRingISM->SetGenerateOverlapEvents(false);
	CloseRangeRingISM->CastShadow = false;
	CloseRangeRingISM->bReceivesDecals = false;
	CloseRangeRingISM->SetHiddenInGame(true, true);
	CloseRangeRingISM->SetVisibility(false, true);
	if (CubeMesh) CloseRangeRingISM->SetStaticMesh(CubeMesh);
	else if (CylinderMesh) CloseRangeRingISM->SetStaticMesh(CylinderMesh);
	if (RingMat) CloseRangeRingISM->SetMaterial(0, RingMat);
	CloseRangeRingISM->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	LongRangeRingISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LongRangeRingISM"));
	LongRangeRingISM->SetupAttachment(RootComponent);
	LongRangeRingISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LongRangeRingISM->SetGenerateOverlapEvents(false);
	LongRangeRingISM->CastShadow = false;
	LongRangeRingISM->bReceivesDecals = false;
	LongRangeRingISM->SetHiddenInGame(true, true);
	LongRangeRingISM->SetVisibility(false, true);
	if (CubeMesh) LongRangeRingISM->SetStaticMesh(CubeMesh);
	else if (CylinderMesh) LongRangeRingISM->SetStaticMesh(CylinderMesh);
	if (RingMat) LongRangeRingISM->SetMaterial(0, RingMat);
	LongRangeRingISM->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

	// ========== Auto-attack cooldown bar (below feet) ==========
	CooldownBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CooldownBarWidget"));
	CooldownBarWidgetComponent->SetupAttachment(RootComponent);
	CooldownBarWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	CooldownBarWidgetComponent->SetDrawSize(FVector2D(120.f, 36.f));
	CooldownBarWidgetComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	CooldownBarWidgetComponent->SetWidgetClass(UT66HeroCooldownBarWidget::StaticClass());

	// Don't rotate pawn with controller (controller only rotates camera)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CombatComponent = CreateDefaultSubobject<UT66CombatComponent>(TEXT("CombatComponent"));
	HeroMovementComponent = CreateDefaultSubobject<UT66HeroMovementComponent>(TEXT("HeroMovementComponent"));
	if (HeroMovementComponent)
	{
		HeroMovementComponent->ApplyDefaultMovementConfig();
	}

	// Prepare built-in SkeletalMeshComponent for imported models.
	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Skel->SetVisibility(false, true); // shown only when a character visual mapping exists
	}

	UpdateGroundAttachmentOffsets();
}

float AT66HeroBase::GetDesiredHeroHeightUU() const
{
	return BodyType == ET66BodyType::TypeB ? T66HeroHeightTypeBUU : T66HeroHeightTypeAUU;
}

void AT66HeroBase::UpdateGroundAttachmentOffsets()
{
	const float CapsuleHalfHeight = GetCapsuleComponent()
		? GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()
		: (GetDesiredHeroHeightUU() * 0.5f);

	if (AttackRangeRingISM)
	{
		AttackRangeRingISM->SetRelativeLocation(FVector(0.f, 0.f, -CapsuleHalfHeight + 2.f));
	}
	if (CloseRangeRingISM)
	{
		CloseRangeRingISM->SetRelativeLocation(FVector(0.f, 0.f, -CapsuleHalfHeight + 2.f));
	}
	if (LongRangeRingISM)
	{
		LongRangeRingISM->SetRelativeLocation(FVector(0.f, 0.f, -CapsuleHalfHeight + 2.f));
	}
	if (CooldownBarWidgetComponent)
	{
		CooldownBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, -CapsuleHalfHeight - 24.f));
	}

	QuickReviveDownedVisualOffset.Z = -(GetDesiredHeroHeightUU() * 0.33f);
}

void AT66HeroBase::ApplyCurrentHeroVisualScale()
{
	USkeletalMeshComponent* Skel = GetMesh();
	if (!Skel || !Skel->GetSkeletalMeshAsset())
	{
		return;
	}

	const FVector BaseScale = bHasCharacterVisualBaseScale ? CharacterVisualBaseScale : Skel->GetRelativeScale3D();
	const float HeightRatio = GetDesiredHeroHeightUU() / T66HeroBaselineVisualHeightUU;
	Skel->SetRelativeScale3D(BaseScale * HeightRatio);
}

void AT66HeroBase::ApplyBodyTypeDimensions(const bool bKeepFeetWorldPosition)
{
	const float DesiredHeight = GetDesiredHeroHeightUU();
	const float DesiredHalfHeight = DesiredHeight * 0.5f;
	float PreviousHalfHeight = DesiredHalfHeight;
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		PreviousHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
		Capsule->SetCapsuleSize(T66HeroCapsuleRadiusUU, DesiredHalfHeight, true);
		if (bKeepFeetWorldPosition)
		{
			const float HalfHeightDelta = DesiredHalfHeight - PreviousHalfHeight;
			if (!FMath::IsNearlyZero(HalfHeightDelta))
			{
				AddActorWorldOffset(FVector(0.f, 0.f, HalfHeightDelta), false, nullptr, ETeleportType::TeleportPhysics);
			}
		}
	}

	if (PlaceholderMesh)
	{
		UStaticMesh* TargetMesh = (BodyType == ET66BodyType::TypeB) ? CubeMesh.Get() : CylinderMesh.Get();
		if (TargetMesh)
		{
			PlaceholderMesh->SetStaticMesh(TargetMesh);
		}

		const float VisualScaleZ = DesiredHeight / 100.0f;
		const FVector TargetScale(0.5f, 0.5f, VisualScaleZ);
		const float PlaceholderHalfHeight = 50.0f * VisualScaleZ;
		PlaceholderMesh->SetRelativeScale3D(TargetScale);
		PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, PlaceholderHalfHeight - DesiredHalfHeight));
	}

	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		const float HalfHeightDelta = DesiredHalfHeight - PreviousHalfHeight;
		if (!FMath::IsNearlyZero(HalfHeightDelta))
		{
			FVector RelLoc = Skel->GetRelativeLocation();
			RelLoc.Z -= HalfHeightDelta;
			Skel->SetRelativeLocation(RelLoc);
		}
	}

	UpdateGroundAttachmentOffsets();
	ApplyCurrentHeroVisualScale();
}

void AT66HeroBase::AddSafeZoneOverlap(int32 Delta)
{
	SafeZoneOverlapCount = FMath::Max(0, SafeZoneOverlapCount + Delta);
}

void AT66HeroBase::CacheMeshAssets()
{
	// Cache cylinder mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		CylinderMesh = CylinderFinder.Object;
	}

	// Cache cube mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		CubeMesh = CubeFinder.Object;
	}
}

void AT66HeroBase::BeginPlay()
{
	Super::BeginPlay();

	// Cache baseline friction values so stage effects can temporarily override safely.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		BaseGroundFriction = Movement->GroundFriction;
		BaseBrakingFrictionFactor = Movement->BrakingFrictionFactor;
		BaseBrakingDecelerationWalking = Movement->BrakingDecelerationWalking;
	}

	// Foundation: cosmetic toggles (e.g. Gooner Mode) live in PlayerSettings.
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66PlayerSettingsSubsystem* PS = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			if (PS->GetGoonerMode())
			{
				// TODO: When real character meshes exist, apply barefoot cosmetic set here.
				UE_LOG(LogT66Hero, Log, TEXT("Gooner Mode enabled (placeholder; no meshes to modify yet)."));
			}
		}
	}

	// Create dynamic material for color changes
	if (PlaceholderMesh && PlaceholderMesh->GetMaterial(0))
	{
		PlaceholderMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	// Debug: Log camera status
	UE_LOG(LogT66Hero, Log, TEXT("HeroBase BeginPlay - Location: (%.1f, %.1f, %.1f)"),
		GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
	
	if (FollowCamera)
	{
		UE_LOG(LogT66Hero, Log, TEXT("HeroBase - FollowCamera is active: %s"),
			FollowCamera->IsActive() ? TEXT("YES") : TEXT("NO"));
	}
	else
	{
		UE_LOG(LogT66Hero, Error, TEXT("HeroBase - FollowCamera is NULL!"));
	}

	// Cache run state for derived stats (level + last-stand).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		CachedRunState = GI->GetSubsystem<UT66RunStateSubsystem>();
		if (CachedRunState)
		{
			CachedRunState->HeroProgressChanged.AddDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
			CachedRunState->SurvivalChanged.AddDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
			CachedRunState->InventoryChanged.AddDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
			CachedRunState->PanelVisibilityChanged.AddDynamic(this, &AT66HeroBase::HandleHUDPanelVisibilityChanged);
		}
	}

	HandleHeroDerivedStatsChanged();
	HandleHUDPanelVisibilityChanged();

	if (CooldownBarWidgetComponent)
	{
		CooldownBarWidgetComponent->InitWidget();
	}

	// Ensure ring updates after components (CombatComponent) finish BeginPlay.
	if (UWorld* World = GetWorld())
	{
		FTimerHandle Tmp;
		World->GetTimerManager().SetTimer(Tmp, this, &AT66HeroBase::UpdateAttackRangeRing, 0.15f, false);
	}

	TryApplyLobbyDrivenVisuals();
	UpdateHeroOcclusionRevealSetup();
}

void AT66HeroBase::BeginSkyDrop()
{
	bIsSkyDropping = true;

	FVector Loc = GetActorLocation();
	Loc.Z += SkyDropAltitude;
	SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
}

void AT66HeroBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (bIsSkyDropping)
	{
		bIsSkyDropping = false;

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			EnableInput(PC);
		}

		UE_LOG(LogT66Hero, Log, TEXT("Sky-drop landing at Z=%.1f"), GetActorLocation().Z);
	}
}

void AT66HeroBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	TryApplyLobbyDrivenVisuals();
	UpdateHeroOcclusionRevealSetup();
}

void AT66HeroBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	TryApplyLobbyDrivenVisuals();
	UpdateHeroOcclusionRevealSetup();
}

void AT66HeroBase::OnRep_HeroAppearance()
{
	if (bIsPreviewMode || HeroID.IsNone())
	{
		return;
	}

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	if (!T66GI)
	{
		return;
	}

	FHeroData DesiredHeroData;
	if (!T66GI->GetHeroData(HeroID, DesiredHeroData))
	{
		return;
	}

	const FName DesiredSkinID = HeroSkinID.IsNone() ? FName(TEXT("Default")) : HeroSkinID;
	InitializeHero(DesiredHeroData, BodyType, DesiredSkinID, false);
	bLobbyDrivenVisualsApplied = true;
}

void AT66HeroBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66HeroBase, HeroID);
	DOREPLIFETIME(AT66HeroBase, BodyType);
	DOREPLIFETIME(AT66HeroBase, HeroSkinID);
}

void AT66HeroBase::HandleHeroDerivedStatsChanged()
{
	if (!CachedRunState) return;
	if (HeroMovementComponent)
	{
		HeroMovementComponent->RefreshWalkSpeedFromRunState();
	}

	UpdateAttackRangeRing();
}

void AT66HeroBase::HandleHUDPanelVisibilityChanged()
{
	UpdateAttackRangeRing();
}

void AT66HeroBase::RefreshAttackRangeRing()
{
	UpdateAttackRangeRing();
}

void AT66HeroBase::CacheVehicleVisualDefaults()
{
	if (bVehicleDefaultVisualTransformsCached)
	{
		return;
	}

	if (PlaceholderMesh)
	{
		DefaultPlaceholderRelativeLocation = PlaceholderMesh->GetRelativeLocation();
		DefaultPlaceholderRelativeRotation = PlaceholderMesh->GetRelativeRotation();
	}

	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		DefaultSkeletalMeshRelativeLocation = Skel->GetRelativeLocation();
		DefaultSkeletalMeshRelativeRotation = Skel->GetRelativeRotation();
	}

	bVehicleDefaultVisualTransformsCached = true;
}

void AT66HeroBase::SetVehicleMounted(bool bMounted, AT66PilotableTractor* InMountedTractor, const FVector& VisualOffset, const FRotator& VisualRotation)
{
	CacheVehicleVisualDefaults();
	bVehicleMounted = bMounted;
	MountedTractor = bMounted ? InMountedTractor : nullptr;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (bMounted)
		{
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
			ConsumeMovementInputVector();
		}
		else
		{
			Movement->SetMovementMode(MOVE_Walking);
			if (HeroMovementComponent)
			{
				HeroMovementComponent->RefreshWalkSpeedFromRunState();
			}
		}
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		if (bMounted)
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Capsule->SetCollisionProfileName(TEXT("Pawn"));
		}
	}

	if (PlaceholderMesh)
	{
		PlaceholderMesh->SetRelativeLocation(DefaultPlaceholderRelativeLocation + (bMounted ? VisualOffset : FVector::ZeroVector));
		PlaceholderMesh->SetRelativeRotation(DefaultPlaceholderRelativeRotation + (bMounted ? VisualRotation : FRotator::ZeroRotator));
	}

	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetRelativeLocation(DefaultSkeletalMeshRelativeLocation + (bMounted ? VisualOffset : FVector::ZeroVector));
		Skel->SetRelativeRotation(DefaultSkeletalMeshRelativeRotation + (bMounted ? VisualRotation : FRotator::ZeroRotator));
		Skel->GlobalAnimRateScale = bMounted ? 0.f : 1.f;
		if (bMounted && CachedIdleAnim)
		{
			Skel->PlayAnimation(CachedIdleAnim, true);
		}
	}

	UpdateAttackRangeRing();
}

void AT66HeroBase::SetQuickReviveDowned(bool bDowned)
{
	if (bQuickReviveDowned == bDowned)
	{
		return;
	}

	CacheVehicleVisualDefaults();
	bQuickReviveDowned = bDowned;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		if (bQuickReviveDowned)
		{
			Movement->StopMovementImmediately();
			Movement->DisableMovement();
			ConsumeMovementInputVector();
		}
		else if (!bVehicleMounted)
		{
			Movement->SetMovementMode(MOVE_Walking);
			if (HeroMovementComponent)
			{
				HeroMovementComponent->RefreshWalkSpeedFromRunState();
			}
		}
	}

	if (PlaceholderMesh)
	{
		PlaceholderMesh->SetRelativeLocation(DefaultPlaceholderRelativeLocation + (bQuickReviveDowned ? QuickReviveDownedVisualOffset : FVector::ZeroVector));
		PlaceholderMesh->SetRelativeRotation(DefaultPlaceholderRelativeRotation + (bQuickReviveDowned ? QuickReviveDownedVisualRotation : FRotator::ZeroRotator));
	}

	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetRelativeLocation(DefaultSkeletalMeshRelativeLocation + (bQuickReviveDowned ? QuickReviveDownedVisualOffset : FVector::ZeroVector));
		Skel->SetRelativeRotation(DefaultSkeletalMeshRelativeRotation + (bQuickReviveDowned ? QuickReviveDownedVisualRotation : FRotator::ZeroRotator));
		Skel->GlobalAnimRateScale = (bQuickReviveDowned || bVehicleMounted) ? 0.f : 1.f;
	}

	UpdateAttackRangeRing();
}


void AT66HeroBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FLagScopedScope LagScope(GetWorld(), TEXT("HeroBase::Tick"));

	if (!bIsPreviewMode && !bVehicleMounted && !bQuickReviveDowned)
	{
		if (!bLobbyDrivenVisualsApplied)
		{
			TryApplyLobbyDrivenVisuals();
		}

		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			auto TraceGround = [this](float UpDistance, float DownDistance, FHitResult& OutHit) -> bool
			{
				UWorld* World = GetWorld();
				if (!World)
				{
					return false;
				}

				FCollisionQueryParams Params(SCENE_QUERY_STAT(T66HeroGroundProbe), false, this);
				const FVector ActorLoc = GetActorLocation();
				const FVector TraceStart = ActorLoc + FVector(0.f, 0.f, UpDistance);
				const FVector TraceEnd = ActorLoc - FVector(0.f, 0.f, DownDistance);
				return World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_WorldStatic, Params) ||
					World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, Params);
			};

			const bool bIsFalling = Movement->IsFalling();
			const bool bForceRecoveryTrace = bIsFalling
				&& (GetActorLocation().Z < (LastSafeGroundLocation.Z - 5000.f) || GetActorLocation().Z < -10000.f);
			GroundTraceAccumSeconds += DeltaSeconds;
			const float GroundTraceInterval = bIsFalling ? GroundTraceIntervalFalling : GroundTraceIntervalGrounded;
			const bool bShouldTraceGround = bForceRecoveryTrace || GroundTraceAccumSeconds >= GroundTraceInterval;

			if (!bIsFalling)
			{
				ContinuousFallSeconds = 0.f;

				if (bShouldTraceGround)
				{
					GroundTraceAccumSeconds = 0.f;
					FHitResult GroundHit;
					if (TraceGround(250.f, 600.f, GroundHit))
					{
						const float HalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.f;
						LastSafeGroundLocation = GroundHit.ImpactPoint + FVector(0.f, 0.f, HalfHeight + 5.f);
						LastSafeGroundRotation = GetActorRotation();
						bHasLastSafeGroundTransform = true;
					}
				}
			}
			else
			{
				ContinuousFallSeconds += DeltaSeconds;

				const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
				const bool bFellFarBelowSafeGround = GetActorLocation().Z < (LastSafeGroundLocation.Z - 5000.f);
				const bool bFellOutOfWorld = GetActorLocation().Z < -10000.f;
				if (bShouldTraceGround)
				{
					GroundTraceAccumSeconds = 0.f;
					FHitResult GroundHit;
					const bool bGroundWithinRecoveryRange = TraceGround(100.f, TerrainRecoveryMissingGroundDistance, GroundHit);
					const bool bCanRecover = bHasLastSafeGroundTransform
						&& !bIsSkyDropping
						&& !bGroundWithinRecoveryRange
						&& (Now - LastTerrainRecoveryTime) >= TerrainRecoveryCooldown;
					const bool bNeedsHardRecovery = bFellFarBelowSafeGround || bFellOutOfWorld;
					if (bCanRecover && bNeedsHardRecovery)
					{
						SetActorLocation(LastSafeGroundLocation, false, nullptr, ETeleportType::TeleportPhysics);
						SetActorRotation(LastSafeGroundRotation, ETeleportType::TeleportPhysics);
						Movement->StopMovementImmediately();
						Movement->SetMovementMode(MOVE_Walking);
						ContinuousFallSeconds = 0.f;
						LastTerrainRecoveryTime = Now;
					}
				}
			}
		}
	}

	// Stage slide: temporary low-friction movement.
	if (StageSlideSecondsRemaining > 0.f)
	{
		StageSlideSecondsRemaining = FMath::Max(0.f, StageSlideSecondsRemaining - DeltaSeconds);
		if (StageSlideSecondsRemaining <= 0.f)
		{
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->GroundFriction = BaseGroundFriction;
				Movement->BrakingFrictionFactor = BaseBrakingFrictionFactor;
				Movement->BrakingDecelerationWalking = BaseBrakingDecelerationWalking;
			}
		}
	}

	// Hero animation: idle while still, walk while moving, jump while airborne.
	if (!bIsPreviewMode && !bVehicleMounted && !bQuickReviveDowned)
	{
		const bool bHasMovementInput = HeroMovementComponent
			? HeroMovementComponent->HasMovementInput()
			: (GetLastMovementInputVector().SizeSquared() > 0.01f);
		const FVector ReplicatedVelocity = GetReplicatedMovement().LinearVelocity;
		const bool bHasReplicatedHorizontalVelocity =
			GetVelocity().SizeSquared2D() > FMath::Square(8.f)
			|| ReplicatedVelocity.SizeSquared2D() > FMath::Square(8.f);
		bool bHasRemoteLocationDelta = false;
		if (bHasLastAnimSampleLocation)
		{
			const float AnimTravelDeltaSq = FVector::DistSquared2D(GetActorLocation(), LastAnimSampleLocation);
			bHasRemoteLocationDelta = AnimTravelDeltaSq > FMath::Square(5.f);
		}
		LastAnimSampleLocation = GetActorLocation();
		bHasLastAnimSampleLocation = true;
		if (GetMesh() && GetMesh()->IsVisible() && (CachedIdleAnim || CachedJumpAnim || CachedWalkAnim))
		{
			EMovementAnimState NewState = EMovementAnimState::Idle;
			if (UCharacterMovementComponent* Movement = GetCharacterMovement(); Movement && Movement->IsFalling())
			{
				NewState = EMovementAnimState::Jump;
			}
			else if (bHasMovementInput || bHasReplicatedHorizontalVelocity || (!IsLocallyControlled() && bHasRemoteLocationDelta))
			{
				NewState = EMovementAnimState::Walk;
			}

			if (LastMovementAnimState != NewState)
			{
				LastMovementAnimState = NewState;
				UAnimationAsset* ToPlay = nullptr;
				switch (NewState)
				{
				case EMovementAnimState::Idle:
					ToPlay = CachedIdleAnim ? CachedIdleAnim : CachedWalkAnim;
					break;
				case EMovementAnimState::Jump:
					ToPlay = CachedJumpAnim ? CachedJumpAnim : CachedWalkAnim;
					break;
				case EMovementAnimState::Walk:
				default:
					ToPlay = CachedWalkAnim ? CachedWalkAnim : CachedIdleAnim;
					break;
				}
				if (ToPlay)
				{
					GetMesh()->PlayAnimation(ToPlay, true);
				}
			}
		}
	}

	// Update auto-attack cooldown bar and range text (only when playing, not preview).
	if (!bIsPreviewMode && CooldownBarWidgetComponent && CombatComponent)
	{
		if (UT66HeroCooldownBarWidget* Bar = Cast<UT66HeroCooldownBarWidget>(CooldownBarWidgetComponent->GetWidget()))
		{
			float p = CombatComponent->GetAutoAttackCooldownProgress();
			const float Now = GetWorld() ? static_cast<float>(GetWorld()->GetTimeSeconds()) : 0.f;
			if (p >= 1.f)
			{
				if (CooldownDisplayHoldUntil < 0.f) CooldownDisplayHoldUntil = Now + CooldownBarHoldFullDuration;
				p = 1.f;
			}
			else
			{
				CooldownDisplayHoldUntil = -1.f;
			}
			if (CooldownDisplayHoldUntil > 0.f && Now < CooldownDisplayHoldUntil)
				p = 1.f;
			else if (CooldownDisplayHoldUntil > 0.f && Now >= CooldownDisplayHoldUntil)
				CooldownDisplayHoldUntil = -1.f;
			Bar->SetProgress(p);
			Bar->SetRangeUnits(FMath::RoundToInt(CombatComponent->AttackRange));
		}
	}

	// Enemy touch damage + bounce: proximity check (enemies block so no overlap events; we query range and apply damage + launch).
	if (!bIsPreviewMode && !bVehicleMounted && !bQuickReviveDowned)
	{
		UWorld* World = GetWorld();
		EnemyTouchCheckAccumSeconds += DeltaSeconds;
		UT66RunStateSubsystem* RunState = CachedRunState;
		if (World && RunState)
		{
			if (EnemyTouchCheckAccumSeconds >= EnemyTouchCheckInterval)
			{
				EnemyTouchCheckAccumSeconds = 0.f;
				const float Now = static_cast<float>(World->GetTimeSeconds());
				TArray<FOverlapResult> Overlaps;
				FCollisionQueryParams Params;
				Params.AddIgnoredActor(this);
				World->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(EnemyTouchRadius), Params);
				AT66EnemyBase* ClosestEnemy = nullptr;
				float ClosestDistSq = EnemyTouchRadius * EnemyTouchRadius;
				for (const FOverlapResult& R : Overlaps)
				{
					AActor* A = R.GetActor();
					if (!A) continue;
					AT66EnemyBase* E = Cast<AT66EnemyBase>(A);
					if (!E || E->CurrentHP <= 0) continue;
					const float DistSq = FVector::DistSquared(GetActorLocation(), E->GetActorLocation());
					if (DistSq < ClosestDistSq)
					{
						ClosestDistSq = DistSq;
						ClosestEnemy = E;
					}
				}
				if (ClosestEnemy && !IsInSafeZone())
				{
					if (Now - LastEnemyBounceTime >= EnemyBounceCooldown)
					{
						LastEnemyBounceTime = Now;
						FVector Away = (GetActorLocation() - ClosestEnemy->GetActorLocation()).GetSafeNormal2D();
						Away.Z = 0.f;
						Away.Normalize();
						const FVector BounceVel = Away * EnemyBounceStrength + FVector(0.f, 0.f, EnemyBounceZ);
						if (UCharacterMovementComponent* Move = GetCharacterMovement())
						{
							LaunchCharacter(BounceVel, true, true);
						}
					}
				}
			}
		}
	}
}

bool AT66HeroBase::ShouldEnableHeroOcclusionReveal() const
{
	if (CVarT66HeroOcclusionRevealEnabled.GetValueOnGameThread() == 0)
	{
		return false;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	return !bIsPreviewMode && PlayerController && PlayerController->IsLocalController();
}

void AT66HeroBase::ConfigureHeroOcclusionComponent(UPrimitiveComponent* Component, const bool bEnable) const
{
	if (!Component)
	{
		return;
	}

	Component->SetRenderCustomDepth(bEnable);
	if (bEnable)
	{
		Component->SetCustomDepthStencilValue(T66HeroOcclusionStencilValue);
	}
}

void AT66HeroBase::DestroyHeroOcclusionRevealVolume()
{
	if (HeroOcclusionRevealVolume)
	{
		HeroOcclusionRevealVolume->Destroy();
		HeroOcclusionRevealVolume = nullptr;
	}

	HeroOcclusionRevealMaterial = nullptr;
}

void AT66HeroBase::UpdateHeroOcclusionRevealSetup()
{
	const bool bEnableReveal = ShouldEnableHeroOcclusionReveal();
	ConfigureHeroOcclusionComponent(GetMesh(), bEnableReveal);
	ConfigureHeroOcclusionComponent(PlaceholderMesh, bEnableReveal);

	if (!bEnableReveal)
	{
		DestroyHeroOcclusionRevealVolume();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		DestroyHeroOcclusionRevealVolume();
		return;
	}

	if (HeroOcclusionRevealVolume && HeroOcclusionRevealVolume->GetWorld() != World)
	{
		DestroyHeroOcclusionRevealVolume();
	}

	if (!HeroOcclusionRevealVolume)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;
		HeroOcclusionRevealVolume = World->SpawnActor<APostProcessVolume>(
			APostProcessVolume::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams);
		if (HeroOcclusionRevealVolume)
		{
			HeroOcclusionRevealVolume->bUnbound = true;
			HeroOcclusionRevealVolume->BlendWeight = 1.0f;
			HeroOcclusionRevealVolume->Priority = T66HeroOcclusionPostProcessPriority;
			HeroOcclusionRevealVolume->Settings.WeightedBlendables.Array.Reset();
#if WITH_EDITOR
			HeroOcclusionRevealVolume->SetActorLabel(TEXT("DEV_HeroOcclusionReveal_PostProcessVolume"));
#endif
		}
	}

	if (!HeroOcclusionRevealVolume)
	{
		return;
	}

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, T66HeroOcclusionMaterialPath);
	if (!BaseMaterial)
	{
		UE_LOG(LogT66Hero, Warning, TEXT("Hero occlusion reveal material missing at %s"), T66HeroOcclusionMaterialPath);
		return;
	}

	if (!HeroOcclusionRevealMaterial)
	{
		HeroOcclusionRevealMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	}

	if (!HeroOcclusionRevealMaterial)
	{
		return;
	}

	HeroOcclusionRevealMaterial->SetVectorParameterValue(T66HeroRevealColorParameter, FLinearColor(0.97f, 0.80f, 0.24f, 1.0f));
	HeroOcclusionRevealMaterial->SetScalarParameterValue(T66HeroRevealOpacityParameter, 0.92f);
	HeroOcclusionRevealMaterial->SetScalarParameterValue(T66HeroRevealDepthBiasParameter, 2.0f);
	HeroOcclusionRevealMaterial->SetScalarParameterValue(T66HeroRevealDepthScaleParameter, 0.08f);
	HeroOcclusionRevealMaterial->SetScalarParameterValue(T66HeroRevealStencilValueParameter, static_cast<float>(T66HeroOcclusionStencilValue));

	FPostProcessSettings& PostProcessSettings = HeroOcclusionRevealVolume->Settings;
	for (FWeightedBlendable& Blendable : PostProcessSettings.WeightedBlendables.Array)
	{
		if (Blendable.Object == HeroOcclusionRevealMaterial)
		{
			Blendable.Weight = 1.0f;
			return;
		}
	}

	PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, HeroOcclusionRevealMaterial));
}

bool AT66HeroBase::TryGetLobbyDrivenVisualParams(FHeroData& OutHeroData, ET66BodyType& OutBodyType, FName& OutSkinID) const
{
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance());
	const AT66SessionPlayerState* SessionPlayerState = GetPlayerState<AT66SessionPlayerState>();
	if (!T66GI || !SessionPlayerState)
	{
		return false;
	}

	const FName SelectedHeroID = SessionPlayerState->GetSelectedHeroID();
	if (SelectedHeroID.IsNone())
	{
		return false;
	}

	if (!T66GI->GetHeroData(SelectedHeroID, OutHeroData))
	{
		return false;
	}

	OutBodyType = SessionPlayerState->GetSelectedHeroBodyType();
	OutSkinID = SessionPlayerState->GetSelectedHeroSkinID().IsNone()
		? FName(TEXT("Default"))
		: SessionPlayerState->GetSelectedHeroSkinID();
	return true;
}

void AT66HeroBase::TryApplyLobbyDrivenVisuals()
{
	if (bIsPreviewMode)
	{
		return;
	}

	FHeroData DesiredHeroData;
	ET66BodyType DesiredBodyType = ET66BodyType::TypeA;
	FName DesiredSkinID = FName(TEXT("Default"));
	if (!TryGetLobbyDrivenVisualParams(DesiredHeroData, DesiredBodyType, DesiredSkinID))
	{
		return;
	}

	const bool bMeshVisible = GetMesh() && GetMesh()->IsVisible();
	const bool bAnimationsCached = CachedIdleAnim || CachedWalkAnim || CachedJumpAnim;
	const bool bNeedsRefresh =
		!bLobbyDrivenVisualsApplied
		|| HeroID != DesiredHeroData.HeroID
		|| BodyType != DesiredBodyType
		|| !bMeshVisible
		|| !bAnimationsCached;

	if (!bNeedsRefresh)
	{
		return;
	}

	InitializeHero(DesiredHeroData, DesiredBodyType, DesiredSkinID, false);
	bLobbyDrivenVisualsApplied = true;
}

void AT66HeroBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyHeroOcclusionRevealVolume();

	if (CachedRunState)
	{
		CachedRunState->HeroProgressChanged.RemoveDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->SurvivalChanged.RemoveDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->InventoryChanged.RemoveDynamic(this, &AT66HeroBase::HandleHeroDerivedStatsChanged);
		CachedRunState->PanelVisibilityChanged.RemoveDynamic(this, &AT66HeroBase::HandleHUDPanelVisibilityChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void AT66HeroBase::UpdateAttackRangeRing()
{
	if (!AttackRangeRingISM)
	{
		return;
	}

	auto HideAllRings = [this]()
	{
		AttackRangeRingISM->SetHiddenInGame(true, true);
		AttackRangeRingISM->SetVisibility(false, true);
		if (CloseRangeRingISM) { CloseRangeRingISM->SetHiddenInGame(true, true); CloseRangeRingISM->SetVisibility(false, true); }
		if (LongRangeRingISM)  { LongRangeRingISM->SetHiddenInGame(true, true);  LongRangeRingISM->SetVisibility(false, true); }
		if (CooldownBarWidgetComponent) CooldownBarWidgetComponent->SetVisibility(false);
	};

	// Hide in preview mode.
	if (bIsPreviewMode || bVehicleMounted || bQuickReviveDowned)
	{
		HideAllRings();
		return;
	}

	const bool bPanelsVisible = CachedRunState ? CachedRunState->GetHUDPanelsVisible() : true;
	if (!bPanelsVisible)
	{
		HideAllRings();
		return;
	}

	// Cooldown bar is now in the HUD (above hearts); never show the world-space one.
	if (CooldownBarWidgetComponent) CooldownBarWidgetComponent->SetVisibility(false);

	const float Range = (CombatComponent ? CombatComponent->AttackRange : 0.f);
	const float ClampedRange = FMath::Clamp(Range, 200.f, 50000.f);
	const float CloseRadius = ClampedRange * 0.10f;  // close range damage zone (0–10% of range)
	const float LongRadius  = ClampedRange * 0.90f;  // long range damage zone (90–100% of range)

	// Helper: build a segmented ring into an ISM at the given radius (same visual style as outer ring).
	auto BuildRing = [this](UInstancedStaticMeshComponent* ISM, float Radius)
	{
		if (!ISM || Radius < 10.f) return;
		ISM->ClearInstances();
		static constexpr int32 NumSeg = 192;
		const float Circ = 2.f * PI * Radius;
		const float SegLen = FMath::Clamp((Circ / static_cast<float>(NumSeg)) * 1.10f, 4.f, 80.f);
		const float SegThick = 8.f;
		const float SegHeight = 2.5f;
		for (int32 i = 0; i < NumSeg; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(NumSeg);
			const float A = 2.f * PI * T;
			const float X = FMath::Cos(A) * Radius;
			const float Y = FMath::Sin(A) * Radius;
			const float YawDeg = FMath::RadiansToDegrees(A) + 90.f;
			const FVector Loc(X, Y, 3.f);
			const FRotator Rot(0.f, YawDeg, 0.f);
			const FVector Scale(SegLen / 100.f, SegThick / 100.f, SegHeight / 100.f);
			ISM->AddInstance(FTransform(Rot, Loc, Scale), false);
		}
		ISM->MarkRenderStateDirty();
	};

	BuildRing(AttackRangeRingISM, ClampedRange);
	BuildRing(CloseRangeRingISM, CloseRadius);
	BuildRing(LongRangeRingISM, LongRadius);

	AttackRangeRingISM->SetHiddenInGame(false, true);
	AttackRangeRingISM->SetVisibility(true, true);
	if (CloseRangeRingISM) { CloseRangeRingISM->SetHiddenInGame(false, true); CloseRangeRingISM->SetVisibility(true, true); }
	if (LongRangeRingISM)  { LongRangeRingISM->SetHiddenInGame(false, true);  LongRangeRingISM->SetVisibility(true, true); }
}

void AT66HeroBase::ApplyStageSlide(float DurationSeconds)
{
	if (DurationSeconds <= 0.f) return;
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		// Very low braking so the hero "slides" along the ground.
		Movement->GroundFriction = 0.15f;
		Movement->BrakingFrictionFactor = 0.05f;
		Movement->BrakingDecelerationWalking = 128.f;
	}
	StageSlideSecondsRemaining = FMath::Max(StageSlideSecondsRemaining, DurationSeconds);
}

void AT66HeroBase::InitializeHero(const FHeroData& InHeroData, ET66BodyType InBodyType, FName InSkinID, bool bPreviewMode)
{
	HeroID = InHeroData.HeroID;
	HeroData = InHeroData;
	BodyType = InBodyType;

	// Set the body type mesh (Cylinder or Cube)
	SetBodyType(InBodyType);

	// Apply the placeholder color from hero data
	SetPlaceholderColor(InHeroData.PlaceholderColor);

	FName SkinID = InSkinID.IsNone() ? FName(TEXT("Default")) : InSkinID;
	HeroSkinID = SkinID;
	bLobbyDrivenVisualsApplied = !bPreviewMode;

	UE_LOG(LogT66Hero, Log, TEXT("Hero initialized: %s, BodyType: %s, Skin: %s, Color: (%.2f, %.2f, %.2f)"),
		*InHeroData.DisplayName.ToString(),
		InBodyType == ET66BodyType::TypeA ? TEXT("TypeA") : TEXT("TypeB"),
		*SkinID.ToString(),
		InHeroData.PlaceholderColor.R,
		InHeroData.PlaceholderColor.G,
		InHeroData.PlaceholderColor.B);

	// Resolve hero visual by HeroID + BodyType + SkinID (e.g. Hero_1_TypeA, Hero_1_TypeB_Beachgoer).
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UT66CharacterVisualSubsystem* Visuals = GI->GetSubsystem<UT66CharacterVisualSubsystem>())
		{
			const FName VisualID = UT66CharacterVisualSubsystem::GetHeroVisualID(HeroID, InBodyType, SkinID);
			// Hero selection preview uses the idle animation so the character is non-static in the showcase.
			const bool bUseIdleAnimation = bPreviewMode;
			UE_LOG(LogT66Hero, Log, TEXT("[ANIM] HeroBase::InitializeHero HeroID=%s BodyType=%s SkinID=%s bPreviewMode=%d bUseIdleAnimation=%d VisualID=%s"),
				*HeroID.ToString(), InBodyType == ET66BodyType::TypeA ? TEXT("A") : TEXT("B"), *SkinID.ToString(),
				bPreviewMode ? 1 : 0, bUseIdleAnimation ? 1 : 0, *VisualID.ToString());
			const bool bApplied = Visuals->ApplyCharacterVisual(VisualID, GetMesh(), PlaceholderMesh, true, bUseIdleAnimation, bPreviewMode);
			if (!bApplied)
			{
				bHasCharacterVisualBaseScale = false;
				CharacterVisualBaseScale = FVector::OneVector;
				if (GetMesh())
				{
					GetMesh()->SetVisibility(false, true);
				}
				if (PlaceholderMesh)
				{
					PlaceholderMesh->SetVisibility(true, true);
				}
			}
			else
			{
				if (GetMesh())
				{
					CharacterVisualBaseScale = GetMesh()->GetRelativeScale3D();
					bHasCharacterVisualBaseScale = true;
					ApplyCurrentHeroVisualScale();
					GetMesh()->SetVisibility(true, true);
				}
			}
			UpdateHeroOcclusionRevealSetup();
			if (bApplied && !bPreviewMode)
			{
				// Cache idle/walk/jump anims and init hero speed params.
				UAnimationAsset* WalkRaw = nullptr;
				UAnimationAsset* JumpRaw = nullptr;
				UAnimationAsset* IdleRaw = nullptr;
				Visuals->GetMovementAnimsForVisual(VisualID, WalkRaw, JumpRaw, IdleRaw);
				CachedWalkAnim = WalkRaw;
				CachedJumpAnim = JumpRaw;
				CachedIdleAnim = IdleRaw;
				// Arthur (Hero_1): no jump animation -- walk anim plays while airborne.
				if (InHeroData.HeroID == FName(TEXT("Hero_1")))
				{
					CachedJumpAnim = nullptr;
				}
				// Force first Tick to play idle (speed 0); if we left Idle we wouldn't call PlayAnimation.
				LastMovementAnimState = EMovementAnimState::Walk;
				if (HeroMovementComponent)
				{
					HeroMovementComponent->SetHeroBaseWalkSpeed(InHeroData.MaxSpeed * 2.0f);
				}
			}
		}
	}
}

void AT66HeroBase::SetPlaceholderColor(FLinearColor Color)
{
	if (!PlaceholderMesh) return;

	CurrentPlaceholderColor = Color;

	if (PlaceholderMaterial)
	{
		FT66VisualUtil::ConfigureFlatColorMaterial(PlaceholderMaterial, Color);
		return;
	}

	UMaterialInterface* ColorMaterial = FT66VisualUtil::GetFlatColorMaterial();
	if (!ColorMaterial)
	{
		ColorMaterial = PlaceholderMesh->GetMaterial(0);
	}

	if (ColorMaterial)
	{
		PlaceholderMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
		if (PlaceholderMaterial)
		{
			PlaceholderMesh->SetMaterial(0, PlaceholderMaterial);
			FT66VisualUtil::ConfigureFlatColorMaterial(PlaceholderMaterial, Color);
		}
	}

	if (PlaceholderMesh)
	{
		PlaceholderMesh->SetDefaultCustomPrimitiveDataFloat(0, Color.R);
		PlaceholderMesh->SetDefaultCustomPrimitiveDataFloat(1, Color.G);
		PlaceholderMesh->SetDefaultCustomPrimitiveDataFloat(2, Color.B);
	}
}

void AT66HeroBase::SetBodyType(ET66BodyType NewBodyType)
{
	BodyType = NewBodyType;

	if (!PlaceholderMesh) return;

	ApplyBodyTypeDimensions(true);
	PlaceholderMaterial = nullptr;
	if (!HeroData.HeroID.IsNone())
	{
		SetPlaceholderColor(HeroData.PlaceholderColor);
	}

	UE_LOG(LogT66Hero, Log, TEXT("Hero body type set to: %s"),
		NewBodyType == ET66BodyType::TypeA ? TEXT("TypeA (Cylinder)") : TEXT("TypeB (Cube)"));
}

void AT66HeroBase::SetPreviewMode(bool bPreview)
{
	bIsPreviewMode = bPreview;

	if (bPreview)
	{
		// Disable camera in preview mode
		if (FollowCamera)
		{
			FollowCamera->SetActive(false);
		}
		
		// Disable movement
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->DisableMovement();
		}
	}
	else
	{
		// Enable camera
		if (FollowCamera)
		{
			FollowCamera->SetActive(true);
		}
		
		// Enable movement
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}

	UpdateHeroOcclusionRevealSetup();
}

void AT66HeroBase::DashForward()
{
	if (HeroMovementComponent)
	{
		HeroMovementComponent->TryDashInWorldDirection(GetActorForwardVector());
	}
}
