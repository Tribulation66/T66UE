// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66LobbyScreen.h"
#include "UI/T66UIManager.h"
#include "UI/Style/T66Style.h"
#include "UI/T66SlateTextureHelpers.h"
#include "Core/T66GameInstance.h"
#include "Core/T66LocalizationSubsystem.h"
#include "Core/T66PartySubsystem.h"
#include "Core/T66SessionSubsystem.h"
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

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartyStateChangedHandle = PartySubsystem->OnPartyStateChanged().AddUObject(this, &UT66LobbyScreen::HandlePartyStateChanged);
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionStateChangedHandle = SessionSubsystem->OnSessionStateChanged().AddUObject(this, &UT66LobbyScreen::HandleSessionStateChanged);
			SessionSubsystem->HandleLobbyScreenActivated();
		}
	}

	FT66Style::DeferRebuild(this);
}

void UT66LobbyScreen::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->OnPartyStateChanged().Remove(PartyStateChangedHandle);
		}

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->OnSessionStateChanged().Remove(SessionStateChangedHandle);
		}
	}

	Super::OnScreenDeactivated_Implementation();
}

int32 UT66LobbyScreen::GetPartySlotCount() const
{
	if (const UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (const UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			int32 DesiredSlots = PartySubsystem->GetPartyMemberCount();
			if (const UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
			{
				DesiredSlots = FMath::Max(DesiredSlots, SessionSubsystem->GetMaxPartyMembers());
			}
			return FMath::Clamp(DesiredSlots, 1, 4);
		}

		if (const UT66GameInstance* T66GI = Cast<UT66GameInstance>(GI))
		{
			switch (T66GI->SelectedPartySize)
			{
			case ET66PartySize::Quad: return 4;
			case ET66PartySize::Trio: return 3;
			case ET66PartySize::Duo: return 2;
			case ET66PartySize::Solo:
			default:
				return 1;
			}
		}
	}

	return 1;
}

void UT66LobbyScreen::StartRunFromLobby()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (GI)
	{
		GI->bStageCatchUpPending = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		GI->bLoadAsPreview = false;
		GI->RunSeed = FMath::Rand();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}

	if (SessionSubsystem && SessionSubsystem->IsPartySessionActive())
	{
		if (UIManager)
		{
			UIManager->HideAllUI();
		}
		SessionSubsystem->StartGameplayTravel();
		return;
	}

	if (UIManager) UIManager->HideAllUI();
	if (GI)
	{
		GI->TransitionToGameplayLevel();
	}
	else
	{
		UGameplayStatics::OpenLevel(this, UT66GameInstance::GetTribulationEntryLevelName());
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
	FText EnterText = Loc ? Loc->GetText_EnterTheTribulation() : NSLOCTEXT("T66.HeroSelection", "EnterTheTribulation", "ENTER THE TRIBULATION");
	FText InviteText = Loc ? Loc->GetText_LobbyInviteFriend() : NSLOCTEXT("T66.Lobby", "InviteFriend", "INVITE");
	FText WaitingForPlayerText = Loc ? Loc->GetText_LobbyWaitingForPlayer() : NSLOCTEXT("T66.Lobby", "WaitingForPlayer", "Waiting for player...");
	FText FriendsTitleText = Loc ? Loc->GetText_LobbyFriends() : NSLOCTEXT("T66.Lobby", "Friends", "FRIENDS");
	FText FriendsStubText = NSLOCTEXT("T66.Lobby", "FriendsStub", "Your Steam friends will appear here when Steam is available.");
	FText ReadyText = Loc ? Loc->GetText_LobbyReady() : NSLOCTEXT("T66.Lobby", "Ready", "READY");
	FText NotReadyText = Loc ? Loc->GetText_LobbyNotReady() : NSLOCTEXT("T66.Lobby", "NotReady", "Not Ready");
	const FText InPartyText = NSLOCTEXT("T66.Lobby", "InParty", "In Party");
	const FText HostText = NSLOCTEXT("T66.Lobby", "Host", "Host");
	const FText OfflineText = NSLOCTEXT("T66.Lobby", "Offline", "OFFLINE");
	const FText WaitingForHostText = NSLOCTEXT("T66.Lobby", "WaitingForHost", "WAITING FOR HOST");

	UT66PartySubsystem* PartySubsystem = GI ? GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const TArray<FT66PartyMemberEntry> PartyMembers = PartySubsystem ? PartySubsystem->GetPartyMembers() : TArray<FT66PartyMemberEntry>();
	const TArray<FT66PartyFriendEntry> Friends = PartySubsystem ? PartySubsystem->GetFriends() : TArray<FT66PartyFriendEntry>();
	const FString SessionStatusText = SessionSubsystem ? SessionSubsystem->GetLastStatusText() : FString();
	const bool bLocalIsHost = IsLocalPlayerHost();

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
		if (T66GI->GetHeroData(T66GI->SelectedHeroID, HeroData))
		{
			HeroPortraitSoft = T66GI->ResolveHeroPortrait(HeroData, T66GI->SelectedHeroBodyType, ET66HeroPortraitVariant::Half);
		}
		T66SlateTexture::BindSharedBrushAsync(
			T66GI->GetSubsystem<UT66UITexturePoolSubsystem>(),
			HeroPortraitSoft, this, LocalPlayerHeroPortraitBrush,
			FName(TEXT("LobbyHeroPortrait")), true);
	}

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

	TSharedRef<SVerticalBox> PlayersBox = SNew(SVerticalBox);
	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		const FT66PartyMemberEntry* PartyMember = PartyMembers.IsValidIndex(Index) ? &PartyMembers[Index] : nullptr;
		const bool bIsLocal = PartyMember ? PartyMember->bIsLocal : false;
		const FString DisplayName = PartyMember ? PartyMember->DisplayName : WaitingForPlayerText.ToString();
		const FString Initial = DisplayName.IsEmpty() ? TEXT("?") : DisplayName.Left(1).ToUpper();
		const FText TopLineText = PartyMember ? FText::FromString(DisplayName) : WaitingForPlayerText;
		const FText BottomLineText = PartyMember
			? (PartyMember->bIsPartyHost ? HostText : (PartyMember->bReady ? ReadyText : NotReadyText))
			: NotReadyText;

		TSharedRef<SWidget> HeroPortraitWidget =
			bIsLocal
			? StaticCastSharedRef<SWidget>(
				SNew(SImage)
				.Image(LocalPlayerHeroPortraitBrush.IsValid() ? LocalPlayerHeroPortraitBrush.Get() : nullptr)
				.Visibility_Lambda([this]() -> EVisibility
				{
					return (LocalPlayerHeroPortraitBrush.IsValid() && LocalPlayerHeroPortraitBrush->GetResourceObject())
						? EVisibility::Visible
						: EVisibility::Collapsed;
				}))
			: StaticCastSharedRef<SWidget>(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel2)
				.Padding(0.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Initial))
					.Font(FT66Style::Tokens::FontBold(24))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
				]);

		TSharedRef<SWidget> CompanionWidget =
			bIsLocal
			? StaticCastSharedRef<SWidget>(
				SNew(SImage)
				.Image(LocalPlayerCompanionPortraitBrush.IsValid() ? LocalPlayerCompanionPortraitBrush.Get() : nullptr)
				.Visibility_Lambda([this]() -> EVisibility
				{
					return (LocalPlayerCompanionPortraitBrush.IsValid() && LocalPlayerCompanionPortraitBrush->GetResourceObject())
						? EVisibility::Visible
						: EVisibility::Collapsed;
				}))
			: StaticCastSharedRef<SWidget>(
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FT66Style::Tokens::Panel));

		PlayersBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, 6.f)
			[
				FT66Style::MakePanel(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(64.f)
						.HeightOverride(64.f)
						[
							HeroPortraitWidget
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(48.f)
						.HeightOverride(48.f)
						[
							CompanionWidget
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(TopLineText)
							.Font(FT66Style::Tokens::FontRegular(16))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text(BottomLineText)
							.Font(FT66Style::Tokens::FontRegular(12))
							.ColorAndOpacity((PartyMember && PartyMember->bReady) ? FT66Style::Tokens::Success : FT66Style::Tokens::TextMuted)
						]
					],
					FT66PanelParams(ET66PanelType::Panel).SetPadding(12.f).SetColor(FT66Style::Tokens::Panel))
			];
	}

	TSharedRef<SVerticalBox> FriendRows = SNew(SVerticalBox);
	if (Friends.Num() == 0)
	{
		FriendRows->AddSlot()
			.AutoHeight()
			.Padding(8.f)
			[
				SNew(STextBlock)
				.Text(FriendsStubText)
				.Font(FT66Style::Tokens::FontRegular(12))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				.AutoWrapText(true)
			];
	}
	else
	{
		for (const FT66PartyFriendEntry& Friend : Friends)
		{
			const bool bInParty = PartySubsystem && PartySubsystem->IsFriendInParty(Friend.PlayerId);
			const bool bCanInvite = SessionSubsystem
				&& SessionSubsystem->CanSendInvites()
				&& bLocalIsHost
				&& Friend.bOnline
				&& !bInParty
				&& PartySubsystem
				&& PartySubsystem->GetPartyMemberCount() < SessionSubsystem->GetMaxPartyMembers();
			const FText ActionText = bInParty ? InPartyText : (Friend.bOnline ? InviteText : OfflineText);

			FriendRows->AddSlot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 10.f)
				[
					FT66Style::MakePanel(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(FText::FromString(Friend.DisplayName))
								.Font(FT66Style::Tokens::FontRegular(15))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 2.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(bInParty ? InPartyText : FText::FromString(Friend.PresenceText))
								.Font(FT66Style::Tokens::FontRegular(12))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(12.f, 0.f, 0.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									ActionText,
									FOnClicked::CreateLambda([this, PartySubsystem, PlayerId = Friend.PlayerId]()
									{
										if (PartySubsystem)
										{
											PartySubsystem->InviteFriend(PlayerId);
										}

										FT66Style::DeferRebuild(this);
										return FReply::Handled();
									}),
									ET66ButtonType::Primary)
								.SetMinWidth(120.f)
								.SetEnabled(bCanInvite))
						],
						ET66PanelType::Panel,
						FMargin(12.f))
				];
		}
	}

	TSharedRef<SScrollBox> FriendsScroll = SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(8.f)
		[
			FriendRows
		];

	static const TArray<ET66Difficulty> LobbyDifficulties = {
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
		ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};
	LobbyDifficultyOptions.Empty();
	for (ET66Difficulty Difficulty : LobbyDifficulties)
	{
		FText DifficultyText = Loc ? Loc->GetText_Difficulty(Difficulty) : NSLOCTEXT("T66.Difficulty", "Unknown", "???");
		LobbyDifficultyOptions.Add(MakeShared<FString>(DifficultyText.ToString()));
	}

	ET66Difficulty CurrentDiff = T66GI ? T66GI->SelectedDifficulty : ET66Difficulty::Easy;
	const int32 CurrentDiffIndex = LobbyDifficulties.IndexOfByKey(CurrentDiff);
	if (CurrentDiffIndex != INDEX_NONE && CurrentDiffIndex < LobbyDifficultyOptions.Num())
	{
		LobbyCurrentDifficultyOption = LobbyDifficultyOptions[CurrentDiffIndex];
	}
	else if (LobbyDifficultyOptions.Num() > 0)
	{
		LobbyCurrentDifficultyOption = LobbyDifficultyOptions[0];
	}

	TSharedRef<SWidget> StartActionWidget = StaticCastSharedRef<SWidget>(
		SNew(STextBlock)
		.Text(WaitingForHostText)
		.Font(FT66Style::Tokens::FontBold(12))
		.ColorAndOpacity(FT66Style::Tokens::TextMuted));

	if (bLocalIsHost)
	{
		if (bReadyCheckConfirmed)
		{
			StartActionWidget = FT66Style::MakeButton(
				EnterText,
				FOnClicked::CreateUObject(this, &UT66LobbyScreen::HandleEnterTribulationClicked),
				ET66ButtonType::Danger,
				220.f);
		}
		else
		{
			StartActionWidget = FT66Style::MakeButton(
				FT66ButtonParams(ReadyCheckText, FOnClicked::CreateUObject(this, &UT66LobbyScreen::HandleReadyCheckClicked))
				.SetMinWidth(180.f)
				.SetEnabled(TAttribute<bool>::CreateLambda([this]()
				{
					UT66GameInstance* LocalGI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
					return LocalGI && LocalGI->HasHeroSelected();
				})));
		}
	}

	TSharedRef<SWidget> BottomRightButtons =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 12.f)
		[
			SNew(SBox)
			.HeightOverride(40.f)
			.MinDesiredWidth(200.f)
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&LobbyDifficultyOptions)
				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
				{
					OnLobbyDifficultyChanged(NewValue, SelectInfo);
				})
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) -> TSharedRef<SWidget>
				{
					return SNew(STextBlock)
						.Text(FText::FromString(*InItem))
						.Font(FT66Style::Tokens::FontRegular(12))
						.ColorAndOpacity(FT66Style::Tokens::Text);
				})
				.InitiallySelectedItem(LobbyCurrentDifficultyOption)
				[
					SNew(STextBlock)
					.Text_Lambda([this, Loc]() -> FText
					{
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
		.Padding(0.f, 4.f)
		[
			FT66Style::MakeButton(SelectHeroText, FOnClicked::CreateUObject(this, &UT66LobbyScreen::HandleSelectHeroClicked), ET66ButtonType::Primary, 180.f)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 8.f)
		[
			SNew(SBox)
			.HeightOverride(44.f)
			[
				StartActionWidget
			]
		];

	TSharedRef<SWidget> MainContent =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.35f)
		.Padding(24.f, 20.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(STextBlock)
					.Text(TitleText)
					.Font(FT66Style::Tokens::FontBold(20))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					PlayersBox
				],
				ET66PanelType::Panel,
				FMargin(16.f))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.45f)
		.Padding(16.f, 20.f)
		[
			FT66Style::MakePanel(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					SNew(STextBlock)
					.Text(FriendsTitleText)
					.Font(FT66Style::Tokens::FontBold(18))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(SessionStatusText))
					.Font(FT66Style::Tokens::FontRegular(12))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.AutoWrapText(true)
					.Visibility(SessionStatusText.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					FriendsScroll
				],
				ET66PanelType::Panel,
				FMargin(16.f))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.2f)
		.Padding(16.f, 20.f)
		.VAlign(VAlign_Bottom)
		[
			BottomRightButtons
		];

	const TAttribute<FMargin> SafeMainPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafePadding(FMargin(40.f, 40.f, 40.f, 80.f));
	});

	const TAttribute<FMargin> SafeBackPadding = TAttribute<FMargin>::CreateLambda([]() -> FMargin
	{
		return FT66Style::GetSafePadding(FMargin(20.f, 0.f, 0.f, 20.f));
	});

	return SNew(SBorder)
		.BorderBackgroundColor(FT66Style::Background())
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				.Padding(SafeMainPadding)
				[
					MainContent
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Bottom)
			.Padding(SafeBackPadding)
			[
				FT66Style::MakeButton(BackText, FOnClicked::CreateUObject(this, &UT66LobbyScreen::HandleBackClicked), ET66ButtonType::Neutral, 120.f)
			]
		];
}

void UT66LobbyScreen::OnLobbyDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid())
	{
		return;
	}

	LobbyCurrentDifficultyOption = NewValue;
	static const TArray<ET66Difficulty> Order = {
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
		ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};

	int32 Index = INDEX_NONE;
	for (int32 OptionIndex = 0; OptionIndex < LobbyDifficultyOptions.Num(); ++OptionIndex)
	{
		if (LobbyDifficultyOptions[OptionIndex].IsValid() && *LobbyDifficultyOptions[OptionIndex] == *NewValue)
		{
			Index = OptionIndex;
			break;
		}
	}

	if (Index != INDEX_NONE && Index < Order.Num())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			GI->SelectedDifficulty = Order[Index];
		}
	}

	RefreshScreen();
}

FReply UT66LobbyScreen::HandleSelectHeroClicked()
{
	OnSelectHeroClicked();
	return FReply::Handled();
}

FReply UT66LobbyScreen::HandleReadyCheckClicked()
{
	OnReadyCheckClicked();
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

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			SessionSubsystem->SetLocalLobbyReady(bConfirmed);
		}
	}
}

void UT66LobbyScreen::RefreshScreen_Implementation()
{
	FT66Style::DeferRebuild(this);
}

void UT66LobbyScreen::OnEnterTribulationClicked()
{
	StartRunFromLobby();
}

void UT66LobbyScreen::OnInviteFriendClicked()
{
}

void UT66LobbyScreen::OnBackClicked()
{
	ShowModal(ET66ScreenType::LobbyBackConfirm);
}

void UT66LobbyScreen::HandlePartyStateChanged()
{
	RefreshScreen();
}

void UT66LobbyScreen::HandleSessionStateChanged()
{
	RefreshScreen();
}

bool UT66LobbyScreen::IsLocalPlayerHost() const
{
	if (const UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (const UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			for (const FT66PartyMemberEntry& Member : PartySubsystem->GetPartyMembers())
			{
				if (Member.bIsLocal)
				{
					return Member.bIsPartyHost;
				}
			}
		}

		if (const UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			return SessionSubsystem->IsHostingPartySession();
		}
	}

	return true;
}
