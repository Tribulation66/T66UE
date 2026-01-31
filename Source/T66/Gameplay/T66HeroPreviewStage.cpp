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

AT66HeroPreviewStage::AT66HeroPreviewStage()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root (invisible)
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Scene capture for rendering the preview
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	SceneCapture->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	SceneCapture->bCaptureEveryFrame = false; // Only capture when we call CapturePreview
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->FOVAngle = 30.f;
	// Do NOT override exposure here; we want it to match the gameplay lighting feel.
	SceneCapture->PostProcessBlendWeight = 0.0f;

	// Light so the preview hero is visible
	PreviewLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewLight"));
	PreviewLight->SetupAttachment(RootComponent);
	PreviewLight->SetRelativeLocation(PreviewPawnOffset * 0.5f); // Between camera and pawn
	PreviewLight->SetIntensity(1800.f);
	PreviewLight->SetAttenuationRadius(3000.f);
	PreviewLight->SetLightColor(FLinearColor(1.f, 0.95f, 0.9f));

	FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(RootComponent);
	FillLight->SetIntensity(650.f);
	FillLight->SetAttenuationRadius(3000.f);
	FillLight->SetLightColor(FLinearColor(0.85f, 0.90f, 1.0f));

	RimLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RimLight"));
	RimLight->SetupAttachment(RootComponent);
	RimLight->SetIntensity(900.f);
	RimLight->SetAttenuationRadius(3500.f);
	RimLight->SetLightColor(FLinearColor(1.0f, 1.0f, 1.0f));

	PreviewPlatform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewPlatform"));
	PreviewPlatform->SetupAttachment(RootComponent);
	PreviewPlatform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		PreviewPlatform->SetStaticMesh(Cylinder);
		PreviewPlatform->SetRelativeScale3D(FVector(2.2f, 2.2f, 0.08f));
		PreviewPlatform->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	}

	// Default to our hero class
	HeroPawnClass = AT66HeroBase::StaticClass();
}

void AT66HeroPreviewStage::BeginPlay()
{
	Super::BeginPlay();
	EnsureCaptureSetup();
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

void AT66HeroPreviewStage::SetPreviewHero(FName HeroID, ET66BodyType BodyType)
{
	PreviewYawDegrees = 0.f;
	UpdatePreviewPawn(HeroID, BodyType);
	FrameCameraToPreview();
	CapturePreview();
}

void AT66HeroPreviewStage::AddPreviewYaw(float DeltaYawDegrees)
{
	PreviewYawDegrees = FMath::Fmod(PreviewYawDegrees + DeltaYawDegrees, 360.f);
	ApplyPreviewRotation();
	CapturePreview();
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

	const FBoxSphereBounds B = Target->Bounds;
	const FVector Center = B.Origin;
	const float Radius = FMath::Max(25.f, B.SphereRadius);

	// Fit the whole bounds sphere in view.
	const float HalfFovRad = FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f);
	// Slightly "zoomed in" so the character feels large (Dota-style).
	const float Dist = (Radius / FMath::Max(0.15f, FMath::Tan(HalfFovRad))) * 1.05f;

	const FVector CamLoc = Center + FVector(-Dist, 0.f, Radius * 0.12f);
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
		// Put platform just below feet.
		const float BottomZ = B.Origin.Z - B.BoxExtent.Z;
		PreviewPlatform->SetWorldLocation(FVector(Center.X, Center.Y, BottomZ - 6.f));
		const float S = FMath::Clamp((Radius / 50.f) * 1.6f, 1.2f, 6.0f);
		PreviewPlatform->SetWorldScale3D(FVector(S, S, 0.08f));
	}
}

void AT66HeroPreviewStage::UpdatePreviewPawn(FName HeroID, ET66BodyType BodyType)
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) return;

	FHeroData HeroData;
	if (!GI->GetHeroData(HeroID, HeroData))
	{
		// No hero data - hide or clear preview
		if (PreviewPawn)
		{
			PreviewPawn->SetActorHiddenInGame(true);
		}
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// Spawn new pawn if we don't have one or we need a fresh one
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
		PreviewPawn->InitializeHero(HeroData, BodyType);
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
	}
}

void AT66HeroPreviewStage::CapturePreview()
{
	if (SceneCapture && RenderTarget)
	{
		SceneCapture->CaptureScene();
	}
}
