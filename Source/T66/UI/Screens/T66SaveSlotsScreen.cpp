// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SaveSlotsScreen.h"
#include "UI/T66UIManager.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

UT66SaveSlotsScreen::UT66SaveSlotsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::SaveSlots;
	bIsModal = false;
}

TSharedRef<SWidget> UT66SaveSlotsScreen::BuildSlateUI()
{
	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.0f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 80.0f, 0.0f, 60.0f)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("SAVE SLOTS")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SVerticalBox::Slot().FillHeight(1.0f).HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Save slots placeholder.\n\n3x3 grid will appear here.")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
					.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
				]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(20.0f, 0.0f, 0.0f, 20.0f)
			[
				SNew(SBox).WidthOverride(120.0f).HeightOverride(50.0f)
				[
					SNew(SButton).HAlign(HAlign_Center).VAlign(VAlign_Center)
					.OnClicked(FOnClicked::CreateUObject(this, &UT66SaveSlotsScreen::HandleBackClicked))
					.ButtonColorAndOpacity(FLinearColor(0.15f, 0.15f, 0.2f, 1.0f))
					[
						SNew(STextBlock).Text(FText::FromString(TEXT("BACK")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FLinearColor::White)
					]
				]
			]
		];
}

FReply UT66SaveSlotsScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

void UT66SaveSlotsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	CurrentPage = 0;
	TotalPages = 1;
}

void UT66SaveSlotsScreen::OnSlotClicked(int32 SlotIndex)
{
	if (IsSlotOccupied(SlotIndex)) NavigateTo(ET66ScreenType::HeroSelection);
}

bool UT66SaveSlotsScreen::IsSlotOccupied(int32 SlotIndex) const { return false; }
void UT66SaveSlotsScreen::OnPreviousPageClicked() { if (CurrentPage > 0) { CurrentPage--; RefreshScreen(); } }
void UT66SaveSlotsScreen::OnNextPageClicked() { if (CurrentPage < TotalPages - 1) { CurrentPage++; RefreshScreen(); } }
void UT66SaveSlotsScreen::OnBackClicked() { NavigateBack(); }
