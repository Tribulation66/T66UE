// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DeckDataTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "T66DeckSaveSubsystem.generated.h"

class UT66DeckFrontendStateSubsystem;
class UT66DeckRunSaveGame;

UCLASS()
class T66DECK_API UT66DeckSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	enum { MaxSlots = 3 };

	TArray<FT66DeckSaveSlotSummary> BuildRunSlotSummaries() const;
	UT66DeckRunSaveGame* LoadRunFromSlot(int32 SlotIndex) const;
	bool SaveRunToSlot(int32 SlotIndex, UT66DeckRunSaveGame* RunSave) const;
	bool DeleteRunFromSlot(int32 SlotIndex) const;
	UT66DeckRunSaveGame* CreateSeededRunSave(const UT66DeckFrontendStateSubsystem* FrontendState) const;
	UT66DeckRunSaveGame* CreateBlankRunSave() const;

private:
	static FString MakeRunSlotName(int32 SlotIndex);
	static FString BuildUtcNowString();
};
