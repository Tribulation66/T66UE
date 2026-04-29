// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66TerrainThemeAssets.h"

#include "Data/T66DataTypes.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	template <typename TObjectType>
	static TObjectType* T66FindOrLoadObject(const TCHAR* ObjectPath)
	{
		if (!ObjectPath || !*ObjectPath)
		{
			return nullptr;
		}

		if (TObjectType* Existing = FindObject<TObjectType>(nullptr, ObjectPath))
		{
			return Existing;
		}

		return LoadObject<TObjectType>(nullptr, ObjectPath);
	}
}

UTexture* FT66TerrainThemeAssets::LoadDifficultyGroundTexture(ET66Difficulty)
{
	static TMap<FString, TWeakObjectPtr<UTexture>> CachedTextures;

	const FString TexturePath = TEXT("/Game/World/Terrain/TowerDungeon/T_TowerDungeonGround.T_TowerDungeonGround");

	if (const TWeakObjectPtr<UTexture>* Existing = CachedTextures.Find(TexturePath))
	{
		if (Existing->IsValid())
		{
			return Existing->Get();
		}
	}

	UTexture* LoadedTexture = T66FindOrLoadObject<UTexture>(*TexturePath);

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

	UMaterialInterface* BaseMaterial = T66FindOrLoadObject<UMaterialInterface>(TEXT("/Game/Materials/M_Environment_Unlit.M_Environment_Unlit"));
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

	UMaterialInterface* FallbackMaterial = T66FindOrLoadObject<UMaterialInterface>(TEXT("/Game/World/Terrain/TowerDungeon/MI_TowerDungeonGround.MI_TowerDungeonGround"));
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
