// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/T66CommunityContentTypes.h"
#include "Styling/SlateBrush.h"
#include "UI/T66ScreenBase.h"
#include "T66ChallengesScreen.generated.h"

UCLASS(Blueprintable)
class T66_API UT66ChallengesScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66ChallengesScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	enum class EDraftStatField : uint8
	{
		Damage,
		AttackSpeed,
		AttackScale,
		Accuracy,
		Armor,
		Evasion,
		Luck,
		Speed,
	};

	class UT66CommunityContentSubsystem* GetCommunitySubsystem() const;
	ET66CommunityContentKind GetActiveKind() const;
	TArray<FT66CommunityContentEntry> GetEntriesForView(int32 TabIndex, int32 SourceTabIndex) const;
	bool FindSelectedEntryForView(int32 TabIndex, int32 SourceTabIndex, FT66CommunityContentEntry& OutEntry);
	bool FindCurrentSelectedEntry(FT66CommunityContentEntry& OutEntry);
	bool FindConfirmedEntry(FT66CommunityContentEntry& OutEntry) const;
	FName GetSelectedEntryIdForView(int32 TabIndex, int32 SourceTabIndex);
	FString GetOriginLabel(const FT66CommunityContentEntry& Entry) const;
	FString GetDraftSubmissionLabel(const FT66CommunityContentEntry& Entry) const;
	FString GetPassiveLabel(ET66PassiveType PassiveType) const;
	FString GetUltimateLabel(ET66UltimateType UltimateType) const;
	FString GetItemLabel(FName ItemId) const;
	TArray<FName> GetSelectableItemIds() const;
	const FSlateBrush* GetOrCreateAvatarBrush(const FString& AvatarUrl);
	void InitializeSelectionState();
	void BeginDraftEditor(const FT66CommunityContentEntry& DraftEntry);
	void EndDraftEditor();
	void CycleDraftPassive(int32 Direction);
	void CycleDraftUltimate(int32 Direction);
	void CycleDraftStartingItem(int32 Direction);
	void AdjustDraftStat(EDraftStatField Field, int32 Delta);

	FReply HandleBackClicked();
	FReply HandleTabSelected(int32 TabIndex);
	FReply HandleSourceTabSelected(int32 SourceTabIndex);
	FReply HandleEntrySelected(int32 EntryIndex);
	FReply HandleConfirmClicked();
	FReply HandleCreateDraftClicked();
	FReply HandleEditDraftClicked();
	FReply HandleDeleteDraftClicked();
	FReply HandleSaveDraftClicked();
	FReply HandlePlayDraftClicked();
	FReply HandleSubmitDraftClicked();
	FReply HandleCancelDraftEditorClicked();
	FReply HandleAdjustDraftReward(int32 Delta);
	FReply HandleAdjustDraftStartLevel(int32 Delta);
	FReply HandleAdjustDraftRequiredStage(int32 Delta);
	FReply HandleAdjustDraftMaxRunTime(int32 Delta);
	FReply HandleToggleDraftMaxStats();
	FReply HandleAdjustDraftStatClicked(EDraftStatField Field, int32 Delta);
	FReply HandleCycleDraftPassiveClicked(int32 Direction);
	FReply HandleCycleDraftUltimateClicked(int32 Direction);
	FReply HandleCycleDraftStartingItemClicked(int32 Direction);
	void HandleDraftTitleChanged(const FText& NewText);
	void HandleDraftDescriptionChanged(const FText& NewText);
	void HandleDraftFullClearChanged(ECheckBoxState NewState);
	void HandleDraftNoDamageChanged(ECheckBoxState NewState);
	void HandleCommunityContentChanged();

	bool bSelectionStateInitialized = false;
	bool bRequestedCommunityRefresh = false;
	bool bCommunityDelegateBound = false;
	bool bDraftEditorActive = false;
	int32 ActiveTabIndex = 0;
	int32 ActiveSourceTabIndex[2] = { 0, 0 };
	FName PendingSelections[2][2];
	FT66CommunityContentEntry DraftEditorEntry;
	TMap<FString, TSharedPtr<FSlateBrush>> AvatarBrushes;
	TSharedPtr<FSlateBrush> DefaultAvatarBrush;
};
