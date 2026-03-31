// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#include "ZibraVDBVolume4.h"

#include "EditorFramework/AssetImportData.h"
#include "HAL/FileManager.h"
#include "Render/ZibraVDBRenderBuffer.h"
#include "RenderingThread.h"
#include "UObject/MetaData.h"
#include "UObject/NoExportTypes.h"
#include "UnrealClient.h"

UZibraVDBVolume4::UZibraVDBVolume4() noexcept = default;
UZibraVDBVolume4::UZibraVDBVolume4(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UZibraVDBVolume4::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
#endif
}

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
void UZibraVDBVolume4::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

#if WITH_EDITORONLY_DATA
	if (AssetImportData)
	{
		Context.AddTag(
			FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}
#endif
}
#else
void UZibraVDBVolume4::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);

#if WITH_EDITORONLY_DATA
	if (AssetImportData)
	{
		OutTags.Add(
			FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}
#endif
}
#endif

void UZibraVDBVolume4::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << ZibraVDBFile;
}

void UZibraVDBVolume4::PostLoad()
{
	Super::PostLoad();
	UpdateUProperties();
}

uint64_t GetVRAMUsageRendering(const ZibraVDB::FZibraVDBHeader& Header, uint64_t FileSize)
{
	constexpr int32 BlockSize = ZIBRAVDB_BLOCK_SIZE * ZIBRAVDB_BLOCK_SIZE * ZIBRAVDB_BLOCK_SIZE;
	const FIntVector BlockGridSize = Header.AABBSize;

	const uint64 MaxSpatialBlockCount = Header.MaxSpatialBlockCount;
	const uint64 MaxChannelBlockCount = Header.MaxChannelBlockCount;
	const FIntVector TextureSize = BlockGridSize * ZIBRAVDB_BLOCK_SIZE;
	constexpr int DownscaledFactor = 4;
	const FIntVector DownscaledTextureSize = FIntVector::DivideAndRoundUp(TextureSize, DownscaledFactor);
	const uint64 MaxRenderBlockCountCBRT = FMath::CeilToInt(FMath::Pow(MaxSpatialBlockCount, 1.0 / 3.0));
	const bool bEnableInterpolation = MaxSpatialBlockCount <= ZIBRAVDB_MAX_BLOCKS_FOR_RAYMARCHING_INTERPOLATION;
	const uint64 RenderBlocksTextureSize =
		MaxRenderBlockCountCBRT * (bEnableInterpolation ? ZIBRAVDB_RENDER_BLOCK_SIZE : ZIBRAVDB_BLOCK_SIZE);
	constexpr uint64_t MAX_NODES = (BlockSize * 2);
	constexpr uint64_t TYPE_COUNT = 14;
	constexpr uint64_t BINFO_COUNT = 3;
	constexpr uint64_t CH_BINFO_COUNT = 2;
	constexpr uint64_t PARAMETERS_COUNT = 9;

	// Decompression
	const uint64 DecompressionBitstream = double(FileSize) / Header.FrameCount * 2.0;	 // Rough estimate
	constexpr uint64 DecompressionTreeBuffer = (MAX_NODES * TYPE_COUNT) * sizeof(uint32_t);
	constexpr uint64 DecompressionDecodeDictionary = (MAX_NODES * TYPE_COUNT) * sizeof(uint32_t);
	constexpr uint64 DecompressionCoefficientReorder = BlockSize * sizeof(uint32);
	const uint64 DecompressionBlockBitOffset = MaxSpatialBlockCount * sizeof(uint32);
	const uint64 DecompressionBlockBitSize = MaxSpatialBlockCount * sizeof(uint32);
	const uint64 DecompressionChannelBitOffset = MaxChannelBlockCount * sizeof(uint32);
	const uint64 DecompressionChannelBlockInfo = MaxChannelBlockCount * CH_BINFO_COUNT * sizeof(uint32);
	const uint64 DecompressionPerChannelParameters = Header.ChannelNames.Num() * PARAMETERS_COUNT * sizeof(uint32);

	const uint64 Decompression = DecompressionBitstream + DecompressionTreeBuffer + DecompressionDecodeDictionary +
								 DecompressionCoefficientReorder + DecompressionBlockBitOffset + DecompressionBlockBitSize +
								 DecompressionChannelBitOffset + DecompressionChannelBlockInfo + DecompressionPerChannelParameters;

	// Rendering
	const uint64 DecompressionPerChannelBlockData = MaxChannelBlockCount * BlockSize * sizeof(uint16_t);
	const uint64 DecompressionPerChannelBlockInfo = MaxChannelBlockCount * sizeof(uint32);
	const uint64 DecompressionPerSpatialBlockInfo = MaxSpatialBlockCount * BINFO_COUNT * sizeof(uint32);
	const uint64 RenderBlockBuffer = MaxSpatialBlockCount * 4 * sizeof(uint32);
	const uint64 BlockIndexTexture = BlockGridSize.X * BlockGridSize.Y * BlockGridSize.Z * sizeof(uint32);	  // PF_R32_UINT
	const uint64 Illumination =
		DownscaledTextureSize.X * DownscaledTextureSize.Y * DownscaledTextureSize.Z * 4 * sizeof(uint16);	 // PF_FloatRGBA
	const uint64 IlluminatedVolume =
		DownscaledTextureSize.X * DownscaledTextureSize.Y * DownscaledTextureSize.Z * 4 * sizeof(uint16);	 // PF_FloatRGBA
	const uint64 ZibVolumeTextureLOD =
		DownscaledTextureSize.X * DownscaledTextureSize.Y * DownscaledTextureSize.Z * sizeof(uint16);	 // PF_R16F
	const uint64 SparseBlocksRGB =
		RenderBlocksTextureSize * RenderBlocksTextureSize * RenderBlocksTextureSize * sizeof(uint32);	 // PF_FloatR11G11B10
	const uint64 SparseBlocksDensity =
		RenderBlocksTextureSize * RenderBlocksTextureSize * RenderBlocksTextureSize * sizeof(uint8);	// PF_R8
	const uint64 SparseBlocksDensityScales =
		MaxRenderBlockCountCBRT * MaxRenderBlockCountCBRT * MaxRenderBlockCountCBRT * sizeof(uint16);	 // PF_R16F

	const uint64 Rendering = DecompressionPerChannelBlockData + DecompressionPerChannelBlockInfo +
							 DecompressionPerSpatialBlockInfo + RenderBlockBuffer + BlockIndexTexture + Illumination +
							 IlluminatedVolume + ZibVolumeTextureLOD + SparseBlocksRGB + SparseBlocksDensity +
							 SparseBlocksDensityScales;

	return Decompression + Rendering;
}

void UZibraVDBVolume4::Import(ZibraVDB::FZibraVDBFile&& InZibraVDBFile)
{
	ZibraVDBFile = MoveTemp(InZibraVDBFile);
	UpdateUProperties();
}

void UZibraVDBVolume4::UpdateUProperties() noexcept
{
	const ZibraVDB::FZibraVDBHeader& Header = ZibraVDBFile.GetHeader();
	const ZibraVDB::FZibraVDBMetadata& Metadata = ZibraVDBFile.GetMetadata();
	FrameCount = Header.FrameCount;
	OriginalVDBSize = Header.OriginalSize / 1024. / 1024. / 1024.;
	CompressionRate = Metadata.CompressionRate;
	VRAMUsage = GetVRAMUsageRendering(Header, ZibraVDBFile.GetBinaryData().Num()) / 1024. / 1024.;

	const auto& Box = Header.AABBSize;
	// SDK reported AABB size is coordinate of next block after end of AABB
	// So we need to substract 1 from each dimension to get actual size
	VDBSize = FIntVector{(Box.X - 1) * ZIBRAVDB_BLOCK_SIZE, (Box.Y - 1) * ZIBRAVDB_BLOCK_SIZE, (Box.Z - 1) * ZIBRAVDB_BLOCK_SIZE};

	SourceZibraVDBFile.FilePath = ZibraVDBFile.GetFileName();

	ChannelNames = Header.ChannelNames;
	// Add default PerChannelCompressionSettings for each channel if not exist.
	for (const auto& ChannelName : ChannelNames)
	{
		if (!PerChannelCompressionSettings.Contains(ChannelName))
		{
			PerChannelCompressionSettings.Add({ChannelName});
		}
	}

	if (DensityChannel.IsEmpty())
	{
		DensityChannel = NoneChannel;
		for (const auto& name : {"density", "Density"})
		{
			if (IsChannelValid(name))
			{
				DensityChannel = name;
				break;
			}
		}
	}

	if (TemperatureChannel.IsEmpty())
	{
		TemperatureChannel = NoneChannel;
		for (const auto& name : {"temperature", "Temperature"})
		{
			if (IsChannelValid(name))
			{
				TemperatureChannel = name;
				break;
			}
		}
	}

	if (FlamesChannel.IsEmpty())
	{
		FlamesChannel = NoneChannel;
		for (const auto& name : {"flame", "flames", "Flame", "Flames"})
		{
			if (IsChannelValid(name))
			{
				FlamesChannel = name;
				break;
			}
		}
	}
}

uint32_t UZibraVDBVolume4::GetDensityChannelIndex() const
{
	return GetChannelIndex(DensityChannel);
}

uint32_t UZibraVDBVolume4::GetTemperatureChannelIndex() const
{
	return GetChannelIndex(TemperatureChannel);
}

uint32_t UZibraVDBVolume4::GetFlamesChannelIndex() const
{
	return GetChannelIndex(FlamesChannel);
}

uint32_t UZibraVDBVolume4::GetDensityChannelMask() const
{
	return GetChannelMask(DensityChannel);
}

uint32_t UZibraVDBVolume4::GetTemperatureChannelMask() const
{
	return GetChannelMask(TemperatureChannel);
}

uint32_t UZibraVDBVolume4::GetFlamesChannelMask() const
{
	return GetChannelMask(FlamesChannel);
}

TArray<FString> UZibraVDBVolume4::GetChannelNamesUI() const
{
	// We want "None" to be the first option in the UI
	TArray<FString> Result;
	Result.Add(NoneChannel);
	for (const FString& ChannelName : ChannelNames)
	{
		Result.Add(ChannelName);
	}
	return Result;
}

uint32_t UZibraVDBVolume4::GetChannelIndex(const FString& ChannelName) const
{
	return ChannelNames.IndexOfByKey(ChannelName);
}

uint32_t UZibraVDBVolume4::GetChannelMask(const FString& ChannelName) const
{
	const int32 Index = ChannelNames.IndexOfByKey(ChannelName);
	return (Index != INDEX_NONE) ? (1 << Index) : 0;
}

bool UZibraVDBVolume4::IsChannelValid(const FString& ChannelName) const
{
	return ChannelNames.Contains(ChannelName);
}

const ZibraVDB::FZibraVDBFile& UZibraVDBVolume4::GetZibraVDBFile() const noexcept
{
	return ZibraVDBFile;
}

#if WITH_EDITOR

FString UZibraVDBVolume4::GetUEAssetID()
{
	UPackage* Package = Cast<UPackage>(GetOuter());
	return Package ? Package->GetPersistentGuid().ToString() : "";
}

FString UZibraVDBVolume4::GetCompressedFileUUID()
{
	const auto& UUID = GetZibraVDBFile().GetHeader().UUID;
	return FString::Printf(TEXT("%016llx%016llx"), UUID[0], UUID[1]);
}

#endif
