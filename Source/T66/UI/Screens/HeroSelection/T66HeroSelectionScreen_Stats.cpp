// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

#include "Styling/CoreStyle.h"

using namespace T66HeroSelectionPrivate;

namespace
{
	struct FHeroSelectionRecordInfoIconEntry
	{
		TStrongObjectPtr<UTexture2D> Texture;
		TSharedPtr<FSlateBrush> Brush;
	};

	TSharedRef<SWidget> MakeHeroSelectionRecordInfoPanel()
	{
		const FSlateFontInfo HeaderFont = FT66Style::Tokens::FontBold(14);
		const FSlateFontInfo BodyFont = FT66Style::Tokens::FontRegular(11);
		const FSlateFontInfo BoldFont = FT66Style::Tokens::FontBold(11);
		const FVector2D TierIconSize(24.f, 24.f);

		auto GetTierBrush = [TierIconSize](const ET66AccountMedalTier Tier) -> const FSlateBrush*
		{
			static TMap<int32, FHeroSelectionRecordInfoIconEntry> Icons;
			const int32 Key = static_cast<int32>(Tier);
			FHeroSelectionRecordInfoIconEntry& Entry = Icons.FindOrAdd(Key);
			if (!Entry.Brush.IsValid())
			{
				Entry.Brush = MakeShared<FSlateBrush>();
				Entry.Brush->DrawAs = ESlateBrushDrawType::Image;
				Entry.Brush->Tiling = ESlateBrushTileType::NoTile;
				Entry.Brush->TintColor = FSlateColor(FLinearColor::White);
				Entry.Brush->ImageSize = TierIconSize;
			}
			if (!Entry.Texture.IsValid())
			{
				const FString Path = GetHeroSelectionMedalImagePath(Tier);
				if (FPaths::FileExists(Path))
				{
					if (UTexture2D* LoadedTexture = T66RuntimeUITextureAccess::ImportFileTexture(
						Path,
						TextureFilter::TF_Trilinear,
						true,
						TEXT("HeroSelectionRecordInfoMedal")))
					{
						Entry.Texture.Reset(LoadedTexture);
					}
					else if (UTexture2D* LoadedTextureWithMips = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
						Path,
						TextureFilter::TF_Trilinear,
						TEXT("HeroSelectionRecordInfoMedal")))
					{
						Entry.Texture.Reset(LoadedTextureWithMips);
					}
				}
			}

			Entry.Brush->SetResourceObject(Entry.Texture.IsValid() ? Entry.Texture.Get() : nullptr);
			return Entry.Texture.IsValid() ? Entry.Brush.Get() : nullptr;
		};

		auto MakeTierRow = [&](const ET66AccountMedalTier Tier, const FText& MedalText, const FText& UnlockText) -> TSharedRef<SWidget>
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(TierIconSize.X)
					.HeightOverride(TierIconSize.Y)
					[
						SNew(SImage)
						.Image(GetTierBrush(Tier))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.VAlign(VAlign_Center)
				.Padding(6.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(FText::Format(NSLOCTEXT("T66.HeroSelection", "RecordInfoMedalTierLine", "{0}: {1}"), MedalText, UnlockText))
					.Font(BoldFont)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				];
		};

		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoHeader", "MEDAL / RANK"))
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
						.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoMedalTitle", "Medal"))
						.Font(HeaderFont)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoMedalBody", "Highest difficulty cleared with this hero or companion."))
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
						.Text(NSLOCTEXT("T66.HeroSelection", "RecordInfoRankTitle", "Rank"))
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
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(0.f, 0.f, 8.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[MakeTierRow(ET66AccountMedalTier::None, NSLOCTEXT("T66.HeroSelection", "MedalTierNoneShort", "Unproven"), NSLOCTEXT("T66.HeroSelection", "MedalTierNoneUnlock", "No clear"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[MakeTierRow(ET66AccountMedalTier::Bronze, NSLOCTEXT("T66.HeroSelection", "MedalTierBronzeShort", "Bronze"), NSLOCTEXT("T66.HeroSelection", "MedalTierEasyUnlock", "Easy"))]
					+ SVerticalBox::Slot().AutoHeight()[MakeTierRow(ET66AccountMedalTier::Silver, NSLOCTEXT("T66.HeroSelection", "MedalTierSilverShort", "Silver"), NSLOCTEXT("T66.HeroSelection", "MedalTierMediumUnlock", "Medium"))]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[MakeTierRow(ET66AccountMedalTier::Gold, NSLOCTEXT("T66.HeroSelection", "MedalTierGoldShort", "Gold"), NSLOCTEXT("T66.HeroSelection", "MedalTierHardUnlock", "Hard"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 3.f)[MakeTierRow(ET66AccountMedalTier::Platinum, NSLOCTEXT("T66.HeroSelection", "MedalTierPlatinumShort", "Platinum"), NSLOCTEXT("T66.HeroSelection", "MedalTierVeryHardUnlock", "Very Hard"))]
					+ SVerticalBox::Slot().AutoHeight()[MakeTierRow(ET66AccountMedalTier::Diamond, NSLOCTEXT("T66.HeroSelection", "MedalTierDiamondShort", "Diamond"), NSLOCTEXT("T66.HeroSelection", "MedalTierImpossibleUnlock", "Impossible"))]
				]
			];
	}

	TSharedRef<SWidget> MakeHeroSelectionSummaryStatsList(UT66LeaderboardRunSummarySaveGame* Snapshot, UT66LocalizationSubsystem* Loc)
	{
		const FText StatLineFormat = NSLOCTEXT("T66.HeroSelection", "SummaryStatLineFormatCompact", "{0} {1}/99");

		auto GetPrimaryStatLabel = [Loc](const int32 StatIndex) -> FText
		{
			if (StatIndex == 1)
			{
				return NSLOCTEXT("T66.Stats", "AttackSpeedShort", "ATT Speed");
			}
			if (StatIndex == 2)
			{
				return NSLOCTEXT("T66.Stats", "AttackScaleShort", "ATT Scale");
			}

			if (Loc)
			{
				switch (StatIndex)
				{
				case 0: return Loc->GetText_Stat_Damage();
				case 3: return Loc->GetText_Stat_Accuracy();
				case 4: return Loc->GetText_Stat_Armor();
				case 5: return Loc->GetText_Stat_Evasion();
				case 6: return Loc->GetText_Stat_Luck();
				case 7: return Loc->GetText_Stat_Speed();
				default: break;
				}
			}

			switch (StatIndex)
			{
			case 0: return NSLOCTEXT("T66.Stats", "Damage", "Damage");
			case 3: return NSLOCTEXT("T66.Stats", "Accuracy", "Accuracy");
			case 4: return NSLOCTEXT("T66.Stats", "Armor", "Armor");
			case 5: return NSLOCTEXT("T66.Stats", "Evasion", "Evasion");
			case 6: return NSLOCTEXT("T66.Stats", "Luck", "Luck");
			case 7: return NSLOCTEXT("T66.Stats", "Speed", "Speed");
			default: return FText::GetEmpty();
			}
		};

		auto GetPrimaryStatValue = [Snapshot](const int32 StatIndex) -> int32
		{
			if (!::IsValid(Snapshot))
			{
				return 0;
			}

			switch (StatIndex)
			{
			case 0: return Snapshot->DamageStat;
			case 1: return Snapshot->AttackSpeedStat;
			case 2: return Snapshot->AttackScaleStat;
			case 3: return Snapshot->AccuracyStat;
			case 4: return Snapshot->ArmorStat;
			case 5: return Snapshot->EvasionStat;
			case 6: return Snapshot->LuckStat;
			case 7: return Snapshot->SpeedStat;
			default: return 0;
			}
		};

		auto MakeStatsColumn = [&](const int32 FirstIndex, const int32 LastIndexExclusive) -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);
			for (int32 StatIndex = FirstIndex; StatIndex < LastIndexExclusive; ++StatIndex)
			{
				const int32 StatValue = GetPrimaryStatValue(StatIndex);
				Column->AddSlot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 2.f)
				[
					SNew(STextBlock)
					.Text(FText::Format(
						StatLineFormat,
						GetPrimaryStatLabel(StatIndex),
						FText::AsNumber(StatValue)))
					.Font(FT66Style::Tokens::FontBold(14))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				];
			}
			return Column;
		};

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.08f)
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				MakeStatsColumn(0, 4)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			.Padding(0.f, 1.f, 0.f, 2.f)
			[
				SNew(SBox)
				.WidthOverride(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.78f, 0.80f, 0.88f, 0.42f))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeStatsColumn(4, 8)
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
					.Text(NSLOCTEXT("T66.HeroSelection", "HeroSummaryStatsHeader", "STATS"))
					.Font(FT66Style::Tokens::FontBold(17))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeHeroSelectionSummaryStatsList(HeroStatsSnapshot, Loc)
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
					.Font(FT66Style::Tokens::FontBold(17))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroSummaryStatsHint", "Select a hero to view their stats."))
					.Font(FT66Style::Tokens::FontRegular(14))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
