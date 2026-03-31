// Copyright Zibra AI Inc 2025-2026. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Render/ZibraVDBRenderBuffer.h"
#include "Subsystems/EngineSubsystem.h"
#include "ZibraVDBVolume4.h"
#include "ZibraVDBDecompressorManager.h"

class UZibraVDBMaterialComponent;
enum class ERayMarchingFiltering : uint8;

#include "ZibraVDBRenderingResourcesSubsystem.generated.h"

class FZibraVDBRenderingResources
{
public:
	TObjectPtr<UTextureRenderTargetVolume> SparseBlocksRGB = nullptr;
	TObjectPtr<UTextureRenderTargetVolume> SparseBlocksDensity = nullptr;
	TObjectPtr<UTextureRenderTargetVolume> MaxDensityPerBlockTexture = nullptr;
	TObjectPtr<UTextureRenderTargetVolume> BlockIndexVolumeTexture = nullptr;
	TObjectPtr<UTextureRenderTargetVolume> TransformTextureResult = nullptr;
	TObjectPtr<UTextureRenderTargetVolume> IlluminatedVolumeTexture = nullptr;

	TSharedPtr<FZibraVDBNativeResource> ZibraVDBNativeResources;

	~FZibraVDBRenderingResources();

	friend UZibraVDBRenderingResourcesSubsystem;

public:
	void UpdateIlluminatedVolumeTexture(const UZibraVDBMaterialComponent& MaterialComponent);
	void UpdateSparseBlocksRGB(const UZibraVDBMaterialComponent& MaterialComponent);
	void UpdateSparseBlocksDensity(const UZibraVDBMaterialComponent& MaterialComponent);
	void UpdateMaxDensityPerBlockTexture(const UZibraVDBMaterialComponent& MaterialComponent);

private:
	void BeforeInitResources();
	void InitResources(const UZibraVDBMaterialComponent& MaterialComponent);
	void InitNativeResources(const UZibraVDBMaterialComponent& MaterialComponent);
	void ReadFrameHeaders(const FZibraVDBDecompressorManager& DecompressorManager, const TArray<FString>& SequenceChannelNames);
	void ReleaseTextures();
	void ReleaseResources();

	static bool IsChannelTransformsEqual(
		uint32_t ChannelCount, const Zibra::CE::Decompression::ChannelInfo (&channels)[Zibra::CE::MAX_CHANNEL_COUNT]) noexcept;
};

UCLASS()
class ZIBRAVDBRUNTIME_API UZibraVDBRenderingResourcesSubsystem : public UEngineSubsystem
{
	GENERATED_UCLASS_BODY()

public:
	TSharedPtr<FZibraVDBRenderingResources> InitResourcesForActor(
		const FString& ActorName, const UZibraVDBMaterialComponent& MaterialComponent);

	void ReInitResourcesForActor(const FString& ActorName, const UZibraVDBMaterialComponent& MaterialComponent);

	void RemoveIfInvalid(const FString& ActorName);

private:
	TMap<FString, TWeakPtr<FZibraVDBRenderingResources>> ResourcesMap;
};
