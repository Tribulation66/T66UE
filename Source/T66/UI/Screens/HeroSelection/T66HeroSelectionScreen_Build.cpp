// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

#include "TimerManager.h"
#include "Widgets/Input/SButton.h"

using namespace T66HeroSelectionPrivate;

TSharedRef<SWidget> UT66HeroSelectionScreen::BuildSlateUI()
{
	UT66LocalizationSubsystem* Loc = GetLocSubsystem();
	UT66HeroSelectionPreviewController* HeroPreviewController = GetOrCreatePreviewController();
	LastBuiltLanguage = Loc ? Loc->GetCurrentLanguage() : ET66Language::English;
	// Ensure hero list and skin state so skin list and 3D preview match (BuildSlateUI can run before OnScreenActivated).
	RefreshHeroList();
	RefreshCompanionList();
	if (AllHeroIDs.Num() > 0 && PreviewedHeroID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
			GI && !GI->SelectedHeroID.IsNone() && AllHeroIDs.Contains(GI->SelectedHeroID))
		{
			PreviewedHeroID = GI->SelectedHeroID;
		}
		else
		{
			PreviewedHeroID = AllHeroIDs[0];
		}
	}
	if (PreviewedCompanionID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			PreviewedCompanionID = GI->SelectedCompanionID;
		}
	}
	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);
	CurrentCompanionIndex = CompanionWheelIDs.IndexOfByKey(PreviewedCompanionID);
	if (CurrentCompanionIndex == INDEX_NONE)
	{
		CurrentCompanionIndex = 0;
	}
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: PreviewedHeroID=%s"), *PreviewedHeroID.ToString());
	if (!PreviewedHeroID.IsNone())
	{
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
			{
				// Check if the current SelectedHeroSkinID is owned by this hero.
				// If not, reset to this hero's equipped skin (or Default).
				const FName CurrentSkin = GI->SelectedHeroSkinID;
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: CurrentSkin (GI->SelectedHeroSkinID) = %s"), *CurrentSkin.ToString());
				
				const bool bIsNone = CurrentSkin.IsNone();
				const bool bIsDefault = CurrentSkin == FName(TEXT("Default"));
				const bool bIsOwned = SkinSub->IsHeroSkinOwned(PreviewedHeroID, CurrentSkin);
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: bIsNone=%d, bIsDefault=%d, bIsOwned=%d"),
					bIsNone ? 1 : 0, bIsDefault ? 1 : 0, bIsOwned ? 1 : 0);
				
				const bool bCurrentSkinOwned = bIsNone || bIsDefault || bIsOwned;
				if (!bCurrentSkinOwned)
				{
					const FName NewSkin = SkinSub->GetEquippedHeroSkinID(PreviewedHeroID);
					UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: %s does NOT own %s, switching to equipped: %s"),
						*PreviewedHeroID.ToString(), *CurrentSkin.ToString(), *NewSkin.ToString());
					GI->SelectedHeroSkinID = NewSkin;
				}
				else
				{
					UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: %s OWNS %s (or is Default/None), keeping it"),
						*PreviewedHeroID.ToString(), *CurrentSkin.ToString());
				}
				if (GI->SelectedHeroSkinID.IsNone())
				{
					GI->SelectedHeroSkinID = FName(TEXT("Default"));
				}
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] BuildSlateUI: final GI->SelectedHeroSkinID = %s"), *GI->SelectedHeroSkinID.ToString());
			}
		}
	}
	GeneratePlaceholderSkins();
	if (HeroPreviewController)
	{
		HeroPreviewController->EnsureCompanionPreviewBrush();
	}
	if (!HeroUltimateIconBrush.IsValid())
	{
		HeroUltimateIconBrush = MakeShared<FSlateBrush>();
		HeroUltimateIconBrush->DrawAs = ESlateBrushDrawType::Image;
		HeroUltimateIconBrush->ImageSize = FVector2D(72.f, 72.f);
	}
	if (!HeroPassiveIconBrush.IsValid())
	{
		HeroPassiveIconBrush = MakeShared<FSlateBrush>();
		HeroPassiveIconBrush->DrawAs = ESlateBrushDrawType::Image;
		HeroPassiveIconBrush->ImageSize = FVector2D(72.f, 72.f);
	}

	// Get localized text
	FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.HeroSelection", "Skins", "SKINS");
	FText StatsText = Loc ? Loc->GetText_BaseStatsHeader() : NSLOCTEXT("T66.HeroSelection", "BaseStatsHeader", "STATS");
	FText EnterText = NSLOCTEXT("T66.HeroSelection", "EnterShort", "ENTER");
	FText ReadyText = NSLOCTEXT("T66.HeroSelection", "Ready", "READY");
	FText UnreadyText = NSLOCTEXT("T66.HeroSelection", "Unready", "UNREADY");
	FText WaitingForPartyText = NSLOCTEXT("T66.HeroSelection", "WaitingForParty", "WAITING FOR PARTY");
	FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	FText BackText = Loc ? Loc->GetText_Back() : NSLOCTEXT("T66.Common", "Back", "BACK");
	const FText ChallengesTooltipText = NSLOCTEXT("T66.HeroSelection", "ChallengesTooltip", "Challenges");
	const FText ModsTooltipText = NSLOCTEXT("T66.HeroSelection", "ModsTooltip", "Mods");
	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));

	// Initialize difficulty dropdown options
	DifficultyOptions.Empty();
	const TArray<ET66Difficulty> Difficulties = T66GI ? T66GI->GetPlayableDifficulties() : TArray<ET66Difficulty>{
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard, ET66Difficulty::VeryHard, ET66Difficulty::Impossible
	};
	for (ET66Difficulty Diff : Difficulties)
	{
		FText DiffText = Loc ? Loc->GetText_Difficulty(Diff) : NSLOCTEXT("T66.Difficulty", "Unknown", "???");
		DifficultyOptions.Add(MakeShared<FString>(DiffText.ToString()));
	}
	// Set current selection
	int32 CurrentDiffIndex = Difficulties.IndexOfByKey(SelectedDifficulty);
	if (CurrentDiffIndex != INDEX_NONE && CurrentDiffIndex < DifficultyOptions.Num())
	{
		CurrentDifficultyOption = DifficultyOptions[CurrentDiffIndex];
	}
	else if (DifficultyOptions.Num() > 0)
	{
		CurrentDifficultyOption = DifficultyOptions[0];
	}

	// AC balance (shown in the skins panel)
	const int32 ACBalance = T66SelectionScreenUtils::GetAchievementCoinBalance(this);
	const FText ACBalanceText = FText::AsNumber(ACBalance);

	UT66PartySubsystem* PartySubsystem = T66GI ? T66GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = T66GI ? T66GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bForceDrugPickerForAutomation = FParse::Param(FCommandLine::Get(), TEXT("T66HeroSelectionBuffPicker"));
	const FText NoCompanionText = Loc ? Loc->GetText_NoCompanion() : NSLOCTEXT("T66.HeroSelection", "NoCompanion", "No Companion");
	FText CurrentHeroDisplayName = NSLOCTEXT("T66.HeroSelection", "HeroFallbackName", "Hero");
	FText CurrentCompanionDisplayName = NoCompanionText;
	if (T66GI)
	{
		FHeroData HeroNameData;
		const FName HeroNameID = !PreviewedHeroID.IsNone() ? PreviewedHeroID : T66GI->SelectedHeroID;
		if (!HeroNameID.IsNone() && T66GI->GetHeroData(HeroNameID, HeroNameData))
		{
			CurrentHeroDisplayName = Loc ? Loc->GetHeroDisplayName(HeroNameData) : HeroNameData.DisplayName;
		}

		FCompanionData CompanionNameData;
		const FName CompanionNameID = !PreviewedCompanionID.IsNone() ? PreviewedCompanionID : T66GI->SelectedCompanionID;
		if (!CompanionNameID.IsNone() && T66GI->GetCompanionData(CompanionNameID, CompanionNameData))
		{
			CurrentCompanionDisplayName = Loc ? Loc->GetCompanionDisplayName(CompanionNameData) : CompanionNameData.DisplayName;
		}
	}
	if (SessionSubsystem)
	{
		SelectedDifficulty = SessionSubsystem->GetSharedLobbyDifficulty();
	}
	const bool bIsLocalPartyHost = !SessionSubsystem || SessionSubsystem->IsLocalPlayerPartyHost();
	const bool bPartyLobbyContextActive = SessionSubsystem && SessionSubsystem->IsPartyLobbyContextActive();
	const int32 LobbyPlayerCount = SessionSubsystem ? SessionSubsystem->GetCurrentLobbyPlayerCount() : 0;
	const int32 ActivePartySlots = bPartyLobbyContextActive
		? FMath::Clamp(LobbyPlayerCount, 1, 4)
		: (PartySubsystem ? FMath::Clamp(PartySubsystem->GetPartyMemberCount(), 1, 4) : 1);
	const bool bHasRemotePartyMembers = bPartyLobbyContextActive
		? LobbyPlayerCount > 1
		: (PartySubsystem && PartySubsystem->HasRemotePartyMembers());
	const bool bUsePartyReadyFlow = bPartyLobbyContextActive && (!bIsLocalPartyHost || bHasRemotePartyMembers);
	const bool bCanEditDifficulty = !bUsePartyReadyFlow || bIsLocalPartyHost;
	const bool bLocalReady = SessionSubsystem && SessionSubsystem->IsLocalLobbyReady();
	const bool bCanStartPartyRun = !bUsePartyReadyFlow || !SessionSubsystem || SessionSubsystem->AreAllPartyMembersReadyForGameplay();
	const FText PrimaryActionText = bUsePartyReadyFlow && !bIsLocalPartyHost
		? (bLocalReady ? UnreadyText : ReadyText)
		: (bCanStartPartyRun ? EnterText : WaitingForPartyText);

	SkinTargetOptions.Empty();
	SkinTargetOptions.Add(MakeShared<FString>(CurrentHeroDisplayName.ToString()));
	SkinTargetOptions.Add(MakeShared<FString>(CurrentCompanionDisplayName.ToString()));
	if (SkinTargetOptions.Num() >= 2)
	{
		CurrentSkinTargetOption = SkinTargetOptions[bShowingCompanionSkins ? 1 : 0];
	}

	InfoTargetOptions.Empty();
	InfoTargetOptions.Add(MakeShared<FString>(CurrentHeroDisplayName.ToString()));
	InfoTargetOptions.Add(MakeShared<FString>(CurrentCompanionDisplayName.ToString()));
	if (InfoTargetOptions.Num() >= 2)
	{
		CurrentInfoTargetOption = InfoTargetOptions[bShowingCompanionInfo ? 1 : 0];
	}
	UT66BuffSubsystem* TempBuffSubsystem = T66GI ? T66GI->GetSubsystem<UT66BuffSubsystem>() : nullptr;
	UT66UITexturePoolSubsystem* SelectionTexPool = T66GI ? T66GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
	if (bForceDrugPickerForAutomation)
	{
		TemporaryBuffPickerSlotIndex = FMath::Clamp(TemporaryBuffPickerSlotIndex, 0, UT66BuffSubsystem::MaxSelectedSingleUseBuffs - 1);
		bShowingTemporaryBuffPicker = false;
		if (TempBuffSubsystem)
		{
			TempBuffSubsystem->BeginHeroSelectionSingleUseBuffEdit(TemporaryBuffPickerSlotIndex);
		}
	}
	if (FParse::Param(FCommandLine::Get(), TEXT("T66HeroSelectionRecordInfo")))
	{
		bShowingHeroRecordInfoPanel = true;
	}

	const FHeroSelectionSharedLayoutMetrics SharedLayoutForIcons = MakeHeroSelectionSharedLayoutMetrics(FT66Style::IsDotaTheme());
	ResolveHeroSelectionLooseIconBrush(
		GetHeroSelectionBalanceIconPath(),
		FVector2D(SharedLayoutForIcons.BalanceBadgeIconWidth, SharedLayoutForIcons.BalanceBadgeIconHeight),
		ACBalanceIconBrush,
		ACBalanceIconTexture,
		TEXT("HeroSelectionBalanceIcon"));
	ResolveHeroSelectionLooseIconBrush(GetHeroSelectionChadIconPath(), FVector2D(22.f, 22.f), ChadCompanionIconBrush, ChadCompanionIconTexture, TEXT("HeroSelectionChadIcon"));
	ResolveHeroSelectionLooseIconBrush(GetHeroSelectionStacyIconPath(), FVector2D(22.f, 22.f), StacyCompanionIconBrush, StacyCompanionIconTexture, TEXT("HeroSelectionStacyIcon"));

	if (HeroPreviewController)
	{
		HeroPreviewController->RefreshCompanionPreviewPanel(T66GI, PreviewedCompanionID, bShowingCompanionInfo);
	}
	const TArray<ET66SecondaryStatType> ActiveTempBuffSlots = TempBuffSubsystem ? TempBuffSubsystem->GetSelectedSingleUseBuffSlots() : TArray<ET66SecondaryStatType>{};
	SelectedTemporaryBuffBrushes.Reset();
	SelectedTemporaryBuffBrushes.SetNum(UT66BuffSubsystem::MaxSelectedSingleUseBuffs);
	for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
	{
		const ET66SecondaryStatType SlotStat = ActiveTempBuffSlots.IsValidIndex(SlotIndex) ? ActiveTempBuffSlots[SlotIndex] : ET66SecondaryStatType::None;
		SelectedTemporaryBuffBrushes[SlotIndex] = T66IsLiveSecondaryStatType(SlotStat)
			? T66TemporaryBuffUI::CreateSecondaryBuffBrush(SelectionTexPool, this, SlotStat, FVector2D(60.f, 60.f))
			: nullptr;
	}

	auto MakeDrugLoadoutButtonShell = [](FOnClicked OnClicked, const TSharedRef<SWidget>& Content) -> TSharedRef<SWidget>
	{
		static FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		return SNew(SButton)
			.ButtonStyle(&ButtonStyle)
			.ContentPadding(0.f)
			.OnClicked(MoveTemp(OnClicked))
			[
				Content
			];
	};

	auto MakeSelectedTemporaryBuffSlot = [&, MakeDrugLoadoutButtonShell](int32 SlotIndex) -> TSharedRef<SWidget>
	{
		const bool bFilled = SelectedTemporaryBuffBrushes.IsValidIndex(SlotIndex) && SelectedTemporaryBuffBrushes[SlotIndex].IsValid();
		const bool bOwnedForSlot = TempBuffSubsystem ? TempBuffSubsystem->IsSelectedSingleUseBuffSlotOwned(SlotIndex) : true;
		return SNew(SBox)
			.WidthOverride(70.f)
			.HeightOverride(70.f)
			[
				MakeDrugLoadoutButtonShell(
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked, SlotIndex),
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(bOwnedForSlot
						? HeroSelectionChromeAccent()
						: FLinearColor(0.95f, 0.08f, 0.14f, 1.0f))
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bOwnedForSlot
							? HeroSelectionChromeInnerFillAlt()
							: FLinearColor(0.14f, 0.07f, 0.07f, 0.96f))
						.Padding(6.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								bFilled
								? StaticCastSharedRef<SWidget>(
									SNew(SScaleBox)
									.Stretch(EStretch::ScaleToFit)
									[
										SNew(SImage)
										.Image_Lambda([this, SlotIndex]() -> const FSlateBrush*
										{
											return SelectedTemporaryBuffBrushes.IsValidIndex(SlotIndex) && SelectedTemporaryBuffBrushes[SlotIndex].IsValid()
												? SelectedTemporaryBuffBrushes[SlotIndex].Get()
												: nullptr;
										})
										.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, bOwnedForSlot ? 1.0f : 0.55f))
									])
								: StaticCastSharedRef<SWidget>(
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66.HeroSelection", "TempBuffEmptySlot", "+"))
									.Font(FT66Style::Tokens::FontBold(24))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted))
							]
						]
					])
			];
	};

	// Build skins list (Default + Beachgoer only). Refreshed in place via RefreshSkinsList() when Equip/Buy.
	SAssignNew(SkinsListBoxWidget, SVerticalBox);
	AddSkinRowsToBox(SkinsListBoxWidget);

	// Get current hero color for preview
	FLinearColor HeroPreviewColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f); // Default gray
	FHeroData CurrentHeroData;
	if (GetPreviewedHeroData(CurrentHeroData))
	{
		HeroPreviewColor = CurrentHeroData.PlaceholderColor;
	}

	// Build hero sprite carousel (colored boxes for each hero)
	TSharedRef<SHorizontalBox> HeroCarousel = SNew(SHorizontalBox);
	const bool bDotaTheme = FT66Style::IsDotaTheme();
	const FLinearColor SelectionShellFill = FLinearColor::Black;
	const FSlateColor SelectionPanelFill = FLinearColor(0.022f, 0.022f, 0.024f, 1.0f);
	const FSlateColor SelectionInsetFill = FLinearColor(0.032f, 0.032f, 0.036f, 1.0f);
	RefreshHeroList(); // Ensure hero list is populated

	// Ensure we have stable brushes for the visible slots.
	HeroCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	HeroCarouselSlotColors.SetNum(HeroSelectionCarouselVisibleSlots);
	HeroCarouselSlotVisibility.SetNum(HeroSelectionCarouselVisibleSlots);
	HeroCarouselImageWidgets.SetNum(HeroSelectionCarouselVisibleSlots);
	for (int32 i = 0; i < HeroCarouselPortraitBrushes.Num(); ++i)
	{
		if (!HeroCarouselPortraitBrushes[i].IsValid())
		{
			HeroCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			HeroCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			HeroCarouselPortraitBrushes[i]->ImageSize = FVector2D(128.f, 128.f);
		}
	}
	RefreshHeroCarouselPortraits();
	
	// Show 9 heroes centered on current (prev4..next4).
	for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; Offset++)
	{
		if (AllHeroIDs.Num() == 0) break;

		const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
		const float Opacity = GetHeroSelectionCarouselOpacity(Offset);
		const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
		const EVisibility InitialHeroSlotVisibility = HeroCarouselSlotVisibility.IsValidIndex(SlotIdx)
			? HeroCarouselSlotVisibility[SlotIdx]
			: EVisibility::Collapsed;
		const bool bCenterHeroSlot = Offset == 0;
		const TSharedRef<SWidget> CarouselSlotWidget =
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(GetHeroSelectionCarouselSlotBrush(bCenterHeroSlot))
			]
			+ SOverlay::Slot()
			.Padding(FMargin(bCenterHeroSlot ? 5.f : 6.f))
			[
				SAssignNew(HeroCarouselImageWidgets[SlotIdx], SImage)
				.Image(HeroCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? HeroCarouselPortraitBrushes[SlotIdx].Get() : nullptr)
				.Visibility(InitialHeroSlotVisibility)
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
			];

		TSharedRef<SWidget> InteractiveCarouselSlot = CarouselSlotWidget;
		if (Offset == 0)
		{
			InteractiveCarouselSlot =
				FT66Style::MakeBareButton(
					FT66BareButtonParams(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleHeroGridClicked), CarouselSlotWidget)
					.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.SetPadding(FMargin(0.f)));
		}

		HeroCarousel->AddSlot()
		.AutoWidth()
		.Padding(3.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(BoxSize)
			.HeightOverride(BoxSize)
			[
				InteractiveCarouselSlot
			]
			];
	}

	TSharedRef<SHorizontalBox> CompanionCarousel = SNew(SHorizontalBox);
	CompanionCarouselPortraitBrushes.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotColors.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotVisibility.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselSlotLabels.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselImageWidgets.SetNum(HeroSelectionCarouselVisibleSlots);
	CompanionCarouselLabelWidgets.SetNum(HeroSelectionCarouselVisibleSlots);
	for (int32 i = 0; i < CompanionCarouselPortraitBrushes.Num(); ++i)
	{
		if (!CompanionCarouselPortraitBrushes[i].IsValid())
		{
			CompanionCarouselPortraitBrushes[i] = MakeShared<FSlateBrush>();
			CompanionCarouselPortraitBrushes[i]->DrawAs = ESlateBrushDrawType::Image;
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(128.f, 128.f);
		}
	}
	RefreshCompanionCarouselPortraits();

	for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; Offset++)
	{
		const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
		const float Opacity = GetHeroSelectionCarouselOpacity(Offset);
		const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
		const EVisibility InitialCompanionSlotVisibility = CompanionCarouselSlotVisibility.IsValidIndex(SlotIdx)
			? CompanionCarouselSlotVisibility[SlotIdx]
			: EVisibility::Collapsed;
		const FText InitialCompanionSlotLabel = CompanionCarouselSlotLabels.IsValidIndex(SlotIdx)
			? CompanionCarouselSlotLabels[SlotIdx]
			: FText::GetEmpty();
		const bool bCenterCompanionSlot = Offset == 0;
		const TSharedRef<SWidget> CompanionSlotWidget =
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(GetHeroSelectionCarouselSlotBrush(bCenterCompanionSlot))
			]
			+ SOverlay::Slot()
			.Padding(FMargin(bCenterCompanionSlot ? 5.f : 6.f))
			[
				SAssignNew(CompanionCarouselImageWidgets[SlotIdx], SImage)
				.Image(CompanionCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? CompanionCarouselPortraitBrushes[SlotIdx].Get() : nullptr)
				.Visibility(InitialCompanionSlotVisibility)
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(CompanionCarouselLabelWidgets[SlotIdx], STextBlock)
				.Text(InitialCompanionSlotLabel)
				.Font(FT66Style::Tokens::FontBold(13))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Visibility(InitialCompanionSlotLabel.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
			];

		TSharedRef<SWidget> InteractiveCompanionSlot = CompanionSlotWidget;
		if (Offset == 0)
		{
			InteractiveCompanionSlot =
				FT66Style::MakeBareButton(
					FT66BareButtonParams(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionGridClicked), CompanionSlotWidget)
					.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.SetPadding(FMargin(0.f)));
		}

		CompanionCarousel->AddSlot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(BoxSize)
			.HeightOverride(BoxSize)
			[
				InteractiveCompanionSlot
			]
		];
	}

	const FTextBlockStyle& TxtButton = FT66Style::Get().GetWidgetStyle<FTextBlockStyle>("T66.Text.Button");
	const FHeroSelectionSharedLayoutMetrics Layout = MakeHeroSelectionSharedLayoutMetrics(bDotaTheme);
	const FVector2D LayoutViewportSize = Layout.LayoutViewportSize;
	const bool bShortViewport = Layout.bShortViewport;
	const float ReferenceLayoutWidth = Layout.ReferenceLayoutWidth;
	const float ReferenceLayoutHeight = Layout.ReferenceLayoutHeight;
	const float PanelTouchOverlap = Layout.PanelTouchOverlap;
	const float LeftPanelWidth = Layout.LeftPanelWidth;
	const float RightPanelWidth = Layout.RightPanelWidth;
	const float CenterPanelX = Layout.CenterPanelX;
	const float CenterPreviewWidth = Layout.CenterPreviewWidth;
	const float PartyFooterWidth = Layout.PartyFooterWidth;
	const float CompanionFooterWidth = Layout.CompanionFooterWidth;
	const float CompanionFooterX = Layout.CompanionFooterX;
	const float RunFooterX = Layout.RunFooterX;
	const float RunFooterWidth = Layout.RunFooterWidth;
	const float CompanionFooterContentWidth = Layout.CompanionFooterContentWidth;
	const float RunFooterContentWidth = Layout.RunFooterContentWidth;
	const float UpperPanelY = Layout.UpperPanelY;
	const float FooterPanelMinHeight = Layout.FooterPanelMinHeight;
	const float FooterPanelY = Layout.FooterPanelY;
	const float UpperSidePanelHeight = Layout.UpperSidePanelHeight;
	const float RightStatsCardHeight = Layout.RightStatsCardHeight;
	const float RightUltRowHeight = Layout.RightUltRowHeight;
	const float PanelGap = Layout.PanelGap;
	const float OuterPanelBleed = Layout.OuterPanelBleed;
	const float TopBarBottomGap = Layout.TopBarBottomGap;
	const float LayoutCompactScale = Layout.LayoutCompactScale;
	const float FooterToggleWidth = Layout.FooterToggleWidth;
	const float FooterToggleHeight = Layout.FooterToggleHeight;
	const float FooterActionHeight = Layout.FooterActionHeight;
	const float BalanceBadgeIconWidth = Layout.BalanceBadgeIconWidth;
	const float BalanceBadgeIconHeight = Layout.BalanceBadgeIconHeight;
	const float LeftSkinsCardHeight = Layout.LeftSkinsCardHeight;
	const float RightPreviewPanelHeight = Layout.RightPreviewPanelHeight;
	const float RightAbilityIconButtonSize = Layout.RightAbilityIconButtonSize;
	const float RightAbilityIconSize = Layout.RightAbilityIconSize;
	const int32 ScreenHeaderFontSize = Layout.ScreenHeaderFontSize;
	const int32 BodyToggleFontSize = Layout.BodyToggleFontSize;
	const int32 PrimaryCtaFontSize = Layout.PrimaryCtaFontSize;
	const int32 HeroArrowFontSize = Layout.HeroArrowFontSize;
	const int32 ACBalanceFontSize = Layout.ACBalanceFontSize;
	const int32 HeroNameFontSize = Layout.HeroNameFontSize;
	const int32 SecondaryButtonFontSize = Layout.SecondaryButtonFontSize;
	const int32 EntityDropdownFontSize = Layout.EntityDropdownFontSize;
	const int32 BodyTextFontSize = Layout.BodyTextFontSize;
	const int32 DifficultyMenuFontSize = Layout.DifficultyMenuFontSize;
	const float HeroArrowButtonWidth = Layout.HeroArrowButtonWidth;
	const float HeroArrowButtonHeight = Layout.HeroArrowButtonHeight;
	const float TopStripBackButtonWidth = Layout.TopStripBackButtonWidth;
	const float TopStripBackButtonHeight = Layout.TopStripBackButtonHeight;

	auto MakePanelSectionHeader = [SecondaryButtonFontSize](const FText& HeaderText) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HeightOverride(26.f)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(HeaderText)
				.Font(FT66Style::Tokens::FontBold(FMath::Max(SecondaryButtonFontSize + 1, 14)))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Left)
			];
	};

	auto MakeTemporaryBuffLoadoutPanel = [this,
		SecondaryButtonFontSize,
		&MakeSelectedTemporaryBuffSlot]() -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> BuffSlotRow = SNew(SHorizontalBox);
		for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
		{
			BuffSlotRow->AddSlot()
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, SlotIndex + 1 < UT66BuffSubsystem::MaxSelectedSingleUseBuffs ? 6.f : 0.f, 0.f))
			[
				MakeSelectedTemporaryBuffSlot(SlotIndex)
			];
		}
		BuffSlotRow->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FMargin(10.f, 0.f, 8.f, 0.f))
		[
			MakeHeroSelectionButton(
				FT66ButtonParams(
					NSLOCTEXT("T66.HeroSelection", "TempBuffBuyOpenPicker", "BUY"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked, TemporaryBuffPickerSlotIndex),
					ET66ButtonType::Primary)
				.SetMinWidth(118.f)
				.SetHeight(70.f)
				.SetFontSize(SecondaryButtonFontSize - 1)
				.SetPadding(FMargin(10.f, 8.f)))
		];
		BuffSlotRow->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			MakeHeroSelectionButton(
				FT66ButtonParams(
					NSLOCTEXT("T66.HeroSelection", "TempBuffClear", "CLEAR"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleClearTemporaryBuffsClicked),
					ET66ButtonType::Neutral)
				.SetMinWidth(118.f)
				.SetHeight(70.f)
				.SetFontSize(SecondaryButtonFontSize - 2)
				.SetPadding(FMargin(10.f, 8.f)))
		];

		return SNew(SBox)
			.MinDesiredHeight(70.f)
			.HAlign(HAlign_Fill)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFitX)
				.StretchDirection(EStretchDirection::Both)
				.HAlign(HAlign_Center)
				[
					BuffSlotRow
				]
			];
	};

	auto MakeTemporaryBuffPickerModal = [this,
		TempBuffSubsystem,
		SelectionTexPool,
		Loc,
		BackText,
		SecondaryButtonFontSize,
		ScreenHeaderFontSize,
		BodyTextFontSize]() -> TSharedRef<SWidget>
	{
		TemporaryBuffPickerBrushes.Reset();
		const int32 FocusedSlotIndex = FMath::Clamp(
			TemporaryBuffPickerSlotIndex,
			0,
			UT66BuffSubsystem::MaxSelectedSingleUseBuffs - 1);
		const TArray<ET66SecondaryStatType> ActiveSlots = TempBuffSubsystem
			? TempBuffSubsystem->GetSelectedSingleUseBuffSlots()
			: TArray<ET66SecondaryStatType>{};
		const ET66SecondaryStatType FocusedSlotStat = ActiveSlots.IsValidIndex(FocusedSlotIndex)
			? ActiveSlots[FocusedSlotIndex]
			: ET66SecondaryStatType::None;

		auto GetDrugPrimaryStatLabel = [Loc](const ET66HeroStatType StatType) -> FText
		{
			if (Loc)
			{
				switch (StatType)
				{
				case ET66HeroStatType::Damage: return Loc->GetText_Stat_Damage();
				case ET66HeroStatType::AttackSpeed: return Loc->GetText_Stat_AttackSpeed();
				case ET66HeroStatType::AttackScale: return Loc->GetText_Stat_AttackScale();
				case ET66HeroStatType::Accuracy: return Loc->GetText_Stat_Accuracy();
				case ET66HeroStatType::Armor: return Loc->GetText_Stat_Armor();
				case ET66HeroStatType::Evasion: return Loc->GetText_Stat_Evasion();
				case ET66HeroStatType::Luck: return Loc->GetText_Stat_Luck();
				default: break;
				}
			}
			return NSLOCTEXT("T66.HeroSelection", "DrugPrimaryStatFallback", "?");
		};
		auto MakeUpperText = [](const FText& Text) -> FText
		{
			return FText::FromString(Text.ToString().ToUpper());
		};
		auto GetDrugRowTitle = [GetDrugPrimaryStatLabel](const ET66HeroStatType StatType) -> FText
		{
			return FText::Format(
				NSLOCTEXT("T66.HeroSelection", "DrugGridRowTitle", "{0} Drugs"),
				GetDrugPrimaryStatLabel(StatType));
		};
		struct FDrugPickerRowDef
		{
			ET66HeroStatType PrimaryStat = ET66HeroStatType::Damage;
			TArray<ET66SecondaryStatType> SecondaryStats;
		};
		const TArray<FDrugPickerRowDef> DrugRows = {
			{ ET66HeroStatType::Damage,      { ET66SecondaryStatType::AoeDamage, ET66SecondaryStatType::BounceDamage, ET66SecondaryStatType::PierceDamage, ET66SecondaryStatType::DotDamage } },
			{ ET66HeroStatType::AttackSpeed, { ET66SecondaryStatType::AoeSpeed, ET66SecondaryStatType::BounceSpeed, ET66SecondaryStatType::PierceSpeed, ET66SecondaryStatType::DotSpeed } },
			{ ET66HeroStatType::AttackScale, { ET66SecondaryStatType::AoeScale, ET66SecondaryStatType::BounceScale, ET66SecondaryStatType::PierceScale, ET66SecondaryStatType::DotScale } },
			{ ET66HeroStatType::Accuracy,    { ET66SecondaryStatType::CritDamage, ET66SecondaryStatType::CritChance, ET66SecondaryStatType::AttackRange, ET66SecondaryStatType::Accuracy } },
			{ ET66HeroStatType::Armor,       { ET66SecondaryStatType::Taunt, ET66SecondaryStatType::DamageReduction, ET66SecondaryStatType::ReflectDamage, ET66SecondaryStatType::Crush } },
			{ ET66HeroStatType::Evasion,     { ET66SecondaryStatType::EvasionChance, ET66SecondaryStatType::CounterAttack, ET66SecondaryStatType::Invisibility, ET66SecondaryStatType::Assassinate } },
			{ ET66HeroStatType::Luck,        { ET66SecondaryStatType::TreasureChest, ET66SecondaryStatType::Cheating, ET66SecondaryStatType::Stealing, ET66SecondaryStatType::LootCrate } },
		};
		const int32 SingleUsePercent = FMath::RoundToInt((UT66BuffSubsystem::SingleUseSecondaryBuffMultiplier - 1.f) * 100.f);
		const FText HintText = FText::Format(
			NSLOCTEXT("T66.HeroSelection", "TempBuffPowerUpHint", "Buy drug cards for +{0}% secondary-stat boosts. Owned drugs can be equipped from Hero Selection, up to 4 total per run."),
			FText::AsNumber(SingleUsePercent));
		const float CardGap = 24.f;
		auto MakeDrugGridActionButton = [this, BodyTextFontSize](
			const FText& ButtonText,
			const FOnClicked& OnClicked,
			const bool bEnabled,
			const bool bPrimary,
			const TSharedRef<SWidget>& Content) -> TSharedRef<SWidget>
		{
			return MakeHeroSelectionButton(
				FT66ButtonParams(ButtonText, OnClicked, bPrimary ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
				.SetMinWidth(0.f)
				.SetHeight(46.f)
				.SetFontSize(FMath::Max(BodyTextFontSize, 14))
				.SetPadding(FMargin(10.f, 5.f))
				.SetEnabled(TAttribute<bool>::CreateLambda([bEnabled]() { return bEnabled; }))
				.SetContent(Content));
		};
		auto MakeDrugGridCard = [this,
			TempBuffSubsystem,
			SelectionTexPool,
			Loc,
			FocusedSlotStat,
			BodyTextFontSize,
			MakeUpperText,
			MakeDrugGridActionButton](const ET66SecondaryStatType StatType) -> TSharedRef<SWidget>
		{
			const int32 OwnedCount = TempBuffSubsystem ? TempBuffSubsystem->GetOwnedSingleUseBuffCount(StatType) : 0;
			const int32 AssignedCount = TempBuffSubsystem ? TempBuffSubsystem->GetSelectedSingleUseBuffSlotAssignedCountForStat(StatType) : 0;
			const int32 AssignedOutsideFocused = AssignedCount - (FocusedSlotStat == StatType ? 1 : 0);
			const bool bCanEquip = OwnedCount > AssignedOutsideFocused;
			const bool bFocusedSlotMatches = FocusedSlotStat == StatType;
			const int32 BuffCost = TempBuffSubsystem ? TempBuffSubsystem->GetSingleUseBuffCost() : UT66BuffSubsystem::SingleUseBuffCostCC;
			const bool bCanBuy = TempBuffSubsystem && TempBuffSubsystem->GetChadCouponBalance() >= BuffCost;
			const bool bUseOwnedCopy = bCanEquip || bFocusedSlotMatches;
			const bool bActionEnabled = bFocusedSlotMatches ? false : (bUseOwnedCopy ? bCanEquip : bCanBuy);
			const FOnClicked DrugActionClicked = bUseOwnedCopy
				? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffEquipClicked, StatType)
				: FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffBuyClicked, StatType);
			const FText DrugActionText = bFocusedSlotMatches
				? NSLOCTEXT("T66.HeroSelection", "TempBuffEquipped", "EQUIPPED")
				: (bUseOwnedCopy
					? NSLOCTEXT("T66.HeroSelection", "TempBuffEquip", "EQUIP")
					: NSLOCTEXT("T66.HeroSelection", "TempBuffBuy", "BUY"));
			const TSharedRef<SWidget> ActionContent = bUseOwnedCopy
				? MakeHeroSelectionFittedLabel(DrugActionText, FMath::Max(BodyTextFontSize, 14), FT66Style::Tokens::Text)
				: StaticCastSharedRef<SWidget>(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(DrugActionText)
						.Font(FT66Style::Tokens::FontBold(FMath::Max(BodyTextFontSize, 14)))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(14.f, 0.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::AsNumber(BuffCost))
						.Font(FT66Style::Tokens::FontBold(FMath::Max(BodyTextFontSize, 14)))
						.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(7.f, 0.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(28.f)
						.HeightOverride(22.f)
						[
							FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
								SNew(SImage)
								.Image_Lambda([this]() -> const FSlateBrush*
								{
									return ACBalanceIconBrush.IsValid() ? ACBalanceIconBrush.Get() : nullptr;
								})))
						]
					]);

			TSharedPtr<FSlateBrush> BuffBrush = T66TemporaryBuffUI::CreateSecondaryBuffBrush(
				SelectionTexPool,
				this,
				StatType,
				FVector2D(124.f, 124.f));
			TemporaryBuffPickerBrushes.Add(BuffBrush);
			const TSharedRef<SWidget> IconWidget = BuffBrush.IsValid()
				? FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
					SNew(SImage)
					.Image(BuffBrush.Get())))
				: StaticCastSharedRef<SWidget>(
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "MissingDrugArt", "ART"))
					.Font(FT66Style::Tokens::FontBold(BodyTextFontSize))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted));

			return SNew(SBox)
				.HeightOverride(300.f)
				.Padding(FMargin(8.f, 0.f))
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(240.f)
							.HeightOverride(50.f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFit)
								.StretchDirection(EStretchDirection::DownOnly)
								[
									SNew(STextBlock)
									.Text(MakeUpperText(GetHeroSelectionDrugName(StatType)))
									.Font(FT66Style::Tokens::FontBold(17))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center)
									.AutoWrapText(true)
									.WrapTextAt(240.f)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
									.Clipping(EWidgetClipping::ClipToBounds)
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0.f, 6.f, 0.f, 8.f)
						[
							SNew(SBox)
							.WidthOverride(142.f)
							.HeightOverride(112.f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(HeroSelectionChromeAccent())
								.Padding(2.f)
								[
									SNew(SBorder)
									.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
									.BorderBackgroundColor(HeroSelectionChromeInnerFill())
									.Padding(8.f)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										IconWidget
									]
								]
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(240.f)
							.HeightOverride(34.f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SScaleBox)
								.Stretch(EStretch::ScaleToFit)
								.StretchDirection(EStretchDirection::DownOnly)
								[
									SNew(STextBlock)
									.Text(GetHeroSelectionDrugEffectText(StatType, Loc))
									.Font(FT66Style::Tokens::FontRegular(FMath::Max(BodyTextFontSize - 1, 12)))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center)
									.AutoWrapText(true)
									.WrapTextAt(240.f)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
									.Clipping(EWidgetClipping::ClipToBounds)
								]
							]
						]
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							SNew(SSpacer)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeDrugGridActionButton(
								DrugActionText,
								DrugActionClicked,
								bActionEnabled,
								bFocusedSlotMatches || !bUseOwnedCopy,
								ActionContent)
						]
				];
		};

		TSharedRef<SVerticalBox> DrugRowsBox = SNew(SVerticalBox);
		DrugRowsBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 14.f)
		[
			MakeHeroSelectionParchmentPanelShell(
				SNew(SBox)
				.HeightOverride(48.f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(HintText)
					.Font(FT66Style::Tokens::FontRegular(FMath::Max(BodyTextFontSize, 14)))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
					.WrapTextAt(1500.f)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Clipping(EWidgetClipping::ClipToBounds)
				],
				FMargin(18.f, 8.f))
		];
		for (int32 RowIndex = 0; RowIndex < DrugRows.Num(); ++RowIndex)
		{
			const FDrugPickerRowDef& RowDef = DrugRows[RowIndex];
			TSharedRef<SHorizontalBox> CardsRow = SNew(SHorizontalBox);
			CardsRow->AddSlot()
			.AutoWidth()
			.Padding(0.f, 0.f, CardGap, 0.f)
			[
				SNew(SBox)
				.WidthOverride(300.f)
				.HeightOverride(300.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(260.f)
					.HeightOverride(120.f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						.StretchDirection(EStretchDirection::DownOnly)
						[
							SNew(STextBlock)
							.Text(GetDrugRowTitle(RowDef.PrimaryStat))
							.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 6))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
							.AutoWrapText(true)
							.WrapTextAt(260.f)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						]
					]
				]
			];
			for (int32 CardIndex = 0; CardIndex < RowDef.SecondaryStats.Num(); ++CardIndex)
			{
				CardsRow->AddSlot()
				.AutoWidth()
				.Padding(CardIndex < RowDef.SecondaryStats.Num() - 1 ? FMargin(0.f, 0.f, CardGap, 0.f) : FMargin(0.f))
				[
					SNew(SBox)
					.WidthOverride(300.f)
					[
						MakeDrugGridCard(RowDef.SecondaryStats[CardIndex])
					]
				];
			}
			DrugRowsBox->AddSlot()
			.AutoHeight()
			.Padding(0.f, RowIndex > 0 ? CardGap : 0.f, 0.f, 0.f)
			[
				MakeHeroSelectionParchmentPanelShell(
					CardsRow,
					FMargin(14.f, 12.f))
			];
		}

		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.StretchDirection(EStretchDirection::Both)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				[
					SNew(SBox)
					.WidthOverride(1920.f)
					.HeightOverride(1080.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor::Black)
						.Padding(FMargin(36.f, 18.f, 36.f, 24.f))
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 0.f, 0.f, 16.f)
							[
								SNew(SBox)
								.HeightOverride(52.f)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									.HAlign(HAlign_Left)
									.VAlign(VAlign_Center)
									[
										MakeHeroSelectionButton(
											FT66ButtonParams(
												BackText,
												FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffPickerCloseClicked),
												ET66ButtonType::Neutral)
											.SetMinWidth(112.f)
											.SetHeight(34.f)
											.SetFontSize(SecondaryButtonFontSize)
											.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f)))
									]
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66.HeroSelection", "TempBuffDrugsHeader", "DRUGS"))
										.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 5))
										.ColorAndOpacity(FT66Style::Tokens::Text)
										.Justification(ETextJustify::Center)
									]
								]
							]
							+ SVerticalBox::Slot()
							.FillHeight(1.f)
							[
								SNew(SScrollBox)
								.ScrollBarStyle(GetHeroSelectionReferenceScrollBarStyle())
								.ScrollBarVisibility(EVisibility::Visible)
								.ScrollBarThickness(FVector2D(24.f, 24.f))
								.ScrollBarPadding(FMargin(14.f, 0.f, 0.f, 0.f))
								+ SScrollBox::Slot()
								[
									DrugRowsBox
								]
							]
						]
					]
				]
			];

#if 0
		auto MakeDrugPickerRowShell = [](const TSharedRef<SWidget>& Content) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(HeroSelectionChromeAccent(0.95f))
				.Padding(1.f)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(HeroSelectionChromeInnerFill())
					.Padding(FMargin(12.f, 10.f))
					.Clipping(EWidgetClipping::ClipToBounds)
					[
						Content
					]
				];
		};
		const FButtonStyle& DrugPickerNoBorderButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		auto MakeDrugPickerActionButton = [&DrugPickerNoBorderButtonStyle](
			FOnClicked OnClicked,
			const bool bPrimary,
			const bool bEnabled,
			const TSharedRef<SWidget>& Content) -> TSharedRef<SWidget>
		{
			const FLinearColor StrokeColor = bPrimary
				? FLinearColor(1.0f, 0.06f, 0.14f, 1.0f)
				: HeroSelectionChromeAccent();

			return SNew(SBox)
				.WidthOverride(128.f)
				.HeightOverride(40.f)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SButton)
					.ButtonStyle(&DrugPickerNoBorderButtonStyle)
					.ContentPadding(0.f)
					.IsEnabled(bEnabled)
					.OnClicked(MoveTemp(OnClicked))
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(StrokeColor)
						.Padding(1.f)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(HeroSelectionChromeInnerFillAlt())
							.Padding(FMargin(10.f, 6.f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Clipping(EWidgetClipping::ClipToBounds)
							[
								Content
							]
						]
					]
				];
		};

		TSharedRef<SVerticalBox> BuffRows = SNew(SVerticalBox);
		for (ET66SecondaryStatType StatType : UT66BuffSubsystem::GetAllSingleUseBuffTypes())
		{
			const int32 OwnedCount = TempBuffSubsystem ? TempBuffSubsystem->GetOwnedSingleUseBuffCount(StatType) : 0;
			const int32 AssignedCount = TempBuffSubsystem ? TempBuffSubsystem->GetSelectedSingleUseBuffSlotAssignedCountForStat(StatType) : 0;
			const int32 AssignedOutsideFocused = AssignedCount - (FocusedSlotStat == StatType ? 1 : 0);
			const bool bCanEquip = OwnedCount > AssignedOutsideFocused;
			const bool bFocusedSlotMatches = FocusedSlotStat == StatType;
			const int32 BuffCost = TempBuffSubsystem ? TempBuffSubsystem->GetSingleUseBuffCost() : UT66BuffSubsystem::SingleUseBuffCostCC;
			const bool bCanBuy = TempBuffSubsystem && TempBuffSubsystem->GetChadCouponBalance() >= BuffCost;
			const bool bUseOwnedCopy = bCanEquip || bFocusedSlotMatches;
			const bool bCanUseAction = bUseOwnedCopy ? (bCanEquip || bFocusedSlotMatches) : bCanBuy;
			const FOnClicked DrugActionClicked = bUseOwnedCopy
				? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffEquipClicked, StatType)
				: FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffBuyClicked, StatType);
			const FText DrugActionText = bFocusedSlotMatches
				? NSLOCTEXT("T66.HeroSelection", "TempBuffEquipped", "EQUIPPED")
				: (bUseOwnedCopy
					? NSLOCTEXT("T66.HeroSelection", "TempBuffEquip", "EQUIP")
					: NSLOCTEXT("T66.HeroSelection", "TempBuffBuy", "BUY"));
			const FText NameText = GetHeroSelectionDrugName(StatType);
			const FText EffectText = GetHeroSelectionDrugEffectText(StatType, Loc);
			const FText CountText = FText::Format(
				NSLOCTEXT("T66.HeroSelection", "TempBuffPickerCountsOwnedOnly", "Owned {0}"),
				FText::AsNumber(OwnedCount));

			TSharedPtr<FSlateBrush> BuffBrush = T66TemporaryBuffUI::CreateSecondaryBuffBrush(
				SelectionTexPool,
				this,
				StatType,
				FVector2D(34.f, 34.f));
			TemporaryBuffPickerBrushes.Add(BuffBrush);

			const TSharedRef<SWidget> IconWidget = BuffBrush.IsValid()
				? FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
					SNew(SImage)
					.Image(BuffBrush.Get())))
				: StaticCastSharedRef<SWidget>(SNew(SSpacer));

			BuffRows->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SBox)
				.HeightOverride(66.f)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					MakeDrugPickerRowShell(
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.f, 0.f, 10.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(36.f)
							.HeightOverride(36.f)
							[
								IconWidget
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
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(NameText)
									.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 3))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.f)
								.VAlign(VAlign_Center)
								.Padding(12.f, 0.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(EffectText)
									.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 1))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
									.Clipping(EWidgetClipping::ClipToBounds)
								]
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0.f, 2.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(CountText)
								.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize - 2))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(10.f, 0.f, 0.f, 0.f)
						[
							MakeDrugPickerActionButton(
								DrugActionClicked,
								bFocusedSlotMatches,
								bCanUseAction,
								bUseOwnedCopy
								? MakeHeroSelectionFittedLabel(DrugActionText, 16, FT66Style::Tokens::Text)
								: StaticCastSharedRef<SWidget>(
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Text(DrugActionText)
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(7.f, 0.f, 0.f, 0.f)
									[
										SNew(STextBlock)
										.Text(FText::AsNumber(BuffCost))
										.Font(FT66Style::Tokens::FontBold(16))
										.ColorAndOpacity(FT66Style::Tokens::Text)
									]
									+ SHorizontalBox::Slot()
									.AutoWidth()
									.VAlign(VAlign_Center)
									.Padding(5.f, 0.f, 0.f, 0.f)
									[
										SNew(SBox)
										.WidthOverride(25.f)
										.HeightOverride(20.f)
										[
											FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
												SNew(SImage)
												.Image_Lambda([this]() -> const FSlateBrush*
												{
													return ACBalanceIconBrush.IsValid() ? ACBalanceIconBrush.Get() : nullptr;
												})))
										]
									]))
						])
				]
			];
		}

		const TSharedRef<SWidget> PickerContent = MakeHeroSelectionPanelShell(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
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
						.Text(NSLOCTEXT("T66.HeroSelection", "TempBuffPickerTitle", "Choose Drugs"))
						.Font(FT66Style::Tokens::FontBold(24))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 2.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::Format(
							NSLOCTEXT("T66.HeroSelection", "TempBuffPickerSlotHint", "Slot {0}"),
							FText::AsNumber(FocusedSlotIndex + 1)))
						.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					MakeHeroSelectionButton(
						FT66ButtonParams(
							NSLOCTEXT("T66.Common", "Close", "CLOSE"),
							FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffPickerCloseClicked),
							ET66ButtonType::Neutral)
						.SetMinWidth(96.f)
						.SetHeight(36.f)
						.SetFontSize(15))
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			.Padding(0.f, 14.f, 0.f, 0.f)
			[
				SNew(SScrollBox)
				.Orientation(Orient_Vertical)
				.ScrollBarThickness(FVector2D(10.f, 10.f))
				.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
				+ SScrollBox::Slot()
				.Padding(0.f, 0.f, 14.f, 0.f)
				[
					BuffRows
				]
			],
			FMargin(22.f, 18.f),
			false);

		return T66ScreenSlateHelpers::MakeCenteredScrimModal(
			PickerContent,
			FMargin(0.f),
			1020.f,
			720.f,
			true);
#endif
	};

	auto MakeCompanionUnityPanel = [this,
		bDotaTheme,
		SelectionInsetFill,
		SecondaryButtonFontSize,
		BodyTextFontSize]() -> TSharedRef<SWidget>
	{
		return MakeHeroSelectionRowShell(
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "CompanionUnityHeader", "UNITY"))
				.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
				.ColorAndOpacity(FT66Style::Tokens::TextMuted)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SAssignNew(CompanionUnityTextWidget, STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "CompanionUnityDefault", "Unity: 0 / 20"))
				.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize + 1))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.WidthOverride(180.f)
				.HeightOverride(10.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						T66ScreenSlateHelpers::MakeReferenceProgressBar(
							TAttribute<TOptional<float>>::Create(TAttribute<TOptional<float>>::FGetter::CreateLambda([this]() -> TOptional<float>
							{
								return FMath::Clamp(CompanionUnityProgress01, 0.f, 1.f);
							})),
							FVector2D(180.f, 10.f),
							FLinearColor(0.78f, 0.43f, 0.13f, 1.0f),
							FMargin(4.f, 2.f))
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.Padding(FMargin(180.f * 0.25f - 1.f, 0.f, 0.f, 0.f))
					[
						SNew(SBox)
						.WidthOverride(2.f)
						.HeightOverride(10.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.Padding(FMargin(180.f * 0.50f - 1.f, 0.f, 0.f, 0.f))
					[
						SNew(SBox)
						.WidthOverride(2.f)
						.HeightOverride(10.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(2.f)
						.HeightOverride(10.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(0.95f, 0.95f, 0.98f, 0.65f))
						]
					]
				]
			],
			FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3));
	};

	auto MakeBalanceBadge = [this,
		BalanceBadgeIconWidth,
		BalanceBadgeIconHeight,
		ACBalanceText,
		ACBalanceFontSize,
		SecondaryButtonFontSize]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(BalanceBadgeIconWidth)
				.HeightOverride(BalanceBadgeIconHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						[
							FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
								SNew(SImage)
								.Image_Lambda([this]() -> const FSlateBrush*
								{
									return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
										? ACBalanceIconBrush.Get()
										: nullptr;
								})))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "CurrencyBadgeFallback", "CC"))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Visibility_Lambda([this]() -> EVisibility
						{
							return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
								? EVisibility::Collapsed
								: EVisibility::Visible;
						})
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				SAssignNew(ACBalanceTextBlock, STextBlock)
				.Text(ACBalanceText)
				.Font(FT66Style::Tokens::FontBold(ACBalanceFontSize))
				.ColorAndOpacity(FT66Style::Tokens::Text)
			];
	};

	auto MakeFocusMaskFill = [SelectionShellFill]() -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(SelectionShellFill);
	};

	auto MakePreviewFocusMask = [bDotaTheme, &MakeFocusMaskFill, LeftPanelWidth, RightPanelWidth, TopBarBottomGap, PanelGap]() -> TSharedRef<SWidget>
	{
		if (!bDotaTheme)
		{
			return SNew(SBox).Visibility(EVisibility::Collapsed);
		}

		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, TopBarBottomGap)
			[
				SNew(SBox)
				.HeightOverride(40.f)
				[
					MakeFocusMaskFill()
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, PanelGap, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(LeftPanelWidth)
					[
						MakeFocusMaskFill()
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(PanelGap, 0.0f)
				[
					SNew(SBox)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(PanelGap, 0.0f, 0.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(RightPanelWidth)
					[
						MakeFocusMaskFill()
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, TopBarBottomGap, 0.0f, 0.0f)
			[
				SNew(SBox)
				.HeightOverride(64.f)
				[
					MakeFocusMaskFill()
				]
			];
	};

	auto MakeBodyTogglePanel = [this,
		bDotaTheme,
		FooterToggleWidth,
		FooterToggleHeight,
		BodyToggleFontSize,
		SelectionPanelFill]() -> TSharedRef<SWidget>
	{
		const FSlateColor TogglePanelColor = bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel2;
		const FSlateColor ChadActiveColor = bDotaTheme ? FLinearColor(0.02f, 0.13f, 0.28f, 1.0f) : FLinearColor(0.02f, 0.16f, 0.38f, 1.0f);
		const FSlateColor StacyActiveColor = bDotaTheme ? FLinearColor(0.28f, 0.02f, 0.18f, 1.0f) : FLinearColor(0.38f, 0.02f, 0.24f, 1.0f);

		auto MakeBodyToggleButton = [this, FooterToggleWidth, FooterToggleHeight, BodyToggleFontSize, TogglePanelColor](
			const TCHAR* Label,
			const TCHAR* FallbackGlyph,
			ET66BodyType BodyType,
			const TSharedPtr<FSlateBrush>& IconBrush,
			const FSlateColor ActiveColor) -> TSharedRef<SWidget>
		{
			const FOnClicked OnClicked = T66BodyTypeAliases::IsChad(BodyType)
				? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleChadBodyClicked)
				: FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleStacyBodyClicked);

			return MakeHeroSelectionButton(FT66ButtonParams(
				FText::AsCultureInvariant(Label),
				OnClicked,
				ET66ButtonType::Neutral)
				.SetMinWidth(FooterToggleWidth)
				.SetHeight(FooterToggleHeight)
				.SetFontSize(BodyToggleFontSize)
				.SetPadding(FMargin(12.f, 8.f, 12.f, 7.f))
				.SetColor(TAttribute<FSlateColor>::CreateLambda([this, BodyType, ActiveColor, TogglePanelColor]() -> FSlateColor
				{
					return SelectedBodyType == BodyType ? ActiveColor : TogglePanelColor;
				}))
				.SetContent(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 6.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(30.f)
						.HeightOverride(30.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
									SNew(SImage)
									.Image_Lambda([IconBrush]() -> const FSlateBrush*
									{
										return IconBrush.IsValid() && ::IsValid(IconBrush->GetResourceObject()) ? IconBrush.Get() : nullptr;
									})))
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Visibility_Lambda([IconBrush]() -> EVisibility
								{
									return IconBrush.IsValid() && ::IsValid(IconBrush->GetResourceObject())
										? EVisibility::Collapsed
										: EVisibility::Visible;
								})
								.Text(FText::AsCultureInvariant(FallbackGlyph))
								.Font(FT66Style::Tokens::FontBold(BodyToggleFontSize - 1))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						MakeHeroSelectionFittedLabel(
							FText::AsCultureInvariant(Label),
							BodyToggleFontSize,
							FT66Style::Tokens::Text)
					]));
		};

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				MakeBodyToggleButton(TEXT("CHAD"), TEXT("M"), T66BodyTypeAliases::Chad, ChadCompanionIconBrush, ChadActiveColor)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				MakeBodyToggleButton(TEXT("STACY"), TEXT("F"), T66BodyTypeAliases::Stacy, StacyCompanionIconBrush, StacyActiveColor)
			];
	};

	auto MakeWorldScrim = []() -> TSharedRef<SWidget>
	{
		return SNew(SBox).Visibility(EVisibility::Collapsed);
	};

	auto MakeSelectionBar = [bDotaTheme, SelectionPanelFill](TSharedRef<SWidget> Content) -> TSharedRef<SWidget>
	{
		if (bDotaTheme)
		{
			return MakeHeroSelectionContentShell(Content, FMargin(0.f));
		}

		return MakeHeroSelectionContentShell(
			Content,
			FMargin(0.f));
	};

	auto MakeHeroStripControls = [this,
		HeroArrowButtonWidth,
		HeroArrowButtonHeight,
		HeroArrowFontSize,
		CenterPreviewWidth,
		HeroCarousel]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(FMath::Max(1.f, CenterPreviewWidth))
			[
				SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				MakeHeroSelectionButton(FT66ButtonParams(
					NSLOCTEXT("T66.Common", "Prev", "<"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandlePrevClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFitX)
				.StretchDirection(EStretchDirection::DownOnly)
				[
					HeroCarousel
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.0f, 0.0f, 0.0f, 0.0f)
			[
				MakeHeroSelectionButton(FT66ButtonParams(
					NSLOCTEXT("T66.Common", "Next", ">"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleNextClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
			]
			];
	};

	auto MakeBodyToggleStrip = [MakeBodyTogglePanel]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			[
				MakeBodyTogglePanel()
			];
	};

	auto MakeCompanionStripControls = [this,
		SecondaryButtonFontSize,
		CompanionFooterContentWidth,
		CompanionCarousel]() -> TSharedRef<SWidget>
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				MakeHeroSelectionButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.HeroSelection", "ChooseCompanionButton", "CHOOSE COMPANION"),
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(CompanionFooterContentWidth)
					.SetHeight(74.f)
					.SetFontSize(SecondaryButtonFontSize + 6)
					.SetPadding(FMargin(12.f, 12.f)))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.Visibility(EVisibility::Collapsed)
				[
					CompanionCarousel
				]
			];
	};

	auto MakeSkinTargetDropdown = [this, EntityDropdownFontSize]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(152.f)
			[
				MakeHeroSelectionDropdown(FT66DropdownParams(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(SkinTargetDropdownText, STextBlock)
						.Text(CurrentSkinTargetOption.IsValid()
							? FText::FromString(*CurrentSkinTargetOption)
							: NSLOCTEXT("T66.HeroSelection", "SkinTargetHeroFallback", "HERO"))
						.Font(FT66Style::Tokens::FontBold(EntityDropdownFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					],
					[this, EntityDropdownFontSize]()
					{
						TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
						for (const TSharedPtr<FString>& Opt : SkinTargetOptions)
						{
							if (!Opt.IsValid())
							{
								continue;
							}

							TSharedPtr<FString> Captured = Opt;
							Box->AddSlot()
								.AutoHeight()
								[
									FT66Style::MakeDropdownOptionButton(
										FText::FromString(*Opt),
										FOnClicked::CreateLambda([this, Captured]()
										{
											OnSkinTargetChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}),
										CurrentSkinTargetOption.IsValid() && *CurrentSkinTargetOption == *Opt,
										0.f,
										36.f,
										EntityDropdownFontSize,
										FMargin(10.f, 8.f, 10.f, 6.f))
								];
						}
						return Box;
					})
					.SetMinWidth(0.f)
					.SetHeight(36.f)
					.SetPadding(FMargin(10.f, 7.f)))
			];
	};

	auto MakeInfoTargetDropdown = [this, EntityDropdownFontSize]() -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.WidthOverride(152.f)
			[
				MakeHeroSelectionDropdown(FT66DropdownParams(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(InfoTargetDropdownText, STextBlock)
						.Text(CurrentInfoTargetOption.IsValid()
							? FText::FromString(*CurrentInfoTargetOption)
							: NSLOCTEXT("T66.HeroSelection", "InfoTargetHeroFallback", "Hero"))
						.Font(FT66Style::Tokens::FontBold(EntityDropdownFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					],
					[this, EntityDropdownFontSize]()
					{
						TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
						for (const TSharedPtr<FString>& Opt : InfoTargetOptions)
						{
							if (!Opt.IsValid())
							{
								continue;
							}

							TSharedPtr<FString> Captured = Opt;
							Box->AddSlot()
								.AutoHeight()
								[
									FT66Style::MakeDropdownOptionButton(
										FText::FromString(*Opt),
										FOnClicked::CreateLambda([this, Captured]()
										{
											OnInfoTargetChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}),
										CurrentInfoTargetOption.IsValid() && *CurrentInfoTargetOption == *Opt,
										0.f,
										36.f,
										EntityDropdownFontSize,
										FMargin(10.f, 8.f, 10.f, 6.f))
								];
						}
						return Box;
					})
					.SetMinWidth(0.f)
					.SetHeight(36.f)
					.SetPadding(FMargin(10.f, 7.f)))
			];
	};

	auto MakePartyBox = [this,
		ActivePartySlots,
		PartySubsystem,
		T66GI,
		SessionSubsystem,
		bUsePartyReadyFlow,
		LayoutCompactScale,
		PartyFooterWidth,
		FooterPanelMinHeight,
		OuterPanelBleed]() -> TSharedRef<SWidget>
	{
		const FLinearColor LeaderSlotAccent(0.29f, 0.24f, 0.13f, 1.0f);
		const FLinearColor PartySlotAccent(0.15f, 0.17f, 0.19f, 1.0f);
		const FLinearColor PartySlotAccentInactive(0.08f, 0.09f, 0.10f, 1.0f);
		const FLinearColor PlaceholderTint(0.20f, 0.22f, 0.24f, 0.55f);
		const FLinearColor ReadyFill(0.16f, 0.44f, 0.21f, 1.0f);
		const FLinearColor ReadyStroke(0.55f, 0.84f, 0.60f, 1.0f);
		const FLinearColor NotReadyFill(0.48f, 0.14f, 0.14f, 1.0f);
		const FLinearColor NotReadyStroke(0.92f, 0.48f, 0.48f, 1.0f);
		const float PartyScale = FMath::Clamp(LayoutCompactScale, 0.82f, 1.12f);
		const float PartyTileSide = FMath::RoundToFloat(84.f * PartyScale);
		const float PartySlotGap = 0.f;
		const float PartyMemberGap = FMath::RoundToFloat(10.f * PartyScale);
		const FVector2D PartyProfileSize(PartyTileSide, PartyTileSide);
		const FVector2D PartyAvatarImageSize(FMath::RoundToFloat(68.f * PartyScale), FMath::RoundToFloat(68.f * PartyScale));
		const FVector2D PartyHeroSize(PartyTileSide, PartyTileSide);
		const FVector2D PartyHeroImageSize(FMath::RoundToFloat(68.f * PartyScale), FMath::RoundToFloat(68.f * PartyScale));
		const float PartyReadyHeight = FMath::RoundToFloat(22.f * PartyScale);
		const float PartyReadyWidth = PartyProfileSize.X;
		const float PartyMemberWidth = PartyProfileSize.X + PartySlotGap + PartyHeroSize.X;
		const float PartyMemberHeight = PartyReadyHeight + PartyProfileSize.Y;
		const TArray<FT66PartyMemberEntry> PartyMembers = PartySubsystem ? PartySubsystem->GetPartyMembers() : TArray<FT66PartyMemberEntry>();
		UT66SteamHelper* SteamHelper = T66GI ? T66GI->GetSubsystem<UT66SteamHelper>() : nullptr;
		UT66UITexturePoolSubsystem* TexPool = T66GI ? T66GI->GetSubsystem<UT66UITexturePoolSubsystem>() : nullptr;
		PartyAvatarBrushes.SetNum(4);
		PartyHeroPortraitBrushes.SetNum(4);
		const bool bTreatPartyAsReadyByDefault = !bUsePartyReadyFlow || PartyMembers.Num() <= 1;

		auto MakeReadyBanner = [&](const bool bReady, const bool bOccupied) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.WidthOverride(PartyReadyWidth)
				.HeightOverride(PartyReadyHeight)
				.Visibility(bOccupied ? EVisibility::Visible : EVisibility::Hidden)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(bReady ? ReadyStroke : NotReadyStroke)
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bReady ? ReadyFill : NotReadyFill)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(bReady
								? NSLOCTEXT("T66.HeroSelection", "PartyReadySmall", "READY")
								: NSLOCTEXT("T66.HeroSelection", "PartyWaitingSmall", "WAIT"))
							.Font(FT66Style::Tokens::FontBold(bReady ? 10 : 9))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
						]
					]
				];
		};

		auto ResolvePartyAvatarTexture = [PartySubsystem, SteamHelper](const FT66PartyMemberEntry& Member) -> UTexture2D*
		{
			if (!SteamHelper)
			{
				return nullptr;
			}

			if (UTexture2D* ExactAvatar = SteamHelper->GetAvatarTextureForSteamId(Member.PlayerId))
			{
				return ExactAvatar;
			}

			if (Member.bIsLocal || Member.DisplayName.Equals(SteamHelper->GetLocalDisplayName(), ESearchCase::IgnoreCase))
			{
				if (UTexture2D* LocalAvatar = SteamHelper->GetLocalAvatarTexture())
				{
					return LocalAvatar;
				}
			}

			const FT66PartyFriendEntry* MatchingFriend = PartySubsystem ? PartySubsystem->GetFriends().FindByPredicate([&Member](const FT66PartyFriendEntry& Friend)
			{
				return Friend.DisplayName.Equals(Member.DisplayName, ESearchCase::IgnoreCase);
			}) : nullptr;
			return MatchingFriend ? SteamHelper->GetAvatarTextureForSteamId(MatchingFriend->PlayerId) : nullptr;
		};

		TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
		PartyAvatarImageWidgets.SetNum(4);
		PartyHeroPortraitImageWidgets.SetNum(4);
		for (int32 SlotIndex = 0; SlotIndex < 4; ++SlotIndex)
		{
			const FT66PartyMemberEntry* PartyMember = PartyMembers.IsValidIndex(SlotIndex) ? &PartyMembers[SlotIndex] : nullptr;
			UTexture2D* AvatarTexture = nullptr;
			if (PartyMember)
			{
				AvatarTexture = ResolvePartyAvatarTexture(*PartyMember);
			}

			if (!PartyAvatarBrushes[SlotIndex].IsValid())
			{
				PartyAvatarBrushes[SlotIndex] = MakeShared<FSlateBrush>();
				PartyAvatarBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
				PartyAvatarBrushes[SlotIndex]->Tiling = ESlateBrushTileType::NoTile;
			}
			PartyAvatarBrushes[SlotIndex]->ImageSize = PartyAvatarImageSize;
			PartyAvatarBrushes[SlotIndex]->SetResourceObject(AvatarTexture);

			if (!PartyHeroPortraitBrushes[SlotIndex].IsValid())
			{
				PartyHeroPortraitBrushes[SlotIndex] = MakeShared<FSlateBrush>();
				PartyHeroPortraitBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
				PartyHeroPortraitBrushes[SlotIndex]->Tiling = ESlateBrushTileType::NoTile;
			}
			PartyHeroPortraitBrushes[SlotIndex]->ImageSize = PartyHeroImageSize;
			PartyHeroPortraitBrushes[SlotIndex]->SetResourceObject(nullptr);

			const bool bHasAvatar = AvatarTexture != nullptr;
			const bool bPartyEnabledSlot = SlotIndex < ActivePartySlots;
			const bool bOccupiedSlot = PartyMember != nullptr;
			const bool bLeaderSlot = SlotIndex == 0;
			const bool bMemberReady = bOccupiedSlot && (bTreatPartyAsReadyByDefault || PartyMember->bReady);
			const FLinearColor SlotAccent = bLeaderSlot
				? LeaderSlotAccent
				: (bOccupiedSlot ? PartySlotAccent : PartySlotAccentInactive);
			const float PlaceholderOpacity = bPartyEnabledSlot ? 0.55f : 0.28f;

			const FName SlotHeroID = bOccupiedSlot && PartyMember->bIsLocal && T66GI
				? T66GI->SelectedHeroID
				: NAME_None;
			if (!SlotHeroID.IsNone() && T66GI && TexPool)
			{
				FHeroData SlotHeroData;
				if (T66GI->GetHeroData(SlotHeroID, SlotHeroData))
				{
					const TSoftObjectPtr<UTexture2D> PortraitSoft = T66GI->ResolveHeroPortrait(
						SlotHeroData,
						T66GI->SelectedHeroBodyType,
						ET66HeroPortraitVariant::Half);
					if (!PortraitSoft.IsNull())
					{
						T66SlateTexture::BindSharedBrushAsync(
							TexPool,
							PortraitSoft,
							this,
							PartyHeroPortraitBrushes[SlotIndex],
							FName(TEXT("PartyHeroPortrait"), SlotIndex + 1),
							true);
					}
				}
			}

			const TSharedRef<SWidget> SlotBaseContent =
				bHasAvatar
				? StaticCastSharedRef<SWidget>(
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					[
						SAssignNew(PartyAvatarImageWidgets[SlotIndex], SImage)
						.Image(PartyAvatarBrushes.IsValidIndex(SlotIndex) && PartyAvatarBrushes[SlotIndex].IsValid()
							? PartyAvatarBrushes[SlotIndex].Get()
							: nullptr)
					])
				: StaticCastSharedRef<SWidget>(
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Top)
					.Padding(0.f, 10.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(12.f)
						.HeightOverride(12.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(PlaceholderTint.R, PlaceholderTint.G, PlaceholderTint.B, PlaceholderOpacity))
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Bottom)
					.Padding(0.f, 0.f, 0.f, 10.f)
					[
						SNew(SBox)
						.WidthOverride(20.f)
						.HeightOverride(14.f)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(FLinearColor(PlaceholderTint.R, PlaceholderTint.G, PlaceholderTint.B, PlaceholderOpacity))
						]
					]);

			const TSharedRef<SWidget> ProfileSlot =
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(GetHeroSelectionPartySlotBrush())
						.ColorAndOpacity(bOccupiedSlot ? FLinearColor::White : SlotAccent)
					]
					+ SOverlay::Slot()
					.Padding(FMargin(9.f))
					[
						SlotBaseContent
					];

			const TSharedRef<SWidget> HeroSlot =
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(bOccupiedSlot ? HeroSelectionChromeAccent(0.95f) : HeroSelectionChromeAccentInactive(0.75f))
				.Padding(1.f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(HeroSelectionChromeInnerFill())
					.Padding(3.f)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFit)
							[
								SAssignNew(PartyHeroPortraitImageWidgets[SlotIndex], SImage)
								.Image(PartyHeroPortraitBrushes.IsValidIndex(SlotIndex) && PartyHeroPortraitBrushes[SlotIndex].IsValid()
									? PartyHeroPortraitBrushes[SlotIndex].Get()
									: nullptr)
								.Visibility(SlotHeroID.IsNone() ? EVisibility::Collapsed : EVisibility::Visible)
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Visibility(SlotHeroID.IsNone() ? EVisibility::Visible : EVisibility::Collapsed)
							.Text(bOccupiedSlot
								? NSLOCTEXT("T66.HeroSelection", "PartyHeroUnknown", "?")
								: NSLOCTEXT("T66.HeroSelection", "PartyHeroEmpty", "+"))
							.Font(FT66Style::Tokens::FontBold(20))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
					]
				];

			PartySlots->AddSlot()
				.AutoWidth()
				.Padding(SlotIndex > 0 ? FMargin(PartyMemberGap, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					SNew(SBox)
					.WidthOverride(PartyMemberWidth)
					.HeightOverride(PartyMemberHeight)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								MakeReadyBanner(bMemberReady, bOccupiedSlot)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SBox)
								.WidthOverride(PartyProfileSize.X)
								.HeightOverride(PartyProfileSize.Y)
								[
									ProfileSlot
								]
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(PartySlotGap, 0.f, 0.f, 0.f)
						.VAlign(VAlign_Bottom)
						[
							SNew(SBox)
							.WidthOverride(PartyHeroSize.X)
							.HeightOverride(PartyHeroSize.Y)
							[
								HeroSlot
							]
						]
					]
				];
		}

		return SNew(SBox)
			.WidthOverride(PartyFooterWidth)
			.HeightOverride(FooterPanelMinHeight)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				MakeHeroSelectionContentShell(
					SNew(SBox)
					.WidthOverride(FMath::Max(1.f, PartyFooterWidth - 20.f))
					.HeightOverride(FMath::Max(1.f, FooterPanelMinHeight - 20.f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Fill)
					.Clipping(EWidgetClipping::ClipToBounds)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFitX)
							.StretchDirection(EStretchDirection::DownOnly)
							[
								PartySlots
							]
						]
					],
					FMargin(10.f + OuterPanelBleed, 10.f, 10.f, 10.f))
			];
	};

	auto MakeTopStripBackButton = [this,
		BackText,
		SecondaryButtonFontSize,
		TopStripBackButtonWidth,
		TopStripBackButtonHeight]() -> TSharedRef<SWidget>
	{
		return MakeHeroSelectionButton(
			FT66ButtonParams(
				BackText,
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBackClicked),
				ET66ButtonType::Neutral)
			.SetMinWidth(TopStripBackButtonWidth)
			.SetHeight(TopStripBackButtonHeight)
			.SetFontSize(SecondaryButtonFontSize)
			.SetPadding(FMargin(12.f, 6.f, 12.f, 4.f)));
	};

	auto MakeRunControls = [this,
		bUsePartyReadyFlow,
		bIsLocalPartyHost,
		bCanEditDifficulty,
		bCanStartPartyRun,
		PrimaryActionText,
		PrimaryCtaFontSize,
		DifficultyMenuFontSize,
		SecondaryButtonFontSize,
		FooterActionHeight,
		ChallengesTooltipText,
		ModsTooltipText,
		Loc,
		RunFooterContentWidth,
		FooterPanelMinHeight]() -> TSharedRef<SWidget>
	{
		auto WrapRunControls = [RunFooterContentWidth, FooterPanelMinHeight](const TSharedRef<SWidget>& Content) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.HeightOverride(FooterPanelMinHeight)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					MakeHeroSelectionContentShell(
						SNew(SBox)
						.WidthOverride(FMath::Max(1.f, RunFooterContentWidth))
						.HeightOverride(FMath::Max(1.f, FooterPanelMinHeight - 24.f))
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Center)
						.Clipping(EWidgetClipping::ClipToBounds)
						[
							SNew(SScaleBox)
							.Stretch(EStretch::ScaleToFitX)
							.StretchDirection(EStretchDirection::DownOnly)
							[
								Content
							]
						],
						FMargin(12.f))
				];
		};
		auto MakeCommunityContentButtons = [this, FooterActionHeight, ChallengesTooltipText, ModsTooltipText, SecondaryButtonFontSize]() -> TSharedRef<SWidget>
		{
			const float TextButtonWidth = 108.f;
			const float ButtonGap = 8.f;
			const int32 TextFontSize = FMath::Max(SecondaryButtonFontSize - 5, 12);

			auto MakeTextButton = [this, FooterActionHeight, TextButtonWidth, TextFontSize](const FText& Label, const FText& Tooltip, const FOnClicked& OnClicked) -> TSharedRef<SWidget>
			{
				return SNew(SBox)
					.WidthOverride(TextButtonWidth)
					.HeightOverride(FooterActionHeight)
					[
						MakeHeroSelectionButton(FT66ButtonParams(
							Tooltip,
							OnClicked,
							ET66ButtonType::Neutral)
							.SetMinWidth(TextButtonWidth)
							.SetHeight(FooterActionHeight)
							.SetPadding(FMargin(4.f, 8.f))
							.SetContent(
								SNew(SBox)
								.WidthOverride(TextButtonWidth)
								.HeightOverride(FooterActionHeight)
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(Label)
									.Font(FT66Style::Tokens::FontBold(TextFontSize))
									.ColorAndOpacity(FT66Style::Tokens::Text)
									.Justification(ETextJustify::Center)
									.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
								]))
					];
			};

			return SNew(SBox)
				.WidthOverride((TextButtonWidth * 2.f) + ButtonGap)
				.HeightOverride(FooterActionHeight)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					[
						MakeTextButton(
							NSLOCTEXT("T66.HeroSelection", "ChallengesButtonText", "CHALLENGES"),
							ChallengesTooltipText,
							FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleChallengesClicked))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(ButtonGap, 0.f, 0.f, 0.f)
					[
						MakeTextButton(
							NSLOCTEXT("T66.HeroSelection", "ModsButtonText", "MODS"),
							ModsTooltipText,
							FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleModsClicked))
					]
				];
		};

		if (bUsePartyReadyFlow && !bIsLocalPartyHost)
		{
			return WrapRunControls(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					MakeHeroSelectionButton(FT66ButtonParams(
						PrimaryActionText,
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked),
						ET66ButtonType::Primary)
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(12.f, 8.f))
						.SetFontSize(PrimaryCtaFontSize))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.f, 0.f, 0.f, 0.f)
				[
					MakeCommunityContentButtons()
				]);
		}

		return WrapRunControls(
			SNew(SHorizontalBox)
			.Clipping(EWidgetClipping::ClipToBounds)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(230.f)
				.HeightOverride(FooterActionHeight)
				.IsEnabled(bCanEditDifficulty)
				[
					MakeHeroSelectionDropdown(FT66DropdownParams(
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SAssignNew(DifficultyDropdownText, STextBlock)
							.Text(CurrentDifficultyOption.IsValid()
								? FText::FromString(*CurrentDifficultyOption)
								: (Loc ? Loc->GetText_Easy() : NSLOCTEXT("T66.Difficulty", "Easy", "Easy")))
							.Font(FT66Style::Tokens::FontBold(DifficultyMenuFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
							.Clipping(EWidgetClipping::ClipToBounds)
						],
						[this, FooterActionHeight, DifficultyMenuFontSize]()
						{
							TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
							for (const TSharedPtr<FString>& Opt : DifficultyOptions)
							{
								if (!Opt.IsValid())
								{
									continue;
								}

								TSharedPtr<FString> Captured = Opt;
								Box->AddSlot()
									.AutoHeight()
									[
										FT66Style::MakeDropdownOptionButton(
											FText::FromString(*Opt),
											FOnClicked::CreateLambda([this, Captured]()
											{
												OnDifficultyChanged(Captured, ESelectInfo::Direct);
												FSlateApplication::Get().DismissAllMenus();
												return FReply::Handled();
											}),
											CurrentDifficultyOption.IsValid() && *CurrentDifficultyOption == *Opt,
											0.f,
											FooterActionHeight,
											DifficultyMenuFontSize,
											FMargin(10.f, 8.f, 10.f, 6.f))
									];
							}
							return Box;
						})
						.SetMinWidth(230.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(10.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(250.f)
				.HeightOverride(FooterActionHeight)
				.IsEnabled(bCanStartPartyRun)
				[
					MakeHeroSelectionSpriteButton(FT66ButtonParams(
						PrimaryActionText,
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked),
						bCanStartPartyRun ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
						.SetMinWidth(250.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(12.f, 8.f))
						.SetFontSize(PrimaryCtaFontSize),
						TAttribute<ET66HeroSpriteFamily>(bCanStartPartyRun ? ET66HeroSpriteFamily::ToggleOn : ET66HeroSpriteFamily::CompactNeutral))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeCommunityContentButtons()
			]);
	};

	TSharedRef<SWidget> LeftPanelSwitcher =
		SAssignNew(LeftPanelWidgetSwitcher, SWidgetSwitcher)
		.WidgetIndex(GetLeftPanelWidgetIndex())
		+ SWidgetSwitcher::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.HeightOverride(LeftSkinsCardHeight)
				[
					SNew(SScrollBox)
					.ScrollBarStyle(GetHeroSelectionReferenceScrollBarStyle())
					.ScrollBarThickness(FVector2D(14.f, 14.f))
					.ScrollBarPadding(FMargin(8.f, 0.f, 0.f, 0.f))
					+ SScrollBox::Slot()
					[
						SkinsListBoxWidget.ToSharedRef()
					]
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SBox)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.HeightOverride(142.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(SBox)
						.HeightOverride(3.f)
						.HAlign(HAlign_Fill)
						[
							SNew(SBorder)
							.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
							.BorderBackgroundColor(HeroSelectionChromeAccent())
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(SBox)
						.HeightOverride(28.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.HeroSelection", "DrugsPanelHeader", "DRUGS"))
							.Font(FT66Style::Tokens::FontBold(FMath::Max(SecondaryButtonFontSize + 2, 16)))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					[
						MakeTemporaryBuffLoadoutPanel()
					]
				]
			]
		]
		+ SWidgetSwitcher::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SBox)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					MakeHeroSelectionButton(
						FT66ButtonParams(
							SkinsText,
							FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleStatsClicked),
							ET66ButtonType::Neutral)
						.SetMinWidth(88.f)
						.SetFontSize(SecondaryButtonFontSize)
						.SetColor(TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
						{
							return bShowingStatsPanel ? FT66Style::ButtonPrimary() : FT66Style::ButtonNeutral();
						})))
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(HeroFullStatsHost, SBox)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroStatsHint", "Select a hero to view their full stats."))
					.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
					.AutoWrapText(true)
				]
			]
		]
		+ SWidgetSwitcher::Slot()
		[
			BuildInlineRetroFXPanel()
		];

	TSharedRef<SWidget> LeftSidePanel = MakeHeroSelectionPanelShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return bShowingInlineRetroFXPanel
						? NSLOCTEXT("T66.HeroSelection", "RetroFXInlineHeader", "RETRO FX")
						: NSLOCTEXT("T66.HeroSelection", "OutfitHeader", "FASHION");
				})
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 2))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
				.Visibility_Lambda([this]() -> EVisibility
				{
					return bShowingInlineRetroFXPanel ? EVisibility::Visible : EVisibility::Collapsed;
				})
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				MakeTopStripBackButton()
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				SNew(SWidgetSwitcher)
				.WidgetIndex_Lambda([this]() -> int32
				{
					return bShowingInlineRetroFXPanel ? 1 : 0;
				})
				+ SWidgetSwitcher::Slot()
				[
					MakeBalanceBadge()
				]
				+ SWidgetSwitcher::Slot()
				[
					MakeHeroSelectionButton(
						FT66ButtonParams(
							NSLOCTEXT("T66.HeroSelection", "RetroFXInlineApplyButton", "APPLY"),
							FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleApplyInlineRetroFXClicked),
							ET66ButtonType::Primary)
						.SetMinWidth(104.f)
						.SetHeight(TopStripBackButtonHeight)
						.SetFontSize(SecondaryButtonFontSize)
						.SetPadding(FMargin(10.f, 6.f, 10.f, 4.f)))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 12.0f)
		[
			SNew(SBox)
			.HeightOverride(36.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Visibility_Lambda([this]() -> EVisibility
			{
				return bShowingInlineRetroFXPanel ? EVisibility::Collapsed : EVisibility::Visible;
			})
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "SkinsPanelHeader", "SKINS"))
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 2))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			LeftPanelSwitcher
		],
		FMargin(FT66Style::Tokens::Space3 + OuterPanelBleed, FT66Style::Tokens::Space3 + OuterPanelBleed, FT66Style::Tokens::Space3, FT66Style::Tokens::Space3));

	TSharedRef<SWidget> CenterColumnWidget =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			[
				MakeSelectionBar(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFitX)
						.StretchDirection(EStretchDirection::DownOnly)
						[
							MakeHeroStripControls()
						]
					]
					)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(0.0f, 0.0f, 0.0f, 0.0f)
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				HeroPreviewController
					? HeroPreviewController->CreateHeroPreviewWidget(HeroPreviewColor)
					: StaticCastSharedRef<SWidget>(SNew(SBox))
			]
		]
		;

	TSharedRef<SWidget> CompanionFooterPanel =
		SNew(SBox)
		.WidthOverride(CompanionFooterWidth)
		.HeightOverride(FooterPanelMinHeight)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			MakeHeroSelectionContentShell(
				SNew(SBox)
				.WidthOverride(CompanionFooterContentWidth)
				.HeightOverride(FMath::Max(1.f, FooterPanelMinHeight - 16.f))
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.Clipping(EWidgetClipping::ClipToBounds)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					.VAlign(VAlign_Center)
					[
						MakeBodyToggleStrip()
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					.Padding(0.0f, 4.0f, 0.0f, 0.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						MakeCompanionStripControls()
					]
				],
				FMargin(10.f, 8.f))
		];

	auto MakeHeroAbilityPanel = [
		this,
		Loc,
		SecondaryButtonFontSize,
		BodyTextFontSize,
		RightAbilityIconButtonSize,
		RightAbilityIconSize]() -> TSharedRef<SWidget>
	{
		auto MakeAbilityItem = [
			this,
			Loc,
			SecondaryButtonFontSize,
			BodyTextFontSize,
			RightAbilityIconButtonSize,
			RightAbilityIconSize](
			const FText& Label,
			const bool bUltimate) -> TSharedRef<SWidget>
		{
			const FOnClicked OnClicked = bUltimate
				? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleUltimatePreviewClicked)
				: FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandlePassivePreviewClicked);
			const TSharedPtr<FSlateBrush>& IconBrush = bUltimate ? HeroUltimateIconBrush : HeroPassiveIconBrush;

			return SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.f, 0.f, 0.f, 4.f)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 1))
					.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.ToolTip(TAttribute<TSharedPtr<IToolTip>>::CreateLambda([this, Loc, bUltimate]() -> TSharedPtr<IToolTip>
					{
						FHeroData HeroData;
						if (!GetPreviewedHeroData(HeroData))
						{
							return MakeHeroSelectionAbilityTooltip(
								bUltimate ? NSLOCTEXT("T66.HeroSelection", "UltimateTooltipFallbackTitle", "Ultimate") : NSLOCTEXT("T66.HeroSelection", "PassiveTooltipFallbackTitle", "Passive"),
								bUltimate ? NSLOCTEXT("T66.HeroSelection", "UltimateTooltipFallbackBody", "Select a hero to inspect their ultimate.") : NSLOCTEXT("T66.HeroSelection", "PassiveTooltipFallbackBody", "Select a hero to inspect their passive."));
						}

						return MakeHeroSelectionAbilityTooltip(
							bUltimate
								? (Loc ? Loc->GetText_UltimateName(HeroData.UltimateType) : NSLOCTEXT("T66.HeroSelection", "UltimateFallbackName", "Ultimate"))
								: (Loc ? Loc->GetText_PassiveName(HeroData.PassiveType) : NSLOCTEXT("T66.HeroSelection", "PassiveFallbackName", "Passive")),
							bUltimate
								? (Loc ? Loc->GetText_UltimateDescription(HeroData.UltimateType) : FText::GetEmpty())
								: (Loc ? Loc->GetText_PassiveDescription(HeroData.PassiveType) : FText::GetEmpty()));
					}))
					[
						FT66Style::MakeBareButton(
							FT66BareButtonParams(
								OnClicked,
								SNew(SBox)
								.WidthOverride(RightAbilityIconButtonSize)
								.HeightOverride(RightAbilityIconButtonSize)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(SBox)
										.WidthOverride(RightAbilityIconSize)
										.HeightOverride(RightAbilityIconSize)
										[
											FT66Style::MakeRetroUIIcon(StaticCastSharedRef<SWidget>(
												SNew(SImage)
												.Image_Lambda([IconBrush]() -> const FSlateBrush*
												{
													return IconBrush.IsValid() ? IconBrush.Get() : nullptr;
												})))
										]
									]
									+ SOverlay::Slot()
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(STextBlock)
										.Visibility_Lambda([IconBrush]() -> EVisibility
										{
											return IconBrush.IsValid() && ::IsValid(IconBrush->GetResourceObject())
											? EVisibility::Collapsed
											: EVisibility::Visible;
									})
									.Text(NSLOCTEXT("T66.HeroSelection", "AbilityPlaceholder", "?"))
									.Font(FT66Style::Tokens::FontBold(BodyTextFontSize))
									.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								]
							]
						)
						.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
						.SetColor(FLinearColor::Transparent)
						.SetPadding(FMargin(0.f)))
					]
				];
		};

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f, 0.f, 5.f, 0.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeAbilityItem(NSLOCTEXT("T66.HeroSelection", "WeaponLabel", "Weapon"), false)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(5.f, 0.f, 0.f, 0.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				MakeAbilityItem(NSLOCTEXT("T66.HeroSelection", "UltimateLabel", "Ultimate"), true)
			];
	};

	TSharedPtr<SImage> HeroPreviewVideoImageWidget;
	TSharedPtr<STextBlock> HeroPreviewPlaceholderTextWidget;
	TSharedPtr<SScaleBox> CompanionInfoPortraitScaleBoxWidget;
	TSharedPtr<STextBlock> CompanionPreviewPlaceholderTextWidget;

	auto MakeRecordInfoButton = [SecondaryButtonFontSize](const FText& Title, const FText& Body) -> TSharedRef<SWidget>
	{
		static FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder");
		return SNew(SBox)
			.WidthOverride(24.f)
			.HeightOverride(24.f)
			.ToolTip(MakeHeroSelectionAbilityTooltip(Title, Body, -1))
			[
				SNew(SButton)
				.ButtonStyle(&ButtonStyle)
				.ContentPadding(0.f)
				.OnClicked(FOnClicked::CreateLambda([]() -> FReply
				{
					return FReply::Handled();
				}))
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(HeroSelectionChromeAccent())
					.Padding(1.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(HeroSelectionChromeInnerFillAlt())
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordInlineInfoButton", "?"))
							.Font(FT66Style::Tokens::FontBold(FMath::Max(SecondaryButtonFontSize - 5, 12)))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
				]
			];
	};

	TSharedRef<SWidget> RightInfoBody =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			SNew(SBox)
			.HeightOverride(48.f)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(280.f)
					.HAlign(HAlign_Center)
					[
						SAssignNew(InfoTargetDropdownText, STextBlock)
						.Text(CurrentInfoTargetOption.IsValid()
							? FText::FromString(*CurrentInfoTargetOption)
							: NSLOCTEXT("T66.HeroSelection", "InfoTargetHeroFallback", "Hero"))
						.Font(FT66Style::Tokens::FontBold(HeroNameFontSize))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.Clipping(EWidgetClipping::ClipToBounds)
					]
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					MakeHeroSelectionButton(
						FT66ButtonParams(
							NSLOCTEXT("T66.HeroSelection", "TempSettingsButton", "SETTINGS"),
							FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleRetroFXSettingsClicked),
							ET66ButtonType::Neutral)
						.SetMinWidth(136.f)
						.SetHeight(32.f)
						.SetFontSize(SecondaryButtonFontSize - 1)
						.SetPadding(FMargin(10.f, 5.f)))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			MakeHeroSelectionParchmentPanelShell(
				SNew(SBox)
				.HeightOverride(RightPreviewPanelHeight)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						[
							SAssignNew(HeroPreviewVideoImageWidget, SImage)
							.Image_Lambda([this]() -> const FSlateBrush*
							{
								const UT66HeroSelectionPreviewController* HeroPreviewController = GetPreviewController();
								return HeroPreviewController ? HeroPreviewController->GetHeroPreviewVideoBrush() : nullptr;
							})
						]
					]
					+ SOverlay::Slot()
					[
						SAssignNew(CompanionInfoPortraitScaleBoxWidget, SScaleBox)
						.Stretch(EStretch::ScaleToFill)
						.Visibility(EVisibility::Collapsed)
						[
							SNew(SImage)
							.Image_Lambda([this]() -> const FSlateBrush*
							{
								const UT66HeroSelectionPreviewController* HeroPreviewController = GetPreviewController();
								return HeroPreviewController ? HeroPreviewController->GetCompanionInfoPortraitBrush() : nullptr;
							})
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(HeroPreviewPlaceholderTextWidget, STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "VideoPreview", "[VIDEO PREVIEW]"))
						.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.Justification(ETextJustify::Center)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SAssignNew(CompanionPreviewPlaceholderTextWidget, STextBlock)
						.Visibility(EVisibility::Collapsed)
						.Text(NSLOCTEXT("T66.HeroSelection", "CompanionPortraitPlaceholder", "Companion portrait unavailable."))
						.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.Justification(ETextJustify::Center)
					]
				],
				FMargin(5.0f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			MakeHeroSelectionParchmentRowShell(
				SNew(SBox)
				.MinDesiredHeight(56.f)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 6.f, 0.f)
					[
						MakeRecordInfoButton(
							NSLOCTEXT("T66.HeroSelection", "HeroRecordRankTooltipTitle", "Rank"),
							NSLOCTEXT("T66.HeroSelection", "HeroRecordRankTooltipBody", "All-time score placement for the selected difficulty, party size, and hero. N/A means no eligible score has been submitted yet."))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankLabel", "RANK"))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 3))
						.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(8.f, 0.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(34.f)
						.HeightOverride(34.f)
						[
							SAssignNew(HeroRecordRankImageWidget, SImage)
							.Image_Lambda([this]() -> const FSlateBrush*
							{
								return HeroRecordRankBrush.IsValid() ? HeroRecordRankBrush.Get() : nullptr;
							})
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SAssignNew(HeroRecordRankWidget, STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankDefault", "..."))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 3))
						.ColorAndOpacity(GetHeroSelectionParchmentText())
					]
				],
				FMargin(20.f, 9.f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			MakeHeroSelectionParchmentRowShell(
				SNew(SBox)
				.MinDesiredHeight(62.f)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 6.f, 0.f)
					[
						MakeRecordInfoButton(
							NSLOCTEXT("T66.HeroSelection", "HeroRecordMaestryTooltipTitle", "Mastery"),
							NSLOCTEXT("T66.HeroSelection", "HeroRecordMaestryTooltipBody", "Hero experience earned by playing this hero. The bar fills as the new mastery system gains backend data."))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMaestryLabel", "MASTERY"))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 3))
						.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(SBox)
						.HeightOverride(16.f)
						[
							T66ScreenSlateHelpers::MakeReferenceProgressBar(
								0.f,
								FVector2D(240.f, 16.f),
								FLinearColor(0.92f, 0.05f, 0.12f, 1.0f),
								FMargin(4.f, 2.f))
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMaestryDefault", "LV 1  0 / 100 XP"))
						.Font(FT66Style::Tokens::FontBold(FMath::Max(SecondaryButtonFontSize, 14)))
						.ColorAndOpacity(GetHeroSelectionParchmentText())
					]
				],
				FMargin(20.f, 9.f))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(RightInfoWidgetSwitcher, SWidgetSwitcher)
			.WidgetIndex(bShowingCompanionInfo ? 1 : 0)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f, 0.0f, 0.0f)
				[
					SNew(SBox)
					.HeightOverride(RightStatsCardHeight)
					[
						FT66Style::MakeBareButton(
							FT66BareButtonParams(
								FOnClicked::CreateLambda([this]() -> FReply
								{
									return bShowingHeroRecordInfoPanel ? FReply::Handled() : HandleOpenStatsPanelClicked();
								}),
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.FillHeight(1.f)
								.Padding(24.f, 8.f, 24.f, 6.f)
								.VAlign(VAlign_Center)
								[
									SAssignNew(HeroSummaryStatsHost, SBox)
									.Clipping(EWidgetClipping::ClipToBounds)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroDescriptionHint", "Select a hero to view their stats."))
										.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize - 2))
										.ColorAndOpacity(GetHeroSelectionParchmentMutedText())
										.AutoWrapText(true)
									]
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(12.f, 0.f, 12.f, 0.f)
								[
									SNew(SBox)
									.HeightOverride(3.f)
									.HAlign(HAlign_Fill)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(HeroSelectionChromeAccent())
									]
								])
							.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
							.SetColor(FLinearColor::Transparent)
							.SetPadding(FMargin(0.f)))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(SBox)
					.HeightOverride(RightUltRowHeight)
					[
						MakeHeroAbilityPanel()
					]
				]
			]
			+ SWidgetSwitcher::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 2.0f, 0.0f, 0.0f)
				[
					SAssignNew(CompanionHealsPerSecondWidget, STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "CompanionHealsPerSecondDefault", "Heals / Second: 0"))
					.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize + 3))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.AutoWrapText(true)
				]
			]
		];

	TSharedRef<SWidget> RightSidePanel = MakeHeroSelectionPanelShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			RightInfoBody
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f, 0.0f, 0.0f)
		[
			SAssignNew(RightFooterWidgetSwitcher, SWidgetSwitcher)
			.WidgetIndex(bShowingCompanionInfo ? 1 : 0)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SBox)
				.Visibility(EVisibility::Collapsed)
			]
			+ SWidgetSwitcher::Slot()
			[
				MakeCompanionUnityPanel()
			]
		],
		FMargin(FT66Style::Tokens::Space4, FT66Style::Tokens::Space4 + OuterPanelBleed, FT66Style::Tokens::Space4, FT66Style::Tokens::Space4),
		true);

	TSharedRef<SWidget> LeftFooterPanel =
		SNew(SBox)
		.WidthOverride(PartyFooterWidth)
		.HeightOverride(FooterPanelMinHeight)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			MakePartyBox()
		];

	TSharedRef<SWidget> RightFooterPanel =
		SNew(SBox)
		.WidthOverride(RunFooterWidth)
		.HeightOverride(FooterPanelMinHeight)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.Clipping(EWidgetClipping::ClipToBounds)
		[
			MakeRunControls()
		];

	TSharedRef<SWidget> ReferenceCanvas = SNew(SBox)
		.WidthOverride(ReferenceLayoutWidth)
		.HeightOverride(ReferenceLayoutHeight)
		[
			SNew(SConstraintCanvas)
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(-OuterPanelBleed, UpperPanelY - OuterPanelBleed, LeftPanelWidth + OuterPanelBleed, UpperSidePanelHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				LeftSidePanel
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(CenterPanelX, UpperPanelY, CenterPreviewWidth, UpperSidePanelHeight))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				CenterColumnWidget
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(ReferenceLayoutWidth - RightPanelWidth, UpperPanelY - OuterPanelBleed, RightPanelWidth + OuterPanelBleed, UpperSidePanelHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				RightSidePanel
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(-OuterPanelBleed, FooterPanelY, PartyFooterWidth + OuterPanelBleed, FooterPanelMinHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				LeftFooterPanel
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(CompanionFooterX, FooterPanelY, CompanionFooterWidth, FooterPanelMinHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				CompanionFooterPanel
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(RunFooterX, FooterPanelY, RunFooterWidth + OuterPanelBleed, FooterPanelMinHeight + OuterPanelBleed))
			.Anchors(FAnchors(0.f, 0.f))
			.Alignment(FVector2D::ZeroVector)
			[
				RightFooterPanel
			]
		];
	TSharedRef<SWidget> Root = SNew(SBox)
		.WidthOverride(LayoutViewportSize.X)
		.HeightOverride(LayoutViewportSize.Y)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::Both)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				ReferenceCanvas
			]
		];
	if (HeroPreviewController)
	{
		HeroPreviewController->BindPreviewPanelWidgets(
			HeroPreviewVideoImageWidget,
			HeroPreviewPlaceholderTextWidget,
			CompanionInfoPortraitScaleBoxWidget,
			CompanionPreviewPlaceholderTextWidget);
	}
	UpdateHeroDisplay();
	if (UWorld* World = GetWorld())
	{
		FTimerHandle PreviewRefreshHandle;
		World->GetTimerManager().SetTimer(
			PreviewRefreshHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				UpdateHeroDisplay();
			}),
			2.0f,
			false);
	}
	return Root;
}

