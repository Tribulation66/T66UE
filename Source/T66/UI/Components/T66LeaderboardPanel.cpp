// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
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
#include "Widgets/Input/SComboBox.h"

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

	TypeOptions.Add(MakeShared<FString>(TEXT("High Score")));
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
		static const TSoftObjectPtr<UTexture2D> GlobalTexSoft(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_Leaderboard_Global.T_Leaderboard_Global")));
		static const TSoftObjectPtr<UTexture2D> FriendsTexSoft(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_Leaderboard_Friends.T_Leaderboard_Friends")));

		T66SlateTexture::BindSharedBrushAsync(TexPool, GlobalTexSoft, UIManager, FilterBrushGlobal, FName("LB_Global"), /*bClearWhileLoading*/ false);
		T66SlateTexture::BindSharedBrushAsync(TexPool, FriendsTexSoft, UIManager, FilterBrushFriends, FName("LB_Friends"), /*bClearWhileLoading*/ false);

		// Streamers GIF frames (individual PNGs: T_Leaderboard_Streamers_Frame_00, _01, _02, ...)
		StreamerFrameBrushes.Empty();
		for (int32 i = 0; i < 10; ++i)
		{
			const FString FramePathStr = FString::Printf(TEXT("/Game/UI/Leaderboard/T_Leaderboard_Streamers_Frame_%02d.T_Leaderboard_Streamers_Frame_%02d"), i, i);
			const FSoftObjectPath FramePath{FramePathStr};
			const TSoftObjectPtr<UTexture2D> FrameSoft{FramePath};
			TSharedPtr<FSlateBrush> FrameBrush = MakeEmptyBrush();
			StreamerFrameBrushes.Add(FrameBrush);
			T66SlateTexture::BindSharedBrushAsync(TexPool, FrameSoft, UIManager, FrameBrush, FName(*FString::Printf(TEXT("LB_Streamer_%d"), i)), /*bClearWhileLoading*/ false);
		}

		// Fallback single streamer texture (used if no frames loaded)
		static const TSoftObjectPtr<UTexture2D> StreamersSoft(FSoftObjectPath(TEXT("/Game/UI/Leaderboard/T_Leaderboard_Streamers.T_Leaderboard_Streamers")));
		T66SlateTexture::BindSharedBrushAsync(TexPool, StreamersSoft, UIManager, FilterBrushStreamers, FName("LB_Streamers"), /*bClearWhileLoading*/ false);
	}

	// Use the first frame as the initial Streamers brush if available
	if (StreamerFrameBrushes.Num() > 0)
	{
		FilterBrushStreamers = StreamerFrameBrushes[0];
	}

	auto MakeIconButton = [this, SquareIconSize](ET66LeaderboardFilter Filter, FReply (ST66LeaderboardPanel::*ClickHandler)(), const FString& FallbackLetter, TSharedPtr<FSlateBrush> IconBrush, TSharedPtr<SImage>* OutImagePtr = nullptr) -> TSharedRef<SWidget>
	{
		TSharedPtr<SImage> ImgWidget;
		TSharedRef<SWidget> Content = IconBrush.IsValid()
			? StaticCastSharedRef<SWidget>(
				SAssignNew(ImgWidget, SImage)
				.Image(IconBrush.Get())
			)
			: StaticCastSharedRef<SWidget>(
				SNew(STextBlock)
				.Text(FText::FromString(FallbackLetter))
				.Font(FT66Style::Tokens::FontBold(22))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			);

		if (OutImagePtr) *OutImagePtr = ImgWidget;

		return FT66Style::MakeButton(
			FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateSP(this, ClickHandler))
			.SetMinWidth(SquareIconSize).SetHeight(SquareIconSize)
			.SetPadding(FMargin(0.f))
			.SetContent(Content)
		);
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
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
				[ MakeIconButton(ET66LeaderboardFilter::Global, &ST66LeaderboardPanel::HandleGlobalClicked, TEXT("G"), FilterBrushGlobal) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
				[ MakeIconButton(ET66LeaderboardFilter::Friends, &ST66LeaderboardPanel::HandleFriendsClicked, TEXT("F"), FilterBrushFriends) ]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
				[ MakeIconButton(ET66LeaderboardFilter::Streamers, &ST66LeaderboardPanel::HandleStreamersClicked, TEXT("S"), FilterBrushStreamers, &StreamerIconImage) ],
				FT66PanelParams(ET66PanelType::Panel).SetPadding(4.0f)
			)
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
						.Text_Lambda([this]() { return GetPartySizeText(CurrentPartySize); })
						.Font(FT66Style::Tokens::FontRegular(11))
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
						.Text_Lambda([this]() { return GetDifficultyText(CurrentDifficulty); })
						.Font(FT66Style::Tokens::FontRegular(11))
						.ColorAndOpacity(FLinearColor::White)
						]
					]
				]
				// Type dropdown (width fits "High Score" / "Speed Run" without truncation)
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
				[
					SNew(SBox).MinDesiredWidth(120.0f).WidthOverride(120.0f).HeightOverride(30.0f)
					[
						SNew(SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&TypeOptions)
						.OnSelectionChanged(this, &ST66LeaderboardPanel::OnTypeChanged)
						.OnGenerateWidget(this, &ST66LeaderboardPanel::MakeTypeWidget)
						.InitiallySelectedItem(SelectedTypeOption)
						[
						SNew(STextBlock)
						.Text_Lambda([this]() { return GetTypeText(CurrentType); })
						.Font(FT66Style::Tokens::FontRegular(11))
						.ColorAndOpacity(FLinearColor::White)
						.OverflowPolicy(ETextOverflowPolicy::Clip)
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
							.Text_Lambda([this]() { return SelectedStageOption.IsValid() ? FText::FromString(*SelectedStageOption) : FText::AsNumber(1); })
							.Font(FT66Style::Tokens::FontRegular(11))
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
				.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
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
							.Text(NSLOCTEXT("T66.Leaderboard", "Rank", "RANK"))
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
						]
					]
					// Name header
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Leaderboard", "Name", "NAME"))
						.Font(FT66Style::Tokens::FontBold(10))
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
							.Text(NSLOCTEXT("T66.Leaderboard", "HighScore", "HIGH SCORE"))
							.Font(FT66Style::Tokens::FontBold(10))
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
							.Text(NSLOCTEXT("T66.Leaderboard", "Time", "TIME"))
							.Font(FT66Style::Tokens::FontBold(10))
							.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
							.Justification(ETextJustify::Right)
						]
					]
				]
			]
			// Entry list: taller so 11 rows visible without scrolling
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SBox)
				.HeightOverride(480.0f)
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

	// Start Streamers GIF animation if we have multiple frames
	if (StreamerFrameBrushes.Num() > 1 && StreamerIconImage.IsValid())
	{
		StreamerCurrentFrame = 0;
		StreamerFrameAccumulator = 0.0f;
		RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(this, &ST66LeaderboardPanel::TickStreamersAnimation));
	}
}

EActiveTimerReturnType ST66LeaderboardPanel::TickStreamersAnimation(double /*InCurrentTime*/, float InDeltaTime)
{
	if (StreamerFrameBrushes.Num() <= 1 || !StreamerIconImage.IsValid())
	{
		return EActiveTimerReturnType::Stop;
	}
	StreamerFrameAccumulator += InDeltaTime;
	const float Period = 1.0f / StreamerFPS;
	if (StreamerFrameAccumulator >= Period)
	{
		StreamerFrameAccumulator -= Period;
		StreamerCurrentFrame = (StreamerCurrentFrame + 1) % StreamerFrameBrushes.Num();
		StreamerIconImage->SetImage(StreamerFrameBrushes[StreamerCurrentFrame].Get());
	}
	return EActiveTimerReturnType::Continue;
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
		
		if (CurrentType == ET66LeaderboardType::HighScore)
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

		const bool bHasLocalSummary =
			LeaderboardSubsystem
			&& (CurrentType == ET66LeaderboardType::HighScore)
			&& LeaderboardSubsystem->HasLocalBestBountyRunSummary(CurrentDifficulty, CurrentPartySize);

		const bool bRowClickable =
			Entry.bIsLocalPlayer
			&& (CurrentType == ET66LeaderboardType::HighScore)
			&& (UIManager != nullptr)
			&& bHasLocalSummary;

		// Build the row content once.
		const TSharedRef<SWidget> RowContents =
			SNew(SBorder)
			.BorderImage(RowBrush)
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
						.Text(FText::Format(NSLOCTEXT("T66.Leaderboard", "RankFormat", "#{0}"), FText::AsNumber(Entry.Rank)))
						.Font(FT66Style::Tokens::FontBold(13))
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
					.Font(FT66Style::Tokens::FontRegular(12))
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
						.Font(FT66Style::Tokens::FontRegular(11))
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
						.Font(FT66Style::Tokens::FontRegular(11))
						.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.9f, 1.0f))
						.Justification(ETextJustify::Right)
					]
				]
			];

		// Your row should be a full-row button (even if the snapshot doesn't exist yet).
		const bool bIsLocalRowButton = Entry.bIsLocalPlayer;
		TSharedRef<SWidget> RowWidget =
			bIsLocalRowButton
			? FT66Style::MakeButton(
				FT66ButtonParams(FText::GetEmpty(), FOnClicked::CreateLambda([this, Entry]() { return HandleLocalEntryClicked(Entry); }))
				.SetMinWidth(0.f)
				.SetPadding(FMargin(0.f))
				.SetColor(FLinearColor::Transparent)
				.SetContent(RowContents)
			)
			: RowContents;

		EntryListBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 2.0f)
		[
			RowWidget
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
		LeaderboardEntries = LeaderboardSubsystem->BuildEntriesForFilter(
			CurrentFilter, CurrentType, CurrentDifficulty, CurrentPartySize, CurrentSpeedRunStage);
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

	if (*NewSelection == TEXT("High Score")) SetLeaderboardType(ET66LeaderboardType::HighScore);
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
	ET66PartySize Size = ET66PartySize::Solo;
	if (InOption.IsValid())
	{
		if (*InOption == TEXT("Duo")) Size = ET66PartySize::Duo;
		else if (*InOption == TEXT("Trio")) Size = ET66PartySize::Trio;
	}
	return SNew(STextBlock)
		.Text(GetPartySizeText(Size))
		.Font(FT66Style::Tokens::FontRegular(11))
		.ColorAndOpacity(FLinearColor::White);
}

TSharedRef<SWidget> ST66LeaderboardPanel::MakeDifficultyWidget(TSharedPtr<FString> InOption)
{
	ET66Difficulty Diff = ET66Difficulty::Easy;
	if (InOption.IsValid())
	{
		if (*InOption == TEXT("Medium")) Diff = ET66Difficulty::Medium;
		else if (*InOption == TEXT("Hard")) Diff = ET66Difficulty::Hard;
		else if (*InOption == TEXT("Very Hard")) Diff = ET66Difficulty::VeryHard;
		else if (*InOption == TEXT("Impossible")) Diff = ET66Difficulty::Impossible;
		else if (*InOption == TEXT("Perdition")) Diff = ET66Difficulty::Perdition;
		else if (*InOption == TEXT("Final")) Diff = ET66Difficulty::Final;
	}
	return SNew(STextBlock)
		.Text(GetDifficultyText(Diff))
		.Font(FT66Style::Tokens::FontRegular(11))
		.ColorAndOpacity(FLinearColor::White);
}

TSharedRef<SWidget> ST66LeaderboardPanel::MakeTypeWidget(TSharedPtr<FString> InOption)
{
	ET66LeaderboardType Type = ET66LeaderboardType::HighScore;
	if (InOption.IsValid() && *InOption == TEXT("Speed Run"))
	{
		Type = ET66LeaderboardType::SpeedRun;
	}
	return SNew(STextBlock)
		.Text(GetTypeText(Type))
		.Font(FT66Style::Tokens::FontRegular(11))
		.ColorAndOpacity(FLinearColor::White);
}

TSharedRef<SWidget> ST66LeaderboardPanel::MakeStageWidget(TSharedPtr<FString> InOption)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*InOption))
		.Font(FT66Style::Tokens::FontRegular(11))
		.ColorAndOpacity(FLinearColor::White);
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
	case ET66LeaderboardType::HighScore: return NSLOCTEXT("T66.Leaderboard", "HighScore", "HIGH SCORE");
	case ET66LeaderboardType::SpeedRun: return NSLOCTEXT("T66.Leaderboard", "SpeedRun", "SPEED RUN");
	default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
	}
}

FReply ST66LeaderboardPanel::HandleLocalEntryClicked(const FLeaderboardEntry& Entry)
{
	if (!Entry.bIsLocalPlayer || !LeaderboardSubsystem || !UIManager)
	{
		return FReply::Unhandled();
	}

	// v0: only local-best bounty can open a run summary snapshot.
	if (CurrentType != ET66LeaderboardType::HighScore)
	{
		return FReply::Handled();
	}

	if (!LeaderboardSubsystem->HasLocalBestBountyRunSummary(CurrentDifficulty, CurrentPartySize))
	{
		return FReply::Handled();
	}

	LeaderboardSubsystem->RequestOpenLocalBestBountyRunSummary(CurrentDifficulty, CurrentPartySize);
	UIManager->ShowModal(ET66ScreenType::RunSummary);
	return FReply::Handled();
}
