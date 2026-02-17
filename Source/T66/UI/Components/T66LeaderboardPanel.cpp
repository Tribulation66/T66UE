// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66UITypes.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
	// Shared column widths so header and row columns align (Rank | Name | Score/Time).
	constexpr float RankColumnWidth = 50.0f;   // content + padding to name
	constexpr float ScoreColumnWidth = 82.0f; // content + padding, aligned with score digits (used for both Score and Time)
}

void ST66LeaderboardPanel::Construct(const FArguments& InArgs)
{
	LocSubsystem = InArgs._LocalizationSubsystem;
	LeaderboardSubsystem = InArgs._LeaderboardSubsystem;
	UIManager = InArgs._UIManager;

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

	TypeOptions.Add(MakeShared<FString>(TEXT("Score")));
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

	const float SquareIconSize = 48.0f;

	// --- Texture pool: async-load icon textures with GC-safe ownership ---
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	if (UIManager)
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(UIManager))
		{
			TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		}
	}

	auto MakeEmptyBrush = []() -> TSharedPtr<FSlateBrush>
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = FVector2D(48.0, 48.0);
		return Brush;
	};

	FilterBrushGlobal = MakeEmptyBrush();
	FilterBrushFriends = MakeEmptyBrush();
	FilterBrushStreamers = MakeEmptyBrush();

	if (TexPool)
	{
		// Static icons imported from SourceAssets/Icons/ via Scripts/ImportLeaderboardIcons.py
		static const TSoftObjectPtr<UTexture2D> GlobalTexSoft(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Global.T_LB_Global")));
		static const TSoftObjectPtr<UTexture2D> FriendsTexSoft(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Friends.T_LB_Friends")));
		static const TSoftObjectPtr<UTexture2D> StreamersTexSoft(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_LB_Streamers.T_LB_Streamers")));

		T66SlateTexture::BindSharedBrushAsync(TexPool, GlobalTexSoft, UIManager, FilterBrushGlobal, FName("LB_Global"), /*bClearWhileLoading*/ false);
		T66SlateTexture::BindSharedBrushAsync(TexPool, FriendsTexSoft, UIManager, FilterBrushFriends, FName("LB_Friends"), /*bClearWhileLoading*/ false);
		T66SlateTexture::BindSharedBrushAsync(TexPool, StreamersTexSoft, UIManager, FilterBrushStreamers, FName("LB_Streamers"), /*bClearWhileLoading*/ false);
	}

	auto MakeIconButton = [this, SquareIconSize](ET66LeaderboardFilter Filter, FReply (ST66LeaderboardPanel::*ClickHandler)(), const FString& FallbackLetter, TSharedPtr<FSlateBrush> IconBrush) -> TSharedRef<SWidget>
	{
		// Raw SButton: sprite fills the entire button area edge-to-edge (no content padding),
		// while the surrounding MakePanel provides the border styling.
		return SNew(SBox)
			.WidthOverride(SquareIconSize)
			.HeightOverride(SquareIconSize)
			[
				SNew(SButton)
				.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.ContentPadding(FMargin(0.f))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.OnClicked(FOnClicked::CreateSP(this, ClickHandler))
				[
					IconBrush.IsValid()
					? StaticCastSharedRef<SWidget>(
						SNew(SImage)
						.Image(IconBrush.Get())
					)
					: StaticCastSharedRef<SWidget>(
						SNew(STextBlock)
						.Text(FText::FromString(FallbackLetter))
						.Font(FT66Style::Tokens::FontBold(22))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
					)
				]
			];
	};

	auto MakeTimeButton = [this](const FText& Text, ET66LeaderboardTime Time, FReply (ST66LeaderboardPanel::*ClickHandler)()) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(
			FT66ButtonParams(Text, FOnClicked::CreateSP(this, ClickHandler))
			.SetMinWidth(160.f)
			.SetColor(TAttribute<FSlateColor>::CreateLambda([this, Time]() -> FSlateColor {
				return (CurrentTimeFilter == Time) ? FSlateColor(FT66Style::Tokens::Accent2) : FSlateColor(FT66Style::Tokens::Panel2);
			}))
		);
	};

	FText WeeklyText = LocSubsystem ? LocSubsystem->GetText_Weekly() : NSLOCTEXT("T66.Leaderboard", "Weekly", "WEEKLY");
	FText AllTimeText = NSLOCTEXT("T66.Leaderboard", "AllTime", "ALL TIME");

	ChildSlot
	[
		SNew(SHorizontalBox)
		// Left: G, F, S buttons at bottom (to the side of the panel)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Bottom)
		.Padding(0.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
			[
				FT66Style::MakePanel(
					MakeIconButton(ET66LeaderboardFilter::Global, &ST66LeaderboardPanel::HandleGlobalClicked, TEXT("G"), FilterBrushGlobal),
					FT66PanelParams(ET66PanelType::Panel).SetPadding(4.0f)
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
			[
				FT66Style::MakePanel(
					MakeIconButton(ET66LeaderboardFilter::Friends, &ST66LeaderboardPanel::HandleFriendsClicked, TEXT("F"), FilterBrushFriends),
					FT66PanelParams(ET66PanelType::Panel).SetPadding(4.0f)
				)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
			[
				FT66Style::MakePanel(
					MakeIconButton(ET66LeaderboardFilter::Streamers, &ST66LeaderboardPanel::HandleStreamersClicked, TEXT("S"), FilterBrushStreamers),
					FT66PanelParams(ET66PanelType::Panel).SetPadding(4.0f)
				)
			]
		]
		// Right: main leaderboard panel content
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				// Account Status button (only visible when account is restricted)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(0.f, 0.f, 0.f, 10.f)
			[
			FT66Style::MakeButton(
				FT66ButtonParams(
					LocSubsystem ? LocSubsystem->GetText_AccountStatus() : NSLOCTEXT("T66.AccountStatus", "Title", "ACCOUNT STATUS"),
					FOnClicked::CreateLambda([this]()
					{
						if (UIManager)
						{
							UIManager->ShowModal(ET66ScreenType::AccountStatus);
						}
						return FReply::Handled();
					})
				)
				.SetMinWidth(160.f)
				.SetPadding(FMargin(12.f, 6.f))
				.SetColor(FT66Style::Tokens::Panel2)
				.SetEnabled(TAttribute<bool>::CreateLambda([this]() { return UIManager != nullptr; }))
				.SetVisibility(TAttribute<EVisibility>::CreateLambda([this]()
				{
					return (LeaderboardSubsystem && LeaderboardSubsystem->ShouldShowAccountStatusButton())
						? EVisibility::Visible
						: EVisibility::Collapsed;
				}))
			)
			]
			// Time toggles (Weekly | All Time) — larger buttons
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 8.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f)
					[ MakeTimeButton(WeeklyText, ET66LeaderboardTime::Current, &ST66LeaderboardPanel::HandleCurrentClicked) ]
					+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f)
					[ MakeTimeButton(AllTimeText, ET66LeaderboardTime::AllTime, &ST66LeaderboardPanel::HandleAllTimeClicked) ]
				]
			// Dropdowns row (Party Size | Difficulty | Type)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.0f, 0.0f, 0.0f, 16.0f)
			[
				SNew(SHorizontalBox)
				// Party Size dropdown
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					FT66Style::MakeDropdown(FT66DropdownParams(
						SNew(STextBlock)
							.Text_Lambda([this]() { return GetPartySizeText(CurrentPartySize); })
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::Text),
						[this]()
						{
							TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
							for (const TSharedPtr<FString>& Opt : PartySizeOptions)
							{
								if (!Opt.IsValid()) continue;
								TSharedPtr<FString> Captured = Opt;
								Box->AddSlot().AutoHeight()
									[
										FT66Style::MakeButton(FT66ButtonParams(FText::FromString(*Opt), FOnClicked::CreateLambda([this, Captured]()
										{
											OnPartySizeChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}), ET66ButtonType::Neutral).SetMinWidth(0.f))
									];
							}
							return Box;
						}).SetMinWidth(100.f).SetHeight(38.f))
				]
				// Difficulty dropdown
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					FT66Style::MakeDropdown(FT66DropdownParams(
						SNew(STextBlock)
							.Text_Lambda([this]() { return GetDifficultyText(CurrentDifficulty); })
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::Text),
						[this]()
						{
							TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
							for (const TSharedPtr<FString>& Opt : DifficultyOptions)
							{
								if (!Opt.IsValid()) continue;
								TSharedPtr<FString> Captured = Opt;
								Box->AddSlot().AutoHeight()
									[
										FT66Style::MakeButton(FT66ButtonParams(FText::FromString(*Opt), FOnClicked::CreateLambda([this, Captured]()
										{
											OnDifficultyChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}), ET66ButtonType::Neutral).SetMinWidth(0.f))
									];
							}
							return Box;
						}).SetMinWidth(130.f).SetHeight(38.f))
				]
				// Type dropdown (width fits "Score" / "Speed Run" without truncation)
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					FT66Style::MakeDropdown(FT66DropdownParams(
						SNew(STextBlock)
							.Text_Lambda([this]() { return GetTypeText(CurrentType); })
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.OverflowPolicy(ETextOverflowPolicy::Clip),
						[this]()
						{
							TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
							for (const TSharedPtr<FString>& Opt : TypeOptions)
							{
								if (!Opt.IsValid()) continue;
								TSharedPtr<FString> Captured = Opt;
								Box->AddSlot().AutoHeight()
									[
										FT66Style::MakeButton(FT66ButtonParams(FText::FromString(*Opt), FOnClicked::CreateLambda([this, Captured]()
										{
											OnTypeChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}), ET66ButtonType::Neutral).SetMinWidth(0.f))
									];
							}
							return Box;
						}).SetMinWidth(130.f).SetHeight(38.f))
				]
				// Stage dropdown (hidden — wiring kept for future use)
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(70.0f)
					.HeightOverride(38.0f)
					.Visibility(EVisibility::Collapsed)
					[
						FT66Style::MakeDropdown(FT66DropdownParams(
							SNew(STextBlock)
								.Text_Lambda([this]() { return SelectedStageOption.IsValid() ? FText::FromString(*SelectedStageOption) : FText::AsNumber(1); })
								.Font(FT66Style::Tokens::FontBold(16))
								.ColorAndOpacity(FT66Style::Tokens::Text),
							[this]()
							{
								TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
								for (const TSharedPtr<FString>& Opt : StageOptions)
								{
									if (!Opt.IsValid()) continue;
									TSharedPtr<FString> Captured = Opt;
									Box->AddSlot().AutoHeight()
										[
											FT66Style::MakeButton(FT66ButtonParams(FText::FromString(*Opt), FOnClicked::CreateLambda([this, Captured]()
											{
												OnStageChanged(Captured, ESelectInfo::Direct);
												FSlateApplication::Get().DismissAllMenus();
												return FReply::Handled();
											}), ET66ButtonType::Neutral).SetMinWidth(0.f))
										];
								}
								return Box;
							}).SetMinWidth(70.f).SetHeight(38.f))
					]
				]
			]
			// Column headers
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
				.Padding(FMargin(10.0f, 6.0f))
				[
					SNew(SHorizontalBox)
					// Column 1: Rank (no header; fixed width to match row alignment)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(RankColumnWidth)
					]
					// Column 2: Name (fills remaining space)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Leaderboard", "Name", "NAME"))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					// Column 3: Score/Time (fixed width to match row, right-aligned)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(ScoreColumnWidth)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() {
								return CurrentType == ET66LeaderboardType::Score
									? NSLOCTEXT("T66.Leaderboard", "Score", "SCORE")
									: NSLOCTEXT("T66.Leaderboard", "Time", "TIME"); })
							.Font(FT66Style::Tokens::FontBold(16))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Right)
						]
					]
				]
			]
			// Entry list: taller so 11 rows visible without scrolling (height reduced to offset larger dropdowns)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SBox)
				.HeightOverride(464.0f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(EntryListBox, SVerticalBox)
					]
				]
			]
		,
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FT66Style::Tokens::Space3)
		)
		]
	];

	RebuildEntryList();
}

void ST66LeaderboardPanel::SetUIManager(UT66UIManager* InUIManager)
{
	UIManager = InUIManager;
	// No rebuild required; click handlers will now be able to open modals.
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
		
		if (CurrentType == ET66LeaderboardType::Score)
		{
			// High score placeholder (kept as fallback when subsystem isn't available).
			Entry.Score = 2000 - (i * 140) + FMath::RandRange(-20, 20);
			Entry.TimeSeconds = -1.0f; // Not relevant for high score
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
		// No solid row background so obsidian panel texture shows through; local player gets subtle tint
		const FSlateBrush* RowBrush = FCoreStyle::Get().GetBrush("NoBorder");
		FLinearColor RowColor = Entry.bIsLocalPlayer 
			? FLinearColor(0.2f, 0.4f, 0.3f, 0.35f)
			: FLinearColor::Transparent;

		// Format time as MM:SS
		FString TimeStr = FormatTime(Entry.TimeSeconds);

		// Format score with commas
		FString ScoreStr = FString::Printf(TEXT("%lld"), Entry.Score);

		// Display name(s): 1 for solo, 2 for duo, 3 for trio (joined by " · ")
		FString NameDisplay = Entry.PlayerName;
		if (Entry.PlayerNames.Num() > 1)
		{
			NameDisplay = FString::Join(Entry.PlayerNames, TEXT(" · "));
		}
		else if (Entry.PlayerNames.Num() == 1)
		{
			NameDisplay = Entry.PlayerNames[0];
		}

		// Build the row content with same 3 column widths as header (Rank | Name | Score/Time).
		const TSharedRef<SWidget> RowContents =
			SNew(SBorder)
			.BorderImage(RowBrush)
			.BorderBackgroundColor(RowColor)
			.Padding(FMargin(10.0f, 6.0f))
			[
				SNew(SHorizontalBox)
				// Column 1: Rank (fixed width to match header)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(RankColumnWidth)
					[
						SNew(STextBlock)
						.Text(FText::Format(NSLOCTEXT("T66.Leaderboard", "RankFormat", "#{0}"), FText::AsNumber(Entry.Rank)))
						.Font(FT66Style::Tokens::FontBold(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
				// Column 2: Player name(s) (fills remaining space)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(NameDisplay))
					.Font(FT66Style::Tokens::FontRegular(16))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				// Column 3: Score or Time (fixed width to match header, right-aligned)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(ScoreColumnWidth)
					[
						SNew(STextBlock)
						.Text(FText::FromString(CurrentType == ET66LeaderboardType::Score ? ScoreStr : TimeStr))
						.Font(FT66Style::Tokens::FontRegular(16))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Right)
					]
				]
			];

		// Every row is a clickable button (Row style: thin border, transparent bg, HAlign_Fill).
		// Local player row navigates to run summary; other rows are stubs for now.
		TSharedRef<SWidget> RowWidget = FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([this, Entry]() { return HandleEntryClicked(Entry); }), ET66ButtonType::Row)
			.SetMinWidth(0.f)
			.SetPadding(FMargin(0.f))
			.SetContent(RowContents)
		);

		EntryListBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 1.0f)
		[
			RowWidget
		];
	}
}

void ST66LeaderboardPanel::OnBackendLeaderboardReady(const FString& Key)
{
	// Backend data arrived — refresh the panel to pick up cached entries.
	// This runs on the game thread (HTTP callback is dispatched to game thread by FHttpModule).
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::OnBackendRunSummaryReady(const FString& EntryId)
{
	if (EntryId != PendingRunSummaryEntryId || !LeaderboardSubsystem || !UIManager)
	{
		return;
	}

	PendingRunSummaryEntryId.Empty();

	UGameInstance* GI = LeaderboardSubsystem->GetGameInstance();
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend)
	{
		return;
	}

	UT66LeaderboardRunSummarySaveGame* Snap = Backend->GetCachedRunSummary(EntryId);
	if (Snap)
	{
		LeaderboardSubsystem->SetPendingFakeRunSummarySnapshot(Snap);
		UIManager->ShowModal(ET66ScreenType::RunSummary);
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
	// Convert current filters to backend API strings
	auto TypeStr = [this]() -> FString {
		return (CurrentType == ET66LeaderboardType::Score) ? TEXT("score") : TEXT("speedrun");
	};
	auto TimeStr = [this]() -> FString {
		return (CurrentTimeFilter == ET66LeaderboardTime::AllTime) ? TEXT("alltime") : TEXT("weekly");
	};
	auto PartyStr = [this]() -> FString {
		switch (CurrentPartySize) {
		case ET66PartySize::Duo: return TEXT("duo");
		case ET66PartySize::Trio: return TEXT("trio");
		default: return TEXT("solo");
		}
	};
	auto DiffStr = [this]() -> FString {
		switch (CurrentDifficulty) {
		case ET66Difficulty::Medium: return TEXT("medium");
		case ET66Difficulty::Hard: return TEXT("hard");
		case ET66Difficulty::VeryHard: return TEXT("veryhard");
		case ET66Difficulty::Impossible: return TEXT("impossible");
		case ET66Difficulty::Perdition: return TEXT("perdition");
		case ET66Difficulty::Final: return TEXT("final");
		default: return TEXT("easy");
		}
	};
	auto FilterStr = [this]() -> FString {
		switch (CurrentFilter) {
		case ET66LeaderboardFilter::Friends: return TEXT("friends");
		case ET66LeaderboardFilter::Streamers: return TEXT("streamers");
		default: return TEXT("global");
		}
	};

	const FString CacheKey = FString::Printf(TEXT("%s_%s_%s_%s_%s"),
		*TypeStr(), *TimeStr(), *PartyStr(), *DiffStr(), *FilterStr());

	// Try to get backend data
	UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;

	if (Backend && Backend->IsBackendConfigured() && Backend->HasCachedLeaderboard(CacheKey))
	{
		// Use backend data
		LeaderboardEntries = Backend->GetCachedLeaderboard(CacheKey);

		// Splice in local player's entry from local save if not already in the list
		if (LeaderboardSubsystem)
		{
			TArray<FLeaderboardEntry> LocalEntries = LeaderboardSubsystem->BuildEntriesForFilter(
				CurrentFilter, CurrentType, CurrentDifficulty, CurrentPartySize, CurrentSpeedRunStage);

			// Find the local player entry from local data
			const FLeaderboardEntry* LocalYou = nullptr;
			for (const FLeaderboardEntry& E : LocalEntries)
			{
				if (E.bIsLocalPlayer)
				{
					LocalYou = &E;
					break;
				}
			}

			if (LocalYou && LocalYou->Score > 0)
			{
				// Check if local player is already in the backend entries (by checking rank vs score)
				bool bAlreadyInList = false;
				for (const FLeaderboardEntry& E : LeaderboardEntries)
				{
					// Can't compare by SteamID from local, so skip for now; just append as rank 11
				}

				if (!bAlreadyInList)
				{
					FLeaderboardEntry You = *LocalYou;
					You.Rank = 11;
					LeaderboardEntries.Add(You);
				}
			}
		}
	}
	else if (LeaderboardSubsystem)
	{
		// Fall back to local data
		LeaderboardEntries = LeaderboardSubsystem->BuildEntriesForFilter(
			CurrentFilter, CurrentType, CurrentDifficulty, CurrentPartySize, CurrentSpeedRunStage);
	}
	else
	{
		GeneratePlaceholderData();
	}

	RebuildEntryList();

	// Fire async backend fetch if not cached yet
	if (Backend && Backend->IsBackendConfigured() && Backend->HasSteamTicket() && !Backend->HasCachedLeaderboard(CacheKey))
	{
		// Subscribe to data-ready callback (safe: subsystem outlives widget during normal gameplay)
		if (!bBoundToBackendDelegate && Backend)
		{
			Backend->OnLeaderboardDataReady.AddRaw(this, &ST66LeaderboardPanel::OnBackendLeaderboardReady);
			bBoundToBackendDelegate = true;
		}

		Backend->FetchLeaderboard(TypeStr(), TimeStr(), PartyStr(), DiffStr(), FilterStr());
	}
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

	if (*NewSelection == TEXT("Score")) SetLeaderboardType(ET66LeaderboardType::Score);
	else if (*NewSelection == TEXT("Speed Run")) SetLeaderboardType(ET66LeaderboardType::SpeedRun);
}

void ST66LeaderboardPanel::OnStageChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (!NewSelection.IsValid()) return;
	SelectedStageOption = NewSelection;
	CurrentSpeedRunStage = FMath::Clamp(FCString::Atoi(**NewSelection), 1, 10);
	RefreshLeaderboard();
}

FText ST66LeaderboardPanel::GetFilterText(ET66LeaderboardFilter Filter) const
{
	if (!LocSubsystem)
	{
		switch (Filter)
		{
		case ET66LeaderboardFilter::Global: return NSLOCTEXT("T66.Leaderboard", "Global", "GLOBAL");
		case ET66LeaderboardFilter::Friends: return NSLOCTEXT("T66.Leaderboard", "Friends", "FRIENDS");
		case ET66LeaderboardFilter::Streamers: return NSLOCTEXT("T66.Leaderboard", "Streamers", "STREAMERS");
		default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
		}
	}

	switch (Filter)
	{
	case ET66LeaderboardFilter::Global: return LocSubsystem->GetText_Global();
	case ET66LeaderboardFilter::Friends: return LocSubsystem->GetText_Friends();
	case ET66LeaderboardFilter::Streamers: return LocSubsystem->GetText_Streamers();
	default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
	}
}

FText ST66LeaderboardPanel::GetPartySizeText(ET66PartySize Size) const
{
	if (!LocSubsystem)
	{
		switch (Size)
		{
		case ET66PartySize::Solo: return NSLOCTEXT("T66.PartySize", "Solo", "SOLO");
		case ET66PartySize::Duo: return NSLOCTEXT("T66.PartySize", "Duo", "DUO");
		case ET66PartySize::Trio: return NSLOCTEXT("T66.PartySize", "Trio", "TRIO");
		default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
		}
	}

	switch (Size)
	{
	case ET66PartySize::Solo: return LocSubsystem->GetText_Solo();
	case ET66PartySize::Duo: return LocSubsystem->GetText_Duo();
	case ET66PartySize::Trio: return LocSubsystem->GetText_Trio();
	default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
	}
}

FText ST66LeaderboardPanel::GetDifficultyText(ET66Difficulty Diff) const
{
	if (!LocSubsystem)
	{
		switch (Diff)
		{
		case ET66Difficulty::Easy: return NSLOCTEXT("T66.Difficulty", "Easy", "Easy");
		case ET66Difficulty::Medium: return NSLOCTEXT("T66.Difficulty", "Medium", "Medium");
		case ET66Difficulty::Hard: return NSLOCTEXT("T66.Difficulty", "Hard", "Hard");
		case ET66Difficulty::VeryHard: return NSLOCTEXT("T66.Difficulty", "VeryHard", "Very Hard");
		case ET66Difficulty::Impossible: return NSLOCTEXT("T66.Difficulty", "Impossible", "Impossible");
		case ET66Difficulty::Perdition: return NSLOCTEXT("T66.Difficulty", "Perdition", "Perdition");
		case ET66Difficulty::Final: return NSLOCTEXT("T66.Difficulty", "Final", "Final");
		default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
		}
	}

	return LocSubsystem->GetText_Difficulty(Diff);
}

FText ST66LeaderboardPanel::GetTimeText(ET66LeaderboardTime Time) const
{
	switch (Time)
	{
	case ET66LeaderboardTime::Current: return LocSubsystem ? LocSubsystem->GetText_Weekly() : NSLOCTEXT("T66.Leaderboard", "Weekly", "WEEKLY");
	case ET66LeaderboardTime::AllTime: return NSLOCTEXT("T66.Leaderboard", "AllTime", "ALL TIME");
	default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
	}
}

FText ST66LeaderboardPanel::GetTypeText(ET66LeaderboardType Type) const
{
	switch (Type)
	{
	case ET66LeaderboardType::Score: return NSLOCTEXT("T66.Leaderboard", "Score", "SCORE");
	case ET66LeaderboardType::SpeedRun: return NSLOCTEXT("T66.Leaderboard", "SpeedRun", "SPEED RUN");
	default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
	}
}

FReply ST66LeaderboardPanel::HandleEntryClicked(const FLeaderboardEntry& Entry)
{
	if (Entry.bIsLocalPlayer)
	{
		return HandleLocalEntryClicked(Entry);
	}
	if (!LeaderboardSubsystem || !UIManager)
	{
		return FReply::Handled();
	}

	// If this entry has a backend run summary, fetch it from the backend
	if (Entry.bHasRunSummary && !Entry.EntryId.IsEmpty())
	{
		UGameInstance* GI = LeaderboardSubsystem->GetGameInstance();
		UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;

		if (Backend && Backend->IsBackendConfigured())
		{
			// Check if already cached
			if (Backend->HasCachedRunSummary(Entry.EntryId))
			{
				UT66LeaderboardRunSummarySaveGame* Snap = Backend->GetCachedRunSummary(Entry.EntryId);
				if (Snap)
				{
					LeaderboardSubsystem->SetPendingFakeRunSummarySnapshot(Snap);
					UIManager->ShowModal(ET66ScreenType::RunSummary);
					return FReply::Handled();
				}
			}

			// Not cached yet — fire async fetch. When it arrives we'll open via the delegate.
			PendingRunSummaryEntryId = Entry.EntryId;
			if (!bBoundToRunSummaryDelegate)
			{
				Backend->OnRunSummaryReady.AddRaw(this, &ST66LeaderboardPanel::OnBackendRunSummaryReady);
				bBoundToRunSummaryDelegate = true;
			}
			Backend->FetchRunSummary(Entry.EntryId);
			return FReply::Handled();
		}
	}

	const int32 PartyCount = (Entry.PartySize == ET66PartySize::Duo) ? 2 : (Entry.PartySize == ET66PartySize::Trio) ? 3 : 1;

	if (PartyCount == 1)
	{
		// Solo: open Run Summary with one fake snapshot.
		if (UT66LeaderboardRunSummarySaveGame* Snap = LeaderboardSubsystem->CreateFakeRunSummarySnapshot(
			CurrentFilter, CurrentType, CurrentDifficulty, CurrentPartySize,
			Entry.Rank, 0, Entry.PlayerName, Entry.Score, Entry.TimeSeconds))
		{
			LeaderboardSubsystem->SetPendingFakeRunSummarySnapshot(Snap);
			UIManager->ShowModal(ET66ScreenType::RunSummary);
		}
		return FReply::Handled();
	}

	// Duo or Trio: open Pick the Player with 2 or 3 fake snapshots.
	TArray<UT66LeaderboardRunSummarySaveGame*> Snapshots;
	for (int32 i = 0; i < PartyCount; ++i)
	{
		const FString DisplayName = Entry.PlayerNames.IsValidIndex(i) ? Entry.PlayerNames[i] : Entry.PlayerName;
		if (UT66LeaderboardRunSummarySaveGame* Snap = LeaderboardSubsystem->CreateFakeRunSummarySnapshot(
			CurrentFilter, CurrentType, CurrentDifficulty, CurrentPartySize,
			Entry.Rank, i, DisplayName, Entry.Score, Entry.TimeSeconds))
		{
			Snapshots.Add(Snap);
		}
	}
	if (Snapshots.Num() == PartyCount)
	{
		LeaderboardSubsystem->SetPendingPickerSnapshots(Snapshots);
		UIManager->ShowModal(ET66ScreenType::PlayerSummaryPicker);
	}
	return FReply::Handled();
}

FReply ST66LeaderboardPanel::HandleLocalEntryClicked(const FLeaderboardEntry& Entry)
{
	if (!Entry.bIsLocalPlayer || !LeaderboardSubsystem || !UIManager)
	{
		return FReply::Unhandled();
	}

	// v0: only local-best bounty can open a run summary snapshot.
	if (CurrentType != ET66LeaderboardType::Score)
	{
		return FReply::Handled();
	}

	if (!LeaderboardSubsystem->HasLocalBestScoreRunSummary(CurrentDifficulty, CurrentPartySize))
	{
		return FReply::Handled();
	}

	LeaderboardSubsystem->RequestOpenLocalBestScoreRunSummary(CurrentDifficulty, CurrentPartySize);
	UIManager->ShowModal(ET66ScreenType::RunSummary);
	return FReply::Handled();
}
