// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66AccountStatusScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66CompanionUnlockSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PowerUpSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"

#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
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

	FText MedalText(ET66AccountMedalTier Tier)
	{
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return NSLOCTEXT("T66.Account", "MedalBronze", "Bronze");
		case ET66AccountMedalTier::Silver:
			return NSLOCTEXT("T66.Account", "MedalSilver", "Silver");
		case ET66AccountMedalTier::Gold:
			return NSLOCTEXT("T66.Account", "MedalGold", "Gold");
		case ET66AccountMedalTier::Platinum:
			return NSLOCTEXT("T66.Account", "MedalPlatinum", "Platinum");
		case ET66AccountMedalTier::Diamond:
			return NSLOCTEXT("T66.Account", "MedalDiamond", "Diamond");
		case ET66AccountMedalTier::None:
		default:
			return NSLOCTEXT("T66.Account", "MedalNone", "Unproven");
		}
	}

	FLinearColor MedalColor(ET66AccountMedalTier Tier)
	{
		switch (Tier)
		{
		case ET66AccountMedalTier::Bronze:
			return FLinearColor(0.67f, 0.43f, 0.26f, 1.0f);
		case ET66AccountMedalTier::Silver:
			return FLinearColor(0.76f, 0.79f, 0.84f, 1.0f);
		case ET66AccountMedalTier::Gold:
			return FLinearColor(0.89f, 0.74f, 0.27f, 1.0f);
		case ET66AccountMedalTier::Platinum:
			return FLinearColor(0.56f, 0.77f, 0.88f, 1.0f);
		case ET66AccountMedalTier::Diamond:
			return FLinearColor(0.45f, 0.86f, 0.99f, 1.0f);
		case ET66AccountMedalTier::None:
		default:
			return AccountMutedText();
		}
	}

	FText RestrictionText(ET66AccountRestrictionKind Restriction)
	{
		switch (Restriction)
		{
		case ET66AccountRestrictionKind::Suspicion:
			return NSLOCTEXT("T66.Account", "RestrictionSuspicion", "Under Review");
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
			.Font(FT66Style::Tokens::FontBold(24))
			.ColorAndOpacity(AccountGold());
	}

	struct FPersonalBestDisplay
	{
		bool bHasRecord = false;
		ET66PartySize PartySize = ET66PartySize::Solo;
		int32 Score = 0;
		float Seconds = 0.f;
		FString RunSummarySlotName;
		FDateTime AchievedAtUtc;
	};

	struct FMainEntry
	{
		FName ID = NAME_None;
		FText DisplayName;
		FString SortKey;
		FLinearColor Accent = FLinearColor::White;
		int32 Unity = 0;
		ET66AccountMedalTier Medal = ET66AccountMedalTier::None;
		int32 GamesPlayed = 0;
	};

	void SortMains(TArray<FMainEntry>& Entries)
	{
		Entries.Sort([](const FMainEntry& A, const FMainEntry& B)
		{
			if (A.Unity != B.Unity)
			{
				return A.Unity > B.Unity;
			}
			if (A.Medal != B.Medal)
			{
				return static_cast<uint8>(A.Medal) > static_cast<uint8>(B.Medal);
			}
			if (A.GamesPlayed != B.GamesPlayed)
			{
				return A.GamesPlayed > B.GamesPlayed;
			}
			return A.SortKey < B.SortKey;
		});
	}
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
	RefreshScreen();
}

void UT66AccountStatusScreen::RefreshScreen_Implementation()
{
	Super::RefreshScreen_Implementation();
	ForceRebuildSlate();
}

TSharedRef<SWidget> UT66AccountStatusScreen::BuildSlateUI()
{
	UGameInstance* GIBase = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GIBase);
	UT66LocalizationSubsystem* Loc = GIBase ? GIBase->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* LB = GIBase ? GIBase->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66AchievementsSubsystem* Achievements = GIBase ? GIBase->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	UT66PowerUpSubsystem* PowerUps = GIBase ? GIBase->GetSubsystem<UT66PowerUpSubsystem>() : nullptr;
	UT66CompanionUnlockSubsystem* CompanionUnlocks = GIBase ? GIBase->GetSubsystem<UT66CompanionUnlockSubsystem>() : nullptr;

	const bool bModalPresentation = UIManager && UIManager->GetCurrentModalType() == ScreenType;
	const float TopInset = bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f);
	const FT66AccountRestrictionRecord Restriction = LB ? LB->GetAccountRestrictionRecord() : FT66AccountRestrictionRecord();
	const bool bAccountEligible = LB ? LB->IsAccountEligibleForLeaderboard() : true;
	const TArray<FName> HeroIDs = T66GI ? T66GI->GetAllHeroIDs() : TArray<FName>();
	const TArray<FName> CompanionIDs = T66GI ? T66GI->GetAllCompanionIDs() : TArray<FName>();
	const TArray<FAchievementData> AchievementDefs = Achievements ? Achievements->GetAllAchievements() : TArray<FAchievementData>();
	const TArray<FT66RecentRunRecord> RecentRuns = LB ? LB->GetRecentRuns() : TArray<FT66RecentRunRecord>();

	int32 UnlockedAchievements = 0;
	for (const FAchievementData& A : AchievementDefs) if (A.bIsUnlocked) { ++UnlockedAchievements; }

	const TArray<ET66HeroStatType> PowerStats = {
		ET66HeroStatType::Damage, ET66HeroStatType::AttackSpeed, ET66HeroStatType::AttackScale,
		ET66HeroStatType::Armor, ET66HeroStatType::Evasion, ET66HeroStatType::Luck
	};
	int32 UnlockedPowerUps = 0;
	for (const ET66HeroStatType StatType : PowerStats) { UnlockedPowerUps += PowerUps ? PowerUps->GetUnlockedFillStepCount(StatType) : 0; }

	int32 UnlockedCompanions = 0;
	for (const FName& CompanionID : CompanionIDs)
	{
		if (!CompanionUnlocks || CompanionUnlocks->IsCompanionUnlocked(CompanionID))
		{
			++UnlockedCompanions;
		}
	}

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

	auto MakeTabButton = [this](const FText& Label, bool bActive, FReply (UT66AccountStatusScreen::*Handler)()) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(
			FT66ButtonParams(Label, FOnClicked::CreateUObject(this, Handler), bActive ? ET66ButtonType::ToggleActive : ET66ButtonType::Neutral)
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetFontSize(16)
			.SetMinWidth(150.f)
			.SetHeight(42.f)
			.SetPadding(FMargin(16.f, 10.f, 16.f, 8.f))
			.SetColor(bActive ? AccountGold() : AccountPanelInnerFill())
			.SetTextColor(bActive ? FLinearColor(0.10f, 0.08f, 0.05f, 1.0f) : AccountText())
			.SetUseGlow(false));
	};

	auto MakeMetric = [](const FText& Label, const FText& Value, const FLinearColor& ValueColor) -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(Label).Font(FT66Style::Tokens::FontRegular(11)).ColorAndOpacity(AccountMutedText())]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 3.f, 0.f, 0.f)[SNew(STextBlock).Text(Value).Font(FT66Style::Tokens::FontBold(18)).ColorAndOpacity(ValueColor)];
	};

	auto MakeChip = [](const FText& Text, const FLinearColor& Color) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Color)
			.Padding(FMargin(9.f, 4.f, 9.f, 3.f))
			[SNew(STextBlock).Text(Text).Font(FT66Style::Tokens::FontBold(11)).ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))];
	};

	auto MakeProgressRow = [&](const FText& Label, int32 Current, int32 Total, const FLinearColor& Fill) -> TSharedRef<SWidget>
	{
		const float Pct = Total > 0 ? static_cast<float>(Current) / static_cast<float>(Total) : 0.f;
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)[SNew(STextBlock).Text(Label).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountText())]
				+ SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text(FText::Format(NSLOCTEXT("T66.Account", "CountFmt", "{0}/{1}"), FText::AsNumber(Current), FText::AsNumber(Total))).Font(FT66Style::Tokens::FontBold(13)).ColorAndOpacity(AccountGold())]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 0.f, 0.f)
			[
				SNew(SProgressBar).Percent(Pct).FillColorAndOpacity(Fill)
			],
			ET66PanelType::Panel2, AccountRowFill(), FMargin(14.f, 12.f));
	};

	auto MakePBScore = [&](ET66Difficulty Difficulty) -> FPersonalBestDisplay
	{
		FPersonalBestDisplay Out;
		if (!LB) return Out;
		if (ActivePBMode == EPBMode::Solo)
		{
			FT66LocalScoreRecord R;
			if (LB->GetLocalBestScoreRecord(Difficulty, ET66PartySize::Solo, R))
			{
				Out.bHasRecord = true; Out.PartySize = ET66PartySize::Solo; Out.Score = R.BestScore; Out.RunSummarySlotName = R.RunSummarySlotName; Out.AchievedAtUtc = R.AchievedAtUtc;
			}
			return Out;
		}

		for (ET66PartySize Party : { ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
		{
			FT66LocalScoreRecord R;
			if (LB->GetLocalBestScoreRecord(Difficulty, Party, R) && (!Out.bHasRecord || R.BestScore > Out.Score))
			{
				Out.bHasRecord = true; Out.PartySize = Party; Out.Score = R.BestScore; Out.RunSummarySlotName = R.RunSummarySlotName; Out.AchievedAtUtc = R.AchievedAtUtc;
			}
		}
		return Out;
	};

	auto MakePBTime = [&](ET66Difficulty Difficulty) -> FPersonalBestDisplay
	{
		FPersonalBestDisplay Out;
		if (!LB) return Out;
		if (ActivePBMode == EPBMode::Solo)
		{
			FT66LocalCompletedRunTimeRecord R;
			if (LB->GetLocalBestCompletedRunTimeRecord(Difficulty, ET66PartySize::Solo, R))
			{
				Out.bHasRecord = true; Out.PartySize = ET66PartySize::Solo; Out.Seconds = R.BestCompletedSeconds; Out.RunSummarySlotName = R.RunSummarySlotName; Out.AchievedAtUtc = R.AchievedAtUtc;
			}
			return Out;
		}

		for (ET66PartySize Party : { ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad })
		{
			FT66LocalCompletedRunTimeRecord R;
			if (LB->GetLocalBestCompletedRunTimeRecord(Difficulty, Party, R) && (!Out.bHasRecord || R.BestCompletedSeconds < Out.Seconds))
			{
				Out.bHasRecord = true; Out.PartySize = Party; Out.Seconds = R.BestCompletedSeconds; Out.RunSummarySlotName = R.RunSummarySlotName; Out.AchievedAtUtc = R.AchievedAtUtc;
			}
		}
		return Out;
	};

	TArray<FMainEntry> HeroMains;
	for (const FName& HeroID : HeroIDs)
	{
		FHeroData Data;
		if (!T66GI || !T66GI->GetHeroData(HeroID, Data)) continue;
		FMainEntry E; E.ID = HeroID; E.DisplayName = Loc ? Loc->GetText_HeroName(HeroID) : Data.DisplayName; E.SortKey = E.DisplayName.ToString(); E.Accent = Data.PlaceholderColor;
		E.Unity = Achievements ? Achievements->GetHeroUnityStagesCleared(HeroID) : 0; E.Medal = Achievements ? Achievements->GetHeroHighestMedal(HeroID) : ET66AccountMedalTier::None; E.GamesPlayed = Achievements ? Achievements->GetHeroGamesPlayed(HeroID) : 0; HeroMains.Add(E);
	}
	SortMains(HeroMains);

	TArray<FMainEntry> CompanionMains;
	for (const FName& CompanionID : CompanionIDs)
	{
		FCompanionData Data;
		if (!T66GI || !T66GI->GetCompanionData(CompanionID, Data)) continue;
		FMainEntry E; E.ID = CompanionID; E.DisplayName = Loc ? Loc->GetText_CompanionName(CompanionID) : Data.DisplayName; E.SortKey = E.DisplayName.ToString(); E.Accent = Data.PlaceholderColor;
		E.Unity = Achievements ? Achievements->GetCompanionUnionStagesCleared(CompanionID) : 0; E.Medal = Achievements ? Achievements->GetCompanionHighestMedal(CompanionID) : ET66AccountMedalTier::None; E.GamesPlayed = Achievements ? Achievements->GetCompanionGamesPlayed(CompanionID) : 0; CompanionMains.Add(E);
	}
	SortMains(CompanionMains);

	TSharedRef<SVerticalBox> HistoryRows = SNew(SVerticalBox);
	if (RecentRuns.Num() == 0)
	{
		HistoryRows->AddSlot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "NoRuns", "No completed runs have been recorded yet.")).Font(FT66Style::Tokens::FontRegular(15)).ColorAndOpacity(AccountMutedText())];
	}
	else
	{
		for (int32 Index = 0; Index < RecentRuns.Num(); ++Index)
		{
			const FT66RecentRunRecord& Run = RecentRuns[Index];
			FHeroData HeroData; FCompanionData CompanionData;
			const FText HeroName = (T66GI && T66GI->GetHeroData(Run.HeroID, HeroData)) ? (Loc ? Loc->GetText_HeroName(Run.HeroID) : HeroData.DisplayName) : FText::FromName(Run.HeroID);
			const FText CompanionName = (T66GI && T66GI->GetCompanionData(Run.CompanionID, CompanionData)) ? (Loc ? Loc->GetText_CompanionName(Run.CompanionID) : CompanionData.DisplayName) : (Run.CompanionID.IsNone() ? NSLOCTEXT("T66.Account", "NoComp", "No Companion") : FText::FromName(Run.CompanionID));
			const FText ResultText = Run.bWasFullClear ? NSLOCTEXT("T66.Account", "Win", "Win") : NSLOCTEXT("T66.Account", "Defeat", "Defeat");
			const FLinearColor ResultColor = Run.bWasFullClear ? AccountSuccess() : AccountDanger();
			const bool bCanOpen = !Run.RunSummarySlotName.IsEmpty();
			HistoryRows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(FText::GetEmpty(), bCanOpen ? FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, Run.RunSummarySlotName) : FOnClicked::CreateLambda([]() { return FReply::Handled(); }), ET66ButtonType::Row)
					.SetBorderVisual(ET66ButtonBorderVisual::None).SetBackgroundVisual(ET66ButtonBackgroundVisual::None).SetPadding(FMargin(0.f)).SetEnabled(TAttribute<bool>(bCanOpen)).SetUseGlow(false)
					.SetContent(
						MakeAccountPanel(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.3f)
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(HeroName).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountText())]
								+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)[SNew(STextBlock).Text(CompanionName).Font(FT66Style::Tokens::FontRegular(12)).ColorAndOpacity(AccountMutedText())]
							]
							+ SHorizontalBox::Slot().FillWidth(0.8f).VAlign(VAlign_Center)[SNew(STextBlock).Text(ResultText).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(ResultColor)]
							+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::Format(NSLOCTEXT("T66.Account", "DiffPartyFmt", "{0} / {1}"), DifficultyText(Run.Difficulty), PartySizeText(Loc, Run.PartySize))).Font(FT66Style::Tokens::FontRegular(13)).ColorAndOpacity(AccountText())]
							+ SHorizontalBox::Slot().FillWidth(0.7f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::AsNumber(Run.Score)).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountGold())]
							+ SHorizontalBox::Slot().FillWidth(0.6f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::AsNumber(Run.StageReached)).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountText())]
							+ SHorizontalBox::Slot().FillWidth(0.7f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FormatDurationText(Run.DurationSeconds)).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountText())]
							+ SHorizontalBox::Slot().FillWidth(0.9f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::FromString(Run.EndedAtUtc.ToString(TEXT("%m/%d/%Y %H:%M")))).Font(FT66Style::Tokens::FontRegular(12)).ColorAndOpacity(AccountMutedText())],
							ET66PanelType::Panel2, Index % 2 == 0 ? AccountRowFill() : AccountRowAltFill(), FMargin(12.f, 10.f, 12.f, 8.f))))
			];
		}
	}

	auto MakePBBlock = [&](const FText& Title, bool bTime) -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
		for (ET66Difficulty Difficulty : { ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible })
		{
			const FPersonalBestDisplay PB = bTime ? MakePBTime(Difficulty) : MakePBScore(Difficulty);
			const bool bCanOpen = PB.bHasRecord && !PB.RunSummarySlotName.IsEmpty();
			const FText Value = PB.bHasRecord ? (bTime ? FormatDurationText(PB.Seconds) : FText::AsNumber(PB.Score)) : NSLOCTEXT("T66.Account", "NoRecord", "No Record");
			const FText Date = PB.bHasRecord && PB.AchievedAtUtc.GetTicks() > 0 ? FText::FromString(PB.AchievedAtUtc.ToString(TEXT("%m/%d/%Y"))) : FText::GetEmpty();
			Rows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				FT66Style::MakeButton(
					FT66ButtonParams(FText::GetEmpty(), bCanOpen ? FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleOpenRunSummaryClicked, PB.RunSummarySlotName) : FOnClicked::CreateLambda([]() { return FReply::Handled(); }), ET66ButtonType::Row)
					.SetBorderVisual(ET66ButtonBorderVisual::None).SetBackgroundVisual(ET66ButtonBackgroundVisual::None).SetPadding(FMargin(0.f)).SetEnabled(TAttribute<bool>(bCanOpen)).SetUseGlow(false)
					.SetContent(
						MakeAccountPanel(
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)[SNew(STextBlock).Text(DifficultyText(Difficulty)).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountText())]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 8.f, 0.f)[SNew(SBox).Visibility(PB.bHasRecord && ActivePBMode == EPBMode::Party ? EVisibility::Visible : EVisibility::Collapsed)[MakeChip(PartySizeText(Loc, PB.PartySize), AccountMutedGold())]]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.f, 0.f, 16.f, 0.f)[SNew(STextBlock).Text(Date).Font(FT66Style::Tokens::FontRegular(12)).ColorAndOpacity(AccountMutedText())]
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(STextBlock).Text(Value).Font(FT66Style::Tokens::FontBold(18)).ColorAndOpacity(PB.bHasRecord ? AccountGold() : AccountMutedText())],
							ET66PanelType::Panel2, AccountRowFill(), FMargin(12.f, 10.f, 12.f, 8.f))))
			];
		}
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[SNew(STextBlock).Text(Title).Font(FT66Style::Tokens::FontBold(17)).ColorAndOpacity(AccountText())]
			+ SVerticalBox::Slot().AutoHeight()[Rows],
			ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(12.f));
	};

	auto MakeFeatured = [&](const FText& Title, const TArray<FMainEntry>& Entries) -> TSharedRef<SWidget>
	{
		const bool bHas = Entries.Num() > 0;
		const FMainEntry E = bHas ? Entries[0] : FMainEntry();
		return MakeAccountPanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[MakeSectionHeader(Title)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
			[
				SNew(SBox)
				.Visibility(bHas ? EVisibility::Visible : EVisibility::Collapsed)
				[
					MakeAccountPanel(
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(E.Accent).Padding(FMargin(16.f, 12.f))[SNew(STextBlock).Text(E.DisplayName).Font(FT66Style::Tokens::FontBold(24)).ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))]]
						+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 10.f)[SNew(STextBlock).Text(MedalText(E.Medal)).Font(FT66Style::Tokens::FontBold(28)).ColorAndOpacity(MedalColor(E.Medal))]
						+ SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1.f)[MakeMetric(NSLOCTEXT("T66.Account", "Unity", "Unity"), FText::AsNumber(E.Unity), AccountGold())] + SHorizontalBox::Slot().FillWidth(1.f)[MakeMetric(NSLOCTEXT("T66.Account", "Games", "Games Played"), FText::AsNumber(E.GamesPlayed), AccountText())]],
						ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(16.f))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[SNew(SBox).Visibility(bHas ? EVisibility::Collapsed : EVisibility::Visible)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "NoData", "No data recorded yet.")).Font(FT66Style::Tokens::FontRegular(14)).ColorAndOpacity(AccountMutedText())]],
			ET66PanelType::Panel, AccountPanelFill(), FMargin(18.f));
	};

	auto MakeList = [&](const FText& Title, const TArray<FMainEntry>& Entries) -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
		Rows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeSectionHeader(Title)];
		if (Entries.Num() <= 1)
		{
			Rows->AddSlot().AutoHeight()[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "NoExtra", "No additional records yet.")).Font(FT66Style::Tokens::FontRegular(14)).ColorAndOpacity(AccountMutedText())];
		}
		else
		{
			for (int32 Index = 1; Index < Entries.Num(); ++Index)
			{
				const FMainEntry& E = Entries[Index];
				Rows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					MakeAccountPanel(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(E.Accent).Padding(FMargin(8.f, 5.f))[SNew(STextBlock).Text(FText::AsNumber(Index + 1)).Font(FT66Style::Tokens::FontBold(12)).ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))]]
						+ SHorizontalBox::Slot().FillWidth(1.2f).VAlign(VAlign_Center)[SNew(STextBlock).Text(E.DisplayName).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountText())]
						+ SHorizontalBox::Slot().FillWidth(0.7f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::AsNumber(E.Unity)).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountGold())]
						+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)[SNew(STextBlock).Text(MedalText(E.Medal)).Font(FT66Style::Tokens::FontBold(13)).ColorAndOpacity(MedalColor(E.Medal))]
						+ SHorizontalBox::Slot().FillWidth(0.8f).VAlign(VAlign_Center)[SNew(STextBlock).Text(FText::AsNumber(E.GamesPlayed)).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountText())],
						ET66PanelType::Panel2, AccountRowFill(), FMargin(12.f, 10.f, 12.f, 8.f))
				];
			}
		}
		return MakeAccountPanel(Rows, ET66PanelType::Panel, AccountPanelFill(), FMargin(18.f));
	};

	TSharedRef<SWidget> OverviewContent =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.58f).Padding(0.f, 0.f, 16.f, 0.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 16.f)
			[
				MakeAccountPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "StatusHeader", "ACCOUNT STATUS"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)[SNew(STextBlock).Text(RestrictionText(Restriction.Restriction)).Font(FT66Style::Tokens::FontBold(28)).ColorAndOpacity(bAccountEligible ? AccountSuccess() : AccountDanger())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[SNew(STextBlock).Text(bAccountEligible ? NSLOCTEXT("T66.Account", "EligibleBody", "Your account is eligible for ranked tracking and personal best progression.") : NSLOCTEXT("T66.Account", "RestrictedBody", "This account is currently blocked from ranked tracking.")).Font(FT66Style::Tokens::FontRegular(14)).ColorAndOpacity(AccountText()).AutoWrapText(true)]
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(AppealStatusText(Restriction.AppealStatus)).Font(FT66Style::Tokens::FontBold(13)).ColorAndOpacity(AccountMutedText())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 10.f, 0.f, 0.f)[SNew(SBox).Visibility(!Restriction.RestrictionReason.IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed)[MakeAccountPanel(SNew(STextBlock).Text(FText::FromString(Restriction.RestrictionReason)).Font(FT66Style::Tokens::FontRegular(13)).ColorAndOpacity(AccountText()).AutoWrapText(true), ET66PanelType::Panel2, AccountPanelInnerFill(), FMargin(12.f))]]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)[SNew(SBox).Visibility(LB && LB->HasAccountRestrictionRunSummary() ? EVisibility::Visible : EVisibility::Collapsed)[FT66Style::MakeButton(FT66ButtonParams(NSLOCTEXT("T66.Account", "ViewReviewed", "VIEW REVIEWED RUN"), FOnClicked::CreateLambda([this, LB]() { if (LB && LB->RequestOpenAccountRestrictionRunSummary()) { ShowModal(ET66ScreenType::RunSummary); } return FReply::Handled(); }), ET66ButtonType::Primary).SetBorderVisual(ET66ButtonBorderVisual::None).SetBackgroundVisual(ET66ButtonBackgroundVisual::None).SetFontSize(13).SetMinWidth(230.f).SetHeight(38.f).SetPadding(FMargin(14.f, 8.f, 14.f, 6.f)).SetColor(AccountGold()).SetTextColor(FLinearColor(0.08f, 0.07f, 0.05f, 1.0f)).SetUseGlow(false))]],
					ET66PanelType::Panel, AccountPanelFill(), FMargin(18.f))
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				MakeAccountPanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "ProgressHeader", "ACCOUNT PROGRESS"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "AchProg", "Achievements Unlocked"), UnlockedAchievements, AchievementDefs.Num(), AccountGold())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "PowerProg", "Power-Ups Unlocked"), UnlockedPowerUps, PowerStats.Num() * UT66PowerUpSubsystem::MaxFillStepsPerStat, FLinearColor(0.76f, 0.63f, 0.30f, 1.0f))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)[MakeProgressRow(NSLOCTEXT("T66.Account", "HeroProg", "Heroes Unlocked"), HeroIDs.Num(), HeroIDs.Num(), FLinearColor(0.68f, 0.78f, 0.92f, 1.0f))]
					+ SVerticalBox::Slot().AutoHeight()[MakeProgressRow(NSLOCTEXT("T66.Account", "CompProg", "Companions Unlocked"), UnlockedCompanions, CompanionIDs.Num(), FLinearColor(0.57f, 0.84f, 0.79f, 1.0f))],
					ET66PanelType::Panel, AccountPanelFill(), FMargin(18.f))
			]
		]
		+ SHorizontalBox::Slot().FillWidth(0.42f)
		[
			MakeAccountPanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "PBHeader", "PERSONAL BESTS"))] + SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 8.f, 0.f)[MakeTabButton(Loc ? Loc->GetText_Solo() : NSLOCTEXT("T66.Account", "Solo", "SOLO"), ActivePBMode == EPBMode::Solo, &UT66AccountStatusScreen::HandleSoloPBModeClicked)] + SHorizontalBox::Slot().AutoWidth()[MakeTabButton(NSLOCTEXT("T66.Account", "Party", "PARTY"), ActivePBMode == EPBMode::Party, &UT66AccountStatusScreen::HandlePartyPBModeClicked)]]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 12.f)[MakePBBlock(NSLOCTEXT("T66.Account", "TopScore", "Highest Score"), false)]
				+ SVerticalBox::Slot().AutoHeight()[MakePBBlock(NSLOCTEXT("T66.Account", "TopTime", "Fastest Completed Time"), true)],
				ET66PanelType::Panel, AccountPanelFill(), FMargin(18.f))
		];

	TSharedRef<SWidget> HistoryContent = MakeAccountPanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)[MakeSectionHeader(NSLOCTEXT("T66.Account", "HistoryHdr", "RECENT RUNS"))]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "HistorySub", "Click any run to open its saved summary.")).Font(FT66Style::Tokens::FontRegular(14)).ColorAndOpacity(AccountMutedText())]
		+ SVerticalBox::Slot().AutoHeight()[HistoryRows],
		ET66PanelType::Panel, AccountPanelFill(), FMargin(18.f));

	TSharedRef<SWidget> MainsContent =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.38f).Padding(0.f, 0.f, 16.f, 0.f)[MakeFeatured(NSLOCTEXT("T66.Account", "FeatHero", "FEATURED HERO"), HeroMains)]
			+ SHorizontalBox::Slot().FillWidth(0.62f)[MakeList(NSLOCTEXT("T66.Account", "HeroList", "HERO RANKINGS"), HeroMains)]
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.38f).Padding(0.f, 0.f, 16.f, 0.f)[MakeFeatured(NSLOCTEXT("T66.Account", "FeatComp", "FEATURED COMPANION"), CompanionMains)]
			+ SHorizontalBox::Slot().FillWidth(0.62f)[MakeList(NSLOCTEXT("T66.Account", "CompList", "COMPANION RANKINGS"), CompanionMains)]
		];

	const TSharedRef<SWidget> ActiveContent = ActiveTab == EAccountTab::History ? HistoryContent : (ActiveTab == EAccountTab::Mains ? MainsContent : OverviewContent);
	const FText Title = Loc ? Loc->GetText_AccountStatus() : NSLOCTEXT("T66.Account", "Title", "ACCOUNT");
	const FText Subtitle = ActiveTab == EAccountTab::History ? NSLOCTEXT("T66.Account", "SubHistory", "Track recent runs and open any saved run summary.") : (ActiveTab == EAccountTab::Mains ? NSLOCTEXT("T66.Account", "SubMains", "See your most trusted heroes and companions ranked by real usage.") : NSLOCTEXT("T66.Account", "SubOverview", "Account standing, progression, and personal bests in one place."));
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	TSharedRef<SWidget> Content =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 14.f)
		[
			MakeAccountPanel(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(Title).Font(FT66Style::Tokens::FontBold(40)).ColorAndOpacity(AccountText())]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 4.f, 0.f, 0.f)[SNew(STextBlock).Text(Subtitle).Font(FT66Style::Tokens::FontRegular(15)).ColorAndOpacity(AccountMutedText())]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(STextBlock).Text(NSLOCTEXT("T66.Account", "Trail", "ACCOUNT / ANALYTICS / RECORDS")).Font(FT66Style::Tokens::FontBold(14)).ColorAndOpacity(AccountGold())],
				ET66PanelType::Panel2, AccountHeaderFill(), FMargin(18.f, 14.f, 18.f, 12.f))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 18.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[MakeTabButton(NSLOCTEXT("T66.Account", "OverviewTab", "OVERVIEW"), ActiveTab == EAccountTab::Overview, &UT66AccountStatusScreen::HandleOverviewTabClicked)]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 10.f, 0.f)[MakeTabButton(NSLOCTEXT("T66.Account", "HistoryTab", "HISTORY"), ActiveTab == EAccountTab::History, &UT66AccountStatusScreen::HandleHistoryTabClicked)]
			+ SHorizontalBox::Slot().AutoWidth()[MakeTabButton(NSLOCTEXT("T66.Account", "MainsTab", "MAINS"), ActiveTab == EAccountTab::Mains, &UT66AccountStatusScreen::HandleMainsTabClicked)]
		]
		+ SVerticalBox::Slot().FillHeight(1.f)[SNew(SScrollBox) + SScrollBox::Slot()[ActiveContent]]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 18.f, 0.f, 0.f)
		[
			SNew(SBox)
			.Visibility(bModalPresentation ? EVisibility::Visible : EVisibility::Collapsed)
			[
				FT66Style::MakeButton(FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66AccountStatusScreen::HandleBackClicked), ET66ButtonType::Neutral).SetBorderVisual(ET66ButtonBorderVisual::None).SetBackgroundVisual(ET66ButtonBackgroundVisual::None).SetFontSize(16).SetMinWidth(150.f).SetHeight(40.f).SetPadding(FMargin(16.f, 8.f, 16.f, 6.f)).SetColor(AccountPanelInnerFill()).SetTextColor(AccountText()).SetUseGlow(false))
			]
		];

	const TSharedRef<SWidget> Frame = SNew(SBox).MaxDesiredWidth(1500.f)[MakeAccountPanel(Content, ET66PanelType::Panel, AccountPanelFill(), FMargin(22.f))];
	if (bModalPresentation)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FT66Style::Scrim())]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(32.f, 40.f))[Frame];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FT66Style::ScreenBackground())
		[
			SNew(SBox)
			.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(FMargin(24.f, 22.f)).HAlign(HAlign_Center)[Frame]
			]
		];
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

FReply UT66AccountStatusScreen::HandleMainsTabClicked()
{
	ActiveTab = EAccountTab::Mains;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandleSoloPBModeClicked()
{
	ActivePBMode = EPBMode::Solo;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66AccountStatusScreen::HandlePartyPBModeClicked()
{
	ActivePBMode = EPBMode::Party;
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
