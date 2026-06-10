// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Gambler/T66CoinFlipGameWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/Animation/T66AnimationCurves.h"
#include "UI/Gambler/T66GamblerGameStage.h"
#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float CoinSize = 190.f;
	constexpr float CoinFlightHeight = 185.f;
	constexpr float CoinRestBottomPadding = 44.f;

	static FText BuildCoinFlipWagerText(const int32 WagerAmount)
	{
		return WagerAmount > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(WagerAmount))
			: FText::GetEmpty();
	}

	static UT66LocalizationSubsystem* ResolveCoinFlipLocalization(const UUserWidget* Widget)
	{
		if (!Widget)
		{
			return nullptr;
		}

		if (const UWorld* World = Widget->GetWorld())
		{
			if (UGameInstance* GI = World->GetGameInstance())
			{
				return GI->GetSubsystem<UT66LocalizationSubsystem>();
			}
		}

		return nullptr;
	}
}

TSharedRef<SWidget> UT66CoinFlipGameWidget::RebuildWidget()
{
	// A host rebuild discards the previous stage; the active timer dies with it.
	ClearStageTimer();

	UT66LocalizationSubsystem* Loc = ResolveCoinFlipLocalization(this);
	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));

	TSharedRef<SWidget> Stage = T66GamblerStage::MakeStage(
		SNew(SOverlay)
		// Win glow behind the coin rest position.
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom)
		[
			SAssignNew(GlowBox, SBox).WidthOverride(320.f).HeightOverride(320.f)
			[
				SNew(SImage).Image(T66GamblerStage::SpriteBrush(TEXT("win_glow.png"), FVector2D(320.f, 320.f)))
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 30.f)
		[
			SAssignNew(ShadowBox, SBox).WidthOverride(200.f).HeightOverride(48.f)
			[
				SNew(SImage).Image(T66GamblerStage::SpriteBrush(TEXT("shadow_soft.png"), FVector2D(200.f, 48.f)))
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, CoinRestBottomPadding)
		[
			SAssignNew(CoinBox, SBox).WidthOverride(CoinSize).HeightOverride(CoinSize)
			[
				SAssignNew(CoinImage, SImage)
				.Image(T66GamblerStage::SpriteBrush(TEXT("coin_heads.png"), FVector2D(CoinSize, CoinSize)))
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 30.f)
		[
			SAssignNew(BurstBox, SBox).WidthOverride(220.f).HeightOverride(220.f)
			[
				SNew(SImage).Image(T66GamblerStage::SpriteBrush(TEXT("ember_burst.png"), FVector2D(220.f, 220.f)))
			]
		]
		+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0.f, 18.f, 0.f, 0.f)
		[
			T66GamblerStage::MakeResultBanner(BannerText)
		]);

	StageRoot = Stage;

	TSharedRef<SWidget> Root = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Gambler", "Back", "BACK"),
					FOnClicked::CreateUObject(this, &UT66CoinFlipGameWidget::OnBackClicked),
					ET66ButtonType::Neutral).SetMinWidth(0.f).SetPadding(FMargin(12.f, 8.f)))
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
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 12.f)
		[
			Stage
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Heads() : NSLOCTEXT("T66.Gambler", "Heads", "HEADS"),
					FOnClicked::CreateUObject(this, &UT66CoinFlipGameWidget::OnHeadsClicked),
					ET66ButtonType::Primary)
					.SetMinWidth(150.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Tails() : NSLOCTEXT("T66.Gambler", "Tails", "TAILS"),
					FOnClicked::CreateUObject(this, &UT66CoinFlipGameWidget::OnTailsClicked),
					ET66ButtonType::Primary)
					.SetMinWidth(150.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
		];

	return Root;
}

void UT66CoinFlipGameWidget::NativeDestruct()
{
	SpinSequence.Cancel();
	bSequenceActive = false;
	ClearStageTimer();
	Super::NativeDestruct();
}

void UT66CoinFlipGameWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
	ResetForOpen();
	EnsureStageTimer();
}

void UT66CoinFlipGameWidget::DeactivateWidgetGame()
{
	SpinSequence.Cancel();
	bSequenceActive = false;
	bRoundArmed = false;
	ClearStageTimer();
	WidgetGameHostContext = FT66WidgetGameHostContext();
}

void UT66CoinFlipGameWidget::RequestWidgetGameExit()
{
	if (ReturnCallback)
	{
		ReturnCallback();
	}
	else if (WidgetGameHostContext.ReturnNavigationCallback)
	{
		WidgetGameHostContext.ReturnNavigationCallback(ET66WidgetGameExitReason::PlayerCancelled);
	}
}

void UT66CoinFlipGameWidget::SetChoiceCallback(TFunction<void(bool)> InChoiceCallback)
{
	ChoiceCallback = MoveTemp(InChoiceCallback);
}

void UT66CoinFlipGameWidget::SetReturnCallback(TFunction<void()> InReturnCallback)
{
	ReturnCallback = MoveTemp(InReturnCallback);
}

void UT66CoinFlipGameWidget::SetRevealCompleteCallback(TFunction<void()> InRevealCompleteCallback)
{
	RevealCompleteCallback = MoveTemp(InRevealCompleteCallback);
}

void UT66CoinFlipGameWidget::ResetForOpen()
{
	SpinSequence.Cancel();
	bSequenceActive = false;
	bRoundArmed = false;
	IdleTimeSeconds = 0.f;

	CoinTranslateY = 0.f;
	CoinScaleX = 1.f;
	CoinScaleY = 1.f;
	CoinAngleDeg = 0.f;
	ShadowScale = 1.f;
	ShadowOpacity = 0.85f;
	GlowOpacity = 0.f;
	GlowScale = 1.f;
	BurstOpacity = 0.f;
	BurstScale = 1.f;
	BannerOpacity = 0.f;
	BannerScale = 1.f;

	SetCoinFace(ECoinFace::Heads);
	if (CoinImage.IsValid())
	{
		CoinImage->SetColorAndOpacity(FLinearColor::White);
	}
	if (BannerText.IsValid())
	{
		BannerText->SetText(FText::GetEmpty());
	}
	SetStatus(FText::GetEmpty(), FT66FlatStyle::Tokens::Text);
	ApplyStageTransforms();
	EnsureStageTimer();
}

void UT66CoinFlipGameWidget::NotifyRoundArmed()
{
	bRoundArmed = true;
	IdleTimeSeconds = 0.f;
	EnsureStageTimer();
}

void UT66CoinFlipGameWidget::SetStatus(const FText& Message, const FLinearColor& Color)
{
	if (StatusText.IsValid())
	{
		StatusText->SetText(Message);
		StatusText->SetColorAndOpacity(FSlateColor(Color));
	}
}

void UT66CoinFlipGameWidget::SetWagerAmount(const int32 WagerAmount)
{
	if (WagerText.IsValid())
	{
		WagerText->SetText(BuildCoinFlipWagerText(WagerAmount));
	}
}

void UT66CoinFlipGameWidget::StartSpin(const bool bResultHeads, const bool bWin, const int32 PayoutGold)
{
	bSpinResultHeads = bResultHeads;
	bSpinWin = bWin;
	SpinPayoutGold = PayoutGold;
	bRoundArmed = false;

	if (BannerText.IsValid())
	{
		const FText FaceText = bResultHeads
			? NSLOCTEXT("T66.Gambler", "Heads", "Heads")
			: NSLOCTEXT("T66.Gambler", "Tails", "Tails");
		BannerText->SetText(FText::Format(
			bWin
				? NSLOCTEXT("T66.Gambler", "CoinFlipWinFmt", "{0}. WIN (+{1})")
				: NSLOCTEXT("T66.Gambler", "CoinFlipLoseFmt", "{0}. LOSE"),
			FaceText,
			FText::AsNumber(PayoutGold)));
		BannerText->SetColorAndOpacity(bWin ? T66GamblerStage::WinGold() : T66GamblerStage::LoseRed());
	}

	BuildSpinSequence();
	EnsureStageTimer();
}

void UT66CoinFlipGameWidget::BuildSpinSequence()
{
	SpinSequence.Cancel();
	SpinSequence = FT66AnimationSequence();

	TWeakObjectPtr<UT66CoinFlipGameWidget> WeakThis(this);
	const bool bWin = bSpinWin;
	const bool bResultHeads = bSpinResultHeads;
	// Even half-flip count lands back on heads, odd lands on tails (spin starts heads-up).
	const float TotalHalfFlips = bResultHeads ? 10.f : 11.f;

	FT66AnimationTimeline Anticipation(FName(TEXT("CoinFlip.Anticipation")));
	Anticipation.SetDuration(0.22f);
	Anticipation.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::Linear));
	Anticipation.SetProgressCallback([WeakThis](const float Value)
	{
		if (UT66CoinFlipGameWidget* Self = WeakThis.Get())
		{
			const float Squash = FMath::Sin(FMath::Clamp(Value, 0.f, 1.f) * PI);
			Self->CoinScaleY = 1.f - 0.16f * Squash;
			Self->CoinScaleX = 1.f + 0.08f * Squash;
			Self->CoinTranslateY = 9.f * Squash;
		}
	});
	Anticipation.AddMarker({ FName(TEXT("CoinToss")), ET66AnimationMarkerType::ProgressBased, 0.85f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	SpinSequence.AddTimeline(MoveTemp(Anticipation));

	FT66AnimationTimeline Flight(FName(TEXT("CoinFlip.Flight")));
	Flight.SetDuration(1.45f);
	Flight.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::Linear));
	Flight.SetProgressCallback([WeakThis, TotalHalfFlips](const float Value)
	{
		if (UT66CoinFlipGameWidget* Self = WeakThis.Get())
		{
			const float P = FMath::Clamp(Value, 0.f, 1.f);
			const float Height = CoinFlightHeight * 4.f * P * (1.f - P);
			Self->CoinTranslateY = -Height;

			// Spin decelerates over the flight so the final settle is readable.
			const float Flips = TotalHalfFlips * (1.f - FMath::Square(1.f - P));
			const float Cos = FMath::Cos(PI * Flips);
			const float AbsCos = FMath::Abs(Cos);
			if (AbsCos < 0.20f)
			{
				Self->SetCoinFace(ECoinFace::Side);
				Self->CoinScaleY = 1.f;
				Self->CoinScaleX = 1.f;
			}
			else
			{
				Self->SetCoinFace(FMath::FloorToInt(Flips) % 2 == 0 ? ECoinFace::Heads : ECoinFace::Tails);
				Self->CoinScaleY = FMath::Max(AbsCos, 0.08f);
				Self->CoinScaleX = 1.f + 0.05f * (1.f - AbsCos);
			}
			Self->CoinAngleDeg = 7.f * FMath::Sin(P * PI);

			const float HeightAlpha = Height / CoinFlightHeight;
			Self->ShadowScale = 1.f - 0.45f * HeightAlpha;
			Self->ShadowOpacity = 0.85f - 0.5f * HeightAlpha;
		}
	});
	SpinSequence.AddTimeline(MoveTemp(Flight));

	FT66AnimationTimeline Bounce(FName(TEXT("CoinFlip.Bounce")));
	Bounce.SetDuration(0.55f);
	Bounce.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::Linear));
	Bounce.SetProgressCallback([WeakThis, bResultHeads](const float Value)
	{
		if (UT66CoinFlipGameWidget* Self = WeakThis.Get())
		{
			const float P = FMath::Clamp(Value, 0.f, 1.f);
			float Height = 0.f;
			if (P < 0.45f)
			{
				Height = 36.f * FMath::Sin(PI * (P / 0.45f));
			}
			else if (P < 0.80f)
			{
				Height = 13.f * FMath::Sin(PI * ((P - 0.45f) / 0.35f));
			}
			Self->SetCoinFace(bResultHeads ? ECoinFace::Heads : ECoinFace::Tails);
			Self->CoinTranslateY = -Height;
			Self->CoinAngleDeg = 0.f;
			const bool bGroundContact = Height < 5.f;
			Self->CoinScaleY = bGroundContact ? 0.88f : 1.f;
			Self->CoinScaleX = bGroundContact ? 1.08f : 1.f;
			if (P > 0.92f)
			{
				Self->CoinScaleY = 1.f;
				Self->CoinScaleX = 1.f;
			}
			Self->ShadowScale = 1.f - 0.30f * (Height / 36.f);
			Self->ShadowOpacity = 0.85f - 0.30f * (Height / 36.f);
			Self->BurstOpacity = FMath::Max(0.f, 1.f - P * 1.7f);
			Self->BurstScale = 0.7f + P * 0.9f;
		}
	});
	Bounce.AddMarker({ FName(TEXT("CoinLand")), ET66AnimationMarkerType::ProgressBased, 0.01f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	SpinSequence.AddTimeline(MoveTemp(Bounce));

	FT66AnimationTimeline Reveal(FName(TEXT("CoinFlip.Reveal")));
	Reveal.SetDuration(0.40f);
	Reveal.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::Overshoot, 0.85f));
	Reveal.SetProgressCallback([WeakThis, bWin](const float Value)
	{
		if (UT66CoinFlipGameWidget* Self = WeakThis.Get())
		{
			Self->BannerOpacity = FMath::Clamp(Value, 0.f, 1.f);
			Self->BannerScale = 0.6f + 0.4f * Value;
			if (bWin)
			{
				Self->GlowOpacity = 0.85f * FMath::Clamp(Value, 0.f, 1.f);
				Self->GlowScale = 0.85f + 0.25f * Value;
			}
			Self->BurstOpacity = 0.f;
		}
	});
	Reveal.AddMarker({ FName(TEXT("RevealStart")), ET66AnimationMarkerType::ProgressBased, 0.01f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	Reveal.AddMarker({ FName(TEXT("RevealComplete")), ET66AnimationMarkerType::ProgressBased, 0.65f, 0.f, ET66AnimationMarkerFirePolicy::Once, NAME_None });
	SpinSequence.AddTimeline(MoveTemp(Reveal));

	FT66AnimationTimeline Hold(FName(TEXT("CoinFlip.Hold")));
	Hold.SetDuration(0.5f);
	Hold.SetCurve(FT66AnimationCurveSpec(ET66AnimationCurve::Linear));
	Hold.SetProgressCallback([WeakThis, bWin](const float Value)
	{
		if (UT66CoinFlipGameWidget* Self = WeakThis.Get())
		{
			if (bWin)
			{
				Self->GlowScale = 1.10f + 0.05f * FMath::Sin(Value * 2.f * PI);
			}
		}
	});
	SpinSequence.AddTimeline(MoveTemp(Hold));

	bSequenceActive = true;
	SpinSequence.Play();
}

void UT66CoinFlipGameWidget::HandleSequenceMarkers(const TArray<FT66AnimationMarkerEvent>& MarkerEvents)
{
	for (const FT66AnimationMarkerEvent& Event : MarkerEvents)
	{
		if (Event.MarkerID == FName(TEXT("CoinToss")))
		{
			T66GamblerStage::PlayUISound(TEXT("Casino.CoinToss"));
		}
		else if (Event.MarkerID == FName(TEXT("CoinLand")))
		{
			T66GamblerStage::PlayUISound(TEXT("Casino.CoinLand"));
		}
		else if (Event.MarkerID == FName(TEXT("RevealStart")))
		{
			if (!bSpinWin && CoinImage.IsValid())
			{
				CoinImage->SetColorAndOpacity(T66GamblerStage::DimTint());
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

FReply UT66CoinFlipGameWidget::OnBackClicked()
{
	RequestWidgetGameExit();
	return FReply::Handled();
}

FReply UT66CoinFlipGameWidget::OnHeadsClicked()
{
	if (!bSequenceActive && ChoiceCallback)
	{
		ChoiceCallback(true);
	}
	return FReply::Handled();
}

FReply UT66CoinFlipGameWidget::OnTailsClicked()
{
	if (!bSequenceActive && ChoiceCallback)
	{
		ChoiceCallback(false);
	}
	return FReply::Handled();
}

void UT66CoinFlipGameWidget::SetCoinFace(const ECoinFace Face)
{
	if (!CoinImage.IsValid() || Face == CurrentFace)
	{
		return;
	}

	CurrentFace = Face;
	switch (Face)
	{
	case ECoinFace::Heads:
		CoinImage->SetImage(T66GamblerStage::SpriteBrush(TEXT("coin_heads.png"), FVector2D(CoinSize, CoinSize)));
		break;
	case ECoinFace::Tails:
		CoinImage->SetImage(T66GamblerStage::SpriteBrush(TEXT("coin_tails.png"), FVector2D(CoinSize, CoinSize)));
		break;
	case ECoinFace::Side:
		CoinImage->SetImage(T66GamblerStage::SpriteBrush(TEXT("coin_edge.png"), FVector2D(CoinSize, CoinSize)));
		break;
	default:
		break;
	}
}

void UT66CoinFlipGameWidget::ApplyStageTransforms()
{
	T66GamblerStage::ApplySpriteTransform(CoinBox, FVector2D(0.f, CoinTranslateY), FVector2D(CoinScaleX, CoinScaleY), CoinAngleDeg);
	T66GamblerStage::ApplySpriteTransform(ShadowBox, FVector2D::ZeroVector, FVector2D(ShadowScale, ShadowScale));
	T66GamblerStage::ApplySpriteTransform(GlowBox, FVector2D::ZeroVector, FVector2D(GlowScale, GlowScale));
	T66GamblerStage::ApplySpriteTransform(BurstBox, FVector2D::ZeroVector, FVector2D(BurstScale, BurstScale));

	if (ShadowBox.IsValid())
	{
		ShadowBox->SetRenderOpacity(ShadowOpacity);
	}
	if (GlowBox.IsValid())
	{
		GlowBox->SetRenderOpacity(GlowOpacity);
	}
	if (BurstBox.IsValid())
	{
		BurstBox->SetRenderOpacity(BurstOpacity);
	}
	if (BannerText.IsValid())
	{
		BannerText->SetRenderOpacity(BannerOpacity);
		T66GamblerStage::ApplySpriteTransform(BannerText, FVector2D::ZeroVector, FVector2D(BannerScale, BannerScale));
	}
}

void UT66CoinFlipGameWidget::EnsureStageTimer()
{
	if (!StageRoot.IsValid() || StageTimerHandle.IsValid())
	{
		return;
	}

	StageTimerHandle = StageRoot->RegisterActiveTimer(
		1.f / 60.f,
		FWidgetActiveTimerDelegate::CreateUObject(this, &UT66CoinFlipGameWidget::HandleStageTick));
}

void UT66CoinFlipGameWidget::ClearStageTimer()
{
	if (StageRoot.IsValid() && StageTimerHandle.IsValid())
	{
		StageRoot->UnRegisterActiveTimer(StageTimerHandle.ToSharedRef());
	}
	StageTimerHandle.Reset();
}

EActiveTimerReturnType UT66CoinFlipGameWidget::HandleStageTick(double, const float DeltaTime)
{
	if (bSequenceActive)
	{
		TArray<FT66AnimationMarkerEvent> MarkerEvents;
		SpinSequence.Tick(FMath::Max(0.f, DeltaTime), MarkerEvents);
		if (MarkerEvents.Num() > 0)
		{
			HandleSequenceMarkers(MarkerEvents);
		}
		if (T66Animation::IsTerminalState(SpinSequence.GetState()))
		{
			bSequenceActive = false;
		}
	}
	else if (bRoundArmed)
	{
		// Gentle idle bob inviting the pick.
		IdleTimeSeconds += DeltaTime;
		CoinTranslateY = -4.f * FMath::Abs(FMath::Sin(IdleTimeSeconds * 2.2f));
		CoinAngleDeg = 2.5f * FMath::Sin(IdleTimeSeconds * 1.6f);
	}

	ApplyStageTransforms();
	return EActiveTimerReturnType::Continue;
}
