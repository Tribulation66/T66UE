// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66CompanionBase.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

AT66CompanionPreviewStage::AT66CompanionPreviewStage()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Simple floor plane for grounding reference.
	PreviewFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewFloor"));
	PreviewFloor->SetupAttachment(RootComponent);
	PreviewFloor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		PreviewFloor->SetStaticMesh(Cylinder);
		PreviewFloor->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
		PreviewFloor->SetRelativeScale3D(FVector(2.2f, 2.2f, 0.08f));
	}

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	// Capture on-demand (preview updates only when companion/rotation/zoom changes).
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->FOVAngle = 30.f;
	// Do NOT override exposure here; we want it to match the gameplay lighting feel.
	SceneCapture->PostProcessBlendWeight = 0.0f;

	PreviewLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewLight"));
	PreviewLight->SetupAttachment(RootComponent);
	PreviewLight->SetRelativeLocation(PreviewPawnOffset * 0.5f);
	PreviewLight->SetIntensity(5200.f);
	PreviewLight->SetAttenuationRadius(6000.f);
	PreviewLight->SetLightColor(FLinearColor(1.f, 0.95f, 0.9f));

	FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(RootComponent);
	FillLight->SetIntensity(2200.f);
	FillLight->SetAttenuationRadius(8000.f);
	FillLight->SetLightColor(FLinearColor(0.85f, 0.90f, 1.0f));

	RimLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RimLight"));
	RimLight->SetupAttachment(RootComponent);
	RimLight->SetIntensity(2600.f);
	RimLight->SetAttenuationRadius(8000.f);
	RimLight->SetLightColor(FLinearColor(1.0f, 1.0f, 1.0f));

	CompanionPawnClass = AT66CompanionBase::StaticClass();
}

void AT66CompanionPreviewStage::BeginPlay()
{
	Super::BeginPlay();
	EnsureCaptureSetup();
}

void AT66CompanionPreviewStage::EnsureCaptureSetup()
{
	if (!SceneCapture) return;
	if (!RenderTarget)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), TEXT("PreviewRenderTarget"));
		if (RenderTarget)
		{
			RenderTarget->RenderTargetFormat = RTF_RGBA8;
			RenderTarget->InitAutoFormat(RenderTargetSize.X, RenderTargetSize.Y);
			RenderTarget->UpdateResource();
		}
	}
	else if (RenderTarget->SizeX != RenderTargetSize.X || RenderTarget->SizeY != RenderTargetSize.Y)
	{
		RenderTarget->InitAutoFormat(RenderTargetSize.X, RenderTargetSize.Y);
		RenderTarget->UpdateResource();
	}
	if (RenderTarget) SceneCapture->TextureTarget = RenderTarget;
}

void AT66CompanionPreviewStage::SetPreviewCompanion(FName CompanionID)
{
	PreviewYawDegrees = 0.f;
	OrbitPitchDegrees = 8.f;
	PreviewZoomMultiplier = 1.0f;
	UpdatePreviewPawn(CompanionID);
	ApplyShadowSettings();
	bHasOrbitFrame = false;
	FrameCameraToPreview();
	CapturePreview();
}

void AT66CompanionPreviewStage::AddPreviewYaw(float DeltaYawDegrees)
{
	PreviewYawDegrees = FMath::Fmod(PreviewYawDegrees + DeltaYawDegrees, 360.f);
	ApplyPreviewRotation();
	FrameCameraToPreview();
	CapturePreview();
}

void AT66CompanionPreviewStage::AddPreviewZoom(float WheelDelta)
{
	static constexpr float StepPerWheel = 0.08f;
	const float MinZ = FMath::Clamp(MinPreviewZoomMultiplier, 0.25f, 1.0f);
	PreviewZoomMultiplier = FMath::Clamp(PreviewZoomMultiplier - (WheelDelta * StepPerWheel), MinZ, 1.0f);
	FrameCameraToPreview();
	CapturePreview();
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
	if (!SceneCapture) return;
	UPrimitiveComponent* Target = GetPreviewTargetComponent();
	if (!Target) return;

	if (!bHasOrbitFrame)
	{
		const FBoxSphereBounds B = Target->Bounds;
		OrbitCenter = B.Origin;
		OrbitRadius = FMath::Max(25.f, B.SphereRadius);
		OrbitBottomZ = (B.Origin.Z - B.BoxExtent.Z);
		bHasOrbitFrame = true;
	}

	const FVector Center = OrbitCenter;
	const float Radius = OrbitRadius;

	const float HalfFovRad = FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f);
	const float BaseMult = FMath::Clamp(CameraDistanceMultiplier, 0.60f, 2.0f);
	const float ZoomMult = FMath::Clamp(PreviewZoomMultiplier, FMath::Clamp(MinPreviewZoomMultiplier, 0.25f, 1.0f), 1.0f);
	const float EffectiveMult = FMath::Clamp(BaseMult * ZoomMult, 0.25f, BaseMult);
	const float Dist = (Radius / FMath::Max(0.15f, FMath::Tan(HalfFovRad))) * EffectiveMult;

	const float PitchRad = FMath::DegreesToRadians(OrbitPitchDegrees);
	const float Z = (FMath::Sin(PitchRad) * Dist) + (Radius * 0.12f);
	const float X = (-FMath::Cos(PitchRad) * Dist);
	FVector CamLoc = Center + FVector(X, 0.f, Z);
	CamLoc.Z = FMath::Max(CamLoc.Z, OrbitBottomZ + 60.f);
	SceneCapture->SetWorldLocation(CamLoc);

	const FRotator LookRot = FRotationMatrix::MakeFromX(Center - CamLoc).Rotator();
	SceneCapture->SetWorldRotation(LookRot);

	if (PreviewLight)
	{
		const FVector Forward = (Center - CamLoc).GetSafeNormal();
		const FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();
		const FVector Up = FVector::UpVector;
		PreviewLight->SetWorldLocation(CamLoc + Forward * (Dist * 0.20f) + Right * (Radius * 0.55f) + Up * (Radius * 0.55f));
	}
	if (FillLight)
	{
		const FVector Forward = (Center - CamLoc).GetSafeNormal();
		const FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();
		const FVector Up = FVector::UpVector;
		FillLight->SetWorldLocation(CamLoc + Forward * (Dist * 0.25f) - Right * (Radius * 0.65f) + Up * (Radius * 0.35f));
	}
	if (RimLight)
	{
		RimLight->SetWorldLocation(Center + FVector(Dist * 0.15f, 0.f, Radius * 0.75f));
	}
	if (PreviewFloor && PreviewPawn)
	{
		// Keep floor under the pawn's origin so "ground" is stable.
		PreviewFloor->SetWorldLocation(FVector(Center.X + FloorForwardOffset, Center.Y, OrbitBottomZ - 6.f));
		const float S = FMath::Clamp((Radius / 50.f) * 1.6f, 1.2f, 6.0f);
		PreviewFloor->SetWorldScale3D(FVector(S, S, 0.08f));
	}
}

void AT66CompanionPreviewStage::ApplyShadowSettings()
{
	if (!bDisablePreviewShadows)
	{
		return;
	}

	if (PreviewFloor)
	{
		PreviewFloor->SetCastShadow(false);
	}

	if (!PreviewPawn)
	{
		return;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimComps;
	PreviewPawn->GetComponents(PrimComps);
	for (UPrimitiveComponent* Prim : PrimComps)
	{
		if (!Prim) continue;
		Prim->SetCastShadow(false);
	}
}

void AT66CompanionPreviewStage::UpdatePreviewPawn(FName CompanionID)
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) return;

	if (CompanionID.IsNone())
	{
		if (PreviewPawn) PreviewPawn->SetActorHiddenInGame(true);
		return;
	}

	FCompanionData CompanionData;
	if (!GI->GetCompanionData(CompanionID, CompanionData))
	{
		if (PreviewPawn) PreviewPawn->SetActorHiddenInGame(true);
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

	if (PreviewPawn)
	{
		PreviewPawn->SetActorHiddenInGame(false);
		PreviewPawn->InitializeCompanion(CompanionData);
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

		// Make sure materials/textures are fully streamed for the preview camera.
		if (PreviewPawn->SkeletalMesh)
		{
			PreviewPawn->SkeletalMesh->bForceMipStreaming = true;
			PreviewPawn->SkeletalMesh->StreamingDistanceMultiplier = 50.f;
			PreviewPawn->SkeletalMesh->bPauseAnims = true; // preview should be static
			PreviewPawn->SkeletalMesh->GlobalAnimRateScale = 0.f;
			PreviewPawn->SkeletalMesh->Stop();
		}
	}
}

void AT66CompanionPreviewStage::CapturePreview()
{
	if (SceneCapture && RenderTarget) SceneCapture->CaptureScene();
}
