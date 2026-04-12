// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66EclipseActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"
#include "Misc/PackageName.h"

DEFINE_LOG_CATEGORY_STATIC(LogT66Eclipse, Log, All);

static const TCHAR* EclipseMaterialPackagePath = TEXT("/Game/Lighting/M_EclipseCorona");
static const TCHAR* EclipseMaterialPath = TEXT("/Game/Lighting/M_EclipseCorona.M_EclipseCorona");

AT66EclipseActor::AT66EclipseActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.066f;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	PlaneMesh->SetupAttachment(RootComponent);
	PlaneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlaneMesh->SetCastShadow(false);
	PlaneMesh->bReceivesDecals = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneFinder.Succeeded())
	{
		PlaneMesh->SetStaticMesh(PlaneFinder.Object);
	}
}

void AT66EclipseActor::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* BaseMat = nullptr;
	if (FPackageName::DoesPackageExist(EclipseMaterialPackagePath))
	{
		BaseMat = LoadObject<UMaterialInterface>(nullptr, EclipseMaterialPath);
	}
	if (BaseMat)
	{
		CoronaDMI = UMaterialInstanceDynamic::Create(BaseMat, this);
		PlaneMesh->SetMaterial(0, CoronaDMI);
	}
	else
	{
		PlaneMesh->SetHiddenInGame(true);
		PlaneMesh->SetVisibility(false, true);
	}

	// Find the moon directional light and position along its direction.
	UWorld* World = GetWorld();
	if (!World) return;

	ADirectionalLight* MoonLight = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		if (It->GetName().Contains(TEXT("Moon")) || It->GetName().Contains(TEXT("moon")))
		{
			MoonLight = *It;
			break;
		}
	}
	// Fallback: use any directional light with AtmosphereSunLightIndex 0
	if (!MoonLight)
	{
		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			if (UDirectionalLightComponent* LC = Cast<UDirectionalLightComponent>(It->GetLightComponent()))
			{
				if (LC->bAtmosphereSunLight && LC->AtmosphereSunLightIndex == 0)
				{
					MoonLight = *It;
					break;
				}
			}
		}
	}

	if (MoonLight)
	{
		const FVector LightDir = MoonLight->GetActorForwardVector();
		const FVector EclipsePos = MoonLight->GetActorLocation() + LightDir * 500000.f;
		SetActorLocation(EclipsePos);
		SetActorScale3D(FVector(8000.f, 8000.f, 1.f));
	}
}

void AT66EclipseActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	OrientTowardCamera();
}

void AT66EclipseActor::OrientTowardCamera()
{
	APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!CamMgr) return;

	const FVector CamLoc = CamMgr->GetCameraLocation();
	const FVector ToCamera = (CamLoc - GetActorLocation()).GetSafeNormal();

	if (!ToCamera.IsNearlyZero())
	{
		const FRotator DesiredRotation = ToCamera.Rotation();
		if (!GetActorRotation().Equals(DesiredRotation, 0.1f))
		{
			SetActorRotation(DesiredRotation);
		}
	}
}

void AT66EclipseActor::SetCoronaIntensity(float Intensity)
{
	if (CoronaDMI)
	{
		CoronaDMI->SetScalarParameterValue(FName("CoronaIntensity"), Intensity);
	}
}
