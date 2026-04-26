// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PauseMenuScreen.h"

#include "Core/T66GameInstance.h"
#include "Core/T66IdolManagerSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66Rarity.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66DamageLogSubsystem.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Gameplay/T66PlayerController.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/T66SlateTextureHelpers.h"
#include "UI/T66StatsPanelSlate.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66RuntimeUIBrushAccess.h"
#include "UI/Style/T66RuntimeUITextureAccess.h"
#include "UI/Style/T66Style.h"

#include "Data/T66DataTypes.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/DataTable.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr float PauseMenuButtonHeight = 58.0f;

	const FSlateBrush* ResolvePauseMenuSettingsReferenceBrush(
		T66RuntimeUIBrushAccess::FOptionalTextureBrush& Entry,
		const FString& RelativePath,
		const FMargin& Margin,
		const TCHAR* DebugLabel)
	{
		return T66RuntimeUIBrushAccess::ResolveOptionalTextureBrush(
			Entry,
			nullptr,
			T66RuntimeUITextureAccess::MakeProjectDirPath(RelativePath),
			Margin,
			DebugLabel);
	}

	const TCHAR* GetPauseMenuButtonPlateFile(ET66ButtonType Type)
	{
		switch (Type)
		{
		case ET66ButtonType::Primary:
		case ET66ButtonType::Success:
		case ET66ButtonType::ToggleActive:
			return TEXT("runflow_pause_button_primary_normal.png");
		case ET66ButtonType::Danger:
			return TEXT("runflow_pause_button_danger_normal.png");
		default:
			return TEXT("runflow_pause_button_neutral_normal.png");
		}
	}

	const FSlateBrush* GetPauseMenuButtonPlateBrush(ET66ButtonType Type)
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Neutral;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Success;
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Danger;

		T66RuntimeUIBrushAccess::FOptionalTextureBrush* Entry = &Neutral;
		if (Type == ET66ButtonType::Primary || Type == ET66ButtonType::Success || Type == ET66ButtonType::ToggleActive)
		{
			Entry = &Success;
		}
		else if (Type == ET66ButtonType::Danger)
		{
			Entry = &Danger;
		}

		return ResolvePauseMenuSettingsReferenceBrush(
			*Entry,
			FString::Printf(TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Buttons/%s"), GetPauseMenuButtonPlateFile(Type)),
			FMargin(0.16f, 0.28f, 0.16f, 0.28f),
			TEXT("PauseMenuButtonPlate"));
	}

	const FSlateBrush* GetPauseMenuShellBrush()
	{
		static T66RuntimeUIBrushAccess::FOptionalTextureBrush Shell;
		return ResolvePauseMenuSettingsReferenceBrush(
			Shell,
			TEXT("SourceAssets/UI/RunFlowReference/SheetSlices/Panels/runflow_pause_modal_shell.png"),
			FMargin(0.035f, 0.12f, 0.035f, 0.12f),
			TEXT("PauseMenuShell"));
	}

	TSharedRef<SWidget> MakePauseMenuShell(const TSharedRef<SWidget>& Content, const FMargin& Padding)
	{
		if (const FSlateBrush* ShellBrush = GetPauseMenuShellBrush())
		{
			return SNew(SBorder)
				.BorderImage(ShellBrush)
				.BorderBackgroundColor(FLinearColor::White)
				.Padding(Padding)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					Content
				];
		}

		return FT66Style::MakePanel(
			Content,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(Padding));
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

TSharedRef<SWidget> UT66PauseMenuScreen::BuildSlateUI()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GameInstance ? GameInstance->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const float ButtonColumnWidth = bDotaTheme ? 430.f : 420.f;
	const float ButtonMinWidth = bDotaTheme ? 340.f : 332.f;
	const FMargin OverlaySafePadding = FT66Style::GetSafePadding(FMargin(24.f, 24.f));

	const FText ResumeText = Loc ? Loc->GetText_Resume() : NSLOCTEXT("T66.PauseMenu", "Resume", "RESUME GAME");
	const FText SaveAndQuitText = Loc ? Loc->GetText_SaveAndQuit() : NSLOCTEXT("T66.PauseMenu", "SaveAndQuit", "SAVE AND QUIT");
	const FText RestartText = Loc ? Loc->GetText_Restart() : NSLOCTEXT("T66.PauseMenu", "Restart", "RESTART");
	const FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.PauseMenu", "Settings", "SETTINGS");
	const FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
	const FText LeaderboardText = NSLOCTEXT("T66.PauseMenu", "Leaderboard", "LEADERBOARD");

	auto MakePauseButton = [this, bDotaTheme, ButtonMinWidth](const FText& Text, FReply (UT66PauseMenuScreen::*ClickFunc)(), ET66ButtonType Type) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Fill)
			.Padding(FMargin(0.f, bDotaTheme ? 4.f : 6.f))
			[
				FT66Style::MakeButton(
					FT66ButtonParams(Text, FOnClicked::CreateUObject(this, ClickFunc), Type)
					.SetFontSize(bDotaTheme ? 20 : 28)
					.SetFontWeight(bDotaTheme ? TEXT("Regular") : TEXT("Bold"))
					.SetPadding(bDotaTheme ? FMargin(20.f, 9.f, 20.f, 8.f) : FMargin(18.f, 12.f))
					.SetMinWidth(ButtonMinWidth)
					.SetHeight(bDotaTheme ? PauseMenuButtonHeight : 66.f)
					.SetUseGlow(false)
					.SetUseDotaPlateOverlay(true)
					.SetDotaPlateOverrideBrush(GetPauseMenuButtonPlateBrush(Type)))
			];
	};

	FSlateFontInfo PauseTitleFont = FT66Style::Tokens::FontBold(bDotaTheme ? 48 : 44);
	if (bDotaTheme)
	{
		PauseTitleFont.LetterSpacing = 120;
	}

	TSharedRef<SWidget> ButtonsPanel = MakePauseMenuShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 18.f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("T66.PauseMenu", "PausedTitle", "PAUSED"))
			.Font(PauseTitleFont)
			.ColorAndOpacity(FT66Style::Tokens::Text)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(ResumeText, &UT66PauseMenuScreen::HandleResumeClicked, ET66ButtonType::Success) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(SaveAndQuitText, &UT66PauseMenuScreen::HandleSaveAndQuitClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(RestartText, &UT66PauseMenuScreen::HandleRestartClicked, ET66ButtonType::Danger) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(SettingsText, &UT66PauseMenuScreen::HandleSettingsClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(AchievementsText, &UT66PauseMenuScreen::HandleAchievementsClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
		[ MakePauseButton(LeaderboardText, &UT66PauseMenuScreen::HandleLeaderboardClicked, ET66ButtonType::Neutral) ],
		FMargin(38.f, 34.f, 38.f, 38.f));

	return T66ScreenSlateHelpers::MakeCenteredScrimModal(
		SNew(SBox)
		.WidthOverride(ButtonColumnWidth)
		[
			ButtonsPanel
		],
		OverlaySafePadding,
		0.0f,
		0.0f,
		true);
}

FReply UT66PauseMenuScreen::HandleResumeClicked() { OnResumeClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSaveAndQuitClicked() { OnSaveAndQuitClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSettingsClicked() { OnSettingsClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleAchievementsClicked() { OnAchievementsClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleLeaderboardClicked() { OnLeaderboardClicked(); return FReply::Handled(); }

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

void UT66PauseMenuScreen::OnRestartClicked()
{
	UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr)
	{
		RunState->ResetForNewRun();
	}
	if (UT66DamageLogSubsystem* DamageLog = GI ? GI->GetSubsystem<UT66DamageLogSubsystem>() : nullptr)
	{
		DamageLog->ResetForNewRun();
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC) PC->SetPause(false);
	UGameplayStatics::OpenLevel(this, UT66GameInstance::GetTribulationEntryLevelName());
}

void UT66PauseMenuScreen::OnSettingsClicked()
{
	ShowModal(ET66ScreenType::Settings);
}

void UT66PauseMenuScreen::OnAchievementsClicked()
{
	ShowModal(ET66ScreenType::Achievements);
}

void UT66PauseMenuScreen::OnLeaderboardClicked()
{
	ShowModal(ET66ScreenType::AccountStatus);
}

