// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBActor.h"

#include "Components/StaticMeshComponent.h"
#include "Render/ZibraVDBRenderBuffer.h"
#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBMaterialComponent.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBVolume4.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#include "EditorViewportClient.h"
#include "Modules/ModuleManager.h"
#endif

AZibraVDBActor::AZibraVDBActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AssetComponent = CreateDefaultSubobject<UZibraVDBAssetComponent>(TEXT("AssetComponent"));
	MaterialComponent = CreateDefaultSubobject<UZibraVDBMaterialComponent>(TEXT("MaterialComponent"));
	PlaybackComponent = CreateDefaultSubobject<UZibraVDBPlaybackComponent>(TEXT("PlaybackComponent"));
	AddOwnedComponent(AssetComponent);
	AddOwnedComponent(MaterialComponent);
	AddOwnedComponent(PlaybackComponent);
	if (!MaterialComponent->bUseHeterogeneousVolume)
	{
		ReflectionManagerComponent = CreateDefaultSubobject<UZibraVdbReflectionsManagerComponent>(TEXT("ReflectionsManager"));	
		AddOwnedComponent(ReflectionManagerComponent);
	}
	
	AssetComponent->SetZibraVDBMaterialComponent(MaterialComponent);
	AssetComponent->SetZibraVDBPlaybackComponent(PlaybackComponent);
	MaterialComponent->SetZibraVDBAssetComponent(AssetComponent);
	PlaybackComponent->SetZibraVDBAssetComponent(AssetComponent);

	RootComponent = MaterialComponent;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

#if WITH_EDITOR
	EditorStyleSettings = GetDefault<UEditorStyleSettings>();
#endif
}

#if WITH_EDITOR
bool AZibraVDBActor::GetReferencedContentObjects(TArray<UObject*>& Objects) const
{
	Super::GetReferencedContentObjects(Objects);
	AssetComponent->GetReferencedContentObjects(Objects);
	return true;
}
#endif

void AZibraVDBActor::UpdateVisibility() noexcept
{
	FZibraVDBMaterialSceneProxy* Proxy = static_cast<FZibraVDBMaterialSceneProxy*>(MaterialComponent->SceneProxy);
	if (!Proxy)
	{
		return;
	}

	bool bVisible = !IsHidden() && PlaybackComponent->Visible;
	Proxy->SetVisible(bVisible);
	MaterialComponent->UpdateVisibility(bVisible);
}

void AZibraVDBActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateVisibility();
}

#if WITH_EDITOR
bool AZibraVDBActor::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif

void AZibraVDBActor::AttachReflectionManagerComponent()
{
	if (ReflectionManagerComponent)
	{
		return;
	}
	ReflectionManagerComponent = NewObject<UZibraVdbReflectionsManagerComponent>(this);

	if (ReflectionManagerComponent)
	{
		AddOwnedComponent(ReflectionManagerComponent);
		ReflectionManagerComponent->RegisterComponent();
	}
}

void AZibraVDBActor::DetachReflectionManagerComponent()
{
	if (ReflectionManagerComponent)
	{
		RemoveOwnedComponent(ReflectionManagerComponent);
		ReflectionManagerComponent->DestroyComponent();
		ReflectionManagerComponent = nullptr;
	}
}


void AZibraVDBActor::LogVerboseRenderParams()
{
	UE_LOG(LogZibraVDBRuntime, Verbose, TEXT("ZibraVDB Rendering Parameters:\n\
		Density Scale:	%f								\n\
		Scattering Color: %s							\n\
		AbsorptionColor: %s								\n\
		FlameScale: %f									\n\
		FlameColorMode: %s								\n\
		FlameColor: %s									\n\
		TemperatureScale: %f							\n\
		TemperatureColorMode: %s						\n\
		TemperatureColor: %s							\n\
		DirectionalLightBrightness: %f					\n\
		PointLightBrightness: %f						\n\
		SpotLightBrightness: %f							\n\
		IlluminationResolution: %f						\n\
		RayMarchingMainStepSize: %f						\n\
		RayMarchingMaxMainSteps: %d						\n\
		ShadowIntensity: %f								\n\
		EnableDirectionalLightShadows: %d				\n\
		EnablePointLightShadows: %d						\n\
		EnableSpotLightShadows: %d						\n\
		RayMarchingSelfShadowStepSize: %f				\n\
		RayMarchingMaxSelfShadowSteps: %d				\n\
		EnableDirectionalLightReceivedShadows: %d		\n\
		EnablePointLightReceivedShadows: %d				\n\
		EnableSpotLightReceivedShadows: %d				\n\
		AllowSkipDecompression: %d						\n\
		"),
		MaterialComponent->DensityScale, *MaterialComponent->ScatteringColor.ToString(),
		*MaterialComponent->AbsorptionColor.ToString(), MaterialComponent->FlameScale,
		MaterialComponent->FlameColorMode == ColorMode::SolidColor ? TEXT("Solid Color") : TEXT("Gradient"),
		*MaterialComponent->FlameColor.ToString(), MaterialComponent->TemperatureScale,
		MaterialComponent->TemperatureColorMode == ColorMode::SolidColor ? TEXT("Solid Color") : TEXT("Gradient"),
		*MaterialComponent->TemperatureColor.ToString(), MaterialComponent->DirectionalLightBrightness,
		MaterialComponent->PointLightBrightness, MaterialComponent->SpotLightBrightness, ConvertDownscaleFactorToScale(MaterialComponent->IlluminationResolution),
		MaterialComponent->RayMarchingMainStepSize, MaterialComponent->RayMarchingMaxMainSteps, MaterialComponent->ShadowIntensity,
		MaterialComponent->EnableDirectionalLightShadows, MaterialComponent->EnablePointLightShadows,
		MaterialComponent->EnableSpotLightShadows, MaterialComponent->RayMarchingSelfShadowStepSize,
		MaterialComponent->RayMarchingMaxSelfShadowSteps, MaterialComponent->EnableDirectionalLightReceivedShadows,
		MaterialComponent->EnablePointLightReceivedShadows, MaterialComponent->EnableSpotLightReceivedShadows,
		MaterialComponent->AllowSkipDecompression);
}

bool AZibraVDBActor::LogCommonVDBInfo()
{
	if (!AssetComponent || !AssetComponent->HasZibraVDBVolume())
	{
		return false;
	}
	const UZibraVDBVolume4* ZibraVDBBVolume = AssetComponent->GetZibraVDBVolume();

	const FIntVector TextureSize = ZibraVDBBVolume->VDBSize;

	// TODO Decompression add channel names to log
	UE_LOG(LogZibraVDBRuntime, Display, TEXT("Instantiating ZibraVDB %s with %d frames of size %dx%dx%d"),
		*ZibraVDBBVolume->GetName(), ZibraVDBBVolume->GetZibraVDBFile().GetHeader().FrameCount, TextureSize.X, TextureSize.Y,
		TextureSize.Z);
	return true;
}

void AZibraVDBActor::PostLoad()
{
	Super::PostLoad();

	LogCommonVDBInfo();

	LogVerboseRenderParams();
}

void AZibraVDBActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!bCommonInfoLogged)
		bCommonInfoLogged = LogCommonVDBInfo();
}

UZibraVDBAssetComponent* AZibraVDBActor::GetAssetComponent() const noexcept
{
	return AssetComponent;
}
UZibraVDBMaterialComponent* AZibraVDBActor::GetMaterialComponent() const noexcept
{
	return MaterialComponent;
}
UZibraVDBPlaybackComponent* AZibraVDBActor::GetPlaybackComponent() const noexcept
{
	return PlaybackComponent;
}

FBoxSphereBounds AZibraVDBActor::GetBBox() const noexcept
{
	if (!GetMaterialComponent() || !GetMaterialComponent()->GetNativeResources() || !GetMaterialComponent()->SceneProxy)
	{
		return FBoxSphereBounds{};
	}
	FZibraVDBMaterialSceneProxy* Proxy = static_cast<FZibraVDBMaterialSceneProxy*>(MaterialComponent->SceneProxy);

	FVector TextureSize = FVector(GetMaterialComponent()->GetNativeResources()->RenderParameters.TextureSize);
	FVector IndexToUnrealUnitsScale = FVector(Proxy->GetCurrentFrameHeader().IndexToUnrealUnitsScale);

	FVector BBoxSize = TextureSize * IndexToUnrealUnitsScale * GetActorScale3D();
	FVector BBoxMeshRelativePosition = GetMaterialComponent()->BBoxMeshComponent->GetRelativeLocation();
	FVector BBoxCenterPos = GetTransform().TransformPosition(BBoxMeshRelativePosition);

	return FBoxSphereBounds(BBoxCenterPos, BBoxSize, BBoxSize.Size());
}
