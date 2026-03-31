// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LanguageSelectScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

namespace
{
	FLinearColor T66LanguageShellFill()
	{
		return FT66Style::Background();
	}

	FLinearColor T66LanguagePanelFill()
	{
		return FT66Style::PanelOuter();
	}

	FLinearColor T66LanguageRowFill()
	{
		return FT66Style::PanelInner();
	}

	FLinearColor T66LanguageNeutralButtonFill()
	{
		return FT66Style::ButtonNeutral();
	}

	TSharedRef<SWidget> MakeLanguageButton(const FT66ButtonParams& Params)
	{
		FT66ButtonParams FlatParams = Params;
		FlatParams
			.SetBorderVisual(ET66ButtonBorderVisual::None)
			.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
			.SetUseGlow(false);

		return FT66Style::MakeButton(FlatParams);
	}

	TSharedRef<SWidget> MakeLanguagePanel(const TSharedRef<SWidget>& Content, ET66PanelType Type, const FLinearColor& FillColor, const FMargin& Padding)
	{
		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(Type)
				.SetBorderVisual(ET66ButtonBorderVisual::None)
				.SetBackgroundVisual(ET66ButtonBackgroundVisual::None)
				.SetColor(FillColor)
				.SetPadding(Padding));
	}
}

UT66LanguageSelectScreen::UT66LanguageSelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::LanguageSelect;
	bIsModal = false;
}

UT66LocalizationSubsystem* UT66LanguageSelectScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}
	return nullptr;
}

TSharedRef<SWidget> UT66LanguageSelectScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();

	// Build language buttons
	TSharedRef<SVerticalBox> LanguageButtons = SNew(SVerticalBox);
	
	if (Loc)
	{
		TArray<ET66Language> Languages = Loc->GetAvailableLanguages();
		for (ET66Language Lang : Languages)
		{
			FText LangName = Loc->GetLanguageDisplayName(Lang);
			// Make selection highlight persistent by binding the row color to PreviewedLanguage.
			// This avoids having to rebuild the whole Slate tree each click (and keeps scroll position).
			const auto GetRowBgColor = [this, Lang]()
			{
				const bool bIsSelected = (Lang == PreviewedLanguage);
				return bIsSelected ? FT66Style::SelectionFill() : T66LanguageRowFill();
			};

			const auto GetRowTextColor = [this, Lang]()
			{
				const bool bIsSelected = (Lang == PreviewedLanguage);
				return bIsSelected ? FLinearColor::White : FLinearColor(0.92f, 0.92f, 0.95f, 1.0f);
			};

			LanguageButtons->AddSlot()
			.AutoHeight()
			// Revert list spacing closer to the original.
			.HAlign(HAlign_Center)
			.Padding(10.0f, 5.0f)
			[
				SNew(SBox)
				.WidthOverride(400.0f)
				.HeightOverride(60.0f)
				[
					SNew(SButton)
					.ButtonStyle(FCoreStyle::Get(), "NoBorder")
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleLanguageClicked, Lang))
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor_Lambda(GetRowBgColor)
						.Padding(FMargin(0.0f))
						[
							SNew(STextBlock)
							.Text(LangName)
							.Font(FT66Style::Tokens::FontBold(22))
							.ColorAndOpacity_Lambda(GetRowTextColor)
							.Justification(ETextJustify::Center)
						]
					]
				]
			];
		}
	}

	FText TitleText = Loc ? Loc->GetText_SelectLanguage() : NSLOCTEXT("T66.LanguageSelect", "Title", "Select Language");
	FText ConfirmText = Loc ? Loc->GetText_Confirm() : NSLOCTEXT("T66.LanguageSelect", "Confirm", "Confirm");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.LanguageSelect", "Back", "Back");

	const float ScreenPadding = 60.0f;
	const bool bModalPresentation = (UIManager && UIManager->GetCurrentModalType() == ScreenType) || (!UIManager && GetOwningPlayer() && GetOwningPlayer()->IsPaused());
	const float TopInset = bModalPresentation ? 0.f : (UIManager ? UIManager->GetFrontendTopBarContentHeight() : 0.f);

	const TSharedRef<SWidget> LanguageList =
		MakeLanguagePanel(
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			.ScrollBarVisibility(EVisibility::Visible)
			.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
			+ SScrollBox::Slot()
			.Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
			[
				LanguageButtons
			],
			ET66PanelType::Panel2,
			T66LanguagePanelFill(),
			FMargin(18.f, 16.f, 14.f, 16.f));

	const TSharedRef<SWidget> PageContent =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 0.0f, 26.0f)
		[
			SNew(STextBlock)
			.Text(TitleText)
			.Font(FT66Style::Tokens::FontBold(42))
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.HAlign(HAlign_Fill)
		[
			LanguageList
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 30.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			.Visibility(bModalPresentation ? EVisibility::Visible : EVisibility::Collapsed)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f, 0.0f)
			[
				MakeLanguageButton(
					FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleBackClicked), ET66ButtonType::Neutral)
					.SetMinWidth(150.f)
					.SetColor(T66LanguageNeutralButtonFill()))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f, 0.0f)
			[
				MakeLanguageButton(
					FT66ButtonParams(ConfirmText, FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleConfirmClicked), ET66ButtonType::Success)
					.SetMinWidth(150.f)
					.SetColor(FT66Style::Tokens::Success))
			]
		];

	if (bModalPresentation)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Scrim())
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(FMargin(ScreenPadding))
			[
				MakeLanguagePanel(
					SNew(SBox)
					.WidthOverride(960.f)
					.Padding(FMargin(40.0f, 30.0f))
					[
						PageContent
					],
					ET66PanelType::Panel,
					T66LanguageShellFill(),
					FMargin(24.f))
			];
	}

	return SNew(SBox)
		.Padding(FMargin(0.f, TopInset, 0.f, 0.f))
		[
			MakeLanguagePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					PageContent
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 24.0f, 0.0f, 0.0f)
				[
					MakeLanguageButton(
						FT66ButtonParams(ConfirmText, FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleConfirmClicked), ET66ButtonType::Success)
						.SetMinWidth(180.f)
						.SetColor(FT66Style::Tokens::Success))
				],
				ET66PanelType::Panel,
				T66LanguageShellFill(),
				FMargin(ScreenPadding, 28.f, ScreenPadding, 28.f))
		];
}

void UT66LanguageSelectScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		OriginalLanguage = Loc->GetCurrentLanguage();
		PreviewedLanguage = OriginalLanguage;
	}
}

void UT66LanguageSelectScreen::SelectLanguage(ET66Language Language)
{
	PreviewedLanguage = Language;
	
	// Just update visual selection, don't apply yet
	// Language will be applied on Confirm
	InvalidateLayoutAndVolatility();
}

FReply UT66LanguageSelectScreen::HandleLanguageClicked(ET66Language Language)
{
	SelectLanguage(Language);
	return FReply::Handled();
}

FReply UT66LanguageSelectScreen::HandleConfirmClicked()
{
	OnConfirmClicked();
	return FReply::Handled();
}

FReply UT66LanguageSelectScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

void UT66LanguageSelectScreen::OnConfirmClicked()
{
	const bool bModalPresentation = UIManager && UIManager->GetCurrentModalType() == ScreenType;

	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->SetLanguage(PreviewedLanguage);
	}

	if (bModalPresentation)
	{
		CloseModal();
	}
	else if (UIManager)
	{
		UIManager->GoBack();
	}

	if (UIManager)
	{
		UIManager->RebuildAllVisibleUI();
	}
}

void UT66LanguageSelectScreen::OnBackClicked()
{
	if (UIManager && UIManager->GetCurrentModalType() == ScreenType)
	{
		CloseModal();
		return;
	}

	if (UIManager)
	{
		UIManager->GoBack();
	}
}

void UT66LanguageSelectScreen::RebuildLanguageList()
{
	// Force widget rebuild
	ForceRebuildSlate();
}
