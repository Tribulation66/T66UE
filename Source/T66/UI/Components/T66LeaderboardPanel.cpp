// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"

void ST66LeaderboardPanel::Construct(const FArguments& InArgs)
{
	LocSubsystem = InArgs._LocalizationSubsystem;
	LeaderboardSubsystem = InArgs._LeaderboardSubsystem;

	// Initialize dropdown options
	PartySizeOptions.Add(MakeShared<FString>(TEXT("Solo")));
	PartySizeOptions.Add(MakeShared<FString>(TEXT("Duo")));
	PartySizeOptions.Add(MakeShared<FString>(TEXT("Trio")));
	SelectedPartySizeOption = PartySizeOptions[0];

	DifficultyOptions.Add(MakeShared<FString>(TEXT("Easy")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Medium")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Hard")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Very Hard")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Impossible")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Perdition")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Final")));
	SelectedDifficultyOption = DifficultyOptions[0];

	TypeOptions.Add(MakeShared<FString>(TEXT("Bounty")));
	TypeOptions.Add(MakeShared<FString>(TEXT("Speed Run")));
	SelectedTypeOption = TypeOptions[0];

	// Speed run stage dropdown (only shown when Speed Run type is selected).
	for (int32 S = 1; S <= 10; ++S)
	{
		StageOptions.Add(MakeShared<FString>(FString::FromInt(S)));
	}
	SelectedStageOption = StageOptions.Num() > 0 ? StageOptions[0] : MakeShared<FString>(TEXT("1"));
	CurrentSpeedRunStage = 1;

	RefreshLeaderboard();

	FText TitleText = LocSubsystem ? LocSubsystem->GetText_Leaderboard() : FText::FromString(TEXT("LEADERBOARD"));
	FText GlobalText = LocSubsystem ? LocSubsystem->GetText_Global() : FText::FromString(TEXT("GLOBAL"));
	FText FriendsText = LocSubsystem ? LocSubsystem->GetText_Friends() : FText::FromString(TEXT("FRIENDS"));
	FText StreamersText = LocSubsystem ? LocSubsystem->GetText_Streamers() : FText::FromString(TEXT("STREAMERS"));

	// Use lambdas that capture 'this' and evaluate state dynamically
	auto MakeFilterButton = [this](const FText& Text, ET66LeaderboardFilter Filter, FReply (ST66LeaderboardPanel::*ClickHandler)()) -> TSharedRef<SWidget>
	{
		const FButtonStyle& Neutral = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
		const FTextBlockStyle& Txt = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");

		return SNew(SBox).MinDesiredWidth(120.0f).HeightOverride(32.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateSP(this, ClickHandler))
				.ButtonStyle(&Neutral)
				.ContentPadding(FMargin(10.f, 6.f))
				.ButtonColorAndOpacity_Lambda([this, Filter]() -> FLinearColor {
					return (CurrentFilter == Filter) ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2;
				})
				[
					SNew(STextBlock)
					.Text(Text)
					.TextStyle(&Txt)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			];
	};

	auto MakeTimeButton = [this](const FText& Text, ET66LeaderboardTime Time, FReply (ST66LeaderboardPanel::*ClickHandler)()) -> TSharedRef<SWidget>
	{
		const FButtonStyle& Neutral = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
		const FTextBlockStyle& Txt = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Chip");

		return SNew(SBox).MinDesiredWidth(120.0f).HeightOverride(30.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center).VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateSP(this, ClickHandler))
				.ButtonStyle(&Neutral)
				.ContentPadding(FMargin(10.f, 6.f))
				.ButtonColorAndOpacity_Lambda([this, Time]() -> FLinearColor {
					return (CurrentTimeFilter == Time) ? FT66Style::Tokens::Accent2 : FT66Style::Tokens::Panel2;
				})
				[
					SNew(STextBlock)
					.Text(Text)
					.TextStyle(&Txt)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			];
	};

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FT66Style::Get().GetBrush("T66.Brush.Panel"))
		.Padding(FMargin(FT66Style::Tokens::Space3))
		[
			SNew(SVerticalBox)
			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(TitleText)
				.TextStyle(&FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Heading"))
			]
			// Filter toggles (Global | Friends | Streamers)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
				[
					MakeFilterButton(GlobalText, ET66LeaderboardFilter::Global, &ST66LeaderboardPanel::HandleGlobalClicked)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
				[
					MakeFilterButton(FriendsText, ET66LeaderboardFilter::Friends, &ST66LeaderboardPanel::HandleFriendsClicked)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f)
				[
					MakeFilterButton(StreamersText, ET66LeaderboardFilter::Streamers, &ST66LeaderboardPanel::HandleStreamersClicked)
				]
			]
			// Time toggles (Current | All Time)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					MakeTimeButton(FText::FromString(TEXT("CURRENT")), ET66LeaderboardTime::Current, &ST66LeaderboardPanel::HandleCurrentClicked)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					MakeTimeButton(FText::FromString(TEXT("ALL TIME")), ET66LeaderboardTime::AllTime, &ST66LeaderboardPanel::HandleAllTimeClicked)
				]
			]
			// Dropdowns row (Party Size | Difficulty | Type)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(SHorizontalBox)
				// Party Size dropdown
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(90.0f).HeightOverride(30.0f)
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&PartySizeOptions)
						.OnSelectionChanged(this, &ST66LeaderboardPanel::OnPartySizeChanged)
						.OnGenerateWidget(this, &ST66LeaderboardPanel::MakePartySizeWidget)
						.InitiallySelectedItem(SelectedPartySizeOption)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return FText::FromString(*SelectedPartySizeOption); })
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
				// Difficulty dropdown
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(100.0f).HeightOverride(30.0f)
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&DifficultyOptions)
						.OnSelectionChanged(this, &ST66LeaderboardPanel::OnDifficultyChanged)
						.OnGenerateWidget(this, &ST66LeaderboardPanel::MakeDifficultyWidget)
						.InitiallySelectedItem(SelectedDifficultyOption)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return FText::FromString(*SelectedDifficultyOption); })
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
				// Type dropdown
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(100.0f).HeightOverride(30.0f)
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&TypeOptions)
						.OnSelectionChanged(this, &ST66LeaderboardPanel::OnTypeChanged)
						.OnGenerateWidget(this, &ST66LeaderboardPanel::MakeTypeWidget)
						.InitiallySelectedItem(SelectedTypeOption)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return FText::FromString(*SelectedTypeOption); })
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
				// Stage dropdown (only for Speed Run)
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(70.0f)
					.HeightOverride(30.0f)
					.Visibility_Lambda([this]() { return CurrentType == ET66LeaderboardType::SpeedRun ? EVisibility::Visible : EVisibility::Collapsed; })
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&StageOptions)
						.OnSelectionChanged(this, &ST66LeaderboardPanel::OnStageChanged)
						.OnGenerateWidget(this, &ST66LeaderboardPanel::MakeStageWidget)
						.InitiallySelectedItem(SelectedStageOption)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return SelectedStageOption.IsValid() ? FText::FromString(*SelectedStageOption) : FText::FromString(TEXT("1")); })
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
							.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
			]
			// Column headers
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.12f, 0.12f, 0.18f, 1.0f))
				.Padding(FMargin(10.0f, 6.0f))
				[
					SNew(SHorizontalBox)
					// Rank header
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 10.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(40.0f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("RANK")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
						]
					]
					// Name header
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("NAME")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
					]
					// Score header (only show for High Score)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(10.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(80.0f)
						.Visibility_Lambda([this]() { return CurrentType == ET66LeaderboardType::HighScore ? EVisibility::Visible : EVisibility::Collapsed; })
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("BOUNTY")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
							.Justification(ETextJustify::Right)
						]
					]
					// Time header (only show for Speed Run)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(60.0f)
						.Visibility_Lambda([this]() { return CurrentType == ET66LeaderboardType::SpeedRun ? EVisibility::Visible : EVisibility::Collapsed; })
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("TIME")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
							.Justification(ETextJustify::Right)
						]
					]
				]
			]
			// Entry list
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(EntryListBox, SVerticalBox)
				]
			]
		]
	];

	RebuildEntryList();
}

void ST66LeaderboardPanel::GeneratePlaceholderData()
{
	LeaderboardEntries.Empty();

	// Fake player names
	TArray<FString> FakeNames = {
		TEXT("xX_DarkSlayer_Xx"),
		TEXT("ProGamer2024"),
		TEXT("TribulationMaster"),
		TEXT("SpeedRunner_99"),
		TEXT("CasualHero"),
		TEXT("NightmareKing"),
		TEXT("SilentStorm"),
		TEXT("BlazeFury"),
		TEXT("IronWill"),
		TEXT("ShadowDancer")
	};

	// Generate 10 entries with varying data based on type
	for (int32 i = 0; i < 10; i++)
	{
		FLeaderboardEntry Entry;
		Entry.Rank = i + 1;
		Entry.PlayerName = FakeNames[i];
		
		if (CurrentType == ET66LeaderboardType::HighScore)
		{
			// Bounty placeholder (kept as fallback when subsystem isn't available).
			Entry.Score = 2000 - (i * 140) + FMath::RandRange(-20, 20);
			Entry.TimeSeconds = -1.0f; // Not relevant for bounty
		}
		else
		{
			Entry.Score = 0; // Not relevant for speed run
			// Speed run times - best times at top (shorter is better)
			Entry.TimeSeconds = 900.0f + (i * 90.0f) + FMath::RandRange(-20.0f, 20.0f);
		}
		
		Entry.StageReached = 66 - (i / 3);
		Entry.HeroID = FName(TEXT("Knight"));
		Entry.PartySize = CurrentPartySize;
		Entry.Difficulty = CurrentDifficulty;
		Entry.bIsLocalPlayer = (i == 7); // Pretend player is rank 8
		LeaderboardEntries.Add(Entry);
	}
}

void ST66LeaderboardPanel::RebuildEntryList()
{
	if (!EntryListBox.IsValid()) return;

	EntryListBox->ClearChildren();

	for (const FLeaderboardEntry& Entry : LeaderboardEntries)
	{
		// Row background color - highlight local player
		FLinearColor RowColor = Entry.bIsLocalPlayer 
			? FLinearColor(0.2f, 0.4f, 0.3f, 1.0f)
			: FLinearColor(0.08f, 0.08f, 0.12f, 1.0f);

		// Format time as MM:SS
		FString TimeStr = FormatTime(Entry.TimeSeconds);

		// Format score with commas
		FString ScoreStr = FString::Printf(TEXT("%lld"), Entry.Score);

		EntryListBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			SNew(SBorder)
			.BorderBackgroundColor(RowColor)
			.Padding(FMargin(10.0f, 8.0f))
			[
				SNew(SHorizontalBox)
				// Rank
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 10.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(40.0f)
					[
						SNew(STextBlock)
						.Text(FText::Format(FText::FromString(TEXT("#{0}")), FText::AsNumber(Entry.Rank)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
						.ColorAndOpacity(Entry.Rank <= 3 ? FLinearColor(1.0f, 0.85f, 0.0f, 1.0f) : FLinearColor::White)
					]
				]
				// Player Name
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry.PlayerName))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.ColorAndOpacity(FLinearColor::White)
				]
				// Score (only for High Score type)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(10.0f, 0.0f)
				[
					SNew(SBox).WidthOverride(80.0f)
					.Visibility(CurrentType == ET66LeaderboardType::HighScore ? EVisibility::Visible : EVisibility::Collapsed)
					[
						SNew(STextBlock)
						.Text(FText::FromString(ScoreStr))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FLinearColor(0.7f, 0.9f, 0.7f, 1.0f))
						.Justification(ETextJustify::Right)
					]
				]
				// Time (only for Speed Run type)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(60.0f)
					.Visibility(CurrentType == ET66LeaderboardType::SpeedRun ? EVisibility::Visible : EVisibility::Collapsed)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TimeStr))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.9f, 1.0f))
						.Justification(ETextJustify::Right)
					]
				]
			]
		];
	}
}

FString ST66LeaderboardPanel::FormatTime(float Seconds) const
{
	if (Seconds < 0.f)
	{
		return FString(TEXT("--:--"));
	}
	int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
	int32 Secs = FMath::FloorToInt(FMath::Fmod(Seconds, 60.0f));
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Secs);
}

void ST66LeaderboardPanel::SetFilter(ET66LeaderboardFilter NewFilter)
{
	CurrentFilter = NewFilter;
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::SetTimeFilter(ET66LeaderboardTime NewTime)
{
	CurrentTimeFilter = NewTime;
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::SetPartySize(ET66PartySize NewPartySize)
{
	CurrentPartySize = NewPartySize;
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::SetDifficulty(ET66Difficulty NewDifficulty)
{
	CurrentDifficulty = NewDifficulty;
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::SetLeaderboardType(ET66LeaderboardType NewType)
{
	CurrentType = NewType;
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::RefreshLeaderboard()
{
	if (LeaderboardSubsystem)
	{
		if (CurrentType == ET66LeaderboardType::HighScore)
		{
			LeaderboardEntries = LeaderboardSubsystem->BuildBountyEntries(CurrentDifficulty, CurrentPartySize);
		}
		else
		{
			LeaderboardEntries = LeaderboardSubsystem->BuildSpeedRunEntries(CurrentDifficulty, CurrentPartySize, CurrentSpeedRunStage);
		}
	}
	else
	{
		GeneratePlaceholderData();
	}
	RebuildEntryList();
}

FReply ST66LeaderboardPanel::HandleGlobalClicked()
{
	SetFilter(ET66LeaderboardFilter::Global);
	return FReply::Handled();
}

FReply ST66LeaderboardPanel::HandleFriendsClicked()
{
	SetFilter(ET66LeaderboardFilter::Friends);
	return FReply::Handled();
}

FReply ST66LeaderboardPanel::HandleStreamersClicked()
{
	SetFilter(ET66LeaderboardFilter::Streamers);
	return FReply::Handled();
}

FReply ST66LeaderboardPanel::HandleCurrentClicked()
{
	SetTimeFilter(ET66LeaderboardTime::Current);
	return FReply::Handled();
}

FReply ST66LeaderboardPanel::HandleAllTimeClicked()
{
	SetTimeFilter(ET66LeaderboardTime::AllTime);
	return FReply::Handled();
}

void ST66LeaderboardPanel::OnPartySizeChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid()) return;
	SelectedPartySizeOption = NewSelection;

	if (*NewSelection == TEXT("Solo")) SetPartySize(ET66PartySize::Solo);
	else if (*NewSelection == TEXT("Duo")) SetPartySize(ET66PartySize::Duo);
	else if (*NewSelection == TEXT("Trio")) SetPartySize(ET66PartySize::Trio);
}

void ST66LeaderboardPanel::OnDifficultyChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid()) return;
	SelectedDifficultyOption = NewSelection;

	if (*NewSelection == TEXT("Easy")) SetDifficulty(ET66Difficulty::Easy);
	else if (*NewSelection == TEXT("Medium")) SetDifficulty(ET66Difficulty::Medium);
	else if (*NewSelection == TEXT("Hard")) SetDifficulty(ET66Difficulty::Hard);
	else if (*NewSelection == TEXT("Very Hard")) SetDifficulty(ET66Difficulty::VeryHard);
	else if (*NewSelection == TEXT("Impossible")) SetDifficulty(ET66Difficulty::Impossible);
	else if (*NewSelection == TEXT("Perdition")) SetDifficulty(ET66Difficulty::Perdition);
	else if (*NewSelection == TEXT("Final")) SetDifficulty(ET66Difficulty::Final);
}

void ST66LeaderboardPanel::OnTypeChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid()) return;
	SelectedTypeOption = NewSelection;

	if (*NewSelection == TEXT("Bounty")) SetLeaderboardType(ET66LeaderboardType::HighScore);
	else if (*NewSelection == TEXT("Speed Run")) SetLeaderboardType(ET66LeaderboardType::SpeedRun);
}

void ST66LeaderboardPanel::OnStageChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid()) return;
	SelectedStageOption = NewSelection;
	CurrentSpeedRunStage = FMath::Clamp(FCString::Atoi(**NewSelection), 1, 10);
	RefreshLeaderboard();
}

TSharedRef<SWidget> ST66LeaderboardPanel::MakePartySizeWidget(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InOption))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
		.ColorAndOpacity(FLinearColor::White);
}

TSharedRef<SWidget> ST66LeaderboardPanel::MakeDifficultyWidget(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InOption))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
		.ColorAndOpacity(FLinearColor::White);
}

TSharedRef<SWidget> ST66LeaderboardPanel::MakeTypeWidget(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InOption))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
		.ColorAndOpacity(FLinearColor::White);
}

TSharedRef<SWidget> ST66LeaderboardPanel::MakeStageWidget(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InOption))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
		.ColorAndOpacity(FLinearColor::White);
}

FText ST66LeaderboardPanel::GetFilterText(ET66LeaderboardFilter Filter) const
{
	if (!LocSubsystem)
	{
		switch (Filter)
		{
		case ET66LeaderboardFilter::Global: return FText::FromString(TEXT("GLOBAL"));
		case ET66LeaderboardFilter::Friends: return FText::FromString(TEXT("FRIENDS"));
		case ET66LeaderboardFilter::Streamers: return FText::FromString(TEXT("STREAMERS"));
		default: return FText::FromString(TEXT("UNKNOWN"));
		}
	}

	switch (Filter)
	{
	case ET66LeaderboardFilter::Global: return LocSubsystem->GetText_Global();
	case ET66LeaderboardFilter::Friends: return LocSubsystem->GetText_Friends();
	case ET66LeaderboardFilter::Streamers: return LocSubsystem->GetText_Streamers();
	default: return FText::FromString(TEXT("UNKNOWN"));
	}
}

FText ST66LeaderboardPanel::GetPartySizeText(ET66PartySize Size) const
{
	if (!LocSubsystem)
	{
		switch (Size)
		{
		case ET66PartySize::Solo: return FText::FromString(TEXT("SOLO"));
		case ET66PartySize::Duo: return FText::FromString(TEXT("DUO"));
		case ET66PartySize::Trio: return FText::FromString(TEXT("TRIO"));
		default: return FText::FromString(TEXT("UNKNOWN"));
		}
	}

	switch (Size)
	{
	case ET66PartySize::Solo: return LocSubsystem->GetText_Solo();
	case ET66PartySize::Duo: return LocSubsystem->GetText_Duo();
	case ET66PartySize::Trio: return LocSubsystem->GetText_Trio();
	default: return FText::FromString(TEXT("UNKNOWN"));
	}
}

FText ST66LeaderboardPanel::GetDifficultyText(ET66Difficulty Diff) const
{
	if (!LocSubsystem)
	{
		switch (Diff)
		{
		case ET66Difficulty::Easy: return FText::FromString(TEXT("EASY"));
		case ET66Difficulty::Medium: return FText::FromString(TEXT("MEDIUM"));
		case ET66Difficulty::Hard: return FText::FromString(TEXT("HARD"));
		case ET66Difficulty::VeryHard: return FText::FromString(TEXT("VERY HARD"));
		case ET66Difficulty::Impossible: return FText::FromString(TEXT("IMPOSSIBLE"));
		case ET66Difficulty::Perdition: return FText::FromString(TEXT("PERDITION"));
		case ET66Difficulty::Final: return FText::FromString(TEXT("FINAL"));
		default: return FText::FromString(TEXT("UNKNOWN"));
		}
	}

	return LocSubsystem->GetText_Difficulty(Diff);
}

FText ST66LeaderboardPanel::GetTimeText(ET66LeaderboardTime Time) const
{
	switch (Time)
	{
	case ET66LeaderboardTime::Current: return FText::FromString(TEXT("CURRENT"));
	case ET66LeaderboardTime::AllTime: return FText::FromString(TEXT("ALL TIME"));
	default: return FText::FromString(TEXT("UNKNOWN"));
	}
}

FText ST66LeaderboardPanel::GetTypeText(ET66LeaderboardType Type) const
{
	switch (Type)
	{
	case ET66LeaderboardType::HighScore: return FText::FromString(TEXT("HIGH SCORE"));
	case ET66LeaderboardType::SpeedRun: return FText::FromString(TEXT("SPEED RUN"));
	default: return FText::FromString(TEXT("UNKNOWN"));
	}
}
