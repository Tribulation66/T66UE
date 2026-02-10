// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66PlayerSummaryPickerScreen.generated.h"

class UT66LeaderboardRunSummarySaveGame;

/**
 * Modal shown when clicking a duo/trio leaderboard row: "Pick the Player".
 * Black background, 2 or 3 options (name, hero icon, Select). Choosing one opens Run Summary for that player.
 */
UCLASS(Blueprintable)
class T66_API UT66PlayerSummaryPickerScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66PlayerSummaryPickerScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleSelectClicked(int32 Index);

	TArray<TSharedPtr<struct FSlateBrush>> HeroBrushes;
};
