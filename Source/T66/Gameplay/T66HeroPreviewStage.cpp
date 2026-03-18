// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66HeroBase.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66PreviewStageEnvironment.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
	constexpr float HeroPreviewLookUpFraction = 0.45f;
	constexpr float MinHeroPreviewRadius = 235.f;
	constexpr float CompanionFrameShiftAlpha = 0.32f;
	constexpr float CompanionFrameRadiusScale = 0.45f;
}

AT66HeroPreviewStage::AT66HeroPreviewStage()
{
	PrimaryActorTick.bCanEverTick = false; // No tick needed — main viewport renders in real-time

	// Root (invisible)
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PreviewPlatform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewPlatform"));
	PreviewPlatform->SetupAttachment(RootComponent);
	PreviewPlatform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		PreviewPlatform->SetStaticMesh(Cylinder);
		PreviewPlatform->SetRelativeScale3D(FVector(4.0f, 4.0f, 0.04f));
		PreviewPlatform->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	}

	// Default to our hero class
	HeroPawnClass = AT66HeroBase::StaticClass();
}

void AT66HeroPreviewStage::BeginPlay()
{
	Super::BeginPlay();

	T66PreviewStageEnvironment::ApplyPreviewGroundMaterial(PreviewPlatform);
	T66PreviewStageEnvironment::CreateEasyFarmPreviewProps(this, RootComponent, EasyPreviewProps);

	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		PreviewDifficulty = GI->SelectedDifficulty;
	}
	RefreshPreviewEnvironment();
}

void AT66HeroPreviewStage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PreviewCompanionPawn)
	{
		PreviewCompanionPawn->Destroy();
		PreviewCompanionPawn = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AT66HeroPreviewStage::SetPreviewHero(FName HeroID, ET66BodyType BodyType, FName SkinID, FName CompanionID)
{
	PreviewYawDegrees = 0.f;
	OrbitPitchDegrees = 15.f;
	PreviewZoomMultiplier = 1.0f;
	UpdatePreviewPawn(HeroID, BodyType, SkinID.IsNone() ? FName(TEXT("Default")) : SkinID);
	ApplyPreviewRotation();
	// Use passed CompanionID or fall back to GI so existing call sites work without change
	FName EffectiveCompanionID = CompanionID;
	if (EffectiveCompanionID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
			EffectiveCompanionID = GI->SelectedCompanionID;
	}
	UpdatePreviewCompanion(EffectiveCompanionID);
	UpdateCompanionPlacement();
	bHasOrbitFrame = false;
	FrameCameraToPreview();
}

void AT66HeroPreviewStage::SetPreviewDifficulty(ET66Difficulty Difficulty)
{
	PreviewDifficulty = Difficulty;
	RefreshPreviewEnvironment();
}

void AT66HeroPreviewStage::AddPreviewYaw(float DeltaYawDegrees)
{
	PreviewYawDegrees = FMath::Fmod(PreviewYawDegrees + DeltaYawDegrees, 360.f);
	ApplyPreviewRotation();
	FrameCameraToPreview();
}

void AT66HeroPreviewStage::AddPreviewZoom(float WheelDelta)
{
	// WheelDelta: positive = wheel up (zoom in), negative = wheel down (zoom out).
	// We clamp max zoom-out to the default framing, so wheel down just returns toward 1.0.
	static constexpr float StepPerWheel = 0.08f;
	const float MinZ = FMath::Clamp(MinPreviewZoomMultiplier, 0.25f, 1.0f);
	PreviewZoomMultiplier = FMath::Clamp(PreviewZoomMultiplier - (WheelDelta * StepPerWheel), MinZ, 1.0f);
	FrameCameraToPreview();
}

void AT66HeroPreviewStage::AddPreviewOrbit(float DeltaYawDegrees, float DeltaPitchDegrees)
{
	// Keep preview as a simple 360 view (no vertical pitch).
	(void)DeltaPitchDegrees;
	AddPreviewYaw(DeltaYawDegrees);
}

UPrimitiveComponent* AT66HeroPreviewStage::GetPreviewTargetComponent() const
{
	if (!PreviewPawn) return nullptr;

	// Prefer visible skeletal mesh, otherwise placeholder.
	if (USkeletalMeshComponent* Skel = PreviewPawn->GetMesh())
	{
		if (Skel->IsVisible() && Skel->GetSkeletalMeshAsset())
		{
			return Skel;
		}
	}
	if (PreviewPawn->PlaceholderMesh && PreviewPawn->PlaceholderMesh->IsVisible())
	{
		return PreviewPawn->PlaceholderMesh;
	}
	return PreviewPawn->GetMesh();
}

void AT66HeroPreviewStage::ApplyPreviewRotation()
{
	if (!PreviewPawn) return;
	// Default faces the camera at 180; add user yaw.
	PreviewPawn->SetActorRotation(FRotator(0.f, 180.f + PreviewYawDegrees, 0.f));
	UpdateCompanionPlacement();
}

void AT66HeroPreviewStage::FrameCameraToPreview()
{
	UPrimitiveComponent* Target = GetPreviewTargetComponent();
	if (!Target || !PreviewPawn) return;

	// Cache orbit framing once per hero selection so the platform/camera don't "swim" as you rotate.
	if (!bHasOrbitFrame)
	{
		const FBoxSphereBounds B = Target->Bounds;
		const FVector HeroLoc = PreviewPawn->GetActorLocation();
		float HeroHalfHeight = 88.f;
		if (const UCapsuleComponent* Cap = PreviewPawn->GetCapsuleComponent())
		{
			HeroHalfHeight = Cap->GetUnscaledCapsuleHalfHeight();
		}

		const float HeroFeetZ = HeroLoc.Z - HeroHalfHeight;
		OrbitCenter = HeroLoc + FVector(0.f, 0.f, HeroHalfHeight * HeroPreviewLookUpFraction);
		OrbitRadius = FMath::Max(MinHeroPreviewRadius, B.SphereRadius);
		OrbitBottomZ = HeroFeetZ;

		// Include companion in frame when present
		if (PreviewCompanionPawn && !PreviewCompanionPawn->IsHidden())
		{
			UPrimitiveComponent* CompPrim = PreviewCompanionPawn->SkeletalMesh && PreviewCompanionPawn->SkeletalMesh->IsVisible()
				? Cast<UPrimitiveComponent>(PreviewCompanionPawn->SkeletalMesh)
				: Cast<UPrimitiveComponent>(PreviewCompanionPawn->PlaceholderMesh);
			if (CompPrim)
			{
				const FBoxSphereBounds CompB = CompPrim->Bounds;
				FVector CompanionShift = PreviewCompanionPawn->GetActorLocation() - HeroLoc;
				CompanionShift.Z = 0.f;
				OrbitCenter += CompanionShift * CompanionFrameShiftAlpha;
				const float PairSpan = FVector::Dist2D(HeroLoc, PreviewCompanionPawn->GetActorLocation());
				OrbitRadius = FMath::Max(OrbitRadius, MinHeroPreviewRadius + (PairSpan * CompanionFrameRadiusScale));
				OrbitRadius = FMath::Max(OrbitRadius, CompB.SphereRadius + (PairSpan * CompanionFrameShiftAlpha));
				OrbitBottomZ = FMath::Min(OrbitBottomZ, CompB.Origin.Z - CompB.BoxExtent.Z);
			}
		}
		bHasOrbitFrame = true;

		UE_LOG(LogTemp, Log, TEXT("[PREVIEW] HeroPreview orbit: Center=(%.1f,%.1f,%.1f) Radius=%.1f BottomZ=%.1f BoundsExtent=(%.1f,%.1f,%.1f)"),
			OrbitCenter.X, OrbitCenter.Y, OrbitCenter.Z, OrbitRadius, OrbitBottomZ,
			B.BoxExtent.X, B.BoxExtent.Y, B.BoxExtent.Z);
	}

	const FVector Center = OrbitCenter;
	const float Radius = OrbitRadius;

	// Fit the whole bounds sphere in view.
	const float HalfFovRad = FMath::DegreesToRadians(CameraFOV * 0.5f);
	const float BaseMult = FMath::Clamp(CameraDistanceMultiplier, 0.60f, 8.0f);
	const float ZoomMult = FMath::Clamp(PreviewZoomMultiplier, FMath::Clamp(MinPreviewZoomMultiplier, 0.25f, 1.0f), 1.0f);
	const float EffectiveMult = FMath::Clamp(BaseMult * ZoomMult, 0.25f, BaseMult);
	const float Dist = (Radius / FMath::Max(0.15f, FMath::Tan(HalfFovRad))) * EffectiveMult;

	const float PitchRad = FMath::DegreesToRadians(OrbitPitchDegrees);
	const float Z = (FMath::Sin(PitchRad) * Dist) + (Radius * 0.12f);
	const float X = (-FMath::Cos(PitchRad) * Dist);
	FVector CamLoc = Center + FVector(X, 0.f, Z);
	// Never allow the camera to dip below the "ground" plane (prevents seeing under the platform).
	CamLoc.Z = FMath::Max(CamLoc.Z, OrbitBottomZ + 60.f);

	const FRotator LookRot = FRotationMatrix::MakeFromX(Center - CamLoc).Rotator();

	// Store computed transform for the FrontendGameMode to apply to the world camera.
	IdealCameraLocation = CamLoc;
	IdealCameraRotation = LookRot;

	if (PreviewPlatform)
	{
		const float GroundZ = GetActorLocation().Z;
		const float PlatformCenterZ = GroundZ - 2.f; // ~4 unit thick disc with scale 0.04
		const float S = FMath::Max(MinGroundScale, FMath::Clamp((Radius / 50.f) * 2.5f, 2.0f, 10.0f));
		float HeroFeetX = Center.X;
		float HeroFeetY = Center.Y;
		if (PreviewPawn)
		{
			const FVector HeroLoc = PreviewPawn->GetActorLocation();
			HeroFeetX = HeroLoc.X;
			HeroFeetY = HeroLoc.Y;
		}
		PreviewPlatform->SetWorldLocation(FVector(HeroFeetX + PlatformForwardOffset, HeroFeetY, PlatformCenterZ));
		PreviewPlatform->SetWorldScale3D(FVector(S, S, 0.04f));
	}
}

void AT66HeroPreviewStage::UpdatePreviewPawn(FName HeroID, ET66BodyType BodyType, FName SkinID)
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) return;

	FHeroData HeroData;
	if (!GI->GetHeroData(HeroID, HeroData))
	{
		if (PreviewPawn)
		{
			PreviewPawn->SetActorHiddenInGame(true);
		}
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	if (!PreviewPawn)
	{
		UClass* ClassToSpawn = HeroPawnClass ? HeroPawnClass.Get() : AT66HeroBase::StaticClass();
		FVector SpawnLoc = GetActorLocation() + GetActorRotation().RotateVector(PreviewPawnOffset);
		FRotator SpawnRot = FRotator(0.f, 0.f, 0.f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		PreviewPawn = World->SpawnActor<AT66HeroBase>(ClassToSpawn, SpawnLoc, SpawnRot, SpawnParams);
		if (PreviewPawn)
		{
			PreviewPawn->SetPreviewMode(true);
			PreviewPawn->SetActorEnableCollision(false);
		}
	}

	if (PreviewPawn)
	{
		PreviewPawn->SetActorHiddenInGame(false);
		PreviewPawn->InitializeHero(HeroData, BodyType, SkinID, true);
		// Freeze the hero so it doesn't fall / move off-camera in the menu level.
		if (UCharacterMovementComponent* Move = PreviewPawn->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->DisableMovement();
			Move->SetMovementMode(MOVE_None);
			Move->GravityScale = 0.f;
		}
		if (UCapsuleComponent* Cap = PreviewPawn->GetCapsuleComponent())
		{
			Cap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		PreviewPawn->SetActorEnableCollision(false);
		// Ground: feet at stage Z so character stands on the platform.
		const float GroundZ = GetActorLocation().Z;
		FVector PawnLoc = GetActorLocation() + GetActorRotation().RotateVector(PreviewPawnOffset);
		if (UCapsuleComponent* Cap = PreviewPawn->GetCapsuleComponent())
			PawnLoc.Z = GroundZ + Cap->GetUnscaledCapsuleHalfHeight();
		else
			PawnLoc.Z = GroundZ;
		PreviewPawn->SetActorLocation(PawnLoc);
		ApplyPreviewRotation();

		// Stream textures; allow alert animation to play.
		if (USkeletalMeshComponent* Skel = PreviewPawn->GetMesh())
		{
			Skel->bForceMipStreaming = true;
			Skel->StreamingDistanceMultiplier = 50.f;
			Skel->SetComponentTickEnabled(true);
			Skel->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
			Skel->bEnableUpdateRateOptimizations = false;
		}
	}
}

void AT66HeroPreviewStage::UpdatePreviewCompanion(FName CompanionID)
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UWorld* World = GetWorld();
	if (!GI || !World) return;

	if (CompanionID.IsNone())
	{
		if (PreviewCompanionPawn)
		{
			PreviewCompanionPawn->Destroy();
			PreviewCompanionPawn = nullptr;
		}
		return;
	}

	FCompanionData CompanionData;
	if (!GI->GetCompanionData(CompanionID, CompanionData))
	{
		if (PreviewCompanionPawn)
		{
			PreviewCompanionPawn->SetActorHiddenInGame(true);
		}
		return;
	}

	FName SkinID = FName(TEXT("Default"));
	if (UT66AchievementsSubsystem* Ach = GI->GetSubsystem<UT66AchievementsSubsystem>())
		SkinID = Ach->GetEquippedCompanionSkinID(CompanionID);
	if (SkinID.IsNone()) SkinID = FName(TEXT("Default"));

	if (!PreviewCompanionPawn)
	{
		const float GroundZ = GetActorLocation().Z;
		FVector SpawnLoc = GetActorLocation() + GetActorRotation().RotateVector(PreviewPawnOffset);
		SpawnLoc.Z = GroundZ; // feet at ground (companion pivot at feet)
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewCompanionPawn = World->SpawnActor<AT66CompanionBase>(AT66CompanionBase::StaticClass(), SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (PreviewCompanionPawn)
		{
			PreviewCompanionPawn->SetPreviewMode(true);
			PreviewCompanionPawn->SetActorEnableCollision(false);
		}
	}

	if (PreviewCompanionPawn)
	{
		PreviewCompanionPawn->SetActorHiddenInGame(false);
		PreviewCompanionPawn->InitializeCompanion(CompanionData, SkinID);
		PreviewCompanionPawn->SetPreviewMode(true);
		if (UT66CompanionUnlockSubsystem* Unlocks = GI->GetSubsystem<UT66CompanionUnlockSubsystem>())
			PreviewCompanionPawn->SetLockedVisual(!Unlocks->IsCompanionUnlocked(CompanionID));
		if (PreviewCompanionPawn->SkeletalMesh)
		{
			PreviewCompanionPawn->SkeletalMesh->bForceMipStreaming = true;
			PreviewCompanionPawn->SkeletalMesh->StreamingDistanceMultiplier = 50.f;
		}
		UpdateCompanionPlacement();
	}
}

void AT66HeroPreviewStage::UpdateCompanionPlacement()
{
	if (!PreviewCompanionPawn || !PreviewPawn) return;
	const FVector HeroLoc = PreviewPawn->GetActorLocation();
	const float HeroYaw = PreviewPawn->GetActorRotation().Yaw;
	const FVector OffsetWorld = FRotator(0.f, HeroYaw, 0.f).RotateVector(CompanionFollowOffset);
	// Companion actor origin = feet (like in-game); hero origin = capsule center. Align companion feet to hero feet so she doesn't float.
	float HeroFeetZ = HeroLoc.Z;
	if (UCapsuleComponent* Cap = PreviewPawn->GetCapsuleComponent())
		HeroFeetZ = HeroLoc.Z - Cap->GetUnscaledCapsuleHalfHeight();
	FVector CompanionLoc = HeroLoc + OffsetWorld;
	CompanionLoc.Z = HeroFeetZ;
	PreviewCompanionPawn->SetActorLocation(CompanionLoc);
	PreviewCompanionPawn->SetActorRotation(FRotator(0.f, HeroYaw, 0.f));
}

void AT66HeroPreviewStage::SetStageVisible(bool bVisible)
{
	bStageVisible = bVisible;
	SetActorHiddenInGame(!bVisible);
	if (PreviewPawn) PreviewPawn->SetActorHiddenInGame(!bVisible);
	if (PreviewCompanionPawn) PreviewCompanionPawn->SetActorHiddenInGame(!bVisible);
	RefreshPreviewEnvironment();
}

void AT66HeroPreviewStage::ResetFramingCache()
{
	bHasOrbitFrame = false;
}

void AT66HeroPreviewStage::RefreshPreviewEnvironment()
{
	if (PreviewPlatform)
	{
		PreviewPlatform->SetVisibility(bStageVisible, true);
		PreviewPlatform->SetHiddenInGame(!bStageVisible, true);
	}

	T66PreviewStageEnvironment::SetPreviewPropsVisibility(
		EasyPreviewProps,
		bStageVisible && T66PreviewStageEnvironment::ShouldShowEasyProps(PreviewDifficulty));
}
