// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66SaveSlotsScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66GameInstance.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Kismet/GameplayStatics.h"
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
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	FText TitleText = Loc ? Loc->GetText_SaveSlots() : FText::FromString(TEXT("SAVE SLOTS"));
	FText EmptyText = Loc ? Loc->GetText_EmptySlot() : FText::FromString(TEXT("EMPTY SLOT"));
	FText BackText = Loc ? Loc->GetText_Back() : FText::FromString(TEXT("BACK"));

	auto MakeSlotButton = [this, SaveSub, EmptyText](int32 SlotIndex) -> TSharedRef<SWidget>
	{
		bool bOccupied = false;
		FString LastPlayed, HeroName, MapName;
		if (SaveSub)
		{
			SaveSub->GetSlotMeta(SlotIndex, bOccupied, LastPlayed, HeroName, MapName);
		}
		FText Label = bOccupied ? FText::FromString(FString::Printf(TEXT("%s\n%s"), *HeroName, *LastPlayed)) : EmptyText;
		FLinearColor BgColor = bOccupied ? FLinearColor(0.15f, 0.25f, 0.2f, 1.0f) : FLinearColor(0.12f, 0.12f, 0.14f, 1.0f);

		return SNew(SBox)
			.WidthOverride(180.0f)
			.HeightOverride(90.0f)
			.Padding(8.0f)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.IsEnabled(bOccupied)
				.OnClicked_Lambda([this, SlotIndex]() { OnSlotClicked(SlotIndex); return FReply::Handled(); })
				.ButtonColorAndOpacity(BgColor)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle(bOccupied ? "Bold" : "Regular", 12))
					.ColorAndOpacity(bOccupied ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f))
					.AutoWrapText(true)
					.Justification(ETextJustify::Center)
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
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 40.0f, 0.0f, 30.0f)
				[
					SNew(STextBlock).Text(TitleText)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SVerticalBox::Slot().FillHeight(1.0f).HAlign(HAlign_Center).VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(0) ]
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(1) ]
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(2) ]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(3) ]
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(4) ]
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(5) ]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(6) ]
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(7) ]
						+ SVerticalBox::Slot().AutoHeight()[ MakeSlotButton(8) ]
					]
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
						SNew(STextBlock).Text(BackText)
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
	if (!IsSlotOccupied(SlotIndex)) return;

	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!GI || !SaveSub) return;

	UT66RunSaveGame* Loaded = SaveSub->LoadFromSlot(SlotIndex);
	if (!Loaded) return;

	GI->SelectedHeroID = Loaded->HeroID;
	GI->SelectedHeroBodyType = Loaded->HeroBodyType;
	GI->SelectedCompanionID = Loaded->CompanionID;
	GI->SelectedDifficulty = Loaded->Difficulty;
	GI->SelectedPartySize = Loaded->PartySize;
	GI->PendingLoadedTransform = Loaded->PlayerTransform;
	GI->bApplyLoadedTransform = true;
	GI->CurrentSaveSlotIndex = SlotIndex;

	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
}

bool UT66SaveSlotsScreen::IsSlotOccupied(int32 SlotIndex) const
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66SaveSubsystem* SaveSub = GI ? GI->GetSubsystem<UT66SaveSubsystem>() : nullptr;
	if (!SaveSub) return false;
	bool bOccupied = false;
	FString D1, D2, D3;
	SaveSub->GetSlotMeta(SlotIndex, bOccupied, D1, D2, D3);
	return bOccupied;
}
void UT66SaveSlotsScreen::OnPreviousPageClicked() { if (CurrentPage > 0) { CurrentPage--; RefreshScreen(); } }
void UT66SaveSlotsScreen::OnNextPageClicked() { if (CurrentPage < TotalPages - 1) { CurrentPage++; RefreshScreen(); } }
void UT66SaveSlotsScreen::OnBackClicked() { NavigateBack(); }
