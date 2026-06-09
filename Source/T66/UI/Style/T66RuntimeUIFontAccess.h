// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"

// Centralizes the file-backed Slate font resolution path so runtime font
// dependencies have a single owner.
namespace T66RuntimeUIFontAccess
{
	FString ResolveLockedUIFontPath();
	FString ResolveFriendslopUIFontPath();
	bool IsBoldWeight(const TCHAR* Weight);
	FSlateFontInfo MakeFontFromAbsoluteFile(const FString& Path, int32 Size);
	FSlateFontInfo MakeFriendslopFont(int32 Size, bool bBold);
	FSlateFontInfo MakeLocalizedEngineFont(int32 Size, bool bBold);
	FSlateFontInfo MakeLocalizedEngineSlateFont(const TCHAR* RelativeFontFile, int32 Size);
}
