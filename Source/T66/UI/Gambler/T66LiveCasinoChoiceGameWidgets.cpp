// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Gambler/T66GuessCupGameWidget.h"
#include "UI/Gambler/T66StickPickGameWidget.h"
#include "UI/Gambler/T66FindJokerGameWidget.h"

#include "UI/Style/T66FlatStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	static FText BuildLiveCasinoWagerText(const int32 WagerAmount)
	{
		return WagerAmount > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "WagerFormat", "Wager: {0}"), FText::AsNumber(WagerAmount))
			: FText::GetEmpty();
	}

	static TSharedRef<SWidget> MakeLiveCasinoButton(const FText& Label, const FOnClicked& OnClicked)
	{
		return FT66FlatStyle::MakeButton(FT66FlatStyle::MakeInRunButtonParams(
			Label,
			OnClicked,
			ET66ButtonType::Primary)
			.SetMinWidth(0.f)
			.SetPadding(FMargin(14.f, 9.f)));
	}

	static TSharedRef<SWidget> MakeLiveCasinoHeader(
		const FText& Title,
		const FText& Description,
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
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(Title)
				.TextStyle(&TextTitle)
				.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 14.f)
			[
				SNew(STextBlock)
				.Text(Description)
				.TextStyle(&TextBody)
				.ColorAndOpacity(FT66FlatStyle::Tokens::TextMuted)
				.AutoWrapText(true)
			];
	}

	static void SetIndexedBorderState(TArray<TSharedPtr<SBorder>>& Borders, const int32 ChosenIndex, const int32 WinningIndex)
	{
		for (int32 Index = 0; Index < Borders.Num(); ++Index)
		{
			if (!Borders[Index].IsValid())
			{
				continue;
			}
			const bool bChosen = Index == ChosenIndex;
			const bool bWinning = Index == WinningIndex;
			Borders[Index]->SetBorderBackgroundColor(
				bWinning
					? FLinearColor(0.28f, 0.75f, 0.36f, 1.f)
					: bChosen
						? FLinearColor(0.75f, 0.28f, 0.28f, 1.f)
						: FT66FlatStyle::Tokens::Panel2);
		}
	}
}

TSharedRef<SWidget> UT66GuessCupGameWidget::RebuildWidget()
{
	CupBorders.SetNum(3);
	const FTextBlockStyle& TextHeading = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading"));
	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));

	TSharedRef<SUniformGridPanel> CupGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(10.f, 0.f));
	for (int32 Index = 0; Index < 3; ++Index)
	{
		CupGrid->AddSlot(Index, 0)
		[
			SAssignNew(CupBorders[Index], SBorder)
			.BorderBackgroundColor(FT66FlatStyle::Tokens::Panel2)
			.Padding(10.f)
			[
				MakeLiveCasinoButton(
					FText::Format(NSLOCTEXT("T66.Gambler", "CupNumberFormat", "Cup {0}"), FText::AsNumber(Index + 1)),
					FOnClicked::CreateUObject(this, &UT66GuessCupGameWidget::OnCupClicked, Index))
			]
		];
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeLiveCasinoHeader(
				NSLOCTEXT("T66.Gambler", "GuessCupTitle", "GUESS THE CUP"),
				NSLOCTEXT("T66.Gambler", "GuessCupDesc", "Pick the cup hiding the token. Correct cup pays 3x."),
				StatusText,
				WagerText,
				FOnClicked::CreateUObject(this, &UT66GuessCupGameWidget::OnBackClicked))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 12.f)
		[
			CupGrid
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SAssignNew(ResultText, STextBlock)
			.Text(NSLOCTEXT("T66.Gambler", "PickACup", "Pick a cup."))
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
		];
}

void UT66GuessCupGameWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
	ResetForOpen();
}

void UT66GuessCupGameWidget::DeactivateWidgetGame()
{
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

void UT66GuessCupGameWidget::ResetForOpen()
{
	SetStatus(FText::GetEmpty(), FT66FlatStyle::Tokens::Text);
	if (ResultText.IsValid())
	{
		ResultText->SetText(NSLOCTEXT("T66.Gambler", "PickACup", "Pick a cup."));
	}
	for (TSharedPtr<SBorder>& Border : CupBorders)
	{
		if (Border.IsValid())
		{
			Border->SetBorderBackgroundColor(FT66FlatStyle::Tokens::Panel2);
		}
	}
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

void UT66GuessCupGameWidget::RevealResult(const int32 ChosenCup, const int32 WinningCup, const int32 PayoutGold)
{
	SetIndexedBorderState(CupBorders, ChosenCup, WinningCup);
	if (ResultText.IsValid())
	{
		ResultText->SetText(PayoutGold > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "CupWinFormat", "Cup {0}. WIN (+{1})"), FText::AsNumber(WinningCup + 1), FText::AsNumber(PayoutGold))
			: FText::Format(NSLOCTEXT("T66.Gambler", "CupLoseFormat", "Cup {0}. LOSE"), FText::AsNumber(WinningCup + 1)));
	}
}

FReply UT66GuessCupGameWidget::OnBackClicked()
{
	RequestWidgetGameExit();
	return FReply::Handled();
}

FReply UT66GuessCupGameWidget::OnCupClicked(const int32 CupIndex)
{
	if (ChoiceCallback)
	{
		ChoiceCallback(CupIndex);
	}
	return FReply::Handled();
}

TSharedRef<SWidget> UT66StickPickGameWidget::RebuildWidget()
{
	StickBorders.SetNum(5);
	const FTextBlockStyle& TextHeading = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading"));
	const FTextBlockStyle& TextBody = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Body"));

	TSharedRef<SUniformGridPanel> StickGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(7.f, 0.f));
	for (int32 Index = 0; Index < 5; ++Index)
	{
		StickGrid->AddSlot(Index, 0)
		[
			SAssignNew(StickBorders[Index], SBorder)
			.BorderBackgroundColor(FT66FlatStyle::Tokens::Panel2)
			.Padding(8.f)
			[
				MakeLiveCasinoButton(
					FText::Format(NSLOCTEXT("T66.Gambler", "StickNumberFormat", "Stick {0}"), FText::AsNumber(Index + 1)),
					FOnClicked::CreateUObject(this, &UT66StickPickGameWidget::OnStickClicked, Index))
			]
		];
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeLiveCasinoHeader(
				NSLOCTEXT("T66.Gambler", "StickPickTitle", "PICK THE STICK"),
				NSLOCTEXT("T66.Gambler", "StickPickDesc", "Pick the called longest or shortest stick. Correct stick pays 5x."),
				StatusText,
				WagerText,
				FOnClicked::CreateUObject(this, &UT66StickPickGameWidget::OnBackClicked))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 10.f)
		[
			SAssignNew(TargetText, STextBlock)
			.Text(NSLOCTEXT("T66.Gambler", "StickTargetPending", "Target: -"))
			.TextStyle(&TextBody)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Accent2)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 10.f, 0.f, 12.f)
		[
			StickGrid
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SAssignNew(ResultText, STextBlock)
			.Text(NSLOCTEXT("T66.Gambler", "PickAStick", "Pick a stick."))
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
		];
}

void UT66StickPickGameWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
	ResetForOpen();
}

void UT66StickPickGameWidget::DeactivateWidgetGame()
{
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

void UT66StickPickGameWidget::ResetForOpen()
{
	SetStatus(FText::GetEmpty(), FT66FlatStyle::Tokens::Text);
	if (ResultText.IsValid())
	{
		ResultText->SetText(NSLOCTEXT("T66.Gambler", "PickAStick", "Pick a stick."));
	}
	for (TSharedPtr<SBorder>& Border : StickBorders)
	{
		if (Border.IsValid())
		{
			Border->SetBorderBackgroundColor(FT66FlatStyle::Tokens::Panel2);
		}
	}
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
	if (TargetText.IsValid())
	{
		TargetText->SetText(bShortest
			? NSLOCTEXT("T66.Gambler", "StickTargetShortest", "Target: SHORTEST")
			: NSLOCTEXT("T66.Gambler", "StickTargetLongest", "Target: LONGEST"));
	}
}

void UT66StickPickGameWidget::RevealResult(const int32 ChosenStick, const int32 TargetStick, const bool bTargetShortest, const int32 PayoutGold)
{
	SetIndexedBorderState(StickBorders, ChosenStick, TargetStick);
	if (ResultText.IsValid())
	{
		const FText Target = bTargetShortest
			? NSLOCTEXT("T66.Gambler", "Shortest", "shortest")
			: NSLOCTEXT("T66.Gambler", "Longest", "longest");
		ResultText->SetText(PayoutGold > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "StickWinFormat", "Stick {0} was the {1}. WIN (+{2})"), FText::AsNumber(TargetStick + 1), Target, FText::AsNumber(PayoutGold))
			: FText::Format(NSLOCTEXT("T66.Gambler", "StickLoseFormat", "Stick {0} was the {1}. LOSE"), FText::AsNumber(TargetStick + 1), Target));
	}
}

FReply UT66StickPickGameWidget::OnBackClicked()
{
	RequestWidgetGameExit();
	return FReply::Handled();
}

FReply UT66StickPickGameWidget::OnStickClicked(const int32 StickIndex)
{
	if (ChoiceCallback)
	{
		ChoiceCallback(StickIndex);
	}
	return FReply::Handled();
}

TSharedRef<SWidget> UT66FindJokerGameWidget::RebuildWidget()
{
	CardBorders.SetNum(10);
	const FTextBlockStyle& TextHeading = FT66FlatStyle::GetTextBlockStyle(TEXT("T66.Text.Heading"));

	TSharedRef<SUniformGridPanel> CardGrid = SNew(SUniformGridPanel).SlotPadding(FMargin(6.f, 6.f));
	for (int32 Index = 0; Index < 10; ++Index)
	{
		CardGrid->AddSlot(Index % 5, Index / 5)
		[
			SAssignNew(CardBorders[Index], SBorder)
			.BorderBackgroundColor(FT66FlatStyle::Tokens::Panel2)
			.Padding(8.f)
			[
				MakeLiveCasinoButton(
					FText::Format(NSLOCTEXT("T66.Gambler", "CardNumberFormat", "Card {0}"), FText::AsNumber(Index + 1)),
					FOnClicked::CreateUObject(this, &UT66FindJokerGameWidget::OnCardClicked, Index))
			]
		];
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeLiveCasinoHeader(
				NSLOCTEXT("T66.Gambler", "FindJokerTitle", "FIND THE JOKER"),
				NSLOCTEXT("T66.Gambler", "FindJokerDesc", "Pick the card hiding the Joker. Correct card pays 10x."),
				StatusText,
				WagerText,
				FOnClicked::CreateUObject(this, &UT66FindJokerGameWidget::OnBackClicked))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 8.f, 0.f, 12.f)
		[
			CardGrid
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SAssignNew(ResultText, STextBlock)
			.Text(NSLOCTEXT("T66.Gambler", "PickACard", "Pick a card."))
			.TextStyle(&TextHeading)
			.ColorAndOpacity(FT66FlatStyle::Tokens::Text)
		];
}

void UT66FindJokerGameWidget::ActivateWidgetGame(const FT66WidgetGameHostContext& HostContext)
{
	WidgetGameHostContext = HostContext;
	ResetForOpen();
}

void UT66FindJokerGameWidget::DeactivateWidgetGame()
{
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

void UT66FindJokerGameWidget::ResetForOpen()
{
	SetStatus(FText::GetEmpty(), FT66FlatStyle::Tokens::Text);
	if (ResultText.IsValid())
	{
		ResultText->SetText(NSLOCTEXT("T66.Gambler", "PickACard", "Pick a card."));
	}
	for (TSharedPtr<SBorder>& Border : CardBorders)
	{
		if (Border.IsValid())
		{
			Border->SetBorderBackgroundColor(FT66FlatStyle::Tokens::Panel2);
		}
	}
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

void UT66FindJokerGameWidget::RevealResult(const int32 ChosenCard, const int32 JokerCard, const int32 PayoutGold)
{
	SetIndexedBorderState(CardBorders, ChosenCard, JokerCard);
	if (ResultText.IsValid())
	{
		ResultText->SetText(PayoutGold > 0
			? FText::Format(NSLOCTEXT("T66.Gambler", "JokerWinFormat", "Joker on card {0}. WIN (+{1})"), FText::AsNumber(JokerCard + 1), FText::AsNumber(PayoutGold))
			: FText::Format(NSLOCTEXT("T66.Gambler", "JokerLoseFormat", "Joker on card {0}. LOSE"), FText::AsNumber(JokerCard + 1)));
	}
}

FReply UT66FindJokerGameWidget::OnBackClicked()
{
	RequestWidgetGameExit();
	return FReply::Handled();
}

FReply UT66FindJokerGameWidget::OnCardClicked(const int32 CardIndex)
{
	if (ChoiceCallback)
	{
		ChoiceCallback(CardIndex);
	}
	return FReply::Handled();
}
