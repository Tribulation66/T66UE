// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBRenderingResourcesSubsystem.h"

#include "Math/Float16.h"
#include "ZibraVDBAssetComponent.h"
#include "ZibraVDBMaterialComponent.h"
#include "ZibraVDBRuntime/Public/ZibraVDBNotifications.h"

#include <TextureResource.h>

TSharedPtr<FZibraVDBRenderingResources> UZibraVDBRenderingResourcesSubsystem::InitResourcesForActor(
	const FString& ActorName, const UZibraVDBMaterialComponent& MaterialComponent)
{
	if (!MaterialComponent.ZibraVDBAssetComponent->GetZibraVDBVolume()->GetZibraVDBFile().HasBinaryData())
	{
		return nullptr;
	}

	TWeakPtr<FZibraVDBRenderingResources>* MapEntry = ResourcesMap.Find(ActorName);

	if (MapEntry != nullptr && !MapEntry->IsValid())
	{
		ResourcesMap.Remove(ActorName);
		MapEntry = nullptr;
	}

	if (MapEntry == nullptr)
	{
		TSharedPtr<FZibraVDBRenderingResources> Resources = MakeShared<FZibraVDBRenderingResources>();
		Resources->InitResources(MaterialComponent);

		ResourcesMap.Add(ActorName, Resources.ToWeakPtr());
		return Resources;
	}
	if (MapEntry->IsValid())
	{
		return MapEntry->Pin();
	}

	return nullptr;
}

void UZibraVDBRenderingResourcesSubsystem::ReInitResourcesForActor(
	const FString& ActorName, const UZibraVDBMaterialComponent& MaterialComponent)
{
	TWeakPtr<FZibraVDBRenderingResources>* MapEntry = ResourcesMap.Find(ActorName);

	if (MapEntry->IsValid())
	{
		TSharedPtr<FZibraVDBRenderingResources> Resources = MapEntry->Pin();
		Resources->ReleaseResources();
		Resources->InitResources(MaterialComponent);
	}
}

void UZibraVDBRenderingResourcesSubsystem::RemoveIfInvalid(const FString& ActorName)
{
	TWeakPtr<FZibraVDBRenderingResources>* MapEntry = ResourcesMap.Find(ActorName);
	if (MapEntry && !MapEntry->IsValid())
	{
		ResourcesMap.Remove(ActorName);
	}
}

UZibraVDBRenderingResourcesSubsystem::UZibraVDBRenderingResourcesSubsystem(class FObjectInitializer const&)
{
}

void FZibraVDBRenderingResources::ReleaseTextures()
{
	if (SparseBlocksRGB)
	{
		SparseBlocksRGB->ReleaseResource();
		SparseBlocksRGB = nullptr;
	}

	if (SparseBlocksDensity)
	{
		SparseBlocksDensity->ReleaseResource();
		SparseBlocksDensity = nullptr;
	}

	if (MaxDensityPerBlockTexture)
	{
		MaxDensityPerBlockTexture->ReleaseResource();
		MaxDensityPerBlockTexture = nullptr;
	}

	if (BlockIndexVolumeTexture)
	{
		BlockIndexVolumeTexture->ReleaseResource();
		BlockIndexVolumeTexture = nullptr;
	}

	if (TransformTextureResult)
	{
		TransformTextureResult->ReleaseResource();
		TransformTextureResult = nullptr;
	}

	if (IlluminatedVolumeTexture)
	{
		IlluminatedVolumeTexture->ReleaseResource();
		IlluminatedVolumeTexture = nullptr;
	}
}

void FZibraVDBRenderingResources::ReleaseResources()
{
	ReleaseTextures();

	if (ZibraVDBNativeResources)
	{
		ZibraVDBNativeResources->ReleaseResource();
		ZibraVDBNativeResources = nullptr;
	}
}

void FZibraVDBRenderingResources::BeforeInitResources()
{
	ReleaseTextures();
}

void FZibraVDBRenderingResources::InitResources(const UZibraVDBMaterialComponent& MaterialComponent)
{
	if (!FZibraVDBSDKIntegration::IsPluginEnabled())
	{
		return;
	}

	BeforeInitResources();
	InitNativeResources(MaterialComponent);
	if ((SparseBlocksRGB == nullptr || SparseBlocksRGB->GetSurfaceWidth() < UE_KINDA_SMALL_NUMBER))
	{
		const FIntVector BlockGridSize = ZibraVDBNativeResources->RenderParameters.MaxBlockGridSize;

		const FIntVector TextureSize = ZibraVDBNativeResources->RenderParameters.TextureSize;

		const FVector3f GridSize = FVector3f(TextureSize);

		const bool bEnableInterpolation = MaterialComponent.RayMarchingFiltering == ERayMarchingFiltering::Trilinear;
		const int32 MaxRenderBlockCountCBRT = ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRT;
		const int32 MaxTextureSize =
			MaxRenderBlockCountCBRT * (bEnableInterpolation ? ZIBRAVDB_RENDER_BLOCK_SIZE : ZIBRAVDB_BLOCK_SIZE);
		const bool bHasDensity = MaterialComponent.ZibraVDBAssetComponent->GetZibraVDBVolume()->GetDensityChannelIndex() != -1;

		SparseBlocksRGB = NewObject<UTextureRenderTargetVolume>();
		SparseBlocksRGB->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
		SparseBlocksRGB->bCanCreateUAV = true;
		SparseBlocksRGB->Filter = TF_Bilinear;
		SparseBlocksRGB->Init(MaxTextureSize, MaxTextureSize, MaxTextureSize, PF_FloatR11G11B10);
		SparseBlocksRGB->UpdateResource();
		SparseBlocksRGB->UpdateResourceImmediate(true);

		SparseBlocksDensity = NewObject<UTextureRenderTargetVolume>();
		SparseBlocksDensity->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
		SparseBlocksDensity->bCanCreateUAV = true;
		SparseBlocksDensity->Filter = TF_Bilinear;
		if (bHasDensity)
		{
			SparseBlocksDensity->Init(MaxTextureSize, MaxTextureSize, MaxTextureSize, PF_R8);
		}
		else
		{
			SparseBlocksDensity->Init(1, 1, 1, PF_R8);
		}
		SparseBlocksDensity->UpdateResource();
		SparseBlocksDensity->UpdateResourceImmediate(true);

		MaxDensityPerBlockTexture = NewObject<UTextureRenderTargetVolume>();
		MaxDensityPerBlockTexture->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
		MaxDensityPerBlockTexture->bCanCreateUAV = true;
		MaxDensityPerBlockTexture->Filter = TF_Nearest;
		if (bHasDensity)
		{
			MaxDensityPerBlockTexture->Init(MaxRenderBlockCountCBRT, MaxRenderBlockCountCBRT, MaxRenderBlockCountCBRT, PF_R16F);
		}
		else
		{
			MaxDensityPerBlockTexture->Init(1, 1, 1, PF_R16F);
		}
		MaxDensityPerBlockTexture->UpdateResource();
		MaxDensityPerBlockTexture->UpdateResourceImmediate(true);

		BlockIndexVolumeTexture = NewObject<UTextureRenderTargetVolume>();
		BlockIndexVolumeTexture->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
		BlockIndexVolumeTexture->bCanCreateUAV = true;
		BlockIndexVolumeTexture->Filter = TF_Nearest;
		BlockIndexVolumeTexture->Init(BlockGridSize.X, BlockGridSize.Y, BlockGridSize.Z, PF_R32_UINT);
		BlockIndexVolumeTexture->UpdateResource();
		BlockIndexVolumeTexture->UpdateResourceImmediate(true);

		TransformTextureResult = NewObject<UTextureRenderTargetVolume>();
		TransformTextureResult->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
		TransformTextureResult->bCanCreateUAV = true;
		TransformTextureResult->Filter = TF_Nearest;
		TransformTextureResult->Init(4, 1, 1, PF_A32B32G32R32F);
		TransformTextureResult->UpdateResource();
		TransformTextureResult->UpdateResourceImmediate(true);

		IlluminatedVolumeTexture = NewObject<UTextureRenderTargetVolume>();
		IlluminatedVolumeTexture->ClearColor = FLinearColor(0.f, 0.f, 0.f, 0.f);
		IlluminatedVolumeTexture->bCanCreateUAV = true;
		IlluminatedVolumeTexture->Filter = TF_Bilinear;
		if (!MaterialComponent.bUseHeterogeneousVolume)
		{
			IlluminatedVolumeTexture->Init(TextureSize.X * MaterialComponent.IlluminatedVolumeResolutionScale,
				TextureSize.Y * MaterialComponent.IlluminatedVolumeResolutionScale,
				TextureSize.Z * MaterialComponent.IlluminatedVolumeResolutionScale, PF_FloatRGBA);
		}
		else
		{
			IlluminatedVolumeTexture->Init(1, 1, 1, PF_FloatRGBA);
		}
		IlluminatedVolumeTexture->UpdateResource();
		IlluminatedVolumeTexture->UpdateResourceImmediate(true);

		SparseBlocksRGB->AddToRoot();
		SparseBlocksDensity->AddToRoot();
		MaxDensityPerBlockTexture->AddToRoot();
		BlockIndexVolumeTexture->AddToRoot();
		TransformTextureResult->AddToRoot();
		IlluminatedVolumeTexture->AddToRoot();

#if !UE_BUILD_SHIPPING
		ENQUEUE_RENDER_COMMAND(SetRTVsNames)
		(
			[this](FRHICommandListImmediate& RHICmdList)
			{
				auto SetTextureName = [](UTextureRenderTargetVolume* RTV, const FString Name) -> void
				{
					if (RTV && RTV->GetResource() && RTV->GetResource()->TextureRHI)
					{
						RTV->GetResource()->TextureRHI->SetName(*Name);
					}
				};

				SetTextureName(SparseBlocksRGB, "ZibraSparseBlocksRGB");
				SetTextureName(SparseBlocksDensity, "ZibraSparseBlocksDensity");
				SetTextureName(MaxDensityPerBlockTexture, "ZibraMaxDensityPerBlockTexture");
				SetTextureName(BlockIndexVolumeTexture, "ZibraBlockIndexVolumeTexture");
				SetTextureName(TransformTextureResult, "ZibraTransformTextureResult");
				SetTextureName(IlluminatedVolumeTexture, "ZibraIlluminatedVolumeTexture");
			});
#endif
	}
}

void FZibraVDBRenderingResources::InitNativeResources(const UZibraVDBMaterialComponent& MaterialComponent)
{
	const ZibraVDB::FZibraVDBFile& ZibraVDBFile = MaterialComponent.ZibraVDBAssetComponent->GetZibraVDBVolume()->GetZibraVDBFile();

	if (!ZibraVDBFile.HasBinaryData())
	{
		return;
	}

	const ZibraVDB::FZibraVDBHeader& Header = ZibraVDBFile.GetHeader();

	const uint32_t MaxSpatialBlockCount = Header.MaxSpatialBlockCount;
	const uint32_t MaxChannelBlockCount = Header.MaxChannelBlockCount;

	// SDK reported AABB size is coordinate of next block after end of AABB
	// So we need to substract 1 from each dimension to get actual size
	const FIntVector MaxBlockGridSize = Header.AABBSize - FIntVector(1, 1, 1);

	const uint32 BlockSize = ZIBRAVDB_BLOCK_SIZE * ZIBRAVDB_BLOCK_SIZE * ZIBRAVDB_BLOCK_SIZE;

	const uint32 RenderBlockBufferElementsPerBlock = 4;

	ZibraVDBNativeResources = MakeShared<FZibraVDBNativeResource>();

	ZibraVDBNativeResources->RenderParameters.TextureSize = MaxBlockGridSize * ZIBRAVDB_BLOCK_SIZE;
	ZibraVDBNativeResources->RenderParameters.MaxBlockGridSize = MaxBlockGridSize;
	const uint32 MaxRenderBlockCountCBRT = FMath::CeilToInt(FMath::Pow(MaxSpatialBlockCount, 1.0f / 3.0f));
	ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRT = MaxRenderBlockCountCBRT;

	const auto MaxRenderBlockCountCBRTMultiplyAndShift =
		ZibraVDB::Utils::CalculateDivisionInvariantIntegers(MaxRenderBlockCountCBRT);
	const auto MaxRenderBlockCountCBRTMultiply = MaxRenderBlockCountCBRTMultiplyAndShift[0];
	const auto MaxRenderBlockCountCBRTShift1 = MaxRenderBlockCountCBRTMultiplyAndShift[1];
	const auto MaxRenderBlockCountCBRTShift2 = MaxRenderBlockCountCBRTMultiplyAndShift[2];
	ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTMultiply = MaxRenderBlockCountCBRTMultiply;
	ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTShift1 = MaxRenderBlockCountCBRTShift1;
	ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRTShift2 = MaxRenderBlockCountCBRTShift2;

	auto ResourcesRequirements = MaterialComponent.GetDecompressorManager().GetDecompressorResourcesRequirements();

	ZibraVDBNativeResources->DecompressionPerChannelBlockData.SetElementCount(
		ResourcesRequirements.decompressionPerChannelBlockDataSizeInBytes / sizeof(FFloat16));
	ZibraVDBNativeResources->DecompressionPerSpatialBlockInfo.SetElementCount(
		ResourcesRequirements.decompressionPerSpatialBlockInfoSizeInBytes / sizeof(uint32_t));
	ZibraVDBNativeResources->DecompressionPerChannelBlockInfo.SetElementCount(
		ResourcesRequirements.decompressionPerChannelBlockInfoSizeInBytes / sizeof(uint32_t));
	ZibraVDBNativeResources->RenderBlockBuffer.SetElementCount(MaxSpatialBlockCount * RenderBlockBufferElementsPerBlock);

	ReadFrameHeaders(MaterialComponent.GetDecompressorManager(), Header.ChannelNames);

	ENQUEUE_RENDER_COMMAND(InitNativeRenderingResources)
	([this](FRHICommandListImmediate& RHICmdList) { ZibraVDBNativeResources->InitResource(RHICmdList); });
}

void FZibraVDBRenderingResources::ReadFrameHeaders(
	const FZibraVDBDecompressorManager& DecompressorManager, const TArray<FString>& SequenceChannelNames)
{
	for (int i = 0; i < ZIB_ARR_SIZE(ZibraVDBNativeResources->RenderParameters.SequenceChannelScales); i++)
	{
		ZibraVDBNativeResources->RenderParameters.SequenceChannelScales[i] = 0.0f;
	}

	bool& bHasPerFrameEqualTransformations = ZibraVDBNativeResources->RenderParameters.bHasPerFrameEqualTransformations;
	bHasPerFrameEqualTransformations = true;

	Zibra::CE::Decompression::PlaybackInfo PlaybackInfo = DecompressorManager.GetPlaybackInfo();
	Zibra::CE::Decompression::FrameRange FrameRange = DecompressorManager.GetFrameRange();

	if (PlaybackInfo.frameCount == 0)
	{
		return;
	}

	ZibraVDBNativeResources->FrameHeaders.SetNum(PlaybackInfo.frameCount);
	for (int FrameIndex = FrameRange.start; FrameIndex <= FrameRange.end; FrameIndex += PlaybackInfo.sequenceIndexIncrement)
	{
		Zibra::CE::Decompression::FrameInfo FrameInfo = DecompressorManager.GetFrameInfo(FrameIndex);
		int32 FrameIndexRaw = int32(FrameIndex - PlaybackInfo.sequenceStartIndex) / int32(PlaybackInfo.sequenceIndexIncrement);
		FZibraVDBNativeResource::FFrameParameters& Frame = ZibraVDBNativeResources->FrameHeaders[FrameIndexRaw];

		Frame.SpatialBlockCount = FrameInfo.spatialBlockCount;
		Frame.ChannelBlockCount = FrameInfo.channelBlockCount;
		// SDK reported AABB size is coordinate of next block after end of AABB
		// So we need to substract 1 from each dimension to get actual size
		Frame.AABBSize = FIntVector(FrameInfo.AABBSize[0] - 1, FrameInfo.AABBSize[1] - 1, FrameInfo.AABBSize[2] - 1);
		Frame.FrameSize = Frame.AABBSize * ZIBRAVDB_BLOCK_SIZE;

		for (uint32_t i = 0; i < FrameInfo.channelsCount; i++)
		{
			Frame.ChannelScales[i] = FrameInfo.channels[i].maxGridValue;
			Frame.ChannelNames.Add(FrameInfo.channels[i].name);

			const int32 ChannelScaleIndex = SequenceChannelNames.Find(FrameInfo.channels[i].name);

			ZibraVDBNativeResources->RenderParameters.SequenceChannelScales[ChannelScaleIndex] = FMath::Max(
				Frame.ChannelScales[i], ZibraVDBNativeResources->RenderParameters.SequenceChannelScales[ChannelScaleIndex]);

			// If grid if empty(only 0 values) we set channel scale to 1 to eliminate divisions by zero later
			if (ZibraVDBNativeResources->RenderParameters.SequenceChannelScales[ChannelScaleIndex] == 0)
				ZibraVDBNativeResources->RenderParameters.SequenceChannelScales[ChannelScaleIndex] = 1;
		}

		const auto& Transform = FrameInfo.channels[0].gridTransform;
		Frame.IndexToWorldTransform = FMatrix44f({Transform.raw[0], Transform.raw[1], Transform.raw[2], Transform.raw[3]},
			{Transform.raw[4], Transform.raw[5], Transform.raw[6], Transform.raw[7]},
			{Transform.raw[8], Transform.raw[9], Transform.raw[10], Transform.raw[11]},
			{Transform.raw[12], Transform.raw[13], Transform.raw[14], Transform.raw[15]});

		FTransform UETransform(FMatrix(Frame.IndexToWorldTransform));

		bHasPerFrameEqualTransformations =
			bHasPerFrameEqualTransformations && IsChannelTransformsEqual(FrameInfo.channelsCount, FrameInfo.channels);

		const FVector3f VoxelScale = FVector3f(UETransform.GetScale3D());
		if (VoxelScale.IsNearlyZero())
		{
			Frame.IndexToWorldRotation = FMatrix44f::Identity;
			Frame.IndexToWorldScale = FVector3f::One();
			Frame.IndexToUnrealUnitsScale = FVector3f::One();
			Frame.IndexToWorldTranslationInVoxels = FVector3f::Zero();
			Frame.IndexToWorldTransformationInVoxels = FMatrix44f::Identity;

			continue;
		}

		Frame.IndexToWorldRotation = FMatrix44f(UETransform.GetRotation().ToMatrix());
		Frame.IndexToWorldScale = VoxelScale;
		Frame.IndexToUnrealUnitsScale = VoxelScale * ZibraVDB::Utils::MetersToUnrealUnits;
		Frame.IndexToWorldTranslationInVoxels = FVector3f(UETransform.GetTranslation()) / VoxelScale;
		Frame.IndexToWorldTransformationInVoxels = Frame.IndexToWorldRotation;
		Frame.IndexToWorldTransformationInVoxels.SetOrigin(Frame.IndexToWorldTranslationInVoxels);
	}
}

FZibraVDBRenderingResources::~FZibraVDBRenderingResources()
{
	SparseBlocksRGB->RemoveFromRoot();
	SparseBlocksDensity->RemoveFromRoot();
	MaxDensityPerBlockTexture->RemoveFromRoot();
	BlockIndexVolumeTexture->RemoveFromRoot();
	TransformTextureResult->RemoveFromRoot();
	if (IlluminatedVolumeTexture)
	{
		IlluminatedVolumeTexture->RemoveFromRoot();
	}
	ReleaseResources();
}

void FZibraVDBRenderingResources::UpdateIlluminatedVolumeTexture(const UZibraVDBMaterialComponent& MaterialComponent)
{
	const auto& TextureSize = ZibraVDBNativeResources->RenderParameters.TextureSize;
	const FIntVector3 IlluminatedVolumeResolution =
		MaterialComponent.bUseHeterogeneousVolume
			? FIntVector3(1)
			: FIntVector3(FMath::Max(TextureSize.X * MaterialComponent.IlluminatedVolumeResolutionScale, 1),
				  FMath::Max(TextureSize.Y * MaterialComponent.IlluminatedVolumeResolutionScale, 1),
				  FMath::Max(TextureSize.Z * MaterialComponent.IlluminatedVolumeResolutionScale, 1));
	if (IlluminatedVolumeTexture->SizeX != IlluminatedVolumeResolution.X ||
		IlluminatedVolumeTexture->SizeY != IlluminatedVolumeResolution.Y ||
		IlluminatedVolumeTexture->SizeZ != IlluminatedVolumeResolution.Z)
	{
		IlluminatedVolumeTexture->Init(
			IlluminatedVolumeResolution.X, IlluminatedVolumeResolution.Y, IlluminatedVolumeResolution.Z, PF_FloatRGBA);
	}
}

void FZibraVDBRenderingResources::UpdateSparseBlocksRGB(const UZibraVDBMaterialComponent& MaterialComponent)
{
	const int VolumeDownscaleFactor = GetDownscaleFactorExponent(MaterialComponent.VolumeResolution);
	const bool bEnableInterpolation = MaterialComponent.RayMarchingFiltering == ERayMarchingFiltering::Trilinear;
	const int32 MaxRenderBlockCountCBRT = ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRT;
	const int32 renderBlockSize =
		(ZIBRAVDB_BLOCK_SIZE / VolumeDownscaleFactor) + bEnableInterpolation * ZIBRAVDB_RENDER_BLOCK_PADDING;
	const int32 TextureSize = MaxRenderBlockCountCBRT * renderBlockSize;

	if (SparseBlocksRGB->SizeX != TextureSize)
	{
		SparseBlocksRGB->Init(TextureSize, TextureSize, TextureSize, PF_FloatR11G11B10);
	}
}

void FZibraVDBRenderingResources::UpdateSparseBlocksDensity(const UZibraVDBMaterialComponent& MaterialComponent)
{
	const int VolumeDownscaleFactor = GetDownscaleFactorExponent(MaterialComponent.VolumeResolution);
	const bool bEnableInterpolation = MaterialComponent.RayMarchingFiltering == ERayMarchingFiltering::Trilinear;
	const int32 MaxRenderBlockCountCBRT = ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRT;
	const bool bHasDensity = MaterialComponent.ZibraVDBAssetComponent->GetZibraVDBVolume()->GetDensityChannelIndex() != -1;
	const int32 renderBlockSize =
		(ZIBRAVDB_BLOCK_SIZE / VolumeDownscaleFactor) + bEnableInterpolation * ZIBRAVDB_RENDER_BLOCK_PADDING;
	const int32 TextureSize = bHasDensity ? MaxRenderBlockCountCBRT * renderBlockSize : 1;

	if (SparseBlocksDensity->SizeX != TextureSize)
	{
		SparseBlocksDensity->Init(TextureSize, TextureSize, TextureSize, PF_R8);
	}
}

void FZibraVDBRenderingResources::UpdateMaxDensityPerBlockTexture(const UZibraVDBMaterialComponent& MaterialComponent)
{
	const int32 MaxRenderBlockCountCBRT = ZibraVDBNativeResources->RenderParameters.MaxRenderBlockCountCBRT;
	const bool bHasDensity = MaterialComponent.ZibraVDBAssetComponent->GetZibraVDBVolume()->GetDensityChannelIndex() != -1;
	const int32 TextureSize = bHasDensity ? MaxRenderBlockCountCBRT : 1;

	if (MaxDensityPerBlockTexture->SizeX != TextureSize)
	{
		MaxDensityPerBlockTexture->Init(TextureSize, TextureSize, TextureSize, PF_R16F);
	}
}

bool FZibraVDBRenderingResources::IsChannelTransformsEqual(
	uint32_t ChannelCount, const Zibra::CE::Decompression::ChannelInfo (&channels)[Zibra::CE::MAX_CHANNEL_COUNT]) noexcept
{
	const auto IsEqual = [](const auto& T1, const auto& T2)
	{
		for (size_t Index = 0; Index < ZIB_ARR_SIZE(T1); Index++)
		{
			if (!FMath::IsNearlyEqual(T1[Index], T2[Index], UE_KINDA_SMALL_NUMBER))
			{
				return false;
			}
		}

		return true;
	};

	const auto IsIdentity = [&](const auto& Transform)
	{
		constexpr float IdentityTransform[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		return IsEqual(Transform, IdentityTransform);
	};

	uint32_t Index = 0;
	for (; Index < ChannelCount; Index++)
	{
		// TODO: after adding per channel AABB, validate them, as well.
		if (!IsIdentity(channels[Index].gridTransform.raw))
		{
			break;
		}
	}
	const auto& FirstTransform = channels[Index].gridTransform.raw;

	for (; Index < ChannelCount; Index++)
	{
		const auto& Transform = channels[Index].gridTransform.raw;

		// TODO: after adding per channel AABB, validate them, as well.
		if (IsIdentity(Transform))
		{
			continue;
		}

		if (!IsEqual(Transform, FirstTransform))
		{
			return false;
		}
	}
	return true;
}
