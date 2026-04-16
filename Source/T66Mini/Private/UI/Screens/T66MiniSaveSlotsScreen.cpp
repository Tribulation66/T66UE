// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66MiniSaveSlotsScreen.h"

#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66MiniFrontendStateSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66MiniRunStateSubsystem.h"
#include "Core/T66MiniRuntimeSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Data/T66MiniDataTypes.h"
#include "Save/T66MiniRunSaveGame.h"
#include "Save/T66MiniSaveSubsystem.h"
#include "UI/T66MiniUIStyle.h"
#include "UI/T66UIManager.h"
#include "UI/T66UITypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

UT66MiniSaveSlotsScreen::UT66MiniSaveSlotsScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::MiniSaveSlots;
	bIsModal = false;
}

void UT66MiniSaveSlotsScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66MiniSaveSlotsScreen::HandleSessionStateChanged);
			SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::MiniSaveSlots, true);
		}
	}
}

void UT66MiniSaveSlotsScreen::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
		}
	}

	SessionStateChangedHandle.Reset();
	Super::OnScreenDeactivated_Implementation();
}

void UT66MiniSaveSlotsScreen::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
		}
	}

	SessionStateChangedHandle.Reset();
	Super::NativeDestruct();
}

void UT66MiniSaveSlotsScreen::HandleSessionStateChanged()
{
	SyncToSharedPartyScreen();
	ForceRebuildSlate();
}

void UT66MiniSaveSlotsScreen::SyncToSharedPartyScreen()
{
	if (!UIManager)
	{
		return;
	}

	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!SessionSubsystem || !SessionSubsystem->IsPartyLobbyContextActive() || SessionSubsystem->IsLocalPlayerPartyHost())
	{
		return;
	}

	const ET66ScreenType DesiredScreen = SessionSubsystem->GetDesiredPartyFrontendScreen();
	switch (DesiredScreen)
	{
	case ET66ScreenType::MiniMainMenu:
	case ET66ScreenType::MiniSaveSlots:
	case ET66ScreenType::MiniCharacterSelect:
	case ET66ScreenType::MiniCompanionSelect:
	case ET66ScreenType::MiniDifficultySelect:
	case ET66ScreenType::MiniIdolSelect:
	case ET66ScreenType::MiniShop:
	case ET66ScreenType::MiniRunSummary:
		if (UIManager->GetCurrentScreenType() != DesiredScreen)
		{
			UIManager->ShowScreen(DesiredScreen);
		}
		break;
	default:
		break;
	}
}

TSharedRef<SWidget> UT66MiniSaveSlotsScreen::BuildSlateUI()
{
	UT66MiniDataSubsystem* DataSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniDataSubsystem>() : nullptr;
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const TArray<FT66MiniSaveSlotSummary> SlotSummaries = SaveSubsystem ? SaveSubsystem->BuildRunSlotSummaries(DataSubsystem) : TArray<FT66MiniSaveSlotSummary>();
	const bool bOnlinePartyActive = PartySubsystem && PartySubsystem->HasRemotePartyMembers();
	const bool bIsPartyHost = !SessionSubsystem || SessionSubsystem->IsLocalPlayerPartyHost();

	TSharedRef<SGridPanel> CardGrid = SNew(SGridPanel);
	for (int32 Index = 0; Index < SlotSummaries.Num(); ++Index)
	{
		const FT66MiniSaveSlotSummary& Summary = SlotSummaries[Index];
		const int32 Row = Index / 2;
		const int32 Column = Index % 2;

		CardGrid->AddSlot(Column, Row)
			.Padding(Column == 0 ? FMargin(0.f, 0.f, 18.f, 18.f) : FMargin(0.f, 0.f, 0.f, 18.f))
			[
				SNew(SBox)
				.WidthOverride(520.f)
				.HeightOverride(182.f)
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(T66MiniUI::PanelFill())
					.Padding(FMargin(18.f))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("Mini Slot %d"), Summary.SlotIndex + 1)))
							.Font(T66MiniUI::BoldFont(22))
							.ColorAndOpacity(FLinearColor::White)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 10.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Summary.bOccupied
								? FString::Printf(TEXT("%s  |  %s  |  Wave %d"), *Summary.HeroDisplayName, *Summary.DifficultyDisplayName, Summary.WaveIndex)
								: TEXT("Empty slot")))
							.Font(T66MiniUI::BodyFont(17))
							.ColorAndOpacity(Summary.bOccupied ? FLinearColor::White : T66MiniUI::MutedText())
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 8.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(FText::FromString(Summary.bOccupied
								? FString::Printf(TEXT("Last Updated: %s"), *Summary.LastUpdatedUtc)
								: TEXT("Mid-wave resume data will populate here once a mini run has been saved.")))
							.Font(T66MiniUI::BodyFont(15))
							.ColorAndOpacity(T66MiniUI::MutedText())
							.AutoWrapText(true)
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.VAlign(VAlign_Bottom)
						[
							SNew(SButton)
							.OnClicked(FOnClicked::CreateLambda([this, SlotIndex = Summary.SlotIndex]()
							{
								return HandleLoadSlotClicked(SlotIndex);
							}))
							.IsEnabled(Summary.bOccupied && (!bOnlinePartyActive || bIsPartyHost))
							.ButtonColorAndOpacity((Summary.bOccupied && (!bOnlinePartyActive || bIsPartyHost)) ? T66MiniUI::AccentGreen() : T66MiniUI::RaisedFill())
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66Mini.SaveSlots", "Load", "LOAD"))
								.Font(T66MiniUI::BoldFont(18))
								.ColorAndOpacity(Summary.bOccupied ? T66MiniUI::ButtonTextDark() : T66MiniUI::MutedText())
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
			];
	}

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(T66MiniUI::WhiteBrush())
			.BorderBackgroundColor(T66MiniUI::ShellFill())
			.Padding(FMargin(28.f, 22.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 18.f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.SaveSlots", "Title", "Mini Save Slots"))
					.Font(T66MiniUI::TitleFont(34))
					.ColorAndOpacity(FLinearColor::White)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					CardGrid
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 12.f, 0.f, 0.f)
				[
					SNew(SBorder)
					.BorderImage(T66MiniUI::WhiteBrush())
					.BorderBackgroundColor(T66MiniUI::PanelFill())
					.Padding(FMargin(14.f))
					[
						SAssignNew(StatusTextBlock, STextBlock)
						.Text(bOnlinePartyActive
							? (bIsPartyHost
								? NSLOCTEXT("T66Mini.SaveSlots", "OnlinePartyHostStatus", "Party host can load a compatible mini co-op save directly into battle. Intermission/shop restores remain disabled until online intermission is finished.")
								: NSLOCTEXT("T66Mini.SaveSlots", "OnlinePartyGuestBlocked", "Only the party host can load a mini party save."))
							: NSLOCTEXT("T66Mini.SaveSlots", "DefaultStatus", "These slots are mini-specific and separate from the regular game's save pipeline."))
						.Font(T66MiniUI::BodyFont(16))
						.ColorAndOpacity(T66MiniUI::MutedText())
						.AutoWrapText(true)
					]
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(28.f, 0.f, 0.f, 24.f)
		[
			SNew(SBox)
			.WidthOverride(140.f)
			.HeightOverride(48.f)
			[
				SNew(SButton)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66MiniSaveSlotsScreen::HandleBackClicked))
				.ButtonColorAndOpacity(T66MiniUI::AccentBlue())
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66Mini.SaveSlots", "Back", "BACK"))
					.Font(T66MiniUI::BoldFont(18))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Center)
				]
			]
		];
}

FReply UT66MiniSaveSlotsScreen::HandleBackClicked()
{
	NavigateBack();
	return FReply::Handled();
}

FReply UT66MiniSaveSlotsScreen::HandleLoadSlotClicked(const int32 SlotIndex)
{
	UT66MiniSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniSaveSubsystem>() : nullptr;
	UT66MiniFrontendStateSubsystem* FrontendState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniFrontendStateSubsystem>() : nullptr;
	UT66MiniRunStateSubsystem* RunState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRunStateSubsystem>() : nullptr;
	UT66MiniRuntimeSubsystem* RuntimeSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66MiniRuntimeSubsystem>() : nullptr;
	UT66PartySubsystem* PartySubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (!SaveSubsystem || !FrontendState || !RunState || !RuntimeSubsystem)
	{
		return FReply::Handled();
	}

	const bool bOnlinePartyActive = PartySubsystem && PartySubsystem->HasRemotePartyMembers();
	const bool bIsPartyHost = !SessionSubsystem || SessionSubsystem->IsLocalPlayerPartyHost();
	if (bOnlinePartyActive && !bIsPartyHost)
	{
		SetStatus(NSLOCTEXT("T66Mini.SaveSlots", "OnlineGuestBlocked", "Only the party host can load a mini party save."));
		return FReply::Handled();
	}

	if (UT66MiniRunSaveGame* RunSave = SaveSubsystem->LoadRunFromSlot(SlotIndex))
	{
		if (bOnlinePartyActive)
		{
			FString FailureReason;
			if (!SaveSubsystem->IsRunCompatibleWithCurrentParty(RunSave, &FailureReason))
			{
				SetStatus(FText::FromString(FailureReason));
				return FReply::Handled();
			}

			if (RunSave->bPendingShopIntermission)
			{
				SetStatus(NSLOCTEXT("T66Mini.SaveSlots", "OnlineIntermissionBlocked", "This mini party save is parked in intermission. Online shop/intermission restore is still disabled, so load a mid-wave party save instead."));
				return FReply::Handled();
			}
		}

		FrontendState->SeedFromRunSave(RunSave);
		RunState->LoadRunFromSave(RunSave);

		if (RunSave->bPendingShopIntermission)
		{
			FrontendState->ResumeIntermissionFlow();
			NavigateTo(ET66ScreenType::MiniShop);
			SetStatus(FText::FromString(FString::Printf(TEXT("Loaded mini slot %d into intermission shop flow."), SlotIndex + 1)));
			return FReply::Handled();
		}

		FString FailureReason;
		if (!RuntimeSubsystem->LaunchMiniBattle(this, &FailureReason))
		{
			SetStatus(FText::FromString(FailureReason));
			return FReply::Handled();
		}

		if (UIManager)
		{
			UIManager->HideAllUI();
		}

		if (bOnlinePartyActive)
		{
			SetStatus(FText::FromString(FString::Printf(TEXT("Loaded mini slot %d and launched the party mini battle runtime."), SlotIndex + 1)));
		}
		else
		{
			SetStatus(FText::FromString(FString::Printf(TEXT("Loaded mini slot %d and launched the isolated mini battle runtime."), SlotIndex + 1)));
		}
	}
	else
	{
		SetStatus(FText::FromString(FString::Printf(TEXT("Mini slot %d could not be loaded."), SlotIndex + 1)));
	}

	return FReply::Handled();
}

void UT66MiniSaveSlotsScreen::SetStatus(const FText& InText)
{
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(InText);
	}
}
