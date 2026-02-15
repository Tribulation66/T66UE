// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PauseMenuScreen.h"
#include "UI/T66UIManager.h"
#include "UI/T66StatsPanelSlate.h"
#include "Gameplay/T66PlayerController.h"
#include "Core/T66GameInstance.h"
#include "Core/T66AchievementsSubsystem.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66RunStateSubsystem.h"
#include "Core/T66SaveSubsystem.h"
#include "Core/T66RunSaveGame.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Data/T66DataTypes.h"

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
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;
	UT66RunStateSubsystem* RunState = GI ? GI->GetSubsystem<UT66RunStateSubsystem>() : nullptr;

	FText ResumeText = Loc ? Loc->GetText_Resume() : NSLOCTEXT("T66.PauseMenu", "Resume", "RESUME GAME");
	FText SaveAndQuitText = Loc ? Loc->GetText_SaveAndQuit() : NSLOCTEXT("T66.PauseMenu", "SaveAndQuit", "SAVE AND QUIT GAME");
	FText RestartText = Loc ? Loc->GetText_Restart() : NSLOCTEXT("T66.PauseMenu", "Restart", "RESTART");
	FText SettingsText = Loc ? Loc->GetText_Settings() : NSLOCTEXT("T66.PauseMenu", "Settings", "SETTINGS");
	FText ReportBugText = Loc ? Loc->GetText_ReportBug() : NSLOCTEXT("T66.PauseMenu", "ReportBug", "REPORT BUG");
	FText AchievementsText = Loc ? Loc->GetText_Achievements() : NSLOCTEXT("T66.Achievements", "Title", "ACHIEVEMENTS");
	FText InProgressText = NSLOCTEXT("T66.PauseMenu", "InProgress", "In progress");

	auto MakePauseButton = [this](const FText& Text, FReply (UT66PauseMenuScreen::*ClickFunc)(), ET66ButtonType Type) -> TSharedRef<SWidget>
	{
		return FT66Style::MakeButton(
			FT66ButtonParams(Text, FOnClicked::CreateUObject(this, ClickFunc), Type)
			.SetFontSize(44)
			.SetPadding(FMargin(14.f))
			.SetMinWidth(0.f));
	};

	TSharedRef<SWidget> RightPanel = FT66Style::MakePanel(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 0.0f, 0.0f, 24.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.PauseMenu", "PausedTitle", "PAUSED"))
				.Font(FT66Style::Tokens::FontBold(36))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 6.0f)
		[ MakePauseButton(ResumeText, &UT66PauseMenuScreen::HandleResumeClicked, ET66ButtonType::Success) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 6.0f)
		[ MakePauseButton(SaveAndQuitText, &UT66PauseMenuScreen::HandleSaveAndQuitClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 6.0f)
		[ MakePauseButton(RestartText, &UT66PauseMenuScreen::HandleRestartClicked, ET66ButtonType::Danger) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 6.0f)
		[ MakePauseButton(SettingsText, &UT66PauseMenuScreen::HandleSettingsClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 6.0f)
		[ MakePauseButton(ReportBugText, &UT66PauseMenuScreen::HandleReportBugClicked, ET66ButtonType::Neutral) ]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 6.0f)
		[ MakePauseButton(AchievementsText, &UT66PauseMenuScreen::HandleAchievementsClicked, ET66ButtonType::Neutral) ]
		,
		FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(FT66Style::Tokens::Space8, FT66Style::Tokens::Space6)));

	// Achievements in progress: >1% completion, sorted by closest to complete
	TArray<FAchievementData> InProgressList;
	if (UT66AchievementsSubsystem* Ach = GI ? GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr)
	{
		TArray<FAchievementData> All = Ach->GetAllAchievements();
		for (const FAchievementData& A : All)
		{
			if (A.RequirementCount <= 0 || A.bIsUnlocked) continue;
			const int32 Pct = (A.RequirementCount > 0) ? (A.CurrentProgress * 100 / A.RequirementCount) : 0;
			if (Pct > 1) InProgressList.Add(A);
		}
		InProgressList.Sort([](const FAchievementData& A, const FAchievementData& B)
		{
			const float ProgA = A.RequirementCount > 0 ? (float)A.CurrentProgress / (float)A.RequirementCount : 0.f;
			const float ProgB = B.RequirementCount > 0 ? (float)B.CurrentProgress / (float)B.RequirementCount : 0.f;
			return ProgA > ProgB;
		});
	}

	TSharedRef<SVerticalBox> InProgressBox = SNew(SVerticalBox);
	for (const FAchievementData& A : InProgressList)
	{
		const FString ProgressStr = FString::Printf(TEXT("%d/%d"), FMath::Min(A.CurrentProgress, A.RequirementCount), A.RequirementCount);
		InProgressBox->AddSlot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FT66Style::Tokens::Panel)
			.Padding(FMargin(8.f, 6.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(A.DisplayName)
					.Font(FT66Style::Tokens::FontBold(12))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.AutoWrapText(true)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.f, 0.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(ProgressStr))
					.Font(FT66Style::Tokens::FontRegular(11))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
			]
		];
	}

	TSharedRef<SWidget> RightColumn = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			RightPanel
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0.f, 12.f, 0.f, 0.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(InProgressText)
					.Font(FT66Style::Tokens::FontBold(16))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						InProgressBox
					]
				],
				FT66PanelParams(ET66PanelType::Panel2).SetPadding(FMargin(FT66Style::Tokens::Space4)))
		];

	const FLinearColor ScrimOpaque(FT66Style::Tokens::Scrim.R, FT66Style::Tokens::Scrim.G, FT66Style::Tokens::Scrim.B, 1.0f);
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ScrimOpaque)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0.f, 0.f, 16.f, 0.f)
				[
					T66StatsPanelSlate::MakeEssentialStatsPanel(RunState, Loc, FT66Style::Tokens::NPCVendorStatsPanelWidth, true)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					RightColumn
				]
			]
		];
}

FReply UT66PauseMenuScreen::HandleResumeClicked() { OnResumeClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSaveAndQuitClicked() { OnSaveAndQuitClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleRestartClicked() { OnRestartClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleSettingsClicked() { OnSettingsClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleReportBugClicked() { OnReportBugClicked(); return FReply::Handled(); }
FReply UT66PauseMenuScreen::HandleAchievementsClicked() { OnAchievementsClicked(); return FReply::Handled(); }

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

	if (UT66RunStateSubsystem* RunState = GI->GetSubsystem<UT66RunStateSubsystem>())
	{
		SaveObj->StageReached = RunState->GetCurrentStage();
		SaveObj->EquippedIdols = RunState->GetEquippedIdols();
	}

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

void UT66PauseMenuScreen::OnAchievementsClicked()
{
	ShowModal(ET66ScreenType::Achievements);
}
