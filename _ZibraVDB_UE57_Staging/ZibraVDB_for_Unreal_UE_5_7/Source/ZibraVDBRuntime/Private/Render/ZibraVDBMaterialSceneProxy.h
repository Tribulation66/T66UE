// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Engine/TextureRenderTargetVolume.h"
#include "PrimitiveSceneProxy.h"
#include "SceneView.h"
#include "ZibraVDBCommon.h"
#include "ZibraVDBRenderBuffer.h"
#include "ZibraVDBRenderingResourcesSubsystem.h"

class UZibraVDBAssetComponent;
class UZibraVDBMaterialComponent;
enum class ERayMarchingFiltering : uint8;

#define ZIB_DECLARE_TEXTURE_3D_WRAPPER_INTERFACE(Name)                                      \
public:                                                                                     \
	bool Has##Name##Texture() const noexcept;                                               \
	const FZibraTexture3DRHIRef& Get##Name##Texture() const noexcept;                       \
	const FShaderResourceViewRHIRef& Get##Name##SRV() const noexcept;                       \
	const FUnorderedAccessViewRHIRef& Get##Name##UAV() const noexcept;                      \
	void Set##Name(FZibraTexture3DRHIRef InTexture, FShaderResourceViewRHIRef InTextureSRV, \
		FUnorderedAccessViewRHIRef InTextureUAV) noexcept;                                  \
	ERHIAccess Get##Name##CurrentState() const noexcept;                                    \
	void Set##Name##Texture(FZibraTexture3DRHIRef InTexture) noexcept;                      \
	void Set##Name##SRV(FShaderResourceViewRHIRef InTextureSRV) noexcept;                   \
	void Set##Name##UAV(FUnorderedAccessViewRHIRef InTextureUAV) noexcept;                  \
	void Set##Name##CurrentState(ERHIAccess InCurrentState) noexcept;                       \
	void Release##Name() noexcept;                                                          \
                                                                                            \
private:                                                                                    \
	FTexture3DRHIWrapper Name;

#define ZIB_DECLARE_SRV_UAV(Name)                                      \
public:                                                                \
	const FShaderResourceViewRHIRef& Get##Name##SRV() const noexcept;  \
	ERHIAccess Get##Name##CurrentState() const noexcept;               \
	const FUnorderedAccessViewRHIRef& Get##Name##UAV() const noexcept; \
	void Set##Name##CurrentState(ERHIAccess InCurrentState) noexcept;  \
                                                                       \
private:                                                               \
	FShaderResourceViewRHIRef Name##SRV;                               \
	FUnorderedAccessViewRHIRef Name##UAV;                              \
	ERHIAccess Name##State;

struct FZibraLightProperty
{
	enum class ELightType
	{
		Directional,
		Point,
		Spot,
		Rect
	};

	ELightType Type;
	FVector3f Direction;
	FVector3f Position;
	FLinearColor Color;
	float Intensity;
	float SourceRadius;
	bool CastShadows;

	// Point and Spot light parameters.
	float AttenuationRadius;

	// Spot light parameters.
	float InnerRadiusLimit;
	float OuterRadiusLimit;
};

class FZibraVDBMaterialSceneProxy final : public FPrimitiveSceneProxy
{
public:
	FZibraVDBMaterialSceneProxy(const UZibraVDBAssetComponent* AssetComponent, UPrimitiveComponent* InComponent);

	void PrepareForRender() noexcept;
	void PrepareForBBoxRender() noexcept;
	bool IsReadyForRender() const noexcept;
	void InitRenderTexturesSRVsUAVs() noexcept;
	class UMaterialInterface* GetMaterial() const noexcept;
	const UZibraVDBAssetComponent* GetAssetComponent() const noexcept;
	const UZibraVDBMaterialComponent* GetMaterialComponent() const noexcept;
	TSharedPtr<FZibraVDBRenderingResources>& GetRenderingResources() noexcept;
	TSharedPtr<FZibraVDBNativeResource> GetNativeResources() noexcept;
	const TSharedPtr<FZibraVDBNativeResource> GetNativeResources() const noexcept;

	void ResetVisibility() noexcept;
	bool IsVisible(const FSceneView* View) const noexcept;
	void UpdateFrameIndex(const int InCurrentFrameIndex) noexcept;
	void UpdateRenderParams() noexcept;
	void UpdateRenderParamsRenderThread(FRHICommandListImmediate& RHICmdList) noexcept;
	bool IsPlaying() const noexcept;
	int GetCurrentFrameIndex() const noexcept;
	const FZibraVDBNativeResource::FFrameParameters& GetCurrentFrameHeader() const noexcept;

	void SetPreviousChannelMasks() noexcept;
	bool HasChannelMasksChanged() const noexcept;

	void SetDecompressedFrame() noexcept;
	void ResetDecompressedFrame() noexcept;
	bool NeedToDecompress() const noexcept;

	bool GetVisible() const noexcept;
	void SetVisible(bool InVisible) noexcept;
	int GetActiveRegions() const noexcept;

	FIntVector2 GetTextureResolution() const noexcept;

	bool IsTextureDirty() const noexcept;
	void SetTextureDirty(bool Dirty) noexcept;

	FMatrix44f GetWorldToVolumeMatrix() const noexcept;
	FMatrix44f GetWorldToIlluminationGridMatrix() const noexcept;
	FMatrix44f GetVoxelToWorldMatrix() const noexcept;
	FMatrix44f GetFrameToSequenceCoordsTransform() const noexcept;
	FMatrix44f GetFrameToSequenceCoordsRotation() const noexcept;
	FVector3f GetFrameToSequenceCoordsTranslation() const noexcept;
	float GetWorldToIlluminationGridScale() const noexcept;

	bool HasDensity() const noexcept;

	int GetLastIlluminationFrame() const noexcept;
	void UpdateLastIlluminationFrame() noexcept;
	bool NeedsIlluminationUpdate() const noexcept;

protected:
	//~ Begin FPrimitiveSceneProxy Interface
	SIZE_T GetTypeHash() const final;

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
	void CreateRenderThreadResources(FRHICommandListBase& RHICmdList) final;
#else
	void CreateRenderThreadResources() final;
#endif
	void DestroyRenderThreadResources() final;
	FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const final;
	uint32 GetMemoryFootprint() const final;
	//~ End FPrimitiveSceneProxy Interface

private:
	void InitRenderTexturesSRVsUAVsRenderThread(FRHICommandListImmediate& RHICmdList) noexcept;

private:
	bool bPlaying = false;
	TSharedPtr<class FZibraVDBMaterialRendering, ESPMode::ThreadSafe> ZibraVDBMaterialRenderExtension;

	// Fixed attributes.
	const UZibraVDBAssetComponent* ZibraVDBAssetComponent = nullptr;
	UZibraVDBMaterialComponent* ZibraVDBMaterialComponent = nullptr;
	class UMaterialInterface* Material = nullptr;

public:
#if !UE_BUILD_SHIPPING
	FString ActorName;
#endif

	int DensityChannelIndex = 0;
	int TemperatureChannelIndex = 0;
	int FlamesChannelIndex = 0;

	int DensityChannelMask = 0;
	int TemperatureChannelMask = 0;
	int FlamesChannelMask = 0;

	float DensityScale = 1.0;
	float TemperatureScale = 1.0;
	float FlameScale = 1.0;

	float VDBDensityScale = 1.0;
	float VDBTemperatureScale = 1.0;
	float VDBFlameScale = 1.0;

	FLinearColor ScatteringColor = FLinearColor(1.0, 1.0, 1.0);
	FLinearColor AbsorptionColor = FLinearColor(1.0, 1.0, 1.0);

	float ShadowIntensity = 1;
	bool EnableDirectionalLightShadows = true;
	bool EnablePointLightShadows = true;
	bool EnableSpotLightShadows = true;
	bool EnableRectLightShadows = true;
	float DirectionalLightBrightness = 1.0;
	float PointLightBrightness = 1.0;
	float SpotLightBrightness = 1.0;
	float RectLightBrightness = 1.0;
	static constexpr float DefaultIlluminationResolution = 0.25;
	float IlluminationResolution = DefaultIlluminationResolution;
	int IlluminationDownscaleFactor = 1;
	float RayMarchingMainStepSize = 0.75;
	float RayMarchingSelfShadowStepSize = 1.5;
	int32 RayMarchingMaxMainSteps = 512;
	int32 RayMarchingMaxSelfShadowSteps = 256;

	bool EnableDirectionalLightReceivedShadows = true;
	bool EnablePointLightReceivedShadows = true;
	bool EnableSpotLightReceivedShadows = true;
	bool EnableRectLightReceivedShadows = true;
	bool SmoothReceivedShadows = true;
	bool AllowSkipDecompression = false;

	int AmbientLightingMode = 1;
	float AmbientLightIntensity = 1.0;
	FLinearColor AmbientLightingColor = FLinearColor(0.0, 0.0, 0.0);
	float AOShadowIntensity = 1;
	float AORadius = 0.04f;
	int32 AORayMarcherStepCount = 2;

	ERayMarchingFiltering RayMarchingFiltering;
	int VolumeDownscaleFactor;

	bool bEnableSphereMasking = false;
	FVector3f SphereMaskPosition = FVector3f(0, 0, 0);
	float SphereMaskRadius = 10.0;
	float SphereMaskFalloff = 0.0;

	bool bUseHeterogeneousVolume = false;
	uint CastedReflectionsCount = 0;
	bool bEnableEmissionMasking = false;
	bool bEnableFlameEmissionMasking = false;
	bool bEnableTemperatureEmissionMasking = false;
	float EmissionMaskCenter = 0.1f;
	float EmissionMaskWidth = 0.1f;
	float EmissionMaskIntensity = 1.f;
	float EmissionMaskRamp = 1.f;

	FZibraTexture2DRHIRef ScatteringColorTexture;
	FZibraTexture2DRHIRef FlameColorTexture;
	FZibraTexture2DRHIRef TemperatureColorTexture;

	FZibraVDBDecompressorManager DecompressorManager;

	int KeyFrameStartIndex = 0;
	int KeyFrameEndIndex = 0;

	bool bUseStaticIllumination = false;

private:
	UTextureRenderTargetVolume* SparseBlocks3DTextureResult = nullptr;

	ZIB_DECLARE_SRV_UAV(SparseBlocksRGB);
	ZIB_DECLARE_SRV_UAV(SparseBlocksDensity);
	ZIB_DECLARE_SRV_UAV(MaxDensityPerBlockTexture);
	ZIB_DECLARE_SRV_UAV(IlluminatedVolumeTexture);
	ZIB_DECLARE_SRV_UAV(BlockIndexTexture);
	ZIB_DECLARE_SRV_UAV(TransformBuffer);

	int FrameIndexToRender = 0;
	int CurrentlyDecompressedFrame = -1;
	bool bVisible = true;
	int LastIlluminationFrame = -1;

	FIntVector2 TextureResolution = FIntVector2(0, 0);
	bool TextureIsDirty = false;

	mutable TArray<const FSceneView*> VisibleViews;

private:
	struct FTexture3DRHIWrapper
	{
		FZibraTexture3DRHIRef Texture;
		FShaderResourceViewRHIRef SRV;
		FUnorderedAccessViewRHIRef UAV;
		ERHIAccess CurrentState = ERHIAccess::Unknown;
	};

	ZIB_DECLARE_TEXTURE_3D_WRAPPER_INTERFACE(ZibraIllumination);
	ZIB_DECLARE_TEXTURE_3D_WRAPPER_INTERFACE(ZibraDensityTextureLOD);
};

#undef ZIB_DECLARE_TEXTURE_3D_WRAPPER_INTERFACE
