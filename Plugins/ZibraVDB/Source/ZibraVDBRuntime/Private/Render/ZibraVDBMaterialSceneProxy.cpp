// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBMaterialSceneProxy.h"

#include "Components/StaticMeshComponent.h"
#include "Curves/CurveLinearColor.h"
#include "TextureResource.h"
#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBMaterialComponent.h"
#include "ZibraVDBMaterialRendering.h"
#include "ZibraVDBRenderBuffer.h"
#include "ZibraVDBRuntime.h"
#include "ZibraVDBVolume4.h"

#include <functional>

FZibraVDBMaterialSceneProxy::FZibraVDBMaterialSceneProxy(
	const UZibraVDBAssetComponent* AssetComponent, UPrimitiveComponent* InComponent)
	: FPrimitiveSceneProxy(InComponent)
	, ZibraVDBMaterialRenderExtension(FZibraVDBRuntimeModule::GetZibraRenderExtension())
	, ZibraVDBAssetComponent(AssetComponent)
	, ZibraVDBMaterialComponent(CastChecked<UZibraVDBMaterialComponent>(InComponent))
	, Material(InComponent->GetMaterial(0))
{
#if !UE_BUILD_SHIPPING
	AActor* Actor = InComponent->GetOwner();
	ActorName = Actor ? Actor->GetName() : TEXT("NULL");
#endif

	if (!AssetComponent || !AssetComponent->HasZibraVDBVolume())
	{
		return;
	}

	const ZibraVDB::FZibraVDBMetadata& ZibraVDBMetadata = AssetComponent->GetZibraVDBVolume()->GetZibraVDBFile().GetMetadata();

	DecompressorManager = ZibraVDBMaterialComponent->GetDecompressorManager();
	Zibra::CE::Decompression::FrameRange FrameRange = DecompressorManager.GetFrameRange();
	KeyFrameStartIndex = FrameRange.start;
	KeyFrameEndIndex = FrameRange.end;

	UpdateRenderParams();
	ENQUEUE_RENDER_COMMAND(FZibraVDBMaterialSceneProxyConstruction)
	(
		[this, FrameIndexToRender = AssetComponent->GetCurrentFrame()](FRHICommandListImmediate& RHICmdList)
		{
			ScatteringColorTexture = ZibraVDBMaterialComponent->GetScatteringColorTexture();
			TemperatureColorTexture = ZibraVDBMaterialComponent->GetTemperatureColorTexture();
			FlameColorTexture = ZibraVDBMaterialComponent->GetFlameColorTexture();

			UpdateFrameIndex(FrameIndexToRender);
			InitRenderTexturesSRVsUAVsRenderThread(RHICmdList);
		});
}

void FZibraVDBMaterialSceneProxy::PrepareForRender() noexcept
{
	if (SparseBlocksRGBUAV.IsValid())
	{
		InitRenderTexturesSRVsUAVs();
	}

	if (GetCurrentFrameIndex() >= GetNativeResources()->FrameHeaders.Num())
	{
		FrameIndexToRender = 0;
	}
}

void FZibraVDBMaterialSceneProxy::PrepareForBBoxRender() noexcept
{
	if (GetCurrentFrameIndex() >= GetNativeResources()->FrameHeaders.Num())
	{
		FrameIndexToRender = 0;
	}
}

bool FZibraVDBMaterialSceneProxy::IsReadyForRender() const noexcept
{
	auto IsRMTexturesValid = true;
	if (CastedReflectionsCount > 0)
	{
		auto VDB = Cast<AZibraVDBActor>(ZibraVDBMaterialComponent->GetOwner());
		if (VDB)
		{
			auto RM = VDB->ReflectionManagerComponent;
			if (RM)
			{
				IsRMTexturesValid = IlluminatedVolumeTextureUAV.IsValid();
			}
		}
	}

	return SparseBlocksRGBUAV.IsValid() && BlockIndexTextureUAV.IsValid() && TransformBufferUAV.IsValid() &&
		   ScatteringColorTexture.IsValid() && TemperatureColorTexture.IsValid() && FlameColorTexture.IsValid() &&
		   IsRMTexturesValid;
}

void FZibraVDBMaterialSceneProxy::InitRenderTexturesSRVsUAVs() noexcept
{
	if (IsInRenderingThread())
	{
		InitRenderTexturesSRVsUAVsRenderThread(FRHICommandListImmediate::Get());
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(InitRenderTexturesUAV)
		([this](FRHICommandListImmediate& RHICmdList) { InitRenderTexturesSRVsUAVsRenderThread(RHICmdList); });
	}
}

void FZibraVDBMaterialSceneProxy::InitRenderTexturesSRVsUAVsRenderThread(FRHICommandListImmediate& RHICmdList) noexcept
{
	check(IsInRenderingThread());

	const auto InitUAVAndSRV = [&RHICmdList](const TObjectPtr<UTextureRenderTargetVolume>& RTV, FUnorderedAccessViewRHIRef& UAV,
								   FShaderResourceViewRHIRef& SRV)
	{
		if (!RTV)
		{
			return;
		}

		const FTextureRHIRef& Texture = RTV->GetResource()->GetTextureRHI();

		if (!UAV || UAV->GetTexture() != Texture)
		{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 506
			UAV = RHICmdList.CreateUnorderedAccessView(Texture, FRHIViewDesc::CreateTextureUAV().SetDimensionFromTexture(Texture));
#else
			UAV = RHICmdList.CreateUnorderedAccessView(Texture);
#endif
		}
		if (!SRV || SRV->GetTexture() != Texture)
		{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 506
			SRV = RHICmdList.CreateShaderResourceView(Texture, FRHIViewDesc::CreateTextureSRV().SetDimensionFromTexture(Texture));
#else
			SRV = RHICmdList.CreateShaderResourceView(Texture, 0);
#endif
		}
	};

	InitUAVAndSRV(ZibraVDBMaterialComponent->GetSparseBlocksRGB(), SparseBlocksRGBUAV, SparseBlocksRGBSRV);
	InitUAVAndSRV(ZibraVDBMaterialComponent->GetSparseBlocksDensity(), SparseBlocksDensityUAV, SparseBlocksDensitySRV);
	InitUAVAndSRV(
		ZibraVDBMaterialComponent->GetMaxDensityPerBlockTexture(), MaxDensityPerBlockTextureUAV, MaxDensityPerBlockTextureSRV);
	InitUAVAndSRV(ZibraVDBMaterialComponent->GetBlockIndexTexture(), BlockIndexTextureUAV, BlockIndexTextureSRV);
	InitUAVAndSRV(ZibraVDBMaterialComponent->GetTransformBuffer(), TransformBufferUAV, TransformBufferSRV);
	InitUAVAndSRV(
		ZibraVDBMaterialComponent->GetIlluminatedVolumeTexture(), IlluminatedVolumeTextureUAV, IlluminatedVolumeTextureSRV);
}

UMaterialInterface* FZibraVDBMaterialSceneProxy::GetMaterial() const noexcept
{
	return Material;
}
const UZibraVDBAssetComponent* FZibraVDBMaterialSceneProxy::GetAssetComponent() const noexcept
{
	return ZibraVDBAssetComponent;
}
const UZibraVDBMaterialComponent* FZibraVDBMaterialSceneProxy::GetMaterialComponent() const noexcept
{
	return ZibraVDBMaterialComponent;
}
TSharedPtr<FZibraVDBRenderingResources>& FZibraVDBMaterialSceneProxy::GetRenderingResources() noexcept
{
	return ZibraVDBMaterialComponent->GetRenderingResources();
}
TSharedPtr<FZibraVDBNativeResource> FZibraVDBMaterialSceneProxy::GetNativeResources() noexcept
{
	return ZibraVDBMaterialComponent->GetNativeResources();
}
const TSharedPtr<FZibraVDBNativeResource> FZibraVDBMaterialSceneProxy::GetNativeResources() const noexcept
{
	return ZibraVDBMaterialComponent->GetNativeResources();
}

void FZibraVDBMaterialSceneProxy::ResetVisibility() noexcept
{
	VisibleViews.Empty();
}
bool FZibraVDBMaterialSceneProxy::IsVisible(const FSceneView* View) const noexcept
{
	return VisibleViews.Find(View) != INDEX_NONE;
}

bool FZibraVDBMaterialSceneProxy::IsPlaying() const noexcept
{
	return bPlaying;
}
int FZibraVDBMaterialSceneProxy::GetCurrentFrameIndex() const noexcept
{
	return FrameIndexToRender;
}
const FZibraVDBNativeResource::FFrameParameters& FZibraVDBMaterialSceneProxy::GetCurrentFrameHeader() const noexcept
{
	check(GetNativeResources().IsValid());
	return GetNativeResources()->FrameHeaders[GetCurrentFrameIndex()];
}

void FZibraVDBMaterialSceneProxy::SetPreviousChannelMasks() noexcept
{
	FZibraVDBNativeResource::FRenderParameters& RenderParameters = GetNativeResources()->RenderParameters;

	RenderParameters.PreviousDensityChannelMask = RenderParameters.DensityChannelMask;
	RenderParameters.PreviousTemperatureChannelMask = RenderParameters.TemperatureChannelMask;
	RenderParameters.PreviousFlamesChannelMask = RenderParameters.FlamesChannelMask;
}

bool FZibraVDBMaterialSceneProxy::HasChannelMasksChanged() const noexcept
{
	FZibraVDBNativeResource::FRenderParameters& RenderParameters = GetNativeResources()->RenderParameters;

	return RenderParameters.PreviousDensityChannelMask != RenderParameters.DensityChannelMask ||
		   RenderParameters.PreviousTemperatureChannelMask != RenderParameters.TemperatureChannelMask ||
		   RenderParameters.PreviousFlamesChannelMask != RenderParameters.FlamesChannelMask;
}

void FZibraVDBMaterialSceneProxy::SetDecompressedFrame() noexcept
{
	CurrentlyDecompressedFrame = FrameIndexToRender;
}
void FZibraVDBMaterialSceneProxy::ResetDecompressedFrame() noexcept
{
	CurrentlyDecompressedFrame = -1;
}
bool FZibraVDBMaterialSceneProxy::NeedToDecompress() const noexcept
{
	return FrameIndexToRender != CurrentlyDecompressedFrame;
}
bool FZibraVDBMaterialSceneProxy::GetVisible() const noexcept
{
	return bVisible;
}
void FZibraVDBMaterialSceneProxy::SetVisible(bool InVisible) noexcept
{
	bVisible = InVisible;
}
int FZibraVDBMaterialSceneProxy::GetActiveRegions() const noexcept
{
	return GetNativeResources()->RenderParameters.MaxRenderBlockCountCBRT;
}

FIntVector2 FZibraVDBMaterialSceneProxy::GetTextureResolution() const noexcept
{
	return TextureResolution;
}

bool FZibraVDBMaterialSceneProxy::IsTextureDirty() const noexcept
{
	return TextureIsDirty;
}

void FZibraVDBMaterialSceneProxy::SetTextureDirty(bool Dirty) noexcept
{
	TextureIsDirty = Dirty;
}

void FZibraVDBMaterialSceneProxy::UpdateFrameIndex(const int InCurrentFrameIndex) noexcept
{
	FrameIndexToRender = InCurrentFrameIndex;
}

void FZibraVDBMaterialSceneProxy::UpdateRenderParams() noexcept
{
	check(ZibraVDBMaterialComponent != nullptr);

	if (!ZibraVDBAssetComponent || !ZibraVDBAssetComponent->HasZibraVDBVolume())
	{
		return;
	}

	const uint32_t FrameCount = ZibraVDBAssetComponent->GetZibraVDBVolume()->GetZibraVDBFile().GetHeader().FrameCount;

	DensityChannelIndex = ZibraVDBAssetComponent->GetZibraVDBVolume()->GetDensityChannelIndex();
	TemperatureChannelIndex = ZibraVDBAssetComponent->GetZibraVDBVolume()->GetTemperatureChannelIndex();
	FlamesChannelIndex = ZibraVDBAssetComponent->GetZibraVDBVolume()->GetFlamesChannelIndex();
	DensityChannelMask = ZibraVDBAssetComponent->GetZibraVDBVolume()->GetDensityChannelMask();
	TemperatureChannelMask = ZibraVDBAssetComponent->GetZibraVDBVolume()->GetTemperatureChannelMask();
	FlamesChannelMask = ZibraVDBAssetComponent->GetZibraVDBVolume()->GetFlamesChannelMask();

	const auto EmissiveTemperatureScale =
		(ZibraVDBMaterialComponent->bEnableEmissionMasking && ZibraVDBMaterialComponent->bEnableTemperatureMasking &&
			FMath::IsNearlyEqual(ZibraVDBMaterialComponent->MaskCenter, 1.f))
			? 0.f
			: 1.f;
	const auto EmissiveFlameScale =
		(ZibraVDBMaterialComponent->bEnableEmissionMasking && ZibraVDBMaterialComponent->bEnableFlameMasking &&
			FMath::IsNearlyEqual(ZibraVDBMaterialComponent->MaskCenter, 1.f))
			? 0.f
			: 1.f;
	DensityScale = ZibraVDBMaterialComponent->GetDensityScaleInternal() * VDBDensityScale;
	TemperatureScale = ZibraVDBMaterialComponent->GetTemperatureScaleInternal() * EmissiveTemperatureScale;
	FlameScale = ZibraVDBMaterialComponent->GetFlameScaleInternal() * VDBFlameScale * EmissiveFlameScale;
	ScatteringColor = ZibraVDBMaterialComponent->ScatteringColor;
	AbsorptionColor = ZibraVDBMaterialComponent->AbsorptionColor;

	ShadowIntensity = ZibraVDBMaterialComponent->GetShadowIntensityInternal();
	EnableDirectionalLightShadows = ZibraVDBMaterialComponent->EnableDirectionalLightShadows;
	EnablePointLightShadows = ZibraVDBMaterialComponent->EnablePointLightShadows;
	EnableSpotLightShadows = ZibraVDBMaterialComponent->EnableSpotLightShadows;
	EnableRectLightShadows = ZibraVDBMaterialComponent->EnableRectLightShadows;
	EnableDirectionalLightReceivedShadows = ZibraVDBMaterialComponent->EnableDirectionalLightReceivedShadows;
	EnablePointLightReceivedShadows = ZibraVDBMaterialComponent->EnablePointLightReceivedShadows;
	EnableSpotLightReceivedShadows = ZibraVDBMaterialComponent->EnableSpotLightReceivedShadows;
	EnableRectLightReceivedShadows = ZibraVDBMaterialComponent->EnableRectLightReceivedShadows;
	SmoothReceivedShadows = ZibraVDBMaterialComponent->SmoothReceivedShadows;
	AllowSkipDecompression = ZibraVDBMaterialComponent->AllowSkipDecompression || FrameCount == 1;
	DirectionalLightBrightness =
		ZibraVDBMaterialComponent->DirectionalLightBrightness * ZibraVDBMaterialComponent->DirectionalLightBrightnessScaleFactor;
	PointLightBrightness =
		ZibraVDBMaterialComponent->PointLightBrightness * ZibraVDBMaterialComponent->PointLightBrightnessScaleFactor;
	SpotLightBrightness =
		ZibraVDBMaterialComponent->SpotLightBrightness * ZibraVDBMaterialComponent->SpotLightBrightnessScaleFactor;
	RectLightBrightness =
		ZibraVDBMaterialComponent->RectLightBrightness * ZibraVDBMaterialComponent->RectLightBrightnessScaleFactor;
	IlluminationResolution = ConvertDownscaleFactorToScale(ZibraVDBMaterialComponent->IlluminationResolution);
	IlluminationDownscaleFactor = GetDownscaleFactorExponent(ZibraVDBMaterialComponent->IlluminationResolution);
	RayMarchingMainStepSize = ZibraVDBMaterialComponent->RayMarchingMainStepSize;
	RayMarchingSelfShadowStepSize = ZibraVDBMaterialComponent->RayMarchingSelfShadowStepSize;
	RayMarchingMaxMainSteps = ZibraVDBMaterialComponent->RayMarchingMaxMainSteps;
	RayMarchingMaxSelfShadowSteps = ZibraVDBMaterialComponent->RayMarchingMaxSelfShadowSteps;

	bool EnableAmbientLighting = (ZibraVDBMaterialComponent->AmbientLightingMode != EAmbientLightingMode::Disabled);
	if (ZibraVDBMaterialComponent->AmbientLightIntensity <= UE_KINDA_SMALL_NUMBER)
	{
		EnableAmbientLighting = false;
	}
	if (ZibraVDBMaterialComponent->AmbientLightingMode == EAmbientLightingMode::Constant &&
		ZibraVDBMaterialComponent->AmbientLightingColor.IsAlmostBlack())
	{
		EnableAmbientLighting = false;
	}

	AmbientLightingMode =
		int(EnableAmbientLighting ? ZibraVDBMaterialComponent->AmbientLightingMode : EAmbientLightingMode::Disabled);
	AmbientLightIntensity = ZibraVDBMaterialComponent->AmbientLightIntensity;
	AmbientLightingColor = ZibraVDBMaterialComponent->AmbientLightingColor;
	AOShadowIntensity = ZibraVDBMaterialComponent->AOShadowIntensity;
	AORadius = ZibraVDBMaterialComponent->AORadius;
	AORayMarcherStepCount = ZibraVDBMaterialComponent->AORayMarcherStepCount;

	RayMarchingFiltering = ZibraVDBMaterialComponent->RayMarchingFiltering;
	VolumeDownscaleFactor = GetDownscaleFactorExponent(ZibraVDBMaterialComponent->VolumeResolution);

	bEnableSphereMasking = ZibraVDBMaterialComponent->bEnableSphereMasking;
	SphereMaskPosition = ZibraVDBMaterialComponent->MaskingSpherePosition;
	SphereMaskRadius = ZibraVDBMaterialComponent->MaskingSphereRadius;
	SphereMaskFalloff = ZibraVDBMaterialComponent->MaskingSphereFalloff;

	bUseHeterogeneousVolume = ZibraVDBMaterialComponent->bUseHeterogeneousVolume;
	CastedReflectionsCount = 0;
	if (!bUseHeterogeneousVolume)
	{
		auto VDB = Cast<AZibraVDBActor>(ZibraVDBMaterialComponent->GetOwner());
		if (VDB)
		{
			auto RM = VDB->ReflectionManagerComponent;
			if (RM)
			{
				CastedReflectionsCount = RM->ActorsToApplyReflections.Num();
			}
		}
	}

	bEnableEmissionMasking = ZibraVDBMaterialComponent->bEnableEmissionMasking;
	bEnableFlameEmissionMasking = ZibraVDBMaterialComponent->bEnableFlameMasking;
	bEnableTemperatureEmissionMasking = ZibraVDBMaterialComponent->bEnableTemperatureMasking;
	EmissionMaskCenter = ZibraVDBMaterialComponent->MaskCenter;
	EmissionMaskWidth = FMath::Loge(1.f - (ZibraVDBMaterialComponent->MaskWidth + 1e-6) / 4.f);
	EmissionMaskIntensity = 1.f - ((FMath::Pow(10.f, -ZibraVDBMaterialComponent->MaskIntensity) - 1.f) / (0.1f - 1.f));
	EmissionMaskRamp = FMath::Exp(ZibraVDBMaterialComponent->MaskRamp);

	bUseStaticIllumination = ZibraVDBMaterialComponent->bUseStaticIllumination;

	DecompressorManager = ZibraVDBMaterialComponent->GetDecompressorManager();
}

SIZE_T FZibraVDBMaterialSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
void FZibraVDBMaterialSceneProxy::CreateRenderThreadResources(FRHICommandListBase& RHICmdList)
#else
void FZibraVDBMaterialSceneProxy::CreateRenderThreadResources()
#endif
{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
	FPrimitiveSceneProxy::CreateRenderThreadResources(RHICmdList);
	ZibraVDBMaterialRenderExtension->AddVDBProxy(this, RHICmdList);
#else
	FPrimitiveSceneProxy::CreateRenderThreadResources();
	ZibraVDBMaterialRenderExtension->AddVDBProxy(this);
#endif
}

void FZibraVDBMaterialSceneProxy::DestroyRenderThreadResources()
{
	FPrimitiveSceneProxy::DestroyRenderThreadResources();

	ZibraVDBMaterialRenderExtension->RemoveVDBProxy(this);
}

FPrimitiveViewRelevance FZibraVDBMaterialSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View) && ShouldRenderInMainPass();
	Result.bDynamicRelevance = true;
	Result.bStaticRelevance = false;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	return Result;
}

FMatrix44f FZibraVDBMaterialSceneProxy::GetWorldToVolumeMatrix() const noexcept
{
	FMatrix44f VolumeToWorld = FMatrix44f(GetLocalToWorld());
	return VolumeToWorld.Inverse();
}

FMatrix44f FZibraVDBMaterialSceneProxy::GetWorldToIlluminationGridMatrix() const noexcept
{
	FVector3f TextureSize = FVector3f(GetNativeResources()->RenderParameters.TextureSize);

	FMatrix44f WorldToVolume = GetWorldToVolumeMatrix();

	FMatrix44f TransformToIndexSpace = FScaleMatrix44f(FVector3f::One() / (GetCurrentFrameHeader().IndexToUnrealUnitsScale));
	FMatrix44f IlluminationDownscale = FScaleMatrix44f(IlluminationResolution);

	// Apply inverse frame transform.
	// Inverse because we rotate the light position and keep vdb stationary.
	FMatrix44f FrameInverseTransform = GetFrameToSequenceCoordsRotation();
	FVector3f FrameTranslation = GetCurrentFrameHeader().IndexToWorldTranslationInVoxels;
	FrameInverseTransform.SetOrigin(FrameTranslation);
	FrameInverseTransform = FrameInverseTransform.Inverse();

	return WorldToVolume * TransformToIndexSpace * FrameInverseTransform * IlluminationDownscale;
}

FMatrix44f FZibraVDBMaterialSceneProxy::GetVoxelToWorldMatrix() const noexcept
{
	FVector3f TextureSize = FVector3f(GetNativeResources()->RenderParameters.TextureSize);
	FMatrix44f EffectScale = FScaleMatrix44f(GetCurrentFrameHeader().IndexToUnrealUnitsScale);
	EffectScale.SetOrigin(-TextureSize * GetCurrentFrameHeader().IndexToUnrealUnitsScale / 2);
	return EffectScale * GetFrameToSequenceCoordsTransform() * FMatrix44f(GetLocalToWorld());
}

FMatrix44f FZibraVDBMaterialSceneProxy::GetFrameToSequenceCoordsTransform() const noexcept
{
	FMatrix44f Transform = GetFrameToSequenceCoordsRotation();
	Transform.SetOrigin(GetFrameToSequenceCoordsTranslation());
	return Transform;
}
FMatrix44f FZibraVDBMaterialSceneProxy::GetFrameToSequenceCoordsRotation() const noexcept
{
	return GetCurrentFrameHeader().IndexToWorldRotation;
}
FVector3f FZibraVDBMaterialSceneProxy::GetFrameToSequenceCoordsTranslation() const noexcept
{
	FVector3f FrameTranslation = GetCurrentFrameHeader().IndexToWorldTranslationInVoxels;

	FVector3f FrameSizeAdjustment =
		(FVector3f(GetCurrentFrameHeader().FrameSize) - FVector3f(GetNativeResources()->RenderParameters.TextureSize)) / 2.f;
	FVector3f TextureSize = FVector3f(GetNativeResources()->RenderParameters.TextureSize);

	FVector3f TranslationInVoxels = FrameTranslation + GetFrameToSequenceCoordsRotation().Rotator().RotateVector(TextureSize / 2.f);
	FVector3f TranslationInUnrealUnits = TranslationInVoxels * GetCurrentFrameHeader().IndexToUnrealUnitsScale;

	return TranslationInUnrealUnits;
}

float FZibraVDBMaterialSceneProxy::GetWorldToIlluminationGridScale() const noexcept
{
	FVector ActorScale = GetLocalToWorld().GetScaleVector();

	return IlluminationResolution / ActorScale.GetMin();
}

bool FZibraVDBMaterialSceneProxy::HasDensity() const noexcept
{
	return GetAssetComponent()->GetZibraVDBVolume()->GetDensityChannelIndex() != -1;
}

int FZibraVDBMaterialSceneProxy::GetLastIlluminationFrame() const noexcept
{
	return LastIlluminationFrame;
}

void FZibraVDBMaterialSceneProxy::UpdateLastIlluminationFrame() noexcept
{
	LastIlluminationFrame = FrameIndexToRender;
}

bool FZibraVDBMaterialSceneProxy::NeedsIlluminationUpdate() const noexcept
{
	return LastIlluminationFrame != FrameIndexToRender;
}

uint32 FZibraVDBMaterialSceneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

#define ZIB_DEFINE_TEXTURE_3D_WRAPPER_INTERFACE(Name)                                                                              \
	bool FZibraVDBMaterialSceneProxy::Has##Name##Texture() const noexcept                                                          \
	{                                                                                                                              \
		return Name.Texture.IsValid();                                                                                             \
	}                                                                                                                              \
	const FZibraTexture3DRHIRef& FZibraVDBMaterialSceneProxy::Get##Name##Texture() const noexcept                                  \
	{                                                                                                                              \
		return Name.Texture;                                                                                                       \
	}                                                                                                                              \
	const FShaderResourceViewRHIRef& FZibraVDBMaterialSceneProxy::Get##Name##SRV() const noexcept                                  \
	{                                                                                                                              \
		return Name.SRV;                                                                                                           \
	}                                                                                                                              \
	const FUnorderedAccessViewRHIRef& FZibraVDBMaterialSceneProxy::Get##Name##UAV() const noexcept                                 \
	{                                                                                                                              \
		return Name.UAV;                                                                                                           \
	}                                                                                                                              \
	ERHIAccess FZibraVDBMaterialSceneProxy::Get##Name##CurrentState() const noexcept                                               \
	{                                                                                                                              \
		return Name.CurrentState;                                                                                                  \
	}                                                                                                                              \
	void FZibraVDBMaterialSceneProxy::Set##Name(                                                                                   \
		FZibraTexture3DRHIRef InTexture, FShaderResourceViewRHIRef InTextureSRV, FUnorderedAccessViewRHIRef InTextureUAV) noexcept \
	{                                                                                                                              \
		Set##Name##Texture(MoveTemp(InTexture));                                                                                   \
		Set##Name##SRV(MoveTemp(InTextureSRV));                                                                                    \
		Set##Name##UAV(MoveTemp(InTextureUAV));                                                                                    \
	}                                                                                                                              \
	void FZibraVDBMaterialSceneProxy::Set##Name##Texture(FZibraTexture3DRHIRef InTexture) noexcept                                 \
	{                                                                                                                              \
		Name.Texture = MoveTemp(InTexture);                                                                                        \
	}                                                                                                                              \
	void FZibraVDBMaterialSceneProxy::Set##Name##SRV(FShaderResourceViewRHIRef InTextureSRV) noexcept                              \
	{                                                                                                                              \
		Name.SRV = MoveTemp(InTextureSRV);                                                                                         \
	}                                                                                                                              \
	void FZibraVDBMaterialSceneProxy::Set##Name##UAV(FUnorderedAccessViewRHIRef InTextureUAV) noexcept                             \
	{                                                                                                                              \
		Name.UAV = MoveTemp(InTextureUAV);                                                                                         \
	}                                                                                                                              \
	void FZibraVDBMaterialSceneProxy::Set##Name##CurrentState(ERHIAccess InCurrentState) noexcept                                  \
	{                                                                                                                              \
		Name.CurrentState = InCurrentState;                                                                                        \
	}                                                                                                                              \
	void FZibraVDBMaterialSceneProxy::Release##Name() noexcept                                                                     \
	{                                                                                                                              \
		Name.Texture.SafeRelease();                                                                                                \
		Name.SRV.SafeRelease();                                                                                                    \
		Name.UAV.SafeRelease();                                                                                                    \
	}

ZIB_DEFINE_TEXTURE_3D_WRAPPER_INTERFACE(ZibraIllumination);
ZIB_DEFINE_TEXTURE_3D_WRAPPER_INTERFACE(ZibraDensityTextureLOD);

#undef ZIB_DEFINE_TEXTURE_3D_WRAPPER_INTERFACE

#define ZIB_DEFINE_SRV_UAV(Name)                                                                   \
	const FShaderResourceViewRHIRef& FZibraVDBMaterialSceneProxy::Get##Name##SRV() const noexcept  \
	{                                                                                              \
		return Name##SRV;                                                                          \
	}                                                                                              \
	ERHIAccess FZibraVDBMaterialSceneProxy::Get##Name##CurrentState() const noexcept               \
	{                                                                                              \
		return Name##State;                                                                        \
	}                                                                                              \
	const FUnorderedAccessViewRHIRef& FZibraVDBMaterialSceneProxy::Get##Name##UAV() const noexcept \
	{                                                                                              \
		return Name##UAV;                                                                          \
	}                                                                                              \
	void FZibraVDBMaterialSceneProxy::Set##Name##CurrentState(ERHIAccess InCurrentState) noexcept  \
	{                                                                                              \
		Name##State = InCurrentState;                                                              \
	}

ZIB_DEFINE_SRV_UAV(SparseBlocksRGB);
ZIB_DEFINE_SRV_UAV(SparseBlocksDensity);
ZIB_DEFINE_SRV_UAV(MaxDensityPerBlockTexture);
ZIB_DEFINE_SRV_UAV(IlluminatedVolumeTexture);
ZIB_DEFINE_SRV_UAV(BlockIndexTexture);
ZIB_DEFINE_SRV_UAV(TransformBuffer);

#undef ZIB_DEFINE_SRV_UAV
