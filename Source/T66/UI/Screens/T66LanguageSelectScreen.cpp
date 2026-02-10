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

UT66LanguageSelectScreen::UT66LanguageSelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::LanguageSelect;
	bIsModal = true;
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
				return bIsSelected ? FLinearColor(0.22f, 0.48f, 0.90f, 1.0f) : FLinearColor(0.12f, 0.12f, 0.18f, 1.0f);
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

	// Create a solid color brush for full opacity background
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(ScreenPadding))
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
				.Padding(FMargin(40.0f, 30.0f))
				[
					SNew(SVerticalBox)
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 0.0f, 0.0f, 30.0f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(36))
						.ColorAndOpacity(FLinearColor::White)
					]
					// Language List
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.HAlign(HAlign_Fill)
					[
						SNew(SScrollBox)
						.Orientation(Orient_Vertical)
						.ScrollBarVisibility(EVisibility::Visible)
						.ConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible)
						+ SScrollBox::Slot()
						.Padding(FMargin(0.0f, 0.0f, 10.0f, 0.0f))
						[
							LanguageButtons
						]
					]
					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.0f, 30.0f, 0.0f, 0.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							FT66Style::MakeButton(BackText, FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleBackClicked), ET66ButtonType::Neutral, 150.f)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							FT66Style::MakeButton(ConfirmText, FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleConfirmClicked), ET66ButtonType::Success, 150.f)
						]
					]
				]
			]
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
	// Get reference to underlying screen before closing modal
	UT66ScreenBase* UnderlyingScreen = UIManager ? UIManager->GetCurrentScreen() : nullptr;
	
	// Close the modal
	CloseModal();

	// Apply the selected language now (after closing) so the selector's own text doesn't visibly switch.
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->SetLanguage(PreviewedLanguage);
	}
	
	// Force a full Slate widget rebuild on the underlying screen to apply new language
	// TakeWidget() triggers RebuildWidget() which calls BuildSlateUI() with the new language
	if (UnderlyingScreen)
	{
		UnderlyingScreen->ForceRebuildSlate();
	}
}

void UT66LanguageSelectScreen::OnBackClicked()
{
	// No change needed - we never applied the preview
	CloseModal();
}

void UT66LanguageSelectScreen::RebuildLanguageList()
{
	// Force widget rebuild
	ForceRebuildSlate();
}
