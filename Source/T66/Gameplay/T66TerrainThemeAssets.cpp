// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TerrainThemeAssets.h"

#include "Data/T66DataTypes.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	struct FT66DifficultyGroundThemeAssetInfo
	{
		const TCHAR* FolderName = nullptr;
		const TCHAR* AssetSuffix = nullptr;
	};

	static FT66DifficultyGroundThemeAssetInfo T66GetDifficultyGroundThemeAssetInfo(const ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Medium: return { TEXT("VeryHardGraveyard"), TEXT("VeryHardGraveyard") };
		case ET66Difficulty::Hard: return { TEXT("ImpossibleNorthPole"), TEXT("ImpossibleNorthPole") };
		case ET66Difficulty::VeryHard: return { TEXT("PerditionMars"), TEXT("PerditionMars") };
		case ET66Difficulty::Impossible: return { TEXT("FinalHell"), TEXT("FinalHell") };
		case ET66Difficulty::Easy:
		default:
			return {};
		}
	}
}

UTexture* FT66TerrainThemeAssets::LoadDifficultyGroundTexture(ET66Difficulty Difficulty)
{
	static TMap<FString, TWeakObjectPtr<UTexture>> CachedTextures;

	FString TexturePath = TEXT("/Game/World/Terrain/Megabonk/T_MegabonkBlock.T_MegabonkBlock");
	if (Difficulty != ET66Difficulty::Easy)
	{
		const FT66DifficultyGroundThemeAssetInfo ThemeInfo = T66GetDifficultyGroundThemeAssetInfo(Difficulty);
		if (ThemeInfo.FolderName && ThemeInfo.AssetSuffix)
		{
			const FString AssetName = FString::Printf(TEXT("T_MegabonkBlock_%s"), ThemeInfo.AssetSuffix);
			TexturePath = FString::Printf(
				TEXT("/Game/World/Terrain/MegabonkThemes/%s/%s.%s"),
				ThemeInfo.FolderName,
				*AssetName,
				*AssetName);
		}
	}

	if (const TWeakObjectPtr<UTexture>* Existing = CachedTextures.Find(TexturePath))
	{
		if (Existing->IsValid())
		{
			return Existing->Get();
		}
	}

	UTexture* LoadedTexture = LoadObject<UTexture>(nullptr, *TexturePath);
	if (!LoadedTexture && Difficulty != ET66Difficulty::Easy)
	{
		LoadedTexture = LoadObject<UTexture>(nullptr, TEXT("/Game/World/Terrain/Megabonk/T_MegabonkBlock.T_MegabonkBlock"));
	}

	CachedTextures.Add(TexturePath, LoadedTexture);
	return LoadedTexture;
}

UMaterialInterface* FT66TerrainThemeAssets::ResolveDifficultyGroundMaterial(UObject* Outer, ET66Difficulty Difficulty)
{
	static TMap<int32, TWeakObjectPtr<UMaterialInterface>> CachedMaterials;
	const int32 CacheKey = static_cast<int32>(Difficulty);
	if (const TWeakObjectPtr<UMaterialInterface>* Existing = CachedMaterials.Find(CacheKey))
	{
		if (Existing->IsValid())
		{
			return Existing->Get();
		}
	}

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
	UTexture* DifficultyTexture = LoadDifficultyGroundTexture(Difficulty);
	if (BaseMaterial && DifficultyTexture)
	{
		if (UMaterialInstanceDynamic* GroundMID = UMaterialInstanceDynamic::Create(BaseMaterial, Outer ? Outer : GetTransientPackage()))
		{
			GroundMID->SetTextureParameterValue(TEXT("DiffuseColorMap"), DifficultyTexture);
			GroundMID->SetTextureParameterValue(TEXT("BaseColorTexture"), DifficultyTexture);
			GroundMID->SetScalarParameterValue(TEXT("Brightness"), 1.0f);
			GroundMID->SetVectorParameterValue(TEXT("Tint"), FLinearColor::White);
			GroundMID->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor::White);
			CachedMaterials.Add(CacheKey, GroundMID);
			return GroundMID;
		}
	}

	UMaterialInterface* FallbackMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/World/Terrain/Megabonk/MI_MegabonkBlock.MI_MegabonkBlock"));
	CachedMaterials.Add(CacheKey, FallbackMaterial);
	return FallbackMaterial;
}

void FT66TerrainThemeAssets::FillDefaultCliffSideMaterials(TArray<TSoftObjectPtr<UMaterialInterface>>& OutMaterials)
{
	OutMaterials.Empty();
	OutMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile1.MI_HillTile1"))));
	OutMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile2.MI_HillTile2"))));
	OutMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile3.MI_HillTile3"))));
	OutMaterials.Add(TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/World/Cliffs/MI_HillTile4.MI_HillTile4"))));
}
