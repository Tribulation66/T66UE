// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LanguageSelectScreen.h"
#include "UI/T66UIManager.h"
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
			bool bIsSelected = (Lang == PreviewedLanguage);
			FLinearColor BgColor = bIsSelected ? FLinearColor(0.3f, 0.5f, 0.8f, 1.0f) : FLinearColor(0.15f, 0.15f, 0.2f, 1.0f);

			LanguageButtons->AddSlot()
			.AutoHeight()
			.Padding(10.0f, 5.0f)
			[
				SNew(SBox)
				.WidthOverride(400.0f)
				.HeightOverride(60.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(const_cast<UT66LanguageSelectScreen*>(this), &UT66LanguageSelectScreen::HandleLanguageClicked, Lang))
					.ButtonColorAndOpacity(BgColor)
					[
						SNew(STextBlock)
						.Text(LangName)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
						.ColorAndOpacity(FLinearColor::White)
						.Justification(ETextJustify::Center)
					]
				]
			];
		}
	}

	FText TitleText = Loc ? Loc->GetText_SelectLanguage() : FText::FromString(TEXT("SELECT LANGUAGE"));
	FText ConfirmText = Loc ? Loc->GetText_Confirm() : FText::FromString(TEXT("CONFIRM"));
	FText BackText = Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"));

	// Create a solid color brush for full opacity background
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
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
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
						.ColorAndOpacity(FLinearColor::White)
					]
					// Language List
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						LanguageButtons
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
							SNew(SBox)
							.WidthOverride(150.0f)
							.HeightOverride(50.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleBackClicked))
								.ButtonColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.35f, 1.0f))
								[
									SNew(STextBlock)
									.Text(BackText)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.0f, 0.0f)
						[
							SNew(SBox)
							.WidthOverride(150.0f)
							.HeightOverride(50.0f)
							[
								SNew(SButton)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.OnClicked(FOnClicked::CreateUObject(this, &UT66LanguageSelectScreen::HandleConfirmClicked))
								.ButtonColorAndOpacity(FLinearColor(0.2f, 0.5f, 0.3f, 1.0f))
								[
									SNew(STextBlock)
									.Text(ConfirmText)
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
									.ColorAndOpacity(FLinearColor::White)
								]
							]
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
	TakeWidget();
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
	// Apply the selected language now
	if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
	{
		Loc->SetLanguage(PreviewedLanguage);
	}
	
	// Get reference to underlying screen before closing modal
	UT66ScreenBase* UnderlyingScreen = UIManager ? UIManager->GetCurrentScreen() : nullptr;
	
	// Close the modal
	CloseModal();
	
	// Force a full Slate widget rebuild on the underlying screen to apply new language
	// TakeWidget() triggers RebuildWidget() which calls BuildSlateUI() with the new language
	if (UnderlyingScreen)
	{
		UnderlyingScreen->TakeWidget();
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
	TakeWidget();
}
