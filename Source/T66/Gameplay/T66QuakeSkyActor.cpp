// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66QuakeSkyActor.h"

#include "Kismet/GameplayStatics.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"

namespace
{
	static const TCHAR* QuakeSkyMaterialPath = TEXT("/Game/World/Sky/QuakeCanopy2/MI_QuakeSky_Canopy2.MI_QuakeSky_Canopy2");

	constexpr float QuakeSkyRadius = 30000.f;
	constexpr float QuakeSkyCapAngleDegrees = 55.f;
	constexpr int32 QuakeSkyRadialSegments = 12;
	constexpr int32 QuakeSkyAngularSegments = 48;
}

AT66QuakeSkyActor::AT66QuakeSkyActor()
{
	PrimaryActorTick.bCanEverTick = true;

	CloudCanopy = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CloudCanopy"));
	RootComponent = CloudCanopy;
	CloudCanopy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CloudCanopy->SetGenerateOverlapEvents(false);
	CloudCanopy->SetCanEverAffectNavigation(false);
	CloudCanopy->SetMobility(EComponentMobility::Movable);
	CloudCanopy->SetCastShadow(false);
	CloudCanopy->bCastDynamicShadow = false;
	CloudCanopy->bVisibleInReflectionCaptures = false;
	CloudCanopy->bVisibleInRealTimeSkyCaptures = false;
	CloudCanopy->bAffectDistanceFieldLighting = false;
	CloudCanopy->SetReceivesDecals(false);
	CloudCanopy->bUseAsyncCooking = true;
	CloudCanopy->bUseComplexAsSimpleCollision = false;

	Tags.Add(TEXT("T66QuakeSky"));
}

void AT66QuakeSkyActor::BeginPlay()
{
	Super::BeginPlay();

	BuildCanopyMesh();

	SkyMaterial = LoadObject<UMaterialInterface>(nullptr, QuakeSkyMaterialPath);
	if (SkyMaterial)
	{
		SkyMaterialInstance = UMaterialInstanceDynamic::Create(SkyMaterial, this);
		if (SkyMaterialInstance)
		{
			CloudCanopy->SetMaterial(0, SkyMaterialInstance);
			if (UWorld* World = GetWorld())
			{
				SkyMaterialInstance->SetScalarParameterValue(TEXT("SkyPanTime"), World->GetTimeSeconds());
			}
		}
		else
		{
			CloudCanopy->SetMaterial(0, SkyMaterial);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[QuakeSky] Missing material asset %s"), QuakeSkyMaterialPath);
	}

	UpdateSkyLocation();
}

void AT66QuakeSkyActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (SkyMaterialInstance)
	{
		if (UWorld* World = GetWorld())
		{
			SkyMaterialInstance->SetScalarParameterValue(TEXT("SkyPanTime"), World->GetTimeSeconds());
		}
	}

	UpdateSkyLocation();
}

void AT66QuakeSkyActor::BuildCanopyMesh()
{
	if (!CloudCanopy || CloudCanopy->GetNumSections() > 0)
	{
		return;
	}

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	TArray<FLinearColor> VertexColors;

	const float MaxThetaRadians = FMath::DegreesToRadians(QuakeSkyCapAngleDegrees);
	const float ProjectedRadius = QuakeSkyRadius * FMath::Sin(MaxThetaRadians);
	const int32 RingStride = QuakeSkyAngularSegments + 1;

	Vertices.Reserve((QuakeSkyRadialSegments + 1) * RingStride);
	UVs.Reserve((QuakeSkyRadialSegments + 1) * RingStride);
	Triangles.Reserve(QuakeSkyRadialSegments * QuakeSkyAngularSegments * 6);

	for (int32 RingIndex = 0; RingIndex <= QuakeSkyRadialSegments; ++RingIndex)
	{
		const float RingAlpha = static_cast<float>(RingIndex) / static_cast<float>(QuakeSkyRadialSegments);
		const float Theta = MaxThetaRadians * RingAlpha;
		const float RingRadius = QuakeSkyRadius * FMath::Sin(Theta);
		const float RingHeight = QuakeSkyRadius * FMath::Cos(Theta);

		for (int32 SegmentIndex = 0; SegmentIndex <= QuakeSkyAngularSegments; ++SegmentIndex)
		{
			const float SegmentAlpha = static_cast<float>(SegmentIndex) / static_cast<float>(QuakeSkyAngularSegments);
			const float Phi = SegmentAlpha * 2.f * PI;
			const float X = RingRadius * FMath::Cos(Phi);
			const float Y = RingRadius * FMath::Sin(Phi);

			Vertices.Add(FVector(X, Y, RingHeight));
			UVs.Add(FVector2D(
				0.5f + (X / (ProjectedRadius * 2.f)),
				0.5f + (Y / (ProjectedRadius * 2.f))));
		}
	}

	for (int32 RingIndex = 0; RingIndex < QuakeSkyRadialSegments; ++RingIndex)
	{
		const int32 RingStart = RingIndex * RingStride;
		const int32 NextRingStart = (RingIndex + 1) * RingStride;

		for (int32 SegmentIndex = 0; SegmentIndex < QuakeSkyAngularSegments; ++SegmentIndex)
		{
			const int32 A = RingStart + SegmentIndex;
			const int32 B = RingStart + SegmentIndex + 1;
			const int32 C = NextRingStart + SegmentIndex;
			const int32 D = NextRingStart + SegmentIndex + 1;

			Triangles.Add(A);
			Triangles.Add(C);
			Triangles.Add(B);

			Triangles.Add(B);
			Triangles.Add(C);
			Triangles.Add(D);
		}
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);
	CloudCanopy->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, false);
}

void AT66QuakeSkyActor::UpdateSkyLocation()
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		FVector ViewLocation = FVector::ZeroVector;
		FRotator ViewRotation = FRotator::ZeroRotator;
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
		SetActorLocation(ViewLocation);
	}
}
