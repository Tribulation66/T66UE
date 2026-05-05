// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Components/T66LeaderboardPanel.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Core/T66WebImageCache.h"
#include "Core/T66LeaderboardRunSummarySaveGame.h"
#include "Core/T66LocalizationSubsystem.h"
#include "UI/T66UITypes.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66Style.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
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
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderTransform.h"

namespace
{
	// Shared column widths so header and row columns align (Rank | Name | Score/Time).
	constexpr float RankColumnWidth = 60.0f;   // content + padding to name
	constexpr float ScoreColumnWidth = 98.0f; // content + padding, aligned with score digits (used for both Score and Time)
	constexpr float RowPortraitSize = 30.0f;
	constexpr int32 LeaderboardBodyFontSize = 12;
	constexpr int32 LeaderboardTitleFontSize = 12;
	constexpr int32 LeaderboardVisibleEntryCount = 10;
	const FLinearColor LeaderboardShellFill(0.78f, 0.55f, 0.28f, 0.0f);
	constexpr bool GMirrorWeeklyToAllTime = false;
	const FString ReferenceRightPanelSourceDir = TEXT("SourceAssets/UI/MainMenuReference/RightPanel");
	const FString ReferenceSharedSourceDir = TEXT("SourceAssets/UI/Reference/Shared");

	const FVector2D ReferenceFilterButtonSize(64.0f, 64.0f);
	const FVector2D ReferenceFilterIconSize(58.0f, 58.0f);
	const FVector2D ReferenceWeeklyTabSize(219.0f, 57.0f);
	const FVector2D ReferenceAllTimeTabSize(219.0f, 57.0f);
	const FVector2D ReferenceLeftDropdownSize(219.0f, 57.0f);
	const FVector2D ReferenceRightDropdownSize(219.0f, 57.0f);
	const FVector2D ReferenceScoreToggleSize(178.0f, 36.0f);
	const FVector2D ReferenceSpeedRunToggleSize(206.0f, 36.0f);
	const FVector2D ReferenceAvatarFrameSize(42.0f, 42.0f);
	const FVector2D ReferenceAvatarInsetSize(32.0f, 32.0f);
	const FLinearColor ReferenceLeaderboardText(0.17f, 0.09f, 0.035f, 1.0f);
	const FLinearColor ReferenceLeaderboardRowText(0.13f, 0.075f, 0.035f, 1.0f);
	const FLinearColor ReferenceLeaderboardMuted(0.36f, 0.22f, 0.12f, 1.0f);
	const FLinearColor ReferenceWoodControlText(0.98f, 0.96f, 0.90f, 1.0f);
	const FLinearColor ReferenceWoodControlMuted(0.86f, 0.84f, 0.78f, 0.88f);
	const FLinearColor ReferenceLeaderboardTitleText(0.84f, 0.64f, 0.38f, 1.0f);
	const FLinearColor ReferenceLeaderboardTitleShadow(0.13f, 0.07f, 0.03f, 0.96f);

	const FSlateBrush* ResolveReferenceRightPanelBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* FileName,
		const FMargin Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(ReferenceRightPanelSourceDir / FileName),
			Margin,
			DebugLabel,
			TextureFilter::TF_Trilinear);
	}

	const FSlateBrush* ResolveMasterLibrarySliceBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const TCHAR* FileName,
		const FMargin Margin,
		const TCHAR* DebugLabel,
		TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(ReferenceSharedSourceDir / FileName),
			Margin,
			DebugLabel,
			Filter);
	}

	const FSlateBrush* ResolveReferenceGeneratedBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& SourceDir,
		const TCHAR* FileName,
		const FMargin Margin,
		const TCHAR* DebugLabel,
		TextureFilter Filter = TextureFilter::TF_Trilinear)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(SourceDir / FileName),
			Margin,
			DebugLabel,
			Filter);
	}

	FVector2D GetLeaderboardBrushSourceSize(const FSlateBrush* SourceBrush)
	{
		if (!SourceBrush)
		{
			return FVector2D(1.f, 1.f);
		}

		FVector2D SourceSize(
			FMath::Max(1.f, SourceBrush->ImageSize.X),
			FMath::Max(1.f, SourceBrush->ImageSize.Y));
		if ((SourceSize.X <= 1.f || SourceSize.Y <= 1.f) && SourceBrush->GetResourceObject())
		{
			if (const UTexture2D* Texture = Cast<UTexture2D>(SourceBrush->GetResourceObject()))
			{
				SourceSize = FVector2D(
					FMath::Max(1, Texture->GetSizeX()),
					FMath::Max(1, Texture->GetSizeY()));
			}
		}

		return SourceSize;
	}

	class ST66LeaderboardSlicedBrushImage : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66LeaderboardSlicedBrushImage)
			: _Brush(nullptr)
			, _DesiredSize(FVector2D(1.f, 1.f))
			, _SourceCapFraction(0.245f)
		{}
			SLATE_ATTRIBUTE(const FSlateBrush*, Brush)
			SLATE_ARGUMENT(FVector2D, DesiredSize)
			SLATE_ARGUMENT(float, SourceCapFraction)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Brush = InArgs._Brush;
			DesiredSize = InArgs._DesiredSize;
			SourceCapFraction = InArgs._SourceCapFraction;
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return DesiredSize;
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
			FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FSlateBrush* SourceBrush = Brush.Get();
			if (!SourceBrush || SourceBrush == FCoreStyle::Get().GetBrush("NoBrush"))
			{
				return LayerId;
			}

			const FVector2D Size = AllottedGeometry.GetLocalSize();
			const FVector2D SourceSize = GetLeaderboardBrushSourceSize(SourceBrush);
			if (Size.X <= 1.f || Size.Y <= 1.f || SourceSize.X <= 1.f || SourceSize.Y <= 1.f)
			{
				return LayerId;
			}

			const float CapU = FMath::Clamp(SourceCapFraction, 0.02f, 0.45f);
			const float HeightScale = Size.Y / SourceSize.Y;
			const float SourceCapWidth = SourceSize.X * CapU;
			const float DestCapWidth = FMath::Clamp(SourceCapWidth * HeightScale, 1.f, Size.X * 0.42f);
			const float DestCenterWidth = FMath::Max(0.f, Size.X - (DestCapWidth * 2.f));

			auto DrawSlice = [&](const FVector2D& Pos, const FVector2D& SliceSize, float U0, float U1)
			{
				if (SliceSize.X <= 0.5f || SliceSize.Y <= 0.5f || U1 <= U0)
				{
					return;
				}

				FSlateBrush LocalBrush = *SourceBrush;
				LocalBrush.DrawAs = ESlateBrushDrawType::Image;
				LocalBrush.Tiling = ESlateBrushTileType::NoTile;
				LocalBrush.Margin = FMargin(0.f);
				LocalBrush.SetUVRegion(FBox2f(FVector2f(U0, 0.f), FVector2f(U1, 1.f)));

				FSlateDrawElement::MakeBox(
					OutDrawElements,
					LayerId,
					AllottedGeometry.ToPaintGeometry(
						FVector2f(SliceSize),
						FSlateLayoutTransform(FVector2f(Pos))),
					&LocalBrush,
					ESlateDrawEffect::None,
					InWidgetStyle.GetColorAndOpacityTint());
			};

			DrawSlice(FVector2D(0.f, 0.f), FVector2D(DestCapWidth, Size.Y), 0.f, CapU);
			DrawSlice(FVector2D(DestCapWidth, 0.f), FVector2D(DestCenterWidth, Size.Y), CapU, 1.f - CapU);
			DrawSlice(FVector2D(Size.X - DestCapWidth, 0.f), FVector2D(DestCapWidth, Size.Y), 1.f - CapU, 1.f);

			return LayerId + 1;
		}

	private:
		TAttribute<const FSlateBrush*> Brush;
		FVector2D DesiredSize = FVector2D(1.f, 1.f);
		float SourceCapFraction = 0.245f;
	};

	class ST66LeaderboardStatePlateButton : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(ST66LeaderboardStatePlateButton)
			: _NormalBrush(nullptr)
			, _HoverBrush(nullptr)
			, _PressedBrush(nullptr)
			, _SelectedBrush(nullptr)
			, _ButtonStyle(nullptr)
			, _ContentPadding(FMargin(0.f))
			, _SourceCapFraction(0.245f)
			, _IsSelected(false)
		{}
			SLATE_ARGUMENT(const FSlateBrush*, NormalBrush)
			SLATE_ARGUMENT(const FSlateBrush*, HoverBrush)
			SLATE_ARGUMENT(const FSlateBrush*, PressedBrush)
			SLATE_ARGUMENT(const FSlateBrush*, SelectedBrush)
			SLATE_ARGUMENT(const FButtonStyle*, ButtonStyle)
			SLATE_ARGUMENT(FMargin, ContentPadding)
			SLATE_ARGUMENT(float, SourceCapFraction)
			SLATE_ATTRIBUTE(bool, IsSelected)
			SLATE_EVENT(FOnClicked, OnClicked)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			NormalBrush = InArgs._NormalBrush;
			HoverBrush = InArgs._HoverBrush;
			PressedBrush = InArgs._PressedBrush;
			SelectedBrush = InArgs._SelectedBrush;
			ContentPadding = InArgs._ContentPadding;
			SourceCapFraction = InArgs._SourceCapFraction;
			IsSelected = InArgs._IsSelected;
			ButtonStyle = InArgs._ButtonStyle
				? *InArgs._ButtonStyle
				: FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
			ButtonStyle
				.SetNormalPadding(FMargin(0.f))
				.SetPressedPadding(FMargin(0.f));

			ChildSlot
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						InArgs._OnClicked,
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(ST66LeaderboardSlicedBrushImage)
							.Brush(this, &ST66LeaderboardStatePlateButton::GetCurrentBrush)
							.DesiredSize(FVector2D(1.f, 1.f))
							.SourceCapFraction(SourceCapFraction)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
							.Padding(ContentPadding)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.RenderTransform(this, &ST66LeaderboardStatePlateButton::GetContentTransform)
							.RenderTransformPivot(FVector2D(0.5f, 0.5f))
							[
								InArgs._Content.Widget
							]
						])
					.SetButtonStyle(&ButtonStyle)
					.SetPadding(FMargin(0.f)),
					&Button)
			];
		}

	private:
		const FSlateBrush* GetCurrentBrush() const
		{
			if (Button.IsValid() && Button->IsPressed() && PressedBrush)
			{
				return PressedBrush;
			}

			if (IsSelected.Get() && SelectedBrush)
			{
				return SelectedBrush;
			}

			if (Button.IsValid() && Button->IsHovered() && HoverBrush)
			{
				return HoverBrush;
			}

			return NormalBrush ? NormalBrush : FCoreStyle::Get().GetBrush("WhiteBrush");
		}

		TOptional<FSlateRenderTransform> GetContentTransform() const
		{
			if (Button.IsValid() && Button->IsPressed())
			{
				return FSlateRenderTransform(FVector2D(0.f, 1.f));
			}

			if (Button.IsValid() && Button->IsHovered())
			{
				return FSlateRenderTransform(FVector2D(0.f, -1.f));
			}

			return TOptional<FSlateRenderTransform>();
		}

		const FSlateBrush* NormalBrush = nullptr;
		const FSlateBrush* HoverBrush = nullptr;
		const FSlateBrush* PressedBrush = nullptr;
		const FSlateBrush* SelectedBrush = nullptr;
		FButtonStyle ButtonStyle;
		FMargin ContentPadding = FMargin(0.f);
		float SourceCapFraction = 0.245f;
		TAttribute<bool> IsSelected = false;
		TSharedPtr<SButton> Button;
	};

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
		if (Tokens.Num() == 0)
		{
			return true;
		}

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
			FName(TEXT("Hero_9")), FName(TEXT("Hero_10")), FName(TEXT("Hero_11")), FName(TEXT("Hero_12"))
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

	FString GetLeaderboardEntryDisplayName(const FLeaderboardEntry& Entry)
	{
		const FString TrimmedPlayerName = Entry.PlayerName.TrimStartAndEnd();
		if (!TrimmedPlayerName.IsEmpty() && !IsSyntheticLeaderboardName(TrimmedPlayerName))
		{
			return TrimmedPlayerName;
		}

		TArray<FString> DisplayNames = Entry.PlayerNames;
		for (FString& Name : DisplayNames)
		{
			Name = Name.TrimStartAndEnd();
		}
		DisplayNames.RemoveAll([](const FString& Name)
		{
			return Name.IsEmpty() || IsSyntheticLeaderboardName(Name);
		});

		if (DisplayNames.Num() > 0)
		{
			return FString::Join(DisplayNames, TEXT(" / "));
		}

		return NSLOCTEXT("T66.Leaderboard", "FavoriteFallbackName", "Saved Run").ToString();
	}

	TArray<FLeaderboardEntry> MakeWeeklySoloEasyDummyLeaderboard()
	{
		const TArray<FString> Names = {
			TEXT("CROWNED CHAD"),
			TEXT("PIXEL WIZARD"),
			TEXT("BOSS DELETE"),
			TEXT("RUN GOD"),
			TEXT("NO HIT NATE"),
			TEXT("CRIT QUEEN"),
			TEXT("LOOT LARRY"),
			TEXT("STAGE SKIP"),
			TEXT("MAGE MAIN"),
			TEXT("ARCADE ACE")
		};
		const int64 Scores[] = {
			184250, 171900, 165420, 158760, 151300,
			146880, 139440, 133910, 128650, 123200
		};

		TArray<FLeaderboardEntry> Entries;
		Entries.Reserve(11);
		for (int32 Index = 0; Index < Names.Num(); ++Index)
		{
			FLeaderboardEntry Entry;
			Entry.Rank = Index + 1;
			Entry.PlayerName = Names[Index];
			Entry.PlayerNames.Add(Entry.PlayerName);
			Entry.Score = Scores[Index];
			Entry.TimeSeconds = 360.f + static_cast<float>(Index * 17);
			Entry.PartySize = ET66PartySize::Solo;
			Entry.Difficulty = ET66Difficulty::Easy;
			Entry.StageReached = 4;
			Entry.HeroID = FName(*FString::Printf(TEXT("Hero_%d"), (Index % 12) + 1));
			Entry.EntryId = FString::Printf(TEXT("dummy-weekly-solo-easy-%02d"), Index + 1);
			Entries.Add(MoveTemp(Entry));
		}

		FLeaderboardEntry LocalEntry;
		LocalEntry.Rank = 42;
		LocalEntry.PlayerName = TEXT("DOPRA");
		LocalEntry.PlayerNames.Add(LocalEntry.PlayerName);
		LocalEntry.Score = 118700;
		LocalEntry.TimeSeconds = 598.f;
		LocalEntry.PartySize = ET66PartySize::Solo;
		LocalEntry.Difficulty = ET66Difficulty::Easy;
		LocalEntry.StageReached = 4;
		LocalEntry.HeroID = FName(TEXT("Hero_1"));
		LocalEntry.bIsLocalPlayer = true;
		LocalEntry.EntryId = TEXT("dummy-weekly-solo-easy-local");
		Entries.Add(MoveTemp(LocalEntry));
		return Entries;
	}

	void ApplyLeaderboardDisplayLimit(TArray<FLeaderboardEntry>& Entries)
	{
		TArray<FLeaderboardEntry> LimitedEntries;
		LimitedEntries.Reserve(LeaderboardVisibleEntryCount + 1);

		bool bHasLocalBelowTop = false;
		FLeaderboardEntry LocalBelowTopEntry;
		for (const FLeaderboardEntry& Entry : Entries)
		{
			if (Entry.bIsLocalPlayer && Entry.Rank > LeaderboardVisibleEntryCount)
			{
				if (!bHasLocalBelowTop)
				{
					LocalBelowTopEntry = Entry;
					bHasLocalBelowTop = true;
				}
				continue;
			}

			if (LimitedEntries.Num() < LeaderboardVisibleEntryCount)
			{
				LimitedEntries.Add(Entry);
			}
		}

		if (bHasLocalBelowTop)
		{
			LimitedEntries.Add(MoveTemp(LocalBelowTopEntry));
		}

		Entries = MoveTemp(LimitedEntries);
	}
}

ST66LeaderboardPanel::~ST66LeaderboardPanel()
{
	// Unbind raw delegates to prevent callbacks into destroyed widget
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
		Backend->OnStreamerRequestDataReady.RemoveAll(this);
	}
}

void ST66LeaderboardPanel::GetStageRangeForDifficulty(const ET66Difficulty Difficulty, int32& OutStartStage, int32& OutEndStage)
{
	switch (Difficulty)
	{
	case ET66Difficulty::Easy:
	default:
		OutStartStage = 1;
		OutEndStage = 4;
		return;
	case ET66Difficulty::Medium:
		OutStartStage = 1;
		OutEndStage = 4;
		return;
	case ET66Difficulty::Hard:
		OutStartStage = 1;
		OutEndStage = 4;
		return;
	case ET66Difficulty::VeryHard:
		OutStartStage = 1;
		OutEndStage = 4;
		return;
	case ET66Difficulty::Impossible:
		OutStartStage = 1;
		OutEndStage = 3;
		return;
	}
}

void ST66LeaderboardPanel::UpdateStageOptionsForDifficulty()
{
	int32 StartStage = 1;
	int32 EndStage = 4;
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
	bDailyChallengeMode = InArgs._DailyChallengeMode;
	bReferenceMirrorMode = InArgs._ReferenceMirrorMode;

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

	if (bDailyChallengeMode)
	{
		CurrentTimeFilter = ET66LeaderboardTime::Daily;
		CurrentFilter = ET66LeaderboardFilter::Global;
		CurrentPartySize = ET66PartySize::Solo;
		CurrentType = ET66LeaderboardType::Score;
	}

	// Speed run stage dropdown is limited to the selected difficulty's displayed stage-label range.
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
	auto MakeLockedRegularFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Regular"), Size);
		Font.LetterSpacing = LetterSpacing;
		return Font;
	};
	auto MakeLockedBoldFont = [](int32 Size, int32 LetterSpacing = 0) -> FSlateFontInfo
	{
		FSlateFontInfo Font = FT66Style::MakeFont(TEXT("Bold"), Size);
		Font.LetterSpacing = LetterSpacing;
		return Font;
	};
	const FSlateFontInfo LeaderboardTitleFont = MakeLockedRegularFont(LeaderboardTitleFontSize, 88);
	const FSlateFontInfo LeaderboardScopeTitleFont = MakeLockedBoldFont(16);
	const FSlateFontInfo LeaderboardBodyFont = MakeLockedRegularFont(LeaderboardBodyFontSize);
	const FSlateFontInfo LeaderboardDropdownFont = MakeLockedRegularFont(12);
	const FSlateFontInfo LeaderboardTimeToggleFont = MakeLockedBoldFont(LeaderboardTitleFontSize, 88);
	const FSlateFontInfo LeaderboardTypeGlyphFont = MakeLockedBoldFont(14);
	const FSlateFontInfo StreamerRequestBodyFont = MakeLockedRegularFont(12);
	const FSlateFontInfo StreamerRequestButtonFont = MakeLockedBoldFont(12);

	auto GetLeaderboardControlsVisibility = [this]() -> EVisibility
	{
		const bool bStandardVisible = !(bDailyChallengeMode || CurrentTimeFilter == ET66LeaderboardTime::Daily);
		return bStandardVisible && !bStreamerRequestOpen
			? EVisibility::Visible
			: EVisibility::Collapsed;
	};
	auto MakeStreamerHelpButton = [this, &NoBorderButtonStyle, StreamerRequestButtonFont]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(22.f)
			.HeightOverride(22.f)
			.Visibility_Lambda([this]()
			{
				const bool bStandardVisible = !(bDailyChallengeMode || CurrentTimeFilter == ET66LeaderboardTime::Daily);
				return bStandardVisible && CurrentFilter == ET66LeaderboardFilter::Streamers
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						FOnClicked::CreateSP(this, &ST66LeaderboardPanel::HandleStreamerRequestClicked),
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.10f, 0.13f, 0.17f, 0.96f))
						.Padding(FMargin(0.f))
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("?")))
							.Font(StreamerRequestButtonFont)
							.ColorAndOpacity(FLinearColor::White)
							.Justification(ETextJustify::Center)
						])
					.SetButtonStyle(&NoBorderButtonStyle)
					.SetPadding(FMargin(0.f)))
			];
	};
	auto MakeStreamerRequestPanel = [this, &NoBorderButtonStyle, StreamerRequestBodyFont, StreamerRequestButtonFont]() -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.08f, 0.075f, 0.065f, 0.96f))
			.Padding(FMargin(12.f, 10.f))
			.Visibility_Lambda([this]()
			{
				return CurrentFilter == ET66LeaderboardFilter::Streamers && bStreamerRequestOpen
					? EVisibility::Visible
					: EVisibility::Collapsed;
			})
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.Leaderboard", "StreamerRequestCopy", "If you are a streamer and want to be included put your twitch/kick/tiktok link and your steam ID"))
					.Font(StreamerRequestBodyFont)
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SAssignNew(StreamerLinkTextBox, SEditableTextBox)
					.HintText(NSLOCTEXT("T66.Leaderboard", "StreamerLinkHint", "Twitch / Kick / TikTok link"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 10.f)
				[
					SAssignNew(StreamerSteamIdTextBox, SEditableTextBox)
					.HintText(NSLOCTEXT("T66.Leaderboard", "StreamerSteamIdHint", "Steam ID"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 10.f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return StreamerRequestStatus; })
					.Font(StreamerRequestBodyFont)
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.AutoWrapText(true)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						FT66Style::MakeBareButton(
							FT66BareButtonParams(
								FOnClicked::CreateSP(this, &ST66LeaderboardPanel::HandleStreamerRequestSubmitClicked),
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.18f, 0.31f, 0.18f, 0.97f))
								.Padding(FMargin(10.f, 7.f))
								.HAlign(HAlign_Center)
								[
									SNew(STextBlock)
									.Text_Lambda([this]()
									{
										return bStreamerRequestSubmitting
											? NSLOCTEXT("T66.Leaderboard", "StreamerRequestSubmitting", "Submitting")
											: NSLOCTEXT("T66.Leaderboard", "StreamerRequestSubmit", "Submit");
									})
									.Font(StreamerRequestButtonFont)
									.ColorAndOpacity(FLinearColor::White)
								])
							.SetButtonStyle(&NoBorderButtonStyle)
							.SetPadding(FMargin(0.f)))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(8.f, 0.f, 0.f, 0.f)
					[
						FT66Style::MakeBareButton(
							FT66BareButtonParams(
								FOnClicked::CreateSP(this, &ST66LeaderboardPanel::HandleStreamerRequestCloseClicked),
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(FLinearColor(0.26f, 0.29f, 0.34f, 0.96f))
								.Padding(FMargin(10.f, 7.f))
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66.Common", "Close", "Close"))
									.Font(StreamerRequestButtonFont)
									.ColorAndOpacity(FLinearColor::White)
								])
							.SetButtonStyle(&NoBorderButtonStyle)
							.SetPadding(FMargin(0.f)))
					]
				]
			];
	};

	if (bReferenceMirrorMode)
	{
		const FMargin WideTabMargin(0.f);
		const FMargin DropdownFieldMargin(0.157f, 0.292f, 0.157f, 0.292f);
		const FMargin AvatarSlotMargin(0.f);
		constexpr TextureFilter ButtonTextureFilter = TextureFilter::TF_Nearest;
		const FSlateBrush* ReferenceWideTabSelectedBrush = ResolveReferenceGeneratedBrush(
			ReferenceTabWeeklyActiveBrush,
			ReferenceSharedSourceDir,
			TEXT("Buttons/Pill/selected.png"),
			WideTabMargin,
			TEXT("LBMasterWeekly"),
			ButtonTextureFilter);
		const FSlateBrush* ReferenceWideTabNormalBrush = ResolveReferenceGeneratedBrush(
			ReferenceTabAllTimeInactiveBrush,
			ReferenceSharedSourceDir,
			TEXT("Buttons/Pill/normal.png"),
			WideTabMargin,
			TEXT("LBMasterAllTime"),
			ButtonTextureFilter);
		const FSlateBrush* ReferenceWideTabHoverBrush = ResolveReferenceGeneratedBrush(
			ReferenceTabHoverBrush,
			ReferenceSharedSourceDir,
			TEXT("Buttons/Pill/hover.png"),
			WideTabMargin,
			TEXT("LBMasterTabHover"),
			ButtonTextureFilter);
		const FSlateBrush* ReferenceWideTabPressedBrush = ResolveReferenceGeneratedBrush(
			ReferenceTabPressedBrush,
			ReferenceSharedSourceDir,
			TEXT("Buttons/Pill/pressed.png"),
			WideTabMargin,
			TEXT("LBMasterTabPressed"),
			ButtonTextureFilter);
		const FSlateBrush* ReferenceLeftDropdownBrush = ResolveReferenceGeneratedBrush(
			ReferenceDropdownLeftBrush,
			ReferenceSharedSourceDir,
			TEXT("Controls/reference_dropdown_field_normal.png"),
			DropdownFieldMargin,
			TEXT("LBMasterDropdownLeft"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* ReferenceRightDropdownBrush = ResolveReferenceGeneratedBrush(
			ReferenceDropdownRightBrush,
			ReferenceSharedSourceDir,
			TEXT("Controls/reference_dropdown_field_normal.png"),
			DropdownFieldMargin,
			TEXT("LBMasterDropdownRight"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* ReferenceChevronBrush = ResolveMasterLibrarySliceBrush(
			ReferenceDropdownChevronBrush,
			TEXT("Icons/dropdown_chevron.png"),
			FMargin(0.f),
			TEXT("LBMasterDropdownChevron"),
			TextureFilter::TF_Nearest);
		const FSlateBrush* ReferenceToggleNormalBrush = ResolveMasterLibrarySliceBrush(
			ReferenceToggleSpeedRunUnselectedBrush,
			TEXT("Controls/radio_circle_normal.png"),
			FMargin(0.f),
			TEXT("LBMasterToggleSpeedRun"));
		ResolveReferenceGeneratedBrush(
			ReferenceAvatarFrameBrush,
			ReferenceSharedSourceDir,
			TEXT("Slots/reference_square_slot_frame_normal.png"),
			AvatarSlotMargin,
			TEXT("LBMasterAvatarFrame"),
			TextureFilter::TF_Nearest);
		if (ReferenceAvatarFrameBrush.Brush.IsValid())
		{
			ReferenceAvatarFrameBrush.Brush->DrawAs = ESlateBrushDrawType::Image;
			ReferenceAvatarFrameBrush.Brush->Tiling = ESlateBrushTileType::NoTile;
			ReferenceAvatarFrameBrush.Brush->Margin = FMargin(0.f);
		}
		const FSlateBrush* ReferenceAvatarFallback01 = ResolveReferenceRightPanelBrush(
			ReferenceAvatarFallbackBrush01,
			TEXT("leaderboard_avatar_fallback_01.png"),
			FMargin(0.f),
			TEXT("LBReferenceAvatarFallback01"));
		const FSlateBrush* ReferenceAvatarFallback02 = ResolveReferenceRightPanelBrush(
			ReferenceAvatarFallbackBrush02,
			TEXT("leaderboard_avatar_fallback_02.png"),
			FMargin(0.f),
			TEXT("LBReferenceAvatarFallback02"));
		const FSlateBrush* ReferenceAvatarFallback03 = ResolveReferenceRightPanelBrush(
			ReferenceAvatarFallbackBrush03,
			TEXT("leaderboard_avatar_fallback_03.png"),
			FMargin(0.f),
			TEXT("LBReferenceAvatarFallback03"));

		auto GetDailyModeVisibility = [this]() -> EVisibility
		{
			return bDailyChallengeMode ? EVisibility::Visible : EVisibility::Collapsed;
		};
		auto GetStandardModeVisibility = [this]() -> EVisibility
		{
			return (bDailyChallengeMode || CurrentTimeFilter == ET66LeaderboardTime::Daily) ? EVisibility::Collapsed : EVisibility::Visible;
		};

		const FSlateFontInfo ReferenceHeaderFont = MakeLockedBoldFont(18, 0);
		const FSlateFontInfo ReferenceScopeTitleFont = MakeLockedBoldFont(30, 0);
		const FSlateFontInfo ReferenceDropdownFont = MakeLockedRegularFont(18);
		const FSlateFontInfo ReferenceDailyFont = MakeLockedBoldFont(21, 0);
		const FSlateFontInfo ReferenceToggleFont = MakeLockedBoldFont(20);

		auto MakeReferenceStatePlateButton = [this, &NoBorderButtonStyle, ReferenceHeaderFont](
			const FSlateBrush* SelectedBrush,
			const FSlateBrush* NormalBrush,
			const FSlateBrush* HoverBrush,
			const FSlateBrush* PressedBrush,
			const FVector2D& Size,
			const FText& Label,
			TFunction<bool()> IsSelected,
			FReply (ST66LeaderboardPanel::*ClickHandler)()) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.WidthOverride(Size.X)
				.HeightOverride(Size.Y)
				[
					SNew(ST66LeaderboardStatePlateButton)
					.ButtonStyle(&NoBorderButtonStyle)
					.NormalBrush(NormalBrush)
					.HoverBrush(HoverBrush)
					.PressedBrush(PressedBrush)
					.SelectedBrush(SelectedBrush)
					.SourceCapFraction(0.105f)
					.IsSelected(TAttribute<bool>::CreateLambda([IsSelected]() { return IsSelected(); }))
					.OnClicked(FOnClicked::CreateSP(this, ClickHandler))
					.ContentPadding(FMargin(12.f, 0.f))
					[
						SNew(STextBlock)
						.Text(Label)
						.Font(ReferenceHeaderFont)
						.ColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([IsSelected]() -> FSlateColor
						{
							return IsSelected()
								? FSlateColor(ReferenceWoodControlText)
								: FSlateColor(ReferenceWoodControlMuted);
						}))
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				];
		};

		auto MakeReferenceTypeButton = [this, &NoBorderButtonStyle, ReferenceToggleFont, ReferenceToggleNormalBrush](
			const ET66LeaderboardType Type,
			const FVector2D& Size) -> TSharedRef<SWidget>
		{
			const FSlateBrush* RadioBrush = ReferenceToggleNormalBrush ? ReferenceToggleNormalBrush : FCoreStyle::Get().GetBrush("WhiteBrush");
			return SNew(SBox)
				.WidthOverride(Size.X)
				.HeightOverride(Size.Y)
				[
					FT66Style::MakeBareButton(
						FT66BareButtonParams(
							FOnClicked::CreateLambda([this, Type]()
							{
								SetLeaderboardType(Type);
								return FReply::Handled();
							}),
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(28.f)
								.HeightOverride(28.f)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									[
										SNew(SImage)
										.Image(RadioBrush)
										.ColorAndOpacity(ReferenceToggleNormalBrush ? FLinearColor::White : ReferenceWoodControlMuted)
									]
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(FText::FromString(TEXT("\u25CF")))
										.Font(FT66Style::MakeFont(TEXT("Bold"), 15))
										.ColorAndOpacity(ReferenceWoodControlText)
										.Visibility_Lambda([this, Type]()
										{
											return CurrentType == Type ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
										})
										.Justification(ETextJustify::Center)
									]
								]
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(FMargin(8.f, 0.f, 0.f, 0.f))
							[
								SNew(STextBlock)
								.Text(GetTypeText(Type))
								.Font(ReferenceToggleFont)
								.ColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([this, Type]() -> FSlateColor
								{
									return CurrentType == Type
										? FSlateColor(ReferenceWoodControlText)
										: FSlateColor(ReferenceWoodControlMuted);
								}))
								.Justification(ETextJustify::Center)
								.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								.Clipping(EWidgetClipping::ClipToBounds)
							])
						.SetButtonStyle(&NoBorderButtonStyle)
						.SetPadding(FMargin(0.f))
						.SetHAlign(HAlign_Left)
						.SetVAlign(VAlign_Center))
				];
		};

		auto MakeReferenceDropdown = [this, ReferenceDropdownFont, ReferenceChevronBrush](
			const FSlateBrush* ShellBrush,
			const FVector2D& Size,
			const TAttribute<FText>& Label,
			TFunction<TSharedRef<SWidget>()> OnGetMenuContent) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.WidthOverride(Size.X)
				.HeightOverride(Size.Y)
				[
					SNew(SComboButton)
					.ComboButtonStyle(LeaderboardDropdownStyle.Get())
					.IsFocusable(false)
					.HasDownArrow(false)
					.ContentPadding(FMargin(0.f))
					.OnGetMenuContent_Lambda([OnGetMenuContent]() { return OnGetMenuContent(); })
					.ButtonContent()
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(ST66LeaderboardSlicedBrushImage)
							.Brush(TAttribute<const FSlateBrush*>::CreateLambda([ShellBrush]() -> const FSlateBrush*
							{
								return ShellBrush ? ShellBrush : FCoreStyle::Get().GetBrush("WhiteBrush");
							}))
							.DesiredSize(Size)
							.SourceCapFraction(0.085f)
						]
						+ SOverlay::Slot()
						.Padding(FMargin(18.f, 9.f, 52.f, 9.f))
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(Label)
							.Font(ReferenceDropdownFont)
							.ColorAndOpacity(ReferenceWoodControlText)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Right)
						.VAlign(VAlign_Center)
						.Padding(FMargin(0.f, 0.f, 22.f, 0.f))
						[
							SNew(SBox)
							.WidthOverride(18.f)
							.HeightOverride(18.f)
							[
								SNew(SImage)
								.Image(ReferenceChevronBrush)
								.ColorAndOpacity(ReferenceChevronBrush ? ReferenceWoodControlText : ReferenceLeaderboardText)
							]
						]
					]
				];
		};

		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.Padding(FMargin(0.f, 8.f, 0.f, 0.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.f, 0.f, 0.f, 0.f))
				[
					SNew(SBox)
					.Visibility_Lambda(GetDailyModeVisibility)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Leaderboard", "DailyLeaderboard", "DAILY LEADERBOARD"))
						.Font(ReferenceDailyFont)
						.ColorAndOpacity(ReferenceLeaderboardText)
						.Justification(ETextJustify::Center)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(FMargin(0.f, 0.f, 0.f, 4.f))
				[
					SNew(SBox)
					.Visibility_Lambda(GetStandardModeVisibility)
					.HeightOverride(44.f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text_Lambda([this]()
							{
								FString Title = GetLeaderboardScopeTitleText().ToString();
								Title.ToUpperInline();
								return FText::FromString(Title);
							})
							.Font(ReferenceScopeTitleFont)
							.ColorAndOpacity(ReferenceLeaderboardTitleText)
							.ShadowOffset(FVector2D(1.f, 1.f))
							.ShadowColorAndOpacity(ReferenceLeaderboardTitleShadow)
							.Justification(ETextJustify::Center)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(8.f, 0.f, 0.f, 0.f)
						[
							MakeStreamerHelpButton()
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.f, 0.f, 0.f, 0.f))
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda(GetLeaderboardControlsVisibility)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeReferenceStatePlateButton(
							ReferenceWideTabSelectedBrush,
							ReferenceWideTabNormalBrush,
							ReferenceWideTabHoverBrush,
							ReferenceWideTabPressedBrush,
							ReferenceWeeklyTabSize,
							NSLOCTEXT("T66.Leaderboard", "Weekly", "WEEKLY"),
							[this]() { return CurrentTimeFilter == ET66LeaderboardTime::Current; },
							&ST66LeaderboardPanel::HandleCurrentClicked)
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
					[
						MakeReferenceStatePlateButton(
							ReferenceWideTabSelectedBrush,
							ReferenceWideTabNormalBrush,
							ReferenceWideTabHoverBrush,
							ReferenceWideTabPressedBrush,
							ReferenceAllTimeTabSize,
							NSLOCTEXT("T66.Leaderboard", "AllTime", "ALL TIME"),
							[this]() { return CurrentTimeFilter == ET66LeaderboardTime::AllTime; },
							&ST66LeaderboardPanel::HandleAllTimeClicked)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.f, 8.f, 0.f, 8.f))
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda(GetLeaderboardControlsVisibility)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakeReferenceDropdown(
							ReferenceLeftDropdownBrush,
							ReferenceLeftDropdownSize,
							TAttribute<FText>::CreateLambda([this]() { return GetPartySizeText(CurrentPartySize); }),
							[this]()
							{
								TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
								for (const TSharedPtr<FString>& Opt : PartySizeOptions)
								{
									if (!Opt.IsValid())
									{
										continue;
									}

									const TSharedPtr<FString> Captured = Opt;
									Box->AddSlot().AutoHeight()
									[
										FT66Style::MakeDropdownOptionButton(FText::FromString(*Opt), FOnClicked::CreateLambda([this, Captured]()
										{
											OnPartySizeChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}), GetPartySizeText(CurrentPartySize).ToString().Equals(*Opt), 0.f, 34.f, 14)
									];
								}
								return Box;
							})
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(8.f, 0.f, 0.f, 0.f)
					[
						MakeReferenceDropdown(
							ReferenceRightDropdownBrush,
							ReferenceRightDropdownSize,
							TAttribute<FText>::CreateLambda([this]() { return GetDifficultyText(CurrentDifficulty); }),
							[this]()
							{
								TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
								for (const TSharedPtr<FString>& Opt : DifficultyOptions)
								{
									if (!Opt.IsValid())
									{
										continue;
									}

									const TSharedPtr<FString> Captured = Opt;
									Box->AddSlot().AutoHeight()
									[
										FT66Style::MakeDropdownOptionButton(FText::FromString(*Opt), FOnClicked::CreateLambda([this, Captured]()
										{
											OnDifficultyChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}), GetDifficultyText(CurrentDifficulty).ToString().Equals(*Opt), 0.f, 34.f, 14)
									];
								}
								return Box;
							})
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.f, 0.f, 0.f, 0.f))
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda(GetLeaderboardControlsVisibility)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					[
						SNew(SBox)
						.WidthOverride(ReferenceLeftDropdownSize.X)
						.HAlign(HAlign_Left)
						[
							MakeReferenceTypeButton(ET66LeaderboardType::Score, ReferenceScoreToggleSize)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Left)
					.Padding(8.f, 0.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(ReferenceRightDropdownSize.X)
						.HAlign(HAlign_Left)
						[
							MakeReferenceTypeButton(ET66LeaderboardType::SpeedRun, ReferenceSpeedRunToggleSize)
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(6.f, 10.f, 0.f, 0.f))
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda(GetLeaderboardControlsVisibility)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(73.f)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Leaderboard", "Rank", "RANK"))
							.Font(ReferenceHeaderFont)
							.ColorAndOpacity(ReferenceLeaderboardText)
							.Justification(ETextJustify::Left)
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.Leaderboard", "Name", "NAME"))
						.Font(ReferenceHeaderFont)
						.ColorAndOpacity(ReferenceLeaderboardText)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(84.f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]()
							{
								return CurrentTimeFilter == ET66LeaderboardTime::Daily || CurrentType == ET66LeaderboardType::Score
									? NSLOCTEXT("T66.Leaderboard", "Score", "SCORE")
									: NSLOCTEXT("T66.Leaderboard", "Time", "TIME");
							})
							.Font(ReferenceHeaderFont)
							.ColorAndOpacity(ReferenceLeaderboardText)
							.Justification(ETextJustify::Right)
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(4.f, 8.f, 0.f, 0.f))
				[
					SNew(SBorder)
					.Visibility_Lambda(GetLeaderboardControlsVisibility)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.44f, 0.27f, 0.12f, 0.36f))
					.Padding(FMargin(0.f, 1.f))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				.Padding(FMargin(0.f, 10.f, 0.f, 0.f))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(EntryListBox, SVerticalBox)
						.Visibility_Lambda(GetLeaderboardControlsVisibility)
					]
					+ SOverlay::Slot()
					[
						MakeStreamerRequestPanel()
					]
				]
			]
		];

		RebuildEntryList();
		return;
	}

	auto MakeIconButton = [this, SquareIconSize, LeaderboardTitleFont](ET66LeaderboardFilter Filter, FReply (ST66LeaderboardPanel::*ClickHandler)(), const FString& FallbackLetter, TSharedPtr<FSlateBrush> IconBrush) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(SquareIconSize)
			.HeightOverride(SquareIconSize)
			[
				FT66Style::MakeBareButton(
					FT66BareButtonParams(
						FOnClicked::CreateSP(this, ClickHandler),
						IconBrush.IsValid()
						? StaticCastSharedRef<SWidget>(
							SNew(SImage)
							.Image(IconBrush.Get())
							.ColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([this, Filter]() -> FSlateColor
							{
								return CurrentFilter == Filter
									? FSlateColor(FLinearColor::White)
									: FSlateColor(FLinearColor(0.66f, 0.66f, 0.68f, 0.78f));
							}))
						)
						: StaticCastSharedRef<SWidget>(
							SNew(STextBlock)
							.Text(FText::FromString(FallbackLetter))
							.Font(LeaderboardTitleFont)
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
						))
					.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.SetPadding(FMargin(0.f))
					.SetHAlign(HAlign_Fill)
					.SetVAlign(VAlign_Fill))
			];
	};

	auto MakeFilterButtonShell = [this, &MakeIconButton](ET66LeaderboardFilter Filter, FReply (ST66LeaderboardPanel::*ClickHandler)(), const FString& FallbackLetter, TSharedPtr<FSlateBrush> IconBrush) -> TSharedRef<SWidget>
	{
		return MakeIconButton(Filter, ClickHandler, FallbackLetter, IconBrush);
	};

	auto MakeFlatTimeButton = [this, &NoBorderButtonStyle, LeaderboardTimeToggleFont](const FText& Text, const FLinearColor& FillColor, FReply (ST66LeaderboardPanel::*ClickHandler)()) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeBareButton(
			FT66BareButtonParams(
				FOnClicked::CreateSP(this, ClickHandler),
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
			)
			.SetButtonStyle(&NoBorderButtonStyle)
			.SetPadding(FMargin(0.f)));
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

	auto MakeTypeButton = [this, &NoBorderButtonStyle, LeaderboardDropdownFont, LeaderboardTypeGlyphFont](ET66LeaderboardType Type) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeBareButton(
			FT66BareButtonParams(
				FOnClicked::CreateLambda([this, Type]()
				{
					SetLeaderboardType(Type);
					return FReply::Handled();
				}),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this, Type]()
					{
						return FText::FromString(CurrentType == Type ? TEXT("●") : TEXT("○"));
					})
					.Font(LeaderboardTypeGlyphFont)
					.ColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([this, Type]() -> FSlateColor
					{
						return CurrentType == Type ? FSlateColor(FT66Style::ButtonPrimary()) : FSlateColor(FT66Style::Tokens::TextMuted);
					}))
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(GetTypeText(Type))
					.Font(LeaderboardDropdownFont)
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			)
			.SetButtonStyle(&NoBorderButtonStyle)
			.SetPadding(FMargin(8.f, 6.f, 10.f, 5.f)));
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
	auto GetDailyModeVisibility = [this]() -> EVisibility
	{
		return bDailyChallengeMode ? EVisibility::Visible : EVisibility::Collapsed;
	};
	auto GetStandardModeVisibility = [this]() -> EVisibility
	{
		return (bDailyChallengeMode || CurrentTimeFilter == ET66LeaderboardTime::Daily) ? EVisibility::Collapsed : EVisibility::Visible;
	};

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SHorizontalBox)
			.Visibility_Lambda(GetStandardModeVisibility)
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
					// Time toggles (Weekly | All Time)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(SBox)
						.Visibility_Lambda(GetStandardModeVisibility)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
							.Text_Lambda([this]()
							{
								FString Title = GetLeaderboardScopeTitleText().ToString();
								Title.ToUpperInline();
								return FText::FromString(Title);
							})
							.Font(LeaderboardScopeTitleFont)
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(8.f, 0.f, 0.f, 0.f)
							[
								MakeStreamerHelpButton()
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 10.0f)
					[
						SNew(SBox)
						.Visibility_Lambda(GetDailyModeVisibility)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.Leaderboard", "DailyLeaderboard", "DAILY LEADERBOARD"))
							.Font(LeaderboardTitleFont)
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 10.0f)
					[
						SNew(SBox)
						.Visibility_Lambda(GetLeaderboardControlsVisibility)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(3.0f, 0.0f)
							[ MakeTimeButton(WeeklyText, ET66LeaderboardTime::Current, &ST66LeaderboardPanel::HandleCurrentClicked) ]
							+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(3.0f, 0.0f)
							[ MakeTimeButton(AllTimeText, ET66LeaderboardTime::AllTime, &ST66LeaderboardPanel::HandleAllTimeClicked) ]
						]
					]
					// Dropdowns row (Party Size | Difficulty)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.Padding(0.0f, 0.0f, 0.0f, 6.0f)
					[
						SNew(SHorizontalBox)
						.Visibility_Lambda(GetLeaderboardControlsVisibility)
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
												FT66Style::MakeDropdownOptionButton(FText::FromString(*Opt), FOnClicked::CreateLambda([this, Captured]()
												{
													OnPartySizeChanged(Captured, ESelectInfo::Direct);
													FSlateApplication::Get().DismissAllMenus();
													return FReply::Handled();
												}), GetPartySizeText(CurrentPartySize).ToString().Equals(*Opt), 0.f, 34.f, 14)
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
												FT66Style::MakeDropdownOptionButton(FText::FromString(*Opt), FOnClicked::CreateLambda([this, Captured]()
												{
													OnDifficultyChanged(Captured, ESelectInfo::Direct);
													FSlateApplication::Get().DismissAllMenus();
													return FReply::Handled();
												}), GetDifficultyText(CurrentDifficulty).ToString().Equals(*Opt), 0.f, 34.f, 14)
											];
									}
									return Box;
								},
								DifficultyDropdownMinWidth)
						]
					]
					// Type selector row
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.Padding(0.0f, 0.0f, 0.0f, 18.0f)
					[
						SNew(SHorizontalBox)
						.Visibility_Lambda(GetLeaderboardControlsVisibility)
						+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.0f, 0.0f, 3.0f, 0.0f)
						[
							MakeTypeButton(ET66LeaderboardType::Score)
						]
						+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(3.0f, 0.0f, 0.0f, 0.0f)
						[
							MakeTypeButton(ET66LeaderboardType::SpeedRun)
						]
					]
					// Column headers
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 4.0f)
					[
						SNew(SBorder)
						.Visibility_Lambda(GetLeaderboardControlsVisibility)
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
										return CurrentTimeFilter == ET66LeaderboardTime::Daily || CurrentType == ET66LeaderboardType::Score
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
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SAssignNew(EntryListBox, SVerticalBox)
							.Visibility_Lambda(GetLeaderboardControlsVisibility)
						]
						+ SOverlay::Slot()
						[
							MakeStreamerRequestPanel()
						]
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

void ST66LeaderboardPanel::RebuildEntryList()
{
	if (!EntryListBox.IsValid()) return;

	EntryListBox->ClearChildren();

	if (LeaderboardEntries.Num() == 0)
	{
		EntryListBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, bReferenceMirrorMode ? 2.f : 10.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.Leaderboard", "NoEntriesFallback", "No leaderboard data yet"))
			.Font(FT66Style::Tokens::FontRegular(bReferenceMirrorMode ? 13 : LeaderboardBodyFontSize))
			.ColorAndOpacity(bReferenceMirrorMode ? ReferenceLeaderboardMuted : FT66Style::Tokens::Text)
		];
		return;
	}

	if (bReferenceMirrorMode)
	{
		const FSlateBrush* AvatarFrameBrush = ReferenceAvatarFrameBrush.Brush.IsValid() ? ReferenceAvatarFrameBrush.Brush.Get() : nullptr;
		const FSlateFontInfo ReferenceRankFont = FT66Style::MakeFont(TEXT("Bold"), 20);
		const FSlateFontInfo ReferenceNameFont = FT66Style::MakeFont(TEXT("Regular"), 20);
		const FSlateFontInfo ReferenceScoreFont = FT66Style::MakeFont(TEXT("Regular"), 19);
		const bool bStretchReferenceRows = LeaderboardEntries.Num() >= LeaderboardVisibleEntryCount;

		for (int32 EntryIndex = 0; EntryIndex < LeaderboardEntries.Num(); ++EntryIndex)
		{
			const FLeaderboardEntry& Entry = LeaderboardEntries[EntryIndex];
			const TSharedRef<bool> bIsRowHovered = MakeShared<bool>(false);
			const FLinearColor BaseRowColor = Entry.bIsLocalPlayer
				? FLinearColor(0.52f, 0.31f, 0.12f, 0.20f)
				: FLinearColor::Transparent;
			const FLinearColor HoverRowColor = Entry.bIsLocalPlayer
				? FLinearColor(0.58f, 0.35f, 0.14f, 0.26f)
				: FLinearColor(0.44f, 0.26f, 0.10f, 0.18f);

			const FString RankString = FString::Printf(TEXT("#%d"), FMath::Max(1, Entry.Rank));
			const FString MetricString = (CurrentTimeFilter == ET66LeaderboardTime::Daily || CurrentType == ET66LeaderboardType::Score)
				? FString::Printf(TEXT("%lld"), Entry.Score)
				: FormatTime(Entry.TimeSeconds);

			auto SetHovered = [bIsRowHovered](const bool bHovered)
			{
				*bIsRowHovered = bHovered;
			};

			const TSharedRef<SWidget> AvatarWidget =
				SNew(SBox)
				.WidthOverride(ReferenceAvatarFrameSize.X)
				.HeightOverride(ReferenceAvatarFrameSize.Y)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SNew(SImage)
						.Image(AvatarFrameBrush)
						.ColorAndOpacity(AvatarFrameBrush ? FLinearColor::White : FLinearColor::Transparent)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(ReferenceAvatarInsetSize.X)
						.HeightOverride(ReferenceAvatarInsetSize.Y)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.19f, 0.11f, 0.06f, 1.0f))
							.Padding(FMargin(0.f))
							[
								SNew(SImage)
								.Image_Lambda([this, Entry]() -> const FSlateBrush*
								{
									const FSlateBrush* PortraitBrush = GetPortraitBrushForEntry(Entry);
									if (PortraitBrush == DefaultAvatarBrush.Get())
									{
										return nullptr;
									}
									if (PortraitBrush)
									{
										const_cast<FSlateBrush*>(PortraitBrush)->ImageSize = ReferenceAvatarInsetSize;
									}
									return PortraitBrush;
								})
							]
						]
					]
				];

			const TSharedRef<SWidget> RowContents =
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(31.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(RankString))
						.Font(ReferenceRankFont)
						.ColorAndOpacity(ReferenceLeaderboardRowText)
					]
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(3.f, 0.f, 0.f, 0.f)
				[
					AvatarWidget
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text_Lambda([this, Entry]()
					{
						return FText::FromString(ResolveEntryDisplayName(Entry));
					})
					.Font(ReferenceNameFont)
					.ColorAndOpacity(ReferenceLeaderboardRowText)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(SBox)
					.WidthOverride(76.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(MetricString))
						.Font(ReferenceScoreFont)
						.ColorAndOpacity(ReferenceLeaderboardRowText)
						.Justification(ETextJustify::Right)
					]
				];

			const FMargin EntryPadding(
				0.f,
				Entry.bIsLocalPlayer && Entry.Rank > LeaderboardVisibleEntryCount ? 4.f : 0.f,
				0.f,
				EntryIndex + 1 < LeaderboardEntries.Num() ? 1.f : 0.f);
			const TSharedRef<SWidget> EntryRowWidget =
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
					.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([bIsRowHovered, BaseRowColor, HoverRowColor]() -> FSlateColor
					{
						return FSlateColor(*bIsRowHovered ? HoverRowColor : BaseRowColor);
					}))
					.Padding(FMargin(5.f, 4.f, 5.f, 4.f))
					[
						FT66Style::MakeBareButton(
							FT66BareButtonParams(
								FOnClicked::CreateLambda([this, Entry]() { return HandleEntryClicked(Entry); }),
								RowContents)
							.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
							.SetPadding(FMargin(0.f))
							.SetOnHovered(FSimpleDelegate::CreateLambda([SetHovered]() { SetHovered(true); }))
							.SetOnUnhovered(FSimpleDelegate::CreateLambda([SetHovered]() { SetHovered(false); })))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.f, 1.f, 0.f, 0.f))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.44f, 0.27f, 0.12f, 0.36f))
					.Padding(FMargin(0.f, 1.f))
				]
			;

			if (bStretchReferenceRows)
			{
				EntryListBox->AddSlot()
				.FillHeight(1.f)
				.Padding(EntryPadding)
				[
					EntryRowWidget
				];
			}
			else
			{
				EntryListBox->AddSlot()
				.AutoHeight()
				.Padding(EntryPadding)
				[
					EntryRowWidget
				];
			}
		}

		return;
	}

	for (const FLeaderboardEntry& Entry : LeaderboardEntries)
	{
		const FLinearColor LeaderboardNameText(0.86f, 0.88f, 0.92f, 1.0f);
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

		const float NameFontSize = static_cast<float>(LeaderboardBodyFontSize);
		const float MemberPortraitSize = FMath::RoundToFloat(NameFontSize + 6.0f);
		const float PartyStripMinHeight = FMath::Max(MemberPortraitSize, NameFontSize) - 1.0f;
		const float RowVerticalPadding = FMath::Max(2.0f, FMath::RoundToFloat(NameFontSize * 0.18f));
		const float RankLabelWidth = FMath::Max(24.0f, RankColumnWidth);
		TSharedRef<SHorizontalBox> NameColumn = SNew(SHorizontalBox);
		for (int32 NameIndex = 0; NameIndex < DisplayNames.Num(); ++NameIndex)
		{
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
						.Image_Lambda([this, Entry, NameIndex]() -> const FSlateBrush*
						{
							const FSlateBrush* MemberPortrait = GetPortraitBrushForEntryMember(Entry, NameIndex);
							return MemberPortrait == DefaultAvatarBrush.Get() ? nullptr : MemberPortrait;
						})
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this, Entry, NameIndex]()
					{
						return FText::FromString(ResolveEntryMemberDisplayName(Entry, NameIndex));
					})
					.Font(FT66Style::Tokens::FontRegular(NameFontSize))
					.ColorAndOpacity(LeaderboardNameText)
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

		const bool bCanOpenSummary = Entry.bIsLocalPlayer || (Entry.bHasRunSummary && !Entry.EntryId.IsEmpty());
		auto SetHovered = [bIsRowHovered](bool bHovered)
		{
			*bIsRowHovered = bHovered;
		};

		const TSharedRef<SWidget> SummaryContents =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(RankLabelWidth)
				[
					SNew(STextBlock)
					.Text(FText::Format(NSLOCTEXT("T66.Leaderboard", "RankFormat", "#{0}"), FText::AsNumber(Entry.Rank)))
					.Font(FT66Style::Tokens::FontRegular(LeaderboardBodyFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				PartyStripScroller
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(ScoreColumnWidth)
				[
					SNew(STextBlock)
					.Text(FText::FromString((CurrentTimeFilter == ET66LeaderboardTime::Daily || CurrentType == ET66LeaderboardType::Score) ? ScoreStr : TimeStr))
					.Font(FT66Style::Tokens::FontRegular(LeaderboardBodyFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Right)
				]
			];

		const TSharedRef<SWidget> SummaryButton =
			FT66Style::MakeBareButton(
				FT66BareButtonParams(
					FOnClicked::CreateLambda([this, Entry]() { return HandleEntryClicked(Entry); }),
					SNew(ST66LeaderboardRowWheelProxy)
					.ScrollBox(PartyStripScrollBox)
					[
						SummaryContents
					])
				.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
				.SetPadding(FMargin(0.f))
				.SetHAlign(HAlign_Fill)
				.SetVAlign(VAlign_Fill)
				.SetEnabled(bCanOpenSummary)
				.SetOnHovered(FSimpleDelegate::CreateLambda([SetHovered]() { SetHovered(true); }))
				.SetOnUnhovered(FSimpleDelegate::CreateLambda([SetHovered]() { SetHovered(false); })));

		const TSharedRef<SWidget> RowWidget =
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.BorderBackgroundColor(TAttribute<FSlateColor>::CreateLambda([bIsRowHovered, BaseRowColor, HoverRowColor]() -> FSlateColor
			{
				return FSlateColor(*bIsRowHovered ? HoverRowColor : BaseRowColor);
			}))
			.Padding(FMargin(7.0f, RowVerticalPadding))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SummaryButton
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

UT66PlayerSettingsSubsystem* ST66LeaderboardPanel::GetPlayerSettings() const
{
	UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
}

bool ST66LeaderboardPanel::IsEntryFavoritable(const FLeaderboardEntry& Entry) const
{
	return !Entry.EntryId.IsEmpty();
}

bool ST66LeaderboardPanel::IsEntryFavorited(const FLeaderboardEntry& Entry) const
{
	if (!IsEntryFavoritable(Entry))
	{
		return false;
	}

	const UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettings();
	return PlayerSettings && PlayerSettings->IsFavoriteLeaderboardRun(Entry.EntryId);
}

FT66FavoriteLeaderboardRun ST66LeaderboardPanel::MakeFavoriteRunFromEntry(const FLeaderboardEntry& Entry) const
{
	FT66FavoriteLeaderboardRun Favorite;
	Favorite.EntryId = Entry.EntryId;
	Favorite.LeaderboardType = CurrentType;
	Favorite.Filter = CurrentFilter;
	Favorite.TimeScope = GMirrorWeeklyToAllTime ? ET66LeaderboardTime::AllTime : CurrentTimeFilter;
	Favorite.Difficulty = CurrentDifficulty;
	Favorite.PartySize = CurrentPartySize;
	Favorite.Rank = Entry.Rank;
	Favorite.DisplayName = ResolveEntryDisplayName(Entry);
	Favorite.Score = Entry.Score;
	Favorite.TimeSeconds = Entry.TimeSeconds;
	Favorite.bHasRunSummary = Entry.bHasRunSummary;
	return Favorite;
}

FReply ST66LeaderboardPanel::HandleFavoriteClicked(FLeaderboardEntry Entry)
{
	UT66PlayerSettingsSubsystem* PlayerSettings = GetPlayerSettings();
	if (!PlayerSettings || !IsEntryFavoritable(Entry))
	{
		return FReply::Handled();
	}

	const bool bShouldFavorite = !PlayerSettings->IsFavoriteLeaderboardRun(Entry.EntryId);
	PlayerSettings->SetFavoriteLeaderboardRun(MakeFavoriteRunFromEntry(Entry), bShouldFavorite);
	RebuildEntryList();
	return FReply::Handled();
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
	if (bDailyChallengeMode || CurrentTimeFilter == ET66LeaderboardTime::Daily)
	{
		CurrentFilter = ET66LeaderboardFilter::Global;
	}
	else
	{
		CurrentFilter = NewFilter;
	}
	if (CurrentFilter != ET66LeaderboardFilter::Streamers)
	{
		bStreamerRequestOpen = false;
		bStreamerRequestSubmitting = false;
		StreamerRequestStatus = FText::GetEmpty();
	}
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::SetTimeFilter(ET66LeaderboardTime NewTime)
{
	if (bDailyChallengeMode)
	{
		CurrentTimeFilter = ET66LeaderboardTime::Daily;
		CurrentFilter = ET66LeaderboardFilter::Global;
		CurrentPartySize = ET66PartySize::Solo;
		CurrentType = ET66LeaderboardType::Score;
		RefreshLeaderboard();
		return;
	}

	if (CurrentTimeFilter == NewTime)
	{
		return;
	}

	CurrentTimeFilter = NewTime;
	if (CurrentTimeFilter == ET66LeaderboardTime::Daily)
	{
		CurrentFilter = ET66LeaderboardFilter::Global;
		CurrentPartySize = ET66PartySize::Solo;
		CurrentType = ET66LeaderboardType::Score;
	}

	if (GMirrorWeeklyToAllTime)
	{
		Invalidate(EInvalidateWidgetReason::Layout);
		return;
	}

	Invalidate(EInvalidateWidgetReason::Layout);
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
	if (bDailyChallengeMode || CurrentTimeFilter == ET66LeaderboardTime::Daily)
	{
		CurrentType = ET66LeaderboardType::Score;
		RefreshLeaderboard();
		return;
	}

	CurrentType = NewType;
	RefreshLeaderboard();
}

void ST66LeaderboardPanel::RefreshLeaderboard()
{
	if (CurrentTimeFilter == ET66LeaderboardTime::Daily)
	{
		const FString CacheKey = TEXT("daily_global");

		UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
		UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
		if (Backend && Backend->HasCachedLeaderboard(CacheKey))
		{
			LeaderboardEntries = Backend->GetCachedLeaderboard(CacheKey);
		}
		else
		{
			LeaderboardEntries.Reset();
		}

		ApplyLeaderboardDisplayLimit(LeaderboardEntries);
		for (int32 EntryIndex = 0; EntryIndex < LeaderboardEntries.Num(); ++EntryIndex)
		{
			NormalizeEntryIdentity(LeaderboardEntries[EntryIndex], EntryIndex);
		}

		RebuildEntryList();

		if (Backend && Backend->IsBackendConfigured() && !Backend->HasCachedLeaderboard(CacheKey))
		{
			if (!bBoundToBackendDelegate && Backend)
			{
				Backend->OnLeaderboardDataReady.AddSP(SharedThis(this), &ST66LeaderboardPanel::OnBackendLeaderboardReady);
				bBoundToBackendDelegate = true;
			}

			Backend->FetchDailyLeaderboard(TEXT("global"));
		}

		return;
	}

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
	if (Backend && Backend->HasCachedLeaderboard(CacheKey))
	{
		const TArray<FLeaderboardEntry> CachedEntries = Backend->GetCachedLeaderboard(CacheKey);

		if (CachedEntries.Num() > 0)
		{
			// Use backend data when the backend actually has something for this key.
			LeaderboardEntries = CachedEntries;
		}
		else
		{
			LeaderboardEntries.Reset();
		}
	}
	else if (Backend)
	{
		LeaderboardEntries.Reset();
	}
	else
	{
		LeaderboardEntries.Reset();
	}

	const bool bUseWeeklySoloEasyDummyData =
		CurrentTimeFilter == ET66LeaderboardTime::Current
		&& CurrentPartySize == ET66PartySize::Solo
		&& CurrentDifficulty == ET66Difficulty::Easy
		&& CurrentFilter == ET66LeaderboardFilter::Global
		&& CurrentType == ET66LeaderboardType::Score;
	if (bUseWeeklySoloEasyDummyData && LeaderboardEntries.Num() == 0)
	{
		LeaderboardEntries = MakeWeeklySoloEasyDummyLeaderboard();
	}

	ApplyLeaderboardDisplayLimit(LeaderboardEntries);

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
			Backend->OnLeaderboardDataReady.AddSP(SharedThis(this), &ST66LeaderboardPanel::OnBackendLeaderboardReady);
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

FReply ST66LeaderboardPanel::HandleStreamerRequestClicked()
{
	if (CurrentFilter != ET66LeaderboardFilter::Streamers)
	{
		return FReply::Handled();
	}

	bStreamerRequestOpen = true;
	StreamerRequestStatus = FText::GetEmpty();
	Invalidate(EInvalidateWidgetReason::Layout);
	return FReply::Handled();
}

FReply ST66LeaderboardPanel::HandleStreamerRequestCloseClicked()
{
	bStreamerRequestOpen = false;
	bStreamerRequestSubmitting = false;
	StreamerRequestStatus = FText::GetEmpty();
	Invalidate(EInvalidateWidgetReason::Layout);
	return FReply::Handled();
}

FReply ST66LeaderboardPanel::HandleStreamerRequestSubmitClicked()
{
	if (bStreamerRequestSubmitting)
	{
		return FReply::Handled();
	}

	const FString CreatorLink = StreamerLinkTextBox.IsValid()
		? StreamerLinkTextBox->GetText().ToString().TrimStartAndEnd()
		: FString();
	const FString SteamId = StreamerSteamIdTextBox.IsValid()
		? StreamerSteamIdTextBox->GetText().ToString().TrimStartAndEnd()
		: FString();

	if (CreatorLink.IsEmpty() || SteamId.IsEmpty())
	{
		StreamerRequestStatus = NSLOCTEXT("T66.Leaderboard", "StreamerRequestMissingFields", "Creator link and Steam ID are required.");
		return FReply::Handled();
	}

	UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
	UT66BackendSubsystem* Backend = GI ? GI->GetSubsystem<UT66BackendSubsystem>() : nullptr;
	if (!Backend || !Backend->IsBackendConfigured())
	{
		StreamerRequestStatus = NSLOCTEXT("T66.Leaderboard", "StreamerRequestUnavailable", "Streamer requests unavailable.");
		return FReply::Handled();
	}

	Backend->OnStreamerRequestDataReady.RemoveAll(this);
	Backend->OnStreamerRequestDataReady.AddSP(SharedThis(this), &ST66LeaderboardPanel::OnStreamerRequestComplete);
	bStreamerRequestSubmitting = true;
	StreamerRequestStatus = NSLOCTEXT("T66.Leaderboard", "StreamerRequestSending", "Submitting...");
	Backend->SubmitStreamerRequest(CreatorLink, SteamId);
	return FReply::Handled();
}

void ST66LeaderboardPanel::OnStreamerRequestComplete(bool bSuccess, const FString& Message)
{
	bStreamerRequestSubmitting = false;
	StreamerRequestStatus = FText::FromString(Message);
	if (bSuccess)
	{
		if (StreamerLinkTextBox.IsValid())
		{
			StreamerLinkTextBox->SetText(FText::GetEmpty());
		}
		if (StreamerSteamIdTextBox.IsValid())
		{
			StreamerSteamIdTextBox->SetText(FText::GetEmpty());
		}
	}
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

FReply ST66LeaderboardPanel::HandleDailyClicked()
{
	SetTimeFilter(ET66LeaderboardTime::Daily);
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
	int32 EndStage = 4;
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
	case ET66LeaderboardTime::Daily: return NSLOCTEXT("T66.Leaderboard", "Daily", "DAILY");
	default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
	}
}

FText ST66LeaderboardPanel::GetTypeText(ET66LeaderboardType Type) const
{
	switch (Type)
	{
	case ET66LeaderboardType::Score: return NSLOCTEXT("T66.Leaderboard", "HighScore", "High Score");
	case ET66LeaderboardType::SpeedRun: return NSLOCTEXT("T66.Leaderboard", "SpeedRun", "Speed Run");
	default: return NSLOCTEXT("T66.Common", "Unknown", "UNKNOWN");
	}
}

FText ST66LeaderboardPanel::GetLeaderboardScopeTitleText() const
{
	switch (CurrentFilter)
	{
	case ET66LeaderboardFilter::Friends:
		return NSLOCTEXT("T66.Leaderboard", "FriendsLeaderboard", "Friends Leaderboard");
	case ET66LeaderboardFilter::Streamers:
		return NSLOCTEXT("T66.Leaderboard", "StreamerLeaderboard", "Streamer Leaderboard");
	case ET66LeaderboardFilter::Global:
	default:
		return NSLOCTEXT("T66.Leaderboard", "GlobalLeaderboard", "Global Leaderboard");
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

		if (Backend && Backend->HasCachedRunSummary(Entry.EntryId))
		{
			UT66LeaderboardRunSummarySaveGame* Snap = Backend->GetCachedRunSummary(Entry.EntryId);
			if (Snap)
			{
				LeaderboardSubsystem->SetPendingFakeRunSummarySnapshot(Snap);
				UIManager->ShowModal(ET66ScreenType::RunSummary);
				return FReply::Handled();
			}
		}

		if (Backend && Backend->IsBackendConfigured())
		{
			// Not cached yet — fire async fetch. When it arrives we'll open via the delegate.
			PendingRunSummaryEntryId = Entry.EntryId;
			if (!bBoundToRunSummaryDelegate)
			{
				Backend->OnRunSummaryReady.AddSP(SharedThis(this), &ST66LeaderboardPanel::OnBackendRunSummaryReady);
				bBoundToRunSummaryDelegate = true;
			}
			Backend->FetchRunSummary(Entry.EntryId);
			return FReply::Handled();
		}
	}
	return FReply::Handled();
}

void ST66LeaderboardPanel::NormalizeEntryIdentity(FLeaderboardEntry& Entry, int32 EntryIndex)
{
	UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;

	const int32 PartyMemberCount = GetPartyMemberCount(Entry.PartySize);
	const int32 FallbackStartIndex = FMath::Max(0, EntryIndex) * 4;
	if (Entry.PlayerSteamIds.Num() == 0 && !Entry.SteamId.IsEmpty())
	{
		Entry.PlayerSteamIds.Add(Entry.SteamId);
	}

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

	if (CleanNames.Num() == 0)
	{
		const FString LocalFallbackName = Entry.bIsLocalPlayer
			? (!Entry.PlayerName.TrimStartAndEnd().IsEmpty() ? Entry.PlayerName.TrimStartAndEnd() : TEXT("YOU"))
			: FString();

		if (!LocalFallbackName.IsEmpty() && !IsSyntheticLeaderboardName(LocalFallbackName))
		{
			CleanNames.Add(LocalFallbackName);
		}
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
			Entry.HeroID = GetFallbackHeroId(T66GI, FallbackStartIndex);
		}
	}
}

const FSlateBrush* ST66LeaderboardPanel::GetPortraitBrushForEntry(const FLeaderboardEntry& Entry)
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

UT66SteamHelper* ST66LeaderboardPanel::GetSteamHelper() const
{
	UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UT66SteamHelper>() : nullptr;
}

FString ST66LeaderboardPanel::ResolveSteamDisplayName(const FString& SteamId) const
{
	const FString TrimmedSteamId = SteamId.TrimStartAndEnd();
	UT66SteamHelper* SteamHelper = GetSteamHelper();
	if (!SteamHelper || TrimmedSteamId.IsEmpty())
	{
		return FString();
	}

	if (TrimmedSteamId.Equals(SteamHelper->GetLocalSteamId(), ESearchCase::CaseSensitive))
	{
		const FString LocalDisplayName = SteamHelper->GetLocalDisplayName().TrimStartAndEnd();
		if (!LocalDisplayName.IsEmpty())
		{
			return LocalDisplayName;
		}
	}

	for (const FT66SteamFriendInfo& FriendInfo : SteamHelper->GetFriendInfos())
	{
		if (TrimmedSteamId.Equals(FriendInfo.SteamId, ESearchCase::CaseSensitive))
		{
			return FriendInfo.DisplayName.TrimStartAndEnd();
		}
	}

	if (UGameInstance* GI = LeaderboardSubsystem ? LeaderboardSubsystem->GetGameInstance() : nullptr)
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			if (TrimmedSteamId.Equals(SteamHelper->GetLocalSteamId(), ESearchCase::CaseSensitive))
			{
				const FString PartyLocalName = PartySubsystem->GetLocalDisplayName().TrimStartAndEnd();
				if (!PartyLocalName.IsEmpty() && !PartyLocalName.Equals(TEXT("You"), ESearchCase::IgnoreCase))
				{
					return PartyLocalName;
				}
			}

			const FT66PartyFriendEntry* Friend = PartySubsystem->GetFriends().FindByPredicate(
				[&TrimmedSteamId](const FT66PartyFriendEntry& Candidate)
				{
					return Candidate.PlayerId.Equals(TrimmedSteamId, ESearchCase::CaseSensitive);
				});
			if (Friend)
			{
				return Friend->DisplayName.TrimStartAndEnd();
			}
		}
	}

	return FString();
}

FString ST66LeaderboardPanel::ResolveEntryMemberDisplayName(const FLeaderboardEntry& Entry, int32 MemberIndex) const
{
	const FString MemberSteamId = Entry.PlayerSteamIds.IsValidIndex(MemberIndex)
		? Entry.PlayerSteamIds[MemberIndex]
		: (MemberIndex == 0 ? Entry.SteamId : FString());
	const FString SteamDisplayName = ResolveSteamDisplayName(MemberSteamId).TrimStartAndEnd();
	if (!SteamDisplayName.IsEmpty())
	{
		return SteamDisplayName;
	}

	TArray<FString> Candidates;
	Candidates.Reserve(4);
	if (Entry.PlayerNames.IsValidIndex(MemberIndex))
	{
		Candidates.Add(Entry.PlayerNames[MemberIndex]);
	}
	if (MemberIndex == 0)
	{
		Candidates.Add(Entry.PlayerName);
	}

	for (FString Candidate : Candidates)
	{
		Candidate = Candidate.TrimStartAndEnd();
		if (!Candidate.IsEmpty() && !IsSyntheticLeaderboardName(Candidate))
		{
			return Candidate;
		}
	}

	return MemberIndex == 0
		? FString(TEXT("Steam Player"))
		: FString::Printf(TEXT("Steam Player %d"), MemberIndex + 1);
}

FString ST66LeaderboardPanel::ResolveEntryDisplayName(const FLeaderboardEntry& Entry) const
{
	const int32 MemberCount = FMath::Max(1, Entry.PlayerNames.Num());
	if (MemberCount == 1)
	{
		return ResolveEntryMemberDisplayName(Entry, 0);
	}

	TArray<FString> MemberNames;
	MemberNames.Reserve(MemberCount);
	for (int32 MemberIndex = 0; MemberIndex < MemberCount; ++MemberIndex)
	{
		MemberNames.Add(ResolveEntryMemberDisplayName(Entry, MemberIndex));
	}
	return FString::Join(MemberNames, TEXT(" / "));
}

const FSlateBrush* ST66LeaderboardPanel::GetPortraitBrushForEntryMember(const FLeaderboardEntry& Entry, int32 MemberIndex)
{
	const FString MemberSteamId = Entry.PlayerSteamIds.IsValidIndex(MemberIndex)
		? Entry.PlayerSteamIds[MemberIndex]
		: (MemberIndex == 0 ? Entry.SteamId : FString());
	if (!MemberSteamId.IsEmpty())
	{
		const FSlateBrush* SteamAvatarBrush = GetOrCreateSteamAvatarBrush(MemberSteamId);
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

	return DefaultAvatarBrush.Get();
}

const FSlateBrush* ST66LeaderboardPanel::GetOrCreateSteamAvatarBrush(const FString& SteamId)
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
	Brush->ImageSize = FVector2D(RowPortraitSize, RowPortraitSize);
	Brush->SetResourceObject(AvatarTexture);
	SteamAvatarBrushes.Add(TrimmedSteamId, Brush);
	return Brush.Get();
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

	const TSoftObjectPtr<UTexture2D> PortraitSoft = T66GI->ResolveHeroPortrait(HeroID, ET66BodyType::Chad, ET66HeroPortraitVariant::Low);
	TSoftObjectPtr<UTexture2D> ReferencePortraitSoft;
	if (bReferenceMirrorMode)
	{
		const FString HeroIdString = HeroID.ToString();
		ReferencePortraitSoft = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(FString::Printf(
			TEXT("/Game/UI/Sprites/Heroes/%s/T_%s_Chad_Low.T_%s_Chad_Low"),
			*HeroIdString,
			*HeroIdString,
			*HeroIdString)));
	}
	const TSoftObjectPtr<UTexture2D>& EffectivePortraitSoft = (!ReferencePortraitSoft.IsNull())
		? ReferencePortraitSoft
		: PortraitSoft;
	if (EffectivePortraitSoft.IsNull())
	{
		return DefaultAvatarBrush.Get();
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->DrawAs = ESlateBrushDrawType::Image;
	Brush->Tiling = ESlateBrushTileType::NoTile;
	Brush->ImageSize = FVector2D(RowPortraitSize, RowPortraitSize);

	if (TexPool)
	{
		UTexture2D* LoadedPortrait = T66SlateTexture::GetLoaded(TexPool, EffectivePortraitSoft);
		if (!LoadedPortrait && bReferenceMirrorMode)
		{
			TexPool->EnsureTexturesLoadedSync({ EffectivePortraitSoft.ToSoftObjectPath() });
			LoadedPortrait = T66SlateTexture::GetLoaded(TexPool, EffectivePortraitSoft);
		}

		Brush->SetResourceObject(LoadedPortrait);
		UObject* Requester = UIManager ? static_cast<UObject*>(UIManager) : static_cast<UObject*>(GI);
		if (!LoadedPortrait && Requester)
		{
			T66SlateTexture::BindSharedBrushAsync(
				TexPool,
				EffectivePortraitSoft,
				Requester,
				Brush,
				FName(*FString::Printf(TEXT("LBHero_%s"), *HeroID.ToString())),
				/*bClearWhileLoading*/ false);
		}
	}

	HeroPortraitBrushes.Add(HeroID, Brush);
	return Brush.Get();
}
