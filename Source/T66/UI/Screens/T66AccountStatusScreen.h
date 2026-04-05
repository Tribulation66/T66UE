// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Misc/Optional.h"
#include "Styling/SlateBrush.h"
#include "UI/T66ScreenBase.h"
#include "T66AccountStatusScreen.generated.h"

/** Simple account-status modal opened from the frontend top bar. */
UCLASS(Blueprintable)
class T66_API UT66AccountStatusScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66AccountStatusScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EAccountTab : uint8
	{
		Overview,
		History,
		Mains,
	};

	enum class EPBMode : uint8
	{
		Solo,
		Party,
	};

	enum class EHistoryCompletionFilter : uint8
	{
		All,
		Completed,
		NotCompleted,
	};

	EAccountTab ActiveTab = EAccountTab::Overview;
	EPBMode ActivePBMode = EPBMode::Solo;
	FName HistoryHeroFilter = NAME_None;
	TOptional<ET66Difficulty> HistoryDifficultyFilter;
	EHistoryCompletionFilter HistoryCompletionFilter = EHistoryCompletionFilter::All;
	TMap<FName, TSharedPtr<FSlateBrush>> HeroPortraitBrushes;

	FReply HandleBackClicked();
	FReply HandleOverviewTabClicked();
	FReply HandleHistoryTabClicked();
	FReply HandleMainsTabClicked();
	FReply HandleSoloPBModeClicked();
	FReply HandlePartyPBModeClicked();
	FReply HandleOpenRunSummaryClicked(FString SlotName);
	const FSlateBrush* GetOrCreateHeroPortraitBrush(class UT66GameInstance* T66GI, FName HeroID);
};

