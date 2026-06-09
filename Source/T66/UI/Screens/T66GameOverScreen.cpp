// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66GameOverScreen.h"

#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66PlayerSettingsSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66FlatStyle.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	enum class ET66GameOverRankPyramidType : uint8
	{
		HighScore,
		SpeedRun,
	};

	struct FT66GameOverRankPyramidData
	{
		ET66GameOverRankPyramidType Type = ET66GameOverRankPyramidType::HighScore;
		int32 AllTimeRank = 0;
		int32 WeeklyRank = 0;
		bool bSpeedRunFullClearRequired = false;
	};

	int32 T66GameOverRankTierIndex(const int32 Rank)
	{
		if (Rank <= 0)
		{
			return 4;
		}
		if (Rank <= 1)
		{
			return 0;
		}
		if (Rank <= 10)
		{
			return 1;
		}
		if (Rank <= 100)
		{
			return 2;
		}
		if (Rank <= 1000)
		{
			return 3;
		}
		return 4;
	}

	FText T66GameOverRankText(const int32 Rank)
	{
		return Rank > 0
			? FText::Format(NSLOCTEXT("T66.GameOver", "RankNumberFormat", "#{0}"), FText::AsNumber(Rank))
			: NSLOCTEXT("T66.GameOver", "RankUnranked", "UNRANKED");
	}
}

UT66GameOverScreen::UT66GameOverScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::GameOver;
	bIsModal = true;
}

void UT66GameOverScreen::GatherRunRewards(int32& OutCoupons, int32& OutAchievements, int32& OutSecretAchievements) const
{
	OutCoupons = 0;
	OutAchievements = 0;
	OutSecretAchievements = 0;

	const UGameInstance* GI = GetGameInstance();
	const UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	if (RunState)
	{
		OutCoupons = FMath::Max(0, RunState->GetPowerCrystalsEarnedThisRun());
	}

	const UT66AchievementsSubsystem* Achievements = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	if (!Achievements)
	{
		return;
	}

	const TArray<FName> CurrentRunAchievements = Achievements->GetCurrentRunUnlockedAchievementIDs();
	const TArray<FAchievementData> AllAchievements = Achievements->GetAllAchievements();
	for (const FName AchievementID : CurrentRunAchievements)
	{
		const FAchievementData* Definition = AllAchievements.FindByPredicate(
			[AchievementID](const FAchievementData& Achievement)
			{
				return Achievement.AchievementID == AchievementID;
			});

		if (Definition && Definition->Category == ET66AchievementCategory::Special)
		{
			++OutSecretAchievements;
		}
		else
		{
			++OutAchievements;
		}
	}
}

TSharedRef<SWidget> UT66GameOverScreen::BuildSlateUI()
{
	constexpr float CanvasW = 1920.f;
	constexpr float CanvasH = 1080.f;
	const FLinearColor Purple = FT66FlatStyle::PurpleAccent();
	const FLinearColor Red = FT66FlatStyle::SelectedText();
	const FLinearColor White = FT66FlatStyle::PrimaryText();

	UGameInstance* GI = GetGameInstance();
	UT66PlayerSettingsSubsystem* PlayerSettings = GI ? GI->GetSubsystem<UT66PlayerSettingsSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* Leaderboard = GI ? GI->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;
	UT66GameInstance* T66GI = GI ? Cast<UT66GameInstance>(GI) : nullptr;
	const ET66Difficulty Difficulty = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const ET66PartySize PartySize = T66GI ? T66GI->SelectedPartySize : ET66PartySize::Solo;
	const bool bHighScoreMode = PlayerSettings ? PlayerSettings->GetHighScoreMode() : true;
	const bool bSpeedRunMode = PlayerSettings ? PlayerSettings->GetSpeedRunMode() : true;
	const bool bRunFullCleared = RunState && RunState->DidRunEndInVictory();

	TArray<FT66GameOverRankPyramidData> Pyramids;
	if (bHighScoreMode)
	{
		FT66GameOverRankPyramidData Pyramid;
		Pyramid.Type = ET66GameOverRankPyramidType::HighScore;
		Pyramid.AllTimeRank = Leaderboard ? Leaderboard->GetLocalScoreRankAllTime(Difficulty, PartySize) : 0;
		Pyramid.WeeklyRank = Leaderboard ? Leaderboard->GetLocalScoreRankWeekly(Difficulty, PartySize) : 0;
		Pyramids.Add(Pyramid);
	}
	if (bSpeedRunMode)
	{
		FT66GameOverRankPyramidData Pyramid;
		Pyramid.Type = ET66GameOverRankPyramidType::SpeedRun;
		Pyramid.AllTimeRank = Leaderboard ? Leaderboard->GetLocalSpeedRunRankAllTime(Difficulty, PartySize) : 0;
		Pyramid.WeeklyRank = Leaderboard ? Leaderboard->GetLocalSpeedRunRankWeekly(Difficulty, PartySize) : 0;
		Pyramid.bSpeedRunFullClearRequired = !bRunFullCleared && Pyramid.AllTimeRank <= 0 && Pyramid.WeeklyRank <= 0;
		Pyramids.Add(Pyramid);
	}

	TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddN = [&Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
		.Anchors(FAnchors(0.f, 0.f))
		.Alignment(FVector2D(0.f, 0.f))
		.Offset(FMargin(X * CanvasW, Y * CanvasH, W * CanvasW, H * CanvasH))
		[
			Widget
		];
	};
	auto Label = [](const FName Tag, const FText& Text, const int32 FontSize, const FLinearColor& Color, const bool bBold = true, const ETextJustify::Type Justify = ETextJustify::Center) -> TSharedRef<SWidget>
	{
		TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
			.Text(Text)
			.Font(bBold ? FT66FlatStyle::MakeBoldFont(FontSize) : FT66FlatStyle::MakeFont(FontSize))
			.ColorAndOpacity(Color)
			.Justification(Justify)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Clipping(EWidgetClipping::ClipToBounds)
			.Visibility(EVisibility::HitTestInvisible);
		return FT66FlatStyle::AttachMetadata(TextBlock, Tag, TEXT("Label"), ET66FlatState::Default, TOptional<FLinearColor>(), false, NAME_None, true);
	};
	auto Panel = [](const FName Tag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatPanel(ET66FlatState::Default, FMargin(0.f), SNullWidget::NullWidget, nullptr, Tag);
	};
	auto PanelState = [](const FName Tag, const ET66FlatState State) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatPanel(State, FMargin(0.f), SNullWidget::NullWidget, nullptr, Tag);
	};
	auto ButtonShell = [](FOnClicked OnClicked, const FName Tag) -> TSharedRef<SWidget>
	{
		return FT66FlatStyle::MakeFlatToggleGroupButton(
			ET66FlatState::Selected,
			SNullWidget::NullWidget,
			MoveTemp(OnClicked),
			FMargin(0.f),
			0.f,
			0.f,
			true,
			Tag);
	};
	auto TierLabel = [](const int32 TierIndex) -> FText
	{
		switch (TierIndex)
		{
		case 0: return NSLOCTEXT("T66.GameOver", "RankTierLegend", "#1 LEGEND");
		case 1: return NSLOCTEXT("T66.GameOver", "RankTierTop10", "TOP 10");
		case 2: return NSLOCTEXT("T66.GameOver", "RankTierTop100", "TOP 100");
		case 3: return NSLOCTEXT("T66.GameOver", "RankTierTop1000", "TOP 1000");
		default: return NSLOCTEXT("T66.GameOver", "RankTierRanked", "RANKED");
		}
	};
	auto AddRankPyramid = [&](const FT66GameOverRankPyramidData& Data, const float X, const float Y, const float W, const float H, const FName RootTag)
	{
		const bool bSpeedRun = Data.Type == ET66GameOverRankPyramidType::SpeedRun;
		const FText TitleText = bSpeedRun
			? NSLOCTEXT("T66.GameOver", "SpeedRunPyramidTitle", "SPEED RUN")
			: NSLOCTEXT("T66.GameOver", "HighScorePyramidTitle", "HIGH SCORE");
		const int32 ActiveTier = T66GameOverRankTierIndex(Data.AllTimeRank);
		AddN(X, Y, W, H, Panel(RootTag));
		AddN(X + W * 0.08f, Y + H * 0.045f, W * 0.84f, H * 0.055f, Label(FName(*(RootTag.ToString() + TEXT(".Title"))), TitleText, 28, White, true));

		const float WidthFactors[5] = { 0.34f, 0.48f, 0.62f, 0.76f, 0.90f };
		const float BandHeight = H * 0.105f;
		const float BandGap = H * 0.018f;
		const float FirstBandY = Y + H * 0.145f;
		for (int32 Tier = 0; Tier < 5; ++Tier)
		{
			const float BandW = W * WidthFactors[Tier];
			const float BandX = X + (W - BandW) * 0.5f;
			const float BandY = FirstBandY + Tier * (BandHeight + BandGap);
			const bool bActive = Tier == ActiveTier;
			const FString TierTag = FString::Printf(TEXT("%s.Tier%d"), *RootTag.ToString(), Tier + 1);
			AddN(BandX, BandY, BandW, BandHeight, PanelState(FName(*TierTag), bActive ? ET66FlatState::Selected : ET66FlatState::Default));
			AddN(
				BandX + BandW * 0.08f,
				BandY + BandHeight * 0.24f,
				BandW * 0.60f,
				BandHeight * 0.42f,
				Label(FName(*(TierTag + TEXT(".Label"))), TierLabel(Tier), 19, bActive ? Red : Purple, true, ETextJustify::Left));
			if (bActive)
			{
				AddN(
					BandX + BandW * 0.70f,
					BandY + BandHeight * 0.24f,
					BandW * 0.22f,
					BandHeight * 0.42f,
					Label(FName(*(TierTag + TEXT(".You"))), NSLOCTEXT("T66.GameOver", "PyramidYouMarker", "YOU"), 19, Red, true, ETextJustify::Right));
			}
		}

		const FText RankText = Data.bSpeedRunFullClearRequired
			? NSLOCTEXT("T66.GameOver", "SpeedRunFullClearRequired", "FULL CLEAR REQUIRED")
			: T66GameOverRankText(Data.AllTimeRank);
		AddN(X + W * 0.08f, Y + H * 0.780f, W * 0.84f, H * 0.060f, Label(FName(*(RootTag.ToString() + TEXT(".AllTimeRank"))), FText::Format(NSLOCTEXT("T66.GameOver", "AllTimeRankFormat", "ALL TIME {0}"), RankText), 24, White, true));
		AddN(X + W * 0.08f, Y + H * 0.850f, W * 0.84f, H * 0.050f, Label(FName(*(RootTag.ToString() + TEXT(".WeeklyRank"))), FText::Format(NSLOCTEXT("T66.GameOver", "WeeklyRankFormat", "WEEKLY {0}"), T66GameOverRankText(Data.WeeklyRank)), 20, Purple, true));
	};

	AddN(0.f, 0.f, 1.f, 1.f,
		FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FT66FlatStyle::BackgroundColor()),
			FName(TEXT("GameOver.Background")),
			TEXT("Background"),
			ET66FlatState::Default));

	if (Pyramids.Num() <= 0)
	{
		AddN(0.200f, 0.315f, 0.600f, 0.160f, Label(FName(TEXT("GameOver.Title")), NSLOCTEXT("T66.GameOver", "Title", "GAME OVER"), 92, Red, true));
		AddN(0.330f, 0.545f, 0.340f, 0.095f, ButtonShell(FOnClicked::CreateUObject(this, &UT66GameOverScreen::HandleContinueClicked), FName(TEXT("GameOver.ContinueButton"))));
		AddN(0.365f, 0.568f, 0.270f, 0.044f, Label(FName(TEXT("GameOver.ContinueButton.Label")), NSLOCTEXT("T66.GameOver", "GoToRunSummary", "GO TO RUN SUMMARY SCREEN"), 28, Red, true));
	}
	else
	{
		AddN(0.200f, 0.045f, 0.600f, 0.110f, Label(FName(TEXT("GameOver.Title")), NSLOCTEXT("T66.GameOver", "Title", "GAME OVER"), 72, Red, true));
		if (Pyramids.Num() == 1)
		{
			AddRankPyramid(Pyramids[0], 0.310f, 0.190f, 0.380f, 0.560f, FName(TEXT("GameOver.RankPyramid1")));
		}
		else
		{
			AddRankPyramid(Pyramids[0], 0.105f, 0.190f, 0.380f, 0.560f, FName(TEXT("GameOver.RankPyramidHighScore")));
			AddRankPyramid(Pyramids[1], 0.515f, 0.190f, 0.380f, 0.560f, FName(TEXT("GameOver.RankPyramidSpeedRun")));
		}

		AddN(0.330f, 0.825f, 0.340f, 0.095f, ButtonShell(FOnClicked::CreateUObject(this, &UT66GameOverScreen::HandleContinueClicked), FName(TEXT("GameOver.ContinueButton"))));
		AddN(0.365f, 0.848f, 0.270f, 0.044f, Label(FName(TEXT("GameOver.ContinueButton.Label")), NSLOCTEXT("T66.GameOver", "GoToRunSummary", "GO TO RUN SUMMARY SCREEN"), 28, Red, true));
	}

	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			[
				SNew(SBox)
				.WidthOverride(CanvasW)
				.HeightOverride(CanvasH)
				[
					Canvas
				]
			]
		];

	return FT66FlatStyle::MakeResponsiveRoot(Root);
}

FReply UT66GameOverScreen::HandleContinueClicked()
{
	if (UIManager)
	{
		UIManager->CloseModal();
		UIManager->ShowModal(ET66ScreenType::RunSummary);
	}
	return FReply::Handled();
}
