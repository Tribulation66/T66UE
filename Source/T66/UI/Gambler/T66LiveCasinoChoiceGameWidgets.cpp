// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Gambler/T66GuessCupGameWidget.h"
#include "UI/Gambler/T66StickPickGameWidget.h"
#include "UI/Gambler/T66FindJokerGameWidget.h"

#include "Core/Animation/T66AnimationCurves.h"
#include "UI/Gambler/T66GamblerGameStage.h"
#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	static FText BuildLiveCasinoWagerText(const int32 WagerAmount)
	{
		return WagerAmount > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(WagerAmount))
			: FText::GetEmpty();
	}

	static TSharedRef<SWidget> MakeLiveCasinoHeader(
		const FText& Title,
		TSharedPtr<STextBlock>& StatusText,
		TSharedPtr<STextBlock>& WagerText,
		const FOnClicked& BackClicked)
	{
		const FTextBlockStyle& TextTitle = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Title"));
		const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
				[
					FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
						NSLOCTEXT("T66.Gambler", "Back", "BACK"),
						BackClicked,
						ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetPadding(FMargin(12.f, 8.f)))
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(14.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Title)
					.TextStyle(&TextTitle)
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SAssignNew(StatusText, STextBlock)
					.Text(FText::GetEmpty())
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SAssignNew(WagerText, STextBlock)
					.Text(FText::GetEmpty())
					.TextStyle(&TextBody)
					.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				]
			];
	}

	/** Invisible, hit-testable zone used for hover + click on stage sprites. */
	static TSharedRef<SBorder> MakeStageHitZone(const float Width, const float Height, const FPointerEventHandler& OnMouseDown)
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("NoBrush")))
			.Padding(0.f)
			.OnMouseButtonDown(OnMouseDown)
			[
				SNew(SBox).WidthOverride(Width).HeightOverride(Height)
			];
	}

	static FT66AnimationTimeline MakeStageTimeline(
		const TCHAR* Name,
		const float Duration,
		const FT66AnimationCurveSpec& Curve,
		TFunction<void(float)> ProgressCallback)
	{
		FT66AnimationTimeline Timeline{ FName(Name) };
		Timeline.SetDuration(Duration);
		Timeline.SetCurve(Curve);
		Timeline.SetMaxMarkerDispatchesPerTick(12);
		Timeline.SetProgressCallback(MoveTemp(ProgressCallback));
		return Timeline;
	}
}

// ============================================================================
// GUESS THE CUP
// ============================================================================

namespace CupStage
{
	constexpr float CupWidth = 160.f;
	constexpr float CupHeight = 180.f;
	constexpr float CupBottomPadding = 28.f;
	constexpr float TokenSize = 76.f;
	constexpr float SlotPitch = 230.f;
	constexpr float RaiseHeight = 85.f;
	constexpr float RevealLift = 120.f;
	constexpr int32 ShuffleSwaps = 6;
}

float UT66GuessCupGameWidget::SlotX(const int32 SlotIndex)
{
	return (SlotIndex - 1) * CupStage::SlotPitch;
}

TSharedRef<SWidget> UT66GuessCupGameWidget::RebuildWidget()
{
	// A host rebuild discards the previous stage; the active timer dies with it.
	ClearStageTimer();

	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));

	TSharedRef<SOverlay> StageContent = SNew(SOverlay);

	StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 4.f)
	[
		SAssignNew(GlowBox, SBox).WidthOverride(300.f).HeightOverride(300.f)
		[
			SNew(SImage).Image(T66GamblerStage::SpriteBrush(TEXT("win_glow.png"), FVector2D(300.f, 300.f)))
		]
	];

	StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 36.f)
	[
		SAssignNew(TokenBox, SBox).WidthOverride(CupStage::TokenSize).HeightOverride(CupStage::TokenSize)
		[
			SNew(SImage).Image(T66GamblerStage::SpriteBrush(TEXT("token.png"), FVector2D(CupStage::TokenSize, CupStage::TokenSize)))
		]
	];

	for (int32 Index = 0; Index < CupCount; ++Index)
	{
		StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, CupStage::CupBottomPadding)
		[
			SAssignNew(Cups[Index].Box, SBox).WidthOverride(CupStage::CupWidth).HeightOverride(CupStage::CupHeight)
			[
				SAssignNew(Cups[Index].Image, SImage)
				.Image(T66GamblerStage::SpriteBrush(TEXT("cup.png"), FVector2D(CupStage::CupWidth, CupStage::CupHeight)))
			]
		];
	}

	for (int32 Index = 0; Index < CupCount; ++Index)
	{
		TSharedRef<SBorder> HitZone = MakeStageHitZone(
			CupStage::CupWidth + 24.f,
			CupStage::CupHeight + 60.f,
			FPointerEventHandler::CreateLambda([WeakThis = TWeakObjectPtr<UT66GuessCupGameWidget>(this), Index](const FGeometry&, const FPointerEvent&)
			{
				if (UT66GuessCupGameWidget* Self = WeakThis.Get())
				{
					return Self->OnCupClicked(Index);
				}
				return FReply::Unhandled();
			}));
		T66GamblerStage::ApplySpriteTransform(HitZone, FVector2D(SlotX(Index), 0.f));
		HitZones[Index] = HitZone;
		StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 8.f)
		[
			HitZone
		];
	}

	StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 16.f, 0.f, 0.f)
	[
		T66GamblerStage::MakeResultBanner(BannerText)
	];

	TSharedRef<SWidget> Stage = T66GamblerStage::MakeStage(StageContent);
	StageRoot = Stage;

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeLiveCasinoHeader(
				NSLOCTEXT("T66.Gambler", "GuessCupTitle", "GUESS THE CUP"),
				StatusText,
				WagerText,
				FOnClicked::CreateUObject(this, &UT66GuessCupGameWidget::OnBackClicked))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			Stage
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SAssignNew(PromptText, STextBlock)
			.Text(NSLOCTEXT("T66.Gambler", "PickACup", "Pick a cup."))
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
		];
}

void UT66GuessCupGameWidget::NativeDestruct()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	ClearStageTimer();
	Super::NativeDestruct();
}

void UT66GuessCupGameWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
	ResetForOpen();
	EnsureStageTimer();
}

void UT66GuessCupGameWidget::DeactivateWidgetGame()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	bChoiceEnabled = false;
	bRoundArmed = false;
	ClearStageTimer();
	WidgetGameHostContext = FT66WidgetGameHostContext();
}

void UT66GuessCupGameWidget::RequestWidgetGameExit()
{
	if (ReturnCallback)
	{
		ReturnCallback();
	}
	else
	{
		WidgetGameHostContext.RequestExit(ET66WidgetGameExitReason::PlayerCancelled);
	}
}

void UT66GuessCupGameWidget::SetChoiceCallback(TFunction<void(int32)> InChoiceCallback)
{
	ChoiceCallback = MoveTemp(InChoiceCallback);
}

void UT66GuessCupGameWidget::SetReturnCallback(TFunction<void()> InReturnCallback)
{
	ReturnCallback = MoveTemp(InReturnCallback);
}

void UT66GuessCupGameWidget::SetRevealCompleteCallback(TFunction<void()> InRevealCompleteCallback)
{
	RevealCompleteCallback = MoveTemp(InRevealCompleteCallback);
}

void UT66GuessCupGameWidget::ResetForOpen()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	bChoiceEnabled = false;
	bRoundArmed = false;
	RevealChosenCup = INDEX_NONE;
	RevealWinningCup = INDEX_NONE;
	RevealPayoutGold = 0;
	TokenOpacity = 0.f;
	TokenTranslateX = SlotX(1);
	TokenTranslateY = 0.f;
	GlowOpacity = 0.f;
	GlowScale = 1.f;
	BannerOpacity = 0.f;
	BannerScale = 1.f;
	PromptPulseTime = 0.f;

	SnapCupsToSlots();
	for (FCupVisual& Cup : Cups)
	{
		if (Cup.Image.IsValid())
		{
			Cup.Image->SetColorAndOpacity(FLinearColor::White);
		}
	}
	if (BannerText.IsValid())
	{
		BannerText->SetText(FText::GetEmpty());
	}
	if (PromptText.IsValid())
	{
		PromptText->SetText(NSLOCTEXT("T66.Gambler", "CupPlaceBet", "Place your bet to start the shuffle."));
	}
	SetStatus(FText::GetEmpty(), FT66FlatStyle::Tokens::Text);
	ApplyStageTransforms();
	EnsureStageTimer();
}

void UT66GuessCupGameWidget::NotifyRoundArmed()
{
	bRoundArmed = true;
	if (PromptText.IsValid())
	{
		PromptText->SetText(NSLOCTEXT("T66.Gambler", "CupWatchShuffle", "Watch the cups..."));
	}
	BuildShuffleSequence();
	EnsureStageTimer();
}

void UT66GuessCupGameWidget::SetStatus(const FText& Message, const FLinearColor& Color)
{
	if (StatusText.IsValid())
	{
		StatusText->SetText(Message);
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UT66GuessCupGameWidget::SetWagerAmount(const int32 WagerAmount)
{
	if (WagerText.IsValid())
	{
		WagerText->SetText(BuildLiveCasinoWagerText(WagerAmount));
	}
}

void UT66GuessCupGameWidget::SnapCupsToSlots()
{
	for (int32 Index = 0; Index < CupCount; ++Index)
	{
		Cups[Index].Slot = Index;
		Cups[Index].Translation = FVector2D(SlotX(Index), 0.f);
		Cups[Index].Scale = FVector2D(1.f, 1.f);
		Cups[Index].AngleDeg = 0.f;
		Cups[Index].HoverLift = 0.f;
	}
}

void UT66GuessCupGameWidget::BuildShuffleSequence()
{
	ActiveSequence.Cancel();
	ActiveSequence = FT66AnimationSequence();
	bChoiceEnabled = false;
	SnapCupsToSlots();
	TokenOpacity = 0.f;
	TokenTranslateX = SlotX(1);

	TWeakObjectPtr<UT66GuessCupGameWidget> WeakThis(this);

	FT66AnimationTimeline Raise = MakeStageTimeline(TEXT("GuessCup.Raise"), 0.40f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis](const float Value)
		{
			if (UT66GuessCupGameWidget* Self = WeakThis.Get())
			{
				for (FCupVisual& Cup : Self->Cups)
				{
					Cup.Translation.Y = -CupStage::RaiseHeight * Value;
				}
				Self->TokenOpacity = FMath::Clamp(Value * 1.5f, 0.f, 1.f);
			}
		});
	ActiveSequence.AddTimeline(MoveTemp(Raise));

	FT66AnimationTimeline Tease = MakeStageTimeline(TEXT("GuessCup.Tease"), 0.55f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](const float Value)
		{
			if (UT66GuessCupGameWidget* Self = WeakThis.Get())
			{
				Self->TokenTranslateY = -14.f * FMath::Abs(FMath::Sin(Value * 2.f * PI));
			}
		});
	ActiveSequence.AddTimeline(MoveTemp(Tease));

	FT66AnimationTimeline Drop = MakeStageTimeline(TEXT("GuessCup.Drop"), 0.30f,
		FT66AnimationCurveSpec(ET66AnimationCurve::BounceOut),
		[WeakThis](const float Value)
		{
			if (UT66GuessCupGameWidget* Self = WeakThis.Get())
			{
				for (FCupVisual& Cup : Self->Cups)
				{
					Cup.Translation.Y = -CupStage::RaiseHeight * (1.f - Value);
				}
				Self->TokenTranslateY = 0.f;
				Self->TokenOpacity = FMath::Clamp(1.f - Value * 2.5f, 0.f, 1.f);
			}
		});
	ActiveSequence.AddTimeline(MoveTemp(Drop));

	// Plan the swap chain up-front so each swap timeline captures fixed from/to slots.
	int32 PlannedSlotOfVisual[CupCount];
	for (int32 Index = 0; Index < CupCount; ++Index)
	{
		PlannedSlotOfVisual[Index] = Index;
	}
	FRandomStream Rng(static_cast<int32>(FPlatformTime::Cycles64() & 0x7fffffff));
	for (int32 Swap = 0; Swap < CupStage::ShuffleSwaps; ++Swap)
	{
		const int32 SlotA = Rng.RandRange(0, CupCount - 1);
		int32 SlotB = Rng.RandRange(0, CupCount - 2);
		if (SlotB >= SlotA)
		{
			++SlotB;
		}

		int32 VisualA = INDEX_NONE;
		int32 VisualB = INDEX_NONE;
		for (int32 Index = 0; Index < CupCount; ++Index)
		{
			if (PlannedSlotOfVisual[Index] == SlotA) { VisualA = Index; }
			if (PlannedSlotOfVisual[Index] == SlotB) { VisualB = Index; }
		}
		PlannedSlotOfVisual[VisualA] = SlotB;
		PlannedSlotOfVisual[VisualB] = SlotA;

		const float FromXA = SlotX(SlotA);
		const float ToXA = SlotX(SlotB);
		const float Duration = FMath::Lerp(0.34f, 0.18f, static_cast<float>(Swap) / FMath::Max(1, CupStage::ShuffleSwaps - 1));

		FT66AnimationTimeline SwapTimeline = MakeStageTimeline(TEXT("GuessCup.Swap"), Duration,
			FT66AnimationCurveSpec(ET66AnimationCurve::EaseInOutQuad),
			[WeakThis, VisualA, VisualB, FromXA, ToXA](const float Value)
			{
				if (UT66GuessCupGameWidget* Self = WeakThis.Get())
				{
					const float Arc = FMath::Sin(FMath::Clamp(Value, 0.f, 1.f) * PI);
					FCupVisual& CupA = Self->Cups[VisualA];
					FCupVisual& CupB = Self->Cups[VisualB];
					CupA.Translation.X = FMath::Lerp(FromXA, ToXA, Value);
					CupA.Translation.Y = -22.f * Arc;
					CupA.Scale = FVector2D(1.f + 0.06f * Arc, 1.f + 0.06f * Arc);
					CupB.Translation.X = FMath::Lerp(ToXA, FromXA, Value);
					CupB.Translation.Y = 14.f * Arc;
					CupB.Scale = FVector2D(1.f - 0.05f * Arc, 1.f - 0.05f * Arc);
				}
			});
		SwapTimeline.AddMarker({ FName(TEXT("ShuffleTick")), ET66AnimationMarkerType::ProgressBased, 0.02f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
		const int32 FinalSlotA = PlannedSlotOfVisual[VisualA];
		const int32 FinalSlotB = PlannedSlotOfVisual[VisualB];
		SwapTimeline.AddMarker({ FName(TEXT("SwapDone")), ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
		// Bind slot bookkeeping into the progress callback's final frame instead of relying on payloads.
		FT66AnimationTimeline Snap = MakeStageTimeline(TEXT("GuessCup.SwapSnap"), 0.011f,
			FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
			[WeakThis, VisualA, VisualB, FinalSlotA, FinalSlotB](const float)
			{
				if (UT66GuessCupGameWidget* Self = WeakThis.Get())
				{
					Self->Cups[VisualA].Slot = FinalSlotA;
					Self->Cups[VisualA].Translation = FVector2D(SlotX(FinalSlotA), 0.f);
					Self->Cups[VisualA].Scale = FVector2D(1.f, 1.f);
					Self->Cups[VisualB].Slot = FinalSlotB;
					Self->Cups[VisualB].Translation = FVector2D(SlotX(FinalSlotB), 0.f);
					Self->Cups[VisualB].Scale = FVector2D(1.f, 1.f);
				}
			});
		ActiveSequence.AddTimeline(MoveTemp(SwapTimeline));
		ActiveSequence.AddTimeline(MoveTemp(Snap));
	}

	FT66AnimationTimeline Settle = MakeStageTimeline(TEXT("GuessCup.Settle"), 0.25f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](const float Value)
		{
			if (UT66GuessCupGameWidget* Self = WeakThis.Get())
			{
				const float Bob = FMath::Sin(FMath::Clamp(Value, 0.f, 1.f) * PI);
				for (FCupVisual& Cup : Self->Cups)
				{
					Cup.Translation.Y = -5.f * Bob;
				}
			}
		});
	Settle.AddMarker({ FName(TEXT("ShuffleDone")), ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(Settle));

	bSequenceActive = true;
	ActiveSequence.Play();
}

void UT66GuessCupGameWidget::RevealResult(const int32 ChosenCup, const int32 WinningCup, const int32 PayoutGold)
{
	RevealChosenCup = FMath::Clamp(ChosenCup, 0, CupCount - 1);
	RevealWinningCup = FMath::Clamp(WinningCup, 0, CupCount - 1);
	RevealPayoutGold = PayoutGold;
	bChoiceEnabled = false;
	bRoundArmed = false;

	if (BannerText.IsValid())
	{
		BannerText->SetText(PayoutGold > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "CupWinFormat", "Cup {0}. WIN (+{1})"), FText::AsNumber(WinningCup + 1), FText::AsNumber(PayoutGold))
			: FText::Format(NSLOCTEXT("T66.Gambler", "CupLoseFormat", "Cup {0}. LOSE"), FText::AsNumber(WinningCup + 1)));
		BannerText->SetColorAndOpacity(PayoutGold > 0 ? T66GamblerStage::WinGold() : T66GamblerStage::LoseRed());
	}
	if (PromptText.IsValid())
	{
		PromptText->SetText(FText::GetEmpty());
	}

	BuildRevealSequence();
	EnsureStageTimer();
}

void UT66GuessCupGameWidget::BuildRevealSequence()
{
	ActiveSequence.Cancel();
	ActiveSequence = FT66AnimationSequence();

	// If a shuffle was still running, settle every cup onto its planned slot first.
	for (int32 Index = 0; Index < CupCount; ++Index)
	{
		Cups[Index].Translation = FVector2D(SlotX(Cups[Index].Slot), 0.f);
		Cups[Index].Scale = FVector2D(1.f, 1.f);
		Cups[Index].AngleDeg = 0.f;
		Cups[Index].HoverLift = 0.f;
	}

	const bool bWin = RevealPayoutGold > 0;
	const int32 ChosenSlot = RevealChosenCup;
	const int32 WinningSlot = RevealWinningCup;
	TWeakObjectPtr<UT66GuessCupGameWidget> WeakThis(this);

	auto VisualAtSlot = [](UT66GuessCupGameWidget* Self, const int32 InSlot) -> FCupVisual&
	{
		for (FCupVisual& Cup : Self->Cups)
		{
			if (Cup.Slot == InSlot)
			{
				return Cup;
			}
		}
		return Self->Cups[0];
	};

	FT66AnimationTimeline LiftChosen = MakeStageTimeline(TEXT("GuessCup.LiftChosen"), 0.50f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis, ChosenSlot, bWin, VisualAtSlot](const float Value)
		{
			if (UT66GuessCupGameWidget* Self = WeakThis.Get())
			{
				FCupVisual& Cup = VisualAtSlot(Self, ChosenSlot);
				Cup.Translation.Y = -CupStage::RevealLift * Value;
				Cup.AngleDeg = -5.f * Value;
				if (bWin)
				{
					Self->TokenTranslateX = SlotX(ChosenSlot);
					Self->TokenOpacity = FMath::Clamp(Value * 1.6f, 0.f, 1.f);
					Self->GlowTranslateX = SlotX(ChosenSlot);
					Self->GlowOpacity = 0.75f * Value;
					Self->GlowScale = 0.85f + 0.25f * Value;
				}
			}
		});
	LiftChosen.AddMarker({ FName(TEXT("CupLift")), ET66AnimationMarkerType::ProgressBased, 0.02f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(LiftChosen));

	if (!bWin)
	{
		FT66AnimationTimeline Beat = MakeStageTimeline(TEXT("GuessCup.Beat"), 0.30f,
			FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
			[WeakThis, ChosenSlot, VisualAtSlot](const float Value)
			{
				if (UT66GuessCupGameWidget* Self = WeakThis.Get())
				{
					FCupVisual& Cup = VisualAtSlot(Self, ChosenSlot);
					Cup.AngleDeg = 4.f * FMath::Sin(Value * 2.f * PI);
				}
			});
		Beat.AddMarker({ FName(TEXT("ChosenLoseTint")), ET66AnimationMarkerType::ProgressBased, 0.5f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
		ActiveSequence.AddTimeline(MoveTemp(Beat));

		FT66AnimationTimeline LiftWinning = MakeStageTimeline(TEXT("GuessCup.LiftWinning"), 0.42f,
			FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
			[WeakThis, WinningSlot, VisualAtSlot](const float Value)
			{
				if (UT66GuessCupGameWidget* Self = WeakThis.Get())
				{
					FCupVisual& Cup = VisualAtSlot(Self, WinningSlot);
					Cup.Translation.Y = -CupStage::RevealLift * Value;
					Self->TokenTranslateX = SlotX(WinningSlot);
					Self->TokenOpacity = FMath::Clamp(Value * 1.6f, 0.f, 1.f);
				}
			});
		LiftWinning.AddMarker({ FName(TEXT("CupLift")), ET66AnimationMarkerType::ProgressBased, 0.02f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
		ActiveSequence.AddTimeline(MoveTemp(LiftWinning));
	}

	FT66AnimationTimeline Banner = MakeStageTimeline(TEXT("GuessCup.Banner"), 0.35f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Overshoot, 0.85f),
		[WeakThis](const float Value)
		{
			if (UT66GuessCupGameWidget* Self = WeakThis.Get())
			{
				Self->BannerOpacity = FMath::Clamp(Value, 0.f, 1.f);
				Self->BannerScale = 0.6f + 0.4f * Value;
			}
		});
	Banner.AddMarker({ FName(TEXT("RevealComplete")), ET66AnimationMarkerType::ProgressBased, 0.65f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(Banner));

	FT66AnimationTimeline Hold = MakeStageTimeline(TEXT("GuessCup.Hold"), 0.45f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis, bWin](const float Value)
		{
			if (UT66GuessCupGameWidget* Self = WeakThis.Get())
			{
				if (bWin)
				{
					Self->GlowScale = 1.10f + 0.05f * FMath::Sin(Value * 2.f * PI);
				}
			}
		});
	ActiveSequence.AddTimeline(MoveTemp(Hold));

	bSequenceActive = true;
	ActiveSequence.Play();
}

void UT66GuessCupGameWidget::HandleSequenceMarkers(const TArray<FT66AnimationMarkerEvent>& MarkerEvents)
{
	for (const FT66AnimationMarkerEvent& Event : MarkerEvents)
	{
		if (Event.MarkerID == FName(TEXT("ShuffleTick")))
		{
			T66GamblerStage::PlayUISound(TEXT("Casino.CupShuffle"));
		}
		else if (Event.MarkerID == FName(TEXT("ShuffleDone")))
		{
			bChoiceEnabled = true;
			if (PromptText.IsValid())
			{
				PromptText->SetText(NSLOCTEXT("T66.Gambler", "PickACupNow", "PICK A CUP!"));
			}
		}
		else if (Event.MarkerID == FName(TEXT("CupLift")))
		{
			T66GamblerStage::PlayUISound(TEXT("Casino.CardFlip"));
		}
		else if (Event.MarkerID == FName(TEXT("ChosenLoseTint")))
		{
			for (FCupVisual& Cup : Cups)
			{
				if (Cup.Slot == RevealChosenCup && Cup.Image.IsValid())
				{
					Cup.Image->SetColorAndOpacity(T66GamblerStage::DimTint());
				}
			}
		}
		else if (Event.MarkerID == FName(TEXT("RevealComplete")))
		{
			if (RevealCompleteCallback)
			{
				RevealCompleteCallback();
			}
		}
	}
}

FReply UT66GuessCupGameWidget::OnBackClicked()
{
	RequestWidgetGameExit();
	return FReply::Handled();
}

FReply UT66GuessCupGameWidget::OnCupClicked(const int32 SlotIndex)
{
	if (!bChoiceEnabled)
	{
		return FReply::Handled();
	}

	bChoiceEnabled = false;
	if (ChoiceCallback)
	{
		ChoiceCallback(SlotIndex);
	}
	return FReply::Handled();
}

void UT66GuessCupGameWidget::ApplyStageTransforms()
{
	for (FCupVisual& Cup : Cups)
	{
		T66GamblerStage::ApplySpriteTransform(
			Cup.Box,
			Cup.Translation + FVector2D(0.f, Cup.HoverLift),
			Cup.Scale,
			Cup.AngleDeg);
	}

	T66GamblerStage::ApplySpriteTransform(TokenBox, FVector2D(TokenTranslateX, TokenTranslateY));
	if (TokenBox.IsValid())
	{
		TokenBox->SetRenderOpacity(TokenOpacity);
	}

	T66GamblerStage::ApplySpriteTransform(GlowBox, FVector2D(GlowTranslateX, 0.f), FVector2D(GlowScale, GlowScale));
	if (GlowBox.IsValid())
	{
		GlowBox->SetRenderOpacity(GlowOpacity);
	}

	if (BannerText.IsValid())
	{
		BannerText->SetRenderOpacity(BannerOpacity);
		T66GamblerStage::ApplySpriteTransform(BannerText, FVector2D::ZeroVector, FVector2D(BannerScale, BannerScale));
	}
	if (PromptText.IsValid() && bChoiceEnabled)
	{
		const float Pulse = 1.f + 0.05f * FMath::Sin(PromptPulseTime * 5.f);
		T66GamblerStage::ApplySpriteTransform(PromptText, FVector2D::ZeroVector, FVector2D(Pulse, Pulse));
	}
}

void UT66GuessCupGameWidget::EnsureStageTimer()
{
	if (!StageRoot.IsValid() || StageTimerHandle.IsValid())
	{
		return;
	}

	StageTimerHandle = StageRoot->RegisterActiveTimer(
		1.f / 60.f,
		FWidgetActiveTimerDelegate::CreateUObject(this, &UT66GuessCupGameWidget::HandleStageTick));
}

void UT66GuessCupGameWidget::ClearStageTimer()
{
	if (StageRoot.IsValid() && StageTimerHandle.IsValid())
	{
		StageRoot->UnRegisterActiveTimer(StageTimerHandle.ToSharedRef());
	}
	StageTimerHandle.Reset();
}

EActiveTimerReturnType UT66GuessCupGameWidget::HandleStageTick(double, const float DeltaTime)
{
	if (bSequenceActive)
	{
		TArray<FT66AnimationMarkerEvent> MarkerEvents;
		ActiveSequence.Tick(FMath::Max(0.f, DeltaTime), MarkerEvents);
		if (MarkerEvents.Num() > 0)
		{
			HandleSequenceMarkers(MarkerEvents);
		}
		if (T66Animation::IsTerminalState(ActiveSequence.GetState()))
		{
			bSequenceActive = false;
		}
	}

	PromptPulseTime += DeltaTime;
	for (int32 Index = 0; Index < CupCount; ++Index)
	{
		const bool bHovered = bChoiceEnabled
			&& HitZones[Index].IsValid()
			&& HitZones[Index]->IsHovered();
		for (FCupVisual& Cup : Cups)
		{
			if (Cup.Slot == Index)
			{
				const float Target = bHovered ? -16.f : 0.f;
				Cup.HoverLift = FMath::FInterpTo(Cup.HoverLift, Target, DeltaTime, 14.f);
			}
		}
	}

	ApplyStageTransforms();
	return EActiveTimerReturnType::Continue;
}

// ============================================================================
// PICK THE STICK
// ============================================================================

namespace StickStage
{
	constexpr float StickWidth = 34.f;
	constexpr float CapHeight = 30.f;
	constexpr float SlotPitch = 70.f;
	constexpr float NeutralLength = 150.f;
	constexpr float BottomPadding = 30.f;
	constexpr float RestTopHeight = 204.f;   // Stick tops above the stage bottom while hidden.
	constexpr float CauldronRimHeight = 152.f;
	constexpr float PullClearance = 14.f;
	const float RevealLengths[5] = { 80.f, 108.f, 136.f, 164.f, 192.f };
}

float UT66StickPickGameWidget::StickSlotX(const int32 StickIndex)
{
	return (StickIndex - 2) * StickStage::SlotPitch;
}

TSharedRef<SWidget> UT66StickPickGameWidget::RebuildWidget()
{
	// A host rebuild discards the previous stage; the active timer dies with it.
	ClearStageTimer();

	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));

	TSharedRef<SOverlay> StageContent = SNew(SOverlay);

	StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 60.f)
	[
		SAssignNew(GlowBox, SBox).WidthOverride(240.f).HeightOverride(240.f)
		[
			SNew(SImage).Image(T66GamblerStage::SpriteBrush(TEXT("win_glow.png"), FVector2D(240.f, 240.f)))
		]
	];

	// Sticks first so the cauldron front wall hides their lower halves.
	for (int32 Index = 0; Index < StickCount; ++Index)
	{
		FStickVisual& Stick = Sticks[Index];
		TSharedRef<SWidget> Body =
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBox).WidthOverride(StickStage::StickWidth).HeightOverride(StickStage::CapHeight)
				[
					SAssignNew(Stick.CapImage, SImage)
					.Image(T66GamblerStage::SpriteBrush(TEXT("stick_cap.png"), FVector2D(StickStage::StickWidth, StickStage::CapHeight)))
				]
			]
			+ SVerticalBox::Slot().FillHeight(1.f)
			[
				SAssignNew(Stick.ShaftImage, SImage)
				.Image(T66GamblerStage::SpriteBrush(TEXT("stick_shaft.png"), FVector2D(StickStage::StickWidth, 120.f)))
			];

		StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, StickStage::BottomPadding)
		[
			SAssignNew(Stick.Box, SBox)
			.WidthOverride(StickStage::StickWidth)
			.HeightOverride(StickStage::NeutralLength)
			[
				Body
			]
		];
		Stick.Body = Stick.Box;
	}

	TSharedPtr<SBox> CauldronBox;
	StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 12.f)
	[
		SAssignNew(CauldronBox, SBox).WidthOverride(440.f).HeightOverride(140.f)
		[
			SNew(SImage).Image(T66GamblerStage::SpriteBrush(TEXT("stick_holder.png"), FVector2D(440.f, 140.f)))
		]
	];
	CauldronWidget = CauldronBox;

	// Hit zones over the protruding stick tops.
	for (int32 Index = 0; Index < StickCount; ++Index)
	{
		TSharedRef<SBorder> HitZone = MakeStageHitZone(
			56.f,
			110.f,
			FPointerEventHandler::CreateLambda([WeakThis = TWeakObjectPtr<UT66StickPickGameWidget>(this), Index](const FGeometry&, const FPointerEvent&)
			{
				if (UT66StickPickGameWidget* Self = WeakThis.Get())
				{
					return Self->OnStickClicked(Index);
				}
				return FReply::Unhandled();
			}));
		T66GamblerStage::ApplySpriteTransform(HitZone, FVector2D(StickSlotX(Index), 0.f));
		HitZones[Index] = HitZone;
		StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 140.f)
		[
			HitZone
		];
	}

	StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 16.f, 0.f, 0.f)
	[
		T66GamblerStage::MakeResultBanner(BannerText)
	];

	TSharedRef<SWidget> Stage = T66GamblerStage::MakeStage(StageContent);
	StageRoot = Stage;

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeLiveCasinoHeader(
				NSLOCTEXT("T66.Gambler", "StickPickTitle", "PICK THE STICK"),
				StatusText,
				WagerText,
				FOnClicked::CreateUObject(this, &UT66StickPickGameWidget::OnBackClicked))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			Stage
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SAssignNew(TargetText, STextBlock)
			.Text(NSLOCTEXT("T66.Gambler", "StickTargetPending", "Target: -"))
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Accent2)
		];
}

void UT66StickPickGameWidget::NativeDestruct()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	ClearStageTimer();
	Super::NativeDestruct();
}

void UT66StickPickGameWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
	ResetForOpen();
	EnsureStageTimer();
}

void UT66StickPickGameWidget::DeactivateWidgetGame()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	bChoiceEnabled = false;
	bRoundArmed = false;
	ClearStageTimer();
	WidgetGameHostContext = FT66WidgetGameHostContext();
}

void UT66StickPickGameWidget::RequestWidgetGameExit()
{
	if (ReturnCallback)
	{
		ReturnCallback();
	}
	else
	{
		WidgetGameHostContext.RequestExit(ET66WidgetGameExitReason::PlayerCancelled);
	}
}

void UT66StickPickGameWidget::SetChoiceCallback(TFunction<void(int32)> InChoiceCallback)
{
	ChoiceCallback = MoveTemp(InChoiceCallback);
}

void UT66StickPickGameWidget::SetReturnCallback(TFunction<void()> InReturnCallback)
{
	ReturnCallback = MoveTemp(InReturnCallback);
}

void UT66StickPickGameWidget::SetRevealCompleteCallback(TFunction<void()> InRevealCompleteCallback)
{
	RevealCompleteCallback = MoveTemp(InRevealCompleteCallback);
}

void UT66StickPickGameWidget::ResetForOpen()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	bChoiceEnabled = false;
	bRoundArmed = false;
	RevealChosenStick = INDEX_NONE;
	RevealTargetStick = INDEX_NONE;
	RevealPayoutGold = 0;
	GlowOpacity = 0.f;
	GlowScale = 1.f;
	BannerOpacity = 0.f;
	BannerScale = 1.f;
	CauldronTranslateY = 0.f;

	for (int32 Index = 0; Index < StickCount; ++Index)
	{
		FStickVisual& Stick = Sticks[Index];
		Stick.LengthPx = StickStage::NeutralLength;
		Stick.RestTranslation = StickStage::NeutralLength + StickStage::BottomPadding - StickStage::RestTopHeight;
		Stick.Translation = Stick.RestTranslation;
		Stick.PullAlpha = 0.f;
		if (Stick.Box.IsValid())
		{
			Stick.Box->SetHeightOverride(StickStage::NeutralLength);
		}
		if (Stick.CapImage.IsValid())
		{
			Stick.CapImage->SetColorAndOpacity(FLinearColor::White);
		}
		if (Stick.ShaftImage.IsValid())
		{
			Stick.ShaftImage->SetColorAndOpacity(FLinearColor::White);
		}
	}

	if (BannerText.IsValid())
	{
		BannerText->SetText(FText::GetEmpty());
	}
	SetStatus(FText::GetEmpty(), FT66FlatStyle::Tokens::Text);
	BuildIntroSequence();
	ApplyStageTransforms();
	EnsureStageTimer();
}

void UT66StickPickGameWidget::NotifyRoundArmed()
{
	bRoundArmed = true;
	bChoiceEnabled = !bSequenceActive;
	EnsureStageTimer();
}

void UT66StickPickGameWidget::SetStatus(const FText& Message, const FLinearColor& Color)
{
	if (StatusText.IsValid())
	{
		StatusText->SetText(Message);
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UT66StickPickGameWidget::SetWagerAmount(const int32 WagerAmount)
{
	if (WagerText.IsValid())
	{
		WagerText->SetText(BuildLiveCasinoWagerText(WagerAmount));
	}
}

void UT66StickPickGameWidget::SetTargetShortest(const bool bShortest)
{
	bPendingTargetShortest = bShortest;
	if (TargetText.IsValid())
	{
		TargetText->SetText(bShortest
			? NSLOCTEXT("T66.Gambler", "StickTargetShortest", "Target: SHORTEST")
			: NSLOCTEXT("T66.Gambler", "StickTargetLongest", "Target: LONGEST"));
	}
}

void UT66StickPickGameWidget::BuildIntroSequence()
{
	ActiveSequence.Cancel();
	ActiveSequence = FT66AnimationSequence();
	TWeakObjectPtr<UT66StickPickGameWidget> WeakThis(this);

	FT66AnimationTimeline SlideIn = MakeStageTimeline(TEXT("StickPick.SlideIn"), 0.45f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis](const float Value)
		{
			if (UT66StickPickGameWidget* Self = WeakThis.Get())
			{
				Self->CauldronTranslateY = 190.f * (1.f - FMath::Clamp(Value, 0.f, 1.f));
			}
		});
	ActiveSequence.AddTimeline(MoveTemp(SlideIn));

	FT66AnimationTimeline Wiggle = MakeStageTimeline(TEXT("StickPick.Wiggle"), 0.35f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](const float Value)
		{
			if (UT66StickPickGameWidget* Self = WeakThis.Get())
			{
				for (int32 Index = 0; Index < StickCount; ++Index)
				{
					const float Phase = Value * PI + Index * 0.7f;
					Self->Sticks[Index].Translation = Self->Sticks[Index].RestTranslation - 4.f * FMath::Sin(Phase) * (1.f - Value);
				}
			}
		});
	Wiggle.AddMarker({ FName(TEXT("IntroDone")), ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(Wiggle));

	bSequenceActive = true;
	ActiveSequence.Play();
}

void UT66StickPickGameWidget::AssignRevealLengths()
{
	float Lengths[StickCount];
	for (int32 Index = 0; Index < StickCount; ++Index)
	{
		Lengths[Index] = StickStage::RevealLengths[Index];
	}

	const float TargetLength = bRevealTargetShortest ? Lengths[0] : Lengths[StickCount - 1];

	TArray<float> Remaining;
	for (int32 Index = 0; Index < StickCount; ++Index)
	{
		if (Lengths[Index] != TargetLength)
		{
			Remaining.Add(Lengths[Index]);
		}
	}

	FRandomStream Rng(RevealChosenStick * 7919 + RevealTargetStick * 104729 + (bRevealTargetShortest ? 13 : 7));
	for (int32 Index = Remaining.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = Rng.RandRange(0, Index);
		Remaining.Swap(Index, SwapIndex);
	}

	int32 RemainingCursor = 0;
	for (int32 Index = 0; Index < StickCount; ++Index)
	{
		Sticks[Index].LengthPx = Index == RevealTargetStick ? TargetLength : Remaining[RemainingCursor++];
	}
}

void UT66StickPickGameWidget::RevealResult(const int32 ChosenStick, const int32 TargetStick, const bool bTargetShortest, const int32 PayoutGold)
{
	RevealChosenStick = FMath::Clamp(ChosenStick, 0, StickCount - 1);
	RevealTargetStick = FMath::Clamp(TargetStick, 0, StickCount - 1);
	bRevealTargetShortest = bTargetShortest;
	RevealPayoutGold = PayoutGold;
	bChoiceEnabled = false;
	bRoundArmed = false;

	if (BannerText.IsValid())
	{
		const FText Target = bTargetShortest
			? NSLOCTEXT("T66.Gambler", "Shortest", "shortest")
			: NSLOCTEXT("T66.Gambler", "Longest", "longest");
		BannerText->SetText(PayoutGold > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "StickWinFormat", "Stick {0} was the {1}. WIN (+{2})"), FText::AsNumber(TargetStick + 1), Target, FText::AsNumber(PayoutGold))
			: FText::Format(NSLOCTEXT("T66.Gambler", "StickLoseFormat", "Stick {0} was the {1}. LOSE"), FText::AsNumber(TargetStick + 1), Target));
		BannerText->SetColorAndOpacity(PayoutGold > 0 ? T66GamblerStage::WinGold() : T66GamblerStage::LoseRed());
	}

	AssignRevealLengths();
	BuildRevealSequence();
	EnsureStageTimer();
}

void UT66StickPickGameWidget::BuildRevealSequence()
{
	ActiveSequence.Cancel();
	ActiveSequence = FT66AnimationSequence();
	CauldronTranslateY = 0.f;

	// Re-anchor every stick at its (now per-length) rest pose so tops stay flush.
	for (int32 Index = 0; Index < StickCount; ++Index)
	{
		FStickVisual& Stick = Sticks[Index];
		Stick.RestTranslation = Stick.LengthPx + StickStage::BottomPadding - StickStage::RestTopHeight;
		Stick.Translation = Stick.RestTranslation;
		Stick.PullAlpha = 0.f;
		if (Stick.Box.IsValid())
		{
			Stick.Box->SetHeightOverride(Stick.LengthPx);
		}
	}

	const bool bWin = RevealPayoutGold > 0;
	const int32 Chosen = RevealChosenStick;
	const int32 Target = RevealTargetStick;
	TWeakObjectPtr<UT66StickPickGameWidget> WeakThis(this);

	auto PullStick = [](UT66StickPickGameWidget* Self, const int32 Index, const float Alpha)
	{
		FStickVisual& Stick = Self->Sticks[Index];
		Stick.PullAlpha = Alpha;
		const float PullDistance = (StickStage::CauldronRimHeight - (StickStage::BottomPadding - Stick.RestTranslation)) + StickStage::PullClearance;
		Stick.Translation = Stick.RestTranslation - PullDistance * Alpha;
	};

	FT66AnimationTimeline PullChosen = MakeStageTimeline(TEXT("StickPick.PullChosen"), 0.9f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseInOutCubic),
		[WeakThis, Chosen, PullStick](const float Value)
		{
			if (UT66StickPickGameWidget* Self = WeakThis.Get())
			{
				PullStick(Self, Chosen, FMath::Clamp(Value, 0.f, 1.f));
			}
		});
	PullChosen.AddMarker({ FName(TEXT("StickDraw")), ET66AnimationMarkerType::ProgressBased, 0.04f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(PullChosen));

	FT66AnimationTimeline Beat = MakeStageTimeline(TEXT("StickPick.Beat"), 0.30f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](const float)
		{
		});
	ActiveSequence.AddTimeline(MoveTemp(Beat));

	FT66AnimationTimeline PullOthers = MakeStageTimeline(TEXT("StickPick.PullOthers"), 0.5f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis, Chosen, PullStick](const float Value)
		{
			if (UT66StickPickGameWidget* Self = WeakThis.Get())
			{
				for (int32 Index = 0; Index < StickCount; ++Index)
				{
					if (Index != Chosen)
					{
						PullStick(Self, Index, FMath::Clamp(Value, 0.f, 1.f));
					}
				}
			}
		});
	PullOthers.AddMarker({ FName(TEXT("StickDraw")), ET66AnimationMarkerType::ProgressBased, 0.04f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(PullOthers));

	FT66AnimationTimeline Highlight = MakeStageTimeline(TEXT("StickPick.Highlight"), 0.40f,
		FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
		[WeakThis, Target, Chosen, bWin, PullStick](const float Value)
		{
			if (UT66StickPickGameWidget* Self = WeakThis.Get())
			{
				Self->GlowTranslateX = StickSlotX(Target);
				Self->GlowOpacity = 0.7f * FMath::Clamp(Value, 0.f, 1.f);
				Self->GlowScale = 0.85f + 0.25f * Value;
				// Target stick celebratory hop on top of its pulled pose.
				const float Hop = 10.f * FMath::Sin(FMath::Clamp(Value, 0.f, 1.f) * PI);
				PullStick(Self, Target, 1.f);
				Self->Sticks[Target].Translation -= Hop;
			}
		});
	Highlight.AddMarker({ FName(TEXT("ChosenJudged")), ET66AnimationMarkerType::ProgressBased, 0.3f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(Highlight));

	FT66AnimationTimeline Banner = MakeStageTimeline(TEXT("StickPick.Banner"), 0.35f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Overshoot, 0.85f),
		[WeakThis](const float Value)
		{
			if (UT66StickPickGameWidget* Self = WeakThis.Get())
			{
				Self->BannerOpacity = FMath::Clamp(Value, 0.f, 1.f);
				Self->BannerScale = 0.6f + 0.4f * Value;
			}
		});
	Banner.AddMarker({ FName(TEXT("RevealComplete")), ET66AnimationMarkerType::ProgressBased, 0.65f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(Banner));

	FT66AnimationTimeline Hold = MakeStageTimeline(TEXT("StickPick.Hold"), 0.45f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis, bWin](const float Value)
		{
			if (UT66StickPickGameWidget* Self = WeakThis.Get())
			{
				if (bWin)
				{
					Self->GlowScale = 1.10f + 0.05f * FMath::Sin(Value * 2.f * PI);
				}
			}
		});
	ActiveSequence.AddTimeline(MoveTemp(Hold));

	bSequenceActive = true;
	ActiveSequence.Play();
}

void UT66StickPickGameWidget::HandleSequenceMarkers(const TArray<FT66AnimationMarkerEvent>& MarkerEvents)
{
	for (const FT66AnimationMarkerEvent& Event : MarkerEvents)
	{
		if (Event.MarkerID == FName(TEXT("StickDraw")))
		{
			T66GamblerStage::PlayUISound(TEXT("Casino.StickDraw"));
		}
		else if (Event.MarkerID == FName(TEXT("IntroDone")))
		{
			bChoiceEnabled = true;
		}
		else if (Event.MarkerID == FName(TEXT("ChosenJudged")))
		{
			if (RevealPayoutGold <= 0)
			{
				FStickVisual& Chosen = Sticks[RevealChosenStick];
				if (Chosen.CapImage.IsValid())
				{
					Chosen.CapImage->SetColorAndOpacity(T66GamblerStage::DimTint());
				}
				if (Chosen.ShaftImage.IsValid())
				{
					Chosen.ShaftImage->SetColorAndOpacity(T66GamblerStage::DimTint());
				}
			}
		}
		else if (Event.MarkerID == FName(TEXT("RevealComplete")))
		{
			if (RevealCompleteCallback)
			{
				RevealCompleteCallback();
			}
		}
	}
}

FReply UT66StickPickGameWidget::OnBackClicked()
{
	RequestWidgetGameExit();
	return FReply::Handled();
}

FReply UT66StickPickGameWidget::OnStickClicked(const int32 StickIndex)
{
	if (!bChoiceEnabled || bSequenceActive)
	{
		return FReply::Handled();
	}

	bChoiceEnabled = false;
	if (ChoiceCallback)
	{
		ChoiceCallback(StickIndex);
	}
	return FReply::Handled();
}

void UT66StickPickGameWidget::ApplyStageTransforms()
{
	T66GamblerStage::ApplySpriteTransform(CauldronWidget, FVector2D(0.f, CauldronTranslateY));
	for (int32 Index = 0; Index < StickCount; ++Index)
	{
		FStickVisual& Stick = Sticks[Index];
		float Hover = 0.f;
		if (bChoiceEnabled && !bSequenceActive && HitZones[Index].IsValid() && HitZones[Index]->IsHovered())
		{
			Hover = -8.f;
		}
		T66GamblerStage::ApplySpriteTransform(
			Stick.Box,
			FVector2D(StickSlotX(Index), Stick.Translation + CauldronTranslateY + Hover));
	}

	T66GamblerStage::ApplySpriteTransform(GlowBox, FVector2D(GlowTranslateX, 0.f), FVector2D(GlowScale, GlowScale));
	if (GlowBox.IsValid())
	{
		GlowBox->SetRenderOpacity(GlowOpacity);
	}
	if (BannerText.IsValid())
	{
		BannerText->SetRenderOpacity(BannerOpacity);
		T66GamblerStage::ApplySpriteTransform(BannerText, FVector2D::ZeroVector, FVector2D(BannerScale, BannerScale));
	}
}

void UT66StickPickGameWidget::EnsureStageTimer()
{
	if (!StageRoot.IsValid() || StageTimerHandle.IsValid())
	{
		return;
	}

	StageTimerHandle = StageRoot->RegisterActiveTimer(
		1.f / 60.f,
		FWidgetActiveTimerDelegate::CreateUObject(this, &UT66StickPickGameWidget::HandleStageTick));
}

void UT66StickPickGameWidget::ClearStageTimer()
{
	if (StageRoot.IsValid() && StageTimerHandle.IsValid())
	{
		StageRoot->UnRegisterActiveTimer(StageTimerHandle.ToSharedRef());
	}
	StageTimerHandle.Reset();
}

EActiveTimerReturnType UT66StickPickGameWidget::HandleStageTick(double, const float DeltaTime)
{
	if (bSequenceActive)
	{
		TArray<FT66AnimationMarkerEvent> MarkerEvents;
		ActiveSequence.Tick(FMath::Max(0.f, DeltaTime), MarkerEvents);
		if (MarkerEvents.Num() > 0)
		{
			HandleSequenceMarkers(MarkerEvents);
		}
		if (T66Animation::IsTerminalState(ActiveSequence.GetState()))
		{
			bSequenceActive = false;
		}
	}

	ApplyStageTransforms();
	return EActiveTimerReturnType::Continue;
}

// ============================================================================
// FIND THE JOKER
// ============================================================================

namespace JokerStage
{
	constexpr float CardWidth = 100.f;
	constexpr float CardHeight = 140.f;
	constexpr float ColumnPitch = 112.f;
	constexpr float RowPitch = 156.f;
	constexpr float DealDuration = 1.30f;
	constexpr float DealCardWindow = 0.32f;
	constexpr float DealStagger = 0.07f;
}

TSharedRef<SWidget> UT66FindJokerGameWidget::RebuildWidget()
{
	// A host rebuild discards the previous stage; the active timer dies with it.
	ClearStageTimer();

	TSharedRef<SOverlay> StageContent = SNew(SOverlay);

	StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Center)
	[
		SAssignNew(GlowBox, SBox).WidthOverride(220.f).HeightOverride(220.f)
		[
			SNew(SImage).Image(T66GamblerStage::SpriteBrush(TEXT("win_glow.png"), FVector2D(220.f, 220.f)))
		]
	];

	for (int32 Index = 0; Index < CardCount; ++Index)
	{
		FCardVisual& Card = Cards[Index];
		TSharedRef<SBorder> CardWidget = SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("NoBrush")))
			.Padding(0.f)
			.OnMouseButtonDown(FPointerEventHandler::CreateLambda(
				[WeakThis = TWeakObjectPtr<UT66FindJokerGameWidget>(this), Index](const FGeometry&, const FPointerEvent&)
				{
					if (UT66FindJokerGameWidget* Self = WeakThis.Get())
					{
						return Self->OnCardClicked(Index);
					}
					return FReply::Unhandled();
				}))
			[
				SNew(SBox).WidthOverride(JokerStage::CardWidth).HeightOverride(JokerStage::CardHeight)
				[
					SAssignNew(Card.Image, SImage)
					.Image(T66GamblerStage::SpriteBrush(TEXT("card_back.png"), FVector2D(JokerStage::CardWidth, JokerStage::CardHeight)))
				]
			];
		Card.Root = CardWidget;

		StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Center)
		[
			CardWidget
		];
	}

	StageContent->AddSlot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 12.f, 0.f, 0.f)
	[
		T66GamblerStage::MakeResultBanner(BannerText)
	];

	TSharedRef<SWidget> Stage = T66GamblerStage::MakeStage(StageContent);
	StageRoot = Stage;

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeLiveCasinoHeader(
				NSLOCTEXT("T66.Gambler", "FindJokerTitle", "FIND THE JOKER"),
				StatusText,
				WagerText,
				FOnClicked::CreateUObject(this, &UT66FindJokerGameWidget::OnBackClicked))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			Stage
		];
}

void UT66FindJokerGameWidget::NativeDestruct()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	ClearStageTimer();
	Super::NativeDestruct();
}

void UT66FindJokerGameWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
	ResetForOpen();
	EnsureStageTimer();
}

void UT66FindJokerGameWidget::DeactivateWidgetGame()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	bChoiceEnabled = false;
	bRoundArmed = false;
	ClearStageTimer();
	WidgetGameHostContext = FT66WidgetGameHostContext();
}

void UT66FindJokerGameWidget::RequestWidgetGameExit()
{
	if (ReturnCallback)
	{
		ReturnCallback();
	}
	else
	{
		WidgetGameHostContext.RequestExit(ET66WidgetGameExitReason::PlayerCancelled);
	}
}

void UT66FindJokerGameWidget::SetChoiceCallback(TFunction<void(int32)> InChoiceCallback)
{
	ChoiceCallback = MoveTemp(InChoiceCallback);
}

void UT66FindJokerGameWidget::SetReturnCallback(TFunction<void()> InReturnCallback)
{
	ReturnCallback = MoveTemp(InReturnCallback);
}

void UT66FindJokerGameWidget::SetRevealCompleteCallback(TFunction<void()> InRevealCompleteCallback)
{
	RevealCompleteCallback = MoveTemp(InRevealCompleteCallback);
}

FVector2D UT66FindJokerGameWidget::DealOffsetForCard(const int32 CardIndex) const
{
	const int32 Column = CardIndex % CardColumns;
	const int32 Row = CardIndex / CardColumns;
	const float BaseX = (Column - 2) * JokerStage::ColumnPitch;
	const float BaseY = (Row == 0 ? -1.f : 1.f) * (JokerStage::RowPitch * 0.5f);
	return FVector2D(-BaseX, -260.f - BaseY);
}

void UT66FindJokerGameWidget::ResetForOpen()
{
	ActiveSequence.Cancel();
	bSequenceActive = false;
	bChoiceEnabled = false;
	bRoundArmed = false;
	RevealChosenCard = INDEX_NONE;
	RevealJokerCard = INDEX_NONE;
	RevealPayoutGold = 0;
	GlowOpacity = 0.f;
	GlowScale = 1.f;
	BannerOpacity = 0.f;
	BannerScale = 1.f;

	for (int32 Index = 0; Index < CardCount; ++Index)
	{
		FCardVisual& Card = Cards[Index];
		const int32 Column = Index % CardColumns;
		const int32 Row = Index / CardColumns;
		Card.Translation = FVector2D(
			(Column - 2) * JokerStage::ColumnPitch,
			(Row == 0 ? -1.f : 1.f) * (JokerStage::RowPitch * 0.5f));
		Card.Scale = FVector2D(1.f, 1.f);
		Card.AngleDeg = 0.f;
		Card.Opacity = 0.f;
		Card.HoverLift = 0.f;
		Card.bDealt = false;
		SetCardFace(Index, ECardFace::Back);
		if (Card.Image.IsValid())
		{
			Card.Image->SetColorAndOpacity(FLinearColor::White);
		}
	}

	if (BannerText.IsValid())
	{
		BannerText->SetText(FText::GetEmpty());
	}
	SetStatus(FText::GetEmpty(), FT66FlatStyle::Tokens::Text);
	BuildDealSequence();
	ApplyStageTransforms();
	EnsureStageTimer();
}

void UT66FindJokerGameWidget::NotifyRoundArmed()
{
	bRoundArmed = true;
	EnsureStageTimer();
}

void UT66FindJokerGameWidget::SetStatus(const FText& Message, const FLinearColor& Color)
{
	if (StatusText.IsValid())
	{
		StatusText->SetText(Message);
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UT66FindJokerGameWidget::SetWagerAmount(const int32 WagerAmount)
{
	if (WagerText.IsValid())
	{
		WagerText->SetText(BuildLiveCasinoWagerText(WagerAmount));
	}
}

void UT66FindJokerGameWidget::SetCardFace(const int32 CardIndex, const ECardFace Face)
{
	FCardVisual& Card = Cards[CardIndex];
	Card.Face = Face;
	if (!Card.Image.IsValid())
	{
		return;
	}

	const TCHAR* FileName = TEXT("card_back.png");
	switch (Face)
	{
	case ECardFace::Joker: FileName = TEXT("card_joker.png"); break;
	case ECardFace::Blank: FileName = TEXT("card_blank.png"); break;
	case ECardFace::Back:
	default: break;
	}
	Card.Image->SetImage(T66GamblerStage::SpriteBrush(FileName, FVector2D(JokerStage::CardWidth, JokerStage::CardHeight)));
}

void UT66FindJokerGameWidget::BuildDealSequence()
{
	ActiveSequence.Cancel();
	ActiveSequence = FT66AnimationSequence();
	TWeakObjectPtr<UT66FindJokerGameWidget> WeakThis(this);

	FT66AnimationTimeline Deal = MakeStageTimeline(TEXT("FindJoker.Deal"), JokerStage::DealDuration,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](const float Value)
		{
			if (UT66FindJokerGameWidget* Self = WeakThis.Get())
			{
				const float Elapsed = FMath::Clamp(Value, 0.f, 1.f) * JokerStage::DealDuration;
				for (int32 Index = 0; Index < CardCount; ++Index)
				{
					FCardVisual& Card = Self->Cards[Index];
					const float Start = Index * JokerStage::DealStagger;
					const float Local = FMath::Clamp((Elapsed - Start) / JokerStage::DealCardWindow, 0.f, 1.f);
					const float Eased = 1.f - FMath::Pow(1.f - Local, 3.f);
					const FVector2D DealOffset = Self->DealOffsetForCard(Index);
					const int32 Column = Index % CardColumns;
					const int32 Row = Index / CardColumns;
					const FVector2D Home(
						(Column - 2) * JokerStage::ColumnPitch,
						(Row == 0 ? -1.f : 1.f) * (JokerStage::RowPitch * 0.5f));
					Card.Translation = Home + DealOffset * (1.f - Eased);
					Card.Opacity = FMath::Clamp(Local * 3.f, 0.f, 1.f);
					Card.AngleDeg = -10.f * (1.f - Eased);
					Card.bDealt = Local >= 1.f;
				}
			}
		});
	Deal.AddMarker({ FName(TEXT("DealStart")), ET66AnimationMarkerType::ProgressBased, 0.01f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	Deal.AddMarker({ FName(TEXT("DealTick")), ET66AnimationMarkerType::ProgressBased, 0.30f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	Deal.AddMarker({ FName(TEXT("DealTick")), ET66AnimationMarkerType::ProgressBased, 0.60f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	Deal.AddMarker({ FName(TEXT("DealTick")), ET66AnimationMarkerType::ProgressBased, 0.90f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(Deal));

	FT66AnimationTimeline Settle = MakeStageTimeline(TEXT("FindJoker.Settle"), 0.25f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis](const float Value)
		{
			if (UT66FindJokerGameWidget* Self = WeakThis.Get())
			{
				const float Pulse = 1.f + 0.03f * FMath::Sin(FMath::Clamp(Value, 0.f, 1.f) * PI);
				for (FCardVisual& Card : Self->Cards)
				{
					Card.Scale = FVector2D(Pulse, Pulse);
					Card.AngleDeg = 0.f;
				}
			}
		});
	Settle.AddMarker({ FName(TEXT("DealDone")), ET66AnimationMarkerType::ProgressBased, 1.f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(Settle));

	bSequenceActive = true;
	ActiveSequence.Play();
}

void UT66FindJokerGameWidget::RevealResult(const int32 ChosenCard, const int32 JokerCard, const int32 PayoutGold)
{
	RevealChosenCard = FMath::Clamp(ChosenCard, 0, CardCount - 1);
	RevealJokerCard = FMath::Clamp(JokerCard, 0, CardCount - 1);
	RevealPayoutGold = PayoutGold;
	bChoiceEnabled = false;
	bRoundArmed = false;

	if (BannerText.IsValid())
	{
		BannerText->SetText(PayoutGold > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "JokerWinFormat", "Joker on card {0}. WIN (+{1})"), FText::AsNumber(JokerCard + 1), FText::AsNumber(PayoutGold))
			: FText::Format(NSLOCTEXT("T66.Gambler", "JokerLoseFormat", "Joker on card {0}. LOSE"), FText::AsNumber(JokerCard + 1)));
		BannerText->SetColorAndOpacity(PayoutGold > 0 ? T66GamblerStage::WinGold() : T66GamblerStage::LoseRed());
	}

	BuildRevealSequence();
	EnsureStageTimer();
}

void UT66FindJokerGameWidget::BuildRevealSequence()
{
	ActiveSequence.Cancel();
	ActiveSequence = FT66AnimationSequence();

	// If the deal was still running, snap every card home first.
	for (int32 Index = 0; Index < CardCount; ++Index)
	{
		FCardVisual& Card = Cards[Index];
		const int32 Column = Index % CardColumns;
		const int32 Row = Index / CardColumns;
		Card.Translation = FVector2D(
			(Column - 2) * JokerStage::ColumnPitch,
			(Row == 0 ? -1.f : 1.f) * (JokerStage::RowPitch * 0.5f));
		Card.Scale = FVector2D(1.f, 1.f);
		Card.AngleDeg = 0.f;
		Card.Opacity = 1.f;
		Card.HoverLift = 0.f;
		Card.bDealt = true;
	}

	const bool bWin = RevealPayoutGold > 0;
	const int32 Chosen = RevealChosenCard;
	const int32 Joker = RevealJokerCard;
	TWeakObjectPtr<UT66FindJokerGameWidget> WeakThis(this);

	auto FlipCard = [](UT66FindJokerGameWidget* Self, const int32 Index, const float Value)
	{
		FCardVisual& Card = Self->Cards[Index];
		const float P = FMath::Clamp(Value, 0.f, 1.f);
		const float Cos = FMath::Cos(PI * P);
		const float Pop = 1.f + 0.12f * FMath::Sin(P * PI);
		Card.Scale = FVector2D(FMath::Max(FMath::Abs(Cos), 0.04f) * Pop, Pop);
		Card.Translation.Y += 0.f;
	};

	FT66AnimationTimeline FlipChosen = MakeStageTimeline(TEXT("FindJoker.FlipChosen"), 0.5f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis, Chosen, FlipCard](const float Value)
		{
			if (UT66FindJokerGameWidget* Self = WeakThis.Get())
			{
				FlipCard(Self, Chosen, Value);
				Self->Cards[Chosen].HoverLift = -12.f * FMath::Clamp(Value * 3.f, 0.f, 1.f);
			}
		});
	FlipChosen.AddMarker({ FName(TEXT("CardFlip")), ET66AnimationMarkerType::ProgressBased, 0.02f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	FlipChosen.AddMarker({ FName(TEXT("ChosenFaceSwap")), ET66AnimationMarkerType::ProgressBased, 0.5f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(FlipChosen));

	if (bWin)
	{
		FT66AnimationTimeline WinGlow = MakeStageTimeline(TEXT("FindJoker.WinGlow"), 0.35f,
			FT66AnimationCurveSpec(ET66AnimationCurve::EaseOutCubic),
			[WeakThis, Chosen](const float Value)
			{
				if (UT66FindJokerGameWidget* Self = WeakThis.Get())
				{
					const FVector2D Pos = Self->Cards[Chosen].Translation;
					Self->GlowTranslateX = Pos.X;
					Self->GlowTranslateY = Pos.Y;
					Self->GlowOpacity = 0.75f * FMath::Clamp(Value, 0.f, 1.f);
					Self->GlowScale = 0.85f + 0.25f * Value;
				}
			});
		ActiveSequence.AddTimeline(MoveTemp(WinGlow));
	}
	else
	{
		FT66AnimationTimeline Beat = MakeStageTimeline(TEXT("FindJoker.Beat"), 0.30f,
			FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
			[WeakThis](const float)
			{
			});
		Beat.AddMarker({ FName(TEXT("ChosenLoseTint")), ET66AnimationMarkerType::ProgressBased, 0.5f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
		ActiveSequence.AddTimeline(MoveTemp(Beat));

		FT66AnimationTimeline FlipJoker = MakeStageTimeline(TEXT("FindJoker.FlipJoker"), 0.45f,
			FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
			[WeakThis, Joker, Chosen, FlipCard](const float Value)
			{
				if (UT66FindJokerGameWidget* Self = WeakThis.Get())
				{
					FlipCard(Self, Joker, Value);
					const FVector2D Pos = Self->Cards[Joker].Translation;
					Self->GlowTranslateX = Pos.X;
					Self->GlowTranslateY = Pos.Y;
					Self->GlowOpacity = 0.45f * FMath::Clamp(Value, 0.f, 1.f);
					Self->GlowScale = 0.85f + 0.2f * Value;
					for (int32 Index = 0; Index < CardCount; ++Index)
					{
						if (Index != Joker && Index != Chosen)
						{
							Self->Cards[Index].Opacity = FMath::Lerp(1.f, 0.55f, FMath::Clamp(Value, 0.f, 1.f));
						}
					}
				}
			});
		FlipJoker.AddMarker({ FName(TEXT("CardFlip")), ET66AnimationMarkerType::ProgressBased, 0.02f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
		FlipJoker.AddMarker({ FName(TEXT("JokerFaceSwap")), ET66AnimationMarkerType::ProgressBased, 0.5f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
		ActiveSequence.AddTimeline(MoveTemp(FlipJoker));
	}

	FT66AnimationTimeline Banner = MakeStageTimeline(TEXT("FindJoker.Banner"), 0.35f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Overshoot, 0.85f),
		[WeakThis](const float Value)
		{
			if (UT66FindJokerGameWidget* Self = WeakThis.Get())
			{
				Self->BannerOpacity = FMath::Clamp(Value, 0.f, 1.f);
				Self->BannerScale = 0.6f + 0.4f * Value;
			}
		});
	Banner.AddMarker({ FName(TEXT("RevealComplete")), ET66AnimationMarkerType::ProgressBased, 0.65f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	ActiveSequence.AddTimeline(MoveTemp(Banner));

	FT66AnimationTimeline Hold = MakeStageTimeline(TEXT("FindJoker.Hold"), 0.45f,
		FT66AnimationCurveSpec(ET66AnimationCurve::Linear),
		[WeakThis, bWin](const float Value)
		{
			if (UT66FindJokerGameWidget* Self = WeakThis.Get())
			{
				if (bWin)
				{
					Self->GlowScale = 1.10f + 0.05f * FMath::Sin(Value * 2.f * PI);
				}
			}
		});
	ActiveSequence.AddTimeline(MoveTemp(Hold));

	bSequenceActive = true;
	ActiveSequence.Play();
}

void UT66FindJokerGameWidget::HandleSequenceMarkers(const TArray<FT66AnimationMarkerEvent>& MarkerEvents)
{
	for (const FT66AnimationMarkerEvent& Event : MarkerEvents)
	{
		if (Event.MarkerID == FName(TEXT("DealStart")))
		{
			T66GamblerStage::PlayUISound(TEXT("Casino.CardDeal"));
		}
		else if (Event.MarkerID == FName(TEXT("DealTick")))
		{
			T66GamblerStage::PlayUISound(TEXT("Casino.CupShuffle"));
		}
		else if (Event.MarkerID == FName(TEXT("DealDone")))
		{
			bChoiceEnabled = true;
		}
		else if (Event.MarkerID == FName(TEXT("CardFlip")))
		{
			T66GamblerStage::PlayUISound(TEXT("Casino.CardFlip"));
		}
		else if (Event.MarkerID == FName(TEXT("ChosenFaceSwap")))
		{
			SetCardFace(RevealChosenCard, RevealPayoutGold > 0 ? ECardFace::Joker : ECardFace::Blank);
		}
		else if (Event.MarkerID == FName(TEXT("JokerFaceSwap")))
		{
			SetCardFace(RevealJokerCard, ECardFace::Joker);
		}
		else if (Event.MarkerID == FName(TEXT("ChosenLoseTint")))
		{
			if (Cards[RevealChosenCard].Image.IsValid())
			{
				Cards[RevealChosenCard].Image->SetColorAndOpacity(T66GamblerStage::DimTint());
			}
		}
		else if (Event.MarkerID == FName(TEXT("RevealComplete")))
		{
			if (RevealCompleteCallback)
			{
				RevealCompleteCallback();
			}
		}
	}
}

FReply UT66FindJokerGameWidget::OnBackClicked()
{
	RequestWidgetGameExit();
	return FReply::Handled();
}

FReply UT66FindJokerGameWidget::OnCardClicked(const int32 CardIndex)
{
	if (!bChoiceEnabled || bSequenceActive)
	{
		return FReply::Handled();
	}

	bChoiceEnabled = false;
	if (ChoiceCallback)
	{
		ChoiceCallback(CardIndex);
	}
	return FReply::Handled();
}

void UT66FindJokerGameWidget::ApplyStageTransforms()
{
	for (int32 Index = 0; Index < CardCount; ++Index)
	{
		FCardVisual& Card = Cards[Index];
		T66GamblerStage::ApplySpriteTransform(
			Card.Root,
			Card.Translation + FVector2D(0.f, Card.HoverLift),
			Card.Scale,
			Card.AngleDeg);
		if (Card.Root.IsValid())
		{
			Card.Root->SetRenderOpacity(Card.Opacity);
		}
	}

	T66GamblerStage::ApplySpriteTransform(GlowBox, FVector2D(GlowTranslateX, GlowTranslateY), FVector2D(GlowScale, GlowScale));
	if (GlowBox.IsValid())
	{
		GlowBox->SetRenderOpacity(GlowOpacity);
	}
	if (BannerText.IsValid())
	{
		BannerText->SetRenderOpacity(BannerOpacity);
		T66GamblerStage::ApplySpriteTransform(BannerText, FVector2D::ZeroVector, FVector2D(BannerScale, BannerScale));
	}
}

void UT66FindJokerGameWidget::EnsureStageTimer()
{
	if (!StageRoot.IsValid() || StageTimerHandle.IsValid())
	{
		return;
	}

	StageTimerHandle = StageRoot->RegisterActiveTimer(
		1.f / 60.f,
		FWidgetActiveTimerDelegate::CreateUObject(this, &UT66FindJokerGameWidget::HandleStageTick));
}

void UT66FindJokerGameWidget::ClearStageTimer()
{
	if (StageRoot.IsValid() && StageTimerHandle.IsValid())
	{
		StageRoot->UnRegisterActiveTimer(StageTimerHandle.ToSharedRef());
	}
	StageTimerHandle.Reset();
}

EActiveTimerReturnType UT66FindJokerGameWidget::HandleStageTick(double, const float DeltaTime)
{
	if (bSequenceActive)
	{
		TArray<FT66AnimationMarkerEvent> MarkerEvents;
		ActiveSequence.Tick(FMath::Max(0.f, DeltaTime), MarkerEvents);
		if (MarkerEvents.Num() > 0)
		{
			HandleSequenceMarkers(MarkerEvents);
		}
		if (T66Animation::IsTerminalState(ActiveSequence.GetState()))
		{
			bSequenceActive = false;
		}
	}

	for (FCardVisual& Card : Cards)
	{
		if (!bSequenceActive && bChoiceEnabled && Card.Face == ECardFace::Back && Card.Root.IsValid())
		{
			const float Target = Card.Root->IsHovered() ? -10.f : 0.f;
			Card.HoverLift = FMath::FInterpTo(Card.HoverLift, Target, DeltaTime, 14.f);
		}
	}

	ApplyStageTransforms();
	return EActiveTimerReturnType::Continue;
}
