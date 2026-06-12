// Copyright Tribulation 66. All Rights Reserved.

#include "T66StatsPanelSlate.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Data/T66DataTypes.h"
#include "UI/T66TooltipResolvers.h"
#include "UI/T66TooltipSlate.h"
#include "UI/Style/T66FlatStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

static int32 AdjustStatsPanelFontSize(int32 BaseSize, int32 FontSizeAdjustment)
{
	return FMath::Max(BaseSize + FontSizeAdjustment, 8);
}

static FSlateFontInfo MakeStatsPanelHeadingFont(int32 FontSizeAdjustment)
{
	return FT66FlatStyle::MakeBoldFont(AdjustStatsPanelFontSize(26, FontSizeAdjustment));
}

static FSlateFontInfo MakeStatsPanelBodyFont(int32 FontSizeAdjustment)
{
	return FT66FlatStyle::MakeFont(AdjustStatsPanelFontSize(15, FontSizeAdjustment));
}

static TSharedPtr<IToolTip> MakeT66Tooltip(const FText& Title, const FText& Description, int32 FontSizeAdjustment = 0)
{
	FT66TooltipPayload Payload = T66TooltipResolvers::MakeRichTooltip(
		FName(TEXT("StatsPanel.Tooltip")),
		ET66TooltipKind::Stat,
		Title,
		Description);
	Payload.FontSizeAdjustment = FontSizeAdjustment;
	return T66TooltipSlate::MakeTooltip(Payload);
}

/** Per-category stat indices (ET66StatType enum values). */
enum class EDerivedStatLine : uint8
{
	None,
	ArmorReduction,
	EvasionChance,
};

struct FStatCategory
{
	FText Header;
	const int32* Indices;
	int32 Num;
	EDerivedStatLine DerivedLine = EDerivedStatLine::None;
	int32 BaseStatIndex = INDEX_NONE;
};

static const int32 CatDamage[] =
{
	static_cast<int32>(ET66StatType::AoeDamage),
	static_cast<int32>(ET66StatType::BounceDamage),
	static_cast<int32>(ET66StatType::SummonDamage),
	static_cast<int32>(ET66StatType::DotDamage),
};
static const int32 CatAttackSpeed[] =
{
	static_cast<int32>(ET66StatType::AoeSpeed),
	static_cast<int32>(ET66StatType::BounceSpeed),
	static_cast<int32>(ET66StatType::SummonSpeed),
	static_cast<int32>(ET66StatType::DotSpeed),
};
static const int32 CatAttackScale[] =
{
	static_cast<int32>(ET66StatType::AoeScale),
	static_cast<int32>(ET66StatType::BounceScale),
	static_cast<int32>(ET66StatType::SummonScale),
	static_cast<int32>(ET66StatType::DotScale),
};
static const int32 CatAccuracy[] =
{
	static_cast<int32>(ET66StatType::CritChance),
	static_cast<int32>(ET66StatType::HeadshotChance),
	static_cast<int32>(ET66StatType::AttackRange),
	static_cast<int32>(ET66StatType::Execute),
};
static const int32 CatArmor[] =
{
	static_cast<int32>(ET66StatType::DamageReduction),
	static_cast<int32>(ET66StatType::ReflectDamage),
	static_cast<int32>(ET66StatType::Taunt),
	static_cast<int32>(ET66StatType::Crush),
};
static const int32 CatEvasion[] =
{
	static_cast<int32>(ET66StatType::EvasionChance),
	static_cast<int32>(ET66StatType::CounterAttack),
	static_cast<int32>(ET66StatType::Invisibility),
	static_cast<int32>(ET66StatType::Assassinate),
};
static const int32 CatLuck[] =
{
	static_cast<int32>(ET66StatType::LootCrate),
	static_cast<int32>(ET66StatType::TreasureChest),
	static_cast<int32>(ET66StatType::LootBag),
	static_cast<int32>(ET66StatType::LootWheel),
};
static const int32 CatElementalPower[] =
{
	static_cast<int32>(ET66StatType::FirePower),
	static_cast<int32>(ET66StatType::IcePower),
	static_cast<int32>(ET66StatType::ElectricityPower),
	static_cast<int32>(ET66StatType::NaturePower),
	static_cast<int32>(ET66StatType::WindPower),
};

static const FStatCategory StatCategories[] =
{
	{ NSLOCTEXT("T66.StatsPanel", "CatDamage",      "Damage"),         CatDamage,      UE_ARRAY_COUNT(CatDamage), EDerivedStatLine::None,           1 },
	{ NSLOCTEXT("T66.StatsPanel", "CatAttackSpeed", "Attack Speed"),   CatAttackSpeed, UE_ARRAY_COUNT(CatAttackSpeed), EDerivedStatLine::None,      2 },
	{ NSLOCTEXT("T66.StatsPanel", "CatAttackScale", "Attack Scale"),   CatAttackScale, UE_ARRAY_COUNT(CatAttackScale), EDerivedStatLine::None,      3 },
	{ NSLOCTEXT("T66.StatsPanel", "CatAccuracy",    "Accuracy"),       CatAccuracy,    UE_ARRAY_COUNT(CatAccuracy), EDerivedStatLine::None,         4 },
	{ NSLOCTEXT("T66.StatsPanel", "CatArmor",       "Armor"),          CatArmor,       UE_ARRAY_COUNT(CatArmor), EDerivedStatLine::ArmorReduction, 5 },
	{ NSLOCTEXT("T66.StatsPanel", "CatEvasion",     "Evasion"),        CatEvasion,     UE_ARRAY_COUNT(CatEvasion), EDerivedStatLine::EvasionChance, 6 },
	{ NSLOCTEXT("T66.StatsPanel", "CatLuck",        "Luck"),           CatLuck,        UE_ARRAY_COUNT(CatLuck), EDerivedStatLine::None,            7 },
	{ NSLOCTEXT("T66.StatsPanel", "CatElemental",   "Elemental Power"), CatElementalPower, UE_ARRAY_COUNT(CatElementalPower), EDerivedStatLine::None, INDEX_NONE },
};
static constexpr int32 NumStatCategories = UE_ARRAY_COUNT(StatCategories);

static FText GetBaseStatLabel(UT66LocalizationSubsystem* Loc, int32 Index)
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
		case 8: return Loc->GetText_Stat_Speed();
		default: return FText::FromString(TEXT("?"));
	}
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
		if (const float* Bonus = Snapshot->StatValues.Find(ET66StatType::DamageReduction))
		{
			return FMath::Clamp(GetArmorReductionFromStatValue(Snapshot->ArmorStat) + *Bonus, 0.f, 0.80f);
		}
		return GetArmorReductionFromStatValue(Snapshot->ArmorStat);
	case EDerivedStatLine::EvasionChance:
		if (const float* Bonus = Snapshot->StatValues.Find(ET66StatType::EvasionChance))
		{
			return FMath::Clamp(GetEvasionChanceFromStatValue(Snapshot->EvasionStat) + *Bonus, 0.f, 0.60f);
		}
		return GetEvasionChanceFromStatValue(Snapshot->EvasionStat);
	case EDerivedStatLine::None:
	default:
		return 0.f;
	}
}

/** Stats Rework: unified stat rows show the accumulated bonus points as "+X%" (1 point == 1%). */
static FText FormatDisplayedStatBonusPercent(float BonusPoints)
{
	return FText::Format(
		NSLOCTEXT("T66.StatsPanel", "StatBonusPercent", "+{0}%"),
		FText::AsNumber(FMath::Max(0, FMath::RoundToInt(BonusPoints))));
}

/** Derived rows (Armor Reduction / Dodge Chance) show the effective percent, not a bonus. */
static FText FormatDisplayedDerivedStatValue(EDerivedStatLine /*DerivedLine*/, float Value)
{
	return FText::Format(
		NSLOCTEXT("T66.StatsPanel", "EffectivePercent", "{0}%"),
		FText::AsNumber(FMath::CeilToInt(FMath::Max(0.f, Value) * 100.f)));
}

static TSharedRef<SWidget> MakeStatsPanelLineContent(
	const FText& LineText,
	const FSlateFontInfo& Font)
{
	return SNew(STextBlock)
		.Text(LineText)
		.Font(Font)
		.ColorAndOpacity(FT66FlatStyle::PrimaryText());
}

static FText FormatCategoryHeaderText(
	UT66LocalizationSubsystem* Loc,
	const FStatCategory& Category)
{
	// Headers are label-only: the old base-stat number left with the primary tier (spec section 1.6).
	return (Category.BaseStatIndex != INDEX_NONE && Loc)
		? GetBaseStatLabel(Loc, Category.BaseStatIndex)
		: Category.Header;
}

void T66StatsPanelSlate::FT66LiveStatsPanel::Reset()
{
	CategoryHeaderLines.Empty();
	CategoryHeaderLines.SetNum(NumStatCategories);
	SecondaryLines.Reset();
	ArmorReductionLine.Reset();
	EvasionChanceLine.Reset();
}

void T66StatsPanelSlate::FT66LiveStatsPanel::Update(UT66RunStateSubsystem* RunState, UT66LocalizationSubsystem* Loc) const
{
	const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.StatsPanel", "StatLineFormat", "{0}: {1}");

	auto SetCategoryHeaderLine = [&](int32 CategoryIndex, const FStatCategory& Category)
	{
		if (!CategoryHeaderLines.IsValidIndex(CategoryIndex) || !CategoryHeaderLines[CategoryIndex].IsValid())
		{
			return;
		}

		CategoryHeaderLines[CategoryIndex]->SetText(FormatCategoryHeaderText(Loc, Category));
	};

	if (!RunState)
	{
		for (const TPair<ET66StatType, TSharedPtr<STextBlock>>& Pair : SecondaryLines)
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

	for (int32 CategoryIndex = 0; CategoryIndex < NumStatCategories; ++CategoryIndex)
	{
		SetCategoryHeaderLine(CategoryIndex, StatCategories[CategoryIndex]);
	}

	for (const TPair<ET66StatType, TSharedPtr<STextBlock>>& Pair : SecondaryLines)
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}

		const ET66StatType SecType = Pair.Key;
		const FText Label = Loc ? Loc->GetText_StatName(SecType) : FText::FromString(TEXT("?"));
		Pair.Value->SetText(FText::Format(StatFmt, Label, FormatDisplayedStatBonusPercent(RunState->GetStatBonusValue(SecType))));
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
	const FSlateFontInfo HeadingFont = MakeStatsPanelHeadingFont(FontSizeAdjustment + HeadingFontSizeAdjustment);
	const FSlateFontInfo BodyFont = MakeStatsPanelBodyFont(FontSizeAdjustment);

	TSharedRef<SVerticalBox> StatsBox = SNew(SVerticalBox);

	if (RunState)
	{
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
						BodyFont)
				]
			];
		};

		if (bExtended)
		{
			// Secondary stats by category: line, bold header, stat lines, line (Shop/Gambler)
			auto AddHorizontalLine = [&]()
			{
				StatsBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 8.f)
				[
					FT66FlatStyle::MakeFlatDivider(Orient_Horizontal)
				];
			};

			auto AddCategoryHeader = [&](const FStatCategory& Category)
			{
				StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
					.Text(FormatCategoryHeaderText(Loc, Category))
					.Font(HeadingFont)
					.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				];
			};

			for (int32 c = 0; c < NumStatCategories; ++c)
			{
				const FStatCategory& Cat = StatCategories[c];
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
					const ET66StatType SecType = static_cast<ET66StatType>(i);
					const FText Label = Loc->GetText_StatName(SecType);
					const FText SecDesc = Loc ? Loc->GetText_StatDescription(SecType) : FText::GetEmpty();
					AddStatLineFloat(
						Label,
						FormatDisplayedStatBonusPercent(RunState->GetStatBonusValue(SecType)),
						Label,
						SecDesc);
				}
				AddHorizontalLine();
			}
		}
	}

	TSharedRef<SWidget> StatsContent = bExtended
		? TSharedRef<SWidget>(SNew(SBox)
			.HeightOverride(400.f)
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
			.Font(HeadingFont)
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			StatsContent
		];

	return SNew(SBox)
		.WidthOverride(WidthOverride)
		[
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(16.f),
				Content)
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

	if (bExtended)
	{
		auto AddHorizontalLine = [&]()
		{
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 8.f)
			[
				FT66FlatStyle::MakeFlatDivider(Orient_Horizontal)
			];
		};

		auto AddCategoryHeader = [&](int32 CategoryIndex)
		{
			StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SAssignNew(LivePanel->CategoryHeaderLines[CategoryIndex], STextBlock)
				.Text(FText::GetEmpty())
				.Font(HeadingFont)
				.ColorAndOpacity(FT66FlatStyle::PrimaryText())
			];
		};

		for (int32 c = 0; c < NumStatCategories; ++c)
		{
			const FStatCategory& Cat = StatCategories[c];
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
						.Font(BodyFont)
						.ColorAndOpacity(FT66FlatStyle::PrimaryText())
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
				const ET66StatType SecType = static_cast<ET66StatType>(Cat.Indices[k]);
				const FText Label = Loc ? Loc->GetText_StatName(SecType) : FText::GetEmpty();
				const FText Description = Loc ? Loc->GetText_StatDescription(SecType) : FText::GetEmpty();
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
						.Font(BodyFont)
						.ColorAndOpacity(FT66FlatStyle::PrimaryText())
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
		.Font(HeadingFont)
		.ColorAndOpacity(FT66FlatStyle::PrimaryText())
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
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(16.f),
				Content)
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
	Options.bExtended = Snapshot && Snapshot->StatValues.Num() > 0;
	return MakeEssentialStatsPanelFromSnapshotWithOptions(Snapshot, Loc, Options);
}

TSharedRef<SWidget> T66StatsPanelSlate::MakeEssentialStatsPanelFromSnapshotWithOptions(
	UT66LeaderboardRunSummarySaveGame* Snapshot,
	UT66LocalizationSubsystem* Loc,
	const FT66SnapshotStatsPanelOptions& Options)
{
	const FText HeaderText = NSLOCTEXT("T66.StatsPanel", "Header", "STATS");
	const FText StatFmt = Loc ? Loc->GetText_StatLineFormat() : NSLOCTEXT("T66.Stats", "StatLineFormat", "{0}: {1}");
	const FSlateFontInfo HeadingFont = MakeStatsPanelHeadingFont(Options.FontSizeAdjustment + Options.HeadingFontSizeAdjustment);
	const FSlateFontInfo BodyFont = MakeStatsPanelBodyFont(Options.FontSizeAdjustment);
	const bool bExtended = Options.bExtended;

	TSharedRef<SVerticalBox> StatsBox = SNew(SVerticalBox);

	if (Snapshot)
	{
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
						BodyFont)
				]
			];
		};

		if (bExtended)
		{
			auto AddHorizontalLine = [&]()
			{
				StatsBox->AddSlot().AutoHeight().Padding(0.f, 8.f, 0.f, 8.f)
				[
					FT66FlatStyle::MakeFlatDivider(Orient_Horizontal)
				];
			};

			auto AddCategoryHeader = [&](const FStatCategory& Category)
			{
				StatsBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
				[
					SNew(STextBlock)
					.Text(FormatCategoryHeaderText(Loc, Category))
					.Font(HeadingFont)
					.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				];
			};

			for (int32 c = 0; c < NumStatCategories; ++c)
			{
				const FStatCategory& Cat = StatCategories[c];
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
					const ET66StatType SecType = static_cast<ET66StatType>(Cat.Indices[k]);
					const FText Label = Loc ? Loc->GetText_StatName(SecType) : FText::FromString(TEXT("?"));
					// StatValues is read as accumulated bonus points (the unified "+X%" language).
					// Writers that still store other units are listed in the spec progress log.
					const float* P = Snapshot->StatValues.Find(SecType);
					AddSecondaryLine(
						Label,
						FormatDisplayedStatBonusPercent(P ? *P : 0.f),
						Label,
						Loc ? Loc->GetText_StatDescription(SecType) : FText::GetEmpty());
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
			.Font(HeadingFont)
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
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

	TSharedRef<SWidget> Panel = FT66FlatStyle::MakeFlatPanel(
		ET66FlatState::Default,
		FMargin(16.f),
		Content);

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
