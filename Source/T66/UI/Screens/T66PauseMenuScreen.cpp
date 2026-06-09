// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PauseMenuScreen.h"

#include "Core/T66AudioSubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66LeaderboardSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/Shutdown/T66ShutdownSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "UI/Components/T66FlatLeaderboardPanel.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66FlatStyle.h"
#include "UI/Style/T66Style.h"

#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/DataTable.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"

namespace
{
	FName PauseMenuTag(const TCHAR* Name)
	{
		return FName(Name);
	}

	TSharedRef<SWidget> MakePauseMenuScrim(const FName Tag)
	{
		return FT66FlatStyle::AttachMetadata(
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.56f))
			.Padding(0.f),
			Tag,
			TEXT("Scrim"),
			ET66FlatState::Default);
	}

	TSharedRef<SWidget> MakePauseMenuButton(
		const FText& Text,
		FOnClicked OnClicked,
		const ET66FlatState State,
		const FName Tag)
	{
		return FT66FlatStyle::MakeFlatButton(
			State,
			Text,
			MoveTemp(OnClicked),
			nullptr,
			nullptr,
			FMargin(18.f, 10.f),
			496.f,
			84.f,
			true,
			26,
			Tag);
	}

	TSharedRef<SWidget> MakePauseMenuCompactButton(
		const FText& Text,
		FOnClicked OnClicked,
		const ET66FlatState State,
		const FName Tag)
	{
		return FT66FlatStyle::MakeFlatButton(
			State,
			Text,
			MoveTemp(OnClicked),
			nullptr,
			nullptr,
			FMargin(18.f, 8.f),
			200.f,
			64.f,
			true,
			22,
			Tag);
	}
}

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

void UT66PauseMenuScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

	UT66AudioSubsystem::PlayUIEventFromAnyWorld(FName(TEXT("UI.Pause.Open")));

	bLeaderboardModalOpen = false;
	FlatLeaderboardPanel.Reset();
}

TSharedRef<SWidget> UT66PauseMenuScreen::BuildSlateUI()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GameInstance ? GameInstance->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66LeaderboardSubsystem* Leaderboard = GameInstance ? GameInstance->GetSubsystem<UT66LeaderboardSubsystem>() : nullptr;

	const FText ResumeText = Loc ? Loc->GetText_Resume() : NSLOCTEXT("T66.PauseMenu", "Resume", "RESUME GAME");
	const FText SaveAndQuitText = Loc ? Loc->GetText_SaveAndQuit() : NSLOCTEXT("T66.PauseMenu", "SaveAndQuit", "SAVE AND QUIT");
	const FText QuitText = Loc ? Loc->GetText_Quit() : NSLOCTEXT("T66.PauseMenu", "Quit", "QUIT");
	const FText RestartText = Loc ? Loc->GetText_Restart() : NSLOCTEXT("T66.PauseMenu", "Restart", "RESTART");
	const FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.PauseMenu", "Settings", "SETTINGS");
	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
	const FText LeaderboardText = NSLOCTEXT("T66.PauseMenu", "Leaderboard", "LEADERBOARD");

	const TSharedRef<SConstraintCanvas> Canvas = SNew(SConstraintCanvas);
	auto AddSlot = [Canvas](const float X, const float Y, const float W, const float H, const TSharedRef<SWidget>& Widget)
	{
		Canvas->AddSlot()
			.Alignment(FVector2D(0.f, 0.f))
			.Offset(FMargin(X, Y, W, H))
			[
				Widget
			];
	};

	AddSlot(0.f, 0.f, 1920.f, 1080.f, MakePauseMenuScrim(PauseMenuTag(TEXT("PauseMenu.Scrim"))));
	AddSlot(657.f, 104.f, 605.f, 872.f,
		FT66FlatStyle::MakeFlatPanel(
			ET66FlatState::Default,
			FMargin(0.f),
			SNullWidget::NullWidget,
			nullptr,
			PauseMenuTag(TEXT("PauseMenu.ModalPanel"))));
	AddSlot(854.f, 148.f, 212.f, 90.f,
		FT66FlatStyle::MakeFlatLabel(
			NSLOCTEXT("T66.PauseMenu", "PausedTitle", "PAUSED"),
			ET66FlatLabelRole::Title,
			ETextJustify::Center,
			PauseMenuTag(TEXT("PauseMenu.Title"))));

	AddSlot(712.f, 264.f, 496.f, 84.f, MakePauseMenuButton(ResumeText, FOnClicked::CreateUObject(this, &UT66PauseMenuScreen::HandleResumeClicked), ET66FlatState::Selected, PauseMenuTag(TEXT("PauseMenu.ResumeButton"))));
	AddSlot(712.f, 365.f, 496.f, 84.f, MakePauseMenuButton(SaveAndQuitText, FOnClicked::CreateUObject(this, &UT66PauseMenuScreen::HandleSaveAndQuitClicked), ET66FlatState::Default, PauseMenuTag(TEXT("PauseMenu.SaveAndQuitButton"))));
	AddSlot(712.f, 466.f, 496.f, 84.f, MakePauseMenuButton(QuitText, FOnClicked::CreateUObject(this, &UT66PauseMenuScreen::HandleQuitClicked), ET66FlatState::Default, PauseMenuTag(TEXT("PauseMenu.QuitButton"))));
	AddSlot(712.f, 567.f, 496.f, 84.f, MakePauseMenuButton(RestartText, FOnClicked::CreateUObject(this, &UT66PauseMenuScreen::HandleRestartClicked), ET66FlatState::Default, PauseMenuTag(TEXT("PauseMenu.RestartButton"))));
	AddSlot(712.f, 668.f, 496.f, 84.f, MakePauseMenuButton(SettingsText, FOnClicked::CreateUObject(this, &UT66PauseMenuScreen::HandleSettingsClicked), ET66FlatState::Default, PauseMenuTag(TEXT("PauseMenu.SettingsButton"))));
	AddSlot(712.f, 769.f, 496.f, 84.f, MakePauseMenuButton(AchievementsText, FOnClicked::CreateUObject(this, &UT66PauseMenuScreen::HandleAchievementsClicked), ET66FlatState::Default, PauseMenuTag(TEXT("PauseMenu.AchievementsButton"))));
	AddSlot(712.f, 870.f, 496.f, 84.f, MakePauseMenuButton(LeaderboardText, FOnClicked::CreateUObject(this, &UT66PauseMenuScreen::HandleLeaderboardClicked), ET66FlatState::Default, PauseMenuTag(TEXT("PauseMenu.LeaderboardButton"))));

	if (bLeaderboardModalOpen)
	{
		AddSlot(0.f, 0.f, 1920.f, 1080.f,
			FT66FlatStyle::AttachMetadata(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.74f))
				.Padding(0.f),
				PauseMenuTag(TEXT("PauseMenu.LeaderboardModal.Scrim")),
				TEXT("Scrim"),
				ET66FlatState::Default));
		AddSlot(646.f, 48.f, 628.f, 984.f,
			FT66FlatStyle::MakeFlatPanel(
				ET66FlatState::Default,
				FMargin(0.f),
				SNullWidget::NullWidget,
				nullptr,
				PauseMenuTag(TEXT("PauseMenu.LeaderboardModal.Panel"))));
		AddSlot(728.f, 78.f, 330.f, 60.f,
			FT66FlatStyle::MakeFlatLabel(
				NSLOCTEXT("T66.PauseMenu", "LeaderboardModalTitle", "LEADERBOARD"),
				ET66FlatLabelRole::Header,
				ETextJustify::Left,
				PauseMenuTag(TEXT("PauseMenu.LeaderboardModal.Title"))));
		AddSlot(1060.f, 72.f, 200.f, 64.f,
			MakePauseMenuCompactButton(
				NSLOCTEXT("T66.Common", "Back", "BACK"),
				FOnClicked::CreateUObject(this, &UT66PauseMenuScreen::HandleLeaderboardBackClicked),
				ET66FlatState::Default,
				PauseMenuTag(TEXT("PauseMenu.LeaderboardModal.BackButton"))));
		AddSlot(722.f, 142.f, 476.f, 884.f,
			SAssignNew(FlatLeaderboardPanel, ST66FlatLeaderboardPanel)
			.LocalizationSubsystem(Loc)
			.LeaderboardSubsystem(Leaderboard)
			.UIManager(UIManager)
			.TagPrefix(TEXT("PauseMenu.LeaderboardModal")));
	}

	const TSharedRef<SWidget> RootContent = SNew(SBox)
		.WidthOverride(1920.f)
		.HeightOverride(1080.f)
		[
			Canvas
		];

	return FT66FlatStyle::AttachMetadata(
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				RootContent
			]
		],
		PauseMenuTag(TEXT("PauseMenu.Root")),
		TEXT("Root"),
		ET66FlatState::Default);
}

FReply UT66PauseMenuScreen::HandleResumeClicked() { OnResumeClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSaveAndQuitClicked() { OnSaveAndQuitClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleQuitClicked() { OnQuitClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSettingsClicked() { OnSettingsClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleAchievementsClicked() { OnAchievementsClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleLeaderboardClicked() { OnLeaderboardClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleLeaderboardBackClicked()
{
	bLeaderboardModalOpen = false;
	FlatLeaderboardPanel.Reset();
	ForceRebuildSlate();
	return FReply::Handled();
}

void UT66PauseMenuScreen::OnResumeClicked()
{
	bLeaderboardModalOpen = false;
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

	PC->SetPause(false);

	if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
	{
		if (UWorld* World = GetWorld(); World && World->GetNetMode() == NM_Client)
		{
			PC->ServerRequestPartySaveAndReturnToFrontend();
		}
		else
		{
			SessionSubsystem->SaveCurrentRunAndReturnToFrontend();
		}
	}
}

void UT66PauseMenuScreen::OnQuitClicked()
{
	AT66PlayerController* PC = GetT66PlayerController();
	if (PC)
	{
		PC->SetPause(false);
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66ShutdownSubsystem* Shutdown = GI->GetSubsystem<UT66ShutdownSubsystem>())
		{
			Shutdown->RequestQuitGame(ET66ShutdownReason::UserQuit, 0);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Shutdown] Pause menu quit fallback to UKismetSystemLibrary::QuitGame because UT66ShutdownSubsystem is unavailable."));
	UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}

void UT66PauseMenuScreen::OnRestartClicked()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->BeginNewRun();
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	if (UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
	{
		T66GI->TransitionToGameplayLevel();
		return;
	}

	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetGameplayLevelName());
}

void UT66PauseMenuScreen::OnSettingsClicked()
{
	bLeaderboardModalOpen = false;
	ShowModal(ET66ScreenType::Settings);
}

void UT66PauseMenuScreen::OnAchievementsClicked()
{
	bLeaderboardModalOpen = false;
	ShowModal(ET66ScreenType::Achievements);
}

void UT66PauseMenuScreen::OnLeaderboardClicked()
{
	bLeaderboardModalOpen = true;
	ForceRebuildSlate();
}

bool UT66PauseMenuScreen::HandleBackAction()
{
	if (bLeaderboardModalOpen)
	{
		bLeaderboardModalOpen = false;
		FlatLeaderboardPanel.Reset();
		ForceRebuildSlate();
		return true;
	}

	return false;
}

