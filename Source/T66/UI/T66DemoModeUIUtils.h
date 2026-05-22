// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"

namespace T66DemoModeUI
{
	bool IsDemoModeActive(const UObject* WorldContextObject);
	bool IsArcadeGameAllowed(const UObject* WorldContextObject, FName ArcadeGameID);
	bool IsCasinoGameAllowed(const UObject* WorldContextObject, FName CasinoGameID);
	FText GetUnavailableContentText(const UObject* WorldContextObject);
	TSharedRef<SWidget> WrapWithComingSoonOverlay(
		const TSharedRef<SWidget>& Content,
		bool bShowOverlay,
		const UObject* WorldContextObject,
		FName Tag = NAME_None);
}
