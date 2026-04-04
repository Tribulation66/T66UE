// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66WebImageCache.h"
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
#include "Widgets/Input/SComboButton.h"
#include "Framework/Application/SlateApplication.h"

namespace
{
	// Shared column widths so header and row columns align (Rank | Name | Score/Time).
	constexpr float RankColumnWidth = 60.0f;   // content + padding to name
	constexpr float ScoreColumnWidth = 98.0f; // content + padding, aligned with score digits (used for both Score and Time)
	constexpr float RowPortraitSize = 30.0f;
	constexpr int32 LeaderboardBodyFontSize = 13;
	constexpr int32 LeaderboardTitleFontSize = 13;
	constexpr int32 LeaderboardVisibleEntryCount = 10;
	const FLinearColor LeaderboardShellFill(0.004f, 0.005f, 0.010f, 0.985f);
	const FLinearColor LeaderboardFilterBorder(0.22f, 0.24f, 0.28f, 1.0f);
	const FLinearColor LeaderboardSelectedBorder(114.f / 255.f, 174.f / 255.f, 124.f / 255.f, 1.0f);
	constexpr bool GMirrorWeeklyToAllTime = true;

	int32 GetPartyMemberCount(ET66PartySize PartySize)
	{
		switch (PartySize)
		{
		case ET66PartySize::Duo: return 2;
		case ET66PartySize::Trio: return 3;
		case ET66PartySize::Quad: return 4;
		default: return 1;
		}
	}

	const TArray<FString>& GetFallbackLeaderboardNamePool()
	{
		static const TArray<FString> Pool = {
			TEXT("BestBuddy"), TEXT("SquadLeader"), TEXT("NightOwl"), TEXT("LootGoblin"),
			TEXT("DungeonDad"), TEXT("PixelPaladin"), TEXT("CouchWarrior"), TEXT("SneakySteve"),
			TEXT("RiftRunner"), TEXT("AshenWolf"), TEXT("NorthStar"), TEXT("ShadyDealer"),
			TEXT("GoldTooth"), TEXT("ArcLight"), TEXT("HollowKing"), TEXT("BrickHouse"),
			TEXT("LuckyShot"), TEXT("ZeroMercy"), TEXT("OldFaithful"), TEXT("HexDoctor"),
			TEXT("DeadeyeDan"), TEXT("SableFox"), TEXT("TorchBearer"), TEXT("SteelDrift"),
			TEXT("Moondrop"), TEXT("VileSaint"), TEXT("RavenCrow"), TEXT("BluntForce"),
			TEXT("WildCard"), TEXT("DustWalker"), TEXT("ColdBlood"), TEXT("WraithLine"),
			TEXT("LastLaugh"), TEXT("IronHorn"), TEXT("HighRoller"), TEXT("GrimNorth"),
			TEXT("Lockjaw"), TEXT("RatKing"), TEXT("Firebreak"), TEXT("Gravelord"),
			TEXT("DreadMoon"), TEXT("Crowbar"), TEXT("LongRoad"), TEXT("BoneSaw"),
			TEXT("RustBelt"), TEXT("SaintZero"), TEXT("BlackCoat"), TEXT("HardReset")
		};
		return Pool;
	}

	const TArray<FString>& GetSyntheticLeaderboardTokens()
	{
		static const TArray<FString> Tokens = {
			TEXT("global"), TEXT("friends"), TEXT("streamers"),
			TEXT("weekly"), TEXT("alltime"), TEXT("solo"), TEXT("duo"), TEXT("trio"), TEXT("quad"),
			TEXT("easy"), TEXT("medium"), TEXT("hard"), TEXT("very"), TEXT("veryhard"),
			TEXT("impossible"), TEXT("perdition"), TEXT("final"),
			TEXT("score"), TEXT("speed"), TEXT("speedrun"), TEXT("run"), TEXT("stage")
		};
		return Tokens;
	}

	bool IsSyntheticLeaderboardName(const FString& Name)
	{
		FString Sanitized = Name.TrimStartAndEnd().ToLower();
		if (Sanitized.IsEmpty())
		{
			return true;
		}

		Sanitized.ReplaceInline(TEXT("_"), TEXT(" "));
		Sanitized.ReplaceInline(TEXT("-"), TEXT(" "));

		FString TokenBuffer;
		TokenBuffer.Reserve(Sanitized.Len());
		for (const TCHAR Ch : Sanitized)
		{
			TokenBuffer.AppendChar(FChar::IsAlnum(Ch) ? Ch : TEXT(' '));
		}

		TArray<FString> Tokens;
		TokenBuffer.ParseIntoArrayWS(Tokens);

		int32 DescriptorMatches = 0;
		bool bHasFilterPrefix = false;
		for (const FString& Token : Tokens)
		{
			if (Token.IsNumeric())
			{
				continue;
			}
			if (Token == TEXT("g") || Token == TEXT("f") || Token == TEXT("s"))
			{
				bHasFilterPrefix = true;
				continue;
			}
			if (GetSyntheticLeaderboardTokens().Contains(Token))
			{
				++DescriptorMatches;
			}
		}

		return DescriptorMatches >= 3 || (bHasFilterPrefix && DescriptorMatches >= 2);
	}

	FName GetFallbackHeroId(UT66GameInstance* T66GI, int32 EntryIndex)
	{
		if (T66GI)
		{
			const TArray<FName> HeroIDs = T66GI->GetAllHeroIDs();
			if (HeroIDs.Num() > 0)
			{
				return HeroIDs[EntryIndex % HeroIDs.Num()];
			}
		}

		static const TArray<FName> DefaultHeroIDs = {
			FName(TEXT("Hero_1")), FName(TEXT("Hero_2")), FName(TEXT("Hero_3")), FName(TEXT("Hero_4")),
			FName(TEXT("Hero_5")), FName(TEXT("Hero_6")), FName(TEXT("Hero_7")), FName(TEXT("Hero_8")),
			FName(TEXT("Hero_9")), FName(TEXT("Hero_10")), FName(TEXT("Hero_11")), FName(TEXT("Hero_12")),
			FName(TEXT("Hero_13")), FName(TEXT("Hero_14")), FName(TEXT("Hero_15")), FName(TEXT("Hero_16"))
		};
		return DefaultHeroIDs[EntryIndex % DefaultHeroIDs.Num()];
	}

	const FScrollBoxStyle& GetLeaderboardRowScrollBoxStyle()
	{
		static FScrollBoxStyle Style = []()
		{
			FScrollBoxStyle ScrollStyle = FCoreStyle::Get().GetWidgetStyle<FScrollBoxStyle>("ScrollBox");
			const FSlateBrush NoBrush = *FCoreStyle::Get().GetBrush("NoBrush");
			ScrollStyle.SetTopShadowBrush(NoBrush);
			ScrollStyle.SetBottomShadowBrush(NoBrush);
			ScrollStyle.SetLeftShadowBrush(NoBrush);
			ScrollStyle.SetRightShadowBrush(NoBrush);
			return ScrollStyle;
		}();
		return Style;
	}

	class ST66LeaderboardRowWheelProxy : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66LeaderboardRowWheelProxy) {}
			SLATE_ARGUMENT(TSharedPtr<SScrollBox>, ScrollBox)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			ScrollBox = InArgs._ScrollBox;
			ChildSlot
			[
				InArgs._Content.Widget
			];
		}

		virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
		{
			if (!ScrollBox.IsValid())
			{
				return SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
			}

			const float PreviousOffset = ScrollBox->GetScrollOffset();
			const float ScrollDelta = -MouseEvent.GetWheelDelta() * 96.0f;
			ScrollBox->SetScrollOffset(PreviousOffset + ScrollDelta);

			if (!FMath::IsNearlyEqual(PreviousOffset, ScrollBox->GetScrollOffset()))
			{
				return FReply::Handled();
			}

			return SCompoundWidget::OnMouseWheel(MyGeometry, MouseEvent);
		}

	private:
		TSharedPtr<SScrollBox> ScrollBox;
	};
}

ST66LeaderboardPanel::~ST66LeaderboardPanel()
{
	// Unbind raw delegates to prevent callbacks into destroyed widget
	if (bBoundToBackendDelegate || bBoundToRunSummaryDelegate)
	{
		UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
		UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
		if (Backend)
		{
			if (bBoundToBackendDelegate)
			{
				Backend->OnLeaderboardDataReady.RemoveAll(this);
			}
			if (bBoundToRunSummaryDelegate)
			{
				Backend->OnRunSummaryReady.RemoveAll(this);
			}
		}
	}
}

void ST66LeaderboardPanel::GetStageRangeForDifficulty(const ET66Difficulty Difficulty, int32& OutStartStage, int32& OutEndStage)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:
	default:
		OutStartStage = 1;
		OutEndStage = 5;
		return;
	case ET66Difficulty::Medium:
		OutStartStage = 6;
		OutEndStage = 10;
		return;
	case ET66Difficulty::Hard:
		OutStartStage = 11;
		OutEndStage = 15;
		return;
	case ET66Difficulty::VeryHard:
		OutStartStage = 16;
		OutEndStage = 20;
		return;
	case ET66Difficulty::Impossible:
		OutStartStage = 21;
		OutEndStage = 23;
		return;
	}
}

void ST66LeaderboardPanel::UpdateStageOptionsForDifficulty()
{
	int32 StartStage = 1;
	int32 EndStage = 5;
	GetStageRangeForDifficulty(CurrentDifficulty, StartStage, EndStage);

	StageOptions.Reset();
	for (int32 Stage = StartStage; Stage <= EndStage; ++Stage)
	{
		StageOptions.Add(MakeShared<FString>(FString::FromInt(Stage)));
	}

	CurrentSpeedRunStage = StartStage;
	SelectedStageOption = StageOptions.Num() > 0 ? StageOptions[0] : MakeShared<FString>(FString::FromInt(StartStage));
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
	PartySizeOptions.Add(MakeShared<FString>(TEXT("Quad")));
	SelectedPartySizeOption = PartySizeOptions[0];

	DifficultyOptions.Add(MakeShared<FString>(TEXT("Easy")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Medium")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Hard")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Very Hard")));
	DifficultyOptions.Add(MakeShared<FString>(TEXT("Impossible")));
	SelectedDifficultyOption = DifficultyOptions[0];

	TypeOptions.Add(MakeShared<FString>(TEXT("Score")));
	TypeOptions.Add(MakeShared<FString>(TEXT("Speed Run")));
	SelectedTypeOption = TypeOptions[0];

	// Speed run stage dropdown is limited to the selected difficulty's live stage range.
	UpdateStageOptionsForDifficulty();

	// Default avatar brush (gray circle placeholder)
	DefaultAvatarBrush = MakeShared<FSlateBrush>();
	DefaultAvatarBrush->DrawAs = ESlateBrushDrawType::Image;
	DefaultAvatarBrush->ImageSize = FVector2D(32.0f, 32.0f);
	DefaultAvatarBrush->TintColor = FSlateColor(FLinearColor(0.14f, 0.15f, 0.17f, 0.32f));

	RefreshLeaderboard();

	const float SquareIconSize = 36.0f;
	const float FilterButtonGap = 5.0f;
	const float DropdownHeight = 34.0f;
	const float PartyDropdownMinWidth = 90.0f;
	const float DifficultyDropdownMinWidth = 104.0f;
	const float TypeDropdownMinWidth = 88.0f;
	const float StageDropdownMinWidth = 72.0f;

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

	LeaderboardDropdownStyle = MakeShared<FComboButtonStyle>(FT66Style::GetDropdownComboButtonStyle());
	LeaderboardDropdownStyle->ButtonStyle = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.FlatTransparent");
	LeaderboardDropdownStyle->ButtonStyle.SetNormalPadding(FMargin(0.f));
	LeaderboardDropdownStyle->ButtonStyle.SetPressedPadding(FMargin(0.f));
	const FSlateBrush* DropdownArrowBrush = &FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton").DownArrowImage;
	const FButtonStyle& NoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
	auto MakeRadianceFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Regular"), Size);
		Font.LetterSpacing = LetterSpacing;
		return Font;
	};
	auto MakeReaverBoldFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), Size);
		Font.LetterSpacing = LetterSpacing;
		return Font;
	};
	const FSlateFontInfo LeaderboardTitleFont = MakeRadianceFont(LeaderboardTitleFontSize, 88);
	const FSlateFontInfo LeaderboardBodyFont = MakeRadianceFont(LeaderboardBodyFontSize);
	const FSlateFontInfo LeaderboardDropdownFont = MakeRadianceFont(13);
	const FSlateFontInfo LeaderboardTimeToggleFont = MakeReaverBoldFont(LeaderboardTitleFontSize, 88);

	auto MakeIconButton = [this, SquareIconSize, LeaderboardTitleFont](ET66LeaderboardFilter Filter, FReply (ST66LeaderboardPanel::*ClickHandler)(), const FString& FallbackLetter, TSharedPtr<FSlateBrush> IconBrush) -> TSharedRef<SWidget>
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
						.Font(LeaderboardTitleFont)
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
					)
				]
			];
	};

	auto MakeFilterButtonShell = [this, &MakeIconButton](ET66LeaderboardFilter Filter, FReply (ST66LeaderboardPanel::*ClickHandler)(), const FString& FallbackLetter, TSharedPtr<FSlateBrush> IconBrush) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([this, Filter]() -> FSlateColor
			{
				return (CurrentFilter == Filter) ? FSlateColor(LeaderboardSelectedBorder) : FSlateColor(LeaderboardFilterBorder);
			}))
			.Padding(2.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(LeaderboardShellFill)
				.Padding(2.0f)
				[
					MakeIconButton(Filter, ClickHandler, FallbackLetter, IconBrush)
				]
			];
	};

	auto MakeFlatTimeButton = [this, &NoBorderButtonStyle, LeaderboardTimeToggleFont](const FText& Text, const FLinearColor& FillColor, FReply (ST66LeaderboardPanel::*ClickHandler)()) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonStyle(&NoBorderButtonStyle)
			.ContentPadding(FMargin(0.f))
			.OnClicked(FOnClicked::CreateSP(this, ClickHandler))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FillColor)
				.Padding(FMargin(12.f, 7.f, 12.f, 5.f))
				[
					SNew(STextBlock)
					.Text(Text)
					.Font(LeaderboardTimeToggleFont)
					.ColorAndOpacity(FLinearColor(0.96f, 0.96f, 0.94f, 1.0f))
					.Justification(ETextJustify::Center)
				]
			];
	};

	auto MakeTimeButton = [this, &MakeFlatTimeButton](const FText& Text, ET66LeaderboardTime Time, FReply (ST66LeaderboardPanel::*ClickHandler)()) -> TSharedRef<SWidget>
	{
		const TAttribute<EVisibility> SelectedVisibility = TAttribute<EVisibility>::CreateLambda([this, Time]() -> EVisibility
		{
			return (CurrentTimeFilter == Time) ? EVisibility::Visible : EVisibility::Collapsed;
		});

		const TAttribute<EVisibility> UnselectedVisibility = TAttribute<EVisibility>::CreateLambda([this, Time]() -> EVisibility
		{
			return (CurrentTimeFilter == Time) ? EVisibility::Collapsed : EVisibility::Visible;
		});

		auto MakeVariant = [Text, ClickHandler, &MakeFlatTimeButton](const FLinearColor& FillColor, const TAttribute<EVisibility>& Visibility) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.Visibility(Visibility)
				[
					MakeFlatTimeButton(Text, FillColor, ClickHandler)
				];
		};

		return SNew(SBox)
			.MinDesiredWidth(116.f)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					MakeVariant(FLinearColor(0.18f, 0.31f, 0.18f, 0.97f), SelectedVisibility)
				]
				+ SOverlay::Slot()
				[
					MakeVariant(FLinearColor(0.26f, 0.29f, 0.34f, 0.96f), UnselectedVisibility)
				]
			];
	};

	auto MakeLeaderboardDropdown = [this, DropdownHeight, DropdownArrowBrush](const TSharedRef<SWidget>& TriggerContent, TFunction<TSharedRef<SWidget>()> OnGetMenuContent, float MinWidth, const TAttribute<EVisibility>& Visibility = EVisibility::Visible) -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.07f, 0.08f, 0.11f, 0.98f))
			.Padding(FMargin(0.f))
			.Visibility(Visibility)
			[
				SNew(SBox)
					.MinDesiredWidth(MinWidth)
					.HeightOverride(DropdownHeight)
				[
					SNew(SComboButton)
					.ComboButtonStyle(LeaderboardDropdownStyle.Get())
					.IsFocusable(false)
					.HasDownArrow(false)
					.OnGetMenuContent_Lambda([OnGetMenuContent]() { return OnGetMenuContent(); })
					.ContentPadding(FMargin(0.f))
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					.ButtonContent()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
						.Padding(FMargin(6.f, 5.f, 4.f, 4.f))
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								TriggerContent
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(8.0f, 0.0f, 0.0f, 0.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(DropdownArrowBrush)
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
					]
				]
			];
	};

	FText WeeklyText = LocSubsystem ? LocSubsystem->GetText_Weekly() : NSLOCTEXT("T66.Leaderboard", "Weekly", "WEEKLY");
	FText AllTimeText = NSLOCTEXT("T66.Leaderboard", "AllTime", "ALL TIME");

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeFilterButtonShell(ET66LeaderboardFilter::Global, &ST66LeaderboardPanel::HandleGlobalClicked, TEXT("G"), FilterBrushGlobal)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(FilterButtonGap, 0.f, 0.f, 0.f)
			[
				MakeFilterButtonShell(ET66LeaderboardFilter::Friends, &ST66LeaderboardPanel::HandleFriendsClicked, TEXT("F"), FilterBrushFriends)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(FilterButtonGap, 0.f, 0.f, 0.f)
			[
				MakeFilterButtonShell(ET66LeaderboardFilter::Streamers, &ST66LeaderboardPanel::HandleStreamersClicked, TEXT("S"), FilterBrushStreamers)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(LeaderboardShellFill)
			.Padding(FMargin(12.f, 8.f, 12.f, 10.f))
			[
				SNew(SScrollBox)
				.ScrollBarVisibility(EVisibility::Visible)
				+ SScrollBox::Slot()
				[
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
									UIManager->ShowScreen(ET66ScreenType::AccountStatus);
								}
								return FReply::Handled();
							})
						)
						.SetMinWidth(128.f)
						.SetFontWeight(TEXT("Regular"))
						.SetPadding(FMargin(10.f, 5.f))
						.SetColor(LeaderboardShellFill)
						.SetEnabled(TAttribute<bool>::CreateLambda([this]() { return UIManager != nullptr; }))
						.SetVisibility(TAttribute<EVisibility>::CreateLambda([this]()
						{
							return (LeaderboardSubsystem && LeaderboardSubsystem->ShouldShowAccountStatusButton())
								? EVisibility::Visible
								: EVisibility::Collapsed;
						})
					))
					]
					// Time toggles (Weekly | All Time)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 10.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(3.0f, 0.0f)
						[ MakeTimeButton(WeeklyText, ET66LeaderboardTime::Current, &ST66LeaderboardPanel::HandleCurrentClicked) ]
						+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(3.0f, 0.0f)
						[ MakeTimeButton(AllTimeText, ET66LeaderboardTime::AllTime, &ST66LeaderboardPanel::HandleAllTimeClicked) ]
					]
					// Dropdowns row (Party Size | Difficulty)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.Padding(0.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(SHorizontalBox)
						// Party Size dropdown
						+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.0f, 0.0f, 3.0f, 0.0f)
						[
							MakeLeaderboardDropdown(
								SNew(STextBlock)
									.Text_Lambda([this]() { return GetPartySizeText(CurrentPartySize); })
									.Font(LeaderboardDropdownFont)
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
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
												}), ET66ButtonType::Neutral).SetMinWidth(0.f).SetFontWeight(TEXT("Regular")))
											];
									}
									return Box;
								},
								PartyDropdownMinWidth)
						]
						// Difficulty dropdown
						+ SHorizontalBox::Slot().FillWidth(1.0f)
						[
							MakeLeaderboardDropdown(
								SNew(STextBlock)
									.Text_Lambda([this]() { return GetDifficultyText(CurrentDifficulty); })
									.Font(LeaderboardDropdownFont)
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
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
												}), ET66ButtonType::Neutral).SetMinWidth(0.f).SetFontWeight(TEXT("Regular")))
											];
									}
									return Box;
								},
								DifficultyDropdownMinWidth)
						]
					]
					// Type dropdown row
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.Padding(0.0f, 0.0f, 0.0f, 18.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.0f)
						[
							MakeLeaderboardDropdown(
								SNew(STextBlock)
									.Text_Lambda([this]() { return GetTypeText(CurrentType); })
									.Font(LeaderboardDropdownFont)
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
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
												}), ET66ButtonType::Neutral).SetMinWidth(0.f).SetFontWeight(TEXT("Regular")))
											];
									}
									return Box;
								},
								TypeDropdownMinWidth)
						]
						// Stage dropdown (hidden — wiring kept for future use)
						+ SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
						[
							MakeLeaderboardDropdown(
									SNew(STextBlock)
										.Text_Lambda([this]() { return SelectedStageOption.IsValid() ? FText::FromString(*SelectedStageOption) : FText::AsNumber(1); })
										.Font(LeaderboardDropdownFont)
										.ColorAndOpacity(FT66Style::Tokens::Text)
										.OverflowPolicy(ETextOverflowPolicy::Ellipsis),
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
													}), ET66ButtonType::Neutral).SetMinWidth(0.f).SetFontWeight(TEXT("Regular")))
												];
										}
										return Box;
									},
									StageDropdownMinWidth,
									EVisibility::Collapsed)
						]
					]
					// Column headers
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
						.Padding(FMargin(8.0f, 5.0f))
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
								.Font(LeaderboardTitleFont)
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
									.Font(LeaderboardTitleFont)
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Right)
								]
							]
						]
					]
					// Entry list
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SAssignNew(EntryListBox, SVerticalBox)
					]
				]
			]
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
	// Placeholder data has been replaced by backend data. This stub is kept
	// only as a compile-time safety net; it should never be reached at runtime
	// when the backend is configured.
	LeaderboardEntries.Empty();
}

void ST66LeaderboardPanel::RebuildEntryList()
{
	if (!EntryListBox.IsValid()) return;

	EntryListBox->ClearChildren();

	if (LeaderboardEntries.Num() == 0 && LeaderboardSubsystem)
	{
		LeaderboardEntries = LeaderboardSubsystem->BuildEntriesForFilter(
			CurrentFilter,
			CurrentType,
			CurrentDifficulty,
			CurrentPartySize,
			CurrentSpeedRunStage);

		for (int32 EntryIndex = 0; EntryIndex < LeaderboardEntries.Num(); ++EntryIndex)
		{
			NormalizeEntryIdentity(LeaderboardEntries[EntryIndex], EntryIndex);
		}
	}

	if (LeaderboardEntries.Num() == 0)
	{
		EntryListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 10.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.Leaderboard", "NoEntriesFallback", "No leaderboard data yet"))
			.Font(FT66Style::Tokens::FontRegular(LeaderboardBodyFontSize))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		];
		return;
	}

	for (const FLeaderboardEntry& Entry : LeaderboardEntries)
	{
		const FLinearColor BaseRowColor = Entry.bIsLocalPlayer
			? FLinearColor(0.2f, 0.4f, 0.3f, 0.35f)
			: FLinearColor::Transparent;
		const FLinearColor HoverRowColor = Entry.bIsLocalPlayer
			? FLinearColor(0.24f, 0.46f, 0.34f, 0.44f)
			: FLinearColor(0.28f, 0.29f, 0.32f, 0.70f);
		const TSharedRef<bool> bIsRowHovered = MakeShared<bool>(false);

		// Format time as MM:SS
		FString TimeStr = FormatTime(Entry.TimeSeconds);

		// Format score with commas
		FString ScoreStr = FString::Printf(TEXT("%lld"), Entry.Score);

		TArray<FString> DisplayNames = Entry.PlayerNames;
		if (DisplayNames.Num() == 0 && !Entry.PlayerName.IsEmpty())
		{
			DisplayNames.Add(Entry.PlayerName);
		}

		UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
		UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
		const float NameFontSize = static_cast<float>(LeaderboardBodyFontSize);
		const float MemberPortraitSize = FMath::RoundToFloat(NameFontSize + 6.0f);
		const float PartyStripMinHeight = FMath::Max(MemberPortraitSize, NameFontSize) - 1.0f;
		const float RowVerticalPadding = FMath::Max(2.0f, FMath::RoundToFloat(NameFontSize * 0.18f));
		TSharedRef<SHorizontalBox> NameColumn = SNew(SHorizontalBox);
		for (int32 NameIndex = 0; NameIndex < DisplayNames.Num(); ++NameIndex)
		{
			const FString& DisplayName = DisplayNames[NameIndex];
			FName MemberHeroID = (NameIndex == 0 && !Entry.HeroID.IsNone())
				? Entry.HeroID
				: GetFallbackHeroId(T66GI, (FMath::Max(1, Entry.Rank) - 1) * 4 + NameIndex);

			const FSlateBrush* MemberPortrait = nullptr;
			if (NameIndex == 0 && !Entry.AvatarUrl.IsEmpty())
			{
				const FSlateBrush* AvatarBrush = GetOrCreateAvatarBrush(Entry.AvatarUrl);
				if (AvatarBrush && AvatarBrush != DefaultAvatarBrush.Get())
				{
					MemberPortrait = AvatarBrush;
				}
			}
			if (MemberPortrait == nullptr)
			{
				MemberPortrait = GetOrCreateHeroPortraitBrush(MemberHeroID);
			}

			NameColumn->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(NameIndex > 0 ? FMargin(8.0f, 0.0f, 0.0f, 0.0f) : FMargin(0.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(MemberPortraitSize)
					.HeightOverride(MemberPortraitSize)
					[
						SNew(SImage)
						.Image(MemberPortrait)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(DisplayName))
					.Font(FT66Style::Tokens::FontRegular(NameFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			];
		}

		TSharedPtr<SScrollBox> PartyStripScrollBox;
		const TSharedRef<SWidget> PartyStripScroller =
			SNew(SBox)
			.MinDesiredHeight(PartyStripMinHeight)
			[
				SAssignNew(PartyStripScrollBox, SScrollBox)
				.Style(&GetLeaderboardRowScrollBoxStyle())
				.Orientation(Orient_Horizontal)
				.ScrollBarVisibility(EVisibility::Collapsed)
				+ SScrollBox::Slot()
				[
					NameColumn
				]
			];
		if (PartyStripScrollBox.IsValid())
		{
			PartyStripScrollBox->SetAnimateWheelScrolling(false);
			PartyStripScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::Always);
		}

		// Build the row content: Rank | Avatar | Name | Score/Time
		const TSharedRef<SWidget> RowContents =
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([bIsRowHovered, BaseRowColor, HoverRowColor]() -> FSlateColor
			{
				return FSlateColor(*bIsRowHovered ? HoverRowColor : BaseRowColor);
			}))
			.Padding(FMargin(7.0f, RowVerticalPadding))
			[
				SNew(SHorizontalBox)
				// Column 1: Rank (fixed width)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(RankColumnWidth)
					[
						SNew(STextBlock)
						.Text(FText::Format(NSLOCTEXT("T66.Leaderboard", "RankFormat", "#{0}"), FText::AsNumber(Entry.Rank)))
						.Font(FT66Style::Tokens::FontRegular(LeaderboardBodyFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
				]
				// Column 2: Party strip (portrait + name, repeated horizontally)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					PartyStripScroller
				]
				// Column 3: Score or Time (fixed width, right-aligned)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(ScoreColumnWidth)
					[
						SNew(STextBlock)
						.Text(FText::FromString(CurrentType == ET66LeaderboardType::Score ? ScoreStr : TimeStr))
						.Font(FT66Style::Tokens::FontRegular(LeaderboardBodyFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Right)
					]
				]
			];

		const TSharedRef<SWidget> RowWidget =
			SNew(SButton)
			.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(FMargin(0.f))
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.OnHovered(FSimpleDelegate::CreateLambda([bIsRowHovered]() { *bIsRowHovered = true; }))
			.OnUnhovered(FSimpleDelegate::CreateLambda([bIsRowHovered]() { *bIsRowHovered = false; }))
			.OnClicked(FOnClicked::CreateLambda([this, Entry]() { return HandleEntryClicked(Entry); }))
			[
				SNew(ST66LeaderboardRowWheelProxy)
				.ScrollBox(PartyStripScrollBox)
				[
					RowContents
				]
			];

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
		if (UIManager->GetCurrentModalType() == ET66ScreenType::Leaderboard)
		{
			LeaderboardSubsystem->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::Leaderboard);
		}
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
	if (CurrentTimeFilter == NewTime)
	{
		return;
	}

	CurrentTimeFilter = NewTime;

	if (GMirrorWeeklyToAllTime)
	{
		Invalidate(EInvalidateWidgetReason::Layout);
		return;
	}

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
	UpdateStageOptionsForDifficulty();
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::SetLeaderboardType(ET66LeaderboardType NewType)
{
	CurrentType = NewType;
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::RefreshLeaderboard()
{
	// Temporary frontend art pass: weekly mirrors all-time so the menu always has data
	// and we do not visually reset the board while weekly backend content is still pending.
	const TArray<FLeaderboardEntry> PreviousEntries = LeaderboardEntries;

	// Convert current filters to backend API strings
	auto TypeStr = [this]() -> FString {
		return (CurrentType == ET66LeaderboardType::Score) ? TEXT("score") : TEXT("speedrun");
	};
	auto TimeStr = [this]() -> FString {
		if (GMirrorWeeklyToAllTime)
		{
			return TEXT("alltime");
		}
		return (CurrentTimeFilter == ET66LeaderboardTime::AllTime) ? TEXT("alltime") : TEXT("weekly");
	};
	auto PartyStr = [this]() -> FString {
		switch (CurrentPartySize) {
		case ET66PartySize::Duo: return TEXT("duo");
		case ET66PartySize::Trio: return TEXT("trio");
		case ET66PartySize::Quad: return TEXT("quad");
		default: return TEXT("solo");
		}
	};
	auto DiffStr = [this]() -> FString {
		switch (CurrentDifficulty) {
		case ET66Difficulty::Medium: return TEXT("medium");
		case ET66Difficulty::Hard: return TEXT("hard");
		case ET66Difficulty::VeryHard: return TEXT("veryhard");
		case ET66Difficulty::Impossible: return TEXT("impossible");
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
	const TArray<FLeaderboardEntry> LocalFallbackEntries = LeaderboardSubsystem
		? LeaderboardSubsystem->BuildEntriesForFilter(CurrentFilter, CurrentType, CurrentDifficulty, CurrentPartySize, CurrentSpeedRunStage)
		: TArray<FLeaderboardEntry>();

	if (Backend && Backend->IsBackendConfigured() && Backend->HasCachedLeaderboard(CacheKey))
	{
		const TArray<FLeaderboardEntry> CachedEntries = Backend->GetCachedLeaderboard(CacheKey);
		const int32 CachedTotalEntries = Backend->GetCachedTotalEntries(CacheKey);

		if (CachedEntries.Num() > 0)
		{
			// Use backend data when the backend actually has something for this key.
			LeaderboardEntries = CachedEntries;
			while (LeaderboardEntries.Num() < LeaderboardVisibleEntryCount)
			{
				const int32 NextRankToFill = LeaderboardEntries.Num() + 1;
				const FLeaderboardEntry* FallbackEntry = LocalFallbackEntries.FindByPredicate([NextRankToFill](const FLeaderboardEntry& Entry)
				{
					return !Entry.bIsLocalPlayer && Entry.Rank == NextRankToFill;
				});
				if (!FallbackEntry)
				{
					break;
				}
				LeaderboardEntries.Add(*FallbackEntry);
			}

			// Find the local player entry from local data
			const FLeaderboardEntry* LocalYou = nullptr;
			for (const FLeaderboardEntry& E : LocalFallbackEntries)
			{
				if (E.bIsLocalPlayer)
				{
					LocalYou = &E;
					break;
				}
			}

			const bool bHasLocalResult =
				LocalYou && ((CurrentType == ET66LeaderboardType::Score && LocalYou->Score > 0)
					|| (CurrentType == ET66LeaderboardType::SpeedRun && LocalYou->TimeSeconds > 0.01f));

			if (bHasLocalResult)
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
					You.Rank = LeaderboardVisibleEntryCount + 1;
					LeaderboardEntries.Add(You);
				}
			}
		}
		else
		{
			// Empty backend payload should never blank the board, even if the backend reported a non-zero total.
			// Keep the local/generated data on screen until we have actual rows to render.
			LeaderboardEntries = LocalFallbackEntries;
		}
	}
	else if (Backend && Backend->IsBackendConfigured())
	{
		// While the async fetch is pending, keep the local/generated board visible.
		LeaderboardEntries = LocalFallbackEntries;
	}
	else
	{
		LeaderboardEntries = LocalFallbackEntries;
	}

	if (LeaderboardEntries.Num() == 0)
	{
		LeaderboardEntries = LocalFallbackEntries.Num() > 0 ? LocalFallbackEntries : PreviousEntries;
	}

	auto HasVisibleResult = [this](const FLeaderboardEntry& Entry) -> bool
	{
		return (CurrentType == ET66LeaderboardType::Score && Entry.Score > 0)
			|| (CurrentType == ET66LeaderboardType::SpeedRun && Entry.TimeSeconds > 0.01f);
	};

	auto FindLocalEntry = [&HasVisibleResult](const TArray<FLeaderboardEntry>& Entries) -> const FLeaderboardEntry*
	{
		for (const FLeaderboardEntry& Entry : Entries)
		{
			if (Entry.bIsLocalPlayer && HasVisibleResult(Entry))
			{
				return &Entry;
			}
		}

		return nullptr;
	};

	const FLeaderboardEntry* LocalVisibleEntry = FindLocalEntry(LeaderboardEntries);
	if (LocalVisibleEntry == nullptr)
	{
		LocalVisibleEntry = FindLocalEntry(LocalFallbackEntries);
	}

	TArray<FLeaderboardEntry> LimitedEntries;
	LimitedEntries.Reserve(LeaderboardVisibleEntryCount + 1);

	bool bLocalAlreadyVisible = false;
	for (const FLeaderboardEntry& Entry : LeaderboardEntries)
	{
		if (LimitedEntries.Num() >= LeaderboardVisibleEntryCount)
		{
			break;
		}

		if (Entry.bIsLocalPlayer)
		{
			bLocalAlreadyVisible = true;
		}

		LimitedEntries.Add(Entry);
	}

	if (!bLocalAlreadyVisible && LocalVisibleEntry != nullptr)
	{
		FLeaderboardEntry LocalRow = *LocalVisibleEntry;
		if (LocalRow.Rank <= 0)
		{
			LocalRow.Rank = LeaderboardVisibleEntryCount + 1;
		}
		LocalRow.bIsLocalPlayer = true;
		LimitedEntries.Add(LocalRow);
	}

	LeaderboardEntries = MoveTemp(LimitedEntries);

	for (int32 EntryIndex = 0; EntryIndex < LeaderboardEntries.Num(); ++EntryIndex)
	{
		NormalizeEntryIdentity(LeaderboardEntries[EntryIndex], EntryIndex);
	}

	RebuildEntryList();

	// Fire async backend fetch if not cached yet (leaderboard endpoint is public, no auth needed)
	if (Backend && Backend->IsBackendConfigured() && !Backend->HasCachedLeaderboard(CacheKey))
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
	else if (*NewSelection == TEXT("Quad")) SetPartySize(ET66PartySize::Quad);
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
	int32 StartStage = 1;
	int32 EndStage = 5;
	GetStageRangeForDifficulty(CurrentDifficulty, StartStage, EndStage);
	CurrentSpeedRunStage = FMath::Clamp(FCString::Atoi(**NewSelection), StartStage, EndStage);
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
		case ET66PartySize::Solo: return NSLOCTEXT("T66.PartySize", "Solo", "Solo");
		case ET66PartySize::Duo: return NSLOCTEXT("T66.PartySize", "Duo", "Duo");
		case ET66PartySize::Trio: return NSLOCTEXT("T66.PartySize", "Trio", "Trio");
		case ET66PartySize::Quad: return NSLOCTEXT("T66.PartySize", "Quad", "Quad");
		default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
		}
	}

	switch (Size)
	{
	case ET66PartySize::Solo: return LocSubsystem->GetText_Solo();
	case ET66PartySize::Duo: return LocSubsystem->GetText_Duo();
	case ET66PartySize::Trio: return LocSubsystem->GetText_Trio();
	case ET66PartySize::Quad: return LocSubsystem->GetText_Quad();
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
	case ET66LeaderboardType::Score: return NSLOCTEXT("T66.Leaderboard", "Score", "Score");
	case ET66LeaderboardType::SpeedRun: return NSLOCTEXT("T66.Leaderboard", "SpeedRun", "Speed Run");
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
					if (UIManager->GetCurrentModalType() == ET66ScreenType::Leaderboard)
					{
						LeaderboardSubsystem->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::Leaderboard);
					}
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

	const int32 PartyCount =
		(Entry.PartySize == ET66PartySize::Quad) ? 4 :
		(Entry.PartySize == ET66PartySize::Trio) ? 3 :
		(Entry.PartySize == ET66PartySize::Duo) ? 2 : 1;

	if (PartyCount == 1)
	{
		// Solo: open Run Summary with one fake snapshot.
		if (UT66LeaderboardRunSummarySaveGame* Snap = LeaderboardSubsystem->CreateFakeRunSummarySnapshot(
			CurrentFilter, CurrentType, CurrentDifficulty, CurrentPartySize,
			Entry.Rank, 0, Entry.PlayerName, Entry.Score, Entry.TimeSeconds))
		{
			if (UIManager->GetCurrentModalType() == ET66ScreenType::Leaderboard)
			{
				LeaderboardSubsystem->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::Leaderboard);
			}
			LeaderboardSubsystem->SetPendingFakeRunSummarySnapshot(Snap);
			UIManager->ShowModal(ET66ScreenType::RunSummary);
		}
		return FReply::Handled();
	}

	// Duo / Trio / Quad: open Pick the Player with one snapshot per party member.
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
		if (UIManager->GetCurrentModalType() == ET66ScreenType::Leaderboard)
		{
			LeaderboardSubsystem->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::Leaderboard);
		}
		LeaderboardSubsystem->SetPendingPickerSnapshots(Snapshots);
		UIManager->ShowModal(ET66ScreenType::PlayerSummaryPicker);
	}
	return FReply::Handled();
}

void ST66LeaderboardPanel::NormalizeEntryIdentity(FLeaderboardEntry& Entry, int32 EntryIndex)
{
	UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;

	const int32 PartyMemberCount = GetPartyMemberCount(Entry.PartySize);
	const TArray<FString>& FallbackNames = GetFallbackLeaderboardNamePool();
	const int32 FallbackStartIndex = FMath::Max(0, EntryIndex) * 4;

	TArray<FString> CleanNames;
	auto TryAddCleanName = [&CleanNames](const FString& Candidate)
	{
		const FString Trimmed = Candidate.TrimStartAndEnd();
		if (Trimmed.IsEmpty() || IsSyntheticLeaderboardName(Trimmed) || CleanNames.Contains(Trimmed))
		{
			return;
		}
		CleanNames.Add(Trimmed);
	};

	for (const FString& Name : Entry.PlayerNames)
	{
		TryAddCleanName(Name);
	}
	TryAddCleanName(Entry.PlayerName);

	if (CleanNames.Num() == 0)
	{
		const FString LocalFallbackName = Entry.bIsLocalPlayer
			? (!Entry.PlayerName.TrimStartAndEnd().IsEmpty() ? Entry.PlayerName.TrimStartAndEnd() : TEXT("YOU"))
			: FString();

		if (!LocalFallbackName.IsEmpty())
		{
			CleanNames.Add(LocalFallbackName);
		}
	}

	int32 FallbackCursor = 0;
	while (CleanNames.Num() < PartyMemberCount)
	{
		const FString& Candidate = FallbackNames[(FallbackStartIndex + FallbackCursor) % FallbackNames.Num()];
		++FallbackCursor;
		if (!CleanNames.Contains(Candidate))
		{
			CleanNames.Add(Candidate);
		}
	}

	if (CleanNames.Num() == 0)
	{
		CleanNames.Add(TEXT("Unknown"));
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
			Entry.HeroID = GetFallbackHeroId(T66GI, FallbackStartIndex);
		}
	}
}

const FSlateBrush* ST66LeaderboardPanel::GetPortraitBrushForEntry(const FLeaderboardEntry& Entry)
{
	if (!Entry.AvatarUrl.IsEmpty())
	{
		const FSlateBrush* AvatarBrush = GetOrCreateAvatarBrush(Entry.AvatarUrl);
		if (AvatarBrush && AvatarBrush != DefaultAvatarBrush.Get())
		{
			return AvatarBrush;
		}
	}

	if (!Entry.HeroID.IsNone())
	{
		if (const FSlateBrush* HeroBrush = GetOrCreateHeroPortraitBrush(Entry.HeroID))
		{
			return HeroBrush;
		}
	}

	return DefaultAvatarBrush.Get();
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
	if (UIManager->GetCurrentModalType() == ET66ScreenType::Leaderboard)
	{
		LeaderboardSubsystem->SetPendingReturnModalAfterViewerRunSummary(ET66ScreenType::Leaderboard);
	}
	UIManager->ShowModal(ET66ScreenType::RunSummary);
	return FReply::Handled();
}

const FSlateBrush* ST66LeaderboardPanel::GetOrCreateAvatarBrush(const FString& AvatarUrl)
{
	if (AvatarUrl.IsEmpty())
	{
		return DefaultAvatarBrush.Get();
	}

	// Return cached brush if already created
	if (TSharedPtr<FSlateBrush>* Found = AvatarBrushes.Find(AvatarUrl))
	{
		return Found->Get();
	}

	// Check web image cache
	UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
	UT66WebImageCache* ImageCache = GI ? GI->GetSubsystem<UT66WebImageCache>() : nullptr;
	if (!ImageCache)
	{
		return DefaultAvatarBrush.Get();
	}

	if (UTexture2D* Tex = ImageCache->GetCachedImage(AvatarUrl))
	{
		TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
		Brush->SetResourceObject(Tex);
		Brush->ImageSize = FVector2D(32.0f, 32.0f);
		AvatarBrushes.Add(AvatarUrl, Brush);
		return Brush.Get();
	}

	// Not yet downloaded — request it and return default for now (callback runs on game thread; weak ref in case panel was destroyed)
	TWeakPtr<ST66LeaderboardPanel> WeakPanel = StaticCastSharedRef<ST66LeaderboardPanel>(AsShared());
	ImageCache->RequestImage(AvatarUrl, [WeakPanel](UTexture2D* Tex)
	{
		if (Tex)
		{
			if (TSharedPtr<ST66LeaderboardPanel> Pin = WeakPanel.Pin())
			{
				Pin->RebuildEntryList();
			}
		}
	});

	return DefaultAvatarBrush.Get();
}

const FSlateBrush* ST66LeaderboardPanel::GetOrCreateHeroPortraitBrush(FName HeroID)
{
	if (HeroID.IsNone())
	{
		return DefaultAvatarBrush.Get();
	}

	if (TSharedPtr<FSlateBrush>* Found = HeroPortraitBrushes.Find(HeroID))
	{
		return Found->Get();
	}

	UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	UT66UITexturePoolSubsystem* TexPool = GI ? GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (!T66GI || !TexPool)
	{
		return DefaultAvatarBrush.Get();
	}

	const TSoftObjectPtr<UTexture2D> PortraitSoft = T66GI->ResolveHeroPortrait(HeroID, ET66BodyType::TypeA, ET66HeroPortraitVariant::Low);
	if (PortraitSoft.IsNull())
	{
		return DefaultAvatarBrush.Get();
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->DrawAs = ESlateBrushDrawType::Image;
	Brush->Tiling = ESlateBrushTileType::NoTile;
	Brush->ImageSize = FVector2D(RowPortraitSize, RowPortraitSize);

	if (TexPool)
	{
		Brush->SetResourceObject(T66SlateTexture::GetLoaded(TexPool, PortraitSoft));
		if (UObject* Requester = UIManager ? static_cast<UObject*>(UIManager) : static_cast<UObject*>(GI))
		{
			T66SlateTexture::BindSharedBrushAsync(
				TexPool,
				PortraitSoft,
				Requester,
				Brush,
				FName(*FString::Printf(TEXT("LBHero_%s"), *HeroID.ToString())),
				/*bClearWhileLoading*/ false);
		}
	}

	HeroPortraitBrushes.Add(HeroID, Brush);
	return Brush.Get();
}
