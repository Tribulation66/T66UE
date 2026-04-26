// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

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
		HeroPreviewController->EnsureHeroPreviewVideoResources();
		HeroPreviewController->EnsureCompanionPreviewBrush();
	}
	if (!HeroUltimateIconBrush.IsValid())
	{
		HeroUltimateIconBrush = MakeShared<FSlateBrush>();
		HeroUltimateIconBrush->DrawAs = ESlateBrushDrawType::Image;
		HeroUltimateIconBrush->ImageSize = FVector2D(56.f, 56.f);
	}
	if (!HeroPassiveIconBrush.IsValid())
	{
		HeroPassiveIconBrush = MakeShared<FSlateBrush>();
		HeroPassiveIconBrush->DrawAs = ESlateBrushDrawType::Image;
		HeroPassiveIconBrush->ImageSize = FVector2D(56.f, 56.f);
	}

	// Get localized text
	FText SkinsText = Loc ? Loc->GetText_Skins() : NSLOCTEXT("T66.HeroSelection", "Skins", "SKINS");
	FText StatsText = Loc ? Loc->GetText_BaseStatsHeader() : NSLOCTEXT("T66.HeroSelection", "BaseStatsHeader", "STATS");
	FText EnterText = Loc ? Loc->GetText_EnterTheTribulation() : NSLOCTEXT("T66.HeroSelection", "EnterTheTribulation", "ENTER THE TRIBULATION");
	FText ReadyText = NSLOCTEXT("T66.HeroSelection", "Ready", "READY");
	FText UnreadyText = NSLOCTEXT("T66.HeroSelection", "Unready", "UNREADY");
	FText WaitingForPartyText = NSLOCTEXT("T66.HeroSelection", "WaitingForParty", "WAITING FOR PARTY");
	FText BuyText = Loc ? Loc->GetText_Buy() : NSLOCTEXT("T66.Common", "Buy", "BUY");
	FText EquipText = Loc ? Loc->GetText_Equip() : NSLOCTEXT("T66.Common", "Equip", "EQUIP");
	FText PreviewText = Loc ? Loc->GetText_Preview() : NSLOCTEXT("T66.Common", "Preview", "PREVIEW");
	const FText ChallengesTooltipText = NSLOCTEXT("T66.HeroSelection", "ChallengesTooltip", "Challenges");

	// Initialize difficulty dropdown options
	DifficultyOptions.Empty();
	TArray<ET66Difficulty> Difficulties = {
		ET66Difficulty::Easy, ET66Difficulty::Medium, ET66Difficulty::Hard,
		ET66Difficulty::VeryHard, ET66Difficulty::Impossible
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

	UT66GameInstance* T66GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66PartySubsystem* PartySubsystem = T66GI ? T66GI->GetSubsystem<UT66PartySubsystem>() : nullptr;
	UT66SessionSubsystem* SessionSubsystem = T66GI ? T66GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
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
	if (!ACBalanceIconBrush.IsValid())
	{
		ACBalanceIconBrush = MakeShared<FSlateBrush>();
		ACBalanceIconBrush->DrawAs = ESlateBrushDrawType::Image;
		ACBalanceIconBrush->Tiling = ESlateBrushTileType::NoTile;
		ACBalanceIconBrush->ImageSize = FVector2D(40.f, 26.f);
		ACBalanceIconBrush->SetResourceObject(nullptr);
	}
	if (SelectionTexPool)
	{
		const TSoftObjectPtr<UTexture2D> BalanceIconSoft = GetHeroSelectionBalanceIcon();
		if (!BalanceIconSoft.IsNull())
		{
			T66SlateTexture::BindSharedBrushAsync(
				SelectionTexPool,
				BalanceIconSoft,
				this,
				ACBalanceIconBrush,
				FName(TEXT("HeroSelectionBalanceIcon")),
				/*bClearWhileLoading*/ true);
		}
	}
	if (!ChallengesButtonIconBrush.IsValid())
	{
		const FString ChallengesIconPath = GetHeroSelectionChallengesIconPath();
		if (IFileManager::Get().FileExists(*ChallengesIconPath))
		{
			ChallengesButtonIconBrush = MakeShared<FSlateImageBrush>(*ChallengesIconPath, FVector2D(24.f, 24.f));
		}
	}
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
			? T66TemporaryBuffUI::CreateSecondaryBuffBrush(SelectionTexPool, this, SlotStat, FVector2D(26.f, 26.f))
			: nullptr;
	}

	auto MakeSelectedTemporaryBuffSlot = [&](int32 SlotIndex) -> TSharedRef<SWidget>
	{
		const bool bFilled = SelectedTemporaryBuffBrushes.IsValidIndex(SlotIndex) && SelectedTemporaryBuffBrushes[SlotIndex].IsValid();
		const bool bOwnedForSlot = TempBuffSubsystem ? TempBuffSubsystem->IsSelectedSingleUseBuffSlotOwned(SlotIndex) : true;
		return MakeHeroSelectionButton(
			FT66ButtonParams(
				FText::GetEmpty(),
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked, SlotIndex),
				ET66ButtonType::Neutral)
				.SetMinWidth(48.f)
				.SetHeight(48.f)
				.SetPadding(FMargin(0.f))
				.SetColor(bOwnedForSlot ? FT66Style::Tokens::Panel : FLinearColor(0.14f, 0.07f, 0.07f, 1.0f))
				.SetContent(
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(bOwnedForSlot ? FT66Style::Tokens::Panel2 : FLinearColor(0.22f, 0.10f, 0.10f, 1.0f))
					]
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
							.Font(FT66Style::Tokens::FontBold(14))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted))
					]
				));
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
	const FSlateColor SelectionToggleFill = FLinearColor(0.055f, 0.055f, 0.06f, 1.0f);
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
			HeroCarouselPortraitBrushes[i]->ImageSize = FVector2D(48.f, 48.f);
		}
	}
	RefreshHeroCarouselPortraits();
	
	// Show 7 heroes centered on current (prev3..next3).
	for (int32 Offset = -HeroSelectionCarouselCenterIndex; Offset <= HeroSelectionCarouselCenterIndex; Offset++)
	{
		if (AllHeroIDs.Num() == 0) break;

		const float BoxSize = GetHeroSelectionCarouselBoxSize(Offset);
		const float Opacity = GetHeroSelectionCarouselOpacity(Offset);
		const int32 SlotIdx = Offset + HeroSelectionCarouselCenterIndex;
		const EVisibility InitialHeroSlotVisibility = HeroCarouselSlotVisibility.IsValidIndex(SlotIdx)
			? HeroCarouselSlotVisibility[SlotIdx]
			: EVisibility::Collapsed;
		const TSharedRef<SWidget> CarouselSlotWidget = bDotaTheme
			? StaticCastSharedRef<SWidget>(FT66Style::MakeSlotFrame(
				SAssignNew(HeroCarouselImageWidgets[SlotIdx], SImage)
				.Image(HeroCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? HeroCarouselPortraitBrushes[SlotIdx].Get() : nullptr)
				.Visibility(InitialHeroSlotVisibility)
				.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity)),
				TAttribute<FSlateColor>::CreateLambda([this, SlotIdx]() -> FSlateColor
				{
					return HeroCarouselSlotColors.IsValidIndex(SlotIdx)
						? FSlateColor(HeroCarouselSlotColors[SlotIdx])
						: FSlateColor(FT66Style::SlotInner());
				}),
				FMargin(2.f)))
			: StaticCastSharedRef<SWidget>(SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderBackgroundColor_Lambda([this, SlotIdx]() -> FSlateColor
					{
						return HeroCarouselSlotColors.IsValidIndex(SlotIdx)
							? FSlateColor(HeroCarouselSlotColors[SlotIdx])
							: FSlateColor(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f));
					})
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				]
				+ SOverlay::Slot()
				[
					SAssignNew(HeroCarouselImageWidgets[SlotIdx], SImage)
					.Image(HeroCarouselPortraitBrushes.IsValidIndex(SlotIdx) ? HeroCarouselPortraitBrushes[SlotIdx].Get() : nullptr)
					.Visibility(InitialHeroSlotVisibility)
					.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
				]);

		TSharedRef<SWidget> InteractiveCarouselSlot = CarouselSlotWidget;
		if (Offset == 0)
		{
			InteractiveCarouselSlot =
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.ContentPadding(0.f)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleHeroGridClicked))
				[
					CarouselSlotWidget
				];
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
			CompanionCarouselPortraitBrushes[i]->ImageSize = FVector2D(48.f, 48.f);
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
		const TSharedRef<SWidget> CompanionSlotWidget = bDotaTheme
			? StaticCastSharedRef<SWidget>(FT66Style::MakeSlotFrame(
				SNew(SOverlay)
				+ SOverlay::Slot()
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
					.Font(FT66Style::Tokens::FontBold(9))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Visibility(InitialCompanionSlotLabel.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
				],
				TAttribute<FSlateColor>::CreateLambda([this, SlotIdx]() -> FSlateColor
				{
					return CompanionCarouselSlotColors.IsValidIndex(SlotIdx)
						? FSlateColor(CompanionCarouselSlotColors[SlotIdx])
						: FSlateColor(FT66Style::SlotInner());
				}),
				FMargin(2.f)))
			: StaticCastSharedRef<SWidget>(SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderBackgroundColor_Lambda([this, SlotIdx]() -> FSlateColor
					{
						return CompanionCarouselSlotColors.IsValidIndex(SlotIdx)
							? FSlateColor(CompanionCarouselSlotColors[SlotIdx])
							: FSlateColor(FLinearColor(0.2f, 0.2f, 0.25f, 1.0f));
					})
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				]
				+ SOverlay::Slot()
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
					.Font(FT66Style::Tokens::FontBold(9))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Visibility(InitialCompanionSlotLabel.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
				]);

		TSharedRef<SWidget> InteractiveCompanionSlot = CompanionSlotWidget;
		if (Offset == 0)
		{
			InteractiveCompanionSlot =
				SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.ContentPadding(0.f)
				.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleCompanionGridClicked))
				[
					CompanionSlotWidget
				];
		}

		CompanionCarousel->AddSlot()
		.AutoWidth()
		.Padding(3.0f, 0.0f)
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
	const float CenterPanelFill = 0.40f;
	const float TopBarBottomGap = 0.f;
	const float PanelBottomInset = 0.f;
	const float PanelGap = 0.f;
	const auto ShrinkFont = [](int32 BaseSize, int32 Minimum = 7) -> int32 { return FMath::Max(Minimum, BaseSize - 4); };
	const float SidePanelMinWidth = 404.f;
	const float FooterToggleWidth = 70.f;
	const float FooterToggleHeight = 28.f;
	const float FooterActionHeight = 46.f;
	const float BalanceBadgeIconWidth = 40.f;
	const float BalanceBadgeIconHeight = 24.f;
	const int32 ScreenHeaderFontSize = ShrinkFont(15, 10);
	const int32 BodyToggleFontSize = ShrinkFont(9, 7);
	const int32 PrimaryCtaFontSize = ShrinkFont(11, 7);
	const int32 HeroArrowFontSize = ShrinkFont(12, 8);
	const int32 ACBalanceFontSize = ShrinkFont(14, 10);
	const int32 HeroNameFontSize = ShrinkFont(13, 9);
	const int32 SecondaryButtonFontSize = ShrinkFont(9, 7);
	const int32 EntityDropdownFontSize = SecondaryButtonFontSize + 3;
	const int32 BodyTextFontSize = ShrinkFont(10, 7);
	const int32 DifficultyMenuFontSize = PrimaryCtaFontSize;
	const float HeroArrowButtonWidth = bDotaTheme ? 30.f : 28.f;
	const float HeroArrowButtonHeight = bDotaTheme ? 26.f : 24.f;
	const TAttribute<FMargin> ScreenSafePadding = TAttribute<FMargin>::CreateLambda([this]() -> FMargin
	{
		const float TopInset = (UIManager && UIManager->IsFrontendTopBarVisible())
			? UIManager->GetFrontendTopBarContentHeight()
			: 0.f;
		return FMargin(0.f, TopInset, 0.f, 0.f);
	});

	auto MakeTemporaryBuffLoadoutPanel = [this,
		bDotaTheme,
		SelectionInsetFill,
		&MakeSelectedTemporaryBuffSlot]() -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> BuffSlotRow = SNew(SHorizontalBox);
		for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
		{
			BuffSlotRow->AddSlot()
			.AutoWidth()
			.Padding(SlotIndex + 1 < UT66BuffSubsystem::MaxSelectedSingleUseBuffs ? FMargin(0.f, 0.f, 6.f, 0.f) : FMargin(0.f))
			[
				MakeSelectedTemporaryBuffSlot(SlotIndex)
			];
		}

		return MakeHeroSelectionRowShell(
			BuffSlotRow,
			FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space3));
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
						SAssignNew(CompanionUnityProgressBar, SProgressBar)
						.Percent(FMath::Clamp(CompanionUnityProgress01, 0.f, 1.f))
						.FillColorAndOpacity(FLinearColor(0.20f, 0.65f, 0.35f, 1.0f))
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
		bDotaTheme,
		SelectionInsetFill,
		BalanceBadgeIconWidth,
		BalanceBadgeIconHeight,
		ACBalanceText,
		ACBalanceFontSize,
		SecondaryButtonFontSize]() -> TSharedRef<SWidget>
	{
		return MakeHeroSelectionRowShell(
			SNew(SHorizontalBox)
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
							SNew(SImage)
							.Image_Lambda([this]() -> const FSlateBrush*
							{
								return ACBalanceIconBrush.IsValid() && ::IsValid(ACBalanceIconBrush->GetResourceObject())
									? ACBalanceIconBrush.Get()
									: nullptr;
							})
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
			],
			FMargin(10.f, 5.f));
	};

	auto MakeFocusMaskFill = [SelectionShellFill]() -> TSharedRef<SWidget>
	{
		return SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(SelectionShellFill);
	};

	auto MakePreviewFocusMask = [bDotaTheme, &MakeFocusMaskFill, SidePanelMinWidth, TopBarBottomGap, PanelGap]() -> TSharedRef<SWidget>
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
					.WidthOverride(SidePanelMinWidth)
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
					.WidthOverride(SidePanelMinWidth)
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
		SelectionToggleFill,
		SelectionPanelFill]() -> TSharedRef<SWidget>
	{
		const FSlateColor TogglePanelColor = bDotaTheme ? SelectionPanelFill : FT66Style::Tokens::Panel2;
		const FSlateColor ToggleActiveColor = bDotaTheme ? SelectionToggleFill : FT66Style::Tokens::Accent2;

		auto MakeBodyToggleButton = [this, FooterToggleWidth, FooterToggleHeight, BodyToggleFontSize, ToggleActiveColor, TogglePanelColor](const TCHAR* Label, ET66BodyType BodyType) -> TSharedRef<SWidget>
		{
			const FOnClicked OnClicked = (BodyType == ET66BodyType::TypeA)
				? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBodyTypeAClicked)
				: FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBodyTypeBClicked);

			return MakeHeroSelectionButton(FT66ButtonParams(
				FText::AsCultureInvariant(Label),
				OnClicked,
				ET66ButtonType::Neutral)
				.SetMinWidth(FooterToggleWidth)
				.SetHeight(FooterToggleHeight)
				.SetFontSize(BodyToggleFontSize)
				.SetPadding(FMargin(6.f, 3.f, 6.f, 2.f))
				.SetColor(TAttribute<FSlateColor>::CreateLambda([this, BodyType, ToggleActiveColor, TogglePanelColor]() -> FSlateColor
				{
					return SelectedBodyType == BodyType ? ToggleActiveColor : TogglePanelColor;
				}))
				.SetContent(
					SNew(STextBlock)
					.Text(FText::AsCultureInvariant(Label))
					.Font(FT66Style::Tokens::FontBold(BodyToggleFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)));
		};

		return MakeHeroSelectionRowShell(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				MakeBodyToggleButton(TEXT("CHAD"), ET66BodyType::TypeA)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				MakeBodyToggleButton(TEXT("STACY"), ET66BodyType::TypeB)
			],
			FMargin(4.f, 4.f, 4.f, 3.f));
	};

	auto MakeWorldScrim = []() -> TSharedRef<SWidget>
	{
		return SNew(SBox).Visibility(EVisibility::Collapsed);
	};

	auto MakeSelectionBar = [bDotaTheme, SelectionPanelFill](TSharedRef<SWidget> Content) -> TSharedRef<SWidget>
	{
		if (bDotaTheme)
		{
			return MakeHeroSelectionContentShell(Content, FMargin(6.f, 6.f));
		}

		return MakeHeroSelectionContentShell(
			Content,
			FMargin(6.f, 6.f));
	};

	auto MakeHeroStripControls = [this,
		HeroArrowButtonWidth,
		HeroArrowButtonHeight,
		HeroArrowFontSize,
		HeroCarousel]() -> TSharedRef<SWidget>
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
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
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				HeroCarousel
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.0f, 0.0f, 0.0f, 0.0f)
			[
				MakeHeroSelectionButton(FT66ButtonParams(
					NSLOCTEXT("T66.Common", "Next", ">"),
					FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleNextClicked),
					ET66ButtonType::Neutral)
					.SetMinWidth(HeroArrowButtonWidth)
					.SetHeight(HeroArrowButtonHeight)
					.SetFontSize(HeroArrowFontSize))
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
					.SetMinWidth(204.f)
					.SetHeight(38.f)
					.SetFontSize(SecondaryButtonFontSize + 1))
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
									MakeHeroSelectionButton(FT66ButtonParams(
										FText::FromString(*Opt),
										FOnClicked::CreateLambda([this, Captured]()
										{
											OnSkinTargetChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}),
										ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetHeight(36.f)
										.SetFontSize(EntityDropdownFontSize)
										.SetPadding(FMargin(10.f, 8.f, 10.f, 6.f))
										.SetContent(
											SNew(SOverlay)
											+ SOverlay::Slot()
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											[
												SNew(STextBlock)
												.Text(FText::FromString(*Opt))
												.Font(FT66Style::Tokens::FontBold(EntityDropdownFontSize))
												.ColorAndOpacity(FT66Style::Tokens::Text)
												.Justification(ETextJustify::Center)
											]))
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
									MakeHeroSelectionButton(FT66ButtonParams(
										FText::FromString(*Opt),
										FOnClicked::CreateLambda([this, Captured]()
										{
											OnInfoTargetChanged(Captured, ESelectInfo::Direct);
											FSlateApplication::Get().DismissAllMenus();
											return FReply::Handled();
										}),
										ET66ButtonType::Neutral)
										.SetMinWidth(0.f)
										.SetHeight(36.f)
										.SetFontSize(EntityDropdownFontSize)
										.SetPadding(FMargin(10.f, 8.f, 10.f, 6.f))
										.SetContent(
											SNew(SOverlay)
											+ SOverlay::Slot()
											.HAlign(HAlign_Center)
											.VAlign(VAlign_Center)
											[
												SNew(STextBlock)
												.Text(FText::FromString(*Opt))
												.Font(FT66Style::Tokens::FontBold(EntityDropdownFontSize))
												.ColorAndOpacity(FT66Style::Tokens::Text)
												.Justification(ETextJustify::Center)
											]))
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
		SelectionTexPool,
		bUsePartyReadyFlow,
		bCanStartPartyRun]() -> TSharedRef<SWidget>
	{
		const FLinearColor ShellFill(0.f, 0.f, 0.f, 0.985f);
		const FLinearColor LeaderSlotAccent(0.29f, 0.24f, 0.13f, 1.0f);
		const FLinearColor PartySlotAccent(0.15f, 0.17f, 0.19f, 1.0f);
		const FLinearColor PartySlotAccentInactive(0.08f, 0.09f, 0.10f, 1.0f);
		const FLinearColor PlaceholderTint(0.20f, 0.22f, 0.24f, 0.55f);
		const FLinearColor ReadyFill(0.16f, 0.44f, 0.21f, 1.0f);
		const FLinearColor ReadyStroke(0.55f, 0.84f, 0.60f, 1.0f);
		const FLinearColor NotReadyFill(0.48f, 0.14f, 0.14f, 1.0f);
		const FLinearColor NotReadyStroke(0.92f, 0.48f, 0.48f, 1.0f);
		const FVector2D PartyAvatarSize(52.f, 52.f);
		const FVector2D PartyHeroPortraitSize(52.f, 72.f);
		const TArray<FT66PartyMemberEntry> PartyMembers = PartySubsystem ? PartySubsystem->GetPartyMembers() : TArray<FT66PartyMemberEntry>();
		UT66SteamHelper* SteamHelper = T66GI ? T66GI->GetSubsystem<UT66SteamHelper>() : nullptr;
		UT66SkinSubsystem* SkinSubsystem = T66GI ? T66GI->GetSubsystem<UT66SkinSubsystem>() : nullptr;
		PartyAvatarBrushes.SetNum(4);
		PartyHeroPortraitBrushes.SetNum(4);
		const bool bTreatPartyAsReadyByDefault = !bUsePartyReadyFlow || PartyMembers.Num() <= 1;

		auto MakeReadyIndicator = [&](const bool bReady, const float Size, const int32 FontSize) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.WidthOverride(Size)
				.HeightOverride(Size)
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
							.Text(FText::FromString(bReady ? TEXT("✓") : TEXT("X")))
							.Font(FT66Style::Tokens::FontBold(FontSize))
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

		struct FPartyHeroSelectionData
		{
			FName HeroID = NAME_None;
			ET66BodyType HeroBodyType = ET66BodyType::TypeA;
			FName HeroSkinID = UT66SkinSubsystem::DefaultSkinID;
		};

		auto ResolvePartyHeroSelection = [this, T66GI](const FT66PartyMemberEntry* Member) -> FPartyHeroSelectionData
		{
			FPartyHeroSelectionData SelectionData;
			if (!Member)
			{
				return SelectionData;
			}

			auto ApplyLocalSelection = [&SelectionData, T66GI]()
			{
				if (!T66GI)
				{
					return;
				}

				SelectionData.HeroID = T66GI->SelectedHeroID;
				SelectionData.HeroBodyType = T66GI->SelectedHeroBodyType;
				SelectionData.HeroSkinID = T66GI->SelectedHeroSkinID.IsNone()
					? UT66SkinSubsystem::DefaultSkinID
					: T66GI->SelectedHeroSkinID;
			};

			if (Member->bIsLocal)
			{
				ApplyLocalSelection();
			}

			if (UWorld* World = GetWorld())
			{
				if (AGameStateBase* GameState = World->GetGameState())
				{
					for (APlayerState* PlayerState : GameState->PlayerArray)
					{
						AT66SessionPlayerState* SessionPlayerState = Cast<AT66SessionPlayerState>(PlayerState);
						if (!SessionPlayerState)
						{
							continue;
						}

						const FT66LobbyPlayerInfo& LobbyInfo = SessionPlayerState->GetLobbyInfo();
						const bool bMatchesById = !Member->PlayerId.IsEmpty()
							&& !LobbyInfo.SteamId.IsEmpty()
							&& Member->PlayerId == LobbyInfo.SteamId;
						const bool bMatchesByName = !Member->DisplayName.IsEmpty()
							&& LobbyInfo.DisplayName.Equals(Member->DisplayName, ESearchCase::IgnoreCase);
						if (!bMatchesById && !bMatchesByName)
						{
							continue;
						}

						SelectionData.HeroID = LobbyInfo.SelectedHeroID;
						SelectionData.HeroBodyType = LobbyInfo.SelectedHeroBodyType;
						SelectionData.HeroSkinID = LobbyInfo.SelectedHeroSkinID.IsNone()
							? UT66SkinSubsystem::DefaultSkinID
							: LobbyInfo.SelectedHeroSkinID;
						return SelectionData;
					}
				}
			}

			if (SelectionData.HeroID.IsNone() && Member->bIsLocal)
			{
				ApplyLocalSelection();
			}

			return SelectionData;
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
			PartyAvatarBrushes[SlotIndex]->ImageSize = PartyAvatarSize;
			PartyAvatarBrushes[SlotIndex]->SetResourceObject(AvatarTexture);

			if (!PartyHeroPortraitBrushes[SlotIndex].IsValid())
			{
				PartyHeroPortraitBrushes[SlotIndex] = MakeShared<FSlateBrush>();
				PartyHeroPortraitBrushes[SlotIndex]->DrawAs = ESlateBrushDrawType::Image;
				PartyHeroPortraitBrushes[SlotIndex]->Tiling = ESlateBrushTileType::NoTile;
			}
			PartyHeroPortraitBrushes[SlotIndex]->ImageSize = PartyHeroPortraitSize;
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
			const float PortraitPlaceholderOpacity = bPartyEnabledSlot ? 0.40f : 0.18f;

			const FPartyHeroSelectionData HeroSelection = ResolvePartyHeroSelection(PartyMember);
			TSoftObjectPtr<UTexture2D> HeroPortraitSoft;
			if (T66GI && !HeroSelection.HeroID.IsNone())
			{
				if (SkinSubsystem)
				{
					HeroPortraitSoft = SkinSubsystem->GetSkinPortrait(
						ET66SkinEntityType::Hero,
						HeroSelection.HeroID,
						HeroSelection.HeroSkinID,
						/*bSelectionPortrait*/ true);
				}
				if (HeroPortraitSoft.IsNull())
				{
					HeroPortraitSoft = T66GI->ResolveHeroPortrait(
						HeroSelection.HeroID,
						HeroSelection.HeroBodyType,
						ET66HeroPortraitVariant::Half);
				}
			}
			if (SelectionTexPool && !HeroPortraitSoft.IsNull())
			{
				T66SlateTexture::BindSharedBrushAsync(
					SelectionTexPool,
					HeroPortraitSoft,
					this,
					PartyHeroPortraitBrushes[SlotIndex],
					FName(*FString::Printf(TEXT("HeroSelectionPartyHeroPortrait_%d"), SlotIndex)),
					/*bClearWhileLoading*/ true);
			}
			const bool bHasHeroPortrait = PartyHeroPortraitBrushes.IsValidIndex(SlotIndex)
				&& PartyHeroPortraitBrushes[SlotIndex].IsValid()
				&& ::IsValid(PartyHeroPortraitBrushes[SlotIndex]->GetResourceObject());

			const TSharedRef<SWidget> SlotBaseContent =
				bHasAvatar
				? StaticCastSharedRef<SWidget>(
					SAssignNew(PartyAvatarImageWidgets[SlotIndex], SImage)
					.Image(PartyAvatarBrushes.IsValidIndex(SlotIndex) && PartyAvatarBrushes[SlotIndex].IsValid()
						? PartyAvatarBrushes[SlotIndex].Get()
						: nullptr))
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

			const TSharedRef<SWidget> SlotContent =
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SlotBaseContent
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.Padding(FMargin(0.f, 0.f, -9.f, 0.f))
					[
						bOccupiedSlot
							? StaticCastSharedRef<SWidget>(MakeReadyIndicator(bMemberReady, 18.f, 9))
							: StaticCastSharedRef<SWidget>(SNew(SSpacer))
				];

			const TSharedRef<SWidget> PortraitContent =
				bHasHeroPortrait
				? StaticCastSharedRef<SWidget>(
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFill)
					[
						SAssignNew(PartyHeroPortraitImageWidgets[SlotIndex], SImage)
						.Image(PartyHeroPortraitBrushes.IsValidIndex(SlotIndex) && PartyHeroPortraitBrushes[SlotIndex].IsValid()
							? PartyHeroPortraitBrushes[SlotIndex].Get()
							: nullptr)
					])
				: StaticCastSharedRef<SWidget>(
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(PlaceholderTint.R, PlaceholderTint.G, PlaceholderTint.B, PortraitPlaceholderOpacity)));

			PartySlots->AddSlot()
				.AutoWidth()
				.Padding(SlotIndex > 0 ? FMargin(10.f, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.WidthOverride(PartyAvatarSize.X)
						.HeightOverride(PartyAvatarSize.Y)
						[
							MakeHeroSelectionRowShell(SlotContent, bLeaderSlot ? FMargin(1.f) : FMargin(6.f))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 8.f, 0.f, 0.f)
					[
						SNew(SBox)
						.WidthOverride(PartyHeroPortraitSize.X)
						.HeightOverride(PartyHeroPortraitSize.Y)
						[
							MakeHeroSelectionRowShell(PortraitContent, FMargin(1.f))
						]
					]
				];
		}

		return SNew(SBox)
			.MinDesiredHeight(162.f)
			[
				MakeHeroSelectionRowShell(
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						PartySlots
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SSpacer)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						MakeReadyIndicator(bCanStartPartyRun, 52.f, 18)
					],
					FMargin(14.f, 14.f, 14.f, 14.f))
			];
	};

	auto MakeRunControls = [this,
		bUsePartyReadyFlow,
		bIsLocalPartyHost,
		bCanEditDifficulty,
		bCanStartPartyRun,
		PrimaryActionText,
		PrimaryCtaFontSize,
		DifficultyMenuFontSize,
		FooterActionHeight,
		ChallengesTooltipText,
		Loc]() -> TSharedRef<SWidget>
	{
		auto WrapRunControls = [](const TSharedRef<SWidget>& Content) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.MinDesiredHeight(84.f)
				[
					MakeHeroSelectionRowShell(Content, FMargin(12.f))
				];
		};
		auto MakeChallengesButton = [this, FooterActionHeight, ChallengesTooltipText]() -> TSharedRef<SWidget>
		{
			const float IconButtonSize = FooterActionHeight;
			const float IconSize = FMath::Max(18.f, FooterActionHeight - 18.f);
			const TSharedRef<SWidget> ButtonContent = ChallengesButtonIconBrush.IsValid()
				? StaticCastSharedRef<SWidget>(
					SNew(SBox)
					.WidthOverride(IconSize)
					.HeightOverride(IconSize)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(ChallengesButtonIconBrush.Get())
					])
				: StaticCastSharedRef<SWidget>(
					SNew(STextBlock)
					.Text(NSLOCTEXT("T66.HeroSelection", "ChallengesFallbackShort", "C"))
					.Font(FT66Style::Tokens::FontBold(9))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center));

			return MakeHeroSelectionButton(FT66ButtonParams(
				ChallengesTooltipText,
				FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleChallengesClicked),
				ET66ButtonType::Neutral)
				.SetMinWidth(IconButtonSize)
				.SetHeight(FooterActionHeight)
				.SetPadding(FMargin(0.f))
				.SetContent(
					SNew(SBox)
					.WidthOverride(IconButtonSize)
					.HeightOverride(FooterActionHeight)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						ButtonContent
					]));
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
					MakeChallengesButton()
				]);
		}

		return WrapRunControls(
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.34f)
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
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
							.Font(FT66Style::Tokens::FontBold(PrimaryCtaFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)
							.Justification(ETextJustify::Center)
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
										MakeHeroSelectionButton(FT66ButtonParams(
											FText::FromString(*Opt),
											FOnClicked::CreateLambda([this, Captured]()
											{
												OnDifficultyChanged(Captured, ESelectInfo::Direct);
												FSlateApplication::Get().DismissAllMenus();
												return FReply::Handled();
											}),
											ET66ButtonType::Neutral)
											.SetMinWidth(0.f)
											.SetHeight(FooterActionHeight)
											.SetFontSize(DifficultyMenuFontSize)
											.SetPadding(FMargin(10.f, 8.f, 10.f, 6.f))
											.SetContent(
												SNew(SOverlay)
												+ SOverlay::Slot()
												.HAlign(HAlign_Center)
												.VAlign(VAlign_Center)
												[
													SNew(STextBlock)
													.Text(FText::FromString(*Opt))
													.Font(FT66Style::Tokens::FontBold(DifficultyMenuFontSize))
													.ColorAndOpacity(FT66Style::Tokens::Text)
													.Justification(ETextJustify::Center)
												]))
									];
							}
							return Box;
						})
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(10.f, 8.f)))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.58f)
			[
				SNew(SBox)
				.IsEnabled(bCanStartPartyRun)
				[
					MakeHeroSelectionSpriteButton(FT66ButtonParams(
						PrimaryActionText,
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked),
						bCanStartPartyRun ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
						.SetHeight(FooterActionHeight)
						.SetPadding(FMargin(12.f, 8.f))
						.SetFontSize(PrimaryCtaFontSize)
						.SetContent(SNew(STextBlock)
							.Text(PrimaryActionText)
							.Font(FT66Style::Tokens::FontBold(PrimaryCtaFontSize))
							.ColorAndOpacity(FT66Style::Tokens::Text)),
						TAttribute<ET66HeroSpriteFamily>(bCanStartPartyRun ? ET66HeroSpriteFamily::ToggleOn : ET66HeroSpriteFamily::CompactNeutral))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.f, 0.f, 0.f, 0.f)
			[
				MakeChallengesButton()
			]);
	};

	TSharedRef<SWidget> LeftPanelSwitcher =
		SAssignNew(LeftPanelWidgetSwitcher, SWidgetSwitcher)
		.WidgetIndex(bShowingStatsPanel ? 1 : 0)
		+ SWidgetSwitcher::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SBox)
				.HeightOverride(36.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SAssignNew(SkinTargetDropdownText, STextBlock)
					.Text(CurrentSkinTargetOption.IsValid()
						? FText::FromString(*CurrentSkinTargetOption)
						: NSLOCTEXT("T66.HeroSelection", "SkinTargetHeroFallback", "HERO"))
					.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 1))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Justification(ETextJustify::Center)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SkinsListBoxWidget.ToSharedRef()
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
		];

	TSharedRef<SWidget> LeftSidePanel = MakeHeroSelectionPanelShell(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
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
				MakeBalanceBadge()
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			LeftPanelSwitcher
		],
		FMargin(FT66Style::Tokens::Space3));

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
						MakeHeroStripControls()
					])
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
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, TopBarBottomGap, 0.0f, 0.0f)
		[
			MakeSelectionBar(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					MakeCompanionStripControls()
				])
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 8.0f, 0.0f, 0.0f)
		[
			MakeBodyToggleStrip()
		];

	auto MakeMedalInfoTooltip = [SecondaryButtonFontSize]() -> TSharedRef<SToolTip>
	{
		auto MakeTierLine = [SecondaryButtonFontSize](const FText& LeftText, const FText& RightText) -> TSharedRef<SWidget>
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.f, 0.f, 12.f, 0.f)
				[
					SNew(STextBlock)
					.Text(LeftText)
					.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
					.ColorAndOpacity(FT66Style::Tokens::Text)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text(RightText)
					.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				];
		};

		return SNew(SToolTip)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("T66.HeroSelection", "MedalTooltipHeader", "MEDALS"))
						.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 1))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierNone", "Unproven"), NSLOCTEXT("T66.HeroSelection", "MedalTierNoneDesc", "No tribulation clear yet."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierBronze", "Bronze"), NSLOCTEXT("T66.HeroSelection", "MedalTierBronzeDesc", "Cleared Easy."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierSilver", "Silver"), NSLOCTEXT("T66.HeroSelection", "MedalTierSilverDesc", "Cleared Medium."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierGold", "Gold"), NSLOCTEXT("T66.HeroSelection", "MedalTierGoldDesc", "Cleared Hard."))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierPlatinum", "Platinum"), NSLOCTEXT("T66.HeroSelection", "MedalTierPlatinumDesc", "Cleared Very Hard."))]
					+ SVerticalBox::Slot().AutoHeight()[MakeTierLine(NSLOCTEXT("T66.HeroSelection", "MedalTierDiamond", "Diamond"), NSLOCTEXT("T66.HeroSelection", "MedalTierDiamondDesc", "Cleared Impossible."))],
					FT66PanelParams(ET66PanelType::Panel)
						.SetColor(FT66Style::Tokens::Panel2)
						.SetPadding(FMargin(12.f, 10.f)))
			];
	};

	TSharedPtr<SImage> HeroPreviewVideoImageWidget;
	TSharedPtr<STextBlock> HeroPreviewPlaceholderTextWidget;
	TSharedPtr<SScaleBox> CompanionInfoPortraitScaleBoxWidget;
	TSharedPtr<STextBlock> CompanionPreviewPlaceholderTextWidget;

	TSharedRef<SWidget> RightInfoBody =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SBox)
			.HeightOverride(44.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "HeroSelectionGameTitle", "CHADPOCALYPSE"))
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 1))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SBox)
			.HeightOverride(36.f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SAssignNew(InfoTargetDropdownText, STextBlock)
				.Text(CurrentInfoTargetOption.IsValid()
					? FText::FromString(*CurrentInfoTargetOption)
					: NSLOCTEXT("T66.HeroSelection", "InfoTargetHeroFallback", "Hero"))
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 1))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			MakeHeroSelectionRowShell(
				SNew(SBox)
				.HeightOverride(184.0f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFill)
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
		.Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			MakeHeroSelectionRowShell(
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.15f)
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
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalLabel", "Medal"))
								.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(6.f, 0.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalInfo", "?"))
								.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
								.ToolTip(MakeMedalInfoTooltip())
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f, 0.0f, 0.0f)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(26.f)
								.HeightOverride(26.f)
								[
									SAssignNew(HeroRecordMedalImageWidget, SImage)
									.Image_Lambda([this]() -> const FSlateBrush*
									{
										return HeroRecordMedalBrush.IsValid() ? HeroRecordMedalBrush.Get() : nullptr;
									})
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.f)
							.VAlign(VAlign_Center)
							.Padding(8.f, 0.f, 0.f, 0.f)
							[
								SAssignNew(HeroRecordMedalWidget, STextBlock)
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalDefault", "Unproven"))
								.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 2))
								.ColorAndOpacity(HeroRecordMedalColor(ET66AccountMedalTier::None))
							]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.85f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordGamesLabel", "Games Played"))
							.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f, 0.0f, 0.0f)
						[
							SAssignNew(HeroRecordGamesWidget, STextBlock)
							.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordGamesDefault", "0"))
							.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 2))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.95f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(HeroRecordThirdLabelWidget, STextBlock)
							.Text(HeroRecordThirdMetricLabel(false))
							.Font(FT66Style::Tokens::FontRegular(SecondaryButtonFontSize))
							.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 2.0f, 0.0f, 0.0f)
						[
							SAssignNew(HeroRecordThirdValueWidget, STextBlock)
							.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordThirdDefault", "0"))
							.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 2))
							.ColorAndOpacity(FT66Style::Tokens::Text)
						]
					]
				],
				FMargin(FT66Style::Tokens::Space3, FT66Style::Tokens::Space2))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(RightInfoWidgetSwitcher, SWidgetSwitcher)
			.WidgetIndex(bShowingCompanionInfo ? 1 : 0)
			+ SWidgetSwitcher::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(0.0f, 2.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
					.ButtonColorAndOpacity(FLinearColor::Transparent)
					.ContentPadding(FMargin(0.f))
					.OnClicked(FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleOpenStatsPanelClicked))
					[
						MakeHeroSelectionRowShell(
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.Padding(0.f, 0.f, 8.f, 0.f)
								[
									SAssignNew(HeroSummaryStatsHost, SBox)
									[
										SNew(STextBlock)
										.Text(NSLOCTEXT("T66.HeroSelection", "SelectHeroDescriptionHint", "Select a hero to view their stats."))
										.Font(FT66Style::Tokens::FontRegular(BodyTextFontSize - 2))
										.ColorAndOpacity(FT66Style::Tokens::TextMuted)
										.AutoWrapText(true)
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Fill)
								[
									SNew(SBox)
									.WidthOverride(1.f)
									.HeightOverride(142.f)
									[
										SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(FLinearColor(0.30f, 0.34f, 0.42f, 0.9f))
									]
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(10.f, 0.f, 0.f, 0.f)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0.f, 0.f, 0.f, 8.f)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										.HAlign(HAlign_Center)
										.Padding(0.f, 0.f, 0.f, 4.f)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66.HeroSelection", "UltimateShortLabel", "ULT"))
											.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize - 2))
											.ColorAndOpacity(FT66Style::Tokens::TextMuted)
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										.HAlign(HAlign_Center)
										[
											SNew(SBox)
											.ToolTip(TAttribute<TSharedPtr<IToolTip>>::CreateLambda([this, Loc]() -> TSharedPtr<IToolTip>
											{
												FHeroData HeroData;
												if (!GetPreviewedHeroData(HeroData))
												{
													return MakeHeroSelectionAbilityTooltip(
														NSLOCTEXT("T66.HeroSelection", "UltimateTooltipFallbackTitle", "Ultimate"),
														NSLOCTEXT("T66.HeroSelection", "UltimateTooltipFallbackBody", "Select a hero to inspect their ultimate."));
												}

												return MakeHeroSelectionAbilityTooltip(
													Loc ? Loc->GetText_UltimateName(HeroData.UltimateType) : NSLOCTEXT("T66.HeroSelection", "UltimateFallbackName", "Ultimate"),
													Loc ? Loc->GetText_UltimateDescription(HeroData.UltimateType) : FText::GetEmpty());
											}))
											[
												MakeHeroSelectionButton(
													FT66ButtonParams(
														FText::GetEmpty(),
														FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleUltimatePreviewClicked),
														ET66ButtonType::Neutral)
													.SetMinWidth(64.f)
													.SetHeight(56.f)
													.SetPadding(FMargin(4.f))
													.SetColor(TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
													{
														const UT66HeroSelectionPreviewController* HeroPreviewController = GetPreviewController();
														return HeroPreviewController && HeroPreviewController->IsSelectedPreviewClip(ET66HeroSelectionPreviewClip::Ultimate)
															? FT66Style::ButtonPrimary()
															: FT66Style::ButtonNeutral();
													}))
													.SetContent(
														SNew(SBox)
														.WidthOverride(36.f)
														.HeightOverride(36.f)
														[
															SNew(SOverlay)
															+ SOverlay::Slot()
															[
																SNew(SImage)
																.Image_Lambda([this]() -> const FSlateBrush*
																{
																	return HeroUltimateIconBrush.IsValid() ? HeroUltimateIconBrush.Get() : nullptr;
																})
															]
															+ SOverlay::Slot()
															.HAlign(HAlign_Center)
															.VAlign(VAlign_Center)
															[
																SNew(STextBlock)
																.Visibility_Lambda([this]() -> EVisibility
																{
																	return HeroUltimateIconBrush.IsValid() && ::IsValid(HeroUltimateIconBrush->GetResourceObject())
																		? EVisibility::Collapsed
																		: EVisibility::Visible;
																})
																.Text(NSLOCTEXT("T66.HeroSelection", "UltimatePlaceholder", "?"))
																.Font(FT66Style::Tokens::FontBold(BodyTextFontSize - 1))
																.ColorAndOpacity(FT66Style::Tokens::TextMuted)
															]
														]))
											]
										]
									]
									+ SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										.HAlign(HAlign_Center)
										.Padding(0.f, 0.f, 0.f, 4.f)
										[
											SNew(STextBlock)
											.Text(NSLOCTEXT("T66.HeroSelection", "PassiveShortLabel", "PASSIVE"))
											.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize - 2))
											.ColorAndOpacity(FT66Style::Tokens::TextMuted)
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										.HAlign(HAlign_Center)
										[
											SNew(SBox)
											.ToolTip(TAttribute<TSharedPtr<IToolTip>>::CreateLambda([this, Loc]() -> TSharedPtr<IToolTip>
											{
												FHeroData HeroData;
												if (!GetPreviewedHeroData(HeroData))
												{
													return MakeHeroSelectionAbilityTooltip(
														NSLOCTEXT("T66.HeroSelection", "PassiveTooltipFallbackTitle", "Passive"),
														NSLOCTEXT("T66.HeroSelection", "PassiveTooltipFallbackBody", "Select a hero to inspect their passive."));
												}

												return MakeHeroSelectionAbilityTooltip(
													Loc ? Loc->GetText_PassiveName(HeroData.PassiveType) : NSLOCTEXT("T66.HeroSelection", "PassiveFallbackName", "Passive"),
													Loc ? Loc->GetText_PassiveDescription(HeroData.PassiveType) : FText::GetEmpty());
											}))
											[
												MakeHeroSelectionButton(
													FT66ButtonParams(
														FText::GetEmpty(),
														FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandlePassivePreviewClicked),
														ET66ButtonType::Neutral)
													.SetMinWidth(64.f)
													.SetHeight(56.f)
													.SetPadding(FMargin(4.f))
													.SetColor(TAttribute<FSlateColor>::CreateLambda([this]() -> FSlateColor
													{
														const UT66HeroSelectionPreviewController* HeroPreviewController = GetPreviewController();
														return HeroPreviewController && HeroPreviewController->IsSelectedPreviewClip(ET66HeroSelectionPreviewClip::Passive)
															? FT66Style::ButtonPrimary()
															: FT66Style::ButtonNeutral();
													}))
													.SetContent(
														SNew(SBox)
														.WidthOverride(36.f)
														.HeightOverride(36.f)
														[
															SNew(SOverlay)
															+ SOverlay::Slot()
															[
																SNew(SImage)
																.Image_Lambda([this]() -> const FSlateBrush*
																{
																	return HeroPassiveIconBrush.IsValid() ? HeroPassiveIconBrush.Get() : nullptr;
																})
															]
															+ SOverlay::Slot()
															.HAlign(HAlign_Center)
															.VAlign(VAlign_Center)
															[
																SNew(STextBlock)
																.Visibility_Lambda([this]() -> EVisibility
																{
																	return HeroPassiveIconBrush.IsValid() && ::IsValid(HeroPassiveIconBrush->GetResourceObject())
																		? EVisibility::Collapsed
																		: EVisibility::Visible;
																})
																.Text(NSLOCTEXT("T66.HeroSelection", "PassivePlaceholder", "?"))
																.Font(FT66Style::Tokens::FontBold(BodyTextFontSize - 1))
																.ColorAndOpacity(FT66Style::Tokens::TextMuted)
															]
														]))
											]
										]
									]
								],
								FMargin(12.f, 10.f))
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
				MakeTemporaryBuffLoadoutPanel()
			]
			+ SWidgetSwitcher::Slot()
			[
				MakeCompanionUnityPanel()
			]
		],
		FMargin(FT66Style::Tokens::Space4),
		true);

	TSharedRef<SWidget> LeftFooterPanel =
		SNew(SBox)
		.WidthOverride(SidePanelMinWidth)
		[
			MakePartyBox()
		];

	TSharedRef<SWidget> RightFooterPanel =
		SNew(SBox)
		.WidthOverride(SidePanelMinWidth)
		[
			MakeRunControls()
		];

	TSharedRef<SWidget> Root = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
		.BorderBackgroundColor(FLinearColor::Transparent)
		.Padding(ScreenSafePadding)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBox)
					.WidthOverride(SidePanelMinWidth)
					[
						LeftSidePanel
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f)
				[
					LeftFooterPanel
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(PanelGap, 0.0f, PanelGap, 0.0f)
			[
				CenterColumnWidget
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBox)
					.WidthOverride(SidePanelMinWidth)
					[
						RightSidePanel
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f)
				[
					RightFooterPanel
				]
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
	if (HeroPreviewController)
	{
		HeroPreviewController->UpdateHeroPreviewVideo(PreviewedHeroID, bShowingCompanionInfo);
	}
	return Root;
}

