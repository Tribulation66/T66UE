// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/T66DataTypes.h"
#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;
class SVerticalBox;
class UT66BackendSubsystem;
class UT66LeaderboardSubsystem;
class UT66LocalizationSubsystem;
class UT66PlayerSettingsSubsystem;
class UT66SteamHelper;
class UT66UIManager;
class UT66WebImageCache;
class UTexture2D;

struct FSlateBrush;
struct FT66FavoriteLeaderboardRun;

/**
 * Functional flat Main Menu leaderboard.
 *
 * This keeps the active Stage 2 / FT66FlatStyle chrome while preserving the
 * backend-driven behavior from the old reference leaderboard path.
 */
class T66_API ST66FlatLeaderboardPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST66FlatLeaderboardPanel) {}
		SLATE_ARGUMENT(UT66LocalizationSubsystem*, LocalizationSubsystem)
		SLATE_ARGUMENT(UT66LeaderboardSubsystem*, LeaderboardSubsystem)
		SLATE_ARGUMENT(UT66UIManager*, UIManager)
		SLATE_ARGUMENT(FString, TagPrefix)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~ST66FlatLeaderboardPanel() override;

	void SetUIManager(UT66UIManager* InUIManager);

	static constexpr float GetContentWidth() { return 460.0f; }
	static constexpr float GetPanelContentInset() { return 30.0f; }
	static constexpr float GetPanelWidth() { return GetContentWidth() + (GetPanelContentInset() * 2.0f); }
	static constexpr float GetPanelHeight() { return 950.0f; }

private:
	UT66BackendSubsystem* GetBackendSubsystem() const;
	UT66PlayerSettingsSubsystem* GetPlayerSettingsSubsystem() const;
	UT66SteamHelper* GetSteamHelper() const;
	UT66WebImageCache* GetWebImageCache() const;
	UGameInstance* GetGameInstance() const;

	TSharedRef<SWidget> BuildPanel();
	TSharedRef<SWidget> BuildContentPanel();
	TSharedRef<SWidget> BuildRowsPanel();
	TSharedRef<SWidget> BuildFilterButton(ET66LeaderboardFilter Filter, const FText& Label, const FString& Name);
	TSharedRef<SWidget> BuildTypeButton(ET66LeaderboardType Type, const FText& Label, const FString& Name);
	TSharedRef<SWidget> BuildTimeButton(ET66LeaderboardTime TimeFilter, const FText& Label, const FString& Name);
	TSharedRef<SWidget> BuildTimeDropdown();
	TSharedRef<SWidget> BuildRuleDropdown();
	TSharedRef<SWidget> BuildPartySizeDropdown();
	TSharedRef<SWidget> BuildDifficultyDropdown();
	TSharedRef<SWidget> BuildMetricCheckButton(ET66LeaderboardType Type, const FText& Label, const FString& Name);
	TSharedRef<SWidget> BuildTimeMenu();
	TSharedRef<SWidget> BuildRuleMenu();
	TSharedRef<SWidget> BuildPartySizeMenu();
	TSharedRef<SWidget> BuildDifficultyMenu();
	TSharedRef<SWidget> BuildStreamerRequestPanel();
	TSharedRef<SWidget> BuildLeaderboardRow(const FLeaderboardEntry& Entry, int32 DisplayIndex, bool bLocalRow);
	TSharedRef<SWidget> BuildMenuSectionLabel(const FString& Label, const FString& TagName) const;
	TSharedRef<SWidget> BuildMenuOption(const FText& Label, FOnClicked OnClicked, bool bSelected, const FString& TagName) const;

	void RefreshLeaderboard();
	void RebuildEntryList();
	void RebuildPanelAndRequestPaint();
	void RequestFrontendPaintRefresh() const;
	FReply SetFilter(ET66LeaderboardFilter NewFilter);
	FReply SetTimeFilter(ET66LeaderboardTime NewTimeFilter);
	FReply SetPartySize(ET66PartySize NewPartySize);
	FReply SetDifficulty(ET66Difficulty NewDifficulty);
	FReply SetLeaderboardType(ET66LeaderboardType NewType);
	FReply SetStreamerRequestOpen(bool bOpen);
	FReply SubmitStreamerRequest();

	void OnBackendLeaderboardReady(const FString& LeaderboardKey);
	void OnBackendMyRankReady(const FString& RankKey, bool bSuccess, int32 Rank, int32 TotalEntries);
	void OnBackendRunSummaryReady(const FString& EntryId);
	void OnStreamerRequestComplete(bool bSuccess, const FString& Message);
	void BindLeaderboardDelegate(UT66BackendSubsystem* Backend);
	void BindMyRankDelegate(UT66BackendSubsystem* Backend);
	void BindRunSummaryDelegate(UT66BackendSubsystem* Backend);

	FReply HandleEntryClicked(const FLeaderboardEntry& Entry);
	FReply HandleLocalEntryClicked(const FLeaderboardEntry& Entry);
	FReply ToggleFavoriteEntry(const FLeaderboardEntry& Entry);

	void NormalizeEntries();
	void NormalizeEntryIdentity(FLeaderboardEntry& Entry, int32 EntryIndex);
	void ApplyDisplayLimit();
	void RefreshDedicatedLocalEntry(UT66BackendSubsystem* Backend);
	void UpdateDedicatedLocalEntryFromCache(UT66BackendSubsystem* Backend, const FString& RankKey);
	FLeaderboardEntry MakeDedicatedLocalEntryPlaceholder() const;

	FName Tag(const FString& Leaf) const;
	FString MakeLeaderboardKey() const;
	FString MakeMyRankKey() const;
	FString CurrentMyRankFilterContext() const;
	FString CurrentBackendType() const;
	FString CurrentBackendTime() const;
	FString CurrentBackendParty() const;
	FString CurrentBackendDifficulty() const;
	FString CurrentBackendFilter() const;
	bool IsDifficultyPlayable(ET66Difficulty Difficulty) const;
	FText GetHeaderText() const;
	FText GetTimeDropdownText() const;
	FText GetRuleDropdownText() const;
	FText GetMetricHeaderText() const;
	FText GetEntryMetricText(const FLeaderboardEntry& Entry) const;
	FText GetRankText(const FLeaderboardEntry& Entry, bool bLocalRow) const;
	bool HasEntryMetricValue(const FLeaderboardEntry& Entry, bool bLocalRow) const;
	FString FormatTime(float Seconds) const;
	FString ResolveEntryDisplayName(const FLeaderboardEntry& Entry) const;
	FString ResolveEntryMemberDisplayName(const FLeaderboardEntry& Entry, int32 MemberIndex) const;
	FString ResolveSteamDisplayName(const FString& SteamId) const;
	FText PartySizeText(ET66PartySize PartySize) const;
	FText DifficultyText(ET66Difficulty Difficulty) const;
	FText TimeFilterText(ET66LeaderboardTime TimeFilter) const;
	FText TypeText(ET66LeaderboardType Type) const;
	bool IsEntryLocalPlayer(const FLeaderboardEntry& Entry) const;
	bool CanOpenRunSummary(const FLeaderboardEntry& Entry) const;
	bool IsEntryFavoritable(const FLeaderboardEntry& Entry) const;
	bool IsEntryFavorited(const FLeaderboardEntry& Entry) const;
	FT66FavoriteLeaderboardRun MakeFavoriteRunFromEntry(const FLeaderboardEntry& Entry) const;

	const FSlateBrush* GetPortraitBrushForEntry(const FLeaderboardEntry& Entry);
	const FSlateBrush* GetPortraitBrushForEntryMember(const FLeaderboardEntry& Entry, int32 MemberIndex);
	const FSlateBrush* GetOrCreateSteamAvatarBrush(const FString& SteamId);
	const FSlateBrush* GetOrCreateAvatarBrush(const FString& AvatarUrl);
	const FSlateBrush* GetOrCreateHeroPortraitBrush(FName HeroID);
	const FSlateBrush* GetFilterIconBrush(ET66LeaderboardFilter Filter);
	void SetBrushTexture(const TSharedPtr<FSlateBrush>& Brush, UTexture2D* Texture);
	void TrackBrushTexture(UTexture2D* Texture);
	void ReleaseRootedBrushTextures();

	TWeakObjectPtr<UT66LocalizationSubsystem> LocalizationSubsystem;
	TWeakObjectPtr<UT66LeaderboardSubsystem> LeaderboardSubsystem;
	TWeakObjectPtr<UT66UIManager> UIManager;
	TWeakObjectPtr<UT66BackendSubsystem> BoundBackendSubsystem;

	FString TagPrefix = TEXT("MainMenu.Right");
	ET66LeaderboardFilter CurrentFilter = ET66LeaderboardFilter::Global;
	ET66LeaderboardTime CurrentTimeFilter = ET66LeaderboardTime::Current;
	ET66PartySize CurrentPartySize = ET66PartySize::Solo;
	ET66Difficulty CurrentDifficulty = ET66Difficulty::Easy;
	ET66LeaderboardType CurrentType = ET66LeaderboardType::Score;

	bool bBoundToLeaderboardDelegate = false;
	bool bBoundToMyRankDelegate = false;
	bool bBoundToRunSummaryDelegate = false;
	bool bStreamerRequestOpen = false;
	bool bStreamerRequestSubmitting = false;
	bool bIsLoading = false;
	FString PendingRunSummaryEntryId;
	FText StatusText;
	FText StreamerRequestStatusText;
	FString StreamerCreatorLinkText;
	FString StreamerSteamIdText;
	FString PendingMyRankKey;

	TArray<FLeaderboardEntry> LeaderboardEntries;
	FLeaderboardEntry DedicatedLocalEntry;
	TSharedPtr<SVerticalBox> EntryListBox;
	TSharedPtr<SEditableTextBox> StreamerCreatorLinkTextBox;
	TSharedPtr<SEditableTextBox> StreamerSteamIdTextBox;

	TSharedPtr<FSlateBrush> DefaultAvatarBrush;
	TMap<FString, TSharedPtr<FSlateBrush>> AvatarBrushes;
	TMap<FString, TSharedPtr<FSlateBrush>> SteamAvatarBrushes;
	TMap<FName, TSharedPtr<FSlateBrush>> HeroPortraitBrushes;
	TMap<ET66LeaderboardFilter, TSharedPtr<FSlateBrush>> FilterIconBrushes;
	TArray<TWeakObjectPtr<UTexture2D>> RootedBrushTextures;
};
