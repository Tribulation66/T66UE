// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UTexture2D;

namespace T66LegacyUITextureAccess
{
	FString MakeProjectDirPath(const FString& RelativePath);
	FString MakeProjectContentPath(const FString& RelativePath);
	FString MakeProjectRuntimeDependencyPath(const FString& RelativePath);
	FString MapLegacySourceRelativePathToRuntimeDependencyRelativePath(const FString& RelativePath);
	TArray<FString> BuildLooseTextureCandidatePaths(const FString& RelativePath);

	UTexture2D* LoadAssetTexture(
		const TCHAR* AssetPath,
		TextureFilter Filter = TextureFilter::TF_Trilinear,
		const TCHAR* DebugLabel = nullptr);

	UTexture2D* ImportFileTexture(
		const FString& FilePath,
		TextureFilter Filter = TextureFilter::TF_Trilinear,
		bool bClamp = false,
		const TCHAR* DebugLabel = nullptr);

	UTexture2D* ImportFileTextureWithGeneratedMips(
		const FString& FilePath,
		TextureFilter Filter = TextureFilter::TF_Trilinear,
		const TCHAR* DebugLabel = nullptr);
}
