// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66MinigameMenuLayout.h"

#include "Core/T66BackendSubsystem.h"
#include "Data/T66DataTypes.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FLinearColor MinigamePanelFill(0.025f, 0.026f, 0.032f, 0.92f);
	const FLinearColor MinigamePanelInner(0.040f, 0.042f, 0.052f, 0.96f);
	const FLinearColor MinigameText(0.94f, 0.92f, 0.86f, 1.0f);
	const FLinearColor MinigameMuted(0.66f, 0.66f, 0.68f, 1.0f);

	FString MakeMinigameTagSegment(const FText& Label)
	{
		FString Segment = Label.ToString();
		const TCHAR* Removals[] = { TEXT(" "), TEXT("'"), TEXT("\""), TEXT("-"), TEXT(":"), TEXT("/"), TEXT("."), TEXT(",") };
		for (const TCHAR* Removal : Removals)
		{
			Segment.ReplaceInline(Removal, TEXT(""));
		}
		return Segment.IsEmpty() ? FString(TEXT("Unnamed")) : Segment;
	}

	FSlateFontInfo MakeMenuFont(const TCHAR* Weight, const int32 Size)
	{
		const bool bBold = FCString::Stricmp(Weight, TEXT("Regular")) != 0;
		FSlateFontInfo Font = bBold
			? FT66FlatStyle::MakeBoldFont(Size)
			: FT66FlatStyle::MakeFont(Size);
		Font.LetterSpacing = 0;
		return Font;
	}
}

ST66MinigameMenuLayout::~ST66MinigameMenuLayout()
{
	if (UT66BackendSubsystem* Backend = BackendSubsystem.Get())
	{
		Backend->OnLeaderboardDataReady.RemoveAll(this);
		Backend->OnMinigameDailyChallengeReady.RemoveAll(this);
	}
}

void ST66MinigameMenuLayout::Construct(const FArguments& InArgs)
{
	GameID = InArgs._GameID;
	Title = InArgs._Title;
	Subtitle = InArgs._Subtitle;
	DailyTitle = InArgs._DailyTitle;
	DailyBody = InArgs._DailyBody;
	DailyRules = InArgs._DailyRules;
	AccentColor = InArgs._AccentColor;
	BackgroundColor = InArgs._BackgroundColor;
	bLoadGameEnabled = InArgs._LoadGameEnabled;
	OnNewGameClicked = InArgs._OnNewGameClicked;
	OnLoadGameClicked = InArgs._OnLoadGameClicked;
	OnDailyClicked = InArgs._OnDailyClicked;
	OnBackClicked = InArgs._OnBackClicked;
	OnBuildDailyEntries = InArgs._OnBuildDailyEntries;
	OnBuildAllTimeEntries = InArgs._OnBuildAllTimeEntries;
	OnGetDailyStatus = InArgs._OnGetDailyStatus;
	OnGetAllTimeStatus = InArgs._OnGetAllTimeStatus;
	BackendSubsystem = InArgs._BackendSubsystem;
	if (UT66BackendSubsystem* Backend = BackendSubsystem.Get())
	{
		Backend->OnLeaderboardDataReady.AddSP(SharedThis(this), &ST66MinigameMenuLayout::OnBackendLeaderboardReady);
		Backend->OnMinigameDailyChallengeReady.AddSP(SharedThis(this), &ST66MinigameMenuLayout::OnBackendDailyChallengeReady);
	}

	DifficultyOptions.Reset();
	for (const FT66MinigameDifficultyOption& Option : InArgs._DifficultyOptions)
	{
		TSharedPtr<FT66MinigameDifficultyOption> Shared = MakeShared<FT66MinigameDifficultyOption>(Option);
		DifficultyOptions.Add(Shared);
		if (!SelectedDifficulty.IsValid())
		{
			SelectedDifficulty = Shared;
		}
	}
	if (!SelectedDifficulty.IsValid())
	{
		FT66MinigameDifficultyOption Fallback;
		Fallback.DifficultyID = FName(TEXT("Easy"));
		Fallback.DisplayName = NSLOCTEXT("T66.Minigames", "EasyFallback", "Easy");
		SelectedDifficulty = MakeShared<FT66MinigameDifficultyOption>(Fallback);
		DifficultyOptions.Add(SelectedDifficulty);
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(BackgroundColor)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.20f))
			]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 48.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(Title)
					.Font(MakeMenuFont(TEXT("Black"), 42))
					.ColorAndOpacity(AccentColor)
					.Justification(ETextJustify::Center)
					.ShadowOffset(FVector2D(2.f, 2.f))
					.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.62f))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Subtitle)
					.Font(MakeMenuFont(TEXT("Regular"), 15))
					.ColorAndOpacity(MinigameMuted)
					.Justification(ETextJustify::Center)
				]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(FMargin(20.f, 20.f, 0.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(210.f)
				.HeightOverride(44.f)
				[
					MakeMenuButton(NSLOCTEXT("T66.Minigames", "Back", "BACK"), OnBackClicked, true, 44.f, ET66ButtonType::Neutral, false, FName(TEXT("MinigameMenu.Button.Back")))
				]
			]
			+ SOverlay::Slot().Padding(FMargin(20.f, 150.f, 20.f, 34.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(390.f)[BuildDailyPanel()]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SBox)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(24.f, 0.f))
				[
					SNew(SBox).WidthOverride(430.f)[BuildCenterPanel()]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SBox)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(430.f)[BuildLeaderboardPanel()]
				]
			]
		]
	];

	RefreshLeaderboardWidgets();
}

FName ST66MinigameMenuLayout::GetSelectedDifficultyID() const
{
	return SelectedDifficulty.IsValid() ? SelectedDifficulty->DifficultyID : FName(TEXT("Easy"));
}

TSharedRef<SWidget> ST66MinigameMenuLayout::BuildDailyPanel() const
{
	return MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(DailyTitle)
			.Font(MakeMenuFont(TEXT("Bold"), 22))
			.ColorAndOpacity(AccentColor)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(DailyBody)
			.Font(MakeMenuFont(TEXT("Regular"), 14))
			.ColorAndOpacity(MinigameText)
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 18.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.Minigames", "DailyRulesHeader", "Rules"))
			.Font(MakeMenuFont(TEXT("Bold"), 14))
			.ColorAndOpacity(MinigameMuted)
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 8.f, 0.f, 0.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text(DailyRules)
				.Font(MakeMenuFont(TEXT("Regular"), 13))
				.ColorAndOpacity(MinigameMuted)
				.AutoWrapText(true)
			]
		],
		FMargin(22.f, 20.f));
}

TSharedRef<SWidget> ST66MinigameMenuLayout::BuildCenterPanel() const
{
	return MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeMenuButton(NSLOCTEXT("T66.Minigames", "NewGame", "NEW GAME"), OnNewGameClicked, true, 72.f, ET66ButtonType::Primary, true, FName(TEXT("MinigameMenu.Button.NewGame")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
		[
			MakeMenuButton(NSLOCTEXT("T66.Minigames", "LoadGame", "LOAD GAME"), OnLoadGameClicked, bLoadGameEnabled, 72.f, ET66ButtonType::Neutral, false, FName(TEXT("MinigameMenu.Button.LoadGame")))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 16.f, 0.f, 0.f)
		[
			MakeMenuButton(NSLOCTEXT("T66.Minigames", "Daily", "DAILY"), OnDailyClicked, true, 72.f, ET66ButtonType::Primary, false, FName(TEXT("MinigameMenu.Button.DailyRun")))
		],
		FMargin(22.f, 20.f));
}

TSharedRef<SWidget> ST66MinigameMenuLayout::BuildLeaderboardPanel()
{
	return MakePanel(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.Minigames", "DailyChallengeLeaderboard", "DAILY CHALLENGE LEADERBOARD"))
			.Font(MakeMenuFont(TEXT("Bold"), 20))
			.ColorAndOpacity(AccentColor)
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
		[
			SAssignNew(ScopeButtonsBox, SHorizontalBox)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
		[
			MakeDifficultyDropdown()
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 14.f, 0.f, 0.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(NSLOCTEXT("T66.Minigames", "Rank", "RANK")).Font(MakeMenuFont(TEXT("Bold"), 12)).ColorAndOpacity(MinigameMuted)
			]
			+ SHorizontalBox::Slot().FillWidth(1.f).Padding(20.f, 0.f, 0.f, 0.f)
			[
				SNew(STextBlock).Text(NSLOCTEXT("T66.Minigames", "Name", "NAME")).Font(MakeMenuFont(TEXT("Bold"), 12)).ColorAndOpacity(MinigameMuted)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(NSLOCTEXT("T66.Minigames", "Score", "SCORE")).Font(MakeMenuFont(TEXT("Bold"), 12)).ColorAndOpacity(MinigameMuted)
			]
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 8.f, 0.f, 0.f)
		[
			SAssignNew(LeaderboardRowsBox, SVerticalBox)
		],
		FMargin(22.f, 20.f));
}

TSharedRef<SWidget> ST66MinigameMenuLayout::MakePanel(const TSharedRef<SWidget>& Content, const FMargin& Padding) const
{
	const FMargin ResolvedPadding(
		Padding.Left,
		Padding.Top + 28.f,
		Padding.Right,
		Padding.Bottom + 8.f);
	return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, ResolvedPadding, Content, nullptr, FName(TEXT("MinigameMenu.Panel")));
}

TSharedRef<SWidget> ST66MinigameMenuLayout::MakeMenuButton(
	const FText& Text,
	const FOnClicked& Handler,
	const bool bEnabled,
	const float Height,
	const ET66ButtonType Type,
	const bool bSelected,
	const FName InTag) const
{
	const ET66FlatState State = !bEnabled
		? ET66FlatState::Disabled
		: (bSelected || Type == ET66ButtonType::Primary || Type == ET66ButtonType::Success || Type == ET66ButtonType::ToggleActive
			? ET66FlatState::Selected
			: ET66FlatState::Default);
	return FT66FlatStyle::MakeFlatButton(
		State,
		Text,
		Handler,
		nullptr,
		nullptr,
		FMargin(16.f, 10.f),
		0.f,
		Height,
		bEnabled,
		20,
		InTag.IsNone() ? FName(*FString::Printf(TEXT("MinigameMenu.Button.%s"), *MakeMinigameTagSegment(Text))) : InTag);
}

TSharedRef<SWidget> ST66MinigameMenuLayout::MakeScopeButton(const FText& Text, const ET66MinigameLeaderboardScope Scope)
{
	const bool bSelected = SelectedScope == Scope;
	return MakeMenuButton(
		Text,
		FOnClicked::CreateSP(this, &ST66MinigameMenuLayout::HandleScopeClicked, Scope),
		true,
		57.f,
		bSelected ? ET66ButtonType::Success : ET66ButtonType::Neutral,
		bSelected,
		Scope == ET66MinigameLeaderboardScope::Daily ? FName(TEXT("MinigameMenu.Button.ScopeDaily")) : FName(TEXT("MinigameMenu.Button.ScopeAllTime")));
}

TSharedRef<SWidget> ST66MinigameMenuLayout::MakeDifficultyDropdown()
{
	return FT66FlatStyle::MakeFlatDropdown(
		ET66FlatState::Selected,
		TAttribute<FText>::CreateLambda([this]()
		{
			return SelectedDifficulty.IsValid() ? SelectedDifficulty->DisplayName : FText::GetEmpty();
		}),
		[this]()
		{
			TSharedRef<SVerticalBox> Options = SNew(SVerticalBox);
			for (const TSharedPtr<FT66MinigameDifficultyOption>& Option : DifficultyOptions)
			{
				if (!Option.IsValid())
				{
					continue;
				}
				Options->AddSlot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 4.f)
					[
						FT66FlatStyle::MakeFlatButton(
							Option == SelectedDifficulty ? ET66FlatState::Selected : ET66FlatState::Default,
							Option->DisplayName,
							FOnClicked::CreateLambda([this, Option]()
							{
								OnDifficultyChanged(Option, ESelectInfo::OnMouseClick);
								return FReply::Handled();
							}),
							nullptr,
							nullptr,
							FMargin(10.f, 6.f),
							210.f,
							34.f,
							true,
							13,
							FName(*FString::Printf(TEXT("MinigameMenu.DifficultyOption.%s"), *MakeMinigameTagSegment(Option->DisplayName))),
							FName(TEXT("MinigameDifficulty")))
					];
			}
			return StaticCastSharedRef<SWidget>(Options);
		},
		true,
		0.f,
		42.f,
		14,
		FName(TEXT("MinigameMenu.DifficultyDropdown")));
}

TSharedRef<SWidget> ST66MinigameMenuLayout::MakeLeaderboardRows() const
{
	TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	const TArray<FT66MinigameLeaderboardEntry> Entries = GetCurrentEntries();
	if (Entries.Num() == 0)
	{
		Rows->AddSlot().AutoHeight().Padding(0.f, 12.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(GetCurrentStatus())
			.Font(MakeMenuFont(TEXT("Regular"), 13))
			.ColorAndOpacity(MinigameMuted)
			.AutoWrapText(true)
		];
		return Rows;
	}

	for (const FT66MinigameLeaderboardEntry& Entry : Entries)
	{
		Rows->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(Entry.bIsLocalPlayer ? AccentColor.CopyWithNewOpacity(0.26f) : FLinearColor(0.09f, 0.09f, 0.11f, 0.92f))
			.Padding(FMargin(10.f, 8.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("#%d"), Entry.Rank)))
					.Font(MakeMenuFont(TEXT("Bold"), 14))
					.ColorAndOpacity(MinigameText)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(18.f, 0.f, 12.f, 0.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry.DisplayName))
					.Font(MakeMenuFont(TEXT("Regular"), 14))
					.ColorAndOpacity(Entry.bIsLocalPlayer ? AccentColor : MinigameText)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::AsNumber(Entry.Score))
					.Font(MakeMenuFont(TEXT("Bold"), 14))
					.ColorAndOpacity(AccentColor)
				]
			]
		];
	}

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			Rows
		];
}

void ST66MinigameMenuLayout::RefreshLeaderboardWidgets()
{
	RequestCurrentBackendData();

	if (ScopeButtonsBox.IsValid())
	{
		ScopeButtonsBox->ClearChildren();
		ScopeButtonsBox->AddSlot().FillWidth(1.f).Padding(0.f, 0.f, 6.f, 0.f)
		[
			MakeScopeButton(NSLOCTEXT("T66.Minigames", "DailyScope", "DAILY"), ET66MinigameLeaderboardScope::Daily)
		];
		ScopeButtonsBox->AddSlot().FillWidth(1.f).Padding(6.f, 0.f, 0.f, 0.f)
		[
			MakeScopeButton(NSLOCTEXT("T66.Minigames", "AllTimeScope", "ALL TIME"), ET66MinigameLeaderboardScope::AllTime)
		];
	}

	if (LeaderboardRowsBox.IsValid())
	{
		LeaderboardRowsBox->ClearChildren();
		LeaderboardRowsBox->AddSlot().FillHeight(1.f)
		[
			MakeLeaderboardRows()
		];
	}
}

void ST66MinigameMenuLayout::OnDifficultyChanged(TSharedPtr<FT66MinigameDifficultyOption> NewSelection, ESelectInfo::Type)
{
	if (NewSelection.IsValid())
	{
		SelectedDifficulty = NewSelection;
		RefreshLeaderboardWidgets();
		Invalidate(EInvalidateWidgetReason::Layout);
	}
}

FReply ST66MinigameMenuLayout::HandleScopeClicked(const ET66MinigameLeaderboardScope Scope)
{
	SelectedScope = Scope;
	RefreshLeaderboardWidgets();
	Invalidate(EInvalidateWidgetReason::Layout);
	return FReply::Handled();
}

TArray<FT66MinigameLeaderboardEntry> ST66MinigameMenuLayout::GetCurrentEntries() const
{
	const FName DifficultyID = GetSelectedDifficultyID();
	if (const UT66BackendSubsystem* Backend = BackendSubsystem.Get())
	{
		const FString CacheKey = GetBackendLeaderboardCacheKey();
		if (Backend->HasCachedLeaderboard(CacheKey))
		{
			TArray<FT66MinigameLeaderboardEntry> Entries;
			for (const FLeaderboardEntry& BackendEntry : Backend->GetCachedLeaderboard(CacheKey))
			{
				FT66MinigameLeaderboardEntry& Entry = Entries.AddDefaulted_GetRef();
				Entry.Rank = BackendEntry.Rank;
				Entry.DisplayName = BackendEntry.PlayerName;
				Entry.Score = BackendEntry.Score;
				Entry.bIsLocalPlayer = BackendEntry.bIsLocalPlayer;
			}
			return Entries;
		}
	}

	if (SelectedScope == ET66MinigameLeaderboardScope::Daily && OnBuildDailyEntries.IsBound())
	{
		return OnBuildDailyEntries.Execute(DifficultyID);
	}
	if (SelectedScope == ET66MinigameLeaderboardScope::AllTime && OnBuildAllTimeEntries.IsBound())
	{
		return OnBuildAllTimeEntries.Execute(DifficultyID);
	}
	return {};
}

FText ST66MinigameMenuLayout::GetCurrentStatus() const
{
	const FName DifficultyID = GetSelectedDifficultyID();
	if (const UT66BackendSubsystem* Backend = BackendSubsystem.Get())
	{
		if (Backend->IsBackendConfigured())
		{
			return SelectedScope == ET66MinigameLeaderboardScope::Daily
				? NSLOCTEXT("T66.Minigames", "DailyLeaderboardLoading", "Loading today's daily leaderboard...")
				: NSLOCTEXT("T66.Minigames", "AllTimeLeaderboardLoading", "Loading all-time leaderboard...");
		}
	}

	if (SelectedScope == ET66MinigameLeaderboardScope::Daily && OnGetDailyStatus.IsBound())
	{
		return OnGetDailyStatus.Execute(DifficultyID);
	}
	if (SelectedScope == ET66MinigameLeaderboardScope::AllTime && OnGetAllTimeStatus.IsBound())
	{
		return OnGetAllTimeStatus.Execute(DifficultyID);
	}
	return NSLOCTEXT("T66.Minigames", "LeaderboardPending", "No entries yet.");
}

void ST66MinigameMenuLayout::RequestCurrentBackendData()
{
	UT66BackendSubsystem* Backend = BackendSubsystem.Get();
	if (!Backend || !Backend->IsBackendConfigured())
	{
		return;
	}

	const FString LeaderboardKey = GetBackendLeaderboardCacheKey();
	if (!Backend->HasCachedLeaderboard(LeaderboardKey) && !RequestedBackendLeaderboardKeys.Contains(LeaderboardKey))
	{
		RequestedBackendLeaderboardKeys.Add(LeaderboardKey);
		Backend->FetchMinigameLeaderboard(
			GetBackendGameToken(),
			GetBackendScopeToken(),
			GetBackendDifficultyToken(),
			TEXT("global"));
	}

	if (SelectedScope == ET66MinigameLeaderboardScope::Daily)
	{
		const FString ChallengeKey = GetBackendDailyChallengeCacheKey();
		FT66MinigameDailyChallengeData CachedChallenge;
		if (!Backend->GetCachedMinigameDailyChallenge(ChallengeKey, CachedChallenge)
			&& !RequestedBackendDailyChallengeKeys.Contains(ChallengeKey))
		{
			RequestedBackendDailyChallengeKeys.Add(ChallengeKey);
			Backend->FetchCurrentMinigameDailyChallenge(GetBackendGameToken(), GetBackendDifficultyToken());
		}
	}
}

void ST66MinigameMenuLayout::OnBackendLeaderboardReady(const FString& CacheKey)
{
	if (!CacheKey.Equals(GetBackendLeaderboardCacheKey(), ESearchCase::IgnoreCase))
	{
		return;
	}

	RefreshLeaderboardWidgets();
	Invalidate(EInvalidateWidgetReason::Layout);
}

void ST66MinigameMenuLayout::OnBackendDailyChallengeReady(const FString& CacheKey)
{
	if (!CacheKey.Equals(GetBackendDailyChallengeCacheKey(), ESearchCase::IgnoreCase))
	{
		return;
	}

	RefreshLeaderboardWidgets();
	Invalidate(EInvalidateWidgetReason::Layout);
}

FString ST66MinigameMenuLayout::GetBackendGameToken() const
{
	return GameID.ToString().ToLower();
}

FString ST66MinigameMenuLayout::GetBackendScopeToken() const
{
	return SelectedScope == ET66MinigameLeaderboardScope::Daily ? TEXT("daily") : TEXT("alltime");
}

FString ST66MinigameMenuLayout::GetBackendDifficultyToken() const
{
	return GetSelectedDifficultyID().ToString().ToLower();
}

FString ST66MinigameMenuLayout::GetBackendLeaderboardCacheKey() const
{
	return FString::Printf(
		TEXT("minigame_%s_%s_%s_global"),
		*GetBackendGameToken(),
		*GetBackendScopeToken(),
		*GetBackendDifficultyToken());
}

FString ST66MinigameMenuLayout::GetBackendDailyChallengeCacheKey() const
{
	return UT66BackendSubsystem::MakeMinigameDailyChallengeCacheKey(
		GetBackendGameToken(),
		GetBackendDifficultyToken());
}
