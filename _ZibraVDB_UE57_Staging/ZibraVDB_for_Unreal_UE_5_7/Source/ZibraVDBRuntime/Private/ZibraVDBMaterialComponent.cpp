// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBMaterialComponent.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/HeterogeneousVolumeComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveLinearColor.h"
#include "Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Light.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Render/ZibraVDBRenderBuffer.h"
#include "Rendering/SkyAtmosphereCommonData.h"
#include "TextureResource.h"
#include "UObject/Class.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectIterator.h"
#include "ZibraVDBActor.h"
#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBNotifications.h"
#include "ZibraVDBPlaybackComponent.h"
#include "ZibraVDBRuntime.h"
#include "ZibraVDBVolume4.h"

#include <bit>

#if WITH_EDITOR
#include "LevelEditorViewport.h"
#include "PropertyEditorModule.h"
#include "UnrealEdGlobals.h"
#endif

#define LOCTEXT_NAMESPACE "ZibraVDBForUnreal"

UZibraVDBMaterialComponent::UZibraVDBMaterialComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_StartPhysics;
	bTickInEditor = true;

	BBoxMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BBoxMeshComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));

	BBoxMeshComponent->SetFlags(RF_Transient | RF_TextExportTransient);
	BBoxMeshComponent->SetStaticMesh(CubeMesh.Object);
	BBoxMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BBoxMeshComponent->SetSimulatePhysics(false);
	BBoxMeshComponent->SetRelativeScale3D(FVector(1, 1, 1));
	BBoxMeshComponent->SetVisibility(!bUseHeterogeneousVolume);
	BBoxMeshComponent->bCastVolumetricTranslucentShadow = true;

	UClass* HeterogeneousVolumeComponentClass = nullptr;
#if WITH_EDITOR && (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 504
	static ConstructorHelpers::FObjectFinder<UBlueprint> HeterogeneousVolumeComponentWrapperFinder(
		TEXT("/ZibraVDB/Blueprints/HeterogeneousComponentWrapper.HeterogeneousComponentWrapper"));
	if (HeterogeneousVolumeComponentWrapperFinder.Succeeded())
	{
		HeterogeneousVolumeComponentClass = HeterogeneousVolumeComponentWrapperFinder.Object->GeneratedClass;
	}
	else
	{
		UE_LOG(
			LogZibraVDBRuntime, Error, TEXT("UZibraVDBMaterialComponent: HeterogeneousComponentWrapper Blueprint is not found."));
		return;
	}
#else
	HeterogeneousVolumeComponentClass = UHeterogeneousVolumeComponent::StaticClass();
#endif

	VolumeComponent = static_cast<UHeterogeneousVolumeComponent*>(CreateDefaultSubobject(TEXT("UHeterogeneousVolumeComponent"),
		HeterogeneousVolumeComponentClass, HeterogeneousVolumeComponentClass, /*bIsRequired =*/true, false));
	VolumeComponent->SetFlags(RF_Transient | RF_TextExportTransient);
	VolumeComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VolumeComponent->SetSimulatePhysics(false);
	VolumeComponent->SetVisibility(bUseHeterogeneousVolume);
	// For UE5.3 bPivotAtCentroid is true by default.
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
	VolumeComponent->bPivotAtCentroid = true;
#endif
	ApplyHVParams();

#if WITH_EDITORONLY_DATA
	BBoxMeshComponent->SetIsVisualizationComponent(true);	 // This hides BBoxMeshComponent in sequencer
#endif

	// Find the ZibraVDBBBoxRender UMaterialInterface.
	Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/ZibraVDB/Materials/ZibraVDBBBoxRender"), nullptr, LOAD_None, nullptr);
	if (!Material)
	{
		UE_LOG(LogZibraVDBRuntime, Error, TEXT("ZibraVDBBBoxRender UMaterialInterface is not found!"));
	}
	ScatteringColorGradient =
		LoadObject<UCurveLinearColor>(nullptr, TEXT("/ZibraVDB/Materials/DefaultDensityColor"), nullptr, LOAD_None, nullptr);
	if (!ScatteringColorGradient)
	{
		UE_LOG(LogZibraVDBRuntime, Error, TEXT("DefaultScatteringColorGradient UMaterialInterface is not found!"));
	}
	FlameColorGradient =
		LoadObject<UCurveLinearColor>(nullptr, TEXT("/ZibraVDB/Materials/DefaultFlameColor"), nullptr, LOAD_None, nullptr);
	if (!FlameColorGradient)
	{
		UE_LOG(LogZibraVDBRuntime, Error, TEXT("DefaultFlameColorGradient UMaterialInterface is not found!"));
	}
	TemperatureColorGradient =
		LoadObject<UCurveLinearColor>(nullptr, TEXT("/ZibraVDB/Materials/DefaultTemperatureColor"), nullptr, LOAD_None, nullptr);
	if (!TemperatureColorGradient)
	{
		UE_LOG(LogZibraVDBRuntime, Error, TEXT("DefaultTemperatureColorGradient UMaterialInterface is not found!"));
	}

	VolumeMaterial =
		LoadObject<UMaterialInterface>(nullptr, TEXT("/ZibraVDB/Materials/ZibraVDBVolume"), nullptr, LOAD_None, nullptr);
	if (!VolumeMaterial)
	{
		UE_LOG(LogZibraVDBRuntime, Error, TEXT("ZibraVDBVolume UMaterialInterface is not found!"));
	}
}

void UZibraVDBMaterialComponent::SetZibraVDBAssetComponent(UZibraVDBAssetComponent* InZibraVDBAssetComponent)
{
	check(InZibraVDBAssetComponent != nullptr);

	ZibraVDBAssetComponent = InZibraVDBAssetComponent;
	ZibraVDBAssetComponent->OnFrameChanged.AddUObject(this, &UZibraVDBMaterialComponent::UpdateFrameIndex);

	OnVolumeChanged();
}

void UZibraVDBMaterialComponent::SetDensityScale(float NewDensityScale)
{
	DensityScale = NewDensityScale;
}

void UZibraVDBMaterialComponent::SetAbsorptionColor(FLinearColor NewAbsorptionColor)
{
	AbsorptionColor = NewAbsorptionColor;
}

void UZibraVDBMaterialComponent::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	ScatteringColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
	ScatteringColorTexture->UpdateResource();
	TemperatureColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
	TemperatureColorTexture->UpdateResource();
	FlameColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
	FlameColorTexture->UpdateResource();

	ApplyHVParams();

	// https://forums.unrealengine.com/t/ue5-template-mismatch-during-attaching-of-instanced-component/1510733/2
	// TODO: BUGBUG: Work around the 'Template Mismatch during attachment. Attaching instanced component to template component.'
	// problem.
	// Remove after Epic acknowledges and fixes this long-standing bug or provides guidance.
	// The constructor-initialized components, upon non-CDO instantiation/initialization, are getting CDO references for their
	// attached parents, instead of instance references. A work-around...
	//     - Use SetupAttachment(), per usual in the constructor to establish the parent-child relationship for CDOs.
	//     - Use AttachToComponent() to override those errant CDO references with instance references, in your
	//     USceneComponent::OnRegister() override.

	BBoxMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	VolumeComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
}

void UZibraVDBMaterialComponent::PostReinitProperties()
{
	Super::PostReinitProperties();

	if (ScatteringColorTexture == nullptr)
	{
		ScatteringColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
		ScatteringColorTexture->UpdateResource();
	}

	if (TemperatureColorTexture == nullptr)
	{
		TemperatureColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
		TemperatureColorTexture->UpdateResource();
	}
	if (FlameColorTexture == nullptr)
	{
		FlameColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
		FlameColorTexture->UpdateResource();
	}
}

void UZibraVDBMaterialComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (ScatteringColorTexture == nullptr)
	{
		ScatteringColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
		ScatteringColorTexture->UpdateResource();
	}

	if (TemperatureColorTexture == nullptr)
	{
		TemperatureColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
		TemperatureColorTexture->UpdateResource();
	}
	if (FlameColorTexture == nullptr)
	{
		FlameColorTexture = UTexture2D::CreateTransient(GRADIENT_TEXTURE_SIZE, 1, PF_FloatRGBA);
		FlameColorTexture->UpdateResource();
	}
}

#if WITH_EDITORONLY_DATA
void UZibraVDBMaterialComponent::PostLoad()
{
	Super::PostLoad();

	if (!EnableAmbientLighting_DEPRECATED)
	{
		check(!IsTemplate());	 // CDO should never trigger upgrade flow
		AmbientLightingMode = EAmbientLightingMode::Disabled;
		EnableAmbientLighting_DEPRECATED = true;
#if WITH_EDITOR
		MarkPackageDirty();
#endif
	}
}
#endif

void UZibraVDBMaterialComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	if (Material)
	{
		OutMaterials.Add(Material);
	}
}
void UZibraVDBMaterialComponent::OnRegister()
{
	Super::OnRegister();

	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		BBoxMeshComponent = nullptr;
		VolumeComponent = nullptr;
		return;
	}

	bIsRegistered = true;

	InitResources();
}

void UZibraVDBMaterialComponent::OnUnregister()
{
	Super::OnUnregister();

	bIsRegistered = false;
	UpdateGlobalSet();
}

FString UZibraVDBMaterialComponent::ConstructRenderingResourcesMapKey() const noexcept
{
	FString Name;

	AActor* ZibraVDBActor = GetOwner();
	check(ZibraVDBActor);

	// Key should be persistent during BP lifetime when ZibraVDBActor is a ChildActor
	// to not reinitialize GPU resources on every interaction with this BP
	if (ZibraVDBActor->GetParentComponent())
	{
		Name = ZibraVDBActor->GetParentComponent()->GetName() + ZibraVDBActor->GetParentComponent()->GetOwner()->GetName();
	}
	else
	{
		Name = ZibraVDBActor->GetName();
	}

	Name += ZibraVDBAssetComponent->GetZibraVDBVolume()->GetName();
	return Name;
}

bool UZibraVDBMaterialComponent::IsVolumeValid()
{
	return ZibraVDBAssetComponent && ZibraVDBAssetComponent->HasZibraVDBVolume() &&
		   ZibraVDBAssetComponent->GetZibraVDBVolume()->GetZibraVDBFile().HasBinaryData();
}

bool UZibraVDBMaterialComponent::IsReadyForRender()
{
	return bIsRegistered && bIsResourcesValid;
}

void UZibraVDBMaterialComponent::UpdateGlobalSet()
{
	if (IsReadyForRender())
	{
		FZibraVDBRuntimeModule::GetZibraRenderExtension()->ActiveZibraVDBs.Add(this);
	}
	else
	{
		FZibraVDBRuntimeModule::GetZibraRenderExtension()->ActiveZibraVDBs.Remove(this);
	}
}

void UZibraVDBMaterialComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	FlushRenderingCommands();

	DecompressorManager.Release();

	AActor* ZibraVDBActor = GetOwner();

	// Do not free GPU resources if ZibraVDBActor is a BP ChildActor and being reregistered
	// when interacting with that BP
	if (ZibraVDBActor && ZibraVDBActor->GetParentComponent() &&
		!ZibraVDBActor->GetParentComponent()->GetOwner()->IsActorBeingDestroyed())
	{
		return;
	}

	ZibraVDBRenderingResources.Reset();

	if (ZibraVDBAssetComponent && ZibraVDBAssetComponent->HasZibraVDBVolume())
	{
		UZibraVDBRenderingResourcesSubsystem* RRS = GEngine->GetEngineSubsystem<UZibraVDBRenderingResourcesSubsystem>();

		if (!RRS)
		{
			return;
		}

		RRS->RemoveIfInvalid(RenderingResourcesMapKey);
	}
}

void UZibraVDBMaterialComponent::UpdateChannelTextures() noexcept
{
	if (!IsReadyForRender())
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(UpdateChannelTextures)
	(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			constexpr int32 TextureComponentCount = 4;

			using LambdaType = TFunction<FLinearColor(float)>;
			using PairType = TPair<LambdaType, UTexture2D*>;
			PairType TexturesToUpdate[] = {
				{[this](const float U) -> FLinearColor
					{
						return ScatteringColorMode == ColorMode::SolidColor ? ScatteringColor
																			: ScatteringColorGradient->GetLinearColorValue(U);
					},
					ScatteringColorTexture},
				{[this](const float U) -> FLinearColor
					{ return FlameColorMode == ColorMode::SolidColor ? FlameColor : FlameColorGradient->GetLinearColorValue(U); },
					FlameColorTexture},
				{[this](const float U) -> FLinearColor
					{
						return TemperatureColorMode == ColorMode::SolidColor ? TemperatureColor
																			 : TemperatureColorGradient->GetLinearColorValue(U);
					},
					TemperatureColorTexture},

			};
			for (const auto& [FetchItemColorLambda, TextureToUpdate] : TexturesToUpdate)
			{
				if (!TextureToUpdate || !TextureToUpdate->GetResource() ||
					TextureToUpdate->HasPendingRenderResourceInitialization() || !TextureToUpdate->GetResource()->GetTexture2DRHI())
				{
					continue;
				}

				TArray<FFloat16> ColorData;
				ColorData.Init(0, GRADIENT_TEXTURE_SIZE * TextureComponentCount);
				for (int Index = 0; Index < GRADIENT_TEXTURE_SIZE; Index++)
				{
					const float U = Index / static_cast<float>(GRADIENT_TEXTURE_SIZE - 1);
					const FLinearColor Color = FetchItemColorLambda(U);
					ColorData[Index * TextureComponentCount + 0] = Color.R;
					ColorData[Index * TextureComponentCount + 1] = Color.G;
					ColorData[Index * TextureComponentCount + 2] = Color.B;
					ColorData[Index * TextureComponentCount + 3] = Color.A;
				}
				FUpdateTextureRegion2D UpdateTextureRegion(0, 0, 0, 0, GRADIENT_TEXTURE_SIZE, 1);
				RHICmdList.UpdateTexture2D(TextureToUpdate->GetResource()->GetTexture2DRHI(), 0, UpdateTextureRegion,
					GRADIENT_TEXTURE_SIZE * TextureComponentCount * sizeof(FFloat16),
					reinterpret_cast<uint8*>(ColorData.GetData()));
			}
		});
}

void UZibraVDBMaterialComponent::UpdateRelativeLocation()
{
	// Do NOT use Proxy here
	// You will introduce race condition

	const uint32 CurrentFrameIndex = ZibraVDBAssetComponent->GetCurrentFrame();
	if (CurrentFrameIndex >= static_cast<uint32>(ZibraVDBRenderingResources->ZibraVDBNativeResources->FrameHeaders.Num()))
	{
		return;
	}
	const auto& FrameHeader = ZibraVDBRenderingResources->ZibraVDBNativeResources->FrameHeaders[CurrentFrameIndex];

	const FVector TextureSize = FVector(ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.TextureSize);
	const FVector FrameSize = FVector(FrameHeader.FrameSize);

	const FVector IndexToWorldScale = FVector(FrameHeader.IndexToWorldScale);
	const FVector IndexToUnrealScale = FVector(FrameHeader.IndexToUnrealUnitsScale);

	const FVector FrameSizeAdjustmentVoxels = (FrameSize - TextureSize) / 2.0f;
	const FVector FrameSizeAdjustment = IndexToUnrealScale * FrameSizeAdjustmentVoxels;
	const FVector4f FrameSizeAdjustmentRotated = FrameHeader.IndexToWorldRotation.TransformFVector4(
		FVector4f(FrameSizeAdjustment.X, FrameSizeAdjustment.Y, FrameSizeAdjustment.Z, 1.0f));
	const FVector FrameSizeAdjustmentRotated3 =
		FVector(FrameSizeAdjustmentRotated.X, FrameSizeAdjustmentRotated.Y, FrameSizeAdjustmentRotated.Z);

	const FVector FrameTranslation = FVector(FrameHeader.IndexToWorldTranslationInVoxels);

	const FVector TranslationInVoxels =
		FrameTranslation + FVector(FrameHeader.IndexToWorldRotation.Rotator().RotateVector(FVector3f(TextureSize) / 2.f));
	const FVector TranslationInUnrealUnits = TranslationInVoxels * FVector(FrameHeader.IndexToUnrealUnitsScale);

	FVector Location = FVector(TranslationInUnrealUnits) + FrameSizeAdjustmentRotated3;
	FQuat Rotation(FrameHeader.IndexToWorldRotation.ToQuat().GetNormalized());
	auto Scale = FrameSize * IndexToWorldScale;

	if (BBoxMeshComponent)
	{
		BBoxMeshComponent->SetRelativeLocation(Location);
		BBoxMeshComponent->SetRelativeRotation(Rotation);
		BBoxMeshComponent->SetRelativeScale3D(Scale.GetAbs());
	}

	if (VolumeComponent)
	{
		VolumeComponent->SetRelativeLocation(TranslationInUnrealUnits);
		VolumeComponent->SetRelativeRotation(Rotation);
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
		VolumeComponent->SetVolumeResolution(FIntVector(TextureSize));
#else
		VolumeComponent->VolumeResolution = FIntVector(TextureSize);
#endif
		VolumeComponent->SetRelativeScale3D(IndexToUnrealScale);
	}
}

void UZibraVDBMaterialComponent::UpdateRayMarchMaterialParams()
{
	if (!ZibraVDBRenderingResources)
	{
		return;
	}

	if (!bUseHeterogeneousVolume)
	{
		UMaterialInstanceDynamic* MaterialInstance = UpdateMaterialInstance();
		MaterialInstance->SetScalarParameterValue("DensityScale", GetDensityScaleInternal());
		MaterialInstance->SetVectorParameterValue("AbsorptionColor", AbsorptionColor);
		MaterialInstance->SetScalarParameterValue("NearPlane", GNearClippingPlane);
		MaterialInstance->SetScalarParameterValue("StepSize", RayMarchingMainStepSize);
		MaterialInstance->SetScalarParameterValue("StepCount", RayMarchingMaxMainSteps);
		MaterialInstance->SetScalarParameterValue("ProjectedShadowsStepCount", RayMarchingMaxProjectedShadowSteps);
		MaterialInstance->SetScalarParameterValue("ProjectedShadowIntensity", ProjectedShadowsIntensity);
		MaterialInstance->SetVectorParameterValue(
			"LightMask", FVector3f(EnableDirectionalLightProjectedShadows, EnablePointAndSpotLightProjectedShadows, 1));
		// Max number of steps inside a block (diagonal)
		MaterialInstance->SetScalarParameterValue(
			"BlockSteps", FMath::CeilToInt(ZIBRAVDB_BLOCK_SIZE * FMath::Sqrt(3.0f) / RayMarchingMainStepSize));
		MaterialInstance->SetScalarParameterValue(
			"EnableInterpolation", (RayMarchingFiltering == ERayMarchingFiltering::Trilinear) ? 1 : 0);
		MaterialInstance->SetScalarParameterValue("VolumeResolutionScale", ConvertDownscaleFactorToScale(VolumeResolution));
		MaterialInstance->SetScalarParameterValue("DepthFadeDistance", DepthFadeDistance);
		MaterialInstance->SetScalarParameterValue("DepthFadeOpacity", DepthFadeOpacity);
		MaterialInstance->SetScalarParameterValue("EnableDepthFade", bEnableDepthFade);
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 506
		MaterialInstance->SetScalarParameterValue("EnableFirstPersonRendering", (VDBFirstPersonPrimitiveType == EZibraFirstPersonPrimitiveType::FirstPerson) ? 1.0f : 0.0f);
#endif
	}
	else
	{
		UMaterialInstanceDynamic* MaterialInstance = UpdateHVMaterialInstance();
		MaterialInstance->SetScalarParameterValue("DensityScale", DensityScale);
		MaterialInstance->SetVectorParameterValue("ScatteringColor", ScatteringColor);
		MaterialInstance->SetScalarParameterValue(
			"EnableInterpolation", (RayMarchingFiltering == ERayMarchingFiltering::Trilinear) ? 1 : 0);
		MaterialInstance->SetScalarParameterValue("VolumeResolutionScale", ConvertDownscaleFactorToScale(VolumeResolution));
	}
}

void UZibraVDBMaterialComponent::UpdateBBoxMeshProperties()
{
	BBoxMeshComponent->SetLightingChannels(
		VDBLightingChannels.bChannel0, VDBLightingChannels.bChannel1, VDBLightingChannels.bChannel2);
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 506
	BBoxMeshComponent->SetFirstPersonPrimitiveType(static_cast<EFirstPersonPrimitiveType>(VDBFirstPersonPrimitiveType));
#endif
}

UMaterialInstanceDynamic* UZibraVDBMaterialComponent::UpdateMaterialInstance(bool isDirty /*= false*/)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(BBoxMeshComponent->GetMaterial(0));

	if (!MaterialInstance || MaterialInstance->GetBaseMaterial() != Material)
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(Material, BBoxMeshComponent);
		MaterialInstance->SetFlags(RF_Transient | RF_TextExportTransient);
		MaterialInstance->SetFlags(RF_Transient | RF_TextExportTransient);
		BBoxMeshComponent->SetMaterial(0, MaterialInstance);
		BBoxMeshComponent->SetVisibility(!bUseHeterogeneousVolume);
		isDirty = true;
	}

	if (isDirty)
	{
		const FIntVector BlockGridSize = ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.BlockGridSize;
		const FIntVector TextureSize = ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.TextureSize;
		const FVector3f GridSize = FVector3f(TextureSize);

		MaterialInstance->SetTextureParameterValue("SparseBlocksRGB", ZibraVDBRenderingResources->SparseBlocksRGB);
		MaterialInstance->SetTextureParameterValue("SparseBlocksDensity", ZibraVDBRenderingResources->SparseBlocksDensity);
		MaterialInstance->SetTextureParameterValue(
			"MaxDensityPerBlockTexture", ZibraVDBRenderingResources->MaxDensityPerBlockTexture);
		MaterialInstance->SetTextureParameterValue("BlockIndexBuffer", ZibraVDBRenderingResources->BlockIndexVolumeTexture);
		MaterialInstance->SetTextureParameterValue("WorldToBoxTransform", ZibraVDBRenderingResources->TransformTextureResult);
		MaterialInstance->SetVectorParameterValue("GridSize", GridSize);
		MaterialInstance->SetVectorParameterValue("BlockGridSize", FVector3f(BlockGridSize));
		MaterialInstance->SetScalarParameterValue(
			"SparseBlockGridSize", ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRT);
		// Bitcast SparseBlockGridSizeMultiply from float to uint in order to not lose any presision.
		MaterialInstance->SetScalarParameterValue("SparseBlockGridSizeMultiply",
			std::bit_cast<float>(
				ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTMultiply));
		MaterialInstance->SetScalarParameterValue("SparseBlockGridSizeShift1",
			ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTShift1);
		MaterialInstance->SetScalarParameterValue("SparseBlockGridSizeShift2",
			ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTShift2);

		// Ensures that texture bindings are up to date. Movie Render Queue fix
		MaterialInstance->EnsureIsComplete();
	}

	return MaterialInstance;
}

UMaterialInstanceDynamic* UZibraVDBMaterialComponent::UpdateHVMaterialInstance(bool isDirty /*= false*/)
{
	UMaterialInstanceDynamic* MaterialInstance = VolumeComponent->MaterialInstanceDynamic;

	if (!MaterialInstance || MaterialInstance->Parent != VolumeMaterial)
	{
		VolumeComponent->SetMaterial(0, VolumeMaterial);
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 504
		// In UE5.4 we don't need to do that, since they added overload for SetMaterial, which automatically creates instance from
		// Material and sets this MaterialInstanceDynamic field.
		VolumeComponent->MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(VolumeComponent->GetMaterial(0), nullptr);
#endif
		MaterialInstance = VolumeComponent->MaterialInstanceDynamic;
		VolumeComponent->SetVisibility(bUseHeterogeneousVolume);
		isDirty = true;
	}

	if (isDirty)
	{
		const FIntVector BlockGridSize = ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.BlockGridSize;
		const FIntVector TextureSize = ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.TextureSize;
		const FVector3f GridSize = FVector3f(TextureSize);

		MaterialInstance->SetTextureParameterValue("SparseBlocksRGB", ZibraVDBRenderingResources->SparseBlocksRGB);
		MaterialInstance->SetTextureParameterValue("SparseBlocksDensity", ZibraVDBRenderingResources->SparseBlocksDensity);
		MaterialInstance->SetTextureParameterValue(
			"MaxDensityPerBlockTexture", ZibraVDBRenderingResources->MaxDensityPerBlockTexture);
		MaterialInstance->SetTextureParameterValue("BlockIndexBuffer", ZibraVDBRenderingResources->BlockIndexVolumeTexture);
		MaterialInstance->SetTextureParameterValue("WorldToBoxTransform", ZibraVDBRenderingResources->TransformTextureResult);
		MaterialInstance->SetTextureParameterValue("ScatteringColorTexture", ScatteringColorTexture);
		MaterialInstance->SetVectorParameterValue("GridSize", GridSize);
		MaterialInstance->SetVectorParameterValue("BlockGridSize", FVector3f(BlockGridSize));
		MaterialInstance->SetScalarParameterValue(
			"SparseBlockGridSize", ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRT);
		// Bitcast SparseBlockGridSizeMultiply from float to uint in order to not lose any presision.
		MaterialInstance->SetScalarParameterValue("SparseBlockGridSizeMultiply",
			std::bit_cast<float>(
				ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTMultiply));
		MaterialInstance->SetScalarParameterValue("SparseBlockGridSizeShift1",
			ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTShift1);
		MaterialInstance->SetScalarParameterValue("SparseBlockGridSizeShift2",
			ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTShift2);
	}

	return MaterialInstance;
}

void UZibraVDBMaterialComponent::UpdateHV()
{
	AZibraVDBActor* ZibraVDBActorParent = Cast<AZibraVDBActor>(GetOuter());
	bool bHVStatusChanged = bLastFrameHVStatus != bUseHeterogeneousVolume;
	if (ZibraVDBActorParent && bHVStatusChanged)
	{
		if (bUseHeterogeneousVolume)
		{
			ZibraVDBActorParent->DetachReflectionManagerComponent();
		}
		else
		{
			ZibraVDBActorParent->AttachReflectionManagerComponent();
		}

		bLastFrameHVStatus = bUseHeterogeneousVolume;
	}

	if (bUseHeterogeneousVolume)
	{
		ApplyHVParams();
	}
}

void UZibraVDBMaterialComponent::TickComponent(
	float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ZibraVDBAssetComponent || !ZibraVDBAssetComponent->HasZibraVDBVolume() || !Material || !SceneProxy)
	{
		return;
	}

	UpdateRelativeLocation();

	// We update params every frame to keep up with non-editor parameters updates(ex. in sequencer)
	UpdateSceneProxyRenderParams();
	UpdateRayMarchMaterialParams();
	UpdateChannelTextures();
	UpdateTranslucentSort();
	UpdateRenderingTextures();
	UpdateHV();
	UpdateBBoxMeshProperties();
}

void UZibraVDBMaterialComponent::UpdateTranslucentSort()
{
	BBoxMeshComponent->SetTranslucentSortPriority(VolumeTranslucencySortPriority);
	BBoxMeshComponent->SetTranslucencySortDistanceOffset(VolumeTranslucencySortDistanceOffset);
}

FPrimitiveSceneProxy* UZibraVDBMaterialComponent::CreateSceneProxy()
{
	if (!Material || !ZibraVDBAssetComponent || !ZibraVDBAssetComponent->HasZibraVDBVolume() || !ZibraVDBRenderingResources ||
		!GetSparseBlocksRGB()->GetResource() || !GetBlockIndexTexture()->GetResource() || !bIsResourcesValid)
	{
		return nullptr;
	}

	return new FZibraVDBMaterialSceneProxy(ZibraVDBAssetComponent, this);
}

bool UZibraVDBMaterialComponent::SupportsStaticLighting() const
{
	return false;
}

int32 UZibraVDBMaterialComponent::GetNumMaterials() const
{
	return (Material) ? 1 : 0;
}

UMaterialInterface* UZibraVDBMaterialComponent::GetMaterial(int32 Index) const
{
	return Material;
}

void UZibraVDBMaterialComponent::SetMaterial(int32 ElementIndex, class UMaterialInterface* InMaterial)
{
	if (InMaterial != Material)
	{
		Material = InMaterial;
		MarkRenderStateDirty();
	}
}

void UZibraVDBMaterialComponent::UpdateSceneProxyRenderParams() noexcept
{
	FZibraVDBMaterialSceneProxy* ZibraVdbMaterialSceneProxy = static_cast<FZibraVDBMaterialSceneProxy*>(SceneProxy);
	if (!ZibraVdbMaterialSceneProxy)
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(UpdateRenderParams)
	([=](FRHICommandList& RHICmdList) noexcept { ZibraVdbMaterialSceneProxy->UpdateRenderParams(); });
}

void UZibraVDBMaterialComponent::UpdateFrameIndex(const uint32 FrameIndex) noexcept
{
	FZibraVDBMaterialSceneProxy* Proxy = static_cast<FZibraVDBMaterialSceneProxy*>(SceneProxy);
	if (Proxy == nullptr)
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(UpdateFrameIndex)
	([=](auto&) { Proxy->UpdateFrameIndex(FrameIndex); });
}

void UZibraVDBMaterialComponent::UpdateResetDecompressedFrameIndex() noexcept
{
	FZibraVDBMaterialSceneProxy* Proxy = static_cast<FZibraVDBMaterialSceneProxy*>(SceneProxy);
	if (Proxy == nullptr)
	{
		return;
	}
	ENQUEUE_RENDER_COMMAND(UpdateResetDecompressedFrameIndex)
	([=](auto&) { Proxy->ResetDecompressedFrame(); });
}

void UZibraVDBMaterialComponent::UpdateRenderTexturesUAV() noexcept
{
	FZibraVDBMaterialSceneProxy* Proxy = static_cast<FZibraVDBMaterialSceneProxy*>(SceneProxy);
	if (Proxy == nullptr)
	{
		return;
	}

	Proxy->InitRenderTexturesSRVsUAVs();
}

void UZibraVDBMaterialComponent::OnVolumeChanged() noexcept
{
	ReleaseResources();
	InitResources();
}

void UZibraVDBMaterialComponent::UpdateRenderingTextures()
{
	ZibraVDBRenderingResources->UpdateIlluminatedVolumeTexture(*this);
	ZibraVDBRenderingResources->UpdateSparseBlocksRGB(*this);
	ZibraVDBRenderingResources->UpdateSparseBlocksDensity(*this);
	ZibraVDBRenderingResources->UpdateMaxDensityPerBlockTexture(*this);
}

bool UZibraVDBMaterialComponent::VisibleInReflections() const noexcept
{
	const AZibraVDBActor* ZibraVDBActor = Cast<AZibraVDBActor>(GetOwner());

	if (!ZibraVDBActor || !ZibraVDBActor->ReflectionManagerComponent)
	{
		return false;
	}

	return !ZibraVDBActor->ReflectionManagerComponent->ActorsToApplyReflections.IsEmpty();
}

FZibraTexture2DRHIRef UZibraVDBMaterialComponent::GetScatteringColorTexture() const noexcept
{
	if (!ScatteringColorTexture || !ScatteringColorTexture->GetResource() ||
		ScatteringColorTexture->HasPendingRenderResourceInitialization() || !ScatteringColorTexture->GetResource()->GetTextureRHI())
	{
		return FZibraTexture2DRHIRef{};
	}
	return ScatteringColorTexture->GetResource()->GetTextureRHI();
}

FZibraTexture2DRHIRef UZibraVDBMaterialComponent::GetTemperatureColorTexture() const noexcept
{
	if (!TemperatureColorTexture || !TemperatureColorTexture->GetResource() ||
		TemperatureColorTexture->HasPendingRenderResourceInitialization() ||
		!TemperatureColorTexture->GetResource()->GetTextureRHI())
	{
		return FZibraTexture2DRHIRef{};
	}
	return TemperatureColorTexture->GetResource()->GetTextureRHI();
}

FZibraTexture2DRHIRef UZibraVDBMaterialComponent::GetFlameColorTexture() const noexcept
{
	if (!FlameColorTexture || !FlameColorTexture->GetResource() || FlameColorTexture->HasPendingRenderResourceInitialization() ||
		!FlameColorTexture->GetResource()->GetTextureRHI())
	{
		return FZibraTexture2DRHIRef{};
	}
	return FlameColorTexture->GetResource()->GetTextureRHI();
}

const FZibraVDBDecompressorManager& UZibraVDBMaterialComponent::GetDecompressorManager() const noexcept
{
	return DecompressorManager;
}

TSharedPtr<FZibraVDBRenderingResources>& UZibraVDBMaterialComponent::GetRenderingResources() noexcept
{
	return ZibraVDBRenderingResources;
}

const TSharedPtr<FZibraVDBRenderingResources>& UZibraVDBMaterialComponent::GetRenderingResources() const noexcept
{
	return ZibraVDBRenderingResources;
}

TSharedPtr<FZibraVDBNativeResource> UZibraVDBMaterialComponent::GetNativeResources() noexcept
{
	return ZibraVDBRenderingResources ? ZibraVDBRenderingResources->ZibraVDBNativeResources : nullptr;
}

const TSharedPtr<FZibraVDBNativeResource> UZibraVDBMaterialComponent::GetNativeResources() const noexcept
{
	return ZibraVDBRenderingResources ? ZibraVDBRenderingResources->ZibraVDBNativeResources : nullptr;
}

float UZibraVDBMaterialComponent::GetDensityScaleInternal() const noexcept
{
	constexpr float Multiplier = 1.5f;
	return DensityScale * Multiplier;
}

float UZibraVDBMaterialComponent::GetFlameScaleInternal() const noexcept
{
	constexpr float Multiplier = 4;
	return FlameScale * Multiplier;
}

float UZibraVDBMaterialComponent::GetTemperatureScaleInternal() const noexcept
{
	constexpr float Multiplier = 4;
	return TemperatureScale * Multiplier;
}

float UZibraVDBMaterialComponent::GetShadowIntensityInternal() const noexcept
{
	constexpr float MultiplierZibra = 10;
	constexpr float MultiplierHV = 4;
	return ShadowIntensity * (bUseHeterogeneousVolume ? MultiplierHV : MultiplierZibra);
}

FPrimitiveComponentId UZibraVDBMaterialComponent::GetBBoxMeshPrimitiveComponentId() const noexcept
{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
	return BBoxMeshComponent->GetPrimitiveSceneId();
#else
	return BBoxMeshComponent->ComponentId;
#endif
}

TObjectPtr<UTextureRenderTargetVolume> UZibraVDBMaterialComponent::GetSparseBlocksRGB() const noexcept
{
	return ZibraVDBRenderingResources ? ZibraVDBRenderingResources->SparseBlocksRGB : nullptr;
}

TObjectPtr<UTextureRenderTargetVolume> UZibraVDBMaterialComponent::GetSparseBlocksDensity() const noexcept
{
	return ZibraVDBRenderingResources ? ZibraVDBRenderingResources->SparseBlocksDensity : nullptr;
}

TObjectPtr<UTextureRenderTargetVolume> UZibraVDBMaterialComponent::GetMaxDensityPerBlockTexture() const noexcept
{
	return ZibraVDBRenderingResources ? ZibraVDBRenderingResources->MaxDensityPerBlockTexture : nullptr;
}

TObjectPtr<UTextureRenderTargetVolume> UZibraVDBMaterialComponent::GetBlockIndexTexture() const noexcept
{
	return ZibraVDBRenderingResources ? ZibraVDBRenderingResources->BlockIndexVolumeTexture : nullptr;
}

TObjectPtr<UTextureRenderTargetVolume> UZibraVDBMaterialComponent::GetTransformBuffer() const noexcept
{
	return ZibraVDBRenderingResources ? ZibraVDBRenderingResources->TransformTextureResult : nullptr;
}

TObjectPtr<UTextureRenderTargetVolume> UZibraVDBMaterialComponent::GetIlluminatedVolumeTexture() const noexcept
{
	return ZibraVDBRenderingResources ? ZibraVDBRenderingResources->IlluminatedVolumeTexture : nullptr;
}

void UZibraVDBMaterialComponent::UpdateVisibility(bool bIsVisible)
{
	if (VolumeComponent)
	{
		VolumeComponent->SetVisibility(bIsVisible && bUseHeterogeneousVolume);
	}
	if (BBoxMeshComponent)
	{
		BBoxMeshComponent->SetVisibility(bIsVisible && !bUseHeterogeneousVolume);
	}
}

void UZibraVDBMaterialComponent::ApplyHVParams()
{
	if (!VolumeComponent)
	{
		return;
	}
	VolumeComponent->LightingChannels = VDBLightingChannels;
	VolumeComponent->CastShadow = HVCastShadows;
	VolumeComponent->bCastVolumetricTranslucentShadow = HVCastShadows;
	VolumeComponent->StepFactor = HVStepFactor;
	VolumeComponent->ShadowStepFactor = HVShadowStepFactor;
	VolumeComponent->ShadowBiasFactor = HVShadowBiasFactor;
	VolumeComponent->LightingDownsampleFactor = HVLightingDownsampleFactor;
	VolumeComponent->SetEmissiveLightSource(HVEmissiveLightSource);
	VolumeComponent->MarkRenderStateDirty();
}

void UZibraVDBMaterialComponent::InitResources()
{
	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		return;
	}

	// Don't recreate resources automatically if they already valid
	if (bIsResourcesValid)
	{
		return;
	}

	// Don't create resources yet, defer until OnRegister (if it ever happens)
	if (!bIsRegistered)
	{
		return;
	}

	if (!IsVolumeValid())
	{
		return;
	}

	RenderingResourcesMapKey = ConstructRenderingResourcesMapKey();

	UZibraVDBRenderingResourcesSubsystem* RRS = GEngine->GetEngineSubsystem<UZibraVDBRenderingResourcesSubsystem>();

	const auto& ZibraVDBFile = ZibraVDBAssetComponent->GetZibraVDBVolume()->GetZibraVDBFile();
	if (!bRayMarchingFilteringInitialized)
	{
		RayMarchingFiltering = (ZibraVDBFile.GetHeader().MaxSpatialBlockCount <= ZIBRAVDB_MAX_BLOCKS_FOR_RAYMARCHING_INTERPOLATION)
								   ? RayMarchingFiltering
								   : ERayMarchingFiltering::None;
		bRayMarchingFilteringInitialized = true;
	}

	auto ReturnCode = DecompressorManager.Initialize(ZibraVDBFile.GetBinaryData());
	if (ReturnCode != Zibra::CE::ZCE_SUCCESS)
	{
		FZibraVDBNotifications::AddNotification(
			FText::Format(LOCTEXT("DecompressorInitializationFailure", "Failed to initialize decompressor: {0}"),
				FText::FromString(FZibraVDBSDKIntegration::ErrorToString(ReturnCode))));
		return;
	}

	ZibraVDBRenderingResources = RRS->InitResourcesForActor(RenderingResourcesMapKey, *this);

	if (!ZibraVDBRenderingResources)
	{
		return;
	}

	ENQUEUE_RENDER_COMMAND(RegisterDecompressorResources)
	(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			Zibra::CE::Decompression::DecompressorResources DecompressorResources{};
			DecompressorResources.decompressionPerChannelBlockData =
				ZibraVDBRenderingResources->ZibraVDBNativeResources->DecompressionPerChannelBlockData.ZibraRHIBuffer;
			DecompressorResources.decompressionPerChannelBlockInfo =
				ZibraVDBRenderingResources->ZibraVDBNativeResources->DecompressionPerChannelBlockInfo.ZibraRHIBuffer;
			DecompressorResources.decompressionPerSpatialBlockInfo =
				ZibraVDBRenderingResources->ZibraVDBNativeResources->DecompressionPerSpatialBlockInfo.ZibraRHIBuffer;
			Zibra::CE::ReturnCode status = DecompressorManager.RegisterResources(DecompressorResources);
			if (status != Zibra::CE::ZCE_SUCCESS)
			{
				FZibraVDBNotifications::AddNotification_RenderThread(
					LOCTEXT("DecompressorResourceRegisterFailure", "Failed to register decompression resources."));
			}
		});

	UpdateChannelTextures();
	UpdateMaterialInstance(true);
	UpdateHVMaterialInstance(true);

	bShowWarningDifferentVoxelSize =
		!ZibraVDBRenderingResources->ZibraVDBNativeResources->RenderParameters.bHasPerFrameEqualTransformations;
	if (bShowWarningDifferentVoxelSize)
	{
		FZibraVDBNotifications::AddNotification(
			FText::Format(LOCTEXT("DifferentVoxelSizeWarning",
							  "Channels have different voxel size and won't be rendered properly (Actor: {0})."),
				FText::FromString(GetOwner()->GetActorNameOrLabel())),
			5, false);
	}

	bIsResourcesValid = true;
	MarkRenderStateDirty();
	UpdateGlobalSet();
}

void UZibraVDBMaterialComponent::ReleaseResources()
{
	if (!bIsResourcesValid)
	{
		return;
	}

	UZibraVDBRenderingResourcesSubsystem* RRS = GEngine->GetEngineSubsystem<UZibraVDBRenderingResourcesSubsystem>();
	RRS->RemoveIfInvalid(RenderingResourcesMapKey);
	ZibraVDBRenderingResources = nullptr;
	bIsResourcesValid = false;
	MarkRenderStateDirty();
	UpdateGlobalSet();
}

#undef LOCTEXT_NAMESPACE
