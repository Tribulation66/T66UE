// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

#include "Styling/CoreStyle.h"

using namespace T66HeroSelectionPrivate;

namespace
{
	void ResolveHeroBestStats(const FHeroData& HeroData, TArray<ET66StatType>& OutBestStats)
	{
		OutBestStats.Reset();
		auto AddUnique = [&OutBestStats](const ET66StatType StatType)
		{
			if (StatType != ET66StatType::None && !OutBestStats.Contains(StatType))
			{
				OutBestStats.Add(StatType);
			}
		};

		// Authored "Best stats" win; fall back to the hero's primary attack-category triple.
		AddUnique(HeroData.BestStat1);
		AddUnique(HeroData.BestStat2);
		AddUnique(HeroData.BestStat3);

		if (OutBestStats.Num() < 3)
		{
			switch (HeroData.PrimaryCategory)
			{
			case ET66AttackCategory::AOE:
				AddUnique(ET66StatType::AoeDamage); AddUnique(ET66StatType::AoeScale); AddUnique(ET66StatType::AoeSpeed); break;
			case ET66AttackCategory::Bounce:
				AddUnique(ET66StatType::BounceDamage); AddUnique(ET66StatType::BounceScale); AddUnique(ET66StatType::BounceSpeed); break;
			case ET66AttackCategory::DOT:
				AddUnique(ET66StatType::DotDamage); AddUnique(ET66StatType::DotScale); AddUnique(ET66StatType::DotSpeed); break;
			case ET66AttackCategory::Pierce:
			default:
				AddUnique(ET66StatType::PierceDamage); AddUnique(ET66StatType::PierceScale); AddUnique(ET66StatType::PierceSpeed); break;
			}
		}

		if (OutBestStats.Num() > 3)
		{
			OutBestStats.SetNum(3);
		}
	}

	FText GetHeroSelectionStatDisplayText(const ET66StatType StatType)
	{
		if (const UEnum* EnumPtr = StaticEnum<ET66StatType>())
		{
			return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(StatType));
		}
		return FText::GetEmpty();
	}

	TSharedRef<SWidget> MakeHeroSelectionBestStatsList(const TArray<ET66StatType>& BestStats)
	{
		TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);
		for (const ET66StatType StatType : BestStats)
		{
			Column->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBox)
				.HeightOverride(30.f)
				.HAlign(HAlign_Center)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					MakeHeroSelectionFittedLabel(
						GetHeroSelectionStatDisplayText(StatType),
						23,
						GetHeroSelectionParchmentText(),
						ETextJustify::Center,
						HAlign_Center)
				]
			];
		}
		return Column;
	}

	TSharedRef<SWidget> MakeHeroSelectionRecordInfoPanel()
	{
		const FSlateFontInfo HeaderFont = FT66Style::Tokens::FontBold(14);
		const FSlateFontInfo BodyFont = FT66Style::Tokens::FontRegular(11);

		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoHeader", "HERO MASTERY / GLOBAL RANK"))
				.Font(FT66Style::Tokens::FontBold(18))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 7.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(0.f, 0.f, 10.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 3.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoMaestryTitle", "Hero Mastery"))
						.Font(HeaderFont)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoMaestryBody", "Hero experience earned by playing this hero. The bar fills toward the next mastery level."))
						.Font(BodyFont)
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 3.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoRankTitle", "Global Rank"))
						.Font(HeaderFont)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoRankBody", "All-time score placement for the selected difficulty, party size, and hero."))
						.Font(BodyFont)
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.AutoWrapText(true)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				FT66FlatStyle::MakeFlatProgressBar(TAttribute<float>(0.f))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoMaestryHint", "LV 1  0 / 100 XP"))
				.Font(FT66Style::Tokens::FontBold(12))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
	}

}

FReply UT66HeroSelectionScreen::HandleStatsClicked()
{
	if (bShowingCompanionInfo)
	{
		return FReply::Handled();
	}

	bShowingStatsPanel = !bShowingStatsPanel;
	bShowingHeroRecordInfoPanel = false;
	RefreshPanelSwitchers();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleOpenStatsPanelClicked()
{
	if (bShowingCompanionInfo)
	{
		return FReply::Handled();
	}

	bShowingStatsPanel = true;
	bShowingHeroRecordInfoPanel = false;
	RefreshPanelSwitchers();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleHeroRecordInfoClicked()
{
	bShowingHeroRecordInfoPanel = !bShowingHeroRecordInfoPanel;
	RefreshHeroStatsPanels();
	return FReply::Handled();
}

void UT66HeroSelectionScreen::EnsureHeroStatsSnapshot()
{
	if (!HeroStatsSnapshot)
	{
		HeroStatsSnapshot = NewObject<UT66LeaderboardRunSummarySaveGame>(this, UT66LeaderboardRunSummarySaveGame::StaticClass(), NAME_None, RF_Transient);
	}
}

void UT66HeroSelectionScreen::PopulateHeroStatsSnapshot(const FHeroData& HeroData, const FT66HeroStatBlock& BaseStats, const FT66HeroStatBonuses& PermanentBuffBonuses)
{
	static_cast<void>(PermanentBuffBonuses);
	EnsureHeroStatsSnapshot();
	if (!HeroStatsSnapshot)
	{
		return;
	}
	UT66LeaderboardRunSummarySaveGame* Snapshot = HeroStatsSnapshot.Get();
	Snapshot->HeroID = HeroData.HeroID;
	Snapshot->HeroLevel = 1;
	Snapshot->DamageStat = BaseStats.Damage;
	Snapshot->AttackSpeedStat = BaseStats.AttackSpeed;
	Snapshot->AttackScaleStat = BaseStats.AttackScale;
	Snapshot->AccuracyStat = BaseStats.Accuracy;
	Snapshot->ArmorStat = BaseStats.Armor;
	Snapshot->EvasionStat = BaseStats.Evasion;
	Snapshot->LuckStat = BaseStats.Luck;
	Snapshot->SpeedStat = BaseStats.Speed;
	// Stats Rework: StatValues stores accumulated bonus points (unified "+X%" rows), not hero base values.
	// A not-yet-started run has no accumulated bonuses, so leave the map empty — the snapshot panel renders
	// missing entries as "+0%" (hero-select passes bExtended=true explicitly) and derives Armor Reduction /
	// Dodge Chance from the base stats above.
	Snapshot->StatValues.Reset();

	ResolveHeroBestStats(HeroData, CurrentHeroBestStats);
}

void UT66HeroSelectionScreen::RefreshHeroStatsPanels()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	if (HeroSummaryStatsHost.IsValid())
	{
		if (bShowingHeroRecordInfoPanel)
		{
			HeroSummaryStatsHost->SetContent(MakeHeroSelectionRecordInfoPanel());
		}
		else if (HeroStatsSnapshot)
		{
			HeroSummaryStatsHost->SetContent(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 5.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "HeroSummaryBestStatsHeader", "BEST STATS"))
				.Font(FT66Style::Tokens::FontBold(28))
					.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
					.Justification(ETextJustify::Center)
					.Clipping(EWidgetClipping::ClipToBounds)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeHeroSelectionBestStatsList(CurrentHeroBestStats)
				]);
		}
		else
		{
			HeroSummaryStatsHost->SetContent(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 5.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "HeroSummaryStatsHeader", "STATS"))
					.Font(FT66Style::Tokens::FontBold(22))
					.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroSummaryStatsHint", "Select a hero to view their stats."))
					.Font(FT66Style::Tokens::FontRegular(17))
					.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
					.AutoWrapText(true)
				]);
		}
	}

	if (HeroFullStatsHost.IsValid())
	{
		if (HeroStatsSnapshot)
		{
			T66StatsPanelSlate::FT66SnapshotStatsPanelOptions FullOptions;
			FullOptions.WidthOverride = 0.f;
			FullOptions.bExtended = true;
			FullOptions.bShowAdjectiveSummaries = false;
			FullOptions.bIncludeHeader = true;
			FullOptions.bIncludeLevel = false;
			FullOptions.FontSizeAdjustment = -4;
			FullOptions.HeadingFontSizeAdjustment = -4;
			FullOptions.ScrollHeightOverride = FT66Style::Tokens::NPCStatsPanelContentHeight + 48.f;
			HeroFullStatsHost->SetContent(T66StatsPanelSlate::MakeEssentialStatsPanelFromSnapshotWithOptions(HeroStatsSnapshot, Loc, FullOptions));
		}
		else
		{
			HeroFullStatsHost->SetContent(
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroFullStatsHint", "Select a hero to view their full stats."))
				.Font(FT66Style::Tokens::FontRegular(13))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true));
		}
	}
}
