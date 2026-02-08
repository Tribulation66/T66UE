// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PartySizePickerScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
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
	// Get localization subsystem
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	FText TitleText = Loc ? Loc->GetText_SelectPartySize() : NSLOCTEXT("T66.PartySize", "Title", "SELECT PARTY SIZE");
	FText SoloText = Loc ? Loc->GetText_Solo() : NSLOCTEXT("T66.PartySize", "Solo", "SOLO");
	FText CoopText = Loc ? Loc->GetText_Coop() : NSLOCTEXT("T66.PartySize", "Coop", "CO-OP");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");

	const FButtonStyle& BtnNeutral = FT66Style::Get().GetWidgetStyle<FButtonStyle>("T66.Button.Neutral");
	auto MakePartySizeCard = [this, &BtnNeutral](const FText& Title, const FText& Description, FReply (UT66PartySizePickerScreen::*ClickFunc)(), const FLinearColor& BgColor) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(FMargin(8.0f, 0.0f))
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
				.ButtonStyle(&BtnNeutral)
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
							.Font(FT66Style::Tokens::FontBold(32))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(Description)
							.Font(FT66Style::Tokens::FontRegular(14))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
					.Text(TitleText)
					.Font(FT66Style::Tokens::FontBold(56))
					.ColorAndOpacity(FLinearColor::White)
				]
			// Party size cards — each fills half the screen
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(20.0f, 0.0f, 20.0f, 60.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.5f)
				[
					MakePartySizeCard(
						SoloText,
						NSLOCTEXT("T66.PartySize", "SoloDesc", "1 player"),
						&UT66PartySizePickerScreen::HandleSoloClicked,
						FLinearColor(0.3f, 0.5f, 0.3f, 1.0f)
					)
				]
				+ SHorizontalBox::Slot().FillWidth(0.5f)
				[
					MakePartySizeCard(
						CoopText,
						NSLOCTEXT("T66.PartySize", "CoopDesc", "2–3 players"),
						&UT66PartySizePickerScreen::HandleCoopClicked,
						FLinearColor(0.3f, 0.3f, 0.5f, 1.0f)
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
				FT66Style::MakeButton(BackText, FOnClicked::CreateUObject(this, &UT66PartySizePickerScreen::HandleBackClicked), ET66ButtonType::Neutral, 120.f, 50.f)
			]
		];
}

FReply UT66PartySizePickerScreen::HandleSoloClicked() { OnSoloClicked(); return FReply::Handled(); }
FReply UT66PartySizePickerScreen::HandleCoopClicked() { OnCoopClicked(); return FReply::Handled(); }
FReply UT66PartySizePickerScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

void UT66PartySizePickerScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		bIsNewGame = GI->bIsNewGameFlow;
	}
}

void UT66PartySizePickerScreen::SelectPartySize(ET66PartySize PartySize)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedPartySize = PartySize;
	}

	if (bIsNewGame) // set from GI->bIsNewGameFlow in OnScreenActivated
	{
		if (PartySize == ET66PartySize::Solo)
		{
			NavigateTo(ET66ScreenType::HeroSelection);
		}
		else
		{
			// Co-op: use Trio (3 slots) for lobby; Duo/Trio only for leaderboards/saves
			NavigateTo(ET66ScreenType::Lobby);
		}
	}
	else
	{
		NavigateTo(ET66ScreenType::SaveSlots);
	}
}

void UT66PartySizePickerScreen::OnSoloClicked() { SelectPartySize(ET66PartySize::Solo); }
void UT66PartySizePickerScreen::OnCoopClicked() { SelectPartySize(ET66PartySize::Trio); } // Co-op UI → Trio lobby (3 slots)
void UT66PartySizePickerScreen::OnBackClicked() { NavigateBack(); }
