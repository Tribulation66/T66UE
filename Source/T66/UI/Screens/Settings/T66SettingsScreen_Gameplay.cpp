// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/Settings/T66SettingsScreen_Private.h"

#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"

using namespace T66SettingsScreenPrivate;

namespace
{
	FText GetBeatTargetBuiltinLabel(UT66LocalizationSubsystem* Loc, const ET66BeatTargetSelectionMode Mode)
	{
		switch (Mode)
		{
		case ET66BeatTargetSelectionMode::PersonalBest:
			return NSLOCTEXT("T66.Settings", "BeatTargetPersonalBest", "Personal Best");
		case ET66BeatTargetSelectionMode::FriendsTop:
			return Loc ? Loc->GetText_Friends() : NSLOCTEXT("T66.Settings", "BeatTargetFriends", "Friends");
		case ET66BeatTargetSelectionMode::StreamersTop:
			return Loc ? Loc->GetText_Streamers() : NSLOCTEXT("T66.Settings", "BeatTargetStreamers", "Streamers");
		case ET66BeatTargetSelectionMode::GlobalTop:
		default:
			return Loc ? Loc->GetText_Global() : NSLOCTEXT("T66.Settings", "BeatTargetGlobal", "Global");
		}
	}

	FText GetBeatTargetDifficultyLabel(UT66LocalizationSubsystem* Loc, const ET66Difficulty Difficulty)
	{
		if (!Loc)
		{
			switch (Difficulty)
			{
			case ET66Difficulty::Medium: return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyMedium", "Medium");
			case ET66Difficulty::Hard: return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyHard", "Hard");
			case ET66Difficulty::VeryHard: return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyVeryHard", "Very Hard");
			case ET66Difficulty::Impossible: return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyImpossible", "Impossible");
			case ET66Difficulty::Easy:
			default:
				return NSLOCTEXT("T66.Settings", "BeatTargetDifficultyEasy", "Easy");
			}
		}

		switch (Difficulty)
		{
		case ET66Difficulty::Medium: return Loc->GetText_Medium();
		case ET66Difficulty::Hard: return Loc->GetText_Hard();
		case ET66Difficulty::VeryHard: return Loc->GetText_VeryHard();
		case ET66Difficulty::Impossible: return Loc->GetText_Impossible();
		case ET66Difficulty::Easy:
		default:
			return Loc->GetText_Easy();
		}
	}

	FText GetBeatTargetPartyLabel(UT66LocalizationSubsystem* Loc, const ET66PartySize PartySize)
	{
		if (!Loc)
		{
			switch (PartySize)
			{
			case ET66PartySize::Duo: return NSLOCTEXT("T66.Settings", "BeatTargetPartyDuo", "Duo");
			case ET66PartySize::Trio: return NSLOCTEXT("T66.Settings", "BeatTargetPartyTrio", "Trio");
			case ET66PartySize::Quad: return NSLOCTEXT("T66.Settings", "BeatTargetPartyQuad", "Quad");
			case ET66PartySize::Solo:
			default:
				return NSLOCTEXT("T66.Settings", "BeatTargetPartySolo", "Solo");
			}
		}

		switch (PartySize)
		{
		case ET66PartySize::Duo: return Loc->GetText_Duo();
		case ET66PartySize::Trio: return Loc->GetText_Trio();
		case ET66PartySize::Quad: return Loc->GetText_Quad();
		case ET66PartySize::Solo:
		default:
			return Loc->GetText_Solo();
		}
	}

	FText GetBeatTargetTimeScopeLabel(UT66LocalizationSubsystem* Loc, const ET66LeaderboardTime TimeScope)
	{
		if (TimeScope == ET66LeaderboardTime::Current)
		{
			return Loc ? Loc->GetText_Weekly() : NSLOCTEXT("T66.Settings", "BeatTargetWeekly", "Weekly");
		}

		return NSLOCTEXT("T66.Settings", "BeatTargetAllTime", "All Time");
	}

	FString GetBeatTargetFilterInitial(const ET66LeaderboardFilter Filter)
	{
		switch (Filter)
		{
		case ET66LeaderboardFilter::Friends:
			return TEXT("F");
		case ET66LeaderboardFilter::Streamers:
			return TEXT("S");
		case ET66LeaderboardFilter::Global:
		default:
			return TEXT("G");
		}
	}

	FText MakeFavoriteBeatTargetLabel(UT66LocalizationSubsystem* Loc, const FT66FavoriteLeaderboardRun& Favorite)
	{
		const FString DisplayName = Favorite.DisplayName.TrimStartAndEnd().IsEmpty()
			? NSLOCTEXT("T66.Settings", "FavoriteRunFallbackName", "Saved Run").ToString()
			: Favorite.DisplayName.TrimStartAndEnd();

		return FText::Format(
			NSLOCTEXT("T66.Settings", "FavoriteBeatTargetDisplay", "{0} #{1} {2} | {3} | {4} {5}"),
			FText::FromString(GetBeatTargetFilterInitial(Favorite.Filter)),
			FText::AsNumber(FMath::Max(1, Favorite.Rank)),
			FText::FromString(DisplayName),
			GetBeatTargetTimeScopeLabel(Loc, Favorite.TimeScope),
			GetBeatTargetDifficultyLabel(Loc, Favorite.Difficulty),
			GetBeatTargetPartyLabel(Loc, Favorite.PartySize));
	}

	FText GetBeatTargetSelectionLabel(UT66LocalizationSubsystem* Loc, const FT66BeatTargetSelection& Selection, const TArray<FT66FavoriteLeaderboardRun>& Favorites)
	{
		if (Selection.Mode != ET66BeatTargetSelectionMode::FavoriteRun)
		{
			return GetBeatTargetBuiltinLabel(Loc, Selection.Mode);
		}

		for (const FT66FavoriteLeaderboardRun& Favorite : Favorites)
		{
			if (Favorite.EntryId == Selection.FavoriteEntryId)
			{
				return MakeFavoriteBeatTargetLabel(Loc, Favorite);
			}
		}

		return NSLOCTEXT("T66.Settings", "FavoriteRunMissing", "Missing Favorite Run");
	}
}

TSharedRef<SWidget> UT66SettingsScreen::BuildFlatGameplaySettingsUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;
	const FName SettingsTabsGroup(TEXT("SettingsTabs"));

	auto DTag = [](const TCHAR* Text) -> FName
	{
		return FName(Text);
	};

	auto ChildTag = [](const FName& BaseTag, const TCHAR* Suffix) -> FName
	{
		return FName(*(BaseTag.ToString() + FString(TEXT(".")) + Suffix));
	};

	auto MakeLabel = [](const FName Tag, const TAttribute<FText>& Text, const int32 FontSize, const FLinearColor& Color, const bool bBold = false, const ETextJustify::Type Justify = ETextJustify::Left, const bool bAutoWrap = true) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justify)
			.AutoWrapText(bAutoWrap)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
			Tag,
			TEXT("Label"),
			ET66FlatState::Default,
			TOptional<FLinearColor>(),
			false,
			NAME_None,
			true);
	};

	auto MakeMetadataRegion = [](const FName Tag, const FString& Role) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBox),
			Tag,
			Role,
			ET66FlatState::Default);
	};

	auto MakeFlatTab = [this, &MakeLabel, &ChildTag, SettingsTabsGroup](const FName Tag, const ET66SettingsTab Tab, const ET66FlatState State, const FText& Text, const float Width, const int32 FontSize = 22) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(Tag, TEXT("Label")), Text, FontSize, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center, false)
			],
			FOnClicked::CreateLambda([this, Tab]()
			{
				SwitchToTab(Tab);
				ForceRebuildSlate();
				return FReply::Handled();
			}),
			FMargin(0.f),
			Width,
			0.079f * CanvasH,
			true,
			Tag,
			SettingsTabsGroup);
	};

	auto MakeChoiceButton = [&MakeLabel, &ChildTag](const FName Tag, const FName ToggleGroup, const ET66FlatState State, const FText& Text, TFunction<void()> Action) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			State,
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(Tag, TEXT("Label")), Text, 22, State == ET66FlatState::Selected ? FT66FlatStyle::SelectedText() : FT66FlatStyle::PrimaryText(), true, ETextJustify::Center, false)
			],
			FOnClicked::CreateLambda([Action = MoveTemp(Action)]()
			{
				Action();
				return FReply::Handled();
			}),
			FMargin(0.f),
			120.f,
			58.f,
			true,
			Tag,
			ToggleGroup);
	};

	auto MakeToggleRow = [this, Loc, &MakeLabel, &ChildTag, &MakeChoiceButton](
		const FName RowTag,
		const FText& Label,
		TFunction<bool()> GetValue,
		TFunction<void(bool)> SetValue) -> TSharedRef<SWidget>
	{
		const bool bValue = GetValue();
		const FName ToggleGroup = RowTag;
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Label")), Label, 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(18.f, 0.f, 0.f, 0.f)
			[
				MakeChoiceButton(
					ChildTag(RowTag, TEXT("OnButton")),
					ToggleGroup,
					bValue ? ET66FlatState::Selected : ET66FlatState::Default,
					GetSettingsOnText(Loc),
					[this, SetValue]() { SetValue(true); ForceRebuildSlate(); })
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(10.f, 0.f, 0.f, 0.f)
			[
				MakeChoiceButton(
					ChildTag(RowTag, TEXT("OffButton")),
					ToggleGroup,
					bValue ? ET66FlatState::Default : ET66FlatState::Selected,
					GetSettingsOffText(Loc),
					[this, SetValue]() { SetValue(false); ForceRebuildSlate(); })
			];

		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(28.f, 18.f, 22.f, 18.f), Row, nullptr, RowTag);
	};

	auto MakeDropdownOption = [this](const FName Tag, const FText& Label, TFunction<void()> Action, const bool bSelected) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatButton(
			bSelected ? ET66FlatState::Selected : ET66FlatState::Default,
			Label,
			FOnClicked::CreateLambda([this, Action = MoveTemp(Action)]()
			{
				Action();
				FSlateApplication::Get().DismissAllMenus();
				ForceRebuildSlate();
				return FReply::Handled();
			}),
			nullptr,
			nullptr,
			FMargin(12.f, 8.f),
			360.f,
			44.f,
			true,
			18,
			Tag);
	};

	auto MakeBeatTargetOptions = [this, Loc, PS, &MakeDropdownOption](const FName BaseTag, TFunction<FT66BeatTargetSelection()> GetSelection, TFunction<void(const FT66BeatTargetSelection&)> SetSelection) -> TSharedRef<SWidget>
	{
		TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
		const FT66BeatTargetSelection CurrentSelection = GetSelection();

		auto AddOption = [&Box, &MakeDropdownOption, BaseTag, CurrentSelection, SetSelection](const TCHAR* Suffix, const FT66BeatTargetSelection& Option, const FText& Label)
		{
			const bool bSelected = CurrentSelection.Mode == Option.Mode && CurrentSelection.FavoriteEntryId == Option.FavoriteEntryId;
			Box->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
			[
				MakeDropdownOption(
					FName(*(BaseTag.ToString() + FString(TEXT(".Option.")) + Suffix)),
					Label,
					[SetSelection, Option]() { SetSelection(Option); },
					bSelected)
			];
		};

		FT66BeatTargetSelection PersonalBestSelection;
		PersonalBestSelection.Mode = ET66BeatTargetSelectionMode::PersonalBest;
		AddOption(TEXT("PersonalBest"), PersonalBestSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::PersonalBest));

		FT66BeatTargetSelection FriendsSelection;
		FriendsSelection.Mode = ET66BeatTargetSelectionMode::FriendsTop;
		AddOption(TEXT("Friends"), FriendsSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::FriendsTop));

		FT66BeatTargetSelection StreamersSelection;
		StreamersSelection.Mode = ET66BeatTargetSelectionMode::StreamersTop;
		AddOption(TEXT("Streamers"), StreamersSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::StreamersTop));

		FT66BeatTargetSelection GlobalSelection;
		GlobalSelection.Mode = ET66BeatTargetSelectionMode::GlobalTop;
		AddOption(TEXT("Global"), GlobalSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::GlobalTop));

		if (PS)
		{
			const TArray<FT66FavoriteLeaderboardRun> Favorites = PS->GetFavoriteLeaderboardRuns();
			int32 FavoriteIndex = 0;
			for (const FT66FavoriteLeaderboardRun& Favorite : Favorites)
			{
				if (Favorite.EntryId.IsEmpty())
				{
					continue;
				}

				FT66BeatTargetSelection FavoriteSelection;
				FavoriteSelection.Mode = ET66BeatTargetSelectionMode::FavoriteRun;
				FavoriteSelection.FavoriteEntryId = Favorite.EntryId;
				const FString FavoriteSuffix = FString::Printf(TEXT("Favorite%d"), FavoriteIndex++);
				AddOption(*FavoriteSuffix, FavoriteSelection, MakeFavoriteBeatTargetLabel(Loc, Favorite));
			}
		}

		return Box;
	};

	auto MakeDropdownRow = [Loc, PS, &MakeLabel, &ChildTag, &MakeBeatTargetOptions](
		const FName RowTag,
		const FText& Label,
		TFunction<FT66BeatTargetSelection()> GetSelection,
		TFunction<void(const FT66BeatTargetSelection&)> SetSelection) -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.38f)
			.VAlign(VAlign_Center)
			[
				MakeLabel(ChildTag(RowTag, TEXT("Label")), Label, 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.62f)
			.VAlign(VAlign_Center)
			.Padding(18.f, 0.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatDropdown(
					ET66FlatState::Selected,
					TAttribute<FText>::CreateLambda([Loc, PS, GetSelection]()
					{
						return GetBeatTargetSelectionLabel(
							Loc,
							GetSelection(),
							PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
					}),
					[RowTag, GetSelection, SetSelection, &MakeBeatTargetOptions]()
					{
						return MakeBeatTargetOptions(RowTag, GetSelection, SetSelection);
					},
					true,
					820.f,
					58.f,
					20,
					ChildTag(RowTag, TEXT("Dropdown")))
			];

		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(28.f, 18.f, 22.f, 18.f), Row, nullptr, RowTag);
	};

	auto MakeSliderRow = [this, PS, &MakeLabel, &ChildTag](const FName RowTag) -> TSharedRef<SWidget>
	{
		TSharedRef<SWidget> ValueText = MakeLabel(
			ChildTag(RowTag, TEXT("Value")),
			TAttribute<FText>::CreateLambda([PS]()
			{
				return FText::AsNumber(FMath::RoundToInt(PS ? PS->GetFogIntensityPercent() : 55.f));
			}),
			22,
			FT66FlatStyle::PrimaryText(),
			true,
			ETextJustify::Center,
			false);

		TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.32f)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeLabel(ChildTag(RowTag, TEXT("Label")), NSLOCTEXT("T66.Settings", "NativeFogIntensityLabelFlat", "Native Fog Intensity"), 26, FT66FlatStyle::PrimaryText(), false, ETextJustify::Left, false)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f, 20.f, 0.f)
				[
					MakeLabel(ChildTag(RowTag, TEXT("Description")), NSLOCTEXT("T66.Settings", "NativeFogIntensityBodyFlat", "Controls the strength of the gameplay haze from 0 to 100."), 17, FT66FlatStyle::SecondaryText(), false, ETextJustify::Left, true)
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.68f)
			.VAlign(VAlign_Center)
			.Padding(18.f, 0.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatSlider(
					ET66FlatState::Default,
					0.f,
					100.f,
					TAttribute<float>::CreateLambda([PS]()
					{
						return PS ? PS->GetFogIntensityPercent() : 55.f;
					}),
					FOnFloatValueChanged::CreateLambda([PS](float Value)
					{
						if (PS)
						{
							PS->SetFogIntensityPercent(Value);
						}
					}),
					ValueText,
					ChildTag(RowTag, TEXT("Slider")))
			];

		return FT66FlatStyle::MakeFlatSubPanel(ET66FlatState::Default, FMargin(28.f, 18.f, 22.f, 18.f), Row, nullptr, RowTag);
	};

	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
		.Anchors(FAnchors(X, Y, X, Y))
		.Alignment(FVector2D::ZeroVector)
		.Offset(FMargin(0.f, 0.f, W * CanvasW, H * CanvasH))
		[
			Widget
		];
	};

	TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	auto AddRow = [&Rows](const TSharedRef<SWidget>& Widget, const float Height, const float BottomPadding = 12.f)
	{
		Rows->AddSlot().AutoHeight().Padding(0.f, 0.f, 16.f, BottomPadding)
		[
			SNew(SBox)
			.HeightOverride(Height)
			[
				Widget
			]
		];
	};

	AddRow(MakeToggleRow(
		DTag(TEXT("SettingsGameplay.Rows.PracticeMode")),
		Loc ? Loc->GetText_PracticeMode() : NSLOCTEXT("T66.Settings.Fallback", "Practice Mode", "Practice Mode"),
		[PS]() { return PS ? PS->GetPracticeMode() : false; },
		[PS](const bool bValue) { if (PS) PS->SetPracticeMode(bValue); }),
		102.f);

	AddRow(MakeToggleRow(
		DTag(TEXT("SettingsGameplay.Rows.SubmitAnonymous")),
		Loc ? Loc->GetText_SubmitLeaderboardAnonymous() : NSLOCTEXT("T66.Settings.Fallback", "Submit Leaderboard as Anonymous", "Submit Leaderboard as Anonymous"),
		[PS]() { return PS ? PS->GetSubmitLeaderboardAnonymous() : false; },
		[PS](const bool bValue) { if (PS) PS->SetSubmitLeaderboardAnonymous(bValue); }),
		102.f);

	AddRow(MakeToggleRow(
		DTag(TEXT("SettingsGameplay.Rows.SpeedRunMode")),
		Loc ? Loc->GetText_SpeedRunMode() : NSLOCTEXT("T66.Settings.Fallback", "Speed Run Mode", "Speed Run Mode"),
		[PS]() { return PS ? PS->GetSpeedRunMode() : false; },
		[PS](const bool bValue) { if (PS) PS->SetSpeedRunMode(bValue); }),
		102.f);

	AddRow(MakeToggleRow(
		DTag(TEXT("SettingsGameplay.Rows.ShowTimeToBeat")),
		NSLOCTEXT("T66.Settings", "ShowTimeToBeatLabelFlat", "Show Time to Beat"),
		[PS]() { return PS ? PS->GetShowTimeToBeat() : true; },
		[PS](const bool bValue) { if (PS) PS->SetShowTimeToBeat(bValue); }),
		102.f);

	AddRow(MakeDropdownRow(
		DTag(TEXT("SettingsGameplay.Rows.TimeToBeatSource")),
		NSLOCTEXT("T66.Settings", "TimeToBeatSourceLabelFlat", "Time to Beat Source"),
		[PS]() { return PS ? PS->GetTimeToBeatSelection() : FT66BeatTargetSelection{}; },
		[PS](const FT66BeatTargetSelection& Selection) { if (PS) PS->SetTimeToBeatSelection(Selection); }),
		102.f);

	AddRow(MakeToggleRow(
		DTag(TEXT("SettingsGameplay.Rows.ShowTimePacing")),
		NSLOCTEXT("T66.Settings", "ShowTimePacingLabelFlat", "Show Time Pacing (Only for Global)"),
		[PS]() { return PS ? PS->GetShowTimePacing() : false; },
		[PS](const bool bValue) { if (PS) PS->SetShowTimePacing(bValue); }),
		102.f);

	AddRow(MakeToggleRow(
		DTag(TEXT("SettingsGameplay.Rows.ShowScoreToBeat")),
		NSLOCTEXT("T66.Settings", "ShowScoreToBeatLabelFlat", "Show Score to Beat"),
		[PS]() { return PS ? PS->GetShowScoreToBeat() : true; },
		[PS](const bool bValue) { if (PS) PS->SetShowScoreToBeat(bValue); }),
		102.f);

	AddRow(MakeDropdownRow(
		DTag(TEXT("SettingsGameplay.Rows.ScoreToBeatSource")),
		NSLOCTEXT("T66.Settings", "ScoreToBeatSourceLabelFlat", "Score to Beat Source"),
		[PS]() { return PS ? PS->GetScoreToBeatSelection() : FT66BeatTargetSelection{}; },
		[PS](const FT66BeatTargetSelection& Selection) { if (PS) PS->SetScoreToBeatSelection(Selection); }),
		102.f);

	AddRow(MakeToggleRow(
		DTag(TEXT("SettingsGameplay.Rows.ShowScorePacing")),
		NSLOCTEXT("T66.Settings", "ShowScorePacingLabelFlat", "Show Score Pacing (Only for Global)"),
		[PS]() { return PS ? PS->GetShowScorePacing() : false; },
		[PS](const bool bValue) { if (PS) PS->SetShowScorePacing(bValue); }),
		102.f);

	AddRow(MakeToggleRow(
		DTag(TEXT("SettingsGameplay.Rows.GoonerMode")),
		Loc ? Loc->GetText_GoonerMode() : NSLOCTEXT("T66.Settings.Fallback", "Gooner Mode", "Gooner Mode"),
		[PS]() { return PS ? PS->GetGoonerMode() : false; },
		[PS](const bool bValue) { if (PS) PS->SetGoonerMode(bValue); }),
		102.f);

	AddRow(MakeSliderRow(DTag(TEXT("SettingsGameplay.Rows.NativeFogIntensity"))), 130.f, 0.f);

	AddN(0.000f, 0.000f, 1.000f, 1.000f,
		FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black),
			DTag(TEXT("SettingsGameplay.Background")),
			TEXT("Background"),
			ET66FlatState::Default));

	AddN(0.000f, 0.095f, 1.000f, 0.905f, MakeMetadataRegion(DTag(TEXT("SettingsGameplay.Root")), TEXT("Root")));
	AddN(0.003f, 0.094f, 0.994f, 0.079f, MakeMetadataRegion(DTag(TEXT("SettingsGameplay.SettingsTabs")), TEXT("ToggleGroup.SettingsTabs")));

	AddN(0.003f, 0.094f, 0.119f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGameplay.SettingsTabs.GameplayButton")), ET66SettingsTab::Gameplay, ET66FlatState::Selected, NSLOCTEXT("T66.Settings", "TabGameplayFlatGameplay", "GAMEPLAY"), 0.119f * CanvasW));
	AddN(0.129f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGameplay.SettingsTabs.GraphicsButton")), ET66SettingsTab::Graphics, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabGraphicsFlatGameplay", "GRAPHICS"), 0.118f * CanvasW));
	AddN(0.253f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGameplay.SettingsTabs.ControlsButton")), ET66SettingsTab::Controls, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabControlsFlatGameplay", "CONTROLS"), 0.118f * CanvasW));
	AddN(0.379f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGameplay.SettingsTabs.HUDButton")), ET66SettingsTab::HUD, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabHUDFlatGameplay", "HUD"), 0.118f * CanvasW));
	AddN(0.503f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGameplay.SettingsTabs.MediaViewerButton")), ET66SettingsTab::MediaViewer, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabMediaViewerFlatGameplay", "MEDIA VIEWER"), 0.118f * CanvasW, 18));
	AddN(0.628f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGameplay.SettingsTabs.AudioButton")), ET66SettingsTab::Audio, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabAudioFlatGameplay", "AUDIO"), 0.118f * CanvasW));
	AddN(0.754f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGameplay.SettingsTabs.CrashingButton")), ET66SettingsTab::Crashing, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabCrashingFlatGameplay", "CRASHING"), 0.118f * CanvasW, 20));
	AddN(0.879f, 0.094f, 0.118f, 0.079f, MakeFlatTab(DTag(TEXT("SettingsGameplay.SettingsTabs.RetroFXButton")), ET66SettingsTab::RetroFX, ET66FlatState::Default, NSLOCTEXT("T66.Settings", "TabRetroFXFlatGameplay", "RETRO FX"), 0.118f * CanvasW, 20));

	AddN(0.002f, 0.194f, 0.978f, 0.753f,
		FT66FlatStyle::AttachMetadata(
			SNew(SScrollBox)
			.ScrollBarVisibility(EVisibility::Visible)
			+ SScrollBox::Slot()
			[
				Rows
			],
			DTag(TEXT("SettingsGameplay.ContentScroll")),
			TEXT("ScrollBox"),
			ET66FlatState::Default));

	return SNew(SScaleBox)
		.Stretch(EStretch::ScaleToFit)
		.StretchDirection(EStretchDirection::Both)
		[
			SNew(SBox)
			.WidthOverride(CanvasW)
			.HeightOverride(CanvasH)
			[
				Canvas
			]
		];
}

TSharedRef<SWidget> UT66SettingsScreen::BuildGameplayTab()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66PlayerSettingsSubsystem* PS = GetPlayerSettings();

	auto MakeBeatTargetSourceMenu = [this, Loc, PS](TFunction<void(const FT66BeatTargetSelection&)> SetSelection, TFunction<FText()> GetCurrentValue) -> TFunction<TSharedRef<SWidget>(const TSharedPtr<STextBlock>&)>
	{
		return [this, Loc, PS, SetSelection, GetCurrentValue](const TSharedPtr<STextBlock>& CurrentValueText) -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);

			auto AddOptionButton = [this, &Box, SetSelection, GetCurrentValue, CurrentValueText](const FT66BeatTargetSelection& OptionSelection, const FText& Label)
			{
				Box->AddSlot().AutoHeight()
				[
					FT66Style::MakeDropdownOptionButton(
						Label,
						FOnClicked::CreateLambda([this, SetSelection, OptionSelection, GetCurrentValue, CurrentValueText]()
						{
							SetSelection(OptionSelection);
							if (CurrentValueText.IsValid())
							{
								CurrentValueText->SetText(GetCurrentValue());
							}
							FSlateApplication::Get().DismissAllMenus();
							return FReply::Handled();
						}),
						GetCurrentValue().EqualTo(Label),
						0.f,
						34.f,
						14)
				];
			};

			FT66BeatTargetSelection PersonalBestSelection;
			PersonalBestSelection.Mode = ET66BeatTargetSelectionMode::PersonalBest;
			AddOptionButton(PersonalBestSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::PersonalBest));

			FT66BeatTargetSelection FriendsSelection;
			FriendsSelection.Mode = ET66BeatTargetSelectionMode::FriendsTop;
			AddOptionButton(FriendsSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::FriendsTop));

			FT66BeatTargetSelection StreamersSelection;
			StreamersSelection.Mode = ET66BeatTargetSelectionMode::StreamersTop;
			AddOptionButton(StreamersSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::StreamersTop));

			FT66BeatTargetSelection GlobalSelection;
			GlobalSelection.Mode = ET66BeatTargetSelectionMode::GlobalTop;
			AddOptionButton(GlobalSelection, GetBeatTargetBuiltinLabel(Loc, ET66BeatTargetSelectionMode::GlobalTop));

			if (PS)
			{
				const TArray<FT66FavoriteLeaderboardRun> Favorites = PS->GetFavoriteLeaderboardRuns();
				for (const FT66FavoriteLeaderboardRun& Favorite : Favorites)
				{
					if (Favorite.EntryId.IsEmpty())
					{
						continue;
					}

					FT66BeatTargetSelection FavoriteSelection;
					FavoriteSelection.Mode = ET66BeatTargetSelectionMode::FavoriteRun;
					FavoriteSelection.FavoriteEntryId = Favorite.EntryId;

					AddOptionButton(FavoriteSelection, MakeFavoriteBeatTargetLabel(Loc, Favorite));
				}
			}

			return Box;
		};
	};

	return SNew(SScrollBox)
		.ScrollBarStyle(GetSettingsReferenceScrollBarStyle())
		.ScrollBarVisibility(EVisibility::Visible)
		.ScrollBarThickness(FVector2D(14.f, 14.f))
		.ScrollBarPadding(FMargin(10.f, 0.f, 2.f, 0.f))
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_PracticeMode() : NSLOCTEXT("T66.Settings.Fallback", "Practice Mode", "Practice Mode"),
					[PS]() { return PS ? PS->GetPracticeMode() : false; },
					[PS](bool b) { if (PS) PS->SetPracticeMode(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SubmitLeaderboardAnonymous() : NSLOCTEXT("T66.Settings.Fallback", "Submit Leaderboard as Anonymous", "Submit Leaderboard as Anonymous"),
					[PS]() { return PS ? PS->GetSubmitLeaderboardAnonymous() : false; },
					[PS](bool b) { if (PS) PS->SetSubmitLeaderboardAnonymous(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_SpeedRunMode() : NSLOCTEXT("T66.Settings.Fallback", "Speed Run Mode", "Speed Run Mode"),
					[PS]() { return PS ? PS->GetSpeedRunMode() : false; },
					[PS](bool b) { if (PS) PS->SetSpeedRunMode(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					NSLOCTEXT("T66.Settings", "ShowTimeToBeatLabel", "Show Time to Beat"),
					[PS]() { return PS ? PS->GetShowTimeToBeat() : true; },
					[PS](bool bValue) { if (PS) PS->SetShowTimeToBeat(bValue); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsDropdownRow(
					NSLOCTEXT("T66.Settings", "TimeToBeatSourceLabel", "Time to Beat Source"),
					[Loc, PS]()
					{
						return GetBeatTargetSelectionLabel(
							Loc,
							PS ? PS->GetTimeToBeatSelection() : FT66BeatTargetSelection{},
							PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
					},
					MakeBeatTargetSourceMenu(
						[PS](const FT66BeatTargetSelection& Selection)
						{
							if (PS)
							{
								PS->SetTimeToBeatSelection(Selection);
							}
						},
						[Loc, PS]()
						{
							return GetBeatTargetSelectionLabel(
								Loc,
								PS ? PS->GetTimeToBeatSelection() : FT66BeatTargetSelection{},
								PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
						}),
					0.4f,
					0.6f,
					true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					NSLOCTEXT("T66.Settings", "ShowTimePacingLabel", "Show Time Pacing (Only for Global)"),
					[PS]() { return PS ? PS->GetShowTimePacing() : false; },
					[PS](bool bValue) { if (PS) PS->SetShowTimePacing(bValue); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					NSLOCTEXT("T66.Settings", "ShowScoreToBeatLabel", "Show Score to Beat"),
					[PS]() { return PS ? PS->GetShowScoreToBeat() : true; },
					[PS](bool bValue) { if (PS) PS->SetShowScoreToBeat(bValue); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsDropdownRow(
					NSLOCTEXT("T66.Settings", "ScoreToBeatSourceLabel", "Score to Beat Source"),
					[Loc, PS]()
					{
						return GetBeatTargetSelectionLabel(
							Loc,
							PS ? PS->GetScoreToBeatSelection() : FT66BeatTargetSelection{},
							PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
					},
					MakeBeatTargetSourceMenu(
						[PS](const FT66BeatTargetSelection& Selection)
						{
							if (PS)
							{
								PS->SetScoreToBeatSelection(Selection);
							}
						},
						[Loc, PS]()
						{
							return GetBeatTargetSelectionLabel(
								Loc,
								PS ? PS->GetScoreToBeatSelection() : FT66BeatTargetSelection{},
								PS ? PS->GetFavoriteLeaderboardRuns() : TArray<FT66FavoriteLeaderboardRun>{});
						}),
					0.4f,
					0.6f,
					true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					NSLOCTEXT("T66.Settings", "ShowScorePacingLabel", "Show Score Pacing (Only for Global)"),
					[PS]() { return PS ? PS->GetShowScorePacing() : false; },
					[PS](bool bValue) { if (PS) PS->SetShowScorePacing(bValue); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsToggleRow(
					Loc,
					Loc ? Loc->GetText_GoonerMode() : NSLOCTEXT("T66.Settings.Fallback", "Gooner Mode", "Gooner Mode"),
					[PS]() { return PS ? PS->GetGoonerMode() : false; },
					[PS](bool b) { if (PS) PS->SetGoonerMode(b); }
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				MakeSettingsPercentSliderRow(
					NSLOCTEXT("T66.Settings", "NativeFogIntensityLabel", "Native Fog Intensity"),
					NSLOCTEXT("T66.Settings", "NativeFogIntensityBody", "Controls the strength of the gameplay haze from 0 to 100. 0 disables native fog entirely, 100 is intentionally very heavy."),
					[PS]() { return PS ? PS->GetFogIntensityPercent() : 55.0f; },
					[PS](float Value) { if (PS) PS->SetFogIntensityPercent(Value); },
					NSLOCTEXT("T66.Settings", "GameplayPercentSliderHelp", "Slide from 0 to 100.")
				)
			]
		];
}


