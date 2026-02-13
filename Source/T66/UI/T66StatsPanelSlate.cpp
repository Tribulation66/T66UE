// Copyright Tribulation 66. All Rights Reserved.

#include "T66StatsPanelSlate.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

/** Max height of the scrollable stats list (primary + secondary); scroll bar appears when content exceeds this. */
static constexpr float StatsScrollMaxHeight = 420.f;

TSharedRef<SWidget> T66StatsPanelSlate::MakeEssentialStatsPanel(
	UT66RunStateSubsystem* RunState,
	UT66LocalizationSubsystem* Loc,
	float WidthOverride,
	bool bExtended)
{
	const FText HeaderText = NSLOCTEXT("T66.StatsPanel", "Header", "STATS");
	const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");

	auto GetLabel = [Loc](int32 Index) -> FText
	{
		if (!Loc) return FText::FromString(TEXT("?"));
		switch (Index)
		{
			case 0: return Loc->GetText_Level();
			case 1: return Loc->GetText_Stat_Damage();
			case 2: return Loc->GetText_Stat_AttackSpeed();
			case 3: return Loc->GetText_Stat_AttackScale();
			case 4: return Loc->GetText_Stat_Armor();
			case 5: return Loc->GetText_Stat_Evasion();
			case 6: return Loc->GetText_Stat_Luck();
			case 7: return Loc->GetText_Stat_Speed();
			default: return FText::FromString(TEXT("?"));
		}
	};

	TSharedRef<SVerticalBox> StatsBox = SNew(SVerticalBox);

	if (RunState)
	{
		const int32 HeroLevel = RunState->GetHeroLevel();
		const int32 DamageStat = RunState->GetDamageStat();
		const int32 AttackSpeedStat = RunState->GetAttackSpeedStat();
		const int32 AttackScaleStat = RunState->GetScaleStat();
		const int32 ArmorStat = RunState->GetArmorStat();
		const int32 EvasionStat = RunState->GetEvasionStat();
		const int32 LuckStat = RunState->GetLuckStat();
		const int32 SpeedStat = RunState->GetSpeedStat();

		auto AddStatLine = [&](const FText& Label, int32 Value)
		{
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(StatFmt, Label, FText::AsNumber(Value)))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
		};

		auto AddStatLineFloat = [&](const FText& Label, float Value, bool bAsPercent)
		{
			FText ValueText = bAsPercent
				? FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.f)))
				: FText::FromString(FString::Printf(TEXT("%.1f"), Value));
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(StatFmt, Label, ValueText))
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
		};

		AddStatLine(GetLabel(0), HeroLevel);
		AddStatLine(GetLabel(1), DamageStat);
		AddStatLine(GetLabel(2), AttackSpeedStat);
		AddStatLine(GetLabel(3), AttackScaleStat);
		AddStatLine(GetLabel(4), ArmorStat);
		AddStatLine(GetLabel(5), EvasionStat);
		AddStatLine(GetLabel(6), LuckStat);
		AddStatLine(GetLabel(7), SpeedStat);

		if (bExtended)
		{
			// All secondary stats (extended list for Vendor/Gambler)
			UEnum* SecEnum = StaticEnum<ET66SecondaryStatType>();
			if (SecEnum && Loc)
			{
				const int32 NumSec = static_cast<int32>(SecEnum->GetMaxEnumValue()) + 1;
				for (int32 i = 1; i < NumSec; ++i)
				{
					const ET66SecondaryStatType SecType = static_cast<ET66SecondaryStatType>(i);
					const FText Label = Loc->GetText_SecondaryStatName(SecType);
					const float Value = RunState->GetSecondaryStatValue(SecType);
					const bool bPercent = (SecType == ET66SecondaryStatType::CritChance || SecType == ET66SecondaryStatType::Crush
						|| SecType == ET66SecondaryStatType::Invisibility || SecType == ET66SecondaryStatType::LifeSteal
						|| SecType == ET66SecondaryStatType::Assassinate || SecType == ET66SecondaryStatType::Cheating
						|| SecType == ET66SecondaryStatType::Stealing);
					AddStatLineFloat(Label, Value, bPercent);
				}
			}
		}
	}

	TSharedRef<SWidget> StatsContent = bExtended
		? TSharedRef<SWidget>(SNew(SBox)
			.HeightOverride(StatsScrollMaxHeight)
			[
				SNew(SScrollBox)
				.ScrollBarVisibility(EVisibility::Visible)
				+ SScrollBox::Slot()
				[
					StatsBox
				]
			])
		: TSharedRef<SWidget>(StatsBox);

	TSharedRef<SWidget> Content = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(HeaderText)
			.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			StatsContent
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SSpacer)
		];

	return SNew(SBox)
		.WidthOverride(WidthOverride)
		[
			FT66Style::MakePanel(
				Content,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4))
		];
}
