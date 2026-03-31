// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "UObject/ObjectMacros.h"

#include "ZibraVDBVolumeDeprecated.generated.h"

UCLASS(meta = (DeprecatedNode, DeprecationMessage = "Use UZibraVDBVolume4 instead."))
class ZIBRAVDBRUNTIME_API UZibraVDBVolume final : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UZibraVDBVolume() noexcept;

	//~ Begin UObject Interface.
	void Serialize(FArchive& Ar) final;

	//~ End UObject Interface.

#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadOnly, Instanced, Category = ImportSettings)
	TObjectPtr<class UAssetImportData> AssetImportData;
#endif

public:
	UPROPERTY(VisibleAnywhere, Category = Details, DisplayName = "Original OpenVDB Size")
	float OriginalVDBSize = 0.f;
	UPROPERTY()
	float VRAMUsage = 0.f;
	UPROPERTY(VisibleAnywhere, Category = Details)
	FIntVector VDBSize = {0, 0, 0};
	UPROPERTY(VisibleAnywhere, Category = Compression, DisplayName = "Compression Rate")
	float CompressionRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = Compression)
	bool bIsLoadedFromZibraVDB = true;
	UPROPERTY()
	float CompressionQuality = 0.5f;
	UPROPERTY()
	bool bUseAdvancedCompressionSettings = false;
	UPROPERTY(VisibleAnywhere, Category = Compression, DisplayName = "Source ZibraVDB File")
	FFilePath SourceZibraVDBFile;
	UPROPERTY()
	bool bIsDensityChannelUsed = false;
	UPROPERTY()
	bool bIsTemperatureChannelUsed = false;
	UPROPERTY()
	bool bIsFlameChannelUsed = false;
	UPROPERTY(VisibleAnywhere, Category = Compression, DisplayName = "Source VDB Directory")
	FDirectoryPath SourceOpenVDBDirectory;
	UPROPERTY()
	TArray<FString> AvailableChannels;
	UPROPERTY()
	FString DensityChannelName;
	UPROPERTY()
	FString TemperatureChannelName;
	UPROPERTY()
	FString FlameChannelName;

private:
	FString FileName;
	uint32_t FrameCount;
	FIntVector MinBBox;
	FIntVector MaxBBox;
	uint32_t MaxBlockCount;
	float CompressionRate2;
	float ZeroOffset;
	float QuantizationParameter;
	TStaticArray<bool, 3> UsedChannels;
	TStaticArray<float, 2> TemperatureRange;
	TArray<uint8> BinaryData;
};
