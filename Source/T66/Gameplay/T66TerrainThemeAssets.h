#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"

class UMaterialInterface;
class UTexture;
enum class ET66Difficulty : uint8;

class T66_API FT66TerrainThemeAssets
{
public:
	static UTexture* LoadDifficultyGroundTexture(ET66Difficulty Difficulty);
	static UMaterialInterface* ResolveDifficultyGroundMaterial(UObject* Outer, ET66Difficulty Difficulty);
	static void FillDefaultCliffSideMaterials(TArray<TSoftObjectPtr<UMaterialInterface>>& OutMaterials);
};
