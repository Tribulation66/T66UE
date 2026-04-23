// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SavePreviewScreen.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Style/T66Style.h"
#include "UI/T66UIManager.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UT66SavePreviewScreen::UT66SavePreviewScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::SavePreview;
	bIsModal = true;
}

UT66LocalizationSubsystem* UT66SavePreviewScreen::GetLocSubsystem() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		return GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	return nullptr;
}

TSharedRef<SWidget> UT66SavePreviewScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	const FText PreviewTitle = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.SavePreview", "PreviewFallback", "PREVIEW");
	const FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText LoadText = NSLOCTEXT("T66.SavePreview", "Load", "LOAD");
	const FText SubtitleText = NSLOCTEXT("T66.SavePreview", "Subtitle", "The run is paused for inspection. Back returns to Save Slots, Load resumes normally.");
	const TSharedRef<SWidget> BackButton =
		FT66Style::MakeButton(
			FT66ButtonParams(BackText, FOnClicked::CreateUObject(this, &UT66SavePreviewScreen::HandleBackClicked), ET66ButtonType::Neutral)
			.SetMinWidth(160.f)
			.SetHeight(42.f)
			.SetFontSize(12));
	const TSharedRef<SWidget> LoadButton =
		FT66Style::MakeButton(
			FT66ButtonParams(LoadText, FOnClicked::CreateUObject(this, &UT66SavePreviewScreen::HandleLoadClicked), ET66ButtonType::Primary)
			.SetMinWidth(160.f)
			.SetHeight(42.f)
			.SetFontSize(12));

	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(32.f, 32.f, 32.f, 28.f))
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(PreviewTitle)
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 4.f, 0.f, 14.f)
				[
					SNew(STextBlock)
					.Text(SubtitleText)
					.Font(FT66Style::Tokens::FontRegular(11))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					T66ScreenSlateHelpers::MakeTwoButtonRow(
						BackButton,
						LoadButton,
						FMargin(0.f, 0.f, 10.f, 0.f),
						FMargin(0.f))
				],
				FT66PanelParams(ET66PanelType::Panel)
					.SetColor(FLinearColor(0.f, 0.f, 0.f, 0.92f))
					.SetPadding(FMargin(18.f, 16.f, 18.f, 16.f)))
		];
}

FReply UT66SavePreviewScreen::HandleBackClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66SavePreviewScreen::HandleLoadClicked()
{
	OnLoadClicked();
	return FReply::Handled();
}

void UT66SavePreviewScreen::OnBackClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	AT66PlayerController* PC = GetOwningPlayer<AT66PlayerController>();
	if (PC)
	{
		PC->SetPause(false);
	}

	if (GI)
	{
		GI->bSaveSlotPreviewMode = false;
		GI->PendingFrontendScreen = ET66ScreenType::SaveSlots;
	}

	if (UIManager)
	{
		UIManager->HideAllUI();
	}

	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetFrontendLevelName());
}

void UT66SavePreviewScreen::OnLoadClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	AT66PlayerController* PC = GetOwningPlayer<AT66PlayerController>();
	if (GI)
	{
		GI->bSaveSlotPreviewMode = false;
		GI->bRestoreSaveSlotsState = false;
		GI->PendingSaveSlotsPage = 0;
	}

	if (UIManager)
	{
		UIManager->CloseModal();
	}

	if (PC)
	{
		PC->SetPause(false);
		PC->RestoreGameplayInputMode();
	}
}
