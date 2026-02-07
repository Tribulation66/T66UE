// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Gameplay/T66PreviewMaterials.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Math/RandomStream.h"

AT66HeroPreviewStage::AT66HeroPreviewStage()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork; // After animation and movement so capture sees updated pose

	// Root (invisible)
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Scene capture for rendering the preview
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	SceneCapture->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	// Capture on-demand (preview updates only when hero/rotation/zoom changes).
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->FOVAngle = 30.f;
	// Do NOT override exposure here; we want it to match the gameplay lighting feel.
	SceneCapture->PostProcessBlendWeight = 0.0f;

	// Light so the preview hero is visible
	PreviewLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewLight"));
	PreviewLight->SetupAttachment(RootComponent);
	PreviewLight->SetRelativeLocation(PreviewPawnOffset * 0.5f); // Between camera and pawn
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

	PreviewPlatform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewPlatform"));
	PreviewPlatform->SetupAttachment(RootComponent);
	PreviewPlatform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		PreviewPlatform->SetStaticMesh(Cylinder);
		PreviewPlatform->SetRelativeScale3D(FVector(4.0f, 4.0f, 0.04f));
		PreviewPlatform->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	}

	// Sky dome (inverted sphere) so the character has a colored backdrop instead of void.
	BackdropSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BackdropSphere"));
	BackdropSphere->SetupAttachment(RootComponent);
	BackdropSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackdropSphere->SetCastShadow(false);
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh)
	{
		BackdropSphere->SetStaticMesh(SphereMesh);
		// Negative scale inverts face winding so the camera (inside) sees the interior.
		// -8 = 800 unit radius — small enough that it's close to camera/lights.
		BackdropSphere->SetRelativeScale3D(FVector(-8.f, -8.f, -8.f));
	}

	// Ambient sky light for uniform backdrop illumination and overall atmosphere color.
	AmbientSkyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("AmbientSkyLight"));
	AmbientSkyLight->SetupAttachment(RootComponent);
	AmbientSkyLight->SetIntensity(4000.f);
	AmbientSkyLight->SetAttenuationRadius(15000.f);
	AmbientSkyLight->SetLightColor(FLinearColor(0.6f, 0.8f, 1.0f));
	AmbientSkyLight->SetCastShadows(false);

	// Star dots — small spheres scattered on the upper hemisphere of the dome.
	// Visible only in dark mode; use a fixed random seed for deterministic layout.
	{
		static constexpr int32 NumStars = 20;
		FRandomStream StarRNG(7777);

		for (int32 i = 0; i < NumStars; ++i)
		{
			UStaticMeshComponent* Star = CreateDefaultSubobject<UStaticMeshComponent>(
				*FString::Printf(TEXT("Star_%02d"), i));
			Star->SetupAttachment(RootComponent);
			Star->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Star->SetCastShadow(false);
			Star->SetVisibility(false); // hidden until dark mode
			if (SphereMesh) Star->SetStaticMesh(SphereMesh);

			const float Scale = 0.06f + StarRNG.FRandRange(0.0f, 0.06f);
			Star->SetRelativeScale3D(FVector(Scale));

			// Random position on the upper hemisphere (radius 700, inside the 800-radius dome).
			const float Theta = StarRNG.FRandRange(0.f, 2.f * UE_PI);
			const float Phi   = StarRNG.FRandRange(0.05f, 0.45f * UE_PI); // upper ~80 degrees
			const float R     = 700.f;
			const FVector Offset(
				R * FMath::Sin(Phi) * FMath::Cos(Theta),
				R * FMath::Sin(Phi) * FMath::Sin(Theta),
				R * FMath::Cos(Phi));

			StarMeshes.Add(Star);
			StarOffsets.Add(Offset);
		}
	}

	// Default to our hero class
	HeroPawnClass = AT66HeroBase::StaticClass();
}

void AT66HeroPreviewStage::BeginPlay()
{
	Super::BeginPlay();
	EnsureCaptureSetup();

	// Load preview materials (from disk or auto-created in editor builds).
	if (UMaterialInterface* GrassMat = T66PreviewMaterials::GetGroundMaterial())
	{
		GroundMID = UMaterialInstanceDynamic::Create(GrassMat, this);
		if (GroundMID && PreviewPlatform) PreviewPlatform->SetMaterial(0, GroundMID);
	}
	else if (PreviewPlatform && PreviewPlatform->GetStaticMesh())
	{
		GroundMID = PreviewPlatform->CreateDynamicMaterialInstance(0);
	}

	if (UMaterialInterface* SkyMat = T66PreviewMaterials::GetSkyMaterial())
	{
		BackdropMID = UMaterialInstanceDynamic::Create(SkyMat, this);
		if (BackdropMID && BackdropSphere) BackdropSphere->SetMaterial(0, BackdropMID);
	}
	else if (BackdropSphere && BackdropSphere->GetStaticMesh())
	{
		BackdropMID = BackdropSphere->CreateDynamicMaterialInstance(0);
	}

	// Star dot material (same pattern: VectorParam → EmissiveColor).
	if (UMaterialInterface* StarMat = T66PreviewMaterials::GetStarMaterial())
	{
		StarMID = UMaterialInstanceDynamic::Create(StarMat, this);
		for (UStaticMeshComponent* Star : StarMeshes)
		{
			if (Star && StarMID) Star->SetMaterial(0, StarMID);
		}
	}

	// Listen for theme changes (Dark/Light → Night/Day lighting).
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66PlayerSettingsSubsystem* Settings = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			Settings->OnSettingsChanged.AddDynamic(this, &AT66HeroPreviewStage::OnThemeChanged);
		}
	}

	ApplyThemeLighting();
}

void AT66HeroPreviewStage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66PlayerSettingsSubsystem* Settings = GI->GetSubsystem<UT66PlayerSettingsSubsystem>())
		{
			Settings->OnSettingsChanged.RemoveDynamic(this, &AT66HeroPreviewStage::OnThemeChanged);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AT66HeroPreviewStage::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Capture every frame so the preview shows the current animated pose (we tick after the pawn).
	if (PreviewPawn && SceneCapture && RenderTarget)
	{
		SceneCapture->CaptureScene();
	}
}

void AT66HeroPreviewStage::EnsureCaptureSetup()
{
	if (!SceneCapture) return;

	// Create render target at runtime if not set
	if (!RenderTarget)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this, UTextureRenderTarget2D::StaticClass(), TEXT("PreviewRenderTarget"));
		if (RenderTarget)
		{
			RenderTarget->RenderTargetFormat = RTF_RGBA8;
			RenderTarget->InitAutoFormat(RenderTargetSize.X, RenderTargetSize.Y);
			RenderTarget->UpdateResource();
			UE_LOG(LogTemp, Log, TEXT("HeroPreviewStage: Created render target %dx%d"), RenderTargetSize.X, RenderTargetSize.Y);
		}
	}
	else if (RenderTarget->SizeX != RenderTargetSize.X || RenderTarget->SizeY != RenderTargetSize.Y)
	{
		RenderTarget->InitAutoFormat(RenderTargetSize.X, RenderTargetSize.Y);
		RenderTarget->UpdateResource();
		UE_LOG(LogTemp, Log, TEXT("HeroPreviewStage: Resized render target to %dx%d"), RenderTargetSize.X, RenderTargetSize.Y);
	}

	if (RenderTarget)
	{
		SceneCapture->TextureTarget = RenderTarget;
	}
}

void AT66HeroPreviewStage::SetPreviewHero(FName HeroID, ET66BodyType BodyType, FName SkinID)
{
	PreviewYawDegrees = 0.f;
	OrbitPitchDegrees = 8.f;
	PreviewZoomMultiplier = 1.0f;
	UpdatePreviewPawn(HeroID, BodyType, SkinID.IsNone() ? FName(TEXT("Default")) : SkinID);
	ApplyShadowSettings();
	bHasOrbitFrame = false;
	FrameCameraToPreview();

	// Drive capture from our Tick (after pawn anim) so the alert animation is visible; avoid bCaptureEveryFrame timing.
	if (SceneCapture)
	{
		SceneCapture->bCaptureEveryFrame = false;
	}
	// Ensure we tick after the preview pawn so animation has updated before we capture.
	if (PreviewPawn)
	{
		AddTickPrerequisiteActor(PreviewPawn);
	}
	CapturePreview();
}

void AT66HeroPreviewStage::AddPreviewYaw(float DeltaYawDegrees)
{
	PreviewYawDegrees = FMath::Fmod(PreviewYawDegrees + DeltaYawDegrees, 360.f);
	ApplyPreviewRotation();
	FrameCameraToPreview();
	CapturePreview();
}

void AT66HeroPreviewStage::AddPreviewZoom(float WheelDelta)
{
	// WheelDelta: positive = wheel up (zoom in), negative = wheel down (zoom out).
	// We clamp max zoom-out to the default framing, so wheel down just returns toward 1.0.
	static constexpr float StepPerWheel = 0.08f;
	const float MinZ = FMath::Clamp(MinPreviewZoomMultiplier, 0.25f, 1.0f);
	PreviewZoomMultiplier = FMath::Clamp(PreviewZoomMultiplier - (WheelDelta * StepPerWheel), MinZ, 1.0f);
	FrameCameraToPreview();
	CapturePreview();
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
}

void AT66HeroPreviewStage::FrameCameraToPreview()
{
	if (!SceneCapture) return;
	UPrimitiveComponent* Target = GetPreviewTargetComponent();
	if (!Target) return;

	// Cache orbit framing once per hero selection so the platform/camera don't "swim" as you rotate.
	if (!bHasOrbitFrame)
	{
		const FBoxSphereBounds B = Target->Bounds;
		OrbitCenter = B.Origin;
		OrbitRadius = FMath::Max(25.f, B.SphereRadius);
		OrbitBottomZ = (B.Origin.Z - B.BoxExtent.Z);
		bHasOrbitFrame = true;

		UE_LOG(LogTemp, Log, TEXT("[PREVIEW] HeroPreview orbit: Center=(%.1f,%.1f,%.1f) Radius=%.1f BottomZ=%.1f BoundsExtent=(%.1f,%.1f,%.1f)"),
			OrbitCenter.X, OrbitCenter.Y, OrbitCenter.Z, OrbitRadius, OrbitBottomZ,
			B.BoxExtent.X, B.BoxExtent.Y, B.BoxExtent.Z);
	}

	const FVector Center = OrbitCenter;
	const float Radius = OrbitRadius;

	// Fit the whole bounds sphere in view.
	const float HalfFovRad = FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f);
	// Slightly "zoomed in" so the character feels large (Dota-style).
	const float BaseMult = FMath::Clamp(CameraDistanceMultiplier, 0.60f, 2.0f);
	const float ZoomMult = FMath::Clamp(PreviewZoomMultiplier, FMath::Clamp(MinPreviewZoomMultiplier, 0.25f, 1.0f), 1.0f);
	const float EffectiveMult = FMath::Clamp(BaseMult * ZoomMult, 0.25f, BaseMult);
	const float Dist = (Radius / FMath::Max(0.15f, FMath::Tan(HalfFovRad))) * EffectiveMult;

	const float PitchRad = FMath::DegreesToRadians(OrbitPitchDegrees);
	const float Z = (FMath::Sin(PitchRad) * Dist) + (Radius * 0.12f);
	const float X = (-FMath::Cos(PitchRad) * Dist);
	FVector CamLoc = Center + FVector(X, 0.f, Z);
	// Never allow the camera to dip below the "ground" plane (prevents seeing under the platform).
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
	if (PreviewPlatform)
	{
		// Put platform just below feet — wider/flatter for a "grass field" look.
		PreviewPlatform->SetWorldLocation(FVector(Center.X + PlatformForwardOffset, Center.Y, OrbitBottomZ - 6.f));
		const float S = FMath::Clamp((Radius / 50.f) * 2.5f, 2.0f, 10.0f);
		PreviewPlatform->SetWorldScale3D(FVector(S, S, 0.04f));
	}
	// Ensure backdrop dome always encloses the camera.  Characters with large mesh
	// bounds push the camera far away; a fixed-size dome (original -8 scale ≈ 400uu
	// radius) can leave the camera outside the dome → black void in the capture.
	{
		const float CamDistFromCenter = (CamLoc - Center).Size();
		// Engine Sphere = 50uu radius at scale 1.  Need dome radius > CamDist.
		static constexpr float SphereBaseRadius = 50.f;
		static constexpr float OriginalScale    = 8.f;
		const float RequiredRadius = CamDistFromCenter * 1.5f + 200.f;
		const float NeededScale    = FMath::Max(OriginalScale, RequiredRadius / SphereBaseRadius);

		if (BackdropSphere)
		{
			BackdropSphere->SetWorldLocation(Center);
			BackdropSphere->SetWorldScale3D(FVector(-NeededScale));
		}
		if (AmbientSkyLight)
		{
			AmbientSkyLight->SetWorldLocation(Center);
			AmbientSkyLight->SetAttenuationRadius(FMath::Max(15000.f, RequiredRadius * 2.5f));
		}
		// Scale star dot positions proportionally so they stay near the dome surface.
		const float StarScale = NeededScale / OriginalScale;
		for (int32 i = 0; i < StarMeshes.Num(); ++i)
		{
			if (StarMeshes[i])
			{
				StarMeshes[i]->SetWorldLocation(Center + StarOffsets[i] * StarScale);
			}
		}

		// Scale key/fill/rim attenuation so lights always reach the character.
		const float MinAtten = FMath::Max(6000.f, CamDistFromCenter * 2.5f);
		if (PreviewLight) PreviewLight->SetAttenuationRadius(MinAtten);
		if (FillLight)    FillLight->SetAttenuationRadius(FMath::Max(8000.f, MinAtten));
		if (RimLight)     RimLight->SetAttenuationRadius(FMath::Max(8000.f, MinAtten));
	}
}

void AT66HeroPreviewStage::ApplyShadowSettings()
{
	if (!bDisablePreviewShadows)
	{
		return;
	}

	if (PreviewPlatform)
	{
		PreviewPlatform->SetCastShadow(false);
	}
	if (BackdropSphere)
	{
		BackdropSphere->SetCastShadow(false);
	}

	if (!PreviewPawn)
	{
		return;
	}

	// Disable shadow casting on all primitive components of the preview pawn (skeletal mesh, placeholder mesh, etc.).
	TInlineComponentArray<UPrimitiveComponent*> PrimComps;
	PreviewPawn->GetComponents(PrimComps);
	for (UPrimitiveComponent* Prim : PrimComps)
	{
		if (!Prim) continue;
		Prim->SetCastShadow(false);
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
		// Keep pawn anchored at the stage origin; camera frames it dynamically.
		FVector PawnLoc = GetActorLocation() + GetActorRotation().RotateVector(PreviewPawnOffset);
		PreviewPawn->SetActorLocation(PawnLoc);
		ApplyPreviewRotation();

		// Stream textures; allow alert animation to play. Force mesh to tick so scene capture sees the animation.
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

void AT66HeroPreviewStage::CapturePreview()
{
	if (SceneCapture && RenderTarget)
	{
		SceneCapture->CaptureScene();
	}
}

void AT66HeroPreviewStage::OnThemeChanged()
{
	ApplyThemeLighting();
}

void AT66HeroPreviewStage::ApplyThemeLighting()
{
	const bool bDay = (FT66Style::GetTheme() == ET66UITheme::Light);

	// --- Key light (sun / moon) ---
	if (PreviewLight)
	{
		PreviewLight->SetLightColor(bDay
			? FLinearColor(1.0f, 0.95f, 0.85f)    // warm sunlight
			: FLinearColor(0.6f, 0.7f, 0.95f));    // cool moonlight
		PreviewLight->SetIntensity(bDay ? 8000.f : 5500.f);
	}

	// --- Fill light ---
	if (FillLight)
	{
		FillLight->SetLightColor(bDay
			? FLinearColor(0.95f, 0.90f, 0.80f)   // warm fill
			: FLinearColor(0.50f, 0.60f, 0.85f));  // blue fill
		FillLight->SetIntensity(bDay ? 3000.f : 2800.f);
	}

	// --- Rim light ---
	if (RimLight)
	{
		RimLight->SetLightColor(bDay
			? FLinearColor(1.0f, 1.0f, 1.0f)      // white rim
			: FLinearColor(0.7f, 0.75f, 0.95f));   // silver-blue rim
		RimLight->SetIntensity(bDay ? 3000.f : 3000.f);
	}

	// --- Ambient sky light (backdrop illumination + atmosphere) ---
	if (AmbientSkyLight)
	{
		AmbientSkyLight->SetLightColor(bDay
			? FLinearColor(0.55f, 0.75f, 1.0f)    // sky blue
			: FLinearColor(0.20f, 0.25f, 0.45f));  // blue-navy ambient
		AmbientSkyLight->SetIntensity(bDay ? 4000.f : 3500.f);
	}

	// --- Ground material (grass) ---
	if (GroundMID)
	{
		const FLinearColor GrassBase = bDay
			? FLinearColor(0.15f, 0.45f, 0.08f)   // bright grass green
			: FLinearColor(0.06f, 0.18f, 0.05f);   // moonlit green (brighter than before)
		const FLinearColor GrassVar = bDay
			? FLinearColor(0.10f, 0.35f, 0.05f)
			: FLinearColor(0.03f, 0.12f, 0.03f);
		GroundMID->SetVectorParameterValue(FName(TEXT("BaseColor")), GrassBase);
		GroundMID->SetVectorParameterValue(FName(TEXT("VariationColor")), GrassVar);
		GroundMID->SetVectorParameterValue(FName(TEXT("Color")), GrassBase);
	}

	// --- Sky dome (single emissive color — no multi-node chain) ---
	// Always blue sky (matching gameplay level's SkyAtmosphere look).
	// Values >> 1.0 so they survive auto-exposure tonemapping.
	if (BackdropMID)
	{
		BackdropMID->SetVectorParameterValue(FName(TEXT("SkyColor")),
			bDay ? FLinearColor(2.0f, 3.5f, 6.0f)    // bright blue sky
			     : FLinearColor(1.2f, 2.0f, 4.0f));   // dimmer but still blue sky
	}

	// --- Stars (small emissive sphere meshes) ---
	const bool bShowStars = !bDay;
	for (UStaticMeshComponent* Star : StarMeshes)
	{
		if (Star) Star->SetVisibility(bShowStars);
	}
	if (StarMID)
	{
		StarMID->SetVectorParameterValue(FName(TEXT("StarColor")),
			bDay ? FLinearColor(0.f, 0.f, 0.f, 0.f)         // hidden (safety)
			     : FLinearColor(8.0f, 8.0f, 7.0f, 1.0f));    // bright warm white
	}
}
