// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ArcadeInteractableTypes.h"
#include "Templates/SubclassOf.h"
#include "UI/T66UITypes.h"
#include "UI/WidgetGames/T66WidgetGameTypes.h"

class UUserWidget;

struct T66_API FT66WidgetGameDescriptor
{
	FName GameID = NAME_None;
	FName LegacyGameID = NAME_None;
	ET66WidgetGameCategory Category = ET66WidgetGameCategory::Arcade;
	ET66WidgetGamePlayModel PlayModel = ET66WidgetGamePlayModel::RealtimePopup;
	FText DisplayName = FText::GetEmpty();
	FText Description = FText::GetEmpty();
	FText ShortCode = FText::GetEmpty();
	FLinearColor AccentColor = FLinearColor::White;
	FName DemoGateID = NAME_None;
	ET66WidgetGameDemoGateKind DemoGateKind = ET66WidgetGameDemoGateKind::None;
	ET66WidgetGameLaunchKind LaunchKind = ET66WidgetGameLaunchKind::ArcadePopup;
	TSubclassOf<UUserWidget> WidgetClass = nullptr;
	ET66ScreenType FrontendScreenType = ET66ScreenType::None;
	ET66ArcadeGameType ArcadeGameType = ET66ArcadeGameType::None;
	FName CasinoPageID = NAME_None;
	FName AssetRoot = NAME_None;
	FName BackendGameToken = NAME_None;
	int32 SortOrder = 0;
	FT66WidgetGameCapabilityFlags Capabilities;
};
