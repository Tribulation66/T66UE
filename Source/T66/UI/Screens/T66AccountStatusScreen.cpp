// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AccountStatusScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66BuffSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Data/T66DataTypes.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 AccountFontDelta = -4;

	int32 AdjustAccountFontSize(int32 BaseSize)
	{
		return FMath::Max(6, BaseSize + AccountFontDelta);
	}

	FSlateFontInfo AccountBoldFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontBold(AdjustAccountFontSize(BaseSize));
	}

	FSlateFontInfo AccountRegularFont(int32 BaseSize)
	{
		return FT66Style::Tokens::FontRegular(AdjustAccountFontSize(BaseSize));
	}

	FLinearColor AccountGold()
	{
		return FLinearColor(0.83f, 0.68f, 0.34f, 1.0f);
	}

	FLinearColor AccountMutedGold()
	{
		return FLinearColor(0.55f, 0.46f, 0.26f, 1.0f);
	}

	FLinearColor AccountPanelFill()
	{
		return FLinearColor(0.05f, 0.06f, 0.08f, 0.94f);
	}

	FLinearColor AccountPanelInnerFill()
	{
		return FLinearColor(0.08f, 0.09f, 0.11f, 0.96f);
	}

	FLinearColor AccountRowFill()
	{
		return FLinearColor(0.10f, 0.10f, 0.12f, 0.92f);
	}

	FLinearColor AccountRowAltFill()
	{
		return FLinearColor(0.12f, 0.12f, 0.14f, 0.92f);
	}

	FLinearColor AccountHeaderFill()
	{
		return FLinearColor(0.14f, 0.09f, 0.05f, 0.96f);
	}

	FLinearColor AccountText()
	{
		return FLinearColor(0.94f, 0.92f, 0.86f, 1.0f);
	}

	FLinearColor AccountMutedText()
	{
		return FLinearColor(0.67f, 0.67f, 0.65f, 1.0f);
	}

	FLinearColor AccountSuccess()
	{
		return FLinearColor(0.35f, 0.82f, 0.45f, 1.0f);
	}

	FLinearColor AccountDanger()
	{
		return FLinearColor(0.86f, 0.27f, 0.20f, 1.0f);
	}

	FText RestrictionText(ET66AccountRestrictionKind Restriction)
	{
		switch (Restriction)
		{
		case ET66AccountRestrictionKind::Suspicion:
			return NSLOCTEXT("T66.Account", "RestrictionSuspicion", "Suspended");
		case ET66AccountRestrictionKind::CheatingCertainty:
			return NSLOCTEXT("T66.Account", "RestrictionCheating", "Restricted");
		case ET66AccountRestrictionKind::None:
		default:
			return NSLOCTEXT("T66.Account", "RestrictionGood", "Good Standing");
		}
	}

	FText AppealStatusText(ET66AppealReviewStatus Status)
	{
		switch (Status)
		{
		case ET66AppealReviewStatus::UnderReview:
			return NSLOCTEXT("T66.Account", "AppealUnderReview", "Appeal: Under Review");
		case ET66AppealReviewStatus::Denied:
			return NSLOCTEXT("T66.Account", "AppealDenied", "Appeal: Denied");
		case ET66AppealReviewStatus::Approved:
			return NSLOCTEXT("T66.Account", "AppealApproved", "Appeal: Approved");
		case ET66AppealReviewStatus::NotSubmitted:
		default:
			return NSLOCTEXT("T66.Account", "AppealNotSubmitted", "Appeal: Not Submitted");
		}
	}

	FText PartySizeText(UT66LocalizationSubsystem* Loc, ET66PartySize PartySize)
	{
		if (!Loc)
		{
			switch (PartySize)
			{
			case ET66PartySize::Duo:
				return NSLOCTEXT("T66.Account", "PartyDuoFallback", "Duo");
			case ET66PartySize::Trio:
				return NSLOCTEXT("T66.Account", "PartyTrioFallback", "Trio");
			case ET66PartySize::Quad:
				return NSLOCTEXT("T66.Account", "PartyQuadFallback", "Quad");
			case ET66PartySize::Solo:
			default:
				return NSLOCTEXT("T66.Account", "PartySoloFallback", "Solo");
			}
		}

		switch (PartySize)
		{
		case ET66PartySize::Duo:
			return Loc->GetText_Duo();
		case ET66PartySize::Trio:
			return Loc->GetText_Trio();
		case ET66PartySize::Quad:
			return Loc->GetText_Quad();
		case ET66PartySize::Solo:
		default:
			return Loc->GetText_Solo();
		}
	}

	FText FormatDurationText(float TotalSeconds)
	{
		const int32 RoundedSeconds = FMath::Max(0, FMath::RoundToInt(TotalSeconds));
		const int32 Hours = RoundedSeconds / 3600;
		const int32 Minutes = (RoundedSeconds % 3600) / 60;
		const int32 Seconds = RoundedSeconds % 60;

		if (Hours > 0)
		{
			return FText::FromString(FString::Printf(TEXT("%d:%02d:%02d"), Hours, Minutes, Seconds));
		}

		return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
	}

	FString DifficultyToApiString(ET66Difficulty Difficulty)
	{
		switch (Difficulty)
		{
		case ET66Difficulty::Medium: return TEXT("medium");
		case ET66Difficulty::Hard: return TEXT("hard");
		case ET66Difficulty::VeryHard: return TEXT("veryhard");
		case ET66Difficulty::Impossible: return TEXT("impossible");
		case ET66Difficulty::Easy:
		default:
			return TEXT("easy");
		}
	}

	FString PartySizeToApiString(ET66PartySize PartySize)
	{
		switch (PartySize)
		{
		case ET66PartySize::Duo: return TEXT("duo");
		case ET66PartySize::Trio: return TEXT("trio");
		case ET66PartySize::Quad: return TEXT("quad");
		case ET66PartySize::Solo:
		default:
			return TEXT("solo");
		}
	}

	TSharedRef<SWidget> MakeAccountPanel(const TSharedRef<SWidget>& Content, ET66PanelType Type, const FLinearColor& Color, const FMargin& Padding)
	{
		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(Color)
				.SetPadding(Padding));
	}

	TSharedRef<SWidget> MakeSectionHeader(const FText& Text)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(AccountBoldFont(18))
			.ColorAndOpacity(AccountGold());
	}

	struct FPersonalBestDisplay
	{
		bool bHasRecord = false;
		bool bHasRankState = false;
		bool bRankRequestSucceeded = false;
		ET66PartySize PartySize = ET66PartySize::Solo;
		int32 Score = 0;
		float Seconds = 0.f;
		int32 GlobalRank = 0;
		FString RunSummarySlotName;
		FDateTime AchievedAtUtc;
	};

	struct FAccountNamedEntry
	{
		FName ID = NAME_None;
		FText DisplayName;
		FString SortKey;
	};
}

UT66AccountStatusScreen::UT66AccountStatusScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::AccountStatus;
	bIsModal = false;
}

void UT66AccountStatusScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	bAppealEditorOpen = false;
	bShowStandingInfoPopup = false;
	AppealDraftMessage.Reset();
	AppealSubmitStatusMessage.Reset();
	bAppealSubmitStatusIsError = false;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			if (BackendMyRankReadyHandle.IsValid())
			{
				Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
				BackendMyRankReadyHandle.Reset();
			}
			BackendMyRankReadyHandle = Backend->OnMyRankDataReady.AddUObject(this, &UT66AccountStatusScreen::HandleBackendMyRankDataReady);
			Backend->OnAppealSubmitComplete.RemoveDynamic(this, &UT66AccountStatusScreen::HandleBackendAppealSubmitComplete);
			Backend->OnAppealSubmitComplete.AddDynamic(this, &UT66AccountStatusScreen::HandleBackendAppealSubmitComplete);
		}
	}
}

void UT66AccountStatusScreen::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnMyRankDataReady.Remove(BackendMyRankReadyHandle);
			BackendMyRankReadyHandle.Reset();
			Backend->OnAppealSubmitComplete.RemoveDynamic(this, &UT66AccountStatusScreen::HandleBackendAppealSubmitComplete);
		}
	}

	Super::OnScreenDeactivated_Implementation();
}

void UT66AccountStatusScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	ForceRebuildSlate();
}

void UT66AccountStatusScreen::HandleBackendMyRankDataReady(const FString& Key, bool bSuccess, int32 Rank, int32 TotalEntries)
{
	static_cast<void>(Key);
	static_cast<void>(bSuccess);
	static_cast<void>(Rank);
	static_cast<void>(TotalEntries);

	if (!HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66AccountStatusScreen::BuildSlateUI()
{
	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GIBase);
	UT66LocalizationSubsystem* Loc = GIBase ? GIBase->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GIBase ? GIBase->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66BackendSubsystem* Backend = GIBase ? GIBase->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Achievements = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66BuffSubsystem* Buffs = GIBase ? GIBase->GetSubsystem<UT66BuffSubsystem>() : nullptr;
	UT66CompanionUnlockSubsystem* CompanionUnlocks = GIBase ? GIBase->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr;
	const TWeakObjectPtr<UT66GameInstance> WeakT66GI = T66GI;
	const TWeakObjectPtr<UT66LocalizationSubsystem> WeakLoc = Loc;
	const TWeakObjectPtr<UT66LeaderboardSubsystem> WeakLB = LB;

	const bool bModalPresentation = UIManager && UIManager->GetCurrentModalType() == ScreenType;
	const float ResponsiveScale = FMath::Max(FT66Style::GetViewportResponsiveScale(), KINDA_SMALL_NUMBER);
	const float TopBarOverlapPx = 22.f;
	// Frontend screens are inside the responsive root, but the top bar is not.
	// Convert the visible top-bar height back into screen-local units, then apply a small overlap
	// so the screen surface meets the bar with no seam.
	const float TopInset = bModalPresentation
		? 0.f
		: FMath::Max(0.f, ((UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f) - TopBarOverlapPx) / ResponsiveScale);
	const FT66AccountRestrictionRecord Restriction = LB ? LB->GetAccountRestrictionRecord() : FT66AccountRestrictionRecord();
	const bool bAccountEligible = LB ? LB->IsAccountEligibleForLeaderboard() : true;
	const bool bHasSuspension = Restriction.Restriction != ET66AccountRestrictionKind::None;
	if (!bHasSuspension && ActiveTab == EAccountTab::Suspension)
	{
		ActiveTab = EAccountTab::Overview;
	}
	const TArray<FName> HeroIDs = T66GI ? T66GI->GetAllHeroIDs() : TArray<FName>();
	const TArray<FName> CompanionIDs = T66GI ? T66GI->GetAllCompanionIDs() : TArray<FName>();
	const TArray<FAchievementData> AchievementDefs = Achievements ? Achievements->GetAllAchievements() : TArray<FAchievementData>();
	const TArray<FT66RecentRunRecord> RecentRuns = LB ? LB->GetRecentRuns() : TArray<FT66RecentRunRecord>();

	int32 UnlockedAchievements = 0;
	for (const FAchievementData& A : AchievementDefs) if (A.bIsUnlocked) { ++UnlockedAchievements; }

	const TArray<ET66HeroStatType> PowerStats = {
		ET66HeroStatType::Damage, ET66HeroStatType::AttackSpeed, ET66HeroStatType::AttackScale, ET66HeroStatType::Accuracy,
		ET66HeroStatType::Armor, ET66HeroStatType::Evasion, ET66HeroStatType::Luck
	};
	int32 UnlockedPowerUps = 0;
	for (const ET66HeroStatType StatType : PowerStats) { UnlockedPowerUps += Buffs ? Buffs->GetUnlockedFillStepCount(StatType) : 0; }

	int32 UnlockedCompanions = 0;
	for (const FName& CompanionID : CompanionIDs)
	{
		if (!CompanionUnlocks || CompanionUnlocks->IsCompanionUnlocked(CompanionID))
		{
			++UnlockedCompanions;
		}
	}

	const int32 LifetimeStagesCleared = Achievements ? Achievements->GetLifetimeStagesCleared() : 0;
	int32 TotalStageCount = 23;
	if (UDataTable* StagesTable = T66GI ? T66GI->GetStagesDataTable() : nullptr)
	{
		TotalStageCount = FMath::Max(StagesTable->GetRowMap().Num(), 1);
	}
	const int32 DisplayStagesCleared = FMath::Clamp(LifetimeStagesCleared, 0, TotalStageCount);

	auto DifficultyText = [Loc](ET66Difficulty Difficulty) -> FText
	{
		if (Loc) return Loc->GetText_Difficulty(Difficulty);
		switch (Difficulty)
		{
		case ET66Difficulty::Easy: return NSLOCTEXT("T66.Account", "Easy", "Easy");
		case ET66Difficulty::Medium: return NSLOCTEXT("T66.Account", "Medium", "Medium");
		case ET66Difficulty::Hard: return NSLOCTEXT("T66.Account", "Hard", "Hard");
		case ET66Difficulty::VeryHard: return NSLOCTEXT("T66.Account", "VeryHard", "Very Hard");
		case ET66Difficulty::Impossible: default: return NSLOCTEXT("T66.Account", "Impossible", "Impossible");
		}
	};

	auto CompletionFilterText = [](EHistoryCompletionFilter Filter) -> FText
	{
		switch (Filter)
		{
		case EHistoryCompletionFilter::Completed:
			return NSLOCTEXT("T66.Account", "FilterCompleted", "Completed");
		case EHistoryCompletionFilter::NotCompleted:
			return NSLOCTEXT("T66.Account", "FilterNotCompleted", "Not Completed");
		case EHistoryCompletionFilter::All:
		default:
			return NSLOCTEXT("T66.Account", "FilterAllRuns", "All Runs");
		}
	};

	TArray<FAccountNamedEntry> HistoryHeroFilterEntries;
	for (const FName& HeroID : HeroIDs)
	{
		FHeroData HeroData;
		if (!T66GI || !T66GI->GetHeroData(HeroID, HeroData))
		{
			continue;
		}

		FAccountNamedEntry Entry;
		Entry.ID = HeroID;
		Entry.DisplayName = Loc ? Loc->GetText_HeroName(HeroID) : HeroData.DisplayName;
		Entry.SortKey = Entry.DisplayName.ToString();
		HistoryHeroFilterEntries.Add(Entry);
	}
	HistoryHeroFilterEntries.Sort([](const FAccountNamedEntry& A, const FAccountNamedEntry& B)
	{
		return A.SortKey < B.SortKey;
	});

	auto MakeTabButton = [this](
		const FText& Label,
		bool bActive,
		FReply (UT66AccountStatusScreen::*Handler)(),
		const FLinearColor& ActiveColor = FLinearColor::Transparent,
		const FLinearColor& InactiveColor = FLinearColor::Transparent,
		const FLinearColor& ActiveTextColor = FLinearColor::Transparent,
		const FLinearColor& InactiveTextColor = FLinearColor::Transparent) -> TSharedRef<SWidget>
	{
		const FLinearColor ResolvedActiveColor = ActiveColor == FLinearColor::Transparent ? AccountGold() : ActiveColor;
		const FLinearColor ResolvedInactiveColor = InactiveColor == FLinearColor::Transparent ? AccountPanelInnerFill() : InactiveColor;
		const FLinearColor ResolvedActiveTextColor = ActiveTextColor == FLinearColor::Transparent ? FLinearColor(0.10f, 0.08f, 0.05f, 1.0f) : ActiveTextColor;
		const FLinearColor ResolvedInactiveTextColor = InactiveTextColor == FLinearColor::Transparent ? AccountText() : InactiveTextColor;

		return FT66Style::MakeButton(
			FT66ButtonParams(Label, FOnClicked::CreateUObject(this, Handler), bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetFontSize(AdjustAccountFontSize(12))
			.SetMinWidth(152.f)
			.SetHeight(0.f)
			.SetPadding(FMargin(16.f, 10.f, 16.f, 8.f))
			.SetColor(bActive ? ResolvedActiveColor : ResolvedInactiveColor)
			.SetTextColor(bActive ? ResolvedActiveTextColor : ResolvedInactiveTextColor)
			.SetUseGlow(false));
	};

	auto MakeProgressRow = [&](const FText& Label, int32 Current, int32 Total, const FLinearColor& Fill) -> TSharedRef<SWidget>
	{
		const float Pct = Total > 0 ? static_cast<float>(Current) / static_cast<float>(Total) : 0.f;
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)[SNew(STextBlock).Text(Label).Font(AccountBoldFont(12)).ColorAndOpacity(AccountText())]
				+ SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text(FText::Format(NSLOCTEXT("T66.Account", "CountFmt", "{0}/{1}"), FText::AsNumber(Current), FText::AsNumber(Total))).Font(AccountBoldFont(12)).ColorAndOpacity(AccountGold())]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
			[
				SNew(SProgressBar).Percent(Pct).FillColorAndOpacity(Fill)
			],
			ET66PanelType::Panel2, AccountRowFill(), FMargin(10.f, 8.f));
	};

	auto GetMyRankKey = [this](ET66LeaderboardType Type, ET66Difficulty Difficulty) -> FString
	{
		return UT66BackendSubsystem::MakeMyRankCacheKey(
			Type == ET66LeaderboardType::Score ? TEXT("score") : TEXT("speedrun"),
			TEXT("alltime"),
			PartySizeToApiString(ActivePBPartySize),
			DifficultyToApiString(Difficulty));
	};

	auto PrimeMyRankRequest = [this, Backend, &GetMyRankKey](ET66LeaderboardType Type, ET66Difficulty Difficulty)
	{
		if (!Backend || !Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
		{
			return;
		}

		const FString RankKey = GetMyRankKey(Type, Difficulty);
		if (!Backend->HasCachedMyRank(RankKey))
		{
			Backend->FetchMyRank(
				Type == ET66LeaderboardType::Score ? TEXT("score") : TEXT("speedrun"),
				TEXT("alltime"),
				PartySizeToApiString(ActivePBPartySize),
				DifficultyToApiString(Difficulty));
		}
	};

	auto ResolveMyRank = [Backend, &GetMyRankKey](ET66LeaderboardType Type, ET66Difficulty Difficulty, bool& bOutHasRankState, bool& bOutRankSuccess, int32& OutRank) -> void
	{
		bOutHasRankState = false;
		bOutRankSuccess = false;
		OutRank = 0;

		if (!Backend)
		{
			return;
		}

		const FString RankKey = GetMyRankKey(Type, Difficulty);
		int32 TotalEntries = 0;
		bOutHasRankState = Backend->GetCachedMyRank(RankKey, bOutRankSuccess, OutRank, TotalEntries);
	};

	auto MakePBScore = [&](ET66Difficulty Difficulty) -> FPersonalBestDisplay
	{
		FPersonalBestDisplay Out;
		if (!LB) return Out;
		FT66LocalScoreRecord R;
		if (LB->GetLocalBestScoreRecord(Difficulty, ActivePBPartySize, R))
		{
			Out.bHasRecord = true;
			Out.PartySize = ActivePBPartySize;
			if (ActivePBViewMode == EPersonalBestViewMode::HighestRank && R.BestRankAllTime > 0)
			{
				Out.Score = R.BestRankScore;
				Out.RunSummarySlotName = R.BestRankRunSummarySlotName;
				Out.AchievedAtUtc = R.BestRankAchievedAtUtc;
				Out.bHasRankState = true;
				Out.bRankRequestSucceeded = true;
				Out.GlobalRank = R.BestRankAllTime;
			}
			else
			{
				Out.Score = R.BestScore;
				Out.RunSummarySlotName = R.RunSummarySlotName;
				Out.AchievedAtUtc = R.AchievedAtUtc;
				if (ActivePBViewMode == EPersonalBestViewMode::PersonalBest && R.BestScoreRankAllTime > 0)
				{
					Out.bHasRankState = true;
					Out.bRankRequestSucceeded = true;
					Out.GlobalRank = R.BestScoreRankAllTime;
				}
				else
				{
					PrimeMyRankRequest(ET66LeaderboardType::Score, Difficulty);
					bool bHasRankState = false;
					bool bRankSuccess = false;
					int32 Rank = 0;
					ResolveMyRank(ET66LeaderboardType::Score, Difficulty, bHasRankState, bRankSuccess, Rank);
					Out.bHasRankState = bHasRankState;
					Out.bRankRequestSucceeded = bRankSuccess;
					Out.GlobalRank = (bHasRankState && bRankSuccess) ? Rank : 0;
				}
			}
		}
		return Out;
	};

	auto MakePBTime = [&](ET66Difficulty Difficulty) -> FPersonalBestDisplay
	{
		FPersonalBestDisplay Out;
		if (!LB) return Out;
		FT66LocalCompletedRunTimeRecord R;
		if (LB->GetLocalBestCompletedRunTimeRecord(Difficulty, ActivePBPartySize, R))
		{
			Out.bHasRecord = true;
			Out.PartySize = ActivePBPartySize;
			if (ActivePBViewMode == EPersonalBestViewMode::HighestRank && R.BestRankAllTime > 0)
			{
				Out.Seconds = R.BestRankCompletedSeconds;
				Out.RunSummarySlotName = R.BestRankRunSummarySlotName;
				Out.AchievedAtUtc = R.BestRankAchievedAtUtc;
				Out.bHasRankState = true;
				Out.bRankRequestSucceeded = true;
				Out.GlobalRank = R.BestRankAllTime;
			}
			else
			{
				Out.Seconds = R.BestCompletedSeconds;
				Out.RunSummarySlotName = R.RunSummarySlotName;
				Out.AchievedAtUtc = R.AchievedAtUtc;
				if (ActivePBViewMode == EPersonalBestViewMode::PersonalBest && R.BestCompletedRankAllTime > 0)
				{
					Out.bHasRankState = true;
					Out.bRankRequestSucceeded = true;
					Out.GlobalRank = R.BestCompletedRankAllTime;
				}
				else
				{
					PrimeMyRankRequest(ET66LeaderboardType::SpeedRun, Difficulty);
					bool bHasRankState = false;
					bool bRankSuccess = false;
					int32 Rank = 0;
					ResolveMyRank(ET66LeaderboardType::SpeedRun, Difficulty, bHasRankState, bRankSuccess, Rank);
					Out.bHasRankState = bHasRankState;
					Out.bRankRequestSucceeded = bRankSuccess;
					Out.GlobalRank = (bHasRankState && bRankSuccess) ? Rank : 0;
				}
			}
		}
		return Out;
	};

	TMap<FString, FName> PBHeroIdBySlot;
	auto ResolvePBHeroID = [&PBHeroIdBySlot](const FString& SlotName) -> FName
	{
		if (SlotName.IsEmpty())
		{
			return NAME_None;
		}

		if (const FName* CachedHeroID = PBHeroIdBySlot.Find(SlotName))
		{
			return *CachedHeroID;
		}

		FName HeroID = NAME_None;
		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			if (USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SlotName, 0))
			{
				if (const UT66LeaderboardRunSummarySaveGame* Summary = Cast<UT66LeaderboardRunSummarySaveGame>(SaveGame))
				{
					HeroID = Summary->HeroID;
				}
			}
		}

		PBHeroIdBySlot.Add(SlotName, HeroID);
		return HeroID;
	};

	auto HeroText = [T66GI, Loc](FName HeroID) -> FText
	{
		if (HeroID.IsNone())
		{
			return FText::GetEmpty();
		}

		FHeroData HeroData;
		if (T66GI && T66GI->GetHeroData(HeroID, HeroData))
		{
			return Loc ? Loc->GetText_HeroName(HeroID) : HeroData.DisplayName;
		}

		return FText::FromName(HeroID);
	};

	auto GetHeroHistoryFilterText = [this, WeakT66GI, WeakLoc]() -> FText
	{
		if (HistoryHeroFilter.IsNone())
		{
			return NSLOCTEXT("T66.Account", "AllHeroes", "All Heroes");
		}

		UT66GameInstance* RuntimeGI = WeakT66GI.Get();
		UT66LocalizationSubsystem* RuntimeLoc = WeakLoc.Get();
		FHeroData HeroData;
		if (RuntimeGI && RuntimeGI->GetHeroData(HistoryHeroFilter, HeroData))
		{
			return RuntimeLoc ? RuntimeLoc->GetText_HeroName(HistoryHeroFilter) : HeroData.DisplayName;
		}

		return FText::FromName(HistoryHeroFilter);
	};

	auto GetDifficultyHistoryFilterText = [this, DifficultyText]() -> FText
	{
		return HistoryDifficultyFilter.IsSet()
			? DifficultyText(HistoryDifficultyFilter.GetValue())
			: NSLOCTEXT("T66.Account", "AllDifficulties", "All Difficulties");
	};

	auto GetPartySizeHistoryFilterText = [this, Loc]() -> FText
	{
		return HistoryPartySizeFilter.IsSet()
			? PartySizeText(Loc, HistoryPartySizeFilter.GetValue())
			: NSLOCTEXT("T66.Account", "AllPartySizes", "All Party Sizes");
	};

	auto GetCompletionHistoryFilterText = [this, CompletionFilterText]() -> FText
	{
		return CompletionFilterText(HistoryCompletionFilter);
	};

	auto MakeHistoryFilterMenuEntry = [this](const FText& Label, bool bActive, TFunction<void()> OnSelected) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(
			FT66ButtonParams(Label, FOnClicked::CreateLambda([this, OnSelected]() { OnSelected(); ForceRebuildSlate(); return FReply::Handled(); }), bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetFontSize(AdjustAccountFontSize(11))
			.SetMinWidth(0.f)
			.SetHeight(28.f)
			.SetPadding(FMargin(10.f, 5.f, 10.f, 4.f))
			.SetColor(bActive ? AccountGold() : AccountPanelInnerFill())
			.SetTextColor(bActive ? FLinearColor(0.10f, 0.08f, 0.05f, 1.0f) : AccountText())
			.SetUseGlow(false));
	};

	auto MakeHistoryFilterDropdown = [&](const FText& Label, TFunction<FText()> GetValueText, TFunction<TSharedRef<SWidget>()> MakeMenu, float MinWidth) -> TSharedRef<SWidget>
	{
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(AccountRegularFont(9))
				.ColorAndOpacity(AccountMutedText())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)
			[
				FT66Style::MakeDropdown(
					FT66DropdownParams(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text_Lambda([GetValueText]() { return GetValueText(); })
							.Font(AccountRegularFont(11))
							.ColorAndOpacity(AccountText())
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Account", "HistoryDropdownArrow", "v"))
							.Font(AccountBoldFont(10))
							.ColorAndOpacity(AccountMutedText())
						],
						MoveTemp(MakeMenu))
					.SetMinWidth(MinWidth)
					.SetHeight(0.f)
					.SetPadding(FMargin(10.f, 7.f, 10.f, 6.f)))
			],
			ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(8.f, 7.f, 8.f, 6.f));
	};

	auto MakeHeroHistoryFilterMenu = [this, HistoryHeroFilterEntries, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "AllHeroesMenu", "All Heroes"),
				HistoryHeroFilter.IsNone(),
				[this]() { HistoryHeroFilter = NAME_None; })
		];

		for (const FAccountNamedEntry& Entry : HistoryHeroFilterEntries)
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					Entry.DisplayName,
					HistoryHeroFilter == Entry.ID,
					[this, HeroID = Entry.ID]() { HistoryHeroFilter = HeroID; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakeDifficultyHistoryFilterMenu = [this, DifficultyText, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "AllDifficultiesMenu", "All Difficulties"),
				!HistoryDifficultyFilter.IsSet(),
				[this]() { HistoryDifficultyFilter.Reset(); })
		];

		for (ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					DifficultyText(Difficulty),
					HistoryDifficultyFilter.IsSet() && HistoryDifficultyFilter.GetValue() == Difficulty,
					[this, Difficulty]() { HistoryDifficultyFilter = Difficulty; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakePartySizeHistoryFilterMenu = [this, Loc, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "AllPartySizesMenu", "All Party Sizes"),
				!HistoryPartySizeFilter.IsSet(),
				[this]() { HistoryPartySizeFilter.Reset(); })
		];

		for (ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					PartySizeText(Loc, PartySize),
					HistoryPartySizeFilter.IsSet() && HistoryPartySizeFilter.GetValue() == PartySize,
					[this, PartySize]() { HistoryPartySizeFilter = PartySize; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakeCompletionHistoryFilterMenu = [this, CompletionFilterText, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		for (EHistoryCompletionFilter Filter : { EHistoryCompletionFilter::All, EHistoryCompletionFilter::Completed, EHistoryCompletionFilter::NotCompleted })
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					CompletionFilterText(Filter),
					HistoryCompletionFilter == Filter,
					[this, Filter]() { HistoryCompletionFilter = Filter; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakePBPartySizeMenu = [this, Loc, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		for (ET66PartySize PartySize : { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
		{
			Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeHistoryFilterMenuEntry(
					PartySizeText(Loc, PartySize),
					ActivePBPartySize == PartySize,
					[this, PartySize]() { ActivePBPartySize = PartySize; })
			];
		}

		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto GetPBViewModeText = [this]() -> FText
	{
		return ActivePBViewMode == EPersonalBestViewMode::HighestRank
			? NSLOCTEXT("T66.Account", "PBViewHighestRank", "Highest Rank")
			: NSLOCTEXT("T66.Account", "PBViewPersonalBest", "Personal Best");
	};

	auto MakePBViewModeMenu = [this, MakeHistoryFilterMenuEntry]() -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
		Menu->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "PBViewPersonalBest", "Personal Best"),
				ActivePBViewMode == EPersonalBestViewMode::PersonalBest,
				[this]() { ActivePBViewMode = EPersonalBestViewMode::PersonalBest; })
		];
		Menu->AddSlot().AutoHeight()
		[
			MakeHistoryFilterMenuEntry(
				NSLOCTEXT("T66.Account", "PBViewHighestRank", "Highest Rank"),
				ActivePBViewMode == EPersonalBestViewMode::HighestRank,
				[this]() { ActivePBViewMode = EPersonalBestViewMode::HighestRank; })
		];
		return MakeAccountPanel(Menu, ET66PanelType::Panel2, AccountPanelFill(), FMargin(6.f));
	};

	auto MakePBViewModeDropdown = [this, GetPBViewModeText, MakePBViewModeMenu]() -> TSharedRef<SWidget>
	{
		return MakeAccountPanel(
			FT66Style::MakeDropdown(
				FT66DropdownParams(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([GetPBViewModeText]() { return GetPBViewModeText(); })
						.Font(AccountRegularFont(14))
						.ColorAndOpacity(AccountText())
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Account", "PBModeDropdownArrow", "v"))
						.Font(AccountBoldFont(12))
						.ColorAndOpacity(AccountMutedText())
					],
					MakePBViewModeMenu)
				.SetMinWidth(0.f)
				.SetHeight(0.f)
				.SetPadding(FMargin(12.f, 9.f, 12.f, 8.f))),
			ET66PanelType::Panel2,
			AccountPanelInnerFill(),
			FMargin(8.f, 7.f, 8.f, 6.f));
	};

	auto MakePBPartySizeDropdown = [this, Loc, MakePBPartySizeMenu]() -> TSharedRef<SWidget>
	{
		return MakeAccountPanel(
			FT66Style::MakeDropdown(
				FT66DropdownParams(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([this, Loc]() { return PartySizeText(Loc, ActivePBPartySize); })
						.Font(AccountRegularFont(14))
						.ColorAndOpacity(AccountText())
					]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Account", "PBPartySizeDropdownArrow", "v"))
						.Font(AccountBoldFont(12))
						.ColorAndOpacity(AccountMutedText())
					],
					MakePBPartySizeMenu)
				.SetMinWidth(0.f)
				.SetHeight(0.f)
				.SetPadding(FMargin(12.f, 9.f, 12.f, 8.f))),
			ET66PanelType::Panel2,
			AccountPanelInnerFill(),
			FMargin(8.f, 7.f, 8.f, 6.f));
	};

	auto DoesRunMatchHistoryFilters = [&](const FT66RecentRunRecord& Run) -> bool
	{
		if (!HistoryHeroFilter.IsNone() && Run.HeroID != HistoryHeroFilter)
		{
			return false;
		}

		if (HistoryDifficultyFilter.IsSet() && Run.Difficulty != HistoryDifficultyFilter.GetValue())
		{
			return false;
		}

		if (HistoryPartySizeFilter.IsSet() && Run.PartySize != HistoryPartySizeFilter.GetValue())
		{
			return false;
		}

		switch (HistoryCompletionFilter)
		{
		case EHistoryCompletionFilter::Completed:
			return Run.bWasFullClear;
		case EHistoryCompletionFilter::NotCompleted:
			return !Run.bWasFullClear;
		case EHistoryCompletionFilter::All:
		default:
			return true;
		}
	};

	TSharedRef<SVerticalBox> HistoryRows = SNew(SVerticalBox);
	int32 FilteredRunCount = 0;
	int32 VisibleHistoryIndex = 0;
	for (const FT66RecentRunRecord& Run : RecentRuns)
	{
		if (!DoesRunMatchHistoryFilters(Run))
		{
			continue;
		}

		++FilteredRunCount;
		FHeroData HeroData;
		FCompanionData CompanionData;
		const bool bHasHeroData = T66GI && T66GI->GetHeroData(Run.HeroID, HeroData);
		const FText HeroName = bHasHeroData ? (Loc ? Loc->GetText_HeroName(Run.HeroID) : HeroData.DisplayName) : FText::FromName(Run.HeroID);
		const FText CompanionName = (T66GI && T66GI->GetCompanionData(Run.CompanionID, CompanionData))
			? (Loc ? Loc->GetText_CompanionName(Run.CompanionID) : CompanionData.DisplayName)
			: (Run.CompanionID.IsNone() ? NSLOCTEXT("T66.Account", "NoComp", "No Companion") : FText::FromName(Run.CompanionID));
		const FText StatusText = Run.bWasFullClear ? NSLOCTEXT("T66.Account", "CompletedRun", "Completed") : NSLOCTEXT("T66.Account", "NotCompletedRun", "Not Completed");
		const FLinearColor StatusColor = Run.bWasFullClear ? AccountSuccess() : AccountDanger();
		const FText RunDetailsText = FText::Format(NSLOCTEXT("T66.Account", "RunDetailsFmt", "{0} / {1}"), DifficultyText(Run.Difficulty), PartySizeText(Loc, Run.PartySize));
		const FText SublineText = FText::Format(NSLOCTEXT("T66.Account", "HistorySublineFmt", "{0} / Stage {1}"), CompanionName, FText::AsNumber(Run.StageReached));
		const bool bCanOpen = !Run.RunSummarySlotName.IsEmpty();
		const FSlateBrush* PortraitBrush = GetOrCreateHeroPortraitBrush(T66GI, Run.HeroID);
		const FString HeroInitial = !HeroName.IsEmptyOrWhitespace() ? HeroName.ToString().Left(1).ToUpper() : FString(TEXT("?"));

		HistoryRows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 2.f)
		[
			FT66Style::MakeButton(
				FT66ButtonParams(FText::GetEmpty(), bCanOpen ? FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, Run.RunSummarySlotName) : FOnClicked::CreateLambda([]() { return FReply::Handled(); }), ET66ButtonType::Row)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetPadding(FMargin(0.f))
				.SetEnabled(TAttribute<bool>(bCanOpen))
				.SetUseGlow(false)
				.SetContent(
					MakeAccountPanel(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.75f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 10.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(32.f)
								.HeightOverride(32.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(bHasHeroData ? HeroData.PlaceholderColor : AccountMutedGold())
									.Padding(0.f)
									[
										PortraitBrush
											? StaticCastSharedRef<SWidget>(SNew(SImage).Image(PortraitBrush))
											: StaticCastSharedRef<SWidget>(
												SNew(STextBlock)
												.Text(FText::FromString(HeroInitial))
												.Font(AccountBoldFont(14))
												.ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
												.Justification(ETextJustify::Center))
									]
								]
							]
							+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(STextBlock)
									.Text(HeroName)
									.Font(AccountBoldFont(12))
									.ColorAndOpacity(AccountText())
								]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 1.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(SublineText)
									.Font(AccountRegularFont(10))
									.ColorAndOpacity(AccountMutedText())
								]
							]
						]
						+ SHorizontalBox::Slot().FillWidth(0.95f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(StatusText)
							.Font(AccountBoldFont(12))
							.ColorAndOpacity(StatusColor)
						]
						+ SHorizontalBox::Slot().FillWidth(1.10f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Run.EndedAtUtc.ToString(TEXT("%m/%d/%Y %H:%M"))))
							.Font(AccountRegularFont(11))
							.ColorAndOpacity(AccountText())
						]
						+ SHorizontalBox::Slot().FillWidth(0.85f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FormatDurationText(Run.DurationSeconds))
							.Font(AccountBoldFont(12))
							.ColorAndOpacity(AccountText())
						]
						+ SHorizontalBox::Slot().FillWidth(1.10f).VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(RunDetailsText)
							.Font(AccountRegularFont(11))
							.ColorAndOpacity(AccountMutedText())
						],
						ET66PanelType::Panel2,
						VisibleHistoryIndex % 2 == 0 ? AccountRowFill() : AccountRowAltFill(),
						FMargin(8.f, 6.f, 8.f, 5.f))))
		];

		++VisibleHistoryIndex;
	}

	const bool bHasHistoryFilters = !HistoryHeroFilter.IsNone()
		|| HistoryDifficultyFilter.IsSet()
		|| HistoryPartySizeFilter.IsSet()
		|| HistoryCompletionFilter != EHistoryCompletionFilter::All;
	if (FilteredRunCount == 0)
	{
		HistoryRows->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(RecentRuns.Num() == 0
				? NSLOCTEXT("T66.Account", "NoRuns", "No runs have been recorded yet.")
				: (bHasHistoryFilters
					? NSLOCTEXT("T66.Account", "NoFilteredRuns", "No runs match the current filters.")
					: NSLOCTEXT("T66.Account", "NoRunsFallback", "No runs have been recorded yet.")))
			.Font(AccountRegularFont(13))
			.ColorAndOpacity(AccountMutedText())
		];
	}

	const TSharedRef<SWidget> HistoryFilterBar = MakeAccountPanel(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.34f).Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeHistoryFilterDropdown(
				NSLOCTEXT("T66.Account", "HistoryHeroFilterLabel", "Hero"),
				GetHeroHistoryFilterText,
				MakeHeroHistoryFilterMenu,
				180.f)
		]
		+ SHorizontalBox::Slot().FillWidth(0.22f).Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeHistoryFilterDropdown(
				NSLOCTEXT("T66.Account", "HistoryDifficultyFilterLabel", "Difficulty"),
				GetDifficultyHistoryFilterText,
				MakeDifficultyHistoryFilterMenu,
				160.f)
		]
		+ SHorizontalBox::Slot().FillWidth(0.22f).Padding(0.f, 0.f, 8.f, 0.f)
		[
			MakeHistoryFilterDropdown(
				NSLOCTEXT("T66.Account", "HistoryPartySizeFilterLabel", "Party Size"),
				GetPartySizeHistoryFilterText,
				MakePartySizeHistoryFilterMenu,
				160.f)
		]
		+ SHorizontalBox::Slot().FillWidth(0.22f)
		[
			MakeHistoryFilterDropdown(
				NSLOCTEXT("T66.Account", "HistoryStatusFilterLabel", "Status"),
				GetCompletionHistoryFilterText,
				MakeCompletionHistoryFilterMenu,
				160.f)
		],
		ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(10.f));

	const TSharedRef<SWidget> HistoryColumnHeader = MakeAccountPanel(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.75f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColHero", "HERO PLAYED")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())]
		+ SHorizontalBox::Slot().FillWidth(0.95f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColStatus", "STATUS")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())]
		+ SHorizontalBox::Slot().FillWidth(1.10f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColDate", "DATE / TIME")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())]
		+ SHorizontalBox::Slot().FillWidth(0.85f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColDuration", "DURATION")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())]
		+ SHorizontalBox::Slot().FillWidth(1.10f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistColRun", "RUN")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())],
		ET66PanelType::Panel2, AccountHeaderFill(), FMargin(8.f, 6.f, 8.f, 5.f));

	auto MakePBBlock = [&](const FText& Title, bool bTime) -> TSharedRef<SWidget>
	{
		const bool bRankInThirdColumn = ActivePBViewMode == EPersonalBestViewMode::PersonalBest;
		const FText ValueHeaderText = bTime
			? NSLOCTEXT("T66.Account", "PBColTime", "TIME")
			: NSLOCTEXT("T66.Account", "PBColScore", "SCORE");
		const FText RankHeaderText = NSLOCTEXT("T66.Account", "PBColRank", "GLOBAL RANK");
		TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
		for (ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
		{
			const FPersonalBestDisplay PB = bTime ? MakePBTime(Difficulty) : MakePBScore(Difficulty);
			const bool bCanOpen = PB.bHasRecord && !PB.RunSummarySlotName.IsEmpty();
			const FName PBHeroID = PB.bHasRecord ? ResolvePBHeroID(PB.RunSummarySlotName) : NAME_None;
			const FText HeroName = PB.bHasRecord
				? (PBHeroID.IsNone()
					? NSLOCTEXT("T66.Account", "PBUnknownHero", "Unknown")
					: HeroText(PBHeroID))
				: FText::GetEmpty();
			const FText Value = PB.bHasRecord ? (bTime ? FormatDurationText(PB.Seconds) : FText::AsNumber(PB.Score)) : NSLOCTEXT("T66.Account", "NoRecord", "No Record");
			const FText Date = PB.bHasRecord && PB.AchievedAtUtc.GetTicks() > 0 ? FText::FromString(PB.AchievedAtUtc.ToString(TEXT("%m/%d/%Y"))) : FText::GetEmpty();
			FText RankText = FText::GetEmpty();
			if (PB.bHasRecord)
			{
				if (!Backend || !Backend->IsBackendConfigured() || !Backend->HasSteamTicket())
				{
					RankText = NSLOCTEXT("T66.Account", "RankUnavailable", "N/A");
				}
				else if (!PB.bHasRankState)
				{
					RankText = NSLOCTEXT("T66.Account", "RankPending", "...");
				}
				else if (!PB.bRankRequestSucceeded)
				{
					RankText = NSLOCTEXT("T66.Account", "RankFailed", "N/A");
				}
				else if (PB.GlobalRank > 0)
				{
					RankText = FText::Format(NSLOCTEXT("T66.Account", "RankFmt", "#{0}"), FText::AsNumber(PB.GlobalRank));
				}
				else
				{
					RankText = NSLOCTEXT("T66.Account", "RankUnranked", "Unranked");
				}
			}

			const FText ThirdColumnText = bRankInThirdColumn ? RankText : Value;
			const FText FourthColumnText = bRankInThirdColumn ? Value : RankText;
			const FLinearColor ThirdColumnColor = bRankInThirdColumn ? AccountText() : (PB.bHasRecord ? AccountGold() : AccountMutedText());
			const FLinearColor FourthColumnColor = bRankInThirdColumn ? (PB.bHasRecord ? AccountGold() : AccountMutedText()) : AccountText();
			const FSlateFontInfo ThirdColumnFont = bRankInThirdColumn ? AccountBoldFont(12) : AccountBoldFont(15);
			const FSlateFontInfo FourthColumnFont = bRankInThirdColumn ? AccountBoldFont(15) : AccountBoldFont(12);

			Rows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(FText::GetEmpty(), bCanOpen ? FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, PB.RunSummarySlotName) : FOnClicked::CreateLambda([]() { return FReply::Handled(); }), ET66ButtonType::Row)
					.SetBorderVisual(ET66ButtonBorderVisual::None).SetBackgroundVisual(ET66ButtonBackgroundVisual::None).SetPadding(FMargin(0.f)).SetEnabled(TAttribute<bool>(bCanOpen)).SetUseGlow(false)
					.SetContent(
						MakeAccountPanel(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(0.90f).VAlign(VAlign_Center)[SNew(STextBlock).Text(DifficultyText(Difficulty)).Font(AccountBoldFont(12)).ColorAndOpacity(AccountText())]
							+ SHorizontalBox::Slot().FillWidth(1.00f).VAlign(VAlign_Center)[SNew(STextBlock).Text(HeroName).Font(AccountRegularFont(10)).ColorAndOpacity(PB.bHasRecord ? AccountText() : AccountMutedText())]
							+ SHorizontalBox::Slot().FillWidth(0.95f).VAlign(VAlign_Center)[SNew(STextBlock).Text(Date).Font(AccountRegularFont(10)).ColorAndOpacity(AccountMutedText())]
							+ SHorizontalBox::Slot().FillWidth(0.95f).VAlign(VAlign_Center)[SNew(STextBlock).Text(ThirdColumnText).Font(ThirdColumnFont).ColorAndOpacity(ThirdColumnColor)]
							+ SHorizontalBox::Slot().FillWidth(1.00f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FourthColumnText).Font(FourthColumnFont).ColorAndOpacity(FourthColumnColor)],
							ET66PanelType::Panel2, AccountRowFill(), FMargin(8.f, 7.f, 8.f, 6.f))))
			];
		}
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[SNew(STextBlock).Text(Title).Font(AccountBoldFont(14)).ColorAndOpacity(AccountText())]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				MakeAccountPanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(0.90f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "PBColDifficulty", "DIFFICULTY")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())]
					+ SHorizontalBox::Slot().FillWidth(1.00f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "PBColHero", "HERO")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())]
					+ SHorizontalBox::Slot().FillWidth(0.95f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "PBColDate", "DATE")).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())]
					+ SHorizontalBox::Slot().FillWidth(0.95f)[SNew(STextBlock).Text(bRankInThirdColumn ? RankHeaderText : ValueHeaderText).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())]
					+ SHorizontalBox::Slot().FillWidth(1.00f)[SNew(STextBlock).Text(bRankInThirdColumn ? ValueHeaderText : RankHeaderText).Font(AccountBoldFont(10)).ColorAndOpacity(AccountGold())],
					ET66PanelType::Panel2, AccountHeaderFill(), FMargin(8.f, 6.f, 8.f, 5.f))
			]
			+ SVerticalBox::Slot().AutoHeight()[Rows],
			ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(10.f));
	};

	TSharedRef<SWidget> OverviewContent =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 12.f, 0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
			[
				MakeAccountPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "StatusHeader", "ACCOUNT STATUS"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(RestrictionText(Restriction.Restriction))
							.Font(AccountBoldFont(22))
							.ColorAndOpacity(bAccountEligible ? AccountSuccess() : AccountDanger())
						]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 2.f, 0.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									NSLOCTEXT("T66.Account", "StandingHelpButton", "?"),
									FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleStandingInfoClicked),
									ET66ButtonType::Neutral)
								.SetBorderVisual(ET66ButtonBorderVisual::None)
								.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
								.SetFontSize(AdjustAccountFontSize(11))
								.SetMinWidth(22.f)
								.SetHeight(22.f)
								.SetPadding(FMargin(0.f))
								.SetColor(AccountPanelInnerFill())
								.SetTextColor(AccountText())
								.SetUseGlow(false))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(SBox)
						.Visibility(bShowStandingInfoPopup ? EVisibility::Visible : EVisibility::Collapsed)
						[
							MakeAccountPanel(
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.Account", "StandingHelpBody", "Cheating, client tampering, or exploit abuse will suspend this account. Suspended accounts and their runs are not eligible for leaderboards or ranked personal-best tracking until the restriction is cleared."))
								.Font(AccountRegularFont(11))
								.ColorAndOpacity(AccountText())
								.AutoWrapText(true),
								ET66PanelType::Panel2,
								AccountPanelInnerFill(),
								FMargin(10.f, 8.f))
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[SNew(STextBlock).Text(bAccountEligible ? NSLOCTEXT("T66.Account", "EligibleBody", "Your account is eligible for ranked tracking and personal best progression.") : NSLOCTEXT("T66.Account", "RestrictedBody", "This account is suspended from leaderboard submissions until the restriction is cleared.")).Font(AccountRegularFont(12)).ColorAndOpacity(AccountText()).AutoWrapText(true)]
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(AppealStatusText(Restriction.AppealStatus)).Font(AccountBoldFont(11)).ColorAndOpacity(AccountMutedText())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)[SNew(SBox).Visibility(!Restriction.RestrictionReason.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)[MakeAccountPanel(SNew(STextBlock).Text(FText::FromString(Restriction.RestrictionReason)).Font(AccountRegularFont(11)).ColorAndOpacity(AccountText()).AutoWrapText(true), ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(10.f))]]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[SNew(SBox).Visibility(LB && LB->HasAccountRestrictionRunSummary() ? EVisibility::Visible : EVisibility::Collapsed)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.Account", "ViewReviewed", "VIEW REVIEWED RUN"), FOnClicked::CreateLambda([this, WeakLB]() { if (UT66LeaderboardSubsystem* RuntimeLB = WeakLB.Get(); RuntimeLB && RuntimeLB->RequestOpenAccountRestrictionRunSummary()) { ShowModal(ET66ScreenType::RunSummary); } return FReply::Handled(); }), ET66ButtonType::Primary).SetBorderVisual(ET66ButtonBorderVisual::None).SetBackgroundVisual(ET66ButtonBackgroundVisual::None).SetFontSize(AdjustAccountFontSize(12)).SetMinWidth(190.f).SetHeight(32.f).SetPadding(FMargin(12.f, 6.f, 12.f, 4.f)).SetColor(AccountGold()).SetTextColor(FLinearColor(0.08f, 0.07f, 0.05f, 1.0f)).SetUseGlow(false))]],
					ET66PanelType::Panel, AccountPanelFill(), FMargin(14.f))
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				MakeAccountPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "ProgressHeader", "ACCOUNT PROGRESS"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "AchProg", "Achievements Unlocked"), UnlockedAchievements, AchievementDefs.Num(), AccountGold())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "PowerProg", "Permanent Buffs Unlocked"), UnlockedPowerUps, PowerStats.Num() * UT66BuffSubsystem::MaxFillStepsPerStat, FLinearColor(0.76f, 0.63f, 0.30f, 1.0f))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "HeroProg", "Heroes Unlocked"), HeroIDs.Num(), HeroIDs.Num(), FLinearColor(0.68f, 0.78f, 0.92f, 1.0f))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "CompProg", "Companions Unlocked"), UnlockedCompanions, CompanionIDs.Num(), FLinearColor(0.57f, 0.84f, 0.79f, 1.0f))]
					+ SVerticalBox::Slot().AutoHeight()[MakeProgressRow(NSLOCTEXT("T66.Account", "StageProg", "Stages Beat"), DisplayStagesCleared, TotalStageCount, FLinearColor(0.82f, 0.47f, 0.30f, 1.0f))],
					ET66PanelType::Panel, AccountPanelFill(), FMargin(14.f))
			]
		]
		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			MakeAccountPanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0.f, 0.f, 8.f, 0.f)[MakePBViewModeDropdown()]
					+ SHorizontalBox::Slot().FillWidth(1.f)[MakePBPartySizeDropdown()]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 10.f)[MakePBBlock(NSLOCTEXT("T66.Account", "TopScore", "Highest Score"), false)]
				+ SVerticalBox::Slot().AutoHeight()[MakePBBlock(NSLOCTEXT("T66.Account", "TopTime", "Best Speed Run"), true)],
				ET66PanelType::Panel, AccountPanelFill(), FMargin(14.f))
		];

	TSharedRef<SWidget> HistoryContent = MakeAccountPanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "HistoryHdr", "RECENT RUNS"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistorySub", "Click any run to open its saved summary.")).Font(AccountRegularFont(12)).ColorAndOpacity(AccountMutedText())]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[HistoryFilterBar]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[HistoryColumnHeader]
		+ SVerticalBox::Slot().AutoHeight()[HistoryRows],
		ET66PanelType::Panel, AccountPanelFill(), FMargin(14.f));

	const FText SuspensionHeadline =
		Restriction.Restriction == ET66AccountRestrictionKind::CheatingCertainty
		? NSLOCTEXT("T66.Account", "SuspensionRestrictedHeadline", "ACCOUNT RESTRICTED")
		: NSLOCTEXT("T66.Account", "SuspensionHeadline", "ACCOUNT SUSPENDED");
	const FText SuspensionBody =
		Restriction.Restriction == ET66AccountRestrictionKind::CheatingCertainty
		? NSLOCTEXT("T66.Account", "SuspensionRestrictedBody", "This account is blocked from leaderboard submissions until the restriction is cleared.")
		: NSLOCTEXT("T66.Account", "SuspensionBody", "This account cannot submit leaderboard scores while the suspension is active.");
	const bool bCanSubmitAppeal = LB && LB->CanSubmitAccountAppeal();
	TSharedRef<SWidget> SuspensionContent = MakeAccountPanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "SuspensionHdr", "SUSPENSION"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(SuspensionHeadline)
			.Font(AccountBoldFont(22))
			.ColorAndOpacity(AccountDanger())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(SuspensionBody)
			.Font(AccountRegularFont(12))
			.ColorAndOpacity(AccountText())
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeAccountPanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Account", "SuspensionReasonLabel", "REASON"))
					.Font(AccountBoldFont(11))
					.ColorAndOpacity(AccountGold())
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Restriction.RestrictionReason.IsEmpty() ? NSLOCTEXT("T66.Account", "NoSuspensionReason", "No reason recorded.") : FText::FromString(Restriction.RestrictionReason))
					.Font(AccountRegularFont(12))
					.ColorAndOpacity(AccountText())
					.AutoWrapText(true)
				],
				ET66PanelType::Panel2,
				AccountPanelInnerFill(),
				FMargin(12.f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(AppealStatusText(Restriction.AppealStatus))
			.Font(AccountBoldFont(11))
			.ColorAndOpacity(AccountMutedText())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
		[
			SNew(SBox)
			.Visibility(!AppealSubmitStatusMessage.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)
			[
				SNew(STextBlock)
				.Text(FText::FromString(AppealSubmitStatusMessage))
				.Font(AccountRegularFont(11))
				.ColorAndOpacity(bAppealSubmitStatusIsError ? AccountDanger() : AccountSuccess())
				.AutoWrapText(true)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(SBox)
			.Visibility(bCanSubmitAppeal && !bAppealEditorOpen ? EVisibility::Visible : EVisibility::Collapsed)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(NSLOCTEXT("T66.Account", "OpenAppeal", "APPEAL"), FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenAppealClicked), ET66ButtonType::Primary)
					.SetBorderVisual(ET66ButtonBorderVisual::None)
					.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
					.SetFontSize(AdjustAccountFontSize(12))
					.SetMinWidth(150.f)
					.SetHeight(32.f)
					.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f))
					.SetColor(AccountGold())
					.SetTextColor(FLinearColor(0.08f, 0.07f, 0.05f, 1.0f))
					.SetUseGlow(false))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(SBox)
			.Visibility(bCanSubmitAppeal && bAppealEditorOpen ? EVisibility::Visible : EVisibility::Collapsed)
			[
				MakeAccountPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(Loc ? Loc->GetText_AccountStatus_AppealTitle() : NSLOCTEXT("T66.Account", "AppealTitle", "APPEAL"))
						.Font(AccountBoldFont(12))
						.ColorAndOpacity(AccountGold())
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 10.f)
					[
						SAssignNew(AppealMessageTextBox, SMultiLineEditableTextBox)
						.AutoWrapText(true)
						.Text(FText::FromString(AppealDraftMessage))
						.HintText(Loc ? Loc->GetText_AccountStatus_AppealHint() : NSLOCTEXT("T66.Account", "AppealHint", "Write your appeal message here..."))
						.OnTextChanged_Lambda([this](const FText& NewText)
						{
							AppealDraftMessage = NewText.ToString();
						})
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(Loc ? Loc->GetText_AccountStatus_SubmitAppeal() : NSLOCTEXT("T66.Account", "SubmitAppeal", "SUBMIT APPEAL"), FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleSubmitAppealClicked), ET66ButtonType::Primary)
								.SetBorderVisual(ET66ButtonBorderVisual::None)
								.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
								.SetFontSize(AdjustAccountFontSize(11))
								.SetMinWidth(168.f)
								.SetHeight(32.f)
								.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f))
								.SetColor(AccountGold())
								.SetTextColor(FLinearColor(0.08f, 0.07f, 0.05f, 1.0f))
								.SetUseGlow(false))
						]
						+ SHorizontalBox::Slot().AutoWidth()
						[
							FT66Style::MakeButton(
								FT66ButtonParams(NSLOCTEXT("T66.Account", "CancelAppeal", "CANCEL"), FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleCancelAppealClicked), ET66ButtonType::Neutral)
								.SetBorderVisual(ET66ButtonBorderVisual::None)
								.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
								.SetFontSize(AdjustAccountFontSize(11))
								.SetMinWidth(118.f)
								.SetHeight(32.f)
								.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f))
								.SetColor(AccountPanelFill())
								.SetTextColor(AccountText())
								.SetUseGlow(false))
						]
					],
					ET66PanelType::Panel2,
					AccountPanelInnerFill(),
					FMargin(12.f))
			]
		],
		ET66PanelType::Panel,
		AccountPanelFill(),
		FMargin(14.f));

	const TSharedRef<SWidget> ActiveContent =
		ActiveTab == EAccountTab::Suspension ? SuspensionContent
		: (ActiveTab == EAccountTab::History ? HistoryContent : OverviewContent);
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	TSharedRef<SWidget> Content =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 12.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.f)[SNew(SSpacer)]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)
			[
				SNew(SBox)
				.Visibility(bHasSuspension ? EVisibility::Visible : EVisibility::Collapsed)
				[
					MakeTabButton(
						NSLOCTEXT("T66.Account", "SuspensionTab", "SUSPENSION"),
						ActiveTab == EAccountTab::Suspension,
						&UT66AccountStatusScreen::HandleSuspensionTabClicked,
						AccountDanger(),
						FLinearColor(0.22f, 0.08f, 0.07f, 1.0f),
						FLinearColor(0.12f, 0.07f, 0.06f, 1.0f),
						FLinearColor(0.96f, 0.80f, 0.78f, 1.0f))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeTabButton(NSLOCTEXT("T66.Account", "OverviewTab", "OVERVIEW"), ActiveTab == EAccountTab::Overview, &UT66AccountStatusScreen::HandleOverviewTabClicked)]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeTabButton(NSLOCTEXT("T66.Account", "HistoryTab", "HISTORY"), ActiveTab == EAccountTab::History, &UT66AccountStatusScreen::HandleHistoryTabClicked)]
			+ SHorizontalBox::Slot().FillWidth(1.f)[SNew(SSpacer)]
		]
		+ SVerticalBox::Slot().FillHeight(1.f)[SNew(SScrollBox) + SScrollBox::Slot()[ActiveContent]]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(SBox)
			.Visibility(bModalPresentation ? EVisibility::Visible : EVisibility::Collapsed)
			[
				FT66Style::MakeButton(FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleBackClicked), ET66ButtonType::Neutral).SetBorderVisual(ET66ButtonBorderVisual::None).SetBackgroundVisual(ET66ButtonBackgroundVisual::None).SetFontSize(AdjustAccountFontSize(13)).SetMinWidth(116.f).SetHeight(34.f).SetPadding(FMargin(12.f, 7.f, 12.f, 5.f)).SetColor(AccountPanelInnerFill()).SetTextColor(AccountText()).SetUseGlow(false))
			]
		];

	const TSharedRef<SWidget> ModalFrame = SNew(SBox).MaxDesiredWidth(1600.f)[MakeAccountPanel(Content, ET66PanelType::Panel, AccountPanelFill(), FMargin(18.f))];
	const TSharedRef<SWidget> FullWidthFrame = MakeAccountPanel(Content, ET66PanelType::Panel, AccountPanelFill(), FMargin(10.f, 0.f, 10.f, 10.f));
	if (bModalPresentation)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FT66Style::Scrim())]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(24.f, 30.f))[ModalFrame];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::ScreenBackground())
		[
			SNew(SBox)
			.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
			[
				FullWidthFrame
			]
		];
}

const FSlateBrush* UT66AccountStatusScreen::GetOrCreateHeroPortraitBrush(UT66GameInstance* T66GI, FName HeroID)
{
	if (!T66GI || HeroID.IsNone())
	{
		return nullptr;
	}

	if (TSharedPtr<FSlateBrush>* Found = HeroPortraitBrushes.Find(HeroID))
	{
		return Found->Get();
	}

	UT66UITexturePoolSubsystem* TexPool = T66GI->GetSubsystem<UT66UITexturePoolSubsystem>();
	if (!TexPool)
	{
		return nullptr;
	}

	const TSoftObjectPtr<UTexture2D> PortraitSoft = T66GI->ResolveHeroPortrait(HeroID, ET66BodyType::TypeA, ET66HeroPortraitVariant::Low);
	if (PortraitSoft.IsNull())
	{
		return nullptr;
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->DrawAs = ESlateBrushDrawType::Image;
	Brush->Tiling = ESlateBrushTileType::NoTile;
	Brush->ImageSize = FVector2D(32.f, 32.f);
	Brush->SetResourceObject(T66SlateTexture::GetLoaded(TexPool, PortraitSoft));

	T66SlateTexture::BindSharedBrushAsync(
		TexPool,
		PortraitSoft,
		this,
		Brush,
		FName(*FString::Printf(TEXT("AccountHero_%s"), *HeroID.ToString())),
		/*bClearWhileLoading*/ false);

	HeroPortraitBrushes.Add(HeroID, Brush);
	return Brush.Get();
}

FReply UT66AccountStatusScreen::HandleBackClicked()
{
	if (UIManager)
	{
		if (UIManager->GetCurrentModalType() == ScreenType)
		{
			if (APlayerController* PC = GetOwningPlayer())
			{
				if (PC->IsPaused())
				{
					UIManager->ShowModal(ET66ScreenType::Leaderboard);
					return FReply::Handled();
				}
			}

			UIManager->CloseModal();
			return FReply::Handled();
		}

		UIManager->GoBack();
	}

	return FReply::Handled();
}

void UT66AccountStatusScreen::HandleBackendAppealSubmitComplete(bool bSuccess, const FString& Message)
{
	AppealSubmitStatusMessage = Message;
	bAppealSubmitStatusIsError = !bSuccess;
	if (bSuccess)
	{
		bAppealEditorOpen = false;
		AppealDraftMessage.Reset();
	}

	if (!HasBuiltSlateUI() || !IsVisible())
	{
		return;
	}

	ForceRebuildSlate();
}

FReply UT66AccountStatusScreen::HandleSuspensionTabClicked()
{
	ActiveTab = EAccountTab::Suspension;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleOverviewTabClicked()
{
	ActiveTab = EAccountTab::Overview;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleHistoryTabClicked()
{
	ActiveTab = EAccountTab::History;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleOpenAppealClicked()
{
	bAppealEditorOpen = true;
	AppealSubmitStatusMessage.Reset();
	bAppealSubmitStatusIsError = false;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleStandingInfoClicked()
{
	bShowStandingInfoPopup = !bShowStandingInfoPopup;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleCancelAppealClicked()
{
	bAppealEditorOpen = false;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleSubmitAppealClicked()
{
	UT66LeaderboardSubsystem* LB = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	FString Message = AppealDraftMessage;
	Message.TrimStartAndEndInline();

	if (!LB || Message.IsEmpty())
	{
		AppealSubmitStatusMessage = Message.IsEmpty()
			? NSLOCTEXT("T66.Account", "AppealRequired", "Appeal message is required.").ToString()
			: NSLOCTEXT("T66.Account", "AppealUnavailable", "Appeals are unavailable right now.").ToString();
		bAppealSubmitStatusIsError = true;
		ForceRebuildSlate();
		return FReply::Handled();
	}

	AppealSubmitStatusMessage = NSLOCTEXT("T66.Account", "AppealSubmitting", "Submitting appeal...").ToString();
	bAppealSubmitStatusIsError = false;
	LB->SubmitAccountAppeal(Message, FString());
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleOpenRunSummaryClicked(FString SlotName)
{
	if (SlotName.IsEmpty())
	{
		return FReply::Handled();
	}

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LeaderboardSubsystem* LB = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	if (!LB)
	{
		return FReply::Handled();
	}

	const bool bModalPresentation = UIManager && UIManager->GetCurrentModalType() == ScreenType;
	if (LB->RequestOpenRunSummarySlot(SlotName, bModalPresentation ? ET66ScreenType::AccountStatus : ET66ScreenType::None))
	{
		ShowModal(ET66ScreenType::RunSummary);
	}

	return FReply::Handled();
}
