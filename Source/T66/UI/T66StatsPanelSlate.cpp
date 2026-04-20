// Copyright Tribulation 66. All Rights Reserved.

#include "T66StatsPanelSlate.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
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
#include "Widgets/SToolTip.h"

static int32 AdjustStatsPanelFontSize(int32 BaseSize, int32 FontSizeAdjustment)
{
	return FMath::Max(BaseSize + FontSizeAdjustment, 8);
}

static FSlateFontInfo MakeStatsPanelHeadingFont(int32 FontSizeAdjustment)
{
	FSlateFontInfo Font = FT66Style::Tokens::FontHeading();
	Font.Size = AdjustStatsPanelFontSize(Font.Size, FontSizeAdjustment);
	return Font;
}

static FSlateFontInfo MakeStatsPanelBodyFont(int32 FontSizeAdjustment)
{
	FSlateFontInfo Font = FT66Style::Tokens::FontBody();
	Font.Size = AdjustStatsPanelFontSize(Font.Size, FontSizeAdjustment);
	return Font;
}

static FSlateFontInfo MakeStatsPanelBoldFont(int32 BaseSize, int32 FontSizeAdjustment)
{
	return FT66Style::Tokens::FontBold(AdjustStatsPanelFontSize(BaseSize, FontSizeAdjustment));
}

static TSharedRef<SToolTip> MakeT66Tooltip(const FText& Title, const FText& Description, int32 FontSizeAdjustment = 0)
{
	return SNew(SToolTip)
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.DarkGroupBorder"))
		.BorderBackgroundColor(FT66Style::Tokens::Bg)
		.Padding(FMargin(10.f, 8.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(STextBlock)
				.Text(Title)
				.Font(MakeStatsPanelBoldFont(14, FontSizeAdjustment))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Description)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.Font(MakeStatsPanelBodyFont(FontSizeAdjustment))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true)
				.WrapTextAt(280.f)
			]
		]
	];
}

/** Per-category stat indices (ET66SecondaryStatType enum values). */
enum class EDerivedStatLine : uint8
{
	None,
	ArmorReduction,
	EvasionChance,
};

struct FSecondaryStatCategory
{
	FText Header;
	const int32* Indices;
	int32 Num;
	EDerivedStatLine DerivedLine = EDerivedStatLine::None;
	int32 PrimaryStatIndex = INDEX_NONE;
};

static const int32 CatDamage[] =
{
	static_cast<int32>(ET66SecondaryStatType::AoeDamage),
	static_cast<int32>(ET66SecondaryStatType::BounceDamage),
	static_cast<int32>(ET66SecondaryStatType::PierceDamage),
	static_cast<int32>(ET66SecondaryStatType::DotDamage),
};
static const int32 CatAttackSpeed[] =
{
	static_cast<int32>(ET66SecondaryStatType::AoeSpeed),
	static_cast<int32>(ET66SecondaryStatType::BounceSpeed),
	static_cast<int32>(ET66SecondaryStatType::PierceSpeed),
	static_cast<int32>(ET66SecondaryStatType::DotSpeed),
};
static const int32 CatAttackScale[] =
{
	static_cast<int32>(ET66SecondaryStatType::AoeScale),
	static_cast<int32>(ET66SecondaryStatType::BounceScale),
	static_cast<int32>(ET66SecondaryStatType::PierceScale),
	static_cast<int32>(ET66SecondaryStatType::DotScale),
};
static const int32 CatAccuracy[] =
{
	static_cast<int32>(ET66SecondaryStatType::CritDamage),
	static_cast<int32>(ET66SecondaryStatType::CritChance),
	static_cast<int32>(ET66SecondaryStatType::AttackRange),
	static_cast<int32>(ET66SecondaryStatType::Accuracy),
};
static const int32 CatArmor[] =
{
	static_cast<int32>(ET66SecondaryStatType::Taunt),
	static_cast<int32>(ET66SecondaryStatType::DamageReduction),
	static_cast<int32>(ET66SecondaryStatType::ReflectDamage),
	static_cast<int32>(ET66SecondaryStatType::Crush),
};
static const int32 CatEvasion[] =
{
	static_cast<int32>(ET66SecondaryStatType::EvasionChance),
	static_cast<int32>(ET66SecondaryStatType::CounterAttack),
	static_cast<int32>(ET66SecondaryStatType::Invisibility),
	static_cast<int32>(ET66SecondaryStatType::Assassinate),
};
static const int32 CatLuck[] =
{
	static_cast<int32>(ET66SecondaryStatType::TreasureChest),
	static_cast<int32>(ET66SecondaryStatType::Cheating),
	static_cast<int32>(ET66SecondaryStatType::Stealing),
	static_cast<int32>(ET66SecondaryStatType::LootCrate),
};

static const FSecondaryStatCategory SecondaryStatCategories[] =
{
	{ NSLOCTEXT("T66.StatsPanel", "CatDamage",      "Damage"),         CatDamage,      UE_ARRAY_COUNT(CatDamage), EDerivedStatLine::None,           1 },
	{ NSLOCTEXT("T66.StatsPanel", "CatAttackSpeed", "Attack Speed"),   CatAttackSpeed, UE_ARRAY_COUNT(CatAttackSpeed), EDerivedStatLine::None,      2 },
	{ NSLOCTEXT("T66.StatsPanel", "CatAttackScale", "Attack Scale"),   CatAttackScale, UE_ARRAY_COUNT(CatAttackScale), EDerivedStatLine::None,      3 },
	{ NSLOCTEXT("T66.StatsPanel", "CatAccuracy",    "Accuracy"),       CatAccuracy,    UE_ARRAY_COUNT(CatAccuracy), EDerivedStatLine::None,         4 },
	{ NSLOCTEXT("T66.StatsPanel", "CatArmor",       "Armor"),          CatArmor,       UE_ARRAY_COUNT(CatArmor), EDerivedStatLine::ArmorReduction, 5 },
	{ NSLOCTEXT("T66.StatsPanel", "CatEvasion",     "Evasion"),        CatEvasion,     UE_ARRAY_COUNT(CatEvasion), EDerivedStatLine::EvasionChance, 6 },
	{ NSLOCTEXT("T66.StatsPanel", "CatLuck",        "Luck"),           CatLuck,        UE_ARRAY_COUNT(CatLuck), EDerivedStatLine::None,            7 },
};
static constexpr int32 NumSecondaryStatCategories = UE_ARRAY_COUNT(SecondaryStatCategories);

static FText GetPrimaryStatLabel(UT66LocalizationSubsystem* Loc, int32 Index)
{
	if (!Loc) return FText::FromString(TEXT("?"));

	switch (Index)
	{
		case 0: return Loc->GetText_Level();
		case 1: return Loc->GetText_Stat_Damage();
		case 2: return Loc->GetText_Stat_AttackSpeed();
		case 3: return Loc->GetText_Stat_AttackScale();
		case 4: return Loc->GetText_Stat_Accuracy();
		case 5: return Loc->GetText_Stat_Armor();
		case 6: return Loc->GetText_Stat_Evasion();
		case 7: return Loc->GetText_Stat_Luck();
		default: return FText::FromString(TEXT("?"));
	}
}

static int32 GetRunStatePrimaryStatValue(const UT66RunStateSubsystem* RunState, int32 Index)
{
	if (!RunState)
	{
		return 0;
	}

	switch (Index)
	{
	case 0: return RunState->GetHeroLevel();
	case 1: return RunState->GetDamageStat();
	case 2: return RunState->GetAttackSpeedStat();
	case 3: return RunState->GetScaleStat();
	case 4: return RunState->GetAccuracyStat();
	case 5: return RunState->GetArmorStat();
	case 6: return RunState->GetEvasionStat();
	case 7: return RunState->GetLuckStat();
	default: return 0;
	}
}

static int32 GetHeroBasePrimaryStatValue(const UT66RunStateSubsystem* RunState, int32 Index)
{
	if (!RunState || Index <= 0)
	{
		return 0;
	}

	const UT66GameInstance* T66GI = Cast<UT66GameInstance>(RunState->GetGameInstance());
	if (!T66GI || T66GI->SelectedHeroID.IsNone())
	{
		return GetRunStatePrimaryStatValue(RunState, Index);
	}

	FT66HeroStatBlock BaseStats;
	FT66HeroPerLevelStatGains IgnoredGains;
	if (!T66GI->GetHeroStatTuning(T66GI->SelectedHeroID, BaseStats, IgnoredGains))
	{
		return GetRunStatePrimaryStatValue(RunState, Index);
	}

	switch (Index)
	{
	case 1: return BaseStats.Damage;
	case 2: return BaseStats.AttackSpeed;
	case 3: return BaseStats.AttackScale;
	case 4: return BaseStats.Accuracy;
	case 5: return BaseStats.Armor;
	case 6: return BaseStats.Evasion;
	case 7: return BaseStats.Luck;
	default: return 0;
	}
}

static FText GetPrimaryStatAdjective(const UT66RunStateSubsystem* RunState, int32 Index)
{
	if (!RunState || Index <= 0)
	{
		return FText::GetEmpty();
	}

	const int32 BaseValue = FMath::Clamp(GetHeroBasePrimaryStatValue(RunState, Index), 1, 10);
	const int32 CurrentValue = FMath::Clamp(GetRunStatePrimaryStatValue(RunState, Index), 1, UT66RunStateSubsystem::MaxHeroStatValue);
	const float Innate01 = FMath::Clamp((static_cast<float>(BaseValue) - 1.f) / 9.f, 0.f, 1.f);
	const float Growth01 = (CurrentValue > BaseValue)
		? FMath::Clamp(
			static_cast<float>(CurrentValue - BaseValue)
				/ static_cast<float>(FMath::Max(1, UT66RunStateSubsystem::MaxHeroStatValue - BaseValue)),
			0.f,
			1.f)
		: 0.f;
	const float Effective01 = FMath::Max(Innate01, Growth01);

	if (Effective01 >= 0.92f)
	{
		return NSLOCTEXT("T66.StatsPanel", "PrimaryAdjExcellent", "Excellent");
	}
	if (Effective01 >= 0.78f)
	{
		return NSLOCTEXT("T66.StatsPanel", "PrimaryAdjExceptional", "Exceptional");
	}
	if (Effective01 >= 0.62f)
	{
		return NSLOCTEXT("T66.StatsPanel", "PrimaryAdjGreat", "Great");
	}
	if (Effective01 >= 0.45f)
	{
		return NSLOCTEXT("T66.StatsPanel", "PrimaryAdjGood", "Good");
	}
	if (Effective01 >= 0.28f)
	{
		return NSLOCTEXT("T66.StatsPanel", "PrimaryAdjMid", "Mid");
	}
	if (Effective01 >= 0.12f)
	{
		return NSLOCTEXT("T66.StatsPanel", "PrimaryAdjPoor", "Poor");
	}

	return NSLOCTEXT("T66.StatsPanel", "PrimaryAdjTerrible", "Terrible");
}

static bool IsSecondaryPercent(ET66SecondaryStatType SecType)
{
	return SecType == ET66SecondaryStatType::CritChance
		|| SecType == ET66SecondaryStatType::ReflectDamage
		|| SecType == ET66SecondaryStatType::Crush
		|| SecType == ET66SecondaryStatType::Invisibility
		|| SecType == ET66SecondaryStatType::CounterAttack
		|| SecType == ET66SecondaryStatType::Assassinate
		|| SecType == ET66SecondaryStatType::Cheating
		|| SecType == ET66SecondaryStatType::Stealing
		|| SecType == ET66SecondaryStatType::DamageReduction
		|| SecType == ET66SecondaryStatType::EvasionChance
		|| SecType == ET66SecondaryStatType::Accuracy;
}

static float GetArmorReductionFromStatValue(int32 ArmorStat)
{
	const float Base = static_cast<float>(FMath::Max(1, ArmorStat) - 1) * 0.008f;
	return FMath::Clamp(Base, 0.f, 0.80f);
}

static float GetEvasionChanceFromStatValue(int32 EvasionStat)
{
	const float Base = static_cast<float>(FMath::Max(1, EvasionStat) - 1) * 0.006f;
	return FMath::Clamp(Base, 0.f, 0.60f);
}

static FText GetDerivedStatLabel(EDerivedStatLine DerivedLine)
{
	switch (DerivedLine)
	{
	case EDerivedStatLine::ArmorReduction:
		return NSLOCTEXT("T66.StatsPanel", "ArmorReduction", "Total Damage Reduction");
	case EDerivedStatLine::EvasionChance:
		return NSLOCTEXT("T66.StatsPanel", "EvasionChance", "Total Dodge Chance");
	case EDerivedStatLine::None:
	default:
		return FText::GetEmpty();
	}
}

static FText GetDerivedStatDescription(EDerivedStatLine DerivedLine)
{
	switch (DerivedLine)
	{
	case EDerivedStatLine::ArmorReduction:
		return NSLOCTEXT("T66.StatsPanel", "ArmorReductionDesc", "Total incoming damage reduction from your Armor stat and Damage Reduction items.");
	case EDerivedStatLine::EvasionChance:
		return NSLOCTEXT("T66.StatsPanel", "EvasionChanceDesc", "Total chance to fully dodge an incoming hit from your Evasion stat and Dodge items.");
	case EDerivedStatLine::None:
	default:
		return FText::GetEmpty();
	}
}

static float GetDerivedStatValue(const UT66RunStateSubsystem* RunState, EDerivedStatLine DerivedLine)
{
	if (!RunState)
	{
		return 0.f;
	}

	switch (DerivedLine)
	{
	case EDerivedStatLine::ArmorReduction:
		return RunState->GetArmorReduction01();
	case EDerivedStatLine::EvasionChance:
		return RunState->GetEvasionChance01();
	case EDerivedStatLine::None:
	default:
		return 0.f;
	}
}

static float GetDerivedStatValueFromSnapshot(const UT66LeaderboardRunSummarySaveGame* Snapshot, EDerivedStatLine DerivedLine)
{
	if (!Snapshot)
	{
		return 0.f;
	}

	switch (DerivedLine)
	{
	case EDerivedStatLine::ArmorReduction:
		if (const float* Bonus = Snapshot->SecondaryStatValues.Find(ET66SecondaryStatType::DamageReduction))
		{
			return FMath::Clamp(GetArmorReductionFromStatValue(Snapshot->ArmorStat) + *Bonus, 0.f, 0.80f);
		}
		return GetArmorReductionFromStatValue(Snapshot->ArmorStat);
	case EDerivedStatLine::EvasionChance:
		if (const float* Bonus = Snapshot->SecondaryStatValues.Find(ET66SecondaryStatType::EvasionChance))
		{
			return FMath::Clamp(GetEvasionChanceFromStatValue(Snapshot->EvasionStat) + *Bonus, 0.f, 0.60f);
		}
		return GetEvasionChanceFromStatValue(Snapshot->EvasionStat);
	case EDerivedStatLine::None:
	default:
		return 0.f;
	}
}

static FText FormatDisplayedValueOutOf99(int32 Value)
{
	return FText::Format(
		NSLOCTEXT("T66.StatsPanel", "ValueOutOf99", "{0}/99"),
		FText::AsNumber(FMath::Clamp(Value, 0, UT66RunStateSubsystem::MaxHeroStatValue)));
}

static FText FormatDisplayedSecondaryStatValue(ET66SecondaryStatType SecType, float Value)
{
	const int32 DisplayValue = IsSecondaryPercent(SecType)
		? FMath::CeilToInt(FMath::Max(0.f, Value) * 100.f)
		: FMath::CeilToInt(FMath::Max(0.f, Value));

	return FormatDisplayedValueOutOf99(DisplayValue);
}

static FText FormatDisplayedDerivedStatValue(EDerivedStatLine DerivedLine, float Value)
{
	switch (DerivedLine)
	{
	case EDerivedStatLine::ArmorReduction:
	case EDerivedStatLine::EvasionChance:
		return FormatDisplayedValueOutOf99(FMath::CeilToInt(FMath::Max(0.f, Value) * 100.f));
	case EDerivedStatLine::None:
	default:
		return FormatDisplayedValueOutOf99(FMath::CeilToInt(FMath::Max(0.f, Value)));
	}
}

static TSharedRef<SWidget> MakeStatsPanelLineContent(
	const FText& LineText,
	const FTextBlockStyle& TextStyle,
	const FSlateFontInfo& Font)
{
	return SNew(STextBlock)
		.Text(LineText)
		.TextStyle(&TextStyle)
		.Font(Font)
		.ColorAndOpacity(FT66Style::Tokens::Text);
}

static FText FormatDisplayedPrimaryStatValue(int32 Index, int32 Value)
{
	return FormatDisplayedValueOutOf99(Value);
}

static FText FormatPrimaryStatLineText(
	UT66LocalizationSubsystem* Loc,
	const UT66RunStateSubsystem* RunState,
	int32 Index,
	bool bIncludeAdjective)
{
	const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.StatsPanel", "StatLineFormat", "{0}: {1}");
	const FText Label = GetPrimaryStatLabel(Loc, Index);
	const FText ValueText = FormatDisplayedPrimaryStatValue(Index, GetRunStatePrimaryStatValue(RunState, Index));
	if (!bIncludeAdjective || Index == 0 || !RunState)
	{
		return FText::Format(StatFmt, Label, ValueText);
	}

	const FText Adjective = GetPrimaryStatAdjective(RunState, Index);
	return FText::Format(
		NSLOCTEXT("T66.StatsPanel", "PrimaryStatLineWithAdjective", "{0}: {1} ({2})"),
		Label,
		ValueText,
		Adjective);
}

static FText FormatCategoryHeaderText(
	UT66LocalizationSubsystem* Loc,
	const UT66RunStateSubsystem* RunState,
	const FSecondaryStatCategory& Category)
{
	const FText Label = (Category.PrimaryStatIndex != INDEX_NONE && Loc)
		? GetPrimaryStatLabel(Loc, Category.PrimaryStatIndex)
		: Category.Header;
	if (!RunState || Category.PrimaryStatIndex == INDEX_NONE)
	{
		return Label;
	}

	return FText::Format(
		NSLOCTEXT("T66.StatsPanel", "CategoryHeaderWithValue", "{0}: {1}"),
		Label,
		FormatDisplayedPrimaryStatValue(Category.PrimaryStatIndex, GetRunStatePrimaryStatValue(RunState, Category.PrimaryStatIndex)));
}

void T66StatsPanelSlate::FT66LiveStatsPanel::Reset()
{
	PrimaryLines.Empty();
	PrimaryLines.SetNum(8);
	CategoryHeaderLines.Empty();
	CategoryHeaderLines.SetNum(NumSecondaryStatCategories);
	SecondaryLines.Reset();
	ArmorReductionLine.Reset();
	EvasionChanceLine.Reset();
}

void T66StatsPanelSlate::FT66LiveStatsPanel::Update(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc) const
{
	const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.StatsPanel", "StatLineFormat", "{0}: {1}");

	auto SetPrimaryLine = [&](int32 Index)
	{
		if (!PrimaryLines.IsValidIndex(Index) || !PrimaryLines[Index].IsValid())
		{
			return;
		}

		PrimaryLines[Index]->SetText(FormatPrimaryStatLineText(Loc, RunState, Index, true));
	};

	auto SetCategoryHeaderLine = [&](int32 CategoryIndex, const FSecondaryStatCategory& Category)
	{
		if (!CategoryHeaderLines.IsValidIndex(CategoryIndex) || !CategoryHeaderLines[CategoryIndex].IsValid())
		{
			return;
		}

		CategoryHeaderLines[CategoryIndex]->SetText(FormatCategoryHeaderText(Loc, RunState, Category));
	};

	if (!RunState)
	{
		for (const TSharedPtr<STextBlock>& Line : PrimaryLines)
		{
			if (Line.IsValid())
			{
				Line->SetText(FText::GetEmpty());
			}
		}

		for (const TPair<ET66SecondaryStatType, TSharedPtr<STextBlock>>& Pair : SecondaryLines)
		{
			if (Pair.Value.IsValid())
			{
				Pair.Value->SetText(FText::GetEmpty());
			}
		}
		if (ArmorReductionLine.IsValid())
		{
			ArmorReductionLine->SetText(FText::GetEmpty());
		}
		if (EvasionChanceLine.IsValid())
		{
			EvasionChanceLine->SetText(FText::GetEmpty());
		}
		for (const TSharedPtr<STextBlock>& HeaderLine : CategoryHeaderLines)
		{
			if (HeaderLine.IsValid())
			{
				HeaderLine->SetText(FText::GetEmpty());
			}
		}
		return;
	}

	for (int32 Index = 0; Index < 8; ++Index)
	{
		SetPrimaryLine(Index);
	}

	if (ArmorReductionLine.IsValid())
	{
		ArmorReductionLine->SetText(FText::Format(
			StatFmt,
			GetDerivedStatLabel(EDerivedStatLine::ArmorReduction),
			FormatDisplayedDerivedStatValue(EDerivedStatLine::ArmorReduction, GetDerivedStatValue(RunState, EDerivedStatLine::ArmorReduction))));
	}

	if (EvasionChanceLine.IsValid())
	{
		EvasionChanceLine->SetText(FText::Format(
			StatFmt,
			GetDerivedStatLabel(EDerivedStatLine::EvasionChance),
			FormatDisplayedDerivedStatValue(EDerivedStatLine::EvasionChance, GetDerivedStatValue(RunState, EDerivedStatLine::EvasionChance))));
	}

	for (int32 CategoryIndex = 0; CategoryIndex < NumSecondaryStatCategories; ++CategoryIndex)
	{
		SetCategoryHeaderLine(CategoryIndex, SecondaryStatCategories[CategoryIndex]);
	}

	for (const TPair<ET66SecondaryStatType, TSharedPtr<STextBlock>>& Pair : SecondaryLines)
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}

		const ET66SecondaryStatType SecType = Pair.Key;
		const FText Label = Loc ? Loc->GetText_SecondaryStatName(SecType) : FText::FromString(TEXT("?"));
		const float Value = RunState->GetSecondaryStatValue(SecType);
		Pair.Value->SetText(FText::Format(StatFmt, Label, FormatDisplayedSecondaryStatValue(SecType, Value)));
	}
}

TSharedRef<SWidget> T66StatsPanelSlate::MakeEssentialStatsPanel(
	UT66RunStateSubsystem* RunState,
	UT66LocalizationSubsystem* Loc,
	float WidthOverride,
	bool bExtended,
	int32 FontSizeAdjustment,
	int32 HeadingFontSizeAdjustment,
	bool /*bShowAdjectiveSummaries*/)
{
	const FText HeaderText = NSLOCTEXT("T66.StatsPanel", "Header", "STATS");
	const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
	const FTextBlockStyle& BodyStyle = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FSlateFontInfo HeadingFont = MakeStatsPanelHeadingFont(FontSizeAdjustment + HeadingFontSizeAdjustment);
	const FSlateFontInfo BodyFont = MakeStatsPanelBodyFont(FontSizeAdjustment);

	TSharedRef<SVerticalBox> StatsBox = SNew(SVerticalBox);

	if (RunState)
	{
		const int32 HeroLevel = RunState->GetHeroLevel();
		const int32 DamageStat = RunState->GetDamageStat();
		const int32 AttackSpeedStat = RunState->GetAttackSpeedStat();
		const int32 AttackScaleStat = RunState->GetScaleStat();
		const int32 AccuracyStat = RunState->GetAccuracyStat();
		const int32 ArmorStat = RunState->GetArmorStat();
		const int32 EvasionStat = RunState->GetEvasionStat();
		const int32 LuckStat = RunState->GetLuckStat();

		auto AddStatLine = [&](int32 Index, const FText& Label, int32 Value, const FText& TooltipTitle, const FText& TooltipDesc)
		{
			FText LineText = FText::Format(StatFmt, Label, FormatDisplayedPrimaryStatValue(Index, Value));
			if (Index > 0)
			{
				LineText = FormatPrimaryStatLineText(Loc, RunState, Index, true);
			}

			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(0.f)
				.ToolTip(MakeT66Tooltip(TooltipTitle, TooltipDesc, FontSizeAdjustment))
				[
					MakeStatsPanelLineContent(
						LineText,
						BodyStyle,
						BodyFont)
				]
			];
		};

		auto AddStatLineFloat = [&](const FText& Label, const FText& ValueText, const FText& TooltipTitle, const FText& TooltipDesc)
		{
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(0.f)
				.ToolTip(MakeT66Tooltip(TooltipTitle, TooltipDesc, FontSizeAdjustment))
				[
					MakeStatsPanelLineContent(
						FText::Format(StatFmt, Label, ValueText),
						BodyStyle,
						BodyFont)
				]
			];
		};

		AddStatLine(0, GetPrimaryStatLabel(Loc, 0), HeroLevel,       GetPrimaryStatLabel(Loc, 0), Loc ? Loc->GetText_PrimaryStatDescription(0) : FText::GetEmpty());
		AddStatLine(1, GetPrimaryStatLabel(Loc, 1), DamageStat,      GetPrimaryStatLabel(Loc, 1), Loc ? Loc->GetText_PrimaryStatDescription(1) : FText::GetEmpty());
		AddStatLine(2, GetPrimaryStatLabel(Loc, 2), AttackSpeedStat, GetPrimaryStatLabel(Loc, 2), Loc ? Loc->GetText_PrimaryStatDescription(2) : FText::GetEmpty());
		AddStatLine(3, GetPrimaryStatLabel(Loc, 3), AttackScaleStat, GetPrimaryStatLabel(Loc, 3), Loc ? Loc->GetText_PrimaryStatDescription(3) : FText::GetEmpty());
		AddStatLine(4, GetPrimaryStatLabel(Loc, 4), AccuracyStat,    GetPrimaryStatLabel(Loc, 4), Loc ? Loc->GetText_PrimaryStatDescription(4) : FText::GetEmpty());
		AddStatLine(5, GetPrimaryStatLabel(Loc, 5), ArmorStat,       GetPrimaryStatLabel(Loc, 5), Loc ? Loc->GetText_PrimaryStatDescription(5) : FText::GetEmpty());
		AddStatLine(6, GetPrimaryStatLabel(Loc, 6), EvasionStat,     GetPrimaryStatLabel(Loc, 6), Loc ? Loc->GetText_PrimaryStatDescription(6) : FText::GetEmpty());
		AddStatLine(7, GetPrimaryStatLabel(Loc, 7), LuckStat,        GetPrimaryStatLabel(Loc, 7), Loc ? Loc->GetText_PrimaryStatDescription(7) : FText::GetEmpty());

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

			auto AddCategoryHeader = [&](const FSecondaryStatCategory& Category)
			{
				StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
					.Text(FormatCategoryHeaderText(Loc, RunState, Category))
					.TextStyle(&HeadingStyle)
					.Font(HeadingFont)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				];
			};

			for (int32 c = 0; c < NumSecondaryStatCategories; ++c)
			{
				const FSecondaryStatCategory& Cat = SecondaryStatCategories[c];
				AddHorizontalLine();
				AddCategoryHeader(Cat);
				if (Cat.DerivedLine != EDerivedStatLine::None)
				{
					const FText DerivedLabel = GetDerivedStatLabel(Cat.DerivedLine);
					const FText DerivedDesc = GetDerivedStatDescription(Cat.DerivedLine);
					const float DerivedValue = GetDerivedStatValue(RunState, Cat.DerivedLine);
					AddStatLineFloat(
						DerivedLabel,
						FormatDisplayedDerivedStatValue(Cat.DerivedLine, DerivedValue),
						DerivedLabel,
						DerivedDesc);
				}
				for (int32 k = 0; k < Cat.Num; ++k)
				{
					const int32 i = Cat.Indices[k];
					const ET66SecondaryStatType SecType = static_cast<ET66SecondaryStatType>(i);
					const FText Label = Loc->GetText_SecondaryStatName(SecType);
					const float Value = RunState->GetSecondaryStatValue(SecType);
					const FText SecDesc = Loc ? Loc->GetText_SecondaryStatDescription(SecType) : FText::GetEmpty();
					AddStatLineFloat(
						Label,
						FormatDisplayedSecondaryStatValue(SecType, Value),
						Label,
						SecDesc);
				}
				AddHorizontalLine();
			}
		}
	}

	TSharedRef<SWidget> StatsContent = bExtended
		? TSharedRef<SWidget>(SNew(SBox)
			.HeightOverride(FT66Style::Tokens::NPCStatsPanelContentHeight)
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
			.Font(HeadingFont)
		]
		+ SVerticalBox::Slot().AutoHeight()
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

TSharedRef<SWidget> T66StatsPanelSlate::MakeLiveEssentialStatsPanel(
	UT66RunStateSubsystem* RunState,
	UT66LocalizationSubsystem* Loc,
	const TSharedRef<FT66LiveStatsPanel>& LivePanel,
	float WidthOverride,
	bool bExtended,
	int32 FontSizeAdjustment)
{
	const FText HeaderText = NSLOCTEXT("T66.StatsPanel", "Header", "STATS");
	const FSlateFontInfo HeadingFont = MakeStatsPanelHeadingFont(FontSizeAdjustment);
	const FSlateFontInfo BodyFont = MakeStatsPanelBodyFont(FontSizeAdjustment);

	LivePanel->Reset();

	TSharedRef<SVerticalBox> StatsBox = SNew(SVerticalBox);

	auto AddPrimaryLine = [&](int32 Index)
	{
		const FText Label = GetPrimaryStatLabel(Loc, Index);
		const FText Description = Loc ? Loc->GetText_PrimaryStatDescription(Index) : FText::GetEmpty();
		StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Padding(0.f)
			.ToolTip(MakeT66Tooltip(Label, Description, FontSizeAdjustment))
			[
				SAssignNew(LivePanel->PrimaryLines[Index], STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
				.Font(BodyFont)
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
		];
	};

	for (int32 Index = 0; Index < 8; ++Index)
	{
		AddPrimaryLine(Index);
	}

	if (bExtended)
	{
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

		auto AddCategoryHeader = [&](int32 CategoryIndex)
		{
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SAssignNew(LivePanel->CategoryHeaderLines[CategoryIndex], STextBlock)
				.Text(FText::GetEmpty())
				.TextStyle(&HeadingStyle)
				.Font(HeadingFont)
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
		};

		for (int32 c = 0; c < NumSecondaryStatCategories; ++c)
		{
			const FSecondaryStatCategory& Cat = SecondaryStatCategories[c];
			AddHorizontalLine();
			AddCategoryHeader(c);

			if (Cat.DerivedLine != EDerivedStatLine::None)
			{
				const FText DerivedLabel = GetDerivedStatLabel(Cat.DerivedLine);
				const FText Description = GetDerivedStatDescription(Cat.DerivedLine);
				TSharedPtr<STextBlock> DerivedLineText;

				StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					.Padding(0.f)
					.ToolTip(MakeT66Tooltip(DerivedLabel, Description, FontSizeAdjustment))
					[
						SAssignNew(DerivedLineText, STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.Font(BodyFont)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				];

				if (Cat.DerivedLine == EDerivedStatLine::ArmorReduction)
				{
					LivePanel->ArmorReductionLine = DerivedLineText;
				}
				else if (Cat.DerivedLine == EDerivedStatLine::EvasionChance)
				{
					LivePanel->EvasionChanceLine = DerivedLineText;
				}
			}

			for (int32 k = 0; k < Cat.Num; ++k)
			{
				const ET66SecondaryStatType SecType = static_cast<ET66SecondaryStatType>(Cat.Indices[k]);
				const FText Label = Loc ? Loc->GetText_SecondaryStatName(SecType) : FText::GetEmpty();
				const FText Description = Loc ? Loc->GetText_SecondaryStatDescription(SecType) : FText::GetEmpty();
				TSharedPtr<STextBlock> LineText;

				StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					.Padding(0.f)
					.ToolTip(MakeT66Tooltip(Label, Description, FontSizeAdjustment))
					[
						SAssignNew(LineText, STextBlock)
						.Text(FText::GetEmpty())
						.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body"))
						.Font(BodyFont)
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				];

				LivePanel->SecondaryLines.Add(SecType, LineText);
			}

			AddHorizontalLine();
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

	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
	Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
	[
		SNew(STextBlock)
		.Text(HeaderText)
		.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
		.Font(HeadingFont)
	];

	if (bExtended)
	{
		Content->AddSlot().FillHeight(1.f)
		[
			StatsContent
		];
	}
	else
	{
		Content->AddSlot().AutoHeight()
		[
			StatsContent
		];
	}

	LivePanel->Update(RunState, Loc);

	return SNew(SBox)
		.WidthOverride(WidthOverride)
		[
			FT66Style::MakePanel(
				Content,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4))
		];
}

TSharedRef<SWidget> T66StatsPanelSlate::MakeEssentialStatsPanelFromSnapshot(
	UT66LeaderboardRunSummarySaveGame* Snapshot,
	UT66LocalizationSubsystem* Loc,
	float WidthOverride,
	int32 FontSizeAdjustment)
{
	FT66SnapshotStatsPanelOptions Options;
	Options.WidthOverride = WidthOverride;
	Options.FontSizeAdjustment = FontSizeAdjustment;
	Options.bExtended = Snapshot && Snapshot->SecondaryStatValues.Num() > 0;
	return MakeEssentialStatsPanelFromSnapshotWithOptions(Snapshot, Loc, Options);
}

TSharedRef<SWidget> T66StatsPanelSlate::MakeEssentialStatsPanelFromSnapshotWithOptions(
	UT66LeaderboardRunSummarySaveGame* Snapshot,
	UT66LocalizationSubsystem* Loc,
	const FT66SnapshotStatsPanelOptions& Options)
{
	const FText HeaderText = NSLOCTEXT("T66.StatsPanel", "Header", "STATS");
	const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
	const FTextBlockStyle& BodyStyle = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Body");
	const FTextBlockStyle& HeadingStyle = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading");
	const FSlateFontInfo HeadingFont = MakeStatsPanelHeadingFont(Options.FontSizeAdjustment + Options.HeadingFontSizeAdjustment);
	const FSlateFontInfo BodyFont = MakeStatsPanelBodyFont(Options.FontSizeAdjustment);
	const bool bExtended = Options.bExtended;

	TSharedRef<SVerticalBox> StatsBox = SNew(SVerticalBox);

	auto GetSnapshotPrimaryStatValue = [Snapshot](int32 Index) -> int32
	{
		if (!Snapshot)
		{
			return 0;
		}

		switch (Index)
		{
		case 0: return Snapshot->HeroLevel;
		case 1: return Snapshot->DamageStat;
		case 2: return Snapshot->AttackSpeedStat;
		case 3: return Snapshot->AttackScaleStat;
		case 4: return Snapshot->AccuracyStat;
		case 5: return Snapshot->ArmorStat;
		case 6: return Snapshot->EvasionStat;
		case 7: return Snapshot->LuckStat;
		default: return 0;
		}
	};

	if (Snapshot)
	{
		auto AddPrimaryLine = [&](int32 Index)
		{
			if (!Options.bIncludeLevel && Index == 0)
			{
				return;
			}

			const FText Label = GetPrimaryStatLabel(Loc, Index);
			const int32 Value = GetSnapshotPrimaryStatValue(Index);
			const FText Description = Loc ? Loc->GetText_PrimaryStatDescription(Index) : FText::GetEmpty();

			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(0.f)
				.ToolTip(MakeT66Tooltip(Label, Description, Options.FontSizeAdjustment))
				[
					MakeStatsPanelLineContent(
						FText::Format(StatFmt, Label, FormatDisplayedPrimaryStatValue(Index, Value)),
						BodyStyle,
						BodyFont)
				]
			];
		};

		auto AddSecondaryLine = [&](const FText& Label, const FText& ValueText, const FText& TooltipTitle, const FText& TooltipDesc)
		{
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Padding(0.f)
				.ToolTip(MakeT66Tooltip(TooltipTitle, TooltipDesc, Options.FontSizeAdjustment))
				[
					MakeStatsPanelLineContent(
						FText::Format(StatFmt, Label, ValueText),
						BodyStyle,
						BodyFont)
				]
			];
		};

		for (int32 Index = 0; Index < 8; ++Index)
		{
			AddPrimaryLine(Index);
		}

		if (bExtended)
		{
			const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
			const FLinearColor LineColor(0.35f, 0.38f, 0.42f, 0.9f);

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

			auto AddCategoryHeader = [&](const FSecondaryStatCategory& Category)
			{
				FText Text = Category.Header;
				if (Category.PrimaryStatIndex != INDEX_NONE)
				{
					Text = FText::Format(
						NSLOCTEXT("T66.StatsPanel", "CategoryHeaderWithValue", "{0}: {1}"),
						GetPrimaryStatLabel(Loc, Category.PrimaryStatIndex),
						FormatDisplayedPrimaryStatValue(Category.PrimaryStatIndex, GetSnapshotPrimaryStatValue(Category.PrimaryStatIndex)));
				}

				StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
					.Text(Text)
					.TextStyle(&HeadingStyle)
					.Font(HeadingFont)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				];
			};

			for (int32 c = 0; c < NumSecondaryStatCategories; ++c)
			{
				const FSecondaryStatCategory& Cat = SecondaryStatCategories[c];
				AddHorizontalLine();
				AddCategoryHeader(Cat);

				if (Cat.DerivedLine != EDerivedStatLine::None)
				{
					const FText DerivedLabel = GetDerivedStatLabel(Cat.DerivedLine);
					const FText DerivedDesc = GetDerivedStatDescription(Cat.DerivedLine);
					const float DerivedValue = GetDerivedStatValueFromSnapshot(Snapshot, Cat.DerivedLine);
					AddSecondaryLine(
						DerivedLabel,
						FormatDisplayedDerivedStatValue(Cat.DerivedLine, DerivedValue),
						DerivedLabel,
						DerivedDesc);
				}

				for (int32 k = 0; k < Cat.Num; ++k)
				{
					const ET66SecondaryStatType SecType = static_cast<ET66SecondaryStatType>(Cat.Indices[k]);
					const FText Label = Loc ? Loc->GetText_SecondaryStatName(SecType) : FText::FromString(TEXT("?"));
					const float* P = Snapshot->SecondaryStatValues.Find(SecType);
					const float Value = P ? *P : 0.f;
					AddSecondaryLine(
						Label,
						FormatDisplayedSecondaryStatValue(SecType, Value),
						Label,
						Loc ? Loc->GetText_SecondaryStatDescription(SecType) : FText::GetEmpty());
				}

				AddHorizontalLine();
			}
		}
	}

	TSharedRef<SWidget> StatsContent = StatsBox;
	if (bExtended)
	{
		TSharedRef<SWidget> ScrollContent = SNew(SScrollBox)
			.ScrollBarVisibility(EVisibility::Visible)
			+ SScrollBox::Slot()
			[
				StatsBox
			];

		if (Options.ScrollHeightOverride > 0.f)
		{
			StatsContent = SNew(SBox)
				.HeightOverride(Options.ScrollHeightOverride)
				[
					ScrollContent
				];
		}
		else
		{
			StatsContent = ScrollContent;
		}
	}

	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
	if (Options.bIncludeHeader)
	{
		Content->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(HeaderText)
			.TextStyle(&HeadingStyle)
			.Font(HeadingFont)
		];
	}

	if (bExtended)
	{
		Content->AddSlot().FillHeight(1.f)
		[
			StatsContent
		];
	}
	else
	{
		Content->AddSlot().AutoHeight()
		[
			StatsContent
		];
	}

	TSharedRef<SWidget> Panel = FT66Style::MakePanel(
		Content,
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space4));

	if (Options.WidthOverride > 0.f)
	{
		return SNew(SBox)
			.WidthOverride(Options.WidthOverride)
			[
				Panel
			];
	}

	return Panel;
}
