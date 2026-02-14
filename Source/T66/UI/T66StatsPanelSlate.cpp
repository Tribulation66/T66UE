// Copyright Tribulation 66. All Rights Reserved.

#include "T66StatsPanelSlate.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/Style/T66Style.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

/** Per-category stat indices (ET66SecondaryStatType enum values). Crit Damage under Damage, Crit Chance under Attack Speed. */
struct FSecondaryStatCategory
{
	FText Header;
	const int32* Indices;
	int32 Num;
};

static const int32 CatDamage[]         = { 1, 2, 3, 4, 13 };  // AOE, Bounce, Pierce, Dot, Crit Damage
static const int32 CatAttackSpeed[]    = { 5, 6, 7, 8, 14 };  // AOE, Bounce, Pierce, Dot Speed, Crit Chance
static const int32 CatAttackScale[]    = { 9, 10, 11, 12 };
static const int32 CatRange[]         = { 15, 16, 17 };      // Close Range, Long Range, Attack Range
static const int32 CatArmor[]         = { 18, 19, 20, 21 };
static const int32 CatEvasion[]       = { 22, 23, 24, 25 };
static const int32 CatLuck[]          = { 26, 27, 28, 29, 30, 31, 32 };
static const int32 CatMovement[]      = { 33 };

static const FSecondaryStatCategory SecondaryStatCategories[] =
{
	{ NSLOCTEXT("T66.StatsPanel", "CatDamage",      "Damage"),         CatDamage,      UE_ARRAY_COUNT(CatDamage) },
	{ NSLOCTEXT("T66.StatsPanel", "CatAttackSpeed", "Attack Speed"),   CatAttackSpeed, UE_ARRAY_COUNT(CatAttackSpeed) },
	{ NSLOCTEXT("T66.StatsPanel", "CatAttackScale", "Attack Scale"),   CatAttackScale, UE_ARRAY_COUNT(CatAttackScale) },
	{ NSLOCTEXT("T66.StatsPanel", "CatRange",       "Range"),          CatRange,       UE_ARRAY_COUNT(CatRange) },
	{ NSLOCTEXT("T66.StatsPanel", "CatArmor",       "Armor"),          CatArmor,       UE_ARRAY_COUNT(CatArmor) },
	{ NSLOCTEXT("T66.StatsPanel", "CatEvasion",     "Evasion"),        CatEvasion,     UE_ARRAY_COUNT(CatEvasion) },
	{ NSLOCTEXT("T66.StatsPanel", "CatLuck",        "Luck"),           CatLuck,        UE_ARRAY_COUNT(CatLuck) },
	{ NSLOCTEXT("T66.StatsPanel", "CatMovement",    "Movement"),      CatMovement,    UE_ARRAY_COUNT(CatMovement) },
};
static constexpr int32 NumSecondaryStatCategories = UE_ARRAY_COUNT(SecondaryStatCategories);

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
			// Secondary stats by category: line, bold header, stat lines, line (Vendor/Gambler)
			const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
			const FLinearColor LineColor(0.35f, 0.38f, 0.42f, 0.9f);
			const FTextBlockStyle& HeadingStyle = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");

			auto AddHorizontalLine = [&]()
			{
				StatsBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 8.f)
				[
					SNew(SBox)
					.HeightOverride(1.f)
					[
						SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(LineColor)
					]
				];
			};

			auto AddCategoryHeader = [&](const FText& Text)
			{
				StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
					.Text(Text)
					.TextStyle(&HeadingStyle)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				];
			};

			for (int32 c = 0; c < NumSecondaryStatCategories; ++c)
			{
				const FSecondaryStatCategory& Cat = SecondaryStatCategories[c];
				AddHorizontalLine();
				AddCategoryHeader(Cat.Header);
				for (int32 k = 0; k < Cat.Num; ++k)
				{
					const int32 i = Cat.Indices[k];
					const ET66SecondaryStatType SecType = static_cast<ET66SecondaryStatType>(i);
					const FText Label = Loc->GetText_SecondaryStatName(SecType);
					const float Value = RunState->GetSecondaryStatValue(SecType);
					const bool bPercent = (SecType == ET66SecondaryStatType::CritChance || SecType == ET66SecondaryStatType::Crush
						|| SecType == ET66SecondaryStatType::Invisibility || SecType == ET66SecondaryStatType::LifeSteal
						|| SecType == ET66SecondaryStatType::Assassinate || SecType == ET66SecondaryStatType::Cheating
						|| SecType == ET66SecondaryStatType::Stealing);
					AddStatLineFloat(Label, Value, bPercent);
				}
				AddHorizontalLine();
			}
		}
	}

	TSharedRef<SWidget> StatsContent = bExtended
		? TSharedRef<SWidget>(SNew(SScrollBox)
			.ScrollBarVisibility(EVisibility::Visible)
			+ SScrollBox::Slot()
			[
				StatsBox
			])
		: TSharedRef<SWidget>(StatsBox);

	TSharedRef<SWidget> Content = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(HeaderText)
			.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			StatsContent
		];

	return SNew(SBox)
		.WidthOverride(WidthOverride)
		[
			FT66Style::MakePanel(
				Content,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4))
		];
}
