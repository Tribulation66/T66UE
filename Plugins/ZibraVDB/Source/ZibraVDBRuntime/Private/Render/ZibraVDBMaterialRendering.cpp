// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBMaterialRendering.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Level.h"
#include "LightSceneProxy.h"
#include "Modules/ModuleManager.h"
#include "RenderCore.h"
#include "Rendering/SkyAtmosphereCommonData.h"
#include "ScenePrivate.h"
#include "SceneRendering.h"
#include "SceneView.h"
#include "ShadowRendering.h"
#include "TextureResource.h"
#include "UObject/UObjectIterator.h"
#include "VirtualShadowMaps/VirtualShadowMapClipmap.h"
#include "ZibraVDBActor.h"
#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBMaterialComponent.h"
#include "ZibraVDBMaterialSceneProxy.h"
#include "ZibraVDBNotifications.h"
#include "ZibraVDBRuntime/Private/Profiler/ZibraVDBGPUProfiler.h"
#include "ZibraVDBShaders/Private/ZibraVDBCustomShaders.h"

#include <bit>

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#include "ZibraLicensingManager.h"
#endif

DECLARE_GPU_STAT(ZibraVDB_PostRenderBasePassDeferred);
DECLARE_GPU_STAT(ZibraVDB_PreRenderView);

DECLARE_GPU_STAT(ClearIlluminatedVolume);
DECLARE_GPU_STAT(ClearRenderBlocks);
DECLARE_GPU_STAT(ComputeDirectShadows);
DECLARE_GPU_STAT(CalculateMaxDensityPerRenderBlock);
DECLARE_GPU_STAT(ComputeRenderBlocks);
DECLARE_GPU_STAT(ComputeRenderBlocksHV);
DECLARE_GPU_STAT(ComposeReflectionsVolume);
DECLARE_GPU_STAT(Decompress);
DECLARE_GPU_STAT(MakeBlockList);
DECLARE_GPU_STAT(VolumeDownscale);

#if WITH_EDITOR
IMPLEMENT_STATIC_UNIFORM_BUFFER_STRUCT(FVirtualShadowMapUniformParameters, "ZibraVirtualShadowMap2", VirtualShadowMapUbSlot);
#endif

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 506
#define CREATE_TEXTURE_3D_SRV(RHICmdList, Texture) \
	RHICmdList.CreateShaderResourceView(Texture, FRHIViewDesc::CreateTextureSRV().SetDimensionFromTexture(Texture))
#define CREATE_TEXTURE_3D_UAV(RHICmdList, Texture) \
	RHICmdList.CreateUnorderedAccessView(Texture, FRHIViewDesc::CreateTextureUAV().SetDimensionFromTexture(Texture))
#else
#define CREATE_TEXTURE_3D_SRV(RHICmdList, Texture) RHICmdList.CreateShaderResourceView(Texture, 0)
#define CREATE_TEXTURE_3D_UAV(RHICmdList, Texture) RHICmdList.CreateUnorderedAccessView(Texture)
#endif
#define CREATE_AND_STORE_TEXTURE_3D(Name, Resolution, Type, InitialState)                                                          \
	if (!Proxy->Has##Name##Texture() || Proxy->Get##Name##Texture()->GetSizeXYZ() != Resolution)                                   \
	{                                                                                                                              \
		Proxy->Release##Name();                                                                                                    \
		FZibraTexture3DRHIRef Name = ZibraVDB::Utils::CreateTexture3D(TEXT(#Name), Resolution.X, Resolution.Y, Resolution.Z, Type, \
			1, TexCreate_UAV | TexCreate_ShaderResource, InitialState);                                                            \
		FShaderResourceViewRHIRef Name##SRV = CREATE_TEXTURE_3D_SRV(GraphBuilder.RHICmdList, Name);                                \
		FUnorderedAccessViewRHIRef Name##UAV = CREATE_TEXTURE_3D_UAV(GraphBuilder.RHICmdList, Name);                               \
		Proxy->Set##Name(MoveTemp(Name), MoveTemp(Name##SRV), MoveTemp(Name##UAV));                                                \
		Proxy->Set##Name##CurrentState(InitialState);                                                                              \
	}

TArray<FZibraVDBMaterialRendering::FZibraVDBGPUProfilerCounters> FZibraVDBMaterialRendering::GPUProfilerCounters;

template <typename T>
static void InsertTransition(TArray<FRHITransitionInfo>& TransitionsArray, T& Buffer, ERHIAccess DesiredState)
{
	if (Buffer.GetCurrentState() != DesiredState || DesiredState == ERHIAccess::UAVMask)
	{
		TransitionsArray.Emplace(Buffer.Buffer, Buffer.GetCurrentState(), DesiredState);
		Buffer.SetCurrentState(DesiredState);
	}
}

#define TRANSITION_3D_TEXTURE(TransitionArray, Name, DesiredState)                                            \
	if (Proxy->Get##Name##CurrentState() != DesiredState || DesiredState == ERHIAccess::UAVMask)              \
	{                                                                                                         \
		TransitionArray.Emplace(Proxy->Get##Name##Texture(), Proxy->Get##Name##CurrentState(), DesiredState); \
		Proxy->Set##Name##CurrentState(DesiredState);                                                         \
	}

#define TRANSITION_PROXY_RESOURCE(TransitionArray, Name, DesiredState) \
	TransitionArray.Emplace(Proxy->Get##Name##UAV(), ERHIAccess::Unknown, DesiredState);

FZibraVDBMaterialRendering::FZibraVDBMaterialRendering(const FAutoRegister& AutoRegister) noexcept
	: FSceneViewExtensionBase(AutoRegister)
#if ZIBRAVDB_ENABLE_PROFILING
	, ZibraVDBGPUDecompressionProfiler(MakeUnique<FZibraVDBGPUProfiler>())
	, ZibraVDBGPURenderProfiler(MakeUnique<FZibraVDBGPUProfiler>())
#endif
{
	Init();

	IRendererModule& Renderer = FModuleManager::LoadModuleChecked<IRendererModule>("Renderer");
	Renderer.RegisterPostOpaqueRenderDelegate(FPostOpaqueRenderDelegate::CreateRaw(this, &FZibraVDBMaterialRendering::PostOpaqueRender));
}

void FZibraVDBMaterialRendering::InitRendering() noexcept
{
	check(IsInRenderingThread());

	ReleaseRendering();
}

void FZibraVDBMaterialRendering::ReleaseRendering() noexcept
{
	check(IsInRenderingThread());
}

void FZibraVDBMaterialRendering::Init() noexcept
{
	if (IsInRenderingThread())
	{
		InitRendering();
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(InitVDBRendering)([this](FRHICommandListImmediate& RHICmdList) { Init(); });
	}
}

void FZibraVDBMaterialRendering::Release() noexcept
{
	if (IsInRenderingThread())
	{
		ReleaseRendering();
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(ReleaseVDBRendering)([this](FRHICommandListImmediate& RHICmdList) { Release(); });
	}
}

int FZibraVDBMaterialRendering::IntCeilDiv(const int A, const int B) noexcept
{
	return (A + B - 1) / B;
}

void FZibraVDBMaterialRendering::UpdateRenderParameters(const FZibraVDBNativeResource::FFrameParameters& FrameHeader,
	FZibraVDBMaterialSceneProxy* Proxy, FZibraVDBNativeResource& RenderResource)
{
	const FIntVector BlockGridSize = FrameHeader.AABBSize;
	const uint32_t SpatialBlockCount = FrameHeader.SpatialBlockCount;
	const FIntVector InputTextureSize = BlockGridSize * ZIBRAVDB_BLOCK_SIZE;

	const FIntVector TextureSize = InputTextureSize;
	const FIntVector NumGroups = FIntVector::DivideAndRoundUp(TextureSize, ZIBRAVDB_BLOCK_SIZE);

	RenderResource.RenderParameters.BlockGridSize = BlockGridSize;
	RenderResource.RenderParameters.NumGroups = NumGroups;
	RenderResource.RenderParameters.SpacialBlockCount = SpatialBlockCount;
	RenderResource.RenderParameters.ChannelBlockCount = FrameHeader.ChannelBlockCount;

	const FString DensityChannelName = Proxy->GetAssetComponent()->GetZibraVDBVolume()->DensityChannel;
	const FString TemperatureChannelName = Proxy->GetAssetComponent()->GetZibraVDBVolume()->TemperatureChannel;
	const FString FlamesChannelName = Proxy->GetAssetComponent()->GetZibraVDBVolume()->FlamesChannel;
	const int32 DensityChannelIndex = FrameHeader.ChannelNames.Find(DensityChannelName);
	const int32 TemperatureChannelIndex = FrameHeader.ChannelNames.Find(TemperatureChannelName);
	const int32 FlamesChannelIndex = FrameHeader.ChannelNames.Find(FlamesChannelName);

	RenderResource.RenderParameters.DensityChannelMask = (DensityChannelIndex != INDEX_NONE) ? (1 << DensityChannelIndex) : 0;
	RenderResource.RenderParameters.TemperatureChannelMask =
		(TemperatureChannelIndex != INDEX_NONE) ? (1 << TemperatureChannelIndex) : 0;
	RenderResource.RenderParameters.FlamesChannelMask = (FlamesChannelIndex != INDEX_NONE) ? (1 << FlamesChannelIndex) : 0;

	RenderResource.RenderParameters.DensityChannelScale =
		Proxy->DensityChannelIndex != INDEX_NONE
			? (1.f / RenderResource.RenderParameters.SequenceChannelScales[Proxy->DensityChannelIndex])
			: 0.0f;
	RenderResource.RenderParameters.TemperatureChannelScale =
		Proxy->TemperatureChannelIndex != INDEX_NONE
			? (1.f / RenderResource.RenderParameters.SequenceChannelScales[Proxy->TemperatureChannelIndex])
			: 0.0f;
	RenderResource.RenderParameters.FlamesChannelScale =
		Proxy->FlamesChannelIndex != INDEX_NONE
			? (1.f / RenderResource.RenderParameters.SequenceChannelScales[Proxy->FlamesChannelIndex])
			: 0.0f;
}

void FZibraVDBMaterialRendering::UpdateFrameTransformation(FRDGBuilder& GraphBuilder, const FZibraVDBMaterialSceneProxy* Proxy,
	const FZibraVDBNativeResource::FFrameParameters& FrameHeader)
{
	FMatrix44f LocalToWorld = FMatrix44f(Proxy->GetLocalToWorld());
	FMatrix44f WorldToLocal = LocalToWorld.Inverse();

	FMatrix44f FrameToSequenceTransform = Proxy->GetFrameToSequenceCoordsTransform();

	FVector3f TextureSize = FVector3f(Proxy->GetNativeResources()->RenderParameters.TextureSize);
	FMatrix44f EffectScale =
		FScaleMatrix44f(FVector3f::One() / (TextureSize * Proxy->GetCurrentFrameHeader().IndexToUnrealUnitsScale));

	FMatrix44f WorldToBox = WorldToLocal * FrameToSequenceTransform.Inverse() * EffectScale;

	FTextureRHIRef TransformBufferTexture = Proxy->GetMaterialComponent()->GetTransformBuffer()->GetResource()->GetTextureRHI();

	FUpdateTextureRegion3D TextureRegion{FIntVector{0, 0, 0}, FIntVector{0, 0, 0}, FIntVector{4, 1, 1}};
	GraphBuilder.RHICmdList.UpdateTexture3D(
		TransformBufferTexture, 0, TextureRegion, 64, 64, reinterpret_cast<const uint8*>(WorldToBox.M));
}

#if ZIBRAVDB_ENABLE_PROFILING
void FZibraVDBMaterialRendering::UpdateProfilerCounters(FZibraVDBMaterialSceneProxy* Proxy, FRDGBuilder& GraphBuilder)
{
	FZibraVDBGPUProfilerCounters ZibraVDBGPUProfilerCounters{};
	ZibraVDBGPUProfilerCounters.ZibraVDBName = Proxy->GetMaterialComponent()->GetOuter()->GetName();

	uint64 DurationMicroseconds = 0;
	if (ZibraVDBGPUDecompressionProfiler->TryGetFrameResult(GraphBuilder.RHICmdList, DurationMicroseconds))
	{
		ZibraVDBGPUProfilerCounters.DecompressionTime = DurationMicroseconds;
	}
	if (ZibraVDBGPURenderProfiler->TryGetFrameResult(GraphBuilder.RHICmdList, DurationMicroseconds))
	{
		ZibraVDBGPUProfilerCounters.RenderTime = DurationMicroseconds;
	}

	GPUProfilerCounters.Add(ZibraVDBGPUProfilerCounters);
}

#endif

void FZibraVDBMaterialRendering::ClearIlluminatedVolumeTexture(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
	const FIntVector& Resolution, FZibraVDBNativeResource& RenderResource)
{
	auto IlluminatedVolume = Proxy->GetIlluminatedVolumeTextureUAV();
	if (IlluminatedVolume.IsValid())
	{
		RDG_EVENT_SCOPE(GraphBuilder, "ClearIlluminatedVolume");
		RDG_GPU_STAT_SCOPE(GraphBuilder, ClearIlluminatedVolume);
		SCOPED_NAMED_EVENT(ClearIlluminatedVolume, FColor::Purple);
		const FIntVector Groups = FIntVector(IntCeilDiv(IlluminatedVolume->GetTexture()->GetSizeX(), ZIBRAVDB_BLOCK_SIZE),
			IntCeilDiv(IlluminatedVolume->GetTexture()->GetSizeY(), ZIBRAVDB_BLOCK_SIZE),
			IntCeilDiv(IlluminatedVolume->GetTexture()->GetSizeZ(), ZIBRAVDB_BLOCK_SIZE));
		ClearTex3DFloat4UAV(
			GraphBuilder, RenderResource, IlluminatedVolume, FVector4f::Zero(), Groups, RDG_EVENT_NAME("ClearIlluminatedVolume"));
	}
}

void FZibraVDBMaterialRendering::ComputeDownscaledVolume(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
	const FIntVector& Resolution, FZibraVDBNativeResource& RenderResource)
{
	RDG_EVENT_SCOPE(GraphBuilder, "VolumeDownscale");
	RDG_GPU_STAT_SCOPE(GraphBuilder, VolumeDownscale);
	SCOPED_NAMED_EVENT(VolumeDownscale, FColor::Purple);

	const FIntVector IllumResolution = FIntVector::DivideAndRoundUp(Resolution, Proxy->IlluminationDownscaleFactor);

	// Downscale the volume texture to the given resolution.
	FVolumeDownscale::FParameters* PassParameters = GraphBuilder.AllocParameters<FVolumeDownscale::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;

	CREATE_AND_STORE_TEXTURE_3D(ZibraDensityTextureLOD, IllumResolution, PF_R16F, ERHIAccess::UAVMask);

	PassParameters->Input = RenderResource.DecompressionPerChannelBlockData.BufferSRV;
	InsertTransition(PassTransitions, RenderResource.DecompressionPerChannelBlockData, ERHIAccess::SRVMask);
	PassParameters->BlockIndexTexture = Proxy->GetBlockIndexTextureSRV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, BlockIndexTexture, ERHIAccess::SRVMask);
	PassParameters->BlockBuffer = RenderResource.RenderBlockBuffer.BufferSRV;
	InsertTransition(PassTransitions, RenderResource.RenderBlockBuffer, ERHIAccess::SRVMask);
	PassParameters->Output = Proxy->GetZibraDensityTextureLODUAV();
	TRANSITION_3D_TEXTURE(PassTransitions, ZibraDensityTextureLOD, ERHIAccess::UAVMask);

	PassParameters->BlockGridSize = FIntVector4(RenderResource.RenderParameters.BlockGridSize, 1);
	PassParameters->DownscaleFactor = Proxy->IlluminationDownscaleFactor;
	PassParameters->DownscaledBlockSize = ZIBRAVDB_BLOCK_SIZE / PassParameters->DownscaleFactor;
	PassParameters->DensityChannelMask = RenderResource.RenderParameters.DensityChannelMask;

	GraphBuilder.AddPass(RDG_EVENT_NAME("VolumeDownscale"), PassParameters, ERDGPassFlags::Compute,
		[=](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.Transition(PassTransitions);
			FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			TShaderMapRef<FVolumeDownscale> VDBShader(GlobalShaderMap);
			FIntVector DispatchCount = FComputeShaderUtils::GetGroupCount(IllumResolution, FIntVector(ZIBRAVDB_BLOCK_SIZE));
			FComputeShaderUtils::Dispatch(RHICmdList, VDBShader, *PassParameters, DispatchCount);
		});
}

static float CalculateLightIntensityInCandelas(const ULightComponent* const LightComponent, FZibraVDBMaterialSceneProxy* Proxy)
{
	float LightIntensity = LightComponent->GetLightUnits() == ELightUnits::EV ? FMath::Pow(2.0f, LightComponent->Intensity)
																			  : LightComponent->Intensity;
	switch (LightComponent->GetLightType())
	{
		case ELightComponentType::LightType_Directional:
			return LightIntensity * Proxy->DirectionalLightBrightness;
			break;
		case ELightComponentType::LightType_Point:
			LightIntensity *= Proxy->PointLightBrightness * ULocalLightComponent::GetUnitsConversionFactor(
																LightComponent->GetLightUnits(), ELightUnits::Candelas);
			break;
		case ELightComponentType::LightType_Spot:
		{
			const USpotLightComponent* SpotLightComponent = Cast<USpotLightComponent>(LightComponent);
			LightIntensity *=
				Proxy->SpotLightBrightness * ULocalLightComponent::GetUnitsConversionFactor(LightComponent->GetLightUnits(),
												 ELightUnits::Candelas, SpotLightComponent->GetCosHalfConeAngle());
			break;
		}
		case ELightComponentType::LightType_Rect:
			LightIntensity *= Proxy->RectLightBrightness * ULocalLightComponent::GetUnitsConversionFactor(
																LightComponent->GetLightUnits(), ELightUnits::Candelas);
			break;
	}
	return LightIntensity;
}

void FZibraVDBMaterialRendering::ComputeDirectShadows(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
	const FIntVector& Resolution, FZibraVDBNativeResource& RenderResource, const FSceneView& View)
{
	RDG_EVENT_SCOPE(GraphBuilder, "ComputeDirectShadows");
	RDG_GPU_STAT_SCOPE(GraphBuilder, ComputeDirectShadows);
	SCOPED_NAMED_EVENT(ComputeDirectShadows, FColor::Purple);

	const FIntVector IllumResolution = FIntVector::DivideAndRoundUp(Resolution, Proxy->IlluminationDownscaleFactor);

	// Shadow ray pass for each illumination grid voxel.
	FVolumeShadowRay::FParameters* PassParameters = GraphBuilder.AllocParameters<FVolumeShadowRay::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;

	// Initialize the illumination grid (direct shadows).
	CREATE_AND_STORE_TEXTURE_3D(ZibraIllumination, IllumResolution, PF_FloatRGBA, ERHIAccess::UAVMask);

	PassParameters->Density = Proxy->GetZibraDensityTextureLODSRV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, ZibraDensityTextureLOD, ERHIAccess::SRVMask);
	PassParameters->IlluminationOUT = Proxy->GetZibraIlluminationUAV();
	TRANSITION_3D_TEXTURE(PassTransitions, ZibraIllumination, ERHIAccess::UAVMask);
	PassParameters->TextureSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	PassParameters->VSMPointSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	const FMatrix44f WorldToGrid = Proxy->GetWorldToIlluminationGridMatrix();

	PassParameters->Light_Position_AttenuationRadius = {};
	PassParameters->Light_PositionWS_EnableReceivedShadow = {};
	PassParameters->Light_Color_EnableRaymarch = {};
	PassParameters->Light_Direction_RadiusLimit = {};
	PassParameters->Light_RectData = {};
	PassParameters->Light_RectTangent = {};
	PassParameters->Light_SourceRadius = {};
	PassParameters->Light_VSM_IDS = {};
	for (int32 i = 0; i < ZIB_MAX_LIGHT_COUNT; ++i)
	{
		GET_SCALAR_ARRAY_ELEMENT(PassParameters->Light_VSM_IDS, i) = INDEX_NONE;
	}

	// Go over all Lights and add them to the shader array.
	size_t InIndex = 0;
	int OutIndex = -1;
	size_t LightCount = 0;
	float MinInfluence = FLT_MAX;
	size_t MinInfluenceIndex = 0;
	TStaticArray<float, ZIB_MAX_LIGHT_COUNT> InfluenceArray;
	const AZibraVDBActor* ZibraVDBActorParent = Cast<AZibraVDBActor>(Proxy->GetMaterialComponent()->GetOuter());
	if (!ZibraVDBActorParent)
	{
		return;
	}

	const FBoxSphereBounds ZibraVDBActorBBox = ZibraVDBActorParent->GetBBox();
	if (ZibraVDBActorBBox.BoxExtent.IsNearlyZero())
	{
		return;
	}

	// Get scene and renderer for VSM lookups
	const FSceneViewFamily& ViewFamily = *View.Family;
	const FSceneRenderer* SceneRenderer = static_cast<const FSceneRenderer*>(ViewFamily.GetSceneRenderer());
	FScene* Scene = ViewFamily.Scene ? static_cast<FScene*>(ViewFamily.Scene) : nullptr;
	check(View.bIsViewInfo);
	const FViewInfo& ViewInfo = static_cast<const FViewInfo&>(View);
	for (TObjectIterator<ULightComponent> Itr; Itr; ++Itr)
	{
		FVector3f LightPositionWS;

		const ULightComponent* LightComponent = *Itr;

		if (!IsLightAffectsVolume(LightComponent, ZibraVDBActorParent))
		{
			continue;
		}

		// Get light color. If AtmosphereSunLight is enabled muliply it by SunLightAtmosphereTransmittance.
		FLinearColor LightColor = LightComponent->LightColor;
		if (LightComponent->bUseTemperature)
		{
			LightColor *= LightComponent->GetColorTemperature();
		}

		if (LightComponent->IsUsedAsAtmosphereSunLight())
		{
			for (USkyAtmosphereComponent* SkyAtmosphere : TObjectRange<USkyAtmosphereComponent>(
					 RF_ClassDefaultObject | RF_ArchetypeObject, true, EInternalObjectFlags::Garbage))
			{
				if (IsSkyAtmosphereAffectsLight(SkyAtmosphere, LightComponent))
				{
					FAtmosphereSetup AtmosphereSetup(*SkyAtmosphere);
					FLinearColor SunLightAtmosphereTransmittance =
						AtmosphereSetup.GetTransmittanceAtGroundLevel(-LightComponent->GetDirection());
					LightColor *= SunLightAtmosphereTransmittance;
					break;	  // We only register the first we find
				}
			}
		}
		// Get light data.
		FVector3f LightPosition = FVector3f(LightComponent->GetComponentLocation());
		float LightIntensity = CalculateLightIntensityInCandelas(LightComponent, Proxy);

		FVector3f LightDirection = FVector3f(LightComponent->GetDirection());
		const bool CastShadows = LightComponent->CastShadows;

		float AttenuationRadius;
		float InnerRadiusLimit;
		float OuterRadiusLimit;
		float LightSourceRadius;
		uint RadiusLimit;

		int EnableSelfShadowRayMarch = 1;
		int EnableReceivedShadow = 1;

		switch (LightComponent->GetLightType())
		{
			case ELightComponentType::LightType_Directional:
			{
				if (FMath::IsNearlyZero(Proxy->DirectionalLightBrightness)) continue;
				const UDirectionalLightComponent* DirectionalLightComponent = Cast<UDirectionalLightComponent>(LightComponent);

				AttenuationRadius = -1.0f;
				InnerRadiusLimit = -1.0f;
				OuterRadiusLimit = -1.0f;
				LightSourceRadius = FMath::Tan(FMath::DegreesToRadians(DirectionalLightComponent->LightSourceAngle));

				LightPositionWS = FVector3f(-1.f, -1.f, -1.f);
				LightPosition = FVector3f(-1.f, -1.f, -1.f);
				LightColor *= LightIntensity;
				LightDirection = LightDirection.GetSafeNormal();
				EnableSelfShadowRayMarch = Proxy->EnableDirectionalLightShadows;
				EnableReceivedShadow = Proxy->EnableDirectionalLightReceivedShadows && CastShadows;
				AttenuationRadius = -1.0f;
				RadiusLimit = -1;
			}
			break;
			case ELightComponentType::LightType_Point:
			{
				if (FMath::IsNearlyZero(Proxy->PointLightBrightness)) continue;
				const UPointLightComponent* PointLightComponent = Cast<UPointLightComponent>(LightComponent);

				AttenuationRadius = PointLightComponent->AttenuationRadius;
				InnerRadiusLimit = -1.0f;
				OuterRadiusLimit = -1.0f;
				LightSourceRadius = PointLightComponent->SourceRadius;

				LightPositionWS = LightPosition;
				LightPosition = WorldToGrid.TransformPosition(LightPositionWS);
				LightDirection = FVector3f(-1.f, -1.f, -1.f);
				LightColor *= LightIntensity;
				EnableSelfShadowRayMarch = Proxy->EnablePointLightShadows;
				EnableReceivedShadow = Proxy->EnablePointLightReceivedShadows && CastShadows;
				RadiusLimit = -1;
			}
			break;
			case ELightComponentType::LightType_Spot:
			{
				if (FMath::IsNearlyZero(Proxy->SpotLightBrightness)) continue;
				const USpotLightComponent* SpotLightComponent = Cast<USpotLightComponent>(LightComponent);

				AttenuationRadius = SpotLightComponent->AttenuationRadius;
				InnerRadiusLimit = FMath::Cos(FMath::DegreesToRadians(SpotLightComponent->InnerConeAngle));
				OuterRadiusLimit = FMath::Cos(FMath::DegreesToRadians(SpotLightComponent->OuterConeAngle));
				LightSourceRadius = SpotLightComponent->SourceRadius;

				LightPositionWS = LightPosition;
				LightPosition = WorldToGrid.TransformPosition(LightPositionWS);
				LightDirection = LightDirection.GetSafeNormal();
				LightColor *= LightIntensity;
				EnableSelfShadowRayMarch = Proxy->EnableSpotLightShadows;
				EnableReceivedShadow = Proxy->EnableSpotLightReceivedShadows && CastShadows;
				// Pack inner and outer cosinus of radius limit into 32 bits.
				RadiusLimit =
					(ZibraVDB::Utils::CosToUint16(InnerRadiusLimit) << 16 | ZibraVDB::Utils::CosToUint16(OuterRadiusLimit));
			}
			break;
			case ELightComponentType::LightType_Rect:
			{
				if (FMath::IsNearlyZero(Proxy->RectLightBrightness)) continue;
				const URectLightComponent* RectLightComponent = Cast<URectLightComponent>(LightComponent);
				AttenuationRadius = RectLightComponent->AttenuationRadius;
				InnerRadiusLimit = -1.0f;
				OuterRadiusLimit = -1.0f;
				LightSourceRadius = 0.0f;
				LightPositionWS = LightPosition;
				LightPosition = WorldToGrid.TransformPosition(LightPositionWS);
				LightDirection = LightDirection.GetSafeNormal();
				LightColor *= LightIntensity;
				EnableSelfShadowRayMarch = Proxy->EnableRectLightShadows;
				EnableReceivedShadow = Proxy->EnableRectLightReceivedShadows && CastShadows;
				RadiusLimit = -1;
			}
			break;
			default:
				continue;
		}

		// Skip near zero intensities and colors.
		if (FMath::IsNearlyZero(LightIntensity) ||
			(FMath::IsNearlyZero(LightColor.R) && FMath::IsNearlyZero(LightColor.G) && FMath::IsNearlyZero(LightColor.B)))
			continue;

		float LightInfluence = LightComponent->GetLightType() == LightType_Directional
								   ? FLT_MAX
								   : LightIntensity / (LightPosition.Length() * LightPosition.Length());
		if (LightCount < ZIB_MAX_LIGHT_COUNT)
		{
			++OutIndex;
			++LightCount;
			if (LightInfluence < MinInfluence)
			{
				MinInfluence = LightInfluence;
				MinInfluenceIndex = OutIndex;
			}
			InfluenceArray[OutIndex] = LightInfluence;
		}
		else
		{
			if (LightInfluence < MinInfluence)
			{
				continue;
			}
			OutIndex = MinInfluenceIndex;
			InfluenceArray[MinInfluenceIndex] = LightInfluence;
			auto* MinElement = Algo::MinElement(InfluenceArray);
			MinInfluence = *MinElement;
			MinInfluenceIndex = MinElement - InfluenceArray.GetData();
		}
		PassParameters->Light_Position_AttenuationRadius[OutIndex] = FVector4f(LightPosition, AttenuationRadius);
		PassParameters->Light_PositionWS_EnableReceivedShadow[OutIndex] =
			FVector4f(LightPositionWS, EnableReceivedShadow);
		PassParameters->Light_Color_EnableRaymarch[OutIndex] =
			FVector4f(LightColor, std::bit_cast<float>(EnableSelfShadowRayMarch));
		PassParameters->Light_Direction_RadiusLimit[OutIndex] = FVector4f(LightDirection, std::bit_cast<float>(RadiusLimit));
		GET_SCALAR_ARRAY_ELEMENT(PassParameters->Light_SourceRadius, OutIndex) = LightSourceRadius;

		if (LightComponent->GetLightType() == ELightComponentType::LightType_Rect)
		{
			const URectLightComponent* RectLightComponent = Cast<URectLightComponent>(LightComponent);
			PassParameters->Light_RectData[OutIndex] = FVector4f(
				RectLightComponent->SourceWidth,
				RectLightComponent->SourceHeight,
				FMath::Cos(FMath::DegreesToRadians(RectLightComponent->BarnDoorAngle)),
				RectLightComponent->BarnDoorLength);
			FVector3f RectTangent = -FVector3f(RectLightComponent->GetComponentTransform().GetUnitAxis(EAxis::Z));
			PassParameters->Light_RectTangent[OutIndex] = FVector4f(RectTangent, 0.0f);
		}

		// Assign VSM ID for this light
		int32 AssignedVSMId = INDEX_NONE;
		if (SceneRenderer && Scene)
		{
			for (auto It = Scene->Lights.CreateConstIterator(); It; ++It)
			{
				int32 SceneLightIdx = It.GetIndex();
				const FLightSceneInfoCompact& SceneLightInfo = *It;
				if (SceneLightIdx >= SceneRenderer->VisibleLightInfos.Num() ||
					!SceneLightInfo.LightSceneInfo ||
					!SceneLightInfo.LightSceneInfo->Proxy ||
					SceneLightInfo.LightSceneInfo->Proxy->GetLightComponent() != LightComponent)
				{
					continue;
				}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 7
				const FVisibleLightInfo& VisibleLightInfo = SceneRenderer->VisibleLightInfos[SceneLightIdx];

				// For directional lights with clipmaps, e.g. the directional light
				if (VisibleLightInfo.VirtualShadowMapClipmaps.Num() > 0)
				{
					const TSharedPtr<FVirtualShadowMapClipmap>* FoundClipmap = Algo::FindByPredicate(
							VisibleLightInfo.VirtualShadowMapClipmaps,
							[&ViewInfo](const TSharedPtr<FVirtualShadowMapClipmap>& Clipmap)
							{
								return Clipmap.IsValid() && Clipmap->GetDependentView() == &ViewInfo;
							}
					);
					// Fallback - use first clipmap if no match found
					if (!FoundClipmap)
					{
						FoundClipmap = &VisibleLightInfo.VirtualShadowMapClipmaps[0];
					}
					if (FoundClipmap && FoundClipmap->IsValid())
					{
						AssignedVSMId = (*FoundClipmap)->GetVirtualShadowMapId();
						break;
					}
				}
				else
				{
					// For local lights (point, spot), look through projected shadows
					// FIXME this isn't correct. works just for single 'visible' light in scene
					for (int32 ShadowIndex = 0; ShadowIndex < VisibleLightInfo.AllProjectedShadows.Num(); ShadowIndex++)
					{
						const FProjectedShadowInfo* ProjectedShadowInfo = VisibleLightInfo.AllProjectedShadows[ShadowIndex];
						if (ProjectedShadowInfo && ProjectedShadowInfo->VirtualShadowMapPerLightCacheEntry.IsValid())
						{
							AssignedVSMId = ProjectedShadowInfo->VirtualShadowMapPerLightCacheEntry->GetVirtualShadowMapId();
							break;
						}
					}
				}
#else
				AssignedVSMId = SceneRenderer->VisibleLightInfos[SceneLightIdx].VirtualShadowMapId;
				if (AssignedVSMId != INDEX_NONE)
				{
					break;
				}
#endif
			}
		}
		GET_SCALAR_ARRAY_ELEMENT(PassParameters->Light_VSM_IDS, OutIndex) = AssignedVSMId;
	}

	PassParameters->LightCount = LightCount;
	PassParameters->StepSize = Proxy->RayMarchingSelfShadowStepSize;
	PassParameters->MaxStepCount = Proxy->RayMarchingMaxSelfShadowSteps;
	PassParameters->ShadowIntensity =
		Proxy->ShadowIntensity * (Proxy->DefaultIlluminationResolution / Proxy->IlluminationResolution);
	PassParameters->SmoothReceivedShadows = Proxy->SmoothReceivedShadows ? 1 : 0;
	PassParameters->AbsorptionColor = FVector3f(Proxy->AbsorptionColor.R, Proxy->AbsorptionColor.G, Proxy->AbsorptionColor.B);
	PassParameters->DensityScale = Proxy->DensityScale * RenderResource.RenderParameters.DensityChannelScale;

	PassParameters->WorldToBox = WorldToGrid;
	PassParameters->BoxToWorld = WorldToGrid.Inverse();
	PassParameters->CameraPositionWS = FVector3f(View.ViewLocation);

	PassParameters->GridSize = FVector3f(IllumResolution);
	PassParameters->View = GetShaderBinding(View.ViewUniformBuffer);
	PassParameters->AmbientLightingMode = Proxy->AmbientLightingMode;
	PassParameters->AmbientLightIntensity = Proxy->AmbientLightIntensity;
	PassParameters->AmbientLightingColor =
		FVector3f(Proxy->AmbientLightingColor.R, Proxy->AmbientLightingColor.G, Proxy->AmbientLightingColor.B);
	PassParameters->AOShadowIntensity = Proxy->AOShadowIntensity;
	PassParameters->AORadius = Proxy->AORadius;
	PassParameters->AORayMarcherStepCount = Proxy->AORayMarcherStepCount;

	if (SceneRenderer)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
		PassParameters->VirtualShadowMap = SceneRenderer->VirtualShadowMapArray.GetUniformBuffer(ViewInfo.SceneRendererPrimaryViewId);
#elif ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
		PassParameters->VirtualShadowMap = SceneRenderer->VirtualShadowMapArray.GetUniformBuffer(ViewInfo.GPUSceneViewId);
#else
		PassParameters->VirtualShadowMap = SceneRenderer->VirtualShadowMapArray.GetUniformBuffer();
#endif
	}

	GraphBuilder.AddPass(RDG_EVENT_NAME("VolumeShadowRay"), PassParameters, ERDGPassFlags::Compute,
		[=](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.Transition(PassTransitions);
			FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			TShaderMapRef<FVolumeShadowRay> VdbShader(GlobalShaderMap);
			FIntVector DispatchCount = FComputeShaderUtils::GetGroupCount(IllumResolution, FIntVector(ZIBRAVDB_BLOCK_SIZE));
			FComputeShaderUtils::Dispatch(RHICmdList, VdbShader, *PassParameters, DispatchCount);
		});
}

bool FZibraVDBMaterialRendering::IsLightAffectsVolume(
	const ULightComponent* LightComponent, const AZibraVDBActor* ZibraVDBActorParent)
{
	if (!IsValid(LightComponent))
	{
		return false;
	}

	if (LightComponent->GetWorld() != ZibraVDBActorParent->GetWorld() || !LightComponent->bAffectsWorld ||
		!LightComponent->IsVisible() || !LightComponent->CanEverRender())
	{
		return false;
	}

	if (LightComponent->GetOwner() != nullptr && LightComponent->GetOwner()->GetLevel() != nullptr &&
		!LightComponent->GetOwner()->GetLevel()->bIsVisible)
	{
		return false;
	}

	const FBoxSphereBounds ZibraVDBActorBBox = ZibraVDBActorParent->GetBBox();

	if (!LightComponent->AffectsBounds(ZibraVDBActorBBox))
	{
		return false;
	}

	const FLightingChannels& ZibraVDBLightingChannels = ZibraVDBActorParent->GetMaterialComponent()->VDBLightingChannels;

	if ((GetLightingChannelMaskForStruct(LightComponent->LightingChannels) &
			GetLightingChannelMaskForStruct(ZibraVDBLightingChannels)) == 0)
	{
		return false;
	}

	return true;
}

bool FZibraVDBMaterialRendering::IsSkyAtmosphereAffectsLight(
	USkyAtmosphereComponent* SkyAtmosphere, const ULightComponent* LightComponent)
{
	if (!IsValid(SkyAtmosphere))
	{
		return false;
	}

	if (SkyAtmosphere->GetWorld() != LightComponent->GetWorld() || !SkyAtmosphere->IsVisible() || !SkyAtmosphere->CanEverRender())
	{
		return false;
	}

	if (SkyAtmosphere->GetOwner() != nullptr && SkyAtmosphere->GetOwner()->GetLevel() != nullptr &&
		!SkyAtmosphere->GetOwner()->GetLevel()->bIsVisible)
	{
		return false;
	}

	return true;
}

FRDGPass* FZibraVDBMaterialRendering::ClearTex3DFloat4UAV(FRDGBuilder& GraphBuilder, FZibraVDBNativeResource& RenderResource,
	const FUnorderedAccessViewRHIRef& RenderTextureUAV, const FVector4f& ClearValue, FIntVector Groups, FRDGEventName Label)
{
	if (!RenderTextureUAV.IsValid())
	{
		return nullptr;
	}
	FClearTex3DFloat4::FParameters* PassParameters = GraphBuilder.AllocParameters<FClearTex3DFloat4::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;
	PassTransitions.Emplace(RenderTextureUAV, ERHIAccess::Unknown, ERHIAccess::UAVMask);

	PassParameters->Tex3DFloat4UAV = RenderTextureUAV;
	PassParameters->Tex3DFloat4ClearValue = ClearValue;
	return GraphBuilder.AddPass(MoveTemp(Label), PassParameters, ERDGPassFlags::Compute,
		[=](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.Transition(PassTransitions);

			FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			TShaderMapRef<FClearTex3DFloat4> VDBShader(GlobalShaderMap);

			FComputeShaderUtils::Dispatch(RHICmdList, VDBShader, *PassParameters, Groups);
		});
}

FRDGPass* FZibraVDBMaterialRendering::ClearTex3DFloatUAV(FRDGBuilder& GraphBuilder, FZibraVDBNativeResource& RenderResource,
	const FUnorderedAccessViewRHIRef& RenderTextureUAV, float ClearValue, FIntVector Groups, FRDGEventName Label)
{
	if (!RenderTextureUAV.IsValid())
	{
		return nullptr;
	}
	FClearTex3DFloat::FParameters* PassParameters = GraphBuilder.AllocParameters<FClearTex3DFloat::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;
	PassTransitions.Emplace(RenderTextureUAV, ERHIAccess::Unknown, ERHIAccess::UAVMask);

	PassParameters->Tex3DFloatUAV = RenderTextureUAV;
	PassParameters->Tex3DFloatClearValue = ClearValue;
	return GraphBuilder.AddPass(MoveTemp(Label), PassParameters, ERDGPassFlags::Compute,
		[=](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.Transition(PassTransitions);

			FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			TShaderMapRef<FClearTex3DFloat> VDBShader(GlobalShaderMap);

			FComputeShaderUtils::Dispatch(RHICmdList, VDBShader, *PassParameters, Groups);
		});
}

FRDGPass* FZibraVDBMaterialRendering::ClearTex3DUintUAV(FRDGBuilder& GraphBuilder, FZibraVDBNativeResource& RenderResource,
	const FUnorderedAccessViewRHIRef& RenderTextureUAV, uint ClearValue, FIntVector Groups, FRDGEventName Label)
{
	if (!RenderTextureUAV.IsValid())
	{
		return nullptr;
	}
	FClearTex3DUint::FParameters* PassParameters = GraphBuilder.AllocParameters<FClearTex3DUint::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;
	PassTransitions.Emplace(RenderTextureUAV, ERHIAccess::Unknown, ERHIAccess::UAVMask);

	PassParameters->Tex3DUintUAV = RenderTextureUAV;
	PassParameters->Tex3DUintClearValue = ClearValue;
	return GraphBuilder.AddPass(MoveTemp(Label), PassParameters, ERDGPassFlags::Compute,
		[=](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.Transition(PassTransitions);

			FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			TShaderMapRef<FClearTex3DUint> VDBShader(GlobalShaderMap);

			FComputeShaderUtils::Dispatch(RHICmdList, VDBShader, *PassParameters, Groups);
		});
}

void FZibraVDBMaterialRendering::ClearRenderBlockBuffer(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy)
{
	if (!Proxy->GetNativeResources()->RenderBlockBuffer.BufferUAV.IsValid() || !Proxy->GetBlockIndexTextureUAV().IsValid())
	{
		return;
	}

	RDG_EVENT_SCOPE(GraphBuilder, "ClearRenderBlocks");
	RDG_GPU_STAT_SCOPE(GraphBuilder, ClearRenderBlocks);
	SCOPED_NAMED_EVENT(ClearRenderBlocks, FColor::Purple);

	TArray<FRHITransitionInfo> PassTransitions;
	InsertTransition(PassTransitions, Proxy->GetNativeResources()->RenderBlockBuffer, ERHIAccess::UAVMask);
	TRANSITION_PROXY_RESOURCE(PassTransitions, BlockIndexTexture, ERHIAccess::UAVMask);

	GraphBuilder.AddPass(RDG_EVENT_NAME("ClearRenderBlocks"), ERDGPassFlags::None,
		[=, this](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.Transition(PassTransitions);
			RHICmdList.ClearUAVUint(Proxy->GetNativeResources()->RenderBlockBuffer.BufferUAV, FUintVector4{uint32_t(-1)});
			RHICmdList.ClearUAVUint(Proxy->GetBlockIndexTextureUAV(), FUintVector4{uint32_t(-1)});
		});
}

void FZibraVDBMaterialRendering::MakeBlockList(
	FRDGBuilder& GraphBuilder, const FZibraVDBMaterialSceneProxy* Proxy, FZibraVDBNativeResource& RenderResource)
{
	RDG_EVENT_SCOPE(GraphBuilder, "MakeBlockList");
	RDG_GPU_STAT_SCOPE(GraphBuilder, MakeBlockList);
	SCOPED_NAMED_EVENT(MakeBlockList, FColor::Purple);

	int BlockCount = RenderResource.RenderParameters.SpacialBlockCount;
	FMakeBlockList::FParameters* PassParameters = GraphBuilder.AllocParameters<FMakeBlockList::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;

	PassParameters->BlockInfo = RenderResource.DecompressionPerSpatialBlockInfo.BufferSRV;
	InsertTransition(PassTransitions, RenderResource.DecompressionPerSpatialBlockInfo, ERHIAccess::SRVMask);
	PassParameters->BlockBuffer = RenderResource.RenderBlockBuffer.BufferUAV;
	InsertTransition(PassTransitions, RenderResource.RenderBlockBuffer, ERHIAccess::UAVMask);
	PassParameters->BlockIndexTexture = Proxy->GetBlockIndexTextureUAV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, BlockIndexTexture, ERHIAccess::UAVMask);

	PassParameters->SpatialBlockCount = RenderResource.RenderParameters.SpacialBlockCount;
	PassParameters->BlockGridSize = RenderResource.RenderParameters.BlockGridSize;

	PassParameters->DensityChannelMask = RenderResource.RenderParameters.DensityChannelMask;
	PassParameters->TemperatureChannelMask = RenderResource.RenderParameters.TemperatureChannelMask;
	PassParameters->FlamesChannelMask = RenderResource.RenderParameters.FlamesChannelMask;

	GraphBuilder.AddPass(RDG_EVENT_NAME("MakeBlockList"), PassParameters, ERDGPassFlags::Compute,
		[=, this](FRHICommandListImmediate& RHICmdList)
		{
#if ZIBRAVDB_ENABLE_PROFILING
			ZibraVDBGPURenderProfiler->BeginFrame(RHICmdList);
#endif

			RHICmdList.Transition(PassTransitions);

			const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			const TShaderMapRef<FMakeBlockList> MakeBlockListShader(GlobalShaderMap);
			const auto Groups = FIntVector(IntCeilDiv(BlockCount, 128), 1, 1);
			FComputeShaderUtils::Dispatch(RHICmdList, MakeBlockListShader, *PassParameters, Groups);
		});
}

void FZibraVDBMaterialRendering::CalculateMaxDensityPerRenderBlock(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
	FZibraVDBNativeResource& RenderResource, uint32_t SpatialBlockCount, bool bEnableInterpolation /*= true*/)
{
	RDG_EVENT_SCOPE(GraphBuilder, "CalculateMaxDensityPerRenderBlock");
	RDG_GPU_STAT_SCOPE(GraphBuilder, CalculateMaxDensityPerRenderBlock);
	SCOPED_NAMED_EVENT(CalculateMaxDensityPerRenderBlock, FColor::Purple);

	FCalculateMaxDensityPerRenderBlock::FParameters* PassParameters =
		GraphBuilder.AllocParameters<FCalculateMaxDensityPerRenderBlock::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;

	const auto& FrameHeader = Proxy->GetCurrentFrameHeader();

	PassParameters->DecompressionPerChannelBlockInfo = RenderResource.DecompressionPerChannelBlockInfo.BufferSRV;
	InsertTransition(PassTransitions, RenderResource.DecompressionPerChannelBlockInfo, ERHIAccess::SRVMask);
	PassParameters->MaxDensityPerBlockTexture = Proxy->GetMaxDensityPerBlockTextureUAV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, MaxDensityPerBlockTexture, ERHIAccess::UAVMask);
	PassParameters->DensityChannelScale = RenderResource.RenderParameters.DensityChannelScale;
	const FString DensityChannelName = Proxy->GetAssetComponent()->GetZibraVDBVolume()->DensityChannel;
	const int32 DensityChannelIndex = FrameHeader.ChannelNames.Find(DensityChannelName);
	PassParameters->DensityDenormalizationScale =
		DensityChannelIndex != INDEX_NONE ? FrameHeader.ChannelScales[DensityChannelIndex] : 0.0f;

	PassParameters->BlockBuffer = RenderResource.RenderBlockBuffer.BufferSRV;
	InsertTransition(PassTransitions, RenderResource.RenderBlockBuffer, ERHIAccess::SRVMask);
	PassParameters->BlockIndexTexture = Proxy->GetBlockIndexTextureSRV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, BlockIndexTexture, ERHIAccess::SRVMask)

	PassParameters->BlockGridSize = FIntVector4(RenderResource.RenderParameters.BlockGridSize, 0);
	PassParameters->SparseBlockGridSize = RenderResource.RenderParameters.MaxRenderBlockCountCBRT;
	PassParameters->SparseBlockGridSizeMultiply = RenderResource.RenderParameters.MaxRenderBlockCountCBRTMultiply;
	PassParameters->SparseBlockGridSizeShift1 = RenderResource.RenderParameters.MaxRenderBlockCountCBRTShift1;
	PassParameters->SparseBlockGridSizeShift2 = RenderResource.RenderParameters.MaxRenderBlockCountCBRTShift2;
	PassParameters->SpatialBlockCount = SpatialBlockCount;

	GraphBuilder.AddPass(RDG_EVENT_NAME("CalculateMaxDensityPerRenderBlock"), PassParameters, ERDGPassFlags::Compute,
		[=, this](FRHICommandListImmediate& RHICmdList) mutable
		{
			RHICmdList.Transition(PassTransitions);

			FCalculateMaxDensityPerRenderBlock::FPermutationDomain PermutationVector;
			PermutationVector.Set<FCalculateMaxDensityPerRenderBlock::FEnableInterpolation>(bEnableInterpolation);

			const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			const TShaderMapRef<FCalculateMaxDensityPerRenderBlock> Shader(GlobalShaderMap, PermutationVector);

			int GroupsLeft = IntCeilDiv(SpatialBlockCount, 128);
			while (GroupsLeft > 0)
			{
				PassParameters->BlockOffset = IntCeilDiv(SpatialBlockCount, 128) - GroupsLeft;

				int GroupsToProcess = FMath::Min(GroupsLeft, ZIBRAVDB_MAX_DISPATCH_BLOCKS_NUMBER);
				FComputeShaderUtils::Dispatch(RHICmdList, Shader, *PassParameters, FIntVector(GroupsToProcess, 1, 1));
				GroupsLeft -= GroupsToProcess;
			}
		});
}

void FZibraVDBMaterialRendering::ComputeRenderBlocks(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
	FZibraVDBNativeResource& RenderResource, uint32_t SpatialBlockCount, bool bIsDensityOnly /*= false*/,
	bool bEnableInterpolation /*= true*/, bool bHVEnabeld /*= false*/)
{
	RDG_EVENT_SCOPE(GraphBuilder, "ComputeRenderBlocks");
	RDG_GPU_STAT_SCOPE(GraphBuilder, ComputeRenderBlocks);
	SCOPED_NAMED_EVENT(ComputeRenderBlocks, FColor::Purple);

	FComputeRenderBlocks::FParameters* PassParameters = GraphBuilder.AllocParameters<FComputeRenderBlocks::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;

	const auto& FrameHeader = Proxy->GetCurrentFrameHeader();

	const FIntVector IllumResolution =
		FIntVector::DivideAndRoundUp(RenderResource.RenderParameters.TextureSize, Proxy->IlluminationDownscaleFactor);
	CREATE_AND_STORE_TEXTURE_3D(ZibraIllumination, IllumResolution, PF_FloatRGBA, ERHIAccess::SRVMask);

	const bool bEnableDownscale = Proxy->VolumeDownscaleFactor != 1;

	const auto Sampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	if (bIsDensityOnly || bHVEnabeld)
	{
		PassParameters->SparseBlocksDensity = Proxy->GetSparseBlocksDensityUAV();
		TRANSITION_PROXY_RESOURCE(PassTransitions, SparseBlocksDensity, ERHIAccess::UAVMask);
		PassParameters->MaxDensityPerBlockTexture = Proxy->GetMaxDensityPerBlockTextureUAV();
		TRANSITION_PROXY_RESOURCE(PassTransitions, MaxDensityPerBlockTexture, ERHIAccess::UAVMask);
	}

	if (!bIsDensityOnly || bHVEnabeld)
	{
		if (!bHVEnabeld)
		{
			PassParameters->Illumination = Proxy->GetZibraIlluminationTexture();
			TRANSITION_3D_TEXTURE(PassTransitions, ZibraIllumination, ERHIAccess::SRVMask);
			// TODO validate sampler params
			PassParameters->samplerIllumination = Sampler;
		}

		PassParameters->ScatteringColorsTexture = Proxy->ScatteringColorTexture;
		PassTransitions.Emplace(Proxy->ScatteringColorTexture, ERHIAccess::Unknown, ERHIAccess::SRVMask);
		PassParameters->FlameColorsTexture = Proxy->FlameColorTexture;
		PassTransitions.Emplace(Proxy->FlameColorTexture, ERHIAccess::Unknown, ERHIAccess::SRVMask);
		PassParameters->TemperatureColorsTexture = Proxy->TemperatureColorTexture;
		PassTransitions.Emplace(Proxy->TemperatureColorTexture, ERHIAccess::Unknown, ERHIAccess::SRVMask);
		PassParameters->ColorsTextureSampler = Sampler;
		PassParameters->SparseBlocks = Proxy->GetSparseBlocksRGBUAV();
		TRANSITION_PROXY_RESOURCE(PassTransitions, SparseBlocksRGB, ERHIAccess::UAVMask);

		PassParameters->TextureSize = FIntVector4(RenderResource.RenderParameters.TextureSize, 0);
		PassParameters->rb_flameScale = Proxy->FlameScale * RenderResource.RenderParameters.FlamesChannelScale;
		PassParameters->rb_temperatureScale = Proxy->TemperatureScale * RenderResource.RenderParameters.TemperatureChannelScale;

		PassParameters->bEnableEmissionMasking = Proxy->bEnableEmissionMasking;
		PassParameters->bEnableFlameEmissionMasking = Proxy->bEnableFlameEmissionMasking;
		PassParameters->bEnableTemperatureEmissionMasking = Proxy->bEnableTemperatureEmissionMasking;
		PassParameters->EmissionMaskCenter = Proxy->EmissionMaskCenter;
		PassParameters->EmissionMaskWidth = Proxy->EmissionMaskWidth;
		PassParameters->EmissionMaskIntensity = Proxy->EmissionMaskIntensity;
		PassParameters->EmissionMaskRamp = Proxy->EmissionMaskRamp;
	}

	PassParameters->Input = RenderResource.DecompressionPerChannelBlockData.BufferSRV;
	InsertTransition(PassTransitions, RenderResource.DecompressionPerChannelBlockData, ERHIAccess::SRVMask);
	PassParameters->BlockBuffer = RenderResource.RenderBlockBuffer.BufferSRV;
	InsertTransition(PassTransitions, RenderResource.RenderBlockBuffer, ERHIAccess::SRVMask);
	PassParameters->BlockIndexTexture = Proxy->GetBlockIndexTextureSRV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, BlockIndexTexture, ERHIAccess::SRVMask);

	PassParameters->rb_densityScale = Proxy->DensityScale;
	PassParameters->DensityChannelScale = RenderResource.RenderParameters.DensityChannelScale;

	PassParameters->BlockGridSize = FIntVector4(RenderResource.RenderParameters.BlockGridSize, 0);
	PassParameters->SparseBlockGridSize = RenderResource.RenderParameters.MaxRenderBlockCountCBRT;
	PassParameters->SparseBlockGridSizeMultiply = RenderResource.RenderParameters.MaxRenderBlockCountCBRTMultiply;
	PassParameters->SparseBlockGridSizeShift1 = RenderResource.RenderParameters.MaxRenderBlockCountCBRTShift1;
	PassParameters->SparseBlockGridSizeShift2 = RenderResource.RenderParameters.MaxRenderBlockCountCBRTShift2;
	PassParameters->rb_offset = 0;

	PassParameters->rb_maskSpherePos = Proxy->SphereMaskPosition;
	PassParameters->rb_maskSphereRadius = Proxy->SphereMaskRadius;
	PassParameters->rb_maskSphereFalloff = Proxy->SphereMaskFalloff;
	PassParameters->rb_frameTransform = Proxy->GetVoxelToWorldMatrix();
	PassParameters->rb_maskEnabled = Proxy->bEnableSphereMasking;
	PassParameters->SpatialBlockCount = SpatialBlockCount;

	PassParameters->DownscaleFactor = Proxy->VolumeDownscaleFactor;
	PassParameters->DownscaledBlockSize = ZIBRAVDB_BLOCK_SIZE / Proxy->VolumeDownscaleFactor;

	GraphBuilder.AddPass(RDG_EVENT_NAME("ComputeRenderBlocks"), PassParameters, ERDGPassFlags::Compute,
		[=, this](FRHICommandListImmediate& RHICmdList) mutable
		{
			RHICmdList.Transition(PassTransitions);
			FComputeRenderBlocks::FPermutationDomain PermutationVector;
			PermutationVector.Set<FComputeRenderBlocks::FDensityOnly>(bIsDensityOnly && !bHVEnabeld);
			PermutationVector.Set<FComputeRenderBlocks::FEnableInterpolation>(bEnableInterpolation);
			PermutationVector.Set<FComputeRenderBlocks::FEnableDownscale>(bEnableDownscale);
			PermutationVector.Set<FComputeRenderBlocks::FHVEnabled>(bHVEnabeld);
			const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			const TShaderMapRef<FComputeRenderBlocks> VDBShader(GlobalShaderMap, PermutationVector);

			const int GroupsCount = IntCeilDiv(
				SpatialBlockCount, Proxy->VolumeDownscaleFactor * Proxy->VolumeDownscaleFactor * Proxy->VolumeDownscaleFactor);
			int GroupsLeft = GroupsCount;
			while (GroupsLeft > 0)
			{
				PassParameters->rb_offset = GroupsCount - GroupsLeft;

				int GroupsToProcess = FMath::Min(GroupsLeft, ZIBRAVDB_MAX_DISPATCH_BLOCKS_NUMBER);

				FComputeShaderUtils::Dispatch(RHICmdList, VDBShader, *PassParameters, FIntVector(GroupsToProcess, 1, 1));
				GroupsLeft -= GroupsToProcess;
			}
#if ZIBRAVDB_ENABLE_PROFILING
			ZibraVDBGPURenderProfiler->EndFrame(RHICmdList);
#endif
		});

	// Skip setting padding if interpolation is turned off.
	if (!bEnableInterpolation)
	{
		return;
	}

	FSetPaddingVoxel::FParameters* Pass2Parameters = GraphBuilder.AllocParameters<FSetPaddingVoxel::FParameters>();
	TArray<FRHITransitionInfo> Pass2Transitions;

	if (bIsDensityOnly || bHVEnabeld)
	{
		Pass2Parameters->SparseBlocksDensity = Proxy->GetSparseBlocksDensityUAV();
		TRANSITION_PROXY_RESOURCE(Pass2Transitions, SparseBlocksDensity, ERHIAccess::UAVMask);
		Pass2Parameters->MaxDensityPerBlockTexture = Proxy->GetMaxDensityPerBlockTextureUAV();
		TRANSITION_PROXY_RESOURCE(Pass2Transitions, MaxDensityPerBlockTexture, ERHIAccess::UAVMask);
	}

	if (!bIsDensityOnly || bHVEnabeld)
	{
		Pass2Parameters->SparseBlocks = Proxy->GetSparseBlocksRGBUAV();
		TRANSITION_PROXY_RESOURCE(Pass2Transitions, SparseBlocksRGB, ERHIAccess::UAVMask);
		Pass2Parameters->TextureSize = FIntVector4(RenderResource.RenderParameters.TextureSize, 0);
		Pass2Parameters->rb_flameScale = Proxy->FlameScale * RenderResource.RenderParameters.FlamesChannelScale;
		Pass2Parameters->rb_temperatureScale = Proxy->TemperatureScale * RenderResource.RenderParameters.TemperatureChannelScale;
	}

	Pass2Parameters->Input = RenderResource.DecompressionPerChannelBlockData.BufferSRV;
	InsertTransition(Pass2Transitions, RenderResource.DecompressionPerChannelBlockData, ERHIAccess::SRVMask);
	Pass2Parameters->BlockBuffer = RenderResource.RenderBlockBuffer.BufferSRV;
	InsertTransition(Pass2Transitions, RenderResource.RenderBlockBuffer, ERHIAccess::SRVMask);
	Pass2Parameters->BlockIndexTexture = Proxy->GetBlockIndexTextureSRV();
	TRANSITION_PROXY_RESOURCE(Pass2Transitions, BlockIndexTexture, ERHIAccess::SRVMask);

	Pass2Parameters->rb_densityScale = Proxy->DensityScale;
	Pass2Parameters->DensityChannelScale = RenderResource.RenderParameters.DensityChannelScale;

	Pass2Parameters->BlockGridSize = FIntVector4(RenderResource.RenderParameters.BlockGridSize, 0);
	Pass2Parameters->SparseBlockGridSize = RenderResource.RenderParameters.MaxRenderBlockCountCBRT;
	Pass2Parameters->SparseBlockGridSizeMultiply = RenderResource.RenderParameters.MaxRenderBlockCountCBRTMultiply;
	Pass2Parameters->SparseBlockGridSizeShift1 = RenderResource.RenderParameters.MaxRenderBlockCountCBRTShift1;
	Pass2Parameters->SparseBlockGridSizeShift2 = RenderResource.RenderParameters.MaxRenderBlockCountCBRTShift2;
	Pass2Parameters->rb_offset = 0;

	Pass2Parameters->SpatialBlockCount = SpatialBlockCount;

	Pass2Parameters->DownscaleFactor = Proxy->VolumeDownscaleFactor;
	Pass2Parameters->DownscaledBlockSize = ZIBRAVDB_BLOCK_SIZE / Proxy->VolumeDownscaleFactor;

	const auto MaxRenderBlockCountCBRT = RenderResource.RenderParameters.MaxRenderBlockCountCBRT;
	const auto BlockSize = bEnableDownscale ? (Pass2Parameters->DownscaledBlockSize) : ZIBRAVDB_BLOCK_SIZE;
	const auto RenderBlockSize = BlockSize + ZIBRAVDB_RENDER_BLOCK_PADDING * bEnableInterpolation;
	GraphBuilder.AddPass(RDG_EVENT_NAME("SetPaddingVoxel"), Pass2Parameters, ERDGPassFlags::Compute,
		[=, this](FRHICommandListImmediate& RHICmdList) mutable
		{
			RHICmdList.Transition(Pass2Transitions);
			FSetPaddingVoxel::FPermutationDomain PermutationVector;
			PermutationVector.Set<FSetPaddingVoxel::FDensityOnly>(bIsDensityOnly && !bHVEnabeld);
			PermutationVector.Set<FSetPaddingVoxel::FEnableDownscale>(bEnableDownscale);
			PermutationVector.Set<FSetPaddingVoxel::FHVEnabled>(bHVEnabeld);
			PermutationVector.Set<FSetPaddingVoxel::FEnableInterpolation>(bEnableInterpolation);
			const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			const TShaderMapRef<FSetPaddingVoxel> VDBShader(GlobalShaderMap, PermutationVector);
			{
				FIntVector Groups = FIntVector(IntCeilDiv(MaxRenderBlockCountCBRT * RenderBlockSize, 8),
					IntCeilDiv(MaxRenderBlockCountCBRT * RenderBlockSize, 8), IntCeilDiv(MaxRenderBlockCountCBRT, 1));
				FComputeShaderUtils::Dispatch(RHICmdList, VDBShader, *Pass2Parameters, Groups);
			}
		});
}

void FZibraVDBMaterialRendering::ComposeReflectionsVolume(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy,
	FZibraVDBNativeResource& RenderResource, bool bEnableInterpolation /*= true*/)
{
	auto IlluminatedVolume = Proxy->GetIlluminatedVolumeTextureUAV();
	if (!IlluminatedVolume.IsValid())
	{
		UE_LOG(LogZibraVDBRuntime, Warning, TEXT("Compose reflections was called but IlluminatedVolume is not valid"));
		return;
	}
	RDG_EVENT_SCOPE(GraphBuilder, "ComposeReflectionsVolume");
	RDG_GPU_STAT_SCOPE(GraphBuilder, ComposeReflectionsVolume);
	SCOPED_NAMED_EVENT(ComposeReflectionsVolume, FColor::Purple);

	const bool bEnableDownscale = Proxy->VolumeDownscaleFactor != 1;

	FComposeReflectionsVolume::FParameters* PassParameters = GraphBuilder.AllocParameters<FComposeReflectionsVolume::FParameters>();
	TArray<FRHITransitionInfo> PassTransitions;

	PassParameters->MaxDensityPerBlockTexture = Proxy->GetMaxDensityPerBlockTextureSRV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, MaxDensityPerBlockTexture, ERHIAccess::SRVMask);
	PassParameters->BlockIndexTexture = Proxy->GetBlockIndexTextureSRV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, BlockIndexTexture, ERHIAccess::SRVMask);
	PassParameters->SparseBlockGridSize = RenderResource.RenderParameters.MaxRenderBlockCountCBRT;
	PassParameters->SparseBlockGridSizeMultiply = RenderResource.RenderParameters.MaxRenderBlockCountCBRTMultiply;
	PassParameters->SparseBlockGridSizeShift1 = RenderResource.RenderParameters.MaxRenderBlockCountCBRTShift1;
	PassParameters->SparseBlockGridSizeShift2 = RenderResource.RenderParameters.MaxRenderBlockCountCBRTShift2;

	PassParameters->SparseBlocks = Proxy->GetSparseBlocksRGBSRV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, SparseBlocksRGB, ERHIAccess::SRVMask);
	PassParameters->SparseBlocksDensity = Proxy->GetSparseBlocksDensitySRV();
	TRANSITION_PROXY_RESOURCE(PassTransitions, SparseBlocksDensity, ERHIAccess::SRVMask);
	PassParameters->IlluminatedVolume = IlluminatedVolume;
	TRANSITION_PROXY_RESOURCE(PassTransitions, IlluminatedVolumeTexture, ERHIAccess::UAVMask);

	FIntVector3 IlluminatedVolumeSize = FIntVector3(IlluminatedVolume->GetTexture()->GetSizeX(),
		IlluminatedVolume->GetTexture()->GetSizeY(), IlluminatedVolume->GetTexture()->GetSizeZ());
	PassParameters->rb_IlluminationSize = IlluminatedVolumeSize;
	PassParameters->rb_IlluminationDownscaleFactor = 1. / Proxy->GetMaterialComponent()->IlluminatedVolumeResolutionScale;

	PassParameters->DownscaleFactor = Proxy->VolumeDownscaleFactor;
	PassParameters->DownscaledBlockSize = ZIBRAVDB_BLOCK_SIZE / Proxy->VolumeDownscaleFactor;

	GraphBuilder.AddPass(RDG_EVENT_NAME("ComposeReflectionsVolume"), PassParameters,
		ERDGPassFlags::Compute,
		[=, this](FRHICommandListImmediate& RHICmdList) mutable
		{
			RHICmdList.Transition(PassTransitions);

			FComposeReflectionsVolume::FPermutationDomain PermutationVector;
			PermutationVector.Set<FComposeReflectionsVolume::FEnableInterpolation>(bEnableInterpolation);
			PermutationVector.Set<FComposeReflectionsVolume::FEnableDownscale>(bEnableDownscale);
			const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
			const TShaderMapRef<FComposeReflectionsVolume> VDBShader(GlobalShaderMap, PermutationVector);

			const FIntVector Groups = FIntVector(IntCeilDiv(IlluminatedVolumeSize.X, ZIBRAVDB_BLOCK_SIZE),
				IntCeilDiv(IlluminatedVolumeSize.Y, ZIBRAVDB_BLOCK_SIZE), IntCeilDiv(IlluminatedVolumeSize.Z, ZIBRAVDB_BLOCK_SIZE));
			FComputeShaderUtils::Dispatch(RHICmdList, VDBShader, *PassParameters, Groups);
		});
}

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
void FZibraVDBMaterialRendering::AddVDBProxy(FZibraVDBMaterialSceneProxy* InProxy, FRHICommandListBase& RHICmdList) noexcept
#else
void FZibraVDBMaterialRendering::AddVDBProxy(FZibraVDBMaterialSceneProxy* InProxy) noexcept
#endif
{
#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 504
	ENQUEUE_RENDER_COMMAND(FAddVdbProxyCommand)
	(
		[this, InProxy](FRHICommandListImmediate& RHICmdList) noexcept
		{
#endif
			check(ZibraVDBProxies.Find(InProxy) == INDEX_NONE);

			for (FZibraVDBMaterialSceneProxy* Proxy : ZibraVDBProxies)
			{
				if (InProxy->GetMaterialComponent()->GetOuter()->GetName() == Proxy->GetMaterialComponent()->GetOuter()->GetName())
				{
					Proxy->SetVisible(false);
				}
			}

			ZibraVDBProxies.Emplace(InProxy);

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) < 504
		});
#endif
}

void FZibraVDBMaterialRendering::RemoveVDBProxy(FZibraVDBMaterialSceneProxy* InProxy) noexcept
{
	ENQUEUE_RENDER_COMMAND(FRemoveVdbProxyCommand)
	(
		[this, InProxy](FRHICommandListImmediate& RHICmdList) noexcept
		{
			for (FZibraVDBMaterialSceneProxy* Proxy : ZibraVDBProxies)
			{
				if (InProxy->GetMaterialComponent()->GetOuter()->GetName() == Proxy->GetMaterialComponent()->GetOuter()->GetName())
				{
					Proxy->SetVisible(true);
				}
			}
			ZibraVDBProxies.Remove(InProxy);
		});
}

void FZibraVDBMaterialRendering::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
}
void FZibraVDBMaterialRendering::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
}

bool ShouldBeFrustumCulled(const FSceneView& View, const FBoxSphereBounds& MeshBounds)
{
	return !View.ViewFrustum.IntersectBox(MeshBounds.Origin, MeshBounds.BoxExtent);
}

bool ShouldBeDistanceCulled(const FSceneView& View, const FBoxSphereBounds& MeshBounds, float MaxDrawDistance)
{
	bool bShouldBeDistanceCulled = false;
	if (MaxDrawDistance > 0)
	{
		float Distance = FVector::Dist(MeshBounds.Origin, View.ViewLocation);
		float EffectiveDistance = Distance - MeshBounds.SphereRadius;
		bShouldBeDistanceCulled = EffectiveDistance > MaxDrawDistance;
	}

	return bShouldBeDistanceCulled;
}

void FZibraVDBMaterialRendering::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	for (const auto& MaterialComponent : ActiveZibraVDBs)
	{
		if (!MaterialComponent)
		{
			continue;
		}

		if (MaterialComponent->VisibleInReflections() && !MaterialComponent->bFullFrustumCulling)
		{
			continue;
		}

		FBoxSphereBounds MeshBounds = MaterialComponent->BBoxMeshComponent->Bounds;
		MeshBounds.BoxExtent *= MaterialComponent->ZibraVDBBoundsScale;
		if (ShouldBeDistanceCulled(InView, MeshBounds, MaterialComponent->MaxDrawDistance) ||
			(MaterialComponent->bFullFrustumCulling && ShouldBeFrustumCulled(InView, MeshBounds)))
		{
			InView.HiddenPrimitives.Add(MaterialComponent->GetBBoxMeshPrimitiveComponentId());
		}
	}
}

void FZibraVDBMaterialRendering::PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
	const FSceneView* View = &InView;

	check(InView.bIsViewInfo);
	const FViewInfo& ViewInfo = static_cast<FViewInfo&>(InView);

	if (ZibraVDBProxies.Num() == 0)
	{
		return;
	}

	const FSceneViewFamily* ViewFamily = InView.Family;

	TArray<FZibraVDBMaterialSceneProxy*> ProxiesToRender = ZibraVDBProxies.FilterByPredicate(
		[=](const FZibraVDBMaterialSceneProxy* Proxy) { return FilterRelevantProxies(Proxy, ViewFamily); });

	GPUProfilerCounters.Empty();

	for (FZibraVDBMaterialSceneProxy* Proxy : ProxiesToRender)
	{
#if !UE_BUILD_SHIPPING
		RDG_EVENT_SCOPE(GraphBuilder, "ZibraVDB_PreRenderView (%s)", *Proxy->ActorName);
#else
		RDG_EVENT_SCOPE(GraphBuilder, "ZibraVDB_PreRenderView");
#endif
		RDG_GPU_STAT_SCOPE(GraphBuilder, ZibraVDB_PreRenderView);
		SCOPED_NAMED_EVENT(ZibraVDB_PreRenderView, FColor::Purple);

		if (!Proxy->GetRenderingResources() || !Proxy->GetNativeResources())
		{
			continue;
		}

		FBoxSphereBounds MeshBounds = Proxy->GetMaterialComponent()->BBoxMeshComponent->Bounds;
		MeshBounds.BoxExtent *= Proxy->GetMaterialComponent()->ZibraVDBBoundsScale;
		bool bShouldBeDistanceCulled =
			View->HiddenPrimitives.Contains(Proxy->GetMaterialComponent()->GetBBoxMeshPrimitiveComponentId());
		bool bShouldBeFrustumCulled = Proxy->GetMaterialComponent()->bAllowFrustumCulling &&
									  Proxy->GetMaterialComponent()->bFullFrustumCulling &&
									  ShouldBeFrustumCulled(InView, MeshBounds);

		if (bShouldBeDistanceCulled || bShouldBeFrustumCulled)
		{
			continue;
		}

		FZibraVDBNativeResource& RenderResource = *Proxy->GetNativeResources();

		Proxy->PrepareForRender();

		if (!Proxy->IsReadyForRender())
		{
			continue;
		}

		const int FrameIndex = Proxy->GetCurrentFrameIndex();
		const auto& TextureSize = RenderResource.RenderParameters.TextureSize;

		const auto& FrameHeader = Proxy->GetCurrentFrameHeader();

		UpdateRenderParameters(FrameHeader, Proxy, RenderResource);

		const bool bEnableInterpolation = Proxy->RayMarchingFiltering == ERayMarchingFiltering::Trilinear;

		if (FrameHeader.SpatialBlockCount == 0)
		{
			ClearRenderBlockBuffer(GraphBuilder, Proxy);
			continue;
		}

		bool bDecompressionHappened = Decompress(GraphBuilder, Proxy, FrameIndex + Proxy->KeyFrameStartIndex);

		if (bDecompressionHappened || Proxy->HasChannelMasksChanged())
		{
			ClearRenderBlockBuffer(GraphBuilder, Proxy);

			MakeBlockList(GraphBuilder, Proxy, RenderResource);

			if (Proxy->HasDensity())
			{
				CalculateMaxDensityPerRenderBlock(
					GraphBuilder, Proxy, RenderResource, FrameHeader.SpatialBlockCount, bEnableInterpolation);
			}

			Proxy->SetPreviousChannelMasks();
		}

		if (Proxy->bUseHeterogeneousVolume)
		{
			if (bDecompressionHappened)
			{
				ComputeRenderBlocks(
					GraphBuilder, Proxy, RenderResource, FrameHeader.SpatialBlockCount, true, bEnableInterpolation, true);
			}
		}
		else if (Proxy->HasDensity())
		{
			if (bDecompressionHappened)
			{
				ComputeRenderBlocks(GraphBuilder, Proxy, RenderResource, FrameHeader.SpatialBlockCount, true, bEnableInterpolation);
			}
		}

#if ZIBRAVDB_ENABLE_PROFILING
		UpdateProfilerCounters(Proxy, GraphBuilder);
#endif
	}
}

bool FZibraVDBMaterialRendering::Decompress(FRDGBuilder& GraphBuilder, FZibraVDBMaterialSceneProxy* Proxy, int FrameIndex)
{
	if (!Proxy->AllowSkipDecompression || Proxy->NeedToDecompress())
	{
		RDG_EVENT_SCOPE(GraphBuilder, "Decompress");
		RDG_GPU_STAT_SCOPE(GraphBuilder, Decompress);
		SCOPED_NAMED_EVENT(Decompress, FColor::Purple);

		GraphBuilder.AddPass(RDG_EVENT_NAME("Decompress"), ERDGPassFlags::None,
			[=, this](FRHICommandListImmediate& RHICmdList)
			{
#if ZIBRAVDB_ENABLE_PROFILING
				ZibraVDBGPUDecompressionProfiler->BeginFrame(RHICmdList);
#endif
				auto ReturnCode = Proxy->DecompressorManager.Decompress(FrameIndex);
#if ZIBRAVDB_ENABLE_PROFILING
				ZibraVDBGPUDecompressionProfiler->EndFrame(RHICmdList);
#endif
			});
		Proxy->SetDecompressedFrame();
		return true;
	}
	return false;
}

void FZibraVDBMaterialRendering::PostOpaqueRender(FPostOpaqueRenderParameters& PostOpaqueRenderParameters)
{
	if (!PostOpaqueRenderParameters.View)
	{
		return;
	}

	auto& GraphBuilder = *PostOpaqueRenderParameters.GraphBuilder;
	const FViewInfo* View = PostOpaqueRenderParameters.View;
	if (ZibraVDBProxies.Num() == 0)
	{
		return;
	}

	const FSceneViewFamily* ViewFamily = View->Family;

	TArray<FZibraVDBMaterialSceneProxy*> ProxiesToRender = ZibraVDBProxies.FilterByPredicate(
		[=](const FZibraVDBMaterialSceneProxy* Proxy) { return FilterRelevantProxies(Proxy, ViewFamily); });

	for (FZibraVDBMaterialSceneProxy* Proxy : ProxiesToRender)
	{
#if !UE_BUILD_SHIPPING
		RDG_EVENT_SCOPE(GraphBuilder, "ZibraVDB_PostRenderBasePassDeferred (%s)", *Proxy->ActorName);
#else
		RDG_EVENT_SCOPE(GraphBuilder, "ZibraVDB_PostRenderBasePassDeferred");
#endif
		RDG_GPU_STAT_SCOPE(GraphBuilder, ZibraVDB_PostRenderBasePassDeferred);
		SCOPED_NAMED_EVENT(ZibraVDB_PostRenderBasePassDeferred, FColor::Purple);

		if (Proxy->bUseHeterogeneousVolume)
		{
			continue;
		}
		if (!Proxy->GetRenderingResources() || !Proxy->GetNativeResources())
		{
			continue;
		}

		FBoxSphereBounds MeshBounds = Proxy->GetMaterialComponent()->BBoxMeshComponent->Bounds;
		MeshBounds.BoxExtent *= Proxy->GetMaterialComponent()->ZibraVDBBoundsScale;
		bool bShouldBeDistanceCulled =
			View->HiddenPrimitives.Contains(Proxy->GetMaterialComponent()->GetBBoxMeshPrimitiveComponentId());
		bool bShouldBeFrustumCulled =
			Proxy->GetMaterialComponent()->bAllowFrustumCulling &&
			(!Proxy->GetMaterialComponent()->VisibleInReflections() || Proxy->GetMaterialComponent()->bFullFrustumCulling) && ShouldBeFrustumCulled(*View, MeshBounds);

		if (bShouldBeDistanceCulled || bShouldBeFrustumCulled)
		{
			continue;
		}

		Proxy->PrepareForRender();

		if (!Proxy->IsReadyForRender())
		{
			continue;
		}

		FZibraVDBNativeResource& RenderResource = *Proxy->GetNativeResources();

		const auto& TextureSize = RenderResource.RenderParameters.TextureSize;

		const auto& FrameHeader = Proxy->GetCurrentFrameHeader();
		const bool bEnableInterpolation = Proxy->RayMarchingFiltering == ERayMarchingFiltering::Trilinear;

		if (FrameHeader.SpatialBlockCount == 0)
		{
			ClearRenderBlockBuffer(GraphBuilder, Proxy);
			continue;
		}

		UpdateFrameTransformation(GraphBuilder, Proxy, FrameHeader);

		if (!Proxy->bUseHeterogeneousVolume && Proxy->CastedReflectionsCount > 0)
		{
			ClearIlluminatedVolumeTexture(GraphBuilder, Proxy, TextureSize, RenderResource);
		}

		bool bComputeIllumination = !Proxy->bUseStaticIllumination || Proxy->NeedsIlluminationUpdate();
		if (!Proxy->bUseHeterogeneousVolume && Proxy->HasDensity() && bComputeIllumination)
		{
			ComputeDownscaledVolume(GraphBuilder, Proxy, TextureSize, RenderResource);
			ComputeDirectShadows(GraphBuilder, Proxy, TextureSize, RenderResource, *View);
			Proxy->UpdateLastIlluminationFrame();
		}

		ComputeRenderBlocks(GraphBuilder, Proxy, RenderResource, FrameHeader.SpatialBlockCount, false, bEnableInterpolation);
		if (Proxy->CastedReflectionsCount > 0)
		{
			ComposeReflectionsVolume(GraphBuilder, Proxy, RenderResource, bEnableInterpolation);
		}
#if ZIBRAVDB_ENABLE_PROFILING
		UpdateProfilerCounters(Proxy, GraphBuilder);
#endif
	}
}

bool FZibraVDBMaterialRendering::FilterRelevantProxies(
	const FZibraVDBMaterialSceneProxy* Proxy, const FSceneViewFamily* ViewFamily) noexcept
{
	if (!Proxy->GetVisible())
	{
		return false;
	}

	const UWorld* ViewWorld = ViewFamily->Scene->GetWorld();
	const UWorld* ProxyWorld = Proxy->GetMaterialComponent()->GetWorld();

	return ViewWorld == ProxyWorld;
}

int32 FZibraVDBMaterialRendering::GetPriority() const
{
	return -1;
}

bool FZibraVDBMaterialRendering::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
	return true;
}

#undef CREATE_AND_STORE_TEXTURE_3D
