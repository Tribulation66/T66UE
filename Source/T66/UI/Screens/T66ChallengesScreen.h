// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66GameInstance.h"
#include "UI/T66ScreenBase.h"
#include "T66ChallengesScreen.generated.h"

UCLASS(Blueprintable)
class T66_API UT66ChallengesScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66ChallengesScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	struct FChallengeEntry
	{
		FText Title;
		FText RewardSummary;
		FText Description;
		TArray<FText> Constraints;
		FText RewardDetail;
		FText CompletionPlaceholder;
		ET66RunModifierKind RunModifierKind = ET66RunModifierKind::None;
		FName RunModifierID = NAME_None;
	};

	const TArray<FChallengeEntry>& GetPlaceholderChallenges() const;
	const TArray<FChallengeEntry>& GetPlaceholderMods() const;
	const TArray<FChallengeEntry>& GetEntriesForTab(int32 TabIndex) const;
	int32 GetSelectedEntryIndex(int32 TabIndex);
	int32 GetConfirmedEntryIndex(int32 TabIndex) const;
	int32 GetActiveTabIndex();
	void InitializeSelectionState();

	FReply HandleBackClicked();
	FReply HandleTabSelected(int32 TabIndex);
	FReply HandleEntrySelected(int32 EntryIndex);
	FReply HandleConfirmClicked();

	struct FSelectionState
	{
		ET66RunModifierKind Kind = ET66RunModifierKind::None;
		FName ID = NAME_None;
	};

	bool bSelectionStateInitialized = false;
	int32 ActiveTabIndex = 0;
	FSelectionState PendingSelections[2];
};
