// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66HeroPreviewStage.h"
#include "Gameplay/T66HeroBase.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"

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

	// Light so the preview hero is visible
	PreviewLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewLight"));
	PreviewLight->SetupAttachment(RootComponent);
	PreviewLight->SetRelativeLocation(PreviewPawnOffset * 0.5f); // Between camera and pawn
	PreviewLight->SetIntensity(3000.f);
	PreviewLight->SetAttenuationRadius(500.f);
	PreviewLight->SetLightColor(FLinearColor(1.f, 0.95f, 0.9f));

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

	if (RenderTarget)
	{
		SceneCapture->TextureTarget = RenderTarget;
	}
}

void AT66HeroPreviewStage::SetPreviewHero(FName HeroID, ET66BodyType BodyType)
{
	UpdatePreviewPawn(HeroID, BodyType);
	CapturePreview();
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
		// Keep pawn in front of camera (in front of this actor)
		FVector PawnLoc = GetActorLocation() + GetActorRotation().RotateVector(PreviewPawnOffset);
		PreviewPawn->SetActorLocation(PawnLoc);
		PreviewPawn->SetActorRotation(FRotator(0.f, 180.f, 0.f)); // Face the camera
		// Point scene capture at the pawn
		if (SceneCapture)
		{
			FVector CaptureLoc = SceneCapture->GetComponentLocation();
			FRotator LookRot = FRotationMatrix::MakeFromX(PawnLoc - CaptureLoc).Rotator();
			SceneCapture->SetWorldRotation(LookRot);
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
