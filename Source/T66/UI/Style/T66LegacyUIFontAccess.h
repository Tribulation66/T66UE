// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"

// Centralizes the legacy file-backed Slate font resolution path so the raw
// runtime dependency has a single owner during cleanup.
namespace T66LegacyUIFontAccess
{
	FString ResolveRadianceFontPath();
	FString ResolveReaverBoldFontPath();
	FString ResolveThemeFontPath(const TCHAR* RelativeThemePath);
	bool IsBoldWeight(const TCHAR* Weight);
	FSlateFontInfo MakeFontFromAbsoluteFile(const FString& Path, int32 Size);
	FSlateFontInfo MakeLocalizedEngineFont(int32 Size, bool bBold);
}
