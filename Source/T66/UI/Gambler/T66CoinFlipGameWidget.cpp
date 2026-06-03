// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Gambler/T66CoinFlipGameWidget.h"

#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
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
	UT66LocalizationSubsystem* Loc = ResolveCoinFlipLocalization(this);
	UT66UITexturePoolSubsystem* TexPool = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			TexPool = GI->GetSubsystem<UT66UITexturePoolSubsystem>();
		}
	}

	const FTextBlockStyle& TextHeading = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading"));
	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));
	static constexpr float CoinSpriteSize = 180.f;

	auto InitCoinBrush = [](FSlateBrush& Brush, const float Size)
	{
		Brush = FSlateBrush();
		Brush.ImageSize = FVector2D(Size, Size);
		Brush.DrawAs = ESlateBrushDrawType::Image;
	};

	InitCoinBrush(CoinBrushHeads, CoinSpriteSize);
	InitCoinBrush(CoinBrushTails, CoinSpriteSize);
	InitCoinBrush(CoinBrushSide, CoinSpriteSize);

	if (TexPool)
	{
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/Coin/Heads.Heads"))), this, CoinBrushHeads, FName(TEXT("CoinFlipGameHeads")), true);
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/Coin/TAILS.TAILS"))), this, CoinBrushTails, FName(TEXT("CoinFlipGameTails")), true);
		T66SlateTexture::BindBrushAsync(TexPool, TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Game/UI/Sprites/Games/Coin/SIDE.SIDE"))), this, CoinBrushSide, FName(TEXT("CoinFlipGameSide")), true);
	}

	return SNew(SVerticalBox)
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
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 10.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_CoinFlip() : FText::GetEmpty())
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(STextBlock)
			.Text(Loc ? Loc->GetText_ChooseHeadsOrTails() : FText::GetEmpty())
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill).Padding(0.f, 0.f, 0.f, 16.f)
		[
			SNew(SBox)
			.HeightOverride(220.f)
			[
				FT66FlatStyle::MakePanel(
					SNew(SOverlay)
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						SNew(SBox).WidthOverride(CoinSpriteSize).HeightOverride(CoinSpriteSize)
						[
							SAssignNew(CoinImage, SImage)
							.Image(&CoinBrushHeads)
						]
					]
					+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.f, 0.f, 0.f, 6.f)
					[
						SAssignNew(ResultText, STextBlock)
						.Text(Loc ? Loc->GetText_ResultDash() : FText::GetEmpty())
						.TextStyle(&TextHeading)
						.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
					],
					FT66PanelParams(ET66PanelType::Panel2).SetPadding(FT66FlatStyle::Tokens::Space6))
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Heads() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66CoinFlipGameWidget::OnHeadsClicked),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10.f, 0.f)
			[
				FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
					Loc ? Loc->GetText_Tails() : FText::GetEmpty(),
					FOnClicked::CreateUObject(this, &UT66CoinFlipGameWidget::OnTailsClicked),
					ET66ButtonType::Primary)
					.SetMinWidth(0.f)
					.SetPadding(FMargin(18.f, 10.f)))
			]
		];
}

void UT66CoinFlipGameWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
	}

	Super::NativeDestruct();
}

void UT66CoinFlipGameWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
	ResetForOpen();
}

void UT66CoinFlipGameWidget::DeactivateWidgetGame()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
	}
	WidgetGameHostContext = FT66WidgetGameHostContext();
	bCoinSpinActive = false;
	CoinSpinLastTickTimeSeconds = 0.f;
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

void UT66CoinFlipGameWidget::ResetForOpen()
{
	UT66LocalizationSubsystem* Loc = ResolveCoinFlipLocalization(this);
	SetCoinFace(ECoinFace::Heads);
	SetStatus(FText::GetEmpty(), FT66FlatStyle::Tokens::Text);
	SetResultText(Loc ? Loc->GetText_ResultDash() : NSLOCTEXT("T66.Gambler", "ResultDash", "Result: -"));
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

void UT66CoinFlipGameWidget::SetResultText(const FText& InResultText)
{
	if (ResultText.IsValid())
	{
		ResultText->SetText(InResultText);
	}
}

void UT66CoinFlipGameWidget::StartSpin(const bool bResultHeads, const float DurationSeconds)
{
	bCoinSpinResultHeads = bResultHeads;
	bCoinSpinActive = true;
	CoinSpinElapsed = 0.f;
	CoinSpinDuration = FMath::Max(0.01f, DurationSeconds);
	CoinSpinLastTickTimeSeconds = 0.f;

	if (UWorld* World = GetWorld())
	{
		CoinSpinLastTickTimeSeconds = static_cast<float>(World->GetTimeSeconds());
		World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
		World->GetTimerManager().SetTimer(CoinSpinTimerHandle, this, &UT66CoinFlipGameWidget::TickCoinSpin, 1.f / 15.f, true);
	}
}

FReply UT66CoinFlipGameWidget::OnBackClicked()
{
	RequestWidgetGameExit();
	return FReply::Handled();
}

FReply UT66CoinFlipGameWidget::OnHeadsClicked()
{
	if (ChoiceCallback)
	{
		ChoiceCallback(true);
	}
	return FReply::Handled();
}

FReply UT66CoinFlipGameWidget::OnTailsClicked()
{
	if (ChoiceCallback)
	{
		ChoiceCallback(false);
	}
	return FReply::Handled();
}

void UT66CoinFlipGameWidget::TickCoinSpin()
{
	if (!bCoinSpinActive)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = static_cast<float>(World->GetTimeSeconds());
	float Delta = CoinSpinLastTickTimeSeconds > 0.f ? Now - CoinSpinLastTickTimeSeconds : 1.f / 15.f;
	CoinSpinLastTickTimeSeconds = Now;
	Delta = FMath::Clamp(Delta, 0.f, 0.10f);
	CoinSpinElapsed += Delta;
	const float Alpha = FMath::Clamp(CoinSpinElapsed / CoinSpinDuration, 0.f, 1.f);

	if (Alpha >= 1.f)
	{
		FinishCoinSpin();
		return;
	}

	const float Ease = FMath::InterpEaseOut(0.f, 1.f, Alpha, 3.f);
	const float TotalHalfFlips = 12.f;
	const float CurrentHalfFlip = TotalHalfFlips * Ease;
	const int32 Phase = FMath::FloorToInt(CurrentHalfFlip * 2.f);
	const int32 Step = Phase % 4;
	switch (Step)
	{
	case 0: SetCoinFace(ECoinFace::Heads); break;
	case 1: SetCoinFace(ECoinFace::Side); break;
	case 2: SetCoinFace(ECoinFace::Tails); break;
	case 3: SetCoinFace(ECoinFace::Side); break;
	default: break;
	}
}

void UT66CoinFlipGameWidget::FinishCoinSpin()
{
	bCoinSpinActive = false;
	CoinSpinLastTickTimeSeconds = 0.f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CoinSpinTimerHandle);
	}
	SetCoinFace(bCoinSpinResultHeads ? ECoinFace::Heads : ECoinFace::Tails);
}

void UT66CoinFlipGameWidget::SetCoinFace(const ECoinFace Face)
{
	if (!CoinImage.IsValid())
	{
		return;
	}

	switch (Face)
	{
	case ECoinFace::Heads:
		CoinImage->SetImage(&CoinBrushHeads);
		break;
	case ECoinFace::Tails:
		CoinImage->SetImage(&CoinBrushTails);
		break;
	case ECoinFace::Side:
		CoinImage->SetImage(&CoinBrushSide);
		break;
	default:
		break;
	}
}
