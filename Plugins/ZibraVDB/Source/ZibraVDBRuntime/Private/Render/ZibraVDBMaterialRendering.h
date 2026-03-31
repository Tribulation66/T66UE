// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Profiler/ZibraVDBGPUProfiler.h"
#include "RendererInterface.h"
#include "SceneViewExtension.h"
#include "ZibraVDBActor.h"
#include "ZibraVDBCommon.h"
#include "ZibraVDBRenderBuffer.h"

class FZibraVDBMaterialSceneProxy;
class FZibraVDBMaterialComponent;
class FRDGPass;
class USkyAtmosphereComponent;
struct FNativeRenderParameters;

class FZibraVDBMaterialRendering final : public FSceneViewExtensionBase
{
public:
	FZibraVDBMaterialRendering(const FAutoRegister& AutoRegister) noexcept;

	void Init() noexcept;
	void Release() noexcept;

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
	void AddVDBProxy(FZibraVDBMaterialSceneProxy* Proxy, FRHICommandListBase& RHICmdList) noexcept;
#else
	void AddVDBProxy(FZibraVDBMaterialSceneProxy* Proxy) noexcept;
#endif
	void RemoveVDBProxy(FZibraVDBMaterialSceneProxy* Proxy) noexcept;

	//~ Begin ISceneViewExtension Interface
	void SetupViewFamily(FSceneViewFamily& InViewFamily) final;
	void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) final;
	void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) final;
	void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) final;
	int32 GetPriority() const final;
	bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const final;
	//~ End ISceneViewExtension Interface

	struct FZibraVDBGPUProfilerCounters
	{
		FString ZibraVDBName;
		uint64 DecompressionTime;
		uint64 RenderTime;
	};

	static TArray<FZibraVDBGPUProfilerCounters> GPUProfilerCounters;
	TSet<UZibraVDBMaterialComponent*> ActiveZibraVDBs;

private:
	void InitRendering() noexcept;
	void ReleaseRendering() noexcept;

	void UpdateRenderParameters(const FZibraVDBNativeResource::FFrameParameters& FrameHeader, FZibraVDBMaterialSceneProxy* Proxy,
		FZibraVDBNativeResource& RenderResource);
	void UpdateFrameTransformation(FRDGBuilder& GraphBuilder, const FZibraVDBMaterialSceneProxy* Proxy,
		const FZibraVDBNativeResource::FFrameParameters& FrameHeader);
	FRDGPass* ClearTex3DFloat4UAV(FRDGBuilder& GraphBuilder, FZibraVDBNativeResource& RenderResource,
		const FUnorderedAccessViewRHIRef& RenderTextureUAV, const FVector4f& ClearValue, FIntVector Groups, FRDGEventName Label);
	FRDGPass* ClearTex3DFloatUAV(FRDGBuilder& GraphBuilder, FZibraVDBNativeResource& RenderResource,
		const FUnorderedAccessViewRHIRef& RenderTextureUAV, float ClearValue, FIntVector Groups, FRDGEventName Label);
	FRDGPass* ClearTex3DUintUAV(FRDGBuilder& GraphBuilder, FZibraVDBNativeResource& RenderResource,
		const FUnorderedAccessViewRHIRef& RenderTextureUAV, uint ClearValue, FIntVector Groups, FRDGEventName Label);
	void ClearRenderBlockBuffer(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy);

	bool Decompress(
		FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy, const int FrameIndex);
	void MakeBlockList(
		FRDGBuilder& GraphBuilder, const FZibraVDBMaterialSceneProxy* Proxy, FZibraVDBNativeResource& RenderResource);
	void CalculateMaxDensityPerRenderBlock(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
		FZibraVDBNativeResource& RenderResource, uint32_t SpatialBlockCount, bool bEnableInterpolation = true);
	void ComputeRenderBlocks(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
		FZibraVDBNativeResource& RenderResource, uint32_t SpatialBlockCount, bool bIsDensityOnly = false,
		bool bEnableInterpolation = true, bool bHVEnabled = false);

	void ComposeReflectionsVolume(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
		FZibraVDBNativeResource& RenderResource, bool bEnableInterpolation = true);

	static int IntCeilDiv(const int A, const int B) noexcept;

	void ClearFrame(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy, FZibraVDBNativeResource& RenderResource,
		bool bDecompressionHappened);
	void ClearMaxDensityPerBlockTexture(
		FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy, FZibraVDBNativeResource& RenderResource);

	void ClearIlluminatedVolumeTexture(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy, const FIntVector& Resolution,
		FZibraVDBNativeResource& RenderResource);

	void ComputeDownscaledVolume(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy, const FIntVector& Resolution,
		FZibraVDBNativeResource& RenderResource);

	void ComputeDirectShadows(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy, const FIntVector& Resolution,
		FZibraVDBNativeResource& RenderResource, const FSceneView& View);
	void ClearEmissionAndDensity(
		FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy, FZibraVDBNativeResource& RenderResource);

	void PostOpaqueRender(FPostOpaqueRenderParameters& PostOpaqueRenderParameters);

	bool IsLightAffectsVolume(const ULightComponent* LightComponent, const AZibraVDBActor* ZibraVDBActorParent);
	bool IsSkyAtmosphereAffectsLight(USkyAtmosphereComponent* SkyAtmosphere, const ULightComponent* LightComponent);

	static FNativeRenderParameters GetNativeRenderParameters(FZibraVDBNativeResource& RenderResource) noexcept;
	static bool FilterRelevantProxies(const FZibraVDBMaterialSceneProxy* Proxy, const FSceneViewFamily* ViewFamily) noexcept;
	TArray<FZibraVDBMaterialSceneProxy*> ZibraVDBProxies;

#if ZIBRAVDB_ENABLE_PROFILING
	void UpdateProfilerCounters(FZibraVDBMaterialSceneProxy* Proxy, FRDGBuilder& GraphBuilder);
	TUniquePtr<FZibraVDBGPUProfiler> ZibraVDBGPUDecompressionProfiler;
	TUniquePtr<FZibraVDBGPUProfiler> ZibraVDBGPURenderProfiler;
#endif
};
