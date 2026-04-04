// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66CompanionBase.h"
#include "Gameplay/T66PreviewStageEnvironment.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Gameplay/T66VisualUtil.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66CompanionPreview, Log, All);

AT66CompanionPreviewStage::AT66CompanionPreviewStage()
{
	PrimaryActorTick.bCanEverTick = false; // No tick needed — main viewport renders in real-time

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Simple floor plane for grounding reference.
	PreviewFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewFloor"));
	PreviewFloor->SetupAttachment(RootComponent);
	PreviewFloor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = FT66VisualUtil::GetBasicShapeCylinder())
	{
		PreviewFloor->SetStaticMesh(Cylinder);
		PreviewFloor->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
		PreviewFloor->SetRelativeScale3D(FVector(2.2f, 2.2f, 0.08f));
	}

	CompanionPawnClass = AT66CompanionBase::StaticClass();
}

void AT66CompanionPreviewStage::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		PreviewDifficulty = GI->SelectedDifficulty;
	}

	T66PreviewStageEnvironment::ApplyPreviewGroundMaterial(PreviewFloor, PreviewDifficulty);
}

void AT66CompanionPreviewStage::BeginPlay()
{
	Super::BeginPlay();

	T66PreviewStageEnvironment::CreateEasyFarmPreviewProps(this, RootComponent, EasyPreviewProps);

	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		PreviewDifficulty = GI->SelectedDifficulty;
	}
	T66PreviewStageEnvironment::ApplyPreviewGroundMaterial(PreviewFloor, PreviewDifficulty);
	RefreshPreviewEnvironment();
}

void AT66CompanionPreviewStage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ActivePreviewCompanionID = NAME_None;
	ActivePreviewSkinID = NAME_None;
	Super::EndPlay(EndPlayReason);
}

void AT66CompanionPreviewStage::SetPreviewCompanion(FName CompanionID, FName SkinID)
{
	const FName EffectiveSkin = SkinID.IsNone() ? FName(TEXT("Default")) : SkinID;
	const bool bPreviewChanged = (CompanionID != ActivePreviewCompanionID) || (EffectiveSkin != ActivePreviewSkinID);
	if (bPreviewChanged)
	{
		PreviewYawDegrees = 0.f;
		OrbitPitchDegrees = 15.f;
		PreviewZoomMultiplier = 1.0f;
		UpdatePreviewPawn(CompanionID, EffectiveSkin);
		bHasOrbitFrame = false;
	}
	FrameCameraToPreview();
}

void AT66CompanionPreviewStage::SetPreviewDifficulty(ET66Difficulty Difficulty)
{
	PreviewDifficulty = Difficulty;
	T66PreviewStageEnvironment::ApplyPreviewGroundMaterial(PreviewFloor, PreviewDifficulty);
	RefreshPreviewEnvironment();
}

void AT66CompanionPreviewStage::AddPreviewYaw(float DeltaYawDegrees)
{
	PreviewYawDegrees = FMath::Fmod(PreviewYawDegrees + DeltaYawDegrees, 360.f);
	ApplyPreviewRotation();
	FrameCameraToPreview();
}

void AT66CompanionPreviewStage::AddPreviewZoom(float WheelDelta)
{
	static constexpr float StepPerWheel = 0.08f;
	const float MinZ = FMath::Clamp(MinPreviewZoomMultiplier, 0.25f, 1.0f);
	PreviewZoomMultiplier = FMath::Clamp(PreviewZoomMultiplier - (WheelDelta * StepPerWheel), MinZ, 1.0f);
	FrameCameraToPreview();
}

void AT66CompanionPreviewStage::AddPreviewOrbit(float DeltaYawDegrees, float DeltaPitchDegrees)
{
	// Keep preview as a simple 360 view (no vertical pitch).
	(void)DeltaPitchDegrees;
	AddPreviewYaw(DeltaYawDegrees);
}

UPrimitiveComponent* AT66CompanionPreviewStage::GetPreviewTargetComponent() const
{
	if (!PreviewPawn) return nullptr;
	// Prefer visible skeletal mesh, otherwise placeholder.
	if (PreviewPawn->SkeletalMesh && PreviewPawn->SkeletalMesh->IsVisible() && PreviewPawn->SkeletalMesh->GetSkeletalMeshAsset())
	{
		return PreviewPawn->SkeletalMesh;
	}
	if (PreviewPawn->PlaceholderMesh && PreviewPawn->PlaceholderMesh->IsVisible())
	{
		return PreviewPawn->PlaceholderMesh;
	}
	return PreviewPawn->SkeletalMesh ? static_cast<UPrimitiveComponent*>(PreviewPawn->SkeletalMesh) : static_cast<UPrimitiveComponent*>(PreviewPawn->PlaceholderMesh);
}

void AT66CompanionPreviewStage::ApplyPreviewRotation()
{
	if (!PreviewPawn) return;
	PreviewPawn->SetActorRotation(FRotator(0.f, 180.f + PreviewYawDegrees, 0.f));
}

void AT66CompanionPreviewStage::FrameCameraToPreview()
{
	UPrimitiveComponent* Target = GetPreviewTargetComponent();

	if (!Target)
	{
		const FVector StageOrigin = GetActorLocation();
		const float GroundZ = StageOrigin.Z;
		const float EmptyPreviewRadius = 900.f;
		const FVector EmptyCenter = StageOrigin + FVector(250.f, 0.f, 120.f);

		OrbitCenter = EmptyCenter;
		OrbitRadius = EmptyPreviewRadius;
		OrbitBottomZ = GroundZ;
		bHasOrbitFrame = true;
	}
	else if (!bHasOrbitFrame)
	{
		const FBoxSphereBounds B = Target->Bounds;
		OrbitCenter = B.Origin;
		OrbitRadius = FMath::Max(25.f, B.SphereRadius);
		OrbitBottomZ = (B.Origin.Z - B.BoxExtent.Z);
		bHasOrbitFrame = true;

		UE_LOG(LogT66CompanionPreview, Verbose, TEXT("[PREVIEW] CompanionPreview orbit: Center=(%.1f,%.1f,%.1f) Radius=%.1f BottomZ=%.1f BoundsExtent=(%.1f,%.1f,%.1f)"),
			OrbitCenter.X, OrbitCenter.Y, OrbitCenter.Z, OrbitRadius, OrbitBottomZ,
			B.BoxExtent.X, B.BoxExtent.Y, B.BoxExtent.Z);
	}

	const FVector Center = OrbitCenter;
	const float Radius = OrbitRadius;

	const float HalfFovRad = FMath::DegreesToRadians(CameraFOV * 0.5f);
	const float BaseMult = FMath::Clamp(CameraDistanceMultiplier, 0.60f, 8.0f);
	const float ZoomMult = FMath::Clamp(PreviewZoomMultiplier, FMath::Clamp(MinPreviewZoomMultiplier, 0.25f, 1.0f), 1.0f);
	const float EffectiveMult = FMath::Clamp(BaseMult * ZoomMult, 0.25f, BaseMult);
	const float Dist = (Radius / FMath::Max(0.15f, FMath::Tan(HalfFovRad))) * EffectiveMult;

	const float PitchRad = FMath::DegreesToRadians(OrbitPitchDegrees);
	const float Z = (FMath::Sin(PitchRad) * Dist) + (Radius * 0.12f);
	const float X = (-FMath::Cos(PitchRad) * Dist);
	FVector CamLoc = Center + FVector(X, 0.f, Z);
	CamLoc.Z = FMath::Max(CamLoc.Z, OrbitBottomZ + 60.f);

	const FRotator LookRot = FRotationMatrix::MakeFromX(Center - CamLoc).Rotator();

	// Store computed transform for the FrontendGameMode to apply to the world camera.
	IdealCameraLocation = CamLoc;
	IdealCameraRotation = LookRot;

	if (PreviewFloor)
	{
		// Floor top at stage ground Z so character feet align with it.
		const float GroundZ = GetActorLocation().Z;
		const float FloorCenterZ = GroundZ - 2.f; // ~4 unit thick disc with scale 0.04
		FVector FloorAnchor = FVector(Center.X, Center.Y, GroundZ);
		if (PreviewPawn)
		{
			FloorAnchor = PreviewPawn->GetActorLocation();
		}
		PreviewFloor->SetWorldLocation(FVector(FloorAnchor.X + FloorForwardOffset, FloorAnchor.Y, FloorCenterZ));
		const float S = FMath::Max(MinGroundScale, FMath::Clamp((Radius / 50.f) * 2.5f, 2.0f, 10.0f));
		PreviewFloor->SetWorldScale3D(FVector(S, S, 0.04f));
	}
}

void AT66CompanionPreviewStage::UpdatePreviewPawn(FName CompanionID, FName SkinID)
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) return;

	if (CompanionID.IsNone())
	{
		if (PreviewPawn) PreviewPawn->SetActorHiddenInGame(true);
		ActivePreviewCompanionID = NAME_None;
		ActivePreviewSkinID = NAME_None;
		return;
	}

	FCompanionData CompanionData;
	if (!GI->GetCompanionData(CompanionID, CompanionData))
	{
		if (PreviewPawn) PreviewPawn->SetActorHiddenInGame(true);
		ActivePreviewCompanionID = NAME_None;
		ActivePreviewSkinID = NAME_None;
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	if (!PreviewPawn)
	{
		UClass* ClassToSpawn = CompanionPawnClass ? CompanionPawnClass.Get() : AT66CompanionBase::StaticClass();
		FVector SpawnLoc = GetActorLocation() + GetActorRotation().RotateVector(PreviewPawnOffset);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewPawn = World->SpawnActor<AT66CompanionBase>(ClassToSpawn, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
		if (PreviewPawn)
		{
			PreviewPawn->SetPreviewMode(true);
			PreviewPawn->SetActorEnableCollision(false);
		}
	}

	const FName EffectiveSkin = SkinID.IsNone() ? FName(TEXT("Default")) : SkinID;
	if (PreviewPawn)
	{
		PreviewPawn->SetActorHiddenInGame(false);
		PreviewPawn->InitializeCompanion(CompanionData, EffectiveSkin);
		PreviewPawn->SetPreviewMode(true);

		// Locked companions should preview as a black silhouette (no real model).
		if (UGameInstance* GI2 = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66CompanionUnlockSubsystem* Unlocks = GI2->GetSubsystem<UT66CompanionUnlockSubsystem>())
			{
				const bool bUnlocked = Unlocks->IsCompanionUnlocked(CompanionID);
				PreviewPawn->SetLockedVisual(!bUnlocked);
			}
		}

		FVector PawnLoc = GetActorLocation() + GetActorRotation().RotateVector(PreviewPawnOffset);
		// Keep pawn anchored; camera frames it dynamically.
		PawnLoc.Z = GetActorLocation().Z;
		PreviewPawn->SetActorLocation(PawnLoc);
		ApplyPreviewRotation();

		// Stream textures so alert animation is visible.
		if (PreviewPawn->SkeletalMesh)
		{
			PreviewPawn->SkeletalMesh->bForceMipStreaming = true;
			PreviewPawn->SkeletalMesh->StreamingDistanceMultiplier = 50.f;
		}

		ActivePreviewCompanionID = CompanionID;
		ActivePreviewSkinID = EffectiveSkin;
	}
}

void AT66CompanionPreviewStage::SetStageVisible(bool bVisible)
{
	bStageVisible = bVisible;
	SetActorHiddenInGame(!bVisible);
	if (PreviewPawn) PreviewPawn->SetActorHiddenInGame(!bVisible);
	RefreshPreviewEnvironment();
}

void AT66CompanionPreviewStage::ResetFramingCache()
{
	bHasOrbitFrame = false;
}

void AT66CompanionPreviewStage::RefreshPreviewEnvironment()
{
	if (PreviewFloor)
	{
		PreviewFloor->SetVisibility(bStageVisible, true);
		PreviewFloor->SetHiddenInGame(!bStageVisible, true);
	}

	T66PreviewStageEnvironment::SetPreviewPropsVisibility(
		EasyPreviewProps,
		bStageVisible && T66PreviewStageEnvironment::ShouldShowEasyProps(PreviewDifficulty));
}
