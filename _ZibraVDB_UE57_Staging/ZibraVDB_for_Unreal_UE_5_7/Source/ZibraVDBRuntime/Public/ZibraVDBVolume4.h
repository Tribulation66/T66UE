// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "UObject/ObjectMacros.h"
#include "ZibraVDBCommon.h"

#include "ZibraVDBVolume4.generated.h"

namespace ZibraVDB
{
	class FZibraVDBFile;
}

USTRUCT()
struct FCompressionSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	bool bActive = true;

	UPROPERTY()
	float Quality = 0.6f;

	bool operator==(const FString& Channel) const
	{
		return Name == Channel;
	}
};

UCLASS(BlueprintType, Config = Engine, hidecategories = (Object))
class ZIBRAVDBRUNTIME_API UZibraVDBVolume4 final : public UObject
{
	GENERATED_UCLASS_BODY()

	static constexpr const TCHAR* NoneChannel = TEXT("None");

public:
	UZibraVDBVolume4() noexcept;

	void Import(ZibraVDB::FZibraVDBFile&& InZibraVDBFile);

	//~ Begin UObject Interface.
	void Serialize(FArchive& Ar) final;
	void PostInitProperties() final;
	void PostLoad() final;

#if (100 * ENGINE_MAJOR_VERSION + ENGINE_MINOR_VERSION) >= 504
	void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const final;
#else
	void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const final;
#endif

	//~ End UObject Interface.

#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadOnly, Instanced, Category = ImportSettings)
	TObjectPtr<class UAssetImportData> AssetImportData;
#endif

	const ZibraVDB::FZibraVDBFile& GetZibraVDBFile() const noexcept;

public:
	// User configurable properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Channels, meta = (GetOptions = "GetChannelNamesUI"))
	FString DensityChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Channels, meta = (GetOptions = "GetChannelNamesUI"))
	FString TemperatureChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Channels, meta = (GetOptions = "GetChannelNamesUI"))
	FString FlamesChannel;

	// Total number of frames in the effect
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Config, Transient, Category = Details)
	int FrameCount = 0;

	// Original OpenVDB size in GiB
	UPROPERTY(VisibleAnywhere, Config, Transient, Category = Details, DisplayName = "Original OpenVDB Size (GiB)")
	float OriginalVDBSize = 0.f;

	// Approximate VRAM usage for decompression and rendering in megabytes
	UPROPERTY(VisibleAnywhere, Config, Transient, Category = Details, DisplayName = "VRAM usage (MiB)")
	float VRAMUsage = 0.f;

	UPROPERTY(VisibleAnywhere, Config, Transient, Category = Details)
	FIntVector VDBSize = {0, 0, 0};

	// Ratio between original OpenVDB size and compressed ZibraVDB size.
	UPROPERTY(VisibleAnywhere, Config, Transient, Category = Import, DisplayName = "Compression Rate")
	float CompressionRate = 0.f;

	UPROPERTY(VisibleAnywhere, Config, Category = Import)
	bool bIsLoadedFromZibraVDB = true;

	/** Sets compression quality. Larger values result in larger file size, but more accurate compression result. */
	UPROPERTY(EditAnywhere, Config, Category = Import,
		meta = (EditCondition = "!bIsLoadedFromZibraVDB", ClampMin = 0.0f, ClampMax = 1.0f, UIMin = 0.0f, UIMax = 1.0f))
	float Quality = 0.6f;

	UPROPERTY(EditAnywhere, Category = Import, meta = (EditCondition = "!bIsLoadedFromZibraVDB", NoResetToDefault))
	bool bUsePerChannelCompressionSettings = false;

	UPROPERTY(EditAnywhere, Category = Import, meta = (NoResetToDefault))
	bool PerChannelCompressionSettingsPlaceholder; // Placeholder

	/// Section loaded from ZibraVDB file

	// Source file path to ZibraVDB
	UPROPERTY(EditAnywhere, Config, Category = Import, DisplayName = "Source ZibraVDB File",
		meta = (FilePathFilter = "ZibraVDB file (*.zibravdb)|*.zibravdb", EditCondition = "bIsLoadedFromZibraVDB",
			EditConditionHides))
	FFilePath SourceZibraVDBFile;

	/// Section loaded from OpenVDB directory

	// Source directory path to OpenVDB VFX
	UPROPERTY(EditAnywhere, Config, Category = Import, DisplayName = "Source OpenVDB Directory",
		meta = (EditCondition = "!bIsLoadedFromZibraVDB", EditConditionHides, FilePathFilter = "OpenVDB File (*.vdb)|*.vdb",
			NoResetToDefault))
	FFilePath SourceOpenVDBFilePath;
	
	UPROPERTY(VisibleAnywhere, Config, Transient, Category = Channels)
	TArray<FString> ChannelNames;

	UPROPERTY()
	TArray<FCompressionSettings> PerChannelCompressionSettings;

	// TODO: Refactor this. Asset editor only data should not reside is ZibraVDBVolume
	UPROPERTY(Transient)
	TArray<FCompressionSettings> PerChannelReimportSettings;

	


#if WITH_EDITOR
	FString GetUEAssetID();
	FString GetCompressedFileUUID();
#endif

	void UpdateUProperties() noexcept;

	uint32_t GetDensityChannelIndex() const;
	uint32_t GetTemperatureChannelIndex() const;
	uint32_t GetFlamesChannelIndex() const;
	uint32_t GetDensityChannelMask() const;
	uint32_t GetTemperatureChannelMask() const;
	uint32_t GetFlamesChannelMask() const;

private:
	UFUNCTION()
	TArray<FString> GetChannelNamesUI() const;

	uint32_t GetChannelIndex(const FString& ChannelName) const;
	uint32_t GetChannelMask(const FString& ChannelName) const;
	bool IsChannelValid(const FString& ChannelName) const;
	ZibraVDB::FZibraVDBFile ZibraVDBFile;
};
