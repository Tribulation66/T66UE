// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

using namespace T66HeroSelectionPrivate;

FReply UT66HeroSelectionScreen::HandleStatsClicked()
{
	if (bShowingCompanionInfo)
	{
		return FReply::Handled();
	}

	bShowingStatsPanel = !bShowingStatsPanel;
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
	RefreshPanelSwitchers();
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
	EnsureHeroStatsSnapshot();
	if (!HeroStatsSnapshot)
	{
		return;
	}
	UT66LeaderboardRunSummarySaveGame* Snapshot = HeroStatsSnapshot.Get();
	Snapshot->HeroID = HeroData.HeroID;
	Snapshot->HeroLevel = 1;
	Snapshot->DamageStat = BaseStats.Damage + PermanentBuffBonuses.Damage;
	Snapshot->AttackSpeedStat = BaseStats.AttackSpeed + PermanentBuffBonuses.AttackSpeed;
	Snapshot->AttackScaleStat = BaseStats.AttackScale + PermanentBuffBonuses.AttackScale;
	Snapshot->AccuracyStat = BaseStats.Accuracy + PermanentBuffBonuses.Accuracy;
	Snapshot->ArmorStat = BaseStats.Armor + PermanentBuffBonuses.Armor;
	Snapshot->EvasionStat = BaseStats.Evasion + PermanentBuffBonuses.Evasion;
	Snapshot->LuckStat = BaseStats.Luck + PermanentBuffBonuses.Luck;
	Snapshot->SpeedStat = BaseStats.Speed;
	Snapshot->SecondaryStatValues.Reset();

	auto SetSecondaryValue = [Snapshot](const ET66SecondaryStatType Type, const float Value)
	{
		Snapshot->SecondaryStatValues.Add(Type, Value);
	};

	SetSecondaryValue(ET66SecondaryStatType::AoeDamage, static_cast<float>(HeroData.BaseAoeDmg + PermanentBuffBonuses.AoeDmg));
	SetSecondaryValue(ET66SecondaryStatType::BounceDamage, static_cast<float>(HeroData.BaseBounceDmg + PermanentBuffBonuses.BounceDmg));
	SetSecondaryValue(ET66SecondaryStatType::PierceDamage, static_cast<float>(HeroData.BasePierceDmg + PermanentBuffBonuses.PierceDmg));
	SetSecondaryValue(ET66SecondaryStatType::DotDamage, static_cast<float>(HeroData.BaseDotDmg + PermanentBuffBonuses.DotDmg));
	SetSecondaryValue(ET66SecondaryStatType::AoeSpeed, static_cast<float>(HeroData.BaseAoeAtkSpd + PermanentBuffBonuses.AoeAtkSpd));
	SetSecondaryValue(ET66SecondaryStatType::BounceSpeed, static_cast<float>(HeroData.BaseBounceAtkSpd + PermanentBuffBonuses.BounceAtkSpd));
	SetSecondaryValue(ET66SecondaryStatType::PierceSpeed, static_cast<float>(HeroData.BasePierceAtkSpd + PermanentBuffBonuses.PierceAtkSpd));
	SetSecondaryValue(ET66SecondaryStatType::DotSpeed, static_cast<float>(HeroData.BaseDotAtkSpd + PermanentBuffBonuses.DotAtkSpd));
	SetSecondaryValue(ET66SecondaryStatType::AoeScale, static_cast<float>(HeroData.BaseAoeAtkScale + PermanentBuffBonuses.AoeAtkScale));
	SetSecondaryValue(ET66SecondaryStatType::BounceScale, static_cast<float>(HeroData.BaseBounceAtkScale + PermanentBuffBonuses.BounceAtkScale));
	SetSecondaryValue(ET66SecondaryStatType::PierceScale, static_cast<float>(HeroData.BasePierceAtkScale + PermanentBuffBonuses.PierceAtkScale));
	SetSecondaryValue(ET66SecondaryStatType::DotScale, static_cast<float>(HeroData.BaseDotAtkScale + PermanentBuffBonuses.DotAtkScale));
	SetSecondaryValue(ET66SecondaryStatType::CritDamage, HeroData.BaseCritDamage);
	SetSecondaryValue(ET66SecondaryStatType::CritChance, HeroData.BaseCritChance);
	SetSecondaryValue(ET66SecondaryStatType::AttackRange, HeroData.BaseAttackRange);
	SetSecondaryValue(ET66SecondaryStatType::Accuracy, HeroData.BaseAccuracy);
	SetSecondaryValue(ET66SecondaryStatType::Taunt, HeroData.BaseTaunt);
	SetSecondaryValue(ET66SecondaryStatType::DamageReduction, 0.f);
	SetSecondaryValue(ET66SecondaryStatType::ReflectDamage, HeroData.BaseReflectDmg);
	SetSecondaryValue(ET66SecondaryStatType::Crush, HeroData.BaseCrushChance);
	SetSecondaryValue(ET66SecondaryStatType::EvasionChance, 0.f);
	SetSecondaryValue(ET66SecondaryStatType::CounterAttack, HeroData.BaseCounterAttack);
	SetSecondaryValue(ET66SecondaryStatType::Invisibility, HeroData.BaseInvisChance);
	SetSecondaryValue(ET66SecondaryStatType::Assassinate, HeroData.BaseAssassinateChance);
	SetSecondaryValue(ET66SecondaryStatType::TreasureChest, 0.f);
	SetSecondaryValue(ET66SecondaryStatType::Cheating, HeroData.BaseCheatChance);
	SetSecondaryValue(ET66SecondaryStatType::Stealing, HeroData.BaseStealChance);
	SetSecondaryValue(ET66SecondaryStatType::LootCrate, 0.f);
}

void UT66HeroSelectionScreen::RefreshHeroStatsPanels()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	if (HeroSummaryStatsHost.IsValid())
	{
		if (HeroStatsSnapshot)
		{
			T66StatsPanelSlate::FT66SnapshotStatsPanelOptions SummaryOptions;
			SummaryOptions.WidthOverride = 0.f;
			SummaryOptions.bExtended = false;
			SummaryOptions.bShowAdjectiveSummaries = false;
			SummaryOptions.bIncludeHeader = false;
			SummaryOptions.bIncludeLevel = false;
			SummaryOptions.FontSizeAdjustment = -12;
			HeroSummaryStatsHost->SetContent(T66StatsPanelSlate::MakeEssentialStatsPanelFromSnapshotWithOptions(HeroStatsSnapshot, Loc, SummaryOptions));
		}
		else
		{
			HeroSummaryStatsHost->SetContent(
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroSummaryStatsHint", "Select a hero to view their stats."))
				.Font(FT66Style::Tokens::FontRegular(14))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true));
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
			FullOptions.bIncludeLevel = true;
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
