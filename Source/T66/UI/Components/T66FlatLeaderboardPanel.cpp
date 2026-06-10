// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66FlatLeaderboardPanel.h"

#include "Core/T66BackendSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66PlayerSettingsSaveGame.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Core/T66WebImageCache.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Internationalization/Text.h"
#include "Styling/CoreStyle.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66FriendslopStyle.h"
#include "UI/Style/T66RuntimeUIFontAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/T66DemoModeUIUtils.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66UIManager.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float ContentWidth = ST66FlatLeaderboardPanel::GetContentWidth();
	constexpr float PanelContentInset = ST66FlatLeaderboardPanel::GetPanelContentInset();
	constexpr float PanelWidth = ST66FlatLeaderboardPanel::GetPanelWidth();
	constexpr float PanelHeight = ST66FlatLeaderboardPanel::GetPanelHeight();
	constexpr float FilterPanelWidth = PanelWidth;
	constexpr float FilterPanelHeight = 90.0f;
	constexpr float FilterPanelGap = 14.0f;
	constexpr float FilterButtonWidth = 132.0f;
	constexpr float FilterButtonHeight = 58.0f;
	constexpr float FilterButtonGap = 18.0f;
	constexpr float FilterButtonX = (PanelWidth - ((FilterButtonWidth * 3.0f) + (FilterButtonGap * 2.0f))) * 0.5f;
	constexpr float FilterButtonY = 16.0f;
	constexpr float ContentHeight = PanelHeight - FilterPanelHeight - FilterPanelGap;
	constexpr float LeaderboardColumnGap = 10.0f;
	constexpr float DropdownColumnWidth = (ContentWidth - LeaderboardColumnGap) * 0.5f;
	constexpr float LeaderboardRowWidth = ContentWidth;
	constexpr float RowHeight = 42.0f;
	constexpr float FlatRowPortraitSize = 28.0f;
	constexpr int32 VisibleRemoteEntryCount = 10;

	bool IsFlatSyntheticLeaderboardName(const FString& Name)
	{
		const FString Trimmed = Name.TrimStartAndEnd();
		return Trimmed.StartsWith(TEXT("Player_"), ESearchCase::IgnoreCase)
			|| Trimmed.StartsWith(TEXT("STEAM_"), ESearchCase::IgnoreCase)
			|| Trimmed.Equals(TEXT("Steam Player"), ESearchCase::IgnoreCase);
	}

	int32 GetFlatPartyMemberCount(const ET66PartySize PartySize)
	{
		switch (PartySize)
		{
		case ET66PartySize::Duo:
			return 2;
		case ET66PartySize::Trio:
			return 3;
		case ET66PartySize::Quad:
			return 4;
		case ET66PartySize::Solo:
		default:
			return 1;
		}
	}

	FName GetFlatFallbackHeroId(UT66GameInstance* GameInstance, const int32 FallbackIndex)
	{
		if (!GameInstance)
		{
			return NAME_None;
		}

		if (UDataTable* HeroTable = GameInstance->GetHeroDataTable())
		{
			const TArray<FName> RowNames = HeroTable->GetRowNames();
			if (RowNames.Num() > 0)
			{
				return RowNames[FMath::Abs(FallbackIndex) % RowNames.Num()];
			}
		}

		return NAME_None;
	}

	const TCHAR* GetFlatLeaderboardFilterIconPath(const ET66LeaderboardFilter Filter)
	{
		switch (Filter)
		{
		case ET66LeaderboardFilter::Friends:
			return TEXT("RuntimeDependencies/T66/UI/Reference/Screens/MainMenu/BloodyRetro/Elements/leaderboard_filter_friends_icon.png");
		case ET66LeaderboardFilter::Streamers:
			return TEXT("RuntimeDependencies/T66/UI/Reference/Screens/MainMenu/BloodyRetro/Elements/leaderboard_filter_streamers_icon.png");
		case ET66LeaderboardFilter::Global:
		default:
			return TEXT("RuntimeDependencies/T66/UI/Reference/Screens/MainMenu/BloodyRetro/Elements/leaderboard_filter_global_icon.png");
		}
	}

	ET66FriendslopChrome GetFriendslopFilterButtonChrome(const ET66LeaderboardFilter Filter)
	{
		switch (Filter)
		{
		case ET66LeaderboardFilter::Friends:
			return ET66FriendslopChrome::FilterFriendsIconButtonRound06;
		case ET66LeaderboardFilter::Streamers:
			return ET66FriendslopChrome::FilterStreamerIconButtonRound06;
		case ET66LeaderboardFilter::Global:
		default:
			return ET66FriendslopChrome::FilterGlobalIconButtonRound06;
		}
	}

	bool IsFriendslopReferenceFixture()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("T66FriendslopReferenceFixture"));
	}
}

void ST66FlatLeaderboardPanel::Construct(const FArguments& InArgs)
{
	LocalizationSubsystem = InArgs._LocalizationSubsystem;
	LeaderboardSubsystem = InArgs._LeaderboardSubsystem;
	UIManager = InArgs._UIManager;
	if (!InArgs._TagPrefix.IsEmpty())
	{
		TagPrefix = InArgs._TagPrefix;
	}

	DefaultAvatarBrush = MakeShared<FSlateBrush>();
	DefaultAvatarBrush->DrawAs = ESlateBrushDrawType::Image;
	DefaultAvatarBrush->ImageSize = FVector2D(FlatRowPortraitSize, FlatRowPortraitSize);
	DefaultAvatarBrush->TintColor = FSlateColor(FLinearColor(0.14f, 0.15f, 0.17f, 0.36f));

	StatusText = NSLOCTEXT("T66.FlatLeaderboard", "Loading", "Loading leaderboard...");
	DedicatedLocalEntry = MakeDedicatedLocalEntryPlaceholder();

	ChildSlot
	[
		BuildPanel()
	];

	RefreshLeaderboard();
}

ST66FlatLeaderboardPanel::~ST66FlatLeaderboardPanel()
{
	if (UT66BackendSubsystem* Backend = BoundBackendSubsystem.Get())
	{
		if (bBoundToLeaderboardDelegate)
		{
			Backend->OnLeaderboardDataReady.RemoveAll(this);
		}
		if (bBoundToMyRankDelegate)
		{
			Backend->OnMyRankDataReady.RemoveAll(this);
		}
		if (bBoundToRunSummaryDelegate)
		{
			Backend->OnRunSummaryReady.RemoveAll(this);
		}
		Backend->OnStreamerRequestDataReady.RemoveAll(this);
	}

	ReleaseRootedBrushTextures();
}

void ST66FlatLeaderboardPanel::SetUIManager(UT66UIManager* InUIManager)
{
	UIManager = InUIManager;
}

UGameInstance* ST66FlatLeaderboardPanel::GetGameInstance() const
{
	if (UT66LeaderboardSubsystem* Leaderboard = LeaderboardSubsystem.Get())
	{
		return Leaderboard->GetGameInstance();
	}
	if (UT66LocalizationSubsystem* Loc = LocalizationSubsystem.Get())
	{
		return Loc->GetGameInstance();
	}
	return nullptr;
}

UT66BackendSubsystem* ST66FlatLeaderboardPanel::GetBackendSubsystem() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
}

UT66PlayerSettingsSubsystem* ST66FlatLeaderboardPanel::GetPlayerSettingsSubsystem() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
}

UT66SteamHelper* ST66FlatLeaderboardPanel::GetSteamHelper() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
}

UT66WebImageCache* ST66FlatLeaderboardPanel::GetWebImageCache() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UT66WebImageCache>() : nullptr;
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildPanel()
{
	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);

	Canvas->AddSlot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(0.f, 0.f, FilterPanelWidth, FilterPanelHeight))
		[
			FT66FriendslopStyle::MakeCustomSurface(
				TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/panel_cap.png"),
				FMargin(0.18f, 0.24f),
				ESlateBrushDrawType::Box,
				FVector2D(360.f, 120.f),
				ET66FlatState::Default,
				FMargin(0.f),
				SNew(SBox),
				nullptr,
				Tag(TEXT("FilterPanel")),
				TEXT("FilterPanel"))
		];

	Canvas->AddSlot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(FilterButtonX, FilterButtonY, FilterButtonWidth, FilterButtonHeight))
		[
			BuildFilterButton(ET66LeaderboardFilter::Global, NSLOCTEXT("T66.FlatLeaderboard", "World", "WORLD"), TEXT("FilterWorldButton"))
		];

	Canvas->AddSlot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(FilterButtonX + FilterButtonWidth + FilterButtonGap, FilterButtonY, FilterButtonWidth, FilterButtonHeight))
		[
			BuildFilterButton(ET66LeaderboardFilter::Friends, NSLOCTEXT("T66.FlatLeaderboard", "Friends", "FRIENDS"), TEXT("FilterFriendsButton"))
		];

	Canvas->AddSlot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(FilterButtonX + ((FilterButtonWidth + FilterButtonGap) * 2.f), FilterButtonY, FilterButtonWidth, FilterButtonHeight))
		[
			BuildFilterButton(ET66LeaderboardFilter::Streamers, NSLOCTEXT("T66.FlatLeaderboard", "Stream", "STREAM"), TEXT("FilterStreamersButton"))
		];

	Canvas->AddSlot()
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(0.f, FilterPanelHeight + FilterPanelGap, PanelWidth, ContentHeight))
		[
			BuildContentPanel()
		];

	return FT66FlatStyle::AttachMetadata(Canvas, Tag(TEXT("Root")), TEXT("Leaderboard"), ET66FlatState::Default);
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildContentPanel()
{
	TSharedRef<SHorizontalBox> HeaderRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			FT66FlatStyle::MakeFlatLabel(
				TAttribute<FText>::CreateSP(this, &ST66FlatLeaderboardPanel::GetHeaderText),
				ET66FlatLabelRole::Header,
				ETextJustify::Center,
				Tag(TEXT("LeaderboardHeader")))
		];

	if (CurrentFilter == ET66LeaderboardFilter::Streamers)
	{
		HeaderRow->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				FT66FriendslopStyle::MakeButton(
					bStreamerRequestOpen ? ET66FlatState::Selected : ET66FlatState::Default,
					bStreamerRequestOpen
						? NSLOCTEXT("T66.FlatLeaderboard", "StreamerList", "LIST")
						: NSLOCTEXT("T66.FlatLeaderboard", "StreamerRequest", "REQUEST"),
					FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetStreamerRequestOpen, !bStreamerRequestOpen),
					nullptr,
					nullptr,
					FMargin(10.f, 5.f),
					92.f,
					30.f,
					true,
					13,
					Tag(TEXT("StreamerRequestButton")),
					NAME_None,
					FT66FriendslopStyle::ButtonChromeForState(bStreamerRequestOpen ? ET66FlatState::Selected : ET66FlatState::Default))
			];
	}

	TSharedRef<SHorizontalBox> TimeRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(0.f, 0.f, LeaderboardColumnGap * 0.5f, 0.f)
		[
			BuildTimeButton(ET66LeaderboardTime::Current, NSLOCTEXT("T66.FlatLeaderboard", "Weekly", "WEEKLY"), TEXT("TimeWeeklyButton"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(LeaderboardColumnGap * 0.5f, 0.f, 0.f, 0.f)
		[
			BuildTimeButton(ET66LeaderboardTime::AllTime, NSLOCTEXT("T66.FlatLeaderboard", "AllTime", "ALL TIME"), TEXT("TimeAllTimeButton"))
		];

	TSharedRef<SHorizontalBox> DropdownRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(0.f, 0.f, LeaderboardColumnGap * 0.5f, 0.f)
		[
			BuildPartySizeDropdown()
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(LeaderboardColumnGap * 0.5f, 0.f, 0.f, 0.f)
		[
			BuildDifficultyDropdown()
		];

	TSharedRef<SHorizontalBox> MetricRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(0.f, 0.f, LeaderboardColumnGap * 0.5f, 0.f)
		[
			BuildMetricCheckButton(ET66LeaderboardType::Score, NSLOCTEXT("T66.FlatLeaderboard", "HighScore", "High Score"), TEXT("HighScoreMetricButton"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.Padding(LeaderboardColumnGap * 0.5f, 0.f, 0.f, 0.f)
		[
			BuildMetricCheckButton(ET66LeaderboardType::SpeedRun, NSLOCTEXT("T66.FlatLeaderboard", "SpeedRunMixedCase", "Speed Run"), TEXT("SpeedRunMetricButton"))
		];

	TSharedRef<SVerticalBox> Column = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 28.f)
		[
			SNew(SBox)
			.HeightOverride(32.f)
			[
				HeaderRow
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SBox)
			.HeightOverride(50.f)
			[
				TimeRow
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(SBox)
			.HeightOverride(50.f)
			[
				DropdownRow
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 40.f)
		[
			SNew(SBox)
			.HeightOverride(50.f)
			[
				MetricRow
			]
		];

	if (bStreamerRequestOpen)
	{
		Column->AddSlot()
			.FillHeight(1.f)
			[
				BuildStreamerRequestPanel()
			];
	}
	else
	{
		Column->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 5.f)
			[
				FT66FlatStyle::AttachMetadata(
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
					.Padding(FMargin(12.f, 3.f))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							FT66FlatStyle::MakeFlatLabel(NSLOCTEXT("T66.FlatLeaderboard", "RankHeader", "RANK"), ET66FlatLabelRole::Button, ETextJustify::Left, Tag(TEXT("RankHeader")))
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.Padding(55.f, 0.f, 0.f, 0.f)
						[
							FT66FlatStyle::MakeFlatLabel(NSLOCTEXT("T66.FlatLeaderboard", "NameHeader", "NAME"), ET66FlatLabelRole::Button, ETextJustify::Left, Tag(TEXT("NameHeader")))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							FT66FlatStyle::MakeFlatLabel(TAttribute<FText>::CreateSP(this, &ST66FlatLeaderboardPanel::GetMetricHeaderText), ET66FlatLabelRole::Button, ETextJustify::Right, Tag(TEXT("ScoreHeader")))
						]
					],
					Tag(TEXT("TableHeaderLabels")),
					TEXT("TableHeaderLabels"),
					ET66FlatState::Default)
			];

		Column->AddSlot()
			.FillHeight(1.f)
			[
				BuildRowsPanel()
			];
	}

	return FT66FriendslopStyle::MakeCustomPanel(
		TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/panel_side.png"),
		FMargin(0.10f, 0.08f),
		FVector2D(680.f, 920.f),
		ET66FlatState::Default,
		FMargin(PanelContentInset, 20.f, PanelContentInset, 20.f),
		Column,
		nullptr,
		Tag(TEXT("LeaderboardPanel")));
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildRowsPanel()
{
	SAssignNew(EntryListBox, SVerticalBox);

	TSharedRef<SScrollBox> ScrollBox = SNew(SScrollBox)
		.Orientation(Orient_Vertical)
		.ScrollBarAlwaysVisible(false)
		+ SScrollBox::Slot()
		[
			EntryListBox.ToSharedRef()
		];

	return FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		[
			ScrollBox
		],
		Tag(TEXT("LeaderboardRows")),
		TEXT("Rows"),
		ET66FlatState::Default);
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildFilterButton(const ET66LeaderboardFilter Filter, const FText& Label, const FString& Name)
{
	// UI Reimagine 2026-06-10: square icon-only hellfire filter toggles (approved v7).
	const ET66FlatState State = CurrentFilter == Filter ? ET66FlatState::Selected : ET66FlatState::Default;
	const TCHAR* IconFile =
		Filter == ET66LeaderboardFilter::Friends ? TEXT("ic_friends.png") :
		Filter == ET66LeaderboardFilter::Streamers ? TEXT("ic_play.png") :
		TEXT("ic_globe.png");
	const FString HellfireDir = TEXT("RuntimeDependencies/T66/UI/FriendslopStyle/Hellfire/MainMenu/");
	const TSharedRef<SWidget> Content = FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		.WidthOverride(40.f)
		.HeightOverride(40.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(FT66FriendslopStyle::GetCustomBrush(HellfireDir + IconFile, FMargin(0.f), ESlateBrushDrawType::Image, FVector2D(40.f, 40.f)))
			.ColorAndOpacity(FLinearColor::White)
		],
		Tag(Name + TEXT(".Icon")),
		TEXT("Icon"),
		State);
	TSharedRef<SWidget> Button = FT66FriendslopStyle::MakeCustomToggleGroupButton(
		HellfireDir + (State == ET66FlatState::Selected ? TEXT("sq_filter_selected.png") : TEXT("sq_filter_idle.png")),
		FMargin(0.22f),
		FVector2D(150.f, 150.f),
		State,
		Content,
		FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetFilter, Filter),
		FMargin(10.f, 7.f),
		FilterButtonWidth,
		FilterButtonHeight,
		true,
		Tag(Name),
		FName(TEXT("MainMenuLeaderboardFilter")));
	Button->SetToolTipText(Label);
	return Button;
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildTypeButton(const ET66LeaderboardType Type, const FText& Label, const FString& Name)
{
	const ET66FlatState State = CurrentType == Type ? ET66FlatState::Selected : ET66FlatState::Default;
	return FT66FriendslopStyle::MakeButton(
		State,
		Label,
		FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetLeaderboardType, Type),
		nullptr,
		nullptr,
		FMargin(12.f, 8.f),
		0.f,
		50.f,
		true,
		18,
		Tag(Name),
		FName(TEXT("MainMenuLeaderboardScope")),
		FT66FriendslopStyle::ButtonChromeForState(State));
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildTimeButton(const ET66LeaderboardTime TimeFilter, const FText& Label, const FString& Name)
{
	const ET66FlatState State = CurrentTimeFilter == TimeFilter ? ET66FlatState::Selected : ET66FlatState::Default;
	return FT66FriendslopStyle::MakeButton(
		State,
		Label,
		FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetTimeFilter, TimeFilter),
		nullptr,
		nullptr,
		FMargin(10.f, 7.f),
		0.f,
		50.f,
		TimeFilter != ET66LeaderboardTime::Daily,
		18,
		Tag(Name),
		FName(TEXT("MainMenuLeaderboardTime")),
		State == ET66FlatState::Selected ? ET66FriendslopChrome::LeaderboardTabRedRound06 : ET66FriendslopChrome::LeaderboardTabDarkRound06);
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildTimeDropdown()
{
	return FT66FlatStyle::MakeFlatDropdown(
		ET66FlatState::Default,
		TAttribute<FText>::CreateSP(this, &ST66FlatLeaderboardPanel::GetTimeDropdownText),
		[this]()
		{
			return BuildTimeMenu();
		},
		false,
		ContentWidth,
		52.f,
		18,
		Tag(TEXT("TypeDropdown")));
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildRuleDropdown()
{
	return FT66FlatStyle::MakeFlatDropdown(
		ET66FlatState::Default,
		TAttribute<FText>::CreateSP(this, &ST66FlatLeaderboardPanel::GetRuleDropdownText),
		[this]()
		{
			return BuildRuleMenu();
		},
		false,
		ContentWidth,
		52.f,
		16,
		Tag(TEXT("ModeDropdown")));
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildPartySizeDropdown()
{
	TSharedRef<SHorizontalBox> ButtonRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(TAttribute<FText>::CreateLambda([this]()
			{
				return PartySizeText(CurrentPartySize);
			}))
					.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(16, true))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8.f, 0.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("v")))
			.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(12, true))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
		];

	TSharedPtr<SComboButton> Combo;
	SAssignNew(Combo, SComboButton)
		.HasDownArrow(false)
		.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder")))
		.ContentPadding(FMargin(0.f))
		.MenuPlacement(MenuPlacement_BelowAnchor)
		.OnGetMenuContent_Lambda([this]()
		{
			return FT66FlatStyle::MakeFlatDropdownMenuPanel(BuildPartySizeMenu(), DropdownColumnWidth);
		})
		.ButtonContent()
		[
			FT66FriendslopStyle::MakeSurface(
				ET66FriendslopChrome::DropdownDarkRound06,
				ET66FlatState::Default,
				FMargin(12.f, 6.f),
				ButtonRow,
				nullptr,
				NAME_None,
				TEXT("DropdownSurface"),
				true,
				NAME_None,
				true)
		];

	return FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		.WidthOverride(DropdownColumnWidth)
		.HeightOverride(50.f)
		[
			Combo.ToSharedRef()
		],
		Tag(TEXT("PartySizeDropdown")),
		TEXT("Dropdown"),
		ET66FlatState::Default,
		TOptional<FLinearColor>(),
		true,
		NAME_None,
		false,
		true);
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildDifficultyDropdown()
{
	TSharedRef<SHorizontalBox> ButtonRow = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(TAttribute<FText>::CreateLambda([this]()
			{
				return DifficultyText(CurrentDifficulty);
			}))
					.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(16, true))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8.f, 0.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("v")))
			.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(12, true))
			.ColorAndOpacity(FT66FlatStyle::PrimaryText())
		];

	TSharedPtr<SComboButton> Combo;
	SAssignNew(Combo, SComboButton)
		.HasDownArrow(false)
		.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder")))
		.ContentPadding(FMargin(0.f))
		.MenuPlacement(MenuPlacement_BelowAnchor)
		.OnGetMenuContent_Lambda([this]()
		{
			return FT66FlatStyle::MakeFlatDropdownMenuPanel(BuildDifficultyMenu(), DropdownColumnWidth);
		})
		.ButtonContent()
		[
			FT66FriendslopStyle::MakeSurface(
				ET66FriendslopChrome::DropdownDarkRound06,
				ET66FlatState::Default,
				FMargin(12.f, 6.f),
				ButtonRow,
				nullptr,
				NAME_None,
				TEXT("DropdownSurface"),
				true,
				NAME_None,
				true)
		];

	return FT66FlatStyle::AttachMetadata(
		SNew(SBox)
		.WidthOverride(DropdownColumnWidth)
		.HeightOverride(50.f)
		[
			Combo.ToSharedRef()
		],
		Tag(TEXT("DifficultyDropdown")),
		TEXT("Dropdown"),
		ET66FlatState::Default,
		TOptional<FLinearColor>(),
		true,
		NAME_None,
		false,
		true);
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildMetricCheckButton(const ET66LeaderboardType Type, const FText& Label, const FString& Name)
{
	const bool bSelected = CurrentType == Type;
	const ET66FlatState State = bSelected ? ET66FlatState::Selected : ET66FlatState::Default;
	TSharedRef<SWidget> Content = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.Padding(FMargin(10.f, 7.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::AttachMetadata(
					SNew(SBox)
					.WidthOverride(24.f)
					.HeightOverride(24.f)
					[
						SNew(SBorder)
						.BorderImage(FT66FriendslopStyle::GetCheckboxBrush(bSelected))
						.Padding(FMargin(0.f))
					],
					Tag(Name + TEXT(".Check")),
					TEXT("CheckboxSquare"),
					State,
					TOptional<FLinearColor>(),
					false,
					NAME_None,
					true)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.f, 0.f, 0.f, 0.f)
			.VAlign(VAlign_Center)
			[
				FT66FlatStyle::MakeFlatLabel(
					Label,
					ET66FlatLabelRole::Button,
					ETextJustify::Left,
					Tag(Name + TEXT(".Label")))
			]
		];

	TSharedRef<SWidget> Button = SNew(SButton)
		.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>(TEXT("NoBorder")))
		.ContentPadding(FMargin(0.f))
		.ClickMethod(EButtonClickMethod::MouseDown)
		.OnClicked(FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetLeaderboardType, Type))
		[
			SNew(SBox)
			.HeightOverride(50.f)
			[
				Content
			]
		];

	return FT66FlatStyle::AttachMetadata(
		Button,
		Tag(Name),
		TEXT("CheckboxButton"),
		State,
		TOptional<FLinearColor>(),
		true,
		FName(TEXT("MainMenuLeaderboardMetric")),
		false,
		true);
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildTimeMenu()
{
	TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			BuildMenuOption(
				NSLOCTEXT("T66.FlatLeaderboard", "Weekly", "WEEKLY"),
				FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetTimeFilter, ET66LeaderboardTime::Current),
				CurrentTimeFilter == ET66LeaderboardTime::Current,
				TEXT("TimeWeeklyOption"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 4.f, 0.f, 0.f)
		[
			BuildMenuOption(
				NSLOCTEXT("T66.FlatLeaderboard", "AllTime", "ALL TIME"),
				FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetTimeFilter, ET66LeaderboardTime::AllTime),
				CurrentTimeFilter == ET66LeaderboardTime::AllTime,
				TEXT("TimeAllTimeOption"))
		];

	return Menu;
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildRuleMenu()
{
	TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);

	Menu->AddSlot()
		.AutoHeight()
		[
			BuildMenuSectionLabel(TEXT("PARTY"), TEXT("PartySectionLabel"))
		];

	const ET66PartySize Parties[] = { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad };
	for (ET66PartySize Party : Parties)
	{
		Menu->AddSlot()
			.AutoHeight()
			.Padding(0.f, 4.f, 0.f, 0.f)
			[
				BuildMenuOption(
					PartySizeText(Party),
					FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetPartySize, Party),
					CurrentPartySize == Party,
					FString::Printf(TEXT("Party%sOption"), *PartySizeText(Party).ToString()))
			];
	}

	Menu->AddSlot()
		.AutoHeight()
		.Padding(0.f, 8.f, 0.f, 0.f)
		[
			BuildMenuSectionLabel(TEXT("DIFFICULTY"), TEXT("DifficultySectionLabel"))
		];

	const ET66Difficulty Difficulties[] = {
		ET66Difficulty::Easy,
		ET66Difficulty::Medium,
		ET66Difficulty::Hard,
		ET66Difficulty::VeryHard,
		ET66Difficulty::Impossible
	};
	for (ET66Difficulty Difficulty : Difficulties)
	{
		Menu->AddSlot()
			.AutoHeight()
			.Padding(0.f, 4.f, 0.f, 0.f)
			[
				BuildMenuOption(
					DifficultyText(Difficulty),
					FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetDifficulty, Difficulty),
					CurrentDifficulty == Difficulty,
					FString::Printf(TEXT("Difficulty%sOption"), *DifficultyText(Difficulty).ToString().Replace(TEXT(" "), TEXT(""))))
			];
	}

	return Menu;
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildPartySizeMenu()
{
	TSharedRef<SVerticalBox> Menu = SNew(SVerticalBox);
	const ET66PartySize Parties[] = { ET66PartySize::Solo, ET66PartySize::Duo, ET66PartySize::Trio, ET66PartySize::Quad };
	for (int32 PartyIndex = 0; PartyIndex < UE_ARRAY_COUNT(Parties); ++PartyIndex)
	{
		const ET66PartySize Party = Parties[PartyIndex];
		Menu->AddSlot()
			.AutoHeight()
			.Padding(0.f, PartyIndex == 0 ? 0.f : 4.f, 0.f, 0.f)
			[
				BuildMenuOption(
					PartySizeText(Party),
					FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetPartySize, Party),
					CurrentPartySize == Party,
					FString::Printf(TEXT("Party%sOption"), *PartySizeText(Party).ToString()))
			];
	}
	return Menu;
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildDifficultyMenu()
{
	const ET66Difficulty Difficulties[] = {
		ET66Difficulty::Easy,
		ET66Difficulty::Medium,
		ET66Difficulty::Hard,
		ET66Difficulty::VeryHard,
		ET66Difficulty::Impossible
	};
	TArray<FT66FlatDropdownOptionData> Options;
	Options.Reserve(UE_ARRAY_COUNT(Difficulties));
	for (int32 DifficultyIndex = 0; DifficultyIndex < UE_ARRAY_COUNT(Difficulties); ++DifficultyIndex)
	{
		const ET66Difficulty Difficulty = Difficulties[DifficultyIndex];
		const bool bPlayable = IsDifficultyPlayable(Difficulty);
		if (!bPlayable)
		{
			continue;
		}
		FT66FlatDropdownOptionData Option;
		Option.Label = DifficultyText(Difficulty);
		Option.State = CurrentDifficulty == Difficulty ? ET66FlatState::Selected : ET66FlatState::Default;
		Option.bEnabled = true;
		Option.bShowUnavailableOverlay = false;
		Option.MinWidth = DropdownColumnWidth;
		Option.Height = 52.f;
		Option.FontSize = 16;
		Option.Tag = Tag(FString::Printf(TEXT("Difficulty%sOption"), *DifficultyText(Difficulty).ToString().Replace(TEXT(" "), TEXT(""))));
		Option.OverlayTag = Tag(FString::Printf(TEXT("Difficulty%sOption.DemoOverlay"), *DifficultyText(Difficulty).ToString().Replace(TEXT(" "), TEXT(""))));
		Option.OnClicked = FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SetDifficulty, Difficulty);
		Options.Add(MoveTemp(Option));
	}
	return FT66FlatStyle::MakeFlatDropdownOptionsMenu(
		Options,
		DropdownColumnWidth,
		52.f,
		16,
		Tag(TEXT("DifficultyOptionsMenu")));
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildMenuSectionLabel(const FString& Label, const FString& TagName) const
{
	return FT66FlatStyle::MakeFlatLabel(
		FText::FromString(Label),
		ET66FlatLabelRole::Caption,
		ETextJustify::Left,
		Tag(TagName));
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildMenuOption(const FText& Label, FOnClicked OnClicked, const bool bSelected, const FString& TagName) const
{
	const ET66FlatState State = bSelected ? ET66FlatState::Selected : ET66FlatState::Default;
	return FT66FlatStyle::MakeFlatDropdownOptionButton(
		State,
		Label,
		MoveTemp(OnClicked),
		0.f,
		0.f,
		0,
		Tag(TagName),
		NAME_None);
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildStreamerRequestPanel()
{
	TSharedRef<SVerticalBox> Form = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 12.f)
		[
			FT66FlatStyle::MakeFlatLabel(
				NSLOCTEXT("T66.FlatLeaderboard", "StreamerRequestTitle", "REQUEST STREAMER ACCESS"),
				ET66FlatLabelRole::SubHeader,
				ETextJustify::Left,
				Tag(TEXT("StreamerRequestTitle")))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			FT66FlatStyle::MakeFlatLabel(
				NSLOCTEXT("T66.FlatLeaderboard", "CreatorLinkLabel", "Creator link"),
				ET66FlatLabelRole::Caption,
				ETextJustify::Left,
				Tag(TEXT("StreamerCreatorLinkLabel")))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 12.f)
		[
			FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.HeightOverride(44.f)
				[
					SAssignNew(StreamerCreatorLinkTextBox, SEditableTextBox)
					.Text(FText::FromString(StreamerCreatorLinkText))
					.HintText(NSLOCTEXT("T66.FlatLeaderboard", "CreatorLinkHint", "https://..."))
					.OnTextChanged_Lambda([this](const FText& NewText)
					{
						StreamerCreatorLinkText = NewText.ToString();
					})
				],
				Tag(TEXT("StreamerCreatorLinkInput")),
				TEXT("TextInput"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				true,
				NAME_None,
				false,
				true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			FT66FlatStyle::MakeFlatLabel(
				NSLOCTEXT("T66.FlatLeaderboard", "SteamIdLabel", "Steam ID"),
				ET66FlatLabelRole::Caption,
				ETextJustify::Left,
				Tag(TEXT("StreamerSteamIdLabel")))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 16.f)
		[
			FT66FlatStyle::AttachMetadata(
				SNew(SBox)
				.HeightOverride(44.f)
				[
					SAssignNew(StreamerSteamIdTextBox, SEditableTextBox)
					.Text(FText::FromString(StreamerSteamIdText))
					.HintText(NSLOCTEXT("T66.FlatLeaderboard", "SteamIdHint", "7656119..."))
					.OnTextChanged_Lambda([this](const FText& NewText)
					{
						StreamerSteamIdText = NewText.ToString();
					})
				],
				Tag(TEXT("StreamerSteamIdInput")),
				TEXT("TextInput"),
				ET66FlatState::Default,
				TOptional<FLinearColor>(),
				true,
				NAME_None,
				false,
				true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 12.f)
		[
			FT66FriendslopStyle::MakeButton(
				bStreamerRequestSubmitting ? ET66FlatState::Disabled : ET66FlatState::Ready,
				bStreamerRequestSubmitting
					? NSLOCTEXT("T66.FlatLeaderboard", "Submitting", "SUBMITTING")
					: NSLOCTEXT("T66.FlatLeaderboard", "SubmitRequest", "SUBMIT"),
				FOnClicked::CreateSP(this, &ST66FlatLeaderboardPanel::SubmitStreamerRequest),
				nullptr,
				nullptr,
				FMargin(12.f, 8.f),
				180.f,
				44.f,
				!bStreamerRequestSubmitting,
				16,
				Tag(TEXT("StreamerSubmitButton")),
				NAME_None,
				FT66FriendslopStyle::ButtonChromeForState(bStreamerRequestSubmitting ? ET66FlatState::Disabled : ET66FlatState::Ready))
		];

	if (!StreamerRequestStatusText.IsEmpty())
	{
		Form->AddSlot()
			.AutoHeight()
			[
				FT66FlatStyle::MakeFlatLabel(
					StreamerRequestStatusText,
					ET66FlatLabelRole::Body,
					ETextJustify::Left,
					Tag(TEXT("StreamerRequestStatus")))
			];
	}

	return FT66FriendslopStyle::MakePanel(
		ET66FlatState::Default,
		FMargin(10.f),
		Form,
		nullptr,
		Tag(TEXT("StreamerRequestPanel")));
}

TSharedRef<SWidget> ST66FlatLeaderboardPanel::BuildLeaderboardRow(const FLeaderboardEntry& Entry, const int32 DisplayIndex, const bool bLocalRow)
{
	const FString RowLeaf = bLocalRow
		? FString(TEXT("RankingRowLocal"))
		: FString::Printf(TEXT("RankingRow%02d"), DisplayIndex + 1);
	const ET66FlatState RowState = bLocalRow ? ET66FlatState::Selected : ET66FlatState::Default;
	const bool bFavoritable = IsEntryFavoritable(Entry);
	const bool bFavorited = IsEntryFavorited(Entry);

	TSharedRef<SHorizontalBox> RowContent = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(42.f)
			[
				SNew(STextBlock)
				.Text(GetRankText(Entry, bLocalRow))
				.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(14, true))
				.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.f, 0.f, 8.f, 0.f)
		[
			FT66FlatStyle::MakeFlatPortraitSlot(
				RowState,
				GetPortraitBrushForEntry(Entry),
				nullptr,
				FVector2D(FlatRowPortraitSize, FlatRowPortraitSize),
				Tag(RowLeaf + TEXT(".Avatar")))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(ResolveEntryDisplayName(Entry)))
			.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(14, true))
			.ColorAndOpacity(bLocalRow ? FT66FlatStyle::DataAccent() : FT66FlatStyle::PrimaryText())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8.f, 0.f, 0.f, 0.f)
		[
			SNew(SBox)
		.WidthOverride(74.f)
			[
				SNew(STextBlock)
				.Text(HasEntryMetricValue(Entry, bLocalRow)
					? GetEntryMetricText(Entry)
					: NSLOCTEXT("T66.FlatLeaderboard", "NoLocalMetric", "N/A"))
				.Font(T66RuntimeUIFontAccess::MakeFriendslopFont(14, true))
				.ColorAndOpacity(FT66FlatStyle::PrimaryText())
				.Justification(ETextJustify::Right)
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		];

	TSharedRef<SWidget> RowButton = bLocalRow
		? FT66FriendslopStyle::MakeToggleGroupButton(
			RowState,
			RowContent,
			FOnClicked::CreateLambda([this, Entry]()
			{
				return HandleEntryClicked(Entry);
			}),
			FMargin(7.f, 4.f),
			LeaderboardRowWidth,
			RowHeight,
			true,
			Tag(RowLeaf),
			NAME_None,
			ET66FriendslopChrome::RankingRowRedRound06)
		: FT66FriendslopStyle::MakeToggleGroupButton(
			RowState,
			RowContent,
			FOnClicked::CreateLambda([this, Entry]()
			{
				return HandleEntryClicked(Entry);
			}),
			FMargin(7.f, 4.f),
			bFavoritable ? LeaderboardRowWidth - 20.f : LeaderboardRowWidth,
			RowHeight,
			true,
			Tag(RowLeaf),
			NAME_None,
			ET66FriendslopChrome::FriendRowRound06);

	if (!bFavoritable)
	{
		return RowButton;
	}

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		[
			RowButton
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(5.f, 0.f, 0.f, 0.f)
		[
			FT66FriendslopStyle::MakeButton(
				bFavorited ? ET66FlatState::Selected : ET66FlatState::Default,
				bFavorited ? FText::FromString(TEXT("*")) : FText::FromString(TEXT("+")),
				FOnClicked::CreateLambda([this, Entry]()
				{
					return ToggleFavoriteEntry(Entry);
				}),
				nullptr,
				nullptr,
				FMargin(4.f),
				34.f,
				RowHeight,
				true,
				16,
				Tag(RowLeaf + TEXT(".FavoriteButton")),
				NAME_None,
				bFavorited ? ET66FriendslopChrome::ButtonPrimaryRed : ET66FriendslopChrome::IconButtonDark)
		];
}

void ST66FlatLeaderboardPanel::RefreshLeaderboard()
{
	if (IsFriendslopReferenceFixture())
	{
		bIsLoading = false;
		LeaderboardEntries.Reset();
		DedicatedLocalEntry = MakeDedicatedLocalEntryPlaceholder();
		DedicatedLocalEntry.Rank = 0;
		DedicatedLocalEntry.PlayerName = TEXT("Solobro");
		DedicatedLocalEntry.PlayerNames = { TEXT("Solobro") };
		DedicatedLocalEntry.Score = 0;
		DedicatedLocalEntry.TimeSeconds = 0.f;
		DedicatedLocalEntry.PartySize = ET66PartySize::Solo;
		DedicatedLocalEntry.Difficulty = ET66Difficulty::Easy;
		DedicatedLocalEntry.bIsLocalPlayer = true;
		DedicatedLocalEntry.bHasRunSummary = false;
		StatusText = FText::GetEmpty();
		RebuildEntryList();
		return;
	}

	UT66BackendSubsystem* Backend = GetBackendSubsystem();
	if (!Backend)
	{
		bIsLoading = false;
		LeaderboardEntries.Reset();
		DedicatedLocalEntry = MakeDedicatedLocalEntryPlaceholder();
		NormalizeEntryIdentity(DedicatedLocalEntry, VisibleRemoteEntryCount);
		StatusText = NSLOCTEXT("T66.FlatLeaderboard", "BackendUnavailable", "Leaderboard backend unavailable.");
		RebuildEntryList();
		return;
	}

	BindLeaderboardDelegate(Backend);
	RefreshDedicatedLocalEntry(Backend);

	const FString Key = MakeLeaderboardKey();
	if (Backend->HasCachedLeaderboard(Key))
	{
		bIsLoading = false;
		LeaderboardEntries = Backend->GetCachedLeaderboard(Key);
		NormalizeEntries();
		ApplyDisplayLimit();
		StatusText = LeaderboardEntries.Num() > 0
			? FText::GetEmpty()
			: NSLOCTEXT("T66.FlatLeaderboard", "NoData", "No leaderboard data yet.");
		RebuildEntryList();
		return;
	}

	bIsLoading = true;
	LeaderboardEntries.Reset();
	StatusText = NSLOCTEXT("T66.FlatLeaderboard", "Loading", "Loading leaderboard...");
	RebuildEntryList();

	Backend->FetchLeaderboard(
		CurrentBackendType(),
		CurrentBackendTime(),
		CurrentBackendParty(),
		CurrentBackendDifficulty(),
		CurrentBackendFilter());
}

void ST66FlatLeaderboardPanel::RebuildPanelAndRequestPaint()
{
	ChildSlot[BuildPanel()];
	Invalidate(EInvalidateWidgetReason::Layout);
	RequestFrontendPaintRefresh();
}

void ST66FlatLeaderboardPanel::RequestFrontendPaintRefresh() const
{
	if (UT66UIManager* RuntimeUIManager = UIManager.Get())
	{
		RuntimeUIManager->RequestFrontendRootPaintRefresh();
	}
}

void ST66FlatLeaderboardPanel::RebuildEntryList()
{
	if (!EntryListBox.IsValid())
	{
		return;
	}

	EntryListBox->ClearChildren();

	if (bIsLoading)
	{
		EntryListBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 18.f, 0.f, 0.f)
			[
				FT66FlatStyle::MakeFlatLabel(
					NSLOCTEXT("T66.FlatLeaderboard", "Loading", "Loading leaderboard..."),
					ET66FlatLabelRole::Body,
					ETextJustify::Center,
					Tag(TEXT("EmptyState")))
			];
		RequestFrontendPaintRefresh();
		return;
	}

	int32 RemoteDisplayIndex = 0;
	for (const FLeaderboardEntry& Entry : LeaderboardEntries)
	{
		EntryListBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 5.f)
			[
				BuildLeaderboardRow(Entry, RemoteDisplayIndex, false)
			];

		++RemoteDisplayIndex;
	}

	EntryListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 5.f)
		[
			BuildLeaderboardRow(DedicatedLocalEntry, RemoteDisplayIndex, true)
		];

	RequestFrontendPaintRefresh();
}

FReply ST66FlatLeaderboardPanel::SetFilter(const ET66LeaderboardFilter NewFilter)
{
	if (CurrentFilter == NewFilter)
	{
		return FReply::Handled();
	}
	CurrentFilter = NewFilter;
	bStreamerRequestOpen = false;
	StreamerRequestStatusText = FText::GetEmpty();
	FSlateApplication::Get().DismissAllMenus();
	RebuildPanelAndRequestPaint();
	RefreshLeaderboard();
	return FReply::Handled();
}

FReply ST66FlatLeaderboardPanel::SetTimeFilter(const ET66LeaderboardTime NewTimeFilter)
{
	if (NewTimeFilter == ET66LeaderboardTime::Daily || CurrentTimeFilter == NewTimeFilter)
	{
		return FReply::Handled();
	}
	CurrentTimeFilter = NewTimeFilter;
	FSlateApplication::Get().DismissAllMenus();
	RebuildPanelAndRequestPaint();
	RefreshLeaderboard();
	return FReply::Handled();
}

FReply ST66FlatLeaderboardPanel::SetPartySize(const ET66PartySize NewPartySize)
{
	if (CurrentPartySize == NewPartySize)
	{
		return FReply::Handled();
	}
	CurrentPartySize = NewPartySize;
	FSlateApplication::Get().DismissAllMenus();
	RebuildPanelAndRequestPaint();
	RefreshLeaderboard();
	return FReply::Handled();
}

FReply ST66FlatLeaderboardPanel::SetDifficulty(const ET66Difficulty NewDifficulty)
{
	if (CurrentDifficulty == NewDifficulty || !IsDifficultyPlayable(NewDifficulty))
	{
		FSlateApplication::Get().DismissAllMenus();
		return FReply::Handled();
	}
	CurrentDifficulty = NewDifficulty;
	FSlateApplication::Get().DismissAllMenus();
	RebuildPanelAndRequestPaint();
	RefreshLeaderboard();
	return FReply::Handled();
}

FReply ST66FlatLeaderboardPanel::SetLeaderboardType(const ET66LeaderboardType NewType)
{
	if (CurrentType == NewType)
	{
		return FReply::Handled();
	}
	CurrentType = NewType;
	RebuildPanelAndRequestPaint();
	RefreshLeaderboard();
	return FReply::Handled();
}

FReply ST66FlatLeaderboardPanel::SetStreamerRequestOpen(const bool bOpen)
{
	bStreamerRequestOpen = bOpen;
	StreamerRequestStatusText = FText::GetEmpty();
	RebuildPanelAndRequestPaint();
	if (!bStreamerRequestOpen)
	{
		RefreshLeaderboard();
	}
	return FReply::Handled();
}

FReply ST66FlatLeaderboardPanel::SubmitStreamerRequest()
{
	UT66BackendSubsystem* Backend = GetBackendSubsystem();
	if (!Backend)
	{
		StreamerRequestStatusText = NSLOCTEXT("T66.FlatLeaderboard", "StreamerBackendUnavailable", "Streamer requests unavailable.");
		RebuildPanelAndRequestPaint();
		return FReply::Handled();
	}

	const FString Link = StreamerCreatorLinkText.TrimStartAndEnd();
	const FString SteamId = StreamerSteamIdText.TrimStartAndEnd();
	if (Link.IsEmpty() || SteamId.IsEmpty())
	{
		StreamerRequestStatusText = NSLOCTEXT("T66.FlatLeaderboard", "StreamerMissingFields", "Creator link and Steam ID are required.");
		RebuildPanelAndRequestPaint();
		return FReply::Handled();
	}

	BoundBackendSubsystem = Backend;
	Backend->OnStreamerRequestDataReady.RemoveAll(this);
	Backend->OnStreamerRequestDataReady.AddSP(SharedThis(this), &ST66FlatLeaderboardPanel::OnStreamerRequestComplete);
	bStreamerRequestSubmitting = true;
	StreamerRequestStatusText = NSLOCTEXT("T66.FlatLeaderboard", "StreamerSubmittingStatus", "Submitting request...");
	RebuildPanelAndRequestPaint();
	Backend->SubmitStreamerRequest(Link, SteamId);
	return FReply::Handled();
}

void ST66FlatLeaderboardPanel::OnBackendLeaderboardReady(const FString& LeaderboardKey)
{
	if (LeaderboardKey != MakeLeaderboardKey())
	{
		return;
	}

	UT66BackendSubsystem* Backend = GetBackendSubsystem();
	if (!Backend)
	{
		return;
	}

	bIsLoading = false;
	LeaderboardEntries = Backend->GetCachedLeaderboard(LeaderboardKey);
	NormalizeEntries();
	ApplyDisplayLimit();
	UpdateDedicatedLocalEntryFromCache(Backend, MakeMyRankKey());
	StatusText = LeaderboardEntries.Num() > 0
		? FText::GetEmpty()
		: NSLOCTEXT("T66.FlatLeaderboard", "NoData", "No leaderboard data yet.");
	RebuildEntryList();
}

void ST66FlatLeaderboardPanel::OnBackendMyRankReady(const FString& RankKey, const bool bSuccess, const int32 Rank, const int32 TotalEntries)
{
	if (RankKey != PendingMyRankKey)
	{
		return;
	}

	if (UT66BackendSubsystem* Backend = GetBackendSubsystem())
	{
		UpdateDedicatedLocalEntryFromCache(Backend, RankKey);
	}
	RebuildEntryList();
}

void ST66FlatLeaderboardPanel::OnBackendRunSummaryReady(const FString& EntryId)
{
	if (EntryId != PendingRunSummaryEntryId)
	{
		return;
	}

	UT66BackendSubsystem* Backend = GetBackendSubsystem();
	UT66LeaderboardSubsystem* Leaderboard = LeaderboardSubsystem.Get();
	UT66UIManager* Manager = UIManager.Get();
	if (!Backend || !Leaderboard || !Manager)
	{
		return;
	}

	if (UT66LeaderboardRunSummarySaveGame* Snapshot = Backend->GetCachedRunSummary(EntryId))
	{
		PendingRunSummaryEntryId.Reset();
		Leaderboard->SetPendingFakeRunSummarySnapshot(Snapshot);
		if (Manager->GetCurrentModalType() == ET66ScreenType::PauseMenu)
		{
			Leaderboard->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::PauseMenu);
		}
		Manager->ShowModal(ET66ScreenType::RunSummary);
	}
}

void ST66FlatLeaderboardPanel::OnStreamerRequestComplete(const bool bSuccess, const FString& Message)
{
	bStreamerRequestSubmitting = false;
	StreamerRequestStatusText = FText::FromString(Message);
	if (bSuccess)
	{
		StreamerCreatorLinkText.Reset();
		StreamerSteamIdText.Reset();
	}
	RebuildPanelAndRequestPaint();
}

void ST66FlatLeaderboardPanel::BindLeaderboardDelegate(UT66BackendSubsystem* Backend)
{
	if (!Backend)
	{
		return;
	}

	if (BoundBackendSubsystem.IsValid() && BoundBackendSubsystem.Get() != Backend)
	{
		if (UT66BackendSubsystem* OldBackend = BoundBackendSubsystem.Get())
		{
			OldBackend->OnLeaderboardDataReady.RemoveAll(this);
			OldBackend->OnMyRankDataReady.RemoveAll(this);
			OldBackend->OnRunSummaryReady.RemoveAll(this);
			OldBackend->OnStreamerRequestDataReady.RemoveAll(this);
		}
		bBoundToLeaderboardDelegate = false;
		bBoundToMyRankDelegate = false;
		bBoundToRunSummaryDelegate = false;
	}

	BoundBackendSubsystem = Backend;
	if (!bBoundToLeaderboardDelegate)
	{
		Backend->OnLeaderboardDataReady.AddSP(SharedThis(this), &ST66FlatLeaderboardPanel::OnBackendLeaderboardReady);
		bBoundToLeaderboardDelegate = true;
	}
}

void ST66FlatLeaderboardPanel::BindMyRankDelegate(UT66BackendSubsystem* Backend)
{
	if (!Backend)
	{
		return;
	}
	BindLeaderboardDelegate(Backend);
	if (!bBoundToMyRankDelegate)
	{
		Backend->OnMyRankDataReady.AddSP(SharedThis(this), &ST66FlatLeaderboardPanel::OnBackendMyRankReady);
		bBoundToMyRankDelegate = true;
	}
}

void ST66FlatLeaderboardPanel::BindRunSummaryDelegate(UT66BackendSubsystem* Backend)
{
	if (!Backend)
	{
		return;
	}
	BindLeaderboardDelegate(Backend);
	if (!bBoundToRunSummaryDelegate)
	{
		Backend->OnRunSummaryReady.AddSP(SharedThis(this), &ST66FlatLeaderboardPanel::OnBackendRunSummaryReady);
		bBoundToRunSummaryDelegate = true;
	}
}

FReply ST66FlatLeaderboardPanel::HandleEntryClicked(const FLeaderboardEntry& Entry)
{
	if (Entry.bIsLocalPlayer)
	{
		return HandleLocalEntryClicked(Entry);
	}

	UT66LeaderboardSubsystem* Leaderboard = LeaderboardSubsystem.Get();
	UT66UIManager* Manager = UIManager.Get();
	if (!Leaderboard || !Manager || !Entry.bHasRunSummary || Entry.EntryId.IsEmpty())
	{
		return FReply::Handled();
	}

	UT66BackendSubsystem* Backend = GetBackendSubsystem();
	if (!Backend)
	{
		return FReply::Handled();
	}

	if (Backend->HasCachedRunSummary(Entry.EntryId))
	{
		if (UT66LeaderboardRunSummarySaveGame* Snapshot = Backend->GetCachedRunSummary(Entry.EntryId))
		{
			Leaderboard->SetPendingFakeRunSummarySnapshot(Snapshot);
			if (Manager->GetCurrentModalType() == ET66ScreenType::PauseMenu)
			{
				Leaderboard->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::PauseMenu);
			}
			Manager->ShowModal(ET66ScreenType::RunSummary);
		}
		return FReply::Handled();
	}

	if (Backend->IsBackendConfigured())
	{
		PendingRunSummaryEntryId = Entry.EntryId;
		BindRunSummaryDelegate(Backend);
		Backend->FetchRunSummary(Entry.EntryId);
	}

	return FReply::Handled();
}

FReply ST66FlatLeaderboardPanel::HandleLocalEntryClicked(const FLeaderboardEntry& Entry)
{
	UT66LeaderboardSubsystem* Leaderboard = LeaderboardSubsystem.Get();
	UT66UIManager* Manager = UIManager.Get();
	if (!Leaderboard || !Manager)
	{
		return FReply::Handled();
	}

	if (Entry.bHasRunSummary && !Entry.EntryId.IsEmpty())
	{
		if (UT66BackendSubsystem* Backend = GetBackendSubsystem())
		{
			if (Backend->HasCachedRunSummary(Entry.EntryId))
			{
				if (UT66LeaderboardRunSummarySaveGame* Snapshot = Backend->GetCachedRunSummary(Entry.EntryId))
				{
					Leaderboard->SetPendingFakeRunSummarySnapshot(Snapshot);
					if (Manager->GetCurrentModalType() == ET66ScreenType::PauseMenu)
					{
						Leaderboard->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::PauseMenu);
					}
					Manager->ShowModal(ET66ScreenType::RunSummary);
				}
				return FReply::Handled();
			}
			PendingRunSummaryEntryId = Entry.EntryId;
			BindRunSummaryDelegate(Backend);
			Backend->FetchRunSummary(Entry.EntryId);
			return FReply::Handled();
		}
	}

	if (CurrentType == ET66LeaderboardType::Score
		&& Leaderboard->HasLocalBestScoreRunSummary(CurrentDifficulty, CurrentPartySize))
	{
		Leaderboard->RequestOpenLocalBestScoreRunSummary(CurrentDifficulty, CurrentPartySize);
		if (Manager->GetCurrentModalType() == ET66ScreenType::PauseMenu)
		{
			Leaderboard->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::PauseMenu);
		}
		Manager->ShowModal(ET66ScreenType::RunSummary);
	}

	return FReply::Handled();
}

FReply ST66FlatLeaderboardPanel::ToggleFavoriteEntry(const FLeaderboardEntry& Entry)
{
	UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettingsSubsystem();
	if (!PlayerSettings || !IsEntryFavoritable(Entry))
	{
		return FReply::Handled();
	}

	const bool bShouldFavorite = !PlayerSettings->IsFavoriteLeaderboardRun(Entry.EntryId);
	PlayerSettings->SetFavoriteLeaderboardRun(MakeFavoriteRunFromEntry(Entry), bShouldFavorite);
	RebuildEntryList();
	return FReply::Handled();
}

void ST66FlatLeaderboardPanel::NormalizeEntries()
{
	for (int32 Index = 0; Index < LeaderboardEntries.Num(); ++Index)
	{
		LeaderboardEntries[Index].bIsLocalPlayer = false;
		NormalizeEntryIdentity(LeaderboardEntries[Index], Index);
	}
}

void ST66FlatLeaderboardPanel::NormalizeEntryIdentity(FLeaderboardEntry& Entry, const int32 EntryIndex)
{
	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;

	const int32 PartyMemberCount = GetFlatPartyMemberCount(Entry.PartySize);
	const int32 FallbackStartIndex = FMath::Max(0, EntryIndex) * 4;
	if (Entry.PlayerSteamIds.Num() == 0 && !Entry.SteamId.IsEmpty())
	{
		Entry.PlayerSteamIds.Add(Entry.SteamId);
	}

	TArray<FString> CleanNames;
	auto TryAddCleanName = [&CleanNames](const FString& Candidate)
	{
		const FString Trimmed = Candidate.TrimStartAndEnd();
		if (Trimmed.IsEmpty() || IsFlatSyntheticLeaderboardName(Trimmed) || CleanNames.Contains(Trimmed))
		{
			return;
		}
		CleanNames.Add(Trimmed);
	};

	for (int32 NameIndex = 0; NameIndex < Entry.PlayerNames.Num(); ++NameIndex)
	{
		const FString SteamName = Entry.PlayerSteamIds.IsValidIndex(NameIndex)
			? ResolveSteamDisplayName(Entry.PlayerSteamIds[NameIndex])
			: FString();
		TryAddCleanName(SteamName);
		TryAddCleanName(Entry.PlayerNames[NameIndex]);
	}
	TryAddCleanName(ResolveSteamDisplayName(Entry.SteamId));
	TryAddCleanName(Entry.PlayerName);

	if (CleanNames.Num() == 0 && Entry.bIsLocalPlayer)
	{
		if (UT66SteamHelper* SteamHelper = GetSteamHelper())
		{
			TryAddCleanName(SteamHelper->GetLocalDisplayName());
		}
		TryAddCleanName(TEXT("YOU"));
	}

	if (CleanNames.Num() == 0)
	{
		CleanNames.Add(TEXT("Steam Player"));
	}

	while (CleanNames.Num() < PartyMemberCount)
	{
		CleanNames.Add(FString::Printf(TEXT("Steam Player %d"), CleanNames.Num() + 1));
	}

	Entry.PlayerNames = CleanNames;
	Entry.PlayerName = CleanNames[0];

	if (Entry.HeroID.IsNone())
	{
		if (Entry.bIsLocalPlayer && T66GI && !T66GI->SelectedHeroID.IsNone())
		{
			Entry.HeroID = T66GI->SelectedHeroID;
		}
		else
		{
			Entry.HeroID = GetFlatFallbackHeroId(T66GI, FallbackStartIndex);
		}
	}
}

void ST66FlatLeaderboardPanel::ApplyDisplayLimit()
{
	if (LeaderboardEntries.Num() > VisibleRemoteEntryCount)
	{
		LeaderboardEntries.SetNum(VisibleRemoteEntryCount);
	}
}

void ST66FlatLeaderboardPanel::RefreshDedicatedLocalEntry(UT66BackendSubsystem* Backend)
{
	DedicatedLocalEntry = MakeDedicatedLocalEntryPlaceholder();
	NormalizeEntryIdentity(DedicatedLocalEntry, VisibleRemoteEntryCount);
	PendingMyRankKey = MakeMyRankKey();

	if (!Backend)
	{
		return;
	}

	BindMyRankDelegate(Backend);
	if (Backend->HasCachedMyRank(PendingMyRankKey))
	{
		UpdateDedicatedLocalEntryFromCache(Backend, PendingMyRankKey);
		return;
	}

	Backend->FetchMyRankFiltered(
		CurrentBackendType(),
		CurrentBackendTime(),
		CurrentBackendParty(),
		CurrentBackendDifficulty(),
		CurrentBackendFilter(),
		CurrentMyRankFilterContext());
}

void ST66FlatLeaderboardPanel::UpdateDedicatedLocalEntryFromCache(UT66BackendSubsystem* Backend, const FString& RankKey)
{
	DedicatedLocalEntry = MakeDedicatedLocalEntryPlaceholder();
	if (!Backend || RankKey.IsEmpty())
	{
		return;
	}

	bool bRankSuccess = false;
	int32 Rank = 0;
	int32 TotalEntries = 0;
	int64 Score = 0;
	FString EntryId;
	if (Backend->GetCachedMyRank(RankKey, bRankSuccess, Rank, TotalEntries, Score, EntryId)
		&& bRankSuccess
		&& Rank > 0)
	{
		DedicatedLocalEntry.Rank = Rank;
		DedicatedLocalEntry.Score = Score;
		DedicatedLocalEntry.TimeSeconds = CurrentType == ET66LeaderboardType::SpeedRun
			? static_cast<float>(Score) / 1000.0f
			: 0.0f;
		DedicatedLocalEntry.EntryId = EntryId;
		DedicatedLocalEntry.bHasRunSummary = !EntryId.IsEmpty();
	}

	NormalizeEntryIdentity(DedicatedLocalEntry, VisibleRemoteEntryCount);
}

FLeaderboardEntry ST66FlatLeaderboardPanel::MakeDedicatedLocalEntryPlaceholder() const
{
	FLeaderboardEntry Entry;
	Entry.Rank = 0;
	Entry.Score = 0;
	Entry.TimeSeconds = -1.0f;
	Entry.PartySize = CurrentPartySize;
	Entry.Difficulty = CurrentDifficulty;
	Entry.bIsLocalPlayer = true;

	if (UT66SteamHelper* SteamHelper = GetSteamHelper())
	{
		Entry.SteamId = SteamHelper->GetLocalSteamId();
		if (!Entry.SteamId.IsEmpty())
		{
			Entry.PlayerSteamIds.Add(Entry.SteamId);
		}

		const FString DisplayName = SteamHelper->GetLocalDisplayName().TrimStartAndEnd();
		Entry.PlayerName = DisplayName.IsEmpty() ? FString(TEXT("YOU")) : DisplayName;
	}

	if (Entry.PlayerName.IsEmpty())
	{
		Entry.PlayerName = TEXT("YOU");
	}
	Entry.PlayerNames.Add(Entry.PlayerName);

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			Entry.HeroID = T66GI->SelectedHeroID;
		}
	}

	return Entry;
}

FName ST66FlatLeaderboardPanel::Tag(const FString& Leaf) const
{
	if (Leaf.IsEmpty())
	{
		return FName(*TagPrefix);
	}
	return FName(*(TagPrefix + TEXT(".") + Leaf));
}

FString ST66FlatLeaderboardPanel::MakeLeaderboardKey() const
{
	return FString::Printf(
		TEXT("%s_%s_%s_%s_%s"),
		*CurrentBackendType(),
		*CurrentBackendTime(),
		*CurrentBackendParty(),
		*CurrentBackendDifficulty(),
		*CurrentBackendFilter());
}

FString ST66FlatLeaderboardPanel::MakeMyRankKey() const
{
	return UT66BackendSubsystem::MakeMyRankCacheKey(
		CurrentBackendType(),
		CurrentBackendTime(),
		CurrentBackendParty(),
		CurrentBackendDifficulty(),
		CurrentBackendFilter(),
		CurrentMyRankFilterContext());
}

FString ST66FlatLeaderboardPanel::CurrentMyRankFilterContext() const
{
	if (CurrentFilter == ET66LeaderboardFilter::Friends)
	{
		if (UT66SteamHelper* SteamHelper = GetSteamHelper())
		{
			return FString::Join(SteamHelper->GetFriendSteamIds(), TEXT(","));
		}
	}

	return FString();
}

FString ST66FlatLeaderboardPanel::CurrentBackendType() const
{
	return CurrentType == ET66LeaderboardType::SpeedRun ? TEXT("speedrun") : TEXT("score");
}

FString ST66FlatLeaderboardPanel::CurrentBackendTime() const
{
	return CurrentTimeFilter == ET66LeaderboardTime::AllTime ? TEXT("alltime") : TEXT("weekly");
}

FString ST66FlatLeaderboardPanel::CurrentBackendParty() const
{
	switch (CurrentPartySize)
	{
	case ET66PartySize::Duo:
		return TEXT("duo");
	case ET66PartySize::Trio:
		return TEXT("trio");
	case ET66PartySize::Quad:
		return TEXT("quad");
	case ET66PartySize::Solo:
	default:
		return TEXT("solo");
	}
}

FString ST66FlatLeaderboardPanel::CurrentBackendDifficulty() const
{
	switch (CurrentDifficulty)
	{
	case ET66Difficulty::Medium:
		return TEXT("medium");
	case ET66Difficulty::Hard:
		return TEXT("hard");
	case ET66Difficulty::VeryHard:
		return TEXT("veryhard");
	case ET66Difficulty::Impossible:
		return TEXT("impossible");
	case ET66Difficulty::Easy:
	default:
		return TEXT("easy");
	}
}

FString ST66FlatLeaderboardPanel::CurrentBackendFilter() const
{
	switch (CurrentFilter)
	{
	case ET66LeaderboardFilter::Friends:
		return TEXT("friends");
	case ET66LeaderboardFilter::Streamers:
		return TEXT("streamers");
	case ET66LeaderboardFilter::Global:
	default:
		return TEXT("global");
	}
}

bool ST66FlatLeaderboardPanel::IsDifficultyPlayable(const ET66Difficulty Difficulty) const
{
	if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		return T66GI->IsDifficultyPlayable(Difficulty);
	}
	return true;
}

FText ST66FlatLeaderboardPanel::GetHeaderText() const
{
	switch (CurrentFilter)
	{
	case ET66LeaderboardFilter::Friends:
		return NSLOCTEXT("T66.FlatLeaderboard", "FriendsHeader", "FRIENDS CHAD RANKING");
	case ET66LeaderboardFilter::Streamers:
		return NSLOCTEXT("T66.FlatLeaderboard", "StreamersHeader", "STREAM CHAD RANKING");
	case ET66LeaderboardFilter::Global:
	default:
		return NSLOCTEXT("T66.FlatLeaderboard", "GlobalHeader", "GLOBAL CHAD RANKING");
	}
}

FText ST66FlatLeaderboardPanel::GetTimeDropdownText() const
{
	return TimeFilterText(CurrentTimeFilter);
}

FText ST66FlatLeaderboardPanel::GetRuleDropdownText() const
{
	return FText::FromString(FString::Printf(
		TEXT("%s %s"),
		*PartySizeText(CurrentPartySize).ToString(),
		*DifficultyText(CurrentDifficulty).ToString()));
}

FText ST66FlatLeaderboardPanel::GetMetricHeaderText() const
{
	return CurrentType == ET66LeaderboardType::SpeedRun
		? NSLOCTEXT("T66.FlatLeaderboard", "TimeHeader", "TIME")
		: NSLOCTEXT("T66.FlatLeaderboard", "ScoreHeader", "SCORE");
}

FText ST66FlatLeaderboardPanel::GetEntryMetricText(const FLeaderboardEntry& Entry) const
{
	if (CurrentType == ET66LeaderboardType::SpeedRun)
	{
		const float Seconds = Entry.TimeSeconds > 0.f ? Entry.TimeSeconds : static_cast<float>(Entry.Score) / 1000.f;
		return FText::FromString(FormatTime(Seconds));
	}

	FNumberFormattingOptions Options;
	Options.UseGrouping = true;
	Options.MaximumFractionalDigits = 0;
	return FText::AsNumber(Entry.Score, &Options);
}

FText ST66FlatLeaderboardPanel::GetRankText(const FLeaderboardEntry& Entry, const bool bLocalRow) const
{
	if (bLocalRow && Entry.Rank <= 0)
	{
		return NSLOCTEXT("T66.FlatLeaderboard", "NoLocalRank", "N/A");
	}

	return FText::Format(NSLOCTEXT("T66.FlatLeaderboard", "RankFormat", "#{0}"), FText::AsNumber(Entry.Rank));
}

bool ST66FlatLeaderboardPanel::HasEntryMetricValue(const FLeaderboardEntry& Entry, const bool bLocalRow) const
{
	if (!bLocalRow)
	{
		return true;
	}

	if (Entry.Rank <= 0)
	{
		return false;
	}

	return CurrentType == ET66LeaderboardType::SpeedRun
		? Entry.Score > 0 || Entry.TimeSeconds > 0.0f
		: Entry.Score > 0;
}

FString ST66FlatLeaderboardPanel::FormatTime(const float Seconds) const
{
	if (Seconds < 0.f)
	{
		return TEXT("--:--");
	}
	const int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
	const int32 Secs = FMath::FloorToInt(FMath::Fmod(Seconds, 60.0f));
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Secs);
}

FString ST66FlatLeaderboardPanel::ResolveEntryDisplayName(const FLeaderboardEntry& Entry) const
{
	if (Entry.PlayerNames.Num() > 1)
	{
		TArray<FString> Names;
		for (int32 Index = 0; Index < Entry.PlayerNames.Num(); ++Index)
		{
			Names.Add(ResolveEntryMemberDisplayName(Entry, Index));
		}
		return FString::Join(Names, TEXT(" / "));
	}

	const FString SteamName = ResolveSteamDisplayName(Entry.SteamId);
	if (!SteamName.IsEmpty())
	{
		return SteamName;
	}

	const FString Trimmed = Entry.PlayerName.TrimStartAndEnd();
	if (!Trimmed.IsEmpty() && !IsFlatSyntheticLeaderboardName(Trimmed))
	{
		return Trimmed;
	}

	return Entry.bIsLocalPlayer ? FString(TEXT("YOU")) : FString(TEXT("Steam Player"));
}

FString ST66FlatLeaderboardPanel::ResolveEntryMemberDisplayName(const FLeaderboardEntry& Entry, const int32 MemberIndex) const
{
	if (Entry.PlayerSteamIds.IsValidIndex(MemberIndex))
	{
		const FString SteamName = ResolveSteamDisplayName(Entry.PlayerSteamIds[MemberIndex]);
		if (!SteamName.IsEmpty())
		{
			return SteamName;
		}
	}

	if (Entry.PlayerNames.IsValidIndex(MemberIndex))
	{
		const FString Name = Entry.PlayerNames[MemberIndex].TrimStartAndEnd();
		if (!Name.IsEmpty() && !IsFlatSyntheticLeaderboardName(Name))
		{
			return Name;
		}
	}

	return FString::Printf(TEXT("Steam Player %d"), MemberIndex + 1);
}

FString ST66FlatLeaderboardPanel::ResolveSteamDisplayName(const FString& SteamId) const
{
	const FString TrimmedSteamId = SteamId.TrimStartAndEnd();
	if (TrimmedSteamId.IsEmpty())
	{
		return FString();
	}

	UT66SteamHelper* SteamHelper = GetSteamHelper();
	if (!SteamHelper)
	{
		return FString();
	}

	if (TrimmedSteamId == SteamHelper->GetLocalSteamId())
	{
		return SteamHelper->GetLocalDisplayName();
	}

	for (const FT66SteamFriendInfo& FriendInfo : SteamHelper->GetFriendInfos())
	{
		if (FriendInfo.SteamId == TrimmedSteamId && !FriendInfo.DisplayName.IsEmpty())
		{
			return FriendInfo.DisplayName;
		}
	}

	return FString();
}

FText ST66FlatLeaderboardPanel::PartySizeText(const ET66PartySize PartySize) const
{
	switch (PartySize)
	{
	case ET66PartySize::Duo:
		return NSLOCTEXT("T66.FlatLeaderboard", "Duo", "DUO");
	case ET66PartySize::Trio:
		return NSLOCTEXT("T66.FlatLeaderboard", "Trio", "TRIO");
	case ET66PartySize::Quad:
		return NSLOCTEXT("T66.FlatLeaderboard", "Quad", "QUAD");
	case ET66PartySize::Solo:
	default:
		return NSLOCTEXT("T66.FlatLeaderboard", "Solo", "SOLO");
	}
}

FText ST66FlatLeaderboardPanel::DifficultyText(const ET66Difficulty Difficulty) const
{
	switch (Difficulty)
	{
	case ET66Difficulty::Medium:
		return NSLOCTEXT("T66.FlatLeaderboard", "Medium", "MEDIUM");
	case ET66Difficulty::Hard:
		return NSLOCTEXT("T66.FlatLeaderboard", "Hard", "HARD");
	case ET66Difficulty::VeryHard:
		return NSLOCTEXT("T66.FlatLeaderboard", "VeryHard", "VERY HARD");
	case ET66Difficulty::Impossible:
		return NSLOCTEXT("T66.FlatLeaderboard", "Impossible", "IMPOSSIBLE");
	case ET66Difficulty::Easy:
	default:
		return NSLOCTEXT("T66.FlatLeaderboard", "Easy", "EASY");
	}
}

FText ST66FlatLeaderboardPanel::TimeFilterText(const ET66LeaderboardTime TimeFilter) const
{
	return TimeFilter == ET66LeaderboardTime::AllTime
		? NSLOCTEXT("T66.FlatLeaderboard", "AllTime", "ALL TIME")
		: NSLOCTEXT("T66.FlatLeaderboard", "Weekly", "WEEKLY");
}

FText ST66FlatLeaderboardPanel::TypeText(const ET66LeaderboardType Type) const
{
	return Type == ET66LeaderboardType::SpeedRun
		? NSLOCTEXT("T66.FlatLeaderboard", "SpeedRunType", "SPEED RUN")
		: NSLOCTEXT("T66.FlatLeaderboard", "ScoreType", "SCORE");
}

bool ST66FlatLeaderboardPanel::IsEntryLocalPlayer(const FLeaderboardEntry& Entry) const
{
	if (Entry.bIsLocalPlayer)
	{
		return true;
	}
	UT66SteamHelper* SteamHelper = GetSteamHelper();
	const FString LocalSteamId = SteamHelper ? SteamHelper->GetLocalSteamId() : FString();
	return !LocalSteamId.IsEmpty() && (Entry.SteamId == LocalSteamId || Entry.PlayerSteamIds.Contains(LocalSteamId));
}

bool ST66FlatLeaderboardPanel::CanOpenRunSummary(const FLeaderboardEntry& Entry) const
{
	if (Entry.bHasRunSummary && !Entry.EntryId.IsEmpty())
	{
		return true;
	}
	return Entry.bIsLocalPlayer
		&& CurrentType == ET66LeaderboardType::Score
		&& LeaderboardSubsystem.IsValid()
		&& LeaderboardSubsystem->HasLocalBestScoreRunSummary(CurrentDifficulty, CurrentPartySize);
}

bool ST66FlatLeaderboardPanel::IsEntryFavoritable(const FLeaderboardEntry& Entry) const
{
	return !Entry.EntryId.IsEmpty();
}

bool ST66FlatLeaderboardPanel::IsEntryFavorited(const FLeaderboardEntry& Entry) const
{
	UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettingsSubsystem();
	return PlayerSettings && PlayerSettings->IsFavoriteLeaderboardRun(Entry.EntryId);
}

FT66FavoriteLeaderboardRun ST66FlatLeaderboardPanel::MakeFavoriteRunFromEntry(const FLeaderboardEntry& Entry) const
{
	FT66FavoriteLeaderboardRun Favorite;
	Favorite.EntryId = Entry.EntryId;
	Favorite.LeaderboardType = CurrentType;
	Favorite.Filter = CurrentFilter;
	Favorite.TimeScope = CurrentTimeFilter;
	Favorite.Difficulty = CurrentDifficulty;
	Favorite.PartySize = CurrentPartySize;
	Favorite.Rank = Entry.Rank;
	Favorite.DisplayName = ResolveEntryDisplayName(Entry);
	Favorite.Score = Entry.Score;
	Favorite.TimeSeconds = Entry.TimeSeconds;
	Favorite.bHasRunSummary = Entry.bHasRunSummary;
	return Favorite;
}

const FSlateBrush* ST66FlatLeaderboardPanel::GetPortraitBrushForEntry(const FLeaderboardEntry& Entry)
{
	if (!Entry.SteamId.IsEmpty())
	{
		const FSlateBrush* SteamAvatarBrush = GetOrCreateSteamAvatarBrush(Entry.SteamId);
		if (SteamAvatarBrush && SteamAvatarBrush != DefaultAvatarBrush.Get() && SteamAvatarBrush->GetResourceObject() != nullptr)
		{
			return SteamAvatarBrush;
		}
	}

	if (!Entry.AvatarUrl.IsEmpty())
	{
		const FSlateBrush* AvatarBrush = GetOrCreateAvatarBrush(Entry.AvatarUrl);
		if (AvatarBrush && AvatarBrush != DefaultAvatarBrush.Get() && AvatarBrush->GetResourceObject() != nullptr)
		{
			return AvatarBrush;
		}
	}

	const FSlateBrush* HeroBrush = GetOrCreateHeroPortraitBrush(Entry.HeroID);
	if (HeroBrush && HeroBrush != DefaultAvatarBrush.Get() && HeroBrush->GetResourceObject() != nullptr)
	{
		return HeroBrush;
	}

	return nullptr;
}

const FSlateBrush* ST66FlatLeaderboardPanel::GetPortraitBrushForEntryMember(const FLeaderboardEntry& Entry, const int32 MemberIndex)
{
	if (Entry.PlayerSteamIds.IsValidIndex(MemberIndex))
	{
		const FSlateBrush* SteamAvatarBrush = GetOrCreateSteamAvatarBrush(Entry.PlayerSteamIds[MemberIndex]);
		if (SteamAvatarBrush && SteamAvatarBrush != DefaultAvatarBrush.Get() && SteamAvatarBrush->GetResourceObject() != nullptr)
		{
			return SteamAvatarBrush;
		}
	}

	if (MemberIndex == 0 && !Entry.AvatarUrl.IsEmpty())
	{
		const FSlateBrush* AvatarBrush = GetOrCreateAvatarBrush(Entry.AvatarUrl);
		if (AvatarBrush && AvatarBrush != DefaultAvatarBrush.Get() && AvatarBrush->GetResourceObject() != nullptr)
		{
			return AvatarBrush;
		}
	}

	const FSlateBrush* HeroBrush = GetOrCreateHeroPortraitBrush(Entry.HeroID);
	return (HeroBrush && HeroBrush != DefaultAvatarBrush.Get() && HeroBrush->GetResourceObject() != nullptr) ? HeroBrush : nullptr;
}

const FSlateBrush* ST66FlatLeaderboardPanel::GetOrCreateSteamAvatarBrush(const FString& SteamId)
{
	const FString TrimmedSteamId = SteamId.TrimStartAndEnd();
	if (TrimmedSteamId.IsEmpty())
	{
		return DefaultAvatarBrush.Get();
	}

	if (TSharedPtr<FSlateBrush>* Found = SteamAvatarBrushes.Find(TrimmedSteamId))
	{
		return Found->Get();
	}

	UT66SteamHelper* SteamHelper = GetSteamHelper();
	UTexture2D* AvatarTexture = SteamHelper ? SteamHelper->GetAvatarTextureForSteamId(TrimmedSteamId) : nullptr;
	if (!AvatarTexture)
	{
		return DefaultAvatarBrush.Get();
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->DrawAs = ESlateBrushDrawType::Image;
	Brush->Tiling = ESlateBrushTileType::NoTile;
	Brush->ImageSize = FVector2D(FlatRowPortraitSize, FlatRowPortraitSize);
	SetBrushTexture(Brush, AvatarTexture);
	SteamAvatarBrushes.Add(TrimmedSteamId, Brush);
	return Brush.Get();
}

const FSlateBrush* ST66FlatLeaderboardPanel::GetOrCreateAvatarBrush(const FString& AvatarUrl)
{
	const FString TrimmedUrl = AvatarUrl.TrimStartAndEnd();
	if (TrimmedUrl.IsEmpty())
	{
		return DefaultAvatarBrush.Get();
	}

	if (TSharedPtr<FSlateBrush>* Found = AvatarBrushes.Find(TrimmedUrl))
	{
		return Found->Get();
	}

	UT66WebImageCache* ImageCache = GetWebImageCache();
	if (!ImageCache)
	{
		return DefaultAvatarBrush.Get();
	}

	if (UTexture2D* Texture = ImageCache->GetCachedImage(TrimmedUrl))
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->DrawAs = ESlateBrushDrawType::Image;
		Brush->Tiling = ESlateBrushTileType::NoTile;
		Brush->ImageSize = FVector2D(FlatRowPortraitSize, FlatRowPortraitSize);
		SetBrushTexture(Brush, Texture);
		AvatarBrushes.Add(TrimmedUrl, Brush);
		return Brush.Get();
	}

	TWeakPtr<ST66FlatLeaderboardPanel> WeakPanel = StaticCastSharedRef<ST66FlatLeaderboardPanel>(AsShared());
	ImageCache->RequestImage(TrimmedUrl, [WeakPanel](UTexture2D* Texture)
	{
		if (Texture)
		{
			if (TSharedPtr<ST66FlatLeaderboardPanel> Panel = WeakPanel.Pin())
			{
				Panel->RebuildEntryList();
			}
		}
	});

	return DefaultAvatarBrush.Get();
}

const FSlateBrush* ST66FlatLeaderboardPanel::GetOrCreateHeroPortraitBrush(const FName HeroID)
{
	if (HeroID.IsNone())
	{
		return DefaultAvatarBrush.Get();
	}

	if (TSharedPtr<FSlateBrush>* Found = HeroPortraitBrushes.Find(HeroID))
	{
		return Found->Get();
	}

	UGameInstance* GI = GetGameInstance();
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66UITexturePoolSubsystem* TexturePool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (!T66GI || !TexturePool)
	{
		return DefaultAvatarBrush.Get();
	}

	const TSoftObjectPtr<UTexture2D> PortraitSoft = T66GI->ResolveHeroPortrait(HeroID, ET66BodyType::Chad, ET66HeroPortraitVariant::Low);
	if (PortraitSoft.IsNull())
	{
		return DefaultAvatarBrush.Get();
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->DrawAs = ESlateBrushDrawType::Image;
	Brush->Tiling = ESlateBrushTileType::NoTile;
	Brush->ImageSize = FVector2D(FlatRowPortraitSize, FlatRowPortraitSize);

	if (UTexture2D* LoadedPortrait = T66SlateTexture::GetLoaded(TexturePool, PortraitSoft))
	{
		SetBrushTexture(Brush, LoadedPortrait);
	}
	else
	{
		UObject* Requester = UIManager.IsValid() ? static_cast<UObject*>(UIManager.Get()) : static_cast<UObject*>(GI);
		if (Requester)
		{
			TWeakPtr<ST66FlatLeaderboardPanel> WeakPanel = StaticCastSharedRef<ST66FlatLeaderboardPanel>(AsShared());
			TWeakPtr<FSlateBrush> WeakBrush = Brush;
			TexturePool->RequestTexture(
				PortraitSoft,
				Requester,
				FName(*FString::Printf(TEXT("FlatLBHero_%s"), *HeroID.ToString())),
				[WeakPanel, WeakBrush](UTexture2D* LoadedPortrait)
				{
					if (TSharedPtr<ST66FlatLeaderboardPanel> Panel = WeakPanel.Pin())
					{
						if (TSharedPtr<FSlateBrush> PinnedBrush = WeakBrush.Pin())
						{
							Panel->SetBrushTexture(PinnedBrush, LoadedPortrait);
							if (LoadedPortrait)
							{
								Panel->RebuildEntryList();
							}
						}
					}
				});
		}
	}

	HeroPortraitBrushes.Add(HeroID, Brush);
	return Brush.Get();
}

const FSlateBrush* ST66FlatLeaderboardPanel::GetFilterIconBrush(const ET66LeaderboardFilter Filter)
{
	if (TSharedPtr<FSlateBrush>* Found = FilterIconBrushes.Find(Filter))
	{
		return Found->Get();
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->DrawAs = ESlateBrushDrawType::Image;
	Brush->Tiling = ESlateBrushTileType::NoTile;
	Brush->ImageSize = FVector2D(46.f, 46.f);

	const FString RelativePath = GetFlatLeaderboardFilterIconPath(Filter);
	for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
	{
		if (!FPaths::FileExists(CandidatePath))
		{
			continue;
		}

		if (UTexture2D* Texture = T66RuntimeUITextureAccess::ImportFileTexture(
			CandidatePath,
			TextureFilter::TF_Nearest,
			false,
			TEXT("FlatLeaderboardFilterIcon")))
		{
			SetBrushTexture(Brush, Texture);
			break;
		}
	}

	if (!Brush->GetResourceObject())
	{
		return nullptr;
	}

	FilterIconBrushes.Add(Filter, Brush);
	return Brush.Get();
}

void ST66FlatLeaderboardPanel::SetBrushTexture(const TSharedPtr<FSlateBrush>& Brush, UTexture2D* Texture)
{
	if (!Brush.IsValid())
	{
		return;
	}

	Brush->SetResourceObject(Texture);
	TrackBrushTexture(Texture);
}

void ST66FlatLeaderboardPanel::TrackBrushTexture(UTexture2D* Texture)
{
	if (!IsValid(Texture))
	{
		return;
	}

	for (const TWeakObjectPtr<UTexture2D>& ExistingTexture : RootedBrushTextures)
	{
		if (ExistingTexture.Get() == Texture)
		{
			return;
		}
	}

	if (!Texture->IsRooted())
	{
		Texture->AddToRoot();
		RootedBrushTextures.Add(Texture);
	}
}

void ST66FlatLeaderboardPanel::ReleaseRootedBrushTextures()
{
	for (const TWeakObjectPtr<UTexture2D>& RootedTexture : RootedBrushTextures)
	{
		if (UTexture2D* Texture = RootedTexture.Get())
		{
			if (IsValid(Texture) && Texture->IsRooted())
			{
				Texture->RemoveFromRoot();
			}
		}
	}
	RootedBrushTextures.Reset();
}
