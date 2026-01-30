// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66CompanionPreviewStage.h"
#include "Gameplay/T66CompanionBase.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/StaticMesh.h"

AT66CompanionPreviewStage::AT66CompanionPreviewStage()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Simple floor plane for grounding reference.
	PreviewFloor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewFloor"));
	PreviewFloor->SetupAttachment(RootComponent);
	PreviewFloor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		PreviewFloor->SetStaticMesh(Cube);
		PreviewFloor->SetRelativeLocation(FVector(150.f, 0.f, -50.f));
		PreviewFloor->SetRelativeScale3D(FVector(2.2f, 2.2f, 1.0f));
	}

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->SetRelativeLocation(FVector(-120.f, 0.f, 60.f));
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCapture->FOVAngle = 30.f;

	PreviewLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewLight"));
	PreviewLight->SetupAttachment(RootComponent);
	PreviewLight->SetRelativeLocation(PreviewPawnOffset * 0.5f);
	PreviewLight->SetIntensity(3000.f);
	PreviewLight->SetAttenuationRadius(500.f);
	PreviewLight->SetLightColor(FLinearColor(1.f, 0.95f, 0.9f));

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
	if (RenderTarget) SceneCapture->TextureTarget = RenderTarget;
}

void AT66CompanionPreviewStage::SetPreviewCompanion(FName CompanionID)
{
	UpdatePreviewPawn(CompanionID);
	CapturePreview();
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
		FVector PawnLoc = GetActorLocation() + GetActorRotation().RotateVector(PreviewPawnOffset);
		// Ground the preview pawn on top of the preview floor.
		static constexpr float CompanionRadius = 20.f; // sphere radius at scale 0.4
		PawnLoc.Z = GetActorLocation().Z + CompanionRadius;
		PreviewPawn->SetActorLocation(PawnLoc);
		PreviewPawn->SetActorRotation(FRotator(0.f, 180.f, 0.f));
		if (SceneCapture)
		{
			FVector CaptureLoc = SceneCapture->GetComponentLocation();
			FRotator LookRot = FRotationMatrix::MakeFromX(PawnLoc - CaptureLoc).Rotator();
			SceneCapture->SetWorldRotation(LookRot);
		}
	}
}

void AT66CompanionPreviewStage::CapturePreview()
{
	if (SceneCapture && RenderTarget) SceneCapture->CaptureScene();
}
