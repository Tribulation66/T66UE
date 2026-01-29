// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/T66UIManager.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

UT66PauseMenuScreen::UT66PauseMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PauseMenu;
	bIsModal = true;
}

AT66PlayerController* UT66PauseMenuScreen::GetT66PlayerController() const
{
	return Cast<AT66PlayerController>(GetOwningPlayer());
}

TSharedRef<SWidget> UT66PauseMenuScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = nullptr;
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		Loc = GI->GetSubsystem<UT66LocalizationSubsystem>();
	}

	FText ResumeText = Loc ? Loc->GetText_Resume() : FText::FromString(TEXT("RESUME GAME"));
	FText SaveAndQuitText = Loc ? Loc->GetText_SaveAndQuit() : FText::FromString(TEXT("SAVE AND QUIT GAME"));
	FText RestartText = Loc ? Loc->GetText_Restart() : FText::FromString(TEXT("RESTART"));
	FText SettingsText = Loc ? Loc->GetText_Settings() : FText::FromString(TEXT("SETTINGS"));
	FText ReportBugText = Loc ? Loc->GetText_ReportBug() : FText::FromString(TEXT("REPORT BUG"));

	auto MakePauseButton = [this](const FText& Text, FReply (UT66PauseMenuScreen::*ClickFunc)(), const FLinearColor& BgColor) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(320.0f)
			.HeightOverride(50.0f)
			.Padding(FMargin(0.0f, 6.0f))
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateUObject(this, ClickFunc))
				.ButtonColorAndOpacity(BgColor)
				[
					SNew(STextBlock)
					.Text(Text)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor::White)
					.Justification(ETextJustify::Center)
				]
			];
	};

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 1.0f))
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.08f, 0.08f, 0.12f, 1.0f))
				.Padding(FMargin(50.0f, 40.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 0.0f, 0.0f, 24.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("PAUSED")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[ MakePauseButton(ResumeText, &UT66PauseMenuScreen::HandleResumeClicked, FLinearColor(0.15f, 0.4f, 0.2f, 1.0f)) ]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[ MakePauseButton(SaveAndQuitText, &UT66PauseMenuScreen::HandleSaveAndQuitClicked, FLinearColor(0.25f, 0.25f, 0.35f, 1.0f)) ]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[ MakePauseButton(RestartText, &UT66PauseMenuScreen::HandleRestartClicked, FLinearColor(0.3f, 0.25f, 0.2f, 1.0f)) ]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[ MakePauseButton(SettingsText, &UT66PauseMenuScreen::HandleSettingsClicked, FLinearColor(0.2f, 0.2f, 0.28f, 1.0f)) ]
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
					[ MakePauseButton(ReportBugText, &UT66PauseMenuScreen::HandleReportBugClicked, FLinearColor(0.28f, 0.2f, 0.2f, 1.0f)) ]
				]
			]
		];
}

FReply UT66PauseMenuScreen::HandleResumeClicked() { OnResumeClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSaveAndQuitClicked() { OnSaveAndQuitClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSettingsClicked() { OnSettingsClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleReportBugClicked() { OnReportBugClicked(); return FReply::Handled(); }

void UT66PauseMenuScreen::OnResumeClicked()
{
	CloseModal();
	AT66PlayerController* PC = GetT66PlayerController();
	if (PC)
	{
		PC->SetPause(false);
		PC->RestoreGameplayInputMode();
	}
}

void UT66PauseMenuScreen::OnSaveAndQuitClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	AT66PlayerController* PC = GetT66PlayerController();
	if (!GI || !PC) return;

	UT66SaveSubsystem* SaveSub = GI->GetSubsystem<UT66SaveSubsystem>();
	if (!SaveSub) return;

	int32 SlotIndex = GI->CurrentSaveSlotIndex;
	if (SlotIndex < 0 || SlotIndex >= UT66SaveSubsystem::MaxSlots)
	{
		SlotIndex = SaveSub->FindFirstEmptySlot();
		if (SlotIndex == INDEX_NONE)
		{
			// All slots full: overwrite oldest
			SlotIndex = SaveSub->FindOldestOccupiedSlot();
			if (SlotIndex == INDEX_NONE) return;
		}
		GI->CurrentSaveSlotIndex = SlotIndex;
	}

	UT66RunSaveGame* SaveObj = NewObject<UT66RunSaveGame>(this);
	if (!SaveObj) return;

	SaveObj->HeroID = GI->SelectedHeroID;
	SaveObj->HeroBodyType = GI->SelectedHeroBodyType;
	SaveObj->CompanionID = GI->SelectedCompanionID;
	SaveObj->Difficulty = GI->SelectedDifficulty;
	SaveObj->PartySize = GI->SelectedPartySize;
	SaveObj->MapName = GetWorld() ? UWorld::RemovePIEPrefix(GetWorld()->GetMapName()) : FString();
	SaveObj->LastPlayedUtc = FDateTime::UtcNow().ToIso8601();

	if (APawn* Pawn = PC->GetPawn())
	{
		SaveObj->PlayerTransform = Pawn->GetActorTransform();
	}

	if (!SaveSub->SaveToSlot(SlotIndex, SaveObj)) return;

	PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("FrontendLevel")));
}

void UT66PauseMenuScreen::OnRestartClicked()
{
	AT66PlayerController* PC = GetT66PlayerController();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
}

void UT66PauseMenuScreen::OnSettingsClicked()
{
	ShowModal(ET66ScreenType::Settings);
}

void UT66PauseMenuScreen::OnReportBugClicked()
{
	ShowModal(ET66ScreenType::ReportBug);
}
