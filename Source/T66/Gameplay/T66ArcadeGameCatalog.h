// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ArcadeInteractableTypes.h"

struct FT66ArcadeGameCatalogEntry
{
	ET66ArcadeGameType GameType = ET66ArcadeGameType::None;
	FName RowID = NAME_None;
	FText PrototypeDisplayName;
	FText InternalDisplayName;
	FText Description;
	FText ShortCode;
	FLinearColor AccentColor = FLinearColor::White;
};

namespace T66ArcadeGameCatalog
{
	T66_API const TArray<FT66ArcadeGameCatalogEntry>& GetPlayableEntries();
	T66_API const FT66ArcadeGameCatalogEntry* FindEntry(ET66ArcadeGameType GameType);
	T66_API bool IsPlayable(ET66ArcadeGameType GameType);
	T66_API FName GetRowID(ET66ArcadeGameType GameType);
	T66_API FText GetPrototypeDisplayName(ET66ArcadeGameType GameType);
	T66_API FText GetInternalDisplayName(ET66ArcadeGameType GameType);
	T66_API FText GetDescription(ET66ArcadeGameType GameType);
	T66_API FText GetShortCode(ET66ArcadeGameType GameType);
	T66_API FLinearColor GetAccentColor(ET66ArcadeGameType GameType, int32 FallbackIndex = 0);
	T66_API bool TryResolveRowData(const UObject* WorldContextObject, FName RowID, FT66ArcadeInteractableData& OutData);
	T66_API bool BuildSessionDataForGame(const UObject* WorldContextObject, ET66ArcadeGameType GameType, FT66ArcadeInteractableData& OutData);
}
