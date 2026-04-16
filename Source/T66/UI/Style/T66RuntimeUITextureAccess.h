// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UTexture2D;

namespace T66RuntimeUITextureAccess
{
	T66_API FString MakeProjectDirPath(const FString& RelativePath);
	T66_API FString MakeProjectContentPath(const FString& RelativePath);
	T66_API FString MakeProjectRuntimeDependencyPath(const FString& RelativePath);
	T66_API FString MapSourceRelativePathToRuntimeDependencyRelativePath(const FString& RelativePath);
	T66_API TArray<FString> BuildLooseTextureCandidatePaths(const FString& RelativePath);

	T66_API UTexture2D* LoadAssetTexture(
		const TCHAR* AssetPath,
		TextureFilter Filter = TextureFilter::TF_Trilinear,
		const TCHAR* DebugLabel = nullptr);

	T66_API UTexture2D* ImportFileTexture(
		const FString& FilePath,
		TextureFilter Filter = TextureFilter::TF_Trilinear,
		bool bClamp = false,
		const TCHAR* DebugLabel = nullptr);

	T66_API UTexture2D* ImportFileTextureWithGeneratedMips(
		const FString& FilePath,
		TextureFilter Filter = TextureFilter::TF_Trilinear,
		const TCHAR* DebugLabel = nullptr);
}
