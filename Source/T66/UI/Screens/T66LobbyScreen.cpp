// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LobbyScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66UITexturePoolSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"

UT66LobbyScreen::UT66LobbyScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::Lobby;
	bIsModal = false;
}

void UT66LobbyScreen::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	// Rebuild so left panel shows updated hero portrait when returning from Hero Selection.
	FT66Style::DeferRebuild(this);
}

void UT66LobbyScreen::OnScreenDeactivated_Implementation()
{
	Super::OnScreenDeactivated_Implementation();
	// Do not clear bHeroSelectionFromLobby here: we're often navigating to Hero Selection, and
	// ShowScreen calls OnScreenDeactivated before Hero Selection's BuildSlateUI() runs. Clearing
	// here would make Hero Selection always show solo layout. Flag is cleared in LobbyBackConfirmModal
	// when user confirms Leave (navigate back to Party Size Picker).
}

int32 UT66LobbyScreen::GetPartySlotCount() const
{
	// Co-op lobby always 3 slots (Trio).
	return 3;
}

void UT66LobbyScreen::StartRunFromLobby()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->bStageBoostPending = (GI->SelectedDifficulty != ET66Difficulty::Easy);
		GI->ProceduralTerrainSeed = FMath::Rand();
	}
	if (UIManager) UIManager->HideAllUI();
	if (GI)
	{
		GI->TransitionToGameplayLevel();
	}
	else
	{
		UGameplayStatics::OpenLevel(this, FName(TEXT("GameplayLevel")));
	}
}

TSharedRef<SWidget> UT66LobbyScreen::BuildSlateUI()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI);
	UT66LocalizationSubsystem* Loc = GI ? GI->GetSubsystem<UT66LocalizationSubsystem>() : nullptr;

	FText TitleText = Loc ? Loc->GetText_LobbyTitle() : NSLOCTEXT("T66.Lobby", "Title", "LOBBY");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	FText SelectHeroText = Loc ? Loc->GetText_LobbySelectHero() : NSLOCTEXT("T66.Lobby", "SelectHero", "SELECT HERO");
	FText ReadyCheckText = Loc ? Loc->GetText_LobbyReadyCheck() : NSLOCTEXT("T66.Lobby", "ReadyCheck", "READY CHECK");
	FText ReadyToStartText = Loc ? Loc->GetText_LobbyReadyToStart() : NSLOCTEXT("T66.Lobby", "ReadyToStart", "Ready to start?");
	FText EnterText = Loc ? Loc->GetText_EnterTheTribulation() : NSLOCTEXT("T66.HeroSelection", "EnterTheTribulation", "ENTER THE TRIBULATION");
	FText InviteText = Loc ? Loc->GetText_LobbyInviteFriend() : NSLOCTEXT("T66.Lobby", "InviteFriend", "INVITE");
	FText YouText = Loc ? Loc->GetText_LobbyYou() : NSLOCTEXT("T66.Lobby", "You", "You");
	FText WaitingText = Loc ? Loc->GetText_LobbyWaitingForPlayer() : NSLOCTEXT("T66.Lobby", "WaitingForPlayer", "Waiting for player...");
	FText FriendsTitleText = Loc ? Loc->GetText_LobbyFriends() : NSLOCTEXT("T66.Lobby", "Friends", "FRIENDS");
	FText FriendsStubText = NSLOCTEXT("T66.Lobby", "FriendsStub", "Friends list will appear when connected to Steam.");
	FText ReadyText = Loc ? Loc->GetText_LobbyReady() : NSLOCTEXT("T66.Lobby", "Ready", "READY");
	FText NotReadyText = Loc ? Loc->GetText_LobbyNotReady() : NSLOCTEXT("T66.Lobby", "NotReady", "Not Ready");

	// Button styles no longer needed: all buttons go through FT66Style::MakeButton

	// Local player hero portrait for left panel (slot 0)
	if (!LocalPlayerHeroPortraitBrush.IsValid())
	{
		LocalPlayerHeroPortraitBrush = MakeShared<FSlateBrush>();
		LocalPlayerHeroPortraitBrush->DrawAs = ESlateBrushDrawType::Image;
		LocalPlayerHeroPortraitBrush->ImageSize = FVector2D(64.f, 64.f);
	}
	if (T66GI && T66GI->GetSubsystem<UT66UITexturePoolSubsystem>())
	{
		FHeroData HeroData;
		TSoftObjectPtr<UTexture2D> HeroPortraitSoft;
		if (T66GI->GetHeroData(T66GI->SelectedHeroID, HeroData) && !HeroData.Portrait.IsNull())
		{
			HeroPortraitSoft = HeroData.Portrait;
		}
		T66SlateTexture::BindSharedBrushAsync(
			T66GI->GetSubsystem<UT66UITexturePoolSubsystem>(),
			HeroPortraitSoft, this, LocalPlayerHeroPortraitBrush,
			FName(TEXT("LobbyHeroPortrait")), true);
	}

	// Local player companion portrait for left panel (slot 0)
	if (!LocalPlayerCompanionPortraitBrush.IsValid())
	{
		LocalPlayerCompanionPortraitBrush = MakeShared<FSlateBrush>();
		LocalPlayerCompanionPortraitBrush->DrawAs = ESlateBrushDrawType::Image;
		LocalPlayerCompanionPortraitBrush->ImageSize = FVector2D(48.f, 48.f);
	}
	if (T66GI && T66GI->GetSubsystem<UT66UITexturePoolSubsystem>())
	{
		FCompanionData CompData;
		TSoftObjectPtr<UTexture2D> CompPortraitSoft;
		if (!T66GI->SelectedCompanionID.IsNone() && T66GI->GetCompanionData(T66GI->SelectedCompanionID, CompData) && !CompData.Portrait.IsNull())
		{
			CompPortraitSoft = CompData.Portrait;
		}
		T66SlateTexture::BindSharedBrushAsync(
			T66GI->GetSubsystem<UT66UITexturePoolSubsystem>(),
			CompPortraitSoft, this, LocalPlayerCompanionPortraitBrush,
			FName(TEXT("LobbyCompanionPortrait")), true);
	}

	const int32 SlotCount = GetPartySlotCount();

	// Left panel: players in lobby (vertical slots) — hero + companion portrait, name, Not Ready/Ready
	TSharedRef<SVerticalBox> PlayersBox = SNew(SVerticalBox);
	for (int32 i = 0; i < SlotCount; ++i)
	{
		const bool bIsLocal = (i == 0);
		PlayersBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 6.0f)
			[
				FT66Style::MakePanel(
				SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(SBox)
						.WidthOverride(64.0f)
						.HeightOverride(64.0f)
						[
							SNew(SImage)
							.Image(LocalPlayerHeroPortraitBrush.IsValid() && bIsLocal ? LocalPlayerHeroPortraitBrush.Get() : nullptr)
							.Visibility_Lambda([this, bIsLocal]() -> EVisibility {
								if (!bIsLocal) return EVisibility::Collapsed;
								return (LocalPlayerHeroPortraitBrush.IsValid() && LocalPlayerHeroPortraitBrush->GetResourceObject()) ? EVisibility::Visible : EVisibility::Collapsed;
							})
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 12.0f, 0.0f)
					[
						SNew(SBox)
						.WidthOverride(48.0f)
						.HeightOverride(48.0f)
						[
							SNew(SImage)
							.Image(LocalPlayerCompanionPortraitBrush.IsValid() && bIsLocal ? LocalPlayerCompanionPortraitBrush.Get() : nullptr)
							.Visibility_Lambda([this, bIsLocal]() -> EVisibility {
								if (!bIsLocal) return EVisibility::Collapsed;
								return (LocalPlayerCompanionPortraitBrush.IsValid() && LocalPlayerCompanionPortraitBrush->GetResourceObject()) ? EVisibility::Visible : EVisibility::Collapsed;
							})
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(bIsLocal ? YouText : WaitingText)
							.Font(FT66Style::Tokens::FontRegular(16))
							.ColorAndOpacity(bIsLocal ? FT66Style::Tokens::Text : FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(bIsLocal ? (bReadyCheckConfirmed ? ReadyText : NotReadyText) : NotReadyText)
							.Font(FT66Style::Tokens::FontRegular(12))
							.ColorAndOpacity(bIsLocal && bReadyCheckConfirmed ? FT66Style::Tokens::Success : FT66Style::Tokens::TextMuted)
						]
					]
			,
			FT66PanelParams(ET66PanelType::Panel).SetPadding(12.0f).SetColor(FT66Style::Tokens::Panel)
			)
			];
	}

	// Right panel: friends list (stub)
	TSharedRef<SScrollBox> FriendsScroll = SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(8.0f)
		[
			SNew(STextBlock)
			.Text(FriendsStubText)
			.Font(FT66Style::Tokens::FontRegular(12))
			.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			.AutoWrapText(true)
		];

	// Difficulty options for Lobby (same list as Hero Selection)
	static const TArray<ET66Difficulty> LobbyDifficulties = {
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
		ET66Difficulty::VeryHard, ET66Difficulty::Impossible, ET66Difficulty::Perdition, ET66Difficulty::Final
	};
	LobbyDifficultyOptions.Empty();
	for (ET66Difficulty Diff : LobbyDifficulties)
	{
		FText DiffText = Loc ? Loc->GetText_Difficulty(Diff) : NSLOCTEXT("T66.Difficulty", "Unknown", "???");
		LobbyDifficultyOptions.Add(MakeShared<FString>(DiffText.ToString()));
	}
	ET66Difficulty CurrentDiff = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	int32 CurrentDiffIndex = LobbyDifficulties.IndexOfByKey(CurrentDiff);
	if (CurrentDiffIndex != INDEX_NONE && CurrentDiffIndex < LobbyDifficultyOptions.Num())
	{
		LobbyCurrentDifficultyOption = LobbyDifficultyOptions[CurrentDiffIndex];
	}
	else if (LobbyDifficultyOptions.Num() > 0)
	{
		LobbyCurrentDifficultyOption = LobbyDifficultyOptions[0];
	}

	TSharedRef<SWidget> BottomRightButtons = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 12.0f)
		[
			SNew(SBox)
			.HeightOverride(40.0f)
			.MinDesiredWidth(200.0f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&LobbyDifficultyOptions)
				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo) {
					OnLobbyDifficultyChanged(NewValue, SelectInfo);
				})
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) -> TSharedRef<SWidget> {
					return SNew(STextBlock)
						.Text(FText::FromString(*InItem))
						.Font(FT66Style::Tokens::FontRegular(12))
						.ColorAndOpacity(FT66Style::Tokens::Text);
				})
				.InitiallySelectedItem(LobbyCurrentDifficultyOption)
				[
					SNew(STextBlock)
					.Text_Lambda([this, Loc]() -> FText {
						return LobbyCurrentDifficultyOption.IsValid()
							? FText::FromString(*LobbyCurrentDifficultyOption)
							: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy"));
					})
					.Font(FT66Style::Tokens::FontBold(12))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			FT66Style::MakeButton(SelectHeroText, FOnClicked::CreateUObject(this, &UT66LobbyScreen::HandleSelectHeroClicked), ET66ButtonType::Primary, 180.f)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f)
		[
			SNew(SBox).HeightOverride(44.0f)
			[
				bReadyCheckConfirmed
					? FT66Style::MakeButton(EnterText, FOnClicked::CreateUObject(this, &UT66LobbyScreen::HandleEnterTribulationClicked), ET66ButtonType::Danger, 220.f)
					: StaticCastSharedRef<SWidget>(FT66Style::MakeButton(FT66ButtonParams(ReadyCheckText, FOnClicked::CreateUObject(this, &UT66LobbyScreen::HandleReadyCheckClicked))
						.SetMinWidth(180.f)
						.SetEnabled(TAttribute<bool>::CreateLambda([this]() {
							UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
							return GI && GI->HasHeroSelected();
						}))))
			]
		];

	TSharedRef<SWidget> MainContent = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.35f)
		.Padding(24.0f, 20.0f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 12.0f)
				[
					SNew(STextBlock)
					.Text(TitleText)
					.Font(FT66Style::Tokens::FontBold(20))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					PlayersBox
				],
				ET66PanelType::Panel,
				FMargin(16.f)
			)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.45f)
		.Padding(16.0f, 20.0f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 12.0f)
				[
					SNew(STextBlock)
					.Text(FriendsTitleText)
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					FriendsScroll
				],
				ET66PanelType::Panel,
				FMargin(16.f)
			)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.2f)
		.Padding(16.0f, 20.0f)
		.VAlign(VAlign_Bottom)
		[
			BottomRightButtons
		];

	TSharedRef<SWidget> Root = SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.03f, 1.0f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(40.0f, 40.0f, 40.0f, 80.0f)
				[
					MainContent
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(20.0f, 0.0f, 0.0f, 20.0f)
			[
				FT66Style::MakeButton(BackText, FOnClicked::CreateUObject(this, &UT66LobbyScreen::HandleBackClicked), ET66ButtonType::Neutral, 120.f)
			]
		];

	return Root;
}

void UT66LobbyScreen::OnLobbyDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid()) return;
	LobbyCurrentDifficultyOption = NewValue;
	static const TArray<ET66Difficulty> Order = {
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
		ET66Difficulty::VeryHard, ET66Difficulty::Impossible, ET66Difficulty::Perdition, ET66Difficulty::Final
	};
	int32 Idx = INDEX_NONE;
	for (int32 i = 0; i < LobbyDifficultyOptions.Num(); ++i)
	{
		if (LobbyDifficultyOptions[i].IsValid() && *LobbyDifficultyOptions[i] == *NewValue)
		{
			Idx = i;
			break;
		}
	}
	if (Idx != INDEX_NONE && Idx < Order.Num())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			GI->SelectedDifficulty = Order[Idx];
		}
	}
	RefreshScreen();
}

FReply UT66LobbyScreen::HandleSelectHeroClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bHeroSelectionFromLobby = true;
	}
	NavigateTo(ET66ScreenType::HeroSelection);
	return FReply::Handled();
}

FReply UT66LobbyScreen::HandleReadyCheckClicked()
{
	ShowModal(ET66ScreenType::LobbyReadyCheck);
	return FReply::Handled();
}

FReply UT66LobbyScreen::HandleEnterTribulationClicked()
{
	OnEnterTribulationClicked();
	return FReply::Handled();
}

FReply UT66LobbyScreen::HandleInviteFriendClicked()
{
	OnInviteFriendClicked();
	return FReply::Handled();
}

FReply UT66LobbyScreen::HandleBackClicked()
{
	ShowModal(ET66ScreenType::LobbyBackConfirm);
	return FReply::Handled();
}

void UT66LobbyScreen::OnSelectHeroClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bHeroSelectionFromLobby = true;
	}
	NavigateTo(ET66ScreenType::HeroSelection);
}

void UT66LobbyScreen::OnReadyCheckClicked()
{
	ShowModal(ET66ScreenType::LobbyReadyCheck);
}

void UT66LobbyScreen::SetReadyCheckConfirmed(bool bConfirmed)
{
	bReadyCheckConfirmed = bConfirmed;
}

void UT66LobbyScreen::RefreshScreen_Implementation()
{
	// Rebuild via DeferRebuild so the viewport shows the updated widget (e.g. READY label, Enter button).
	FT66Style::DeferRebuild(this);
}

void UT66LobbyScreen::OnEnterTribulationClicked()
{
	StartRunFromLobby();
}

void UT66LobbyScreen::OnInviteFriendClicked()
{
	// Stub: Steam invite not connected yet.
}

void UT66LobbyScreen::OnBackClicked()
{
	ShowModal(ET66ScreenType::LobbyBackConfirm);
}
