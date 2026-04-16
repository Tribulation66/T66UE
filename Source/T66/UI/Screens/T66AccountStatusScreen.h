// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Misc/Optional.h"
#include "Styling/SlateBrush.h"
#include "UI/T66ScreenBase.h"
#include "T66AccountStatusScreen.generated.h"

class SMultiLineEditableTextBox;

/** Simple account-status modal opened from the frontend top bar. */
UCLASS(Blueprintable)
class T66_API UT66AccountStatusScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66AccountStatusScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EAccountTab : uint8
	{
		Suspension,
		Overview,
		History,
	};

	enum class EHistoryCompletionFilter : uint8
	{
		All,
		Completed,
		NotCompleted,
	};

	enum class EPersonalBestViewMode : uint8
	{
		PersonalBest,
		HighestRank,
	};

	EAccountTab ActiveTab = EAccountTab::Overview;
	ET66PartySize ActivePBPartySize = ET66PartySize::Solo;
	EPersonalBestViewMode ActivePBViewMode = EPersonalBestViewMode::PersonalBest;
	FName HistoryHeroFilter = NAME_None;
	TOptional<ET66Difficulty> HistoryDifficultyFilter;
	TOptional<ET66PartySize> HistoryPartySizeFilter;
	EHistoryCompletionFilter HistoryCompletionFilter = EHistoryCompletionFilter::All;
	TMap<FName, TSharedPtr<FSlateBrush>> HeroPortraitBrushes;
	FDelegateHandle BackendMyRankReadyHandle;
	TSharedPtr<SMultiLineEditableTextBox> AppealMessageTextBox;
	bool bAppealEditorOpen = false;
	bool bShowStandingInfoPopup = false;
	FString AppealDraftMessage;
	FString AppealSubmitStatusMessage;
	bool bAppealSubmitStatusIsError = false;

	FReply HandleBackClicked();
	FReply HandleSuspensionTabClicked();
	FReply HandleOverviewTabClicked();
	FReply HandleHistoryTabClicked();
	FReply HandleOpenAppealClicked();
	FReply HandleCancelAppealClicked();
	FReply HandleSubmitAppealClicked();
	FReply HandleStandingInfoClicked();
	FReply HandleOpenRunSummaryClicked(FString SlotName);
	void HandleBackendMyRankDataReady(const FString& Key, bool bSuccess, int32 Rank, int32 TotalEntries);
	UFUNCTION()
	void HandleBackendAppealSubmitComplete(bool bSuccess, const FString& Message);
	const FSlateBrush* GetOrCreateHeroPortraitBrush(class UT66GameInstance* T66GI, FName HeroID);
};

