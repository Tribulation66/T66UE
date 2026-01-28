// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PartySizePickerScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

UT66PartySizePickerScreen::UT66PartySizePickerScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PartySizePicker;
	bIsModal = false;
	bIsNewGame = true;
}

TSharedRef<SWidget> UT66PartySizePickerScreen::BuildSlateUI()
{
	auto MakePartySizeCard = [this](const FText& Title, const FText& Description, FReply (UT66PartySizePickerScreen::*ClickFunc)(), const FLinearColor& BgColor) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(200.0f)
			.HeightOverride(180.0f)
			.Padding(FMargin(15.0f, 0.0f))
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
				.ButtonColorAndOpacity(BgColor)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.0f, 0.0f, 0.0f, 10.0f)
						[
							SNew(STextBlock)
							.Text(Title)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(Description)
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
							.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
						]
					]
				]
			];
	};

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.0f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				// Title
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 80.0f, 0.0f, 60.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("SELECT PARTY SIZE")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 56))
					.ColorAndOpacity(FLinearColor::White)
				]
				// Party size cards
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakePartySizeCard(
							FText::FromString(TEXT("SOLO")),
							FText::FromString(TEXT("Play alone")),
							&UT66PartySizePickerScreen::HandleSoloClicked,
							FLinearColor(0.3f, 0.5f, 0.3f, 1.0f)
						)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakePartySizeCard(
							FText::FromString(TEXT("DUO")),
							FText::FromString(TEXT("Play with 1 friend")),
							&UT66PartySizePickerScreen::HandleDuoClicked,
							FLinearColor(0.3f, 0.3f, 0.5f, 1.0f)
						)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						MakePartySizeCard(
							FText::FromString(TEXT("TRIO")),
							FText::FromString(TEXT("Play with 2 friends")),
							&UT66PartySizePickerScreen::HandleTrioClicked,
							FLinearColor(0.5f, 0.3f, 0.3f, 1.0f)
						)
					]
				]
			]
			// Back button
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.0f, 0.0f, 0.0f, 20.0f)
			[
				SNew(SBox)
				.WidthOverride(120.0f)
				.HeightOverride(50.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66PartySizePickerScreen::HandleBackClicked))
					.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("BACK")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
		];
}

FReply UT66PartySizePickerScreen::HandleSoloClicked() { OnSoloClicked(); return FReply::Handled(); }
FReply UT66PartySizePickerScreen::HandleDuoClicked() { OnDuoClicked(); return FReply::Handled(); }
FReply UT66PartySizePickerScreen::HandleTrioClicked() { OnTrioClicked(); return FReply::Handled(); }
FReply UT66PartySizePickerScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

void UT66PartySizePickerScreen::SelectPartySize(ET66PartySize PartySize)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedPartySize = PartySize;
	}

	if (bIsNewGame)
	{
		NavigateTo(ET66ScreenType::HeroSelection);
	}
	else
	{
		NavigateTo(ET66ScreenType::SaveSlots);
	}
}

void UT66PartySizePickerScreen::OnSoloClicked() { SelectPartySize(ET66PartySize::Solo); }
void UT66PartySizePickerScreen::OnDuoClicked() { SelectPartySize(ET66PartySize::Duo); }
void UT66PartySizePickerScreen::OnTrioClicked() { SelectPartySize(ET66PartySize::Trio); }
void UT66PartySizePickerScreen::OnBackClicked() { NavigateBack(); }
