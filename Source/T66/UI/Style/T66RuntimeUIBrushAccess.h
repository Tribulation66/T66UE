// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "UObject/StrongObjectPtr.h"

class UTexture2D;

DECLARE_LOG_CATEGORY_EXTERN(LogT66RuntimeUI, Log, All);

namespace T66RuntimeUIBrushAccess
{
	enum class ET66DotaPlateBrushKind : uint8
	{
		Neutral,
		Primary,
		Danger,
	};

	struct T66_API FOptionalTextureBrush
	{
		TStrongObjectPtr<UTexture2D> ImportedTexture;
		TStrongObjectPtr<UTexture2D> FileTexture;
		TSharedPtr<FSlateBrush> Brush;
		bool bChecked = false;
		bool bSimpleReferenceFallback = false;

		UTexture2D* GetTexture() const;
	};

	T66_API const FSlateBrush* ResolveDotaButtonPlateBrush(ET66DotaPlateBrushKind Kind);

	T66_API bool ShouldUseSimpleReferenceFallback(const FString& SourcePath);

	T66_API void ConfigureSimpleReferenceFallbackBrush(
		FSlateBrush& Brush,
		const FString& SourcePath,
		const FVector2D& ImageSize,
		const FMargin& Margin,
		ESlateBrushDrawType::Type DrawAs = ESlateBrushDrawType::Box);

	T66_API UTexture2D* LoadOptionalTexture(
		FOptionalTextureBrush& Entry,
		const TCHAR* ImportedAssetPath,
		const FString& FallbackFilePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel = nullptr,
		TextureFilter Filter = TextureFilter::TF_Trilinear);

	T66_API const FSlateBrush* ResolveOptionalTextureBrush(
		FOptionalTextureBrush& Entry,
		const TCHAR* ImportedAssetPath,
		const FString& FallbackFilePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel = nullptr,
		TextureFilter Filter = TextureFilter::TF_Trilinear);
}
