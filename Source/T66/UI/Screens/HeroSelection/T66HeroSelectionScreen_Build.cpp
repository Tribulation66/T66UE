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
	UT66AchievementsSubsystem* AchievementsSubsystem = T66GI ? T66GI->GetSubsystem<UT66AchievementsSubsystem>() : nullptr;
	const bool bDrugsUnlocked = HasUnlockedHeroSelectionDrugs(AchievementsSubsystem);
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
	if (FParse::Param(FCommandLine::Get(), TEXT("T66HeroSelectionBuffPicker")))
	{
		TemporaryBuffPickerSlotIndex = FMath::Clamp(TemporaryBuffPickerSlotIndex, 0, UT66BuffSubsystem::MaxSelectedSingleUseBuffs - 1);
		bShowingTemporaryBuffPicker = bDrugsUnlocked;
		if (TempBuffSubsystem)
		{
			TempBuffSubsystem->SetSelectedSingleUseBuffEditSlotIndex(TemporaryBuffPickerSlotIndex);
		}
	}
	if (FParse::Param(FCommandLine::Get(), TEXT("T66HeroSelectionRecordInfo")))
	{
		bShowingHeroRecordInfoPanel = true;
	}

	auto ResolveLooseIconBrush = [](
		const FString& RelativePath,
		const FVector2D& ImageSize,
		TSharedPtr<FSlateBrush>& Brush,
		TStrongObjectPtr<UTexture2D>& Texture,
		const TCHAR* DebugName)
	{
		if (!Brush.IsValid())
		{
			Brush = MakeShared<FSlateBrush>();
			Brush->DrawAs = ESlateBrushDrawType::Image;
			Brush->Tiling = ESlateBrushTileType::NoTile;
			Brush->TintColor = FSlateColor(FLinearColor::White);
			Brush->ImageSize = ImageSize;
		}

		if (!Texture.IsValid())
		{
			for (const FString& CandidatePath : T66RuntimeUITextureAccess::BuildLooseTextureCandidatePaths(RelativePath))
			{
				if (!FPaths::FileExists(CandidatePath))
				{
					continue;
				}

				if (UTexture2D* LoadedTexture = T66RuntimeUITextureAccess::ImportFileTexture(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					true,
					DebugName))
				{
					Texture.Reset(LoadedTexture);
					break;
				}

				if (UTexture2D* LoadedTexture = T66RuntimeUITextureAccess::ImportFileTextureWithGeneratedMips(
					CandidatePath,
					TextureFilter::TF_Trilinear,
					DebugName))
				{
					Texture.Reset(LoadedTexture);
					break;
				}
			}
		}

		Brush->SetResourceObject(Texture.IsValid() ? Texture.Get() : nullptr);
	};

	ResolveLooseIconBrush(GetHeroSelectionBalanceIconPath(), FVector2D(52.f, 34.f), ACBalanceIconBrush, ACBalanceIconTexture, TEXT("HeroSelectionBalanceIcon"));
	ResolveLooseIconBrush(GetHeroSelectionChallengesIconPath(), FVector2D(32.f, 32.f), ChallengesButtonIconBrush, ChallengesButtonIconTexture, TEXT("HeroSelectionChallengesIcon"));
	ResolveLooseIconBrush(GetHeroSelectionChadIconPath(), FVector2D(22.f, 22.f), ChadCompanionIconBrush, ChadCompanionIconTexture, TEXT("HeroSelectionChadIcon"));
	ResolveLooseIconBrush(GetHeroSelectionStacyIconPath(), FVector2D(22.f, 22.f), StacyCompanionIconBrush, StacyCompanionIconTexture, TEXT("HeroSelectionStacyIcon"));

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
			? T66TemporaryBuffUI::CreateSecondaryBuffBrush(SelectionTexPool, this, SlotStat, FVector2D(42.f, 42.f))
			: nullptr;
	}

	auto MakeSelectedTemporaryBuffSlot = [&, bDrugsUnlocked](int32 SlotIndex) -> TSharedRef<SWidget>
	{
		const bool bFilled = SelectedTemporaryBuffBrushes.IsValidIndex(SlotIndex) && SelectedTemporaryBuffBrushes[SlotIndex].IsValid();
		const bool bOwnedForSlot = TempBuffSubsystem ? TempBuffSubsystem->IsSelectedSingleUseBuffSlotOwned(SlotIndex) : true;
		return SNew(SBox)
			.IsEnabled(bDrugsUnlocked)
			[
				MakeHeroSelectionButton(
					FT66ButtonParams(
						FText::GetEmpty(),
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked, SlotIndex),
						ET66ButtonType::Neutral)
					.SetMinWidth(50.f)
					.SetHeight(50.f)
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
								.Font(FT66Style::Tokens::FontBold(18))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted))
						]
					))
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
			HeroCarouselPortraitBrushes[i]->ImageSize = FVector2D(48.f, 48.f);
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
					.Font(FT66Style::Tokens::FontBold(13))
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
					.Font(FT66Style::Tokens::FontBold(13))
					.ColorAndOpacity(FT66Style::Tokens::Text)
					.Visibility(InitialCompanionSlotLabel.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
				]);

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
	const float LeftPanelWidth = 400.f;
	const float RightPanelWidth = 440.f;
	const float FooterToggleWidth = 116.f;
	const float FooterToggleHeight = 44.f;
	const float FooterActionHeight = 46.f;
	const float BalanceBadgeIconWidth = 56.f;
	const float BalanceBadgeIconHeight = 34.f;
	const int32 ScreenHeaderFontSize = 21;
	const int32 BodyToggleFontSize = 21;
	const int32 PrimaryCtaFontSize = 22;
	const int32 HeroArrowFontSize = 20;
	const int32 ACBalanceFontSize = ScreenHeaderFontSize + 2;
	const int32 HeroNameFontSize = 19;
	const int32 SecondaryButtonFontSize = 18;
	const int32 EntityDropdownFontSize = 20;
	const int32 BodyTextFontSize = 15;
	const int32 DifficultyMenuFontSize = 20;
	const float HeroArrowButtonWidth = bDotaTheme ? 42.f : 38.f;
	const float HeroArrowButtonHeight = bDotaTheme ? 34.f : 32.f;
	const float TopStripBackButtonWidth = 112.f;
	const float TopStripBackButtonHeight = 34.f;
	const TAttribute<FMargin> ScreenSafePadding = TAttribute<FMargin>::CreateLambda([this]() -> FMargin
	{
		const float TopInset = (UIManager && UIManager->IsFrontendTopBarVisible())
			? UIManager->GetFrontendTopBarContentHeight()
			: 0.f;
		return FMargin(0.f, TopInset, 0.f, 0.f);
	});

	auto MakeTemporaryBuffLoadoutPanel = [this,
		SecondaryButtonFontSize,
		bDrugsUnlocked,
		&MakeSelectedTemporaryBuffSlot]() -> TSharedRef<SWidget>
	{
		TSharedRef<SHorizontalBox> BuffSlotRow = SNew(SHorizontalBox);
		for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
		{
			BuffSlotRow->AddSlot()
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
			[
				MakeSelectedTemporaryBuffSlot(SlotIndex)
			];
		}
		BuffSlotRow->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.IsEnabled(bDrugsUnlocked)
			[
				MakeHeroSelectionButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.HeroSelection", "TempBuffClear", "CLEAR"),
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleClearTemporaryBuffsClicked),
						ET66ButtonType::Neutral)
					.SetMinWidth(54.f)
					.SetHeight(50.f)
					.SetFontSize(SecondaryButtonFontSize - 3))
			]
		];

		return MakeHeroSelectionRowShell(
			SNew(SBox)
			.MinDesiredHeight(44.f)
			[
				BuffSlotRow
			],
			FMargin(6.f, 5.f));
	};

	auto MakeTemporaryBuffPickerModal = [this,
		TempBuffSubsystem,
		SelectionTexPool,
		Loc,
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
				? StaticCastSharedRef<SWidget>(
					SNew(SImage)
					.Image(BuffBrush.Get()))
				: StaticCastSharedRef<SWidget>(SNew(SSpacer));

			BuffRows->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				MakeHeroSelectionRowShell(
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
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(12.f, 0.f, 0.f, 0.f)
							[
								SNew(STextBlock)
								.Text(EffectText)
								.Font(FT66Style::Tokens::FontBold(BodyTextFontSize + 1))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
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
						MakeHeroSelectionButton(
							FT66ButtonParams(
								FText::GetEmpty(),
								FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffBuyClicked, StatType),
								ET66ButtonType::Neutral)
							.SetMinWidth(100.f)
							.SetHeight(36.f)
							.SetFontSize(15)
							.SetPadding(FMargin(10.f, 7.f))
							.SetContent(
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("T66.HeroSelection", "TempBuffBuy", "BUY"))
									.Font(FT66Style::Tokens::FontBold(15))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(6.f, 0.f, 0.f, 0.f)
								[
									SNew(STextBlock)
									.Text(FText::AsNumber(BuffCost))
									.Font(FT66Style::Tokens::FontBold(15))
									.ColorAndOpacity(FT66Style::Tokens::Text)
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								.Padding(5.f, 0.f, 0.f, 0.f)
								[
									SNew(SBox)
									.WidthOverride(23.f)
									.HeightOverride(18.f)
									[
										SNew(SImage)
										.Image_Lambda([this]() -> const FSlateBrush*
										{
											return ACBalanceIconBrush.IsValid() ? ACBalanceIconBrush.Get() : nullptr;
										})
									]
								])
							.SetEnabled(bCanBuy))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(8.f, 0.f, 0.f, 0.f)
					[
						MakeHeroSelectionButton(
							FT66ButtonParams(
								bFocusedSlotMatches
									? NSLOCTEXT("T66.HeroSelection", "TempBuffEquipped", "EQUIPPED")
									: NSLOCTEXT("T66.HeroSelection", "TempBuffEquip", "EQUIP"),
								FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleTemporaryBuffEquipClicked, StatType),
								bFocusedSlotMatches ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
							.SetMinWidth(108.f)
							.SetHeight(36.f)
							.SetFontSize(15)
							.SetPadding(FMargin(10.f, 7.f))
							.SetEnabled(bCanEquip || bFocusedSlotMatches))
					],
					FMargin(12.f, 10.f))
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
			680.f,
			560.f,
			true);
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
			const FOnClicked OnClicked = (BodyType == ET66BodyType::TypeA)
				? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBodyTypeAClicked)
				: FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleBodyTypeBClicked);

			return MakeHeroSelectionButton(FT66ButtonParams(
				FText::AsCultureInvariant(Label),
				OnClicked,
				ET66ButtonType::Neutral)
				.SetMinWidth(FooterToggleWidth)
				.SetHeight(FooterToggleHeight - 8.f)
				.SetFontSize(BodyToggleFontSize - 5)
				.SetPadding(FMargin(8.f, 4.f, 8.f, 3.f))
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
						.WidthOverride(20.f)
						.HeightOverride(20.f)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Image_Lambda([IconBrush]() -> const FSlateBrush*
								{
									return IconBrush.IsValid() && ::IsValid(IconBrush->GetResourceObject()) ? IconBrush.Get() : nullptr;
								})
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
								.Font(FT66Style::Tokens::FontBold(BodyToggleFontSize - 6))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::AsCultureInvariant(Label))
						.Font(FT66Style::Tokens::FontBold(BodyToggleFontSize - 5))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]));
		};

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				MakeBodyToggleButton(TEXT("CHAD"), TEXT("M"), ET66BodyType::TypeA, ChadCompanionIconBrush, ChadActiveColor)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				MakeBodyToggleButton(TEXT("STACY"), TEXT("F"), ET66BodyType::TypeB, StacyCompanionIconBrush, StacyActiveColor)
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
					.SetMinWidth(246.f)
					.SetHeight(34.f)
					.SetFontSize(SecondaryButtonFontSize - 1)
					.SetPadding(FMargin(12.f, 5.f)))
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
		SessionSubsystem,
		bUsePartyReadyFlow]() -> TSharedRef<SWidget>
	{
		const FLinearColor LeaderSlotAccent(0.29f, 0.24f, 0.13f, 1.0f);
		const FLinearColor PartySlotAccent(0.15f, 0.17f, 0.19f, 1.0f);
		const FLinearColor PartySlotAccentInactive(0.08f, 0.09f, 0.10f, 1.0f);
		const FLinearColor PlaceholderTint(0.20f, 0.22f, 0.24f, 0.55f);
		const FLinearColor ReadyFill(0.16f, 0.44f, 0.21f, 1.0f);
		const FLinearColor ReadyStroke(0.55f, 0.84f, 0.60f, 1.0f);
		const FLinearColor NotReadyFill(0.48f, 0.14f, 0.14f, 1.0f);
		const FLinearColor NotReadyStroke(0.92f, 0.48f, 0.48f, 1.0f);
		const FVector2D PartySlotSize(62.f, 62.f);
		const FVector2D PartyAvatarSize(44.f, 44.f);
		const TArray<FT66PartyMemberEntry> PartyMembers = PartySubsystem ? PartySubsystem->GetPartyMembers() : TArray<FT66PartyMemberEntry>();
		UT66SteamHelper* SteamHelper = T66GI ? T66GI->GetSubsystem<UT66SteamHelper>() : nullptr;
		PartyAvatarBrushes.SetNum(4);
		PartyHeroPortraitBrushes.Reset();
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

		TSharedRef<SHorizontalBox> PartySlots = SNew(SHorizontalBox);
		PartyAvatarImageWidgets.SetNum(4);
		PartyHeroPortraitImageWidgets.Reset();
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

			const bool bHasAvatar = AvatarTexture != nullptr;
			const bool bPartyEnabledSlot = SlotIndex < ActivePartySlots;
			const bool bOccupiedSlot = PartyMember != nullptr;
			const bool bLeaderSlot = SlotIndex == 0;
			const bool bMemberReady = bOccupiedSlot && (bTreatPartyAsReadyByDefault || PartyMember->bReady);
			const FLinearColor SlotAccent = bLeaderSlot
				? LeaderSlotAccent
				: (bOccupiedSlot ? PartySlotAccent : PartySlotAccentInactive);
			const float PlaceholderOpacity = bPartyEnabledSlot ? 0.55f : 0.28f;

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

			const TSharedRef<SWidget> SlotContent =
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
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					.Padding(FMargin(0.f, 0.f, 3.f, 3.f))
					[
						bOccupiedSlot
							? StaticCastSharedRef<SWidget>(MakeReadyIndicator(bMemberReady, 16.f, 8))
							: StaticCastSharedRef<SWidget>(SNew(SSpacer))
					];

			PartySlots->AddSlot()
				.AutoWidth()
				.Padding(SlotIndex > 0 ? FMargin(8.f, 0.f, 0.f, 0.f) : FMargin(0.f))
				[
					SNew(SBox)
					.WidthOverride(PartySlotSize.X)
					.HeightOverride(PartySlotSize.Y)
					[
						SlotContent
					]
				];
		}

		const bool bLocalPartyReady = !bUsePartyReadyFlow || !SessionSubsystem || SessionSubsystem->IsLocalLobbyReady();
		PartySlots->AddSlot()
			.AutoWidth()
			.Padding(10.f, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(98.f)
				.HeightOverride(62.f)
				[
					SNew(SBorder)
					.BorderImage(GetHeroSelectionPartySlotBrush())
					.BorderBackgroundColor(bLocalPartyReady ? ReadyFill : NotReadyFill)
					.Padding(7.f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(bLocalPartyReady
							? NSLOCTEXT("T66.HeroSelection", "PartyReadyBoxReady", "READY")
							: NSLOCTEXT("T66.HeroSelection", "PartyReadyBoxNotReady", "NOT READY"))
						.Font(FT66Style::Tokens::FontBold(15))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
					]
				]
			];

		return SNew(SBox)
			.MinDesiredHeight(82.f)
			[
				MakeHeroSelectionRowShell(
					SNew(SBox)
					.HAlign(HAlign_Left)
					[
						PartySlots
					],
					FMargin(10.f))
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
			const float IconButtonSize = FooterActionHeight + 14.f;
			const float IconSize = FMath::Max(22.f, FooterActionHeight - 14.f);
			const bool bHasChallengesIcon = ChallengesButtonIconBrush.IsValid() && ::IsValid(ChallengesButtonIconBrush->GetResourceObject());
			const TSharedRef<SWidget> ButtonContent = bHasChallengesIcon
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
					.Font(FT66Style::Tokens::FontBold(20))
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
			.FillWidth(0.36f)
			.VAlign(VAlign_Fill)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
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
			.FillWidth(0.50f)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.HeightOverride(FooterActionHeight)
				.IsEnabled(bCanStartPartyRun)
				[
					MakeHeroSelectionSpriteButton(FT66ButtonParams(
						PrimaryActionText,
						FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleEnterClicked),
						bCanStartPartyRun ? ET66ButtonType::Primary : ET66ButtonType::Neutral)
						.SetMinWidth(0.f)
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
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SkinsListBoxWidget.ToSharedRef()
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				MakeTemporaryBuffLoadoutPanel()
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
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("T66.HeroSelection", "OutfitHeader", "FASHION"))
				.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 2))
				.ColorAndOpacity(FT66Style::Tokens::Text)
				.Justification(ETextJustify::Center)
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
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 6.0f, 0.0f, 0.0f)
		[
			MakeBodyToggleStrip()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f, 0.0f, 0.0f)
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			[
				MakeCompanionStripControls()
			]
		]
		;

	auto MakeHeroAbilityPanel = [this, Loc, SecondaryButtonFontSize, BodyTextFontSize]() -> TSharedRef<SWidget>
	{
		auto MakeAbilityItem = [this, Loc, SecondaryButtonFontSize, BodyTextFontSize](
			const FText& Label,
			const bool bUltimate) -> TSharedRef<SWidget>
		{
			const FOnClicked OnClicked = bUltimate
				? FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandleUltimatePreviewClicked)
				: FOnClicked::CreateUObject(this, &UT66HeroSelectionScreen::HandlePassivePreviewClicked);
			const TSharedPtr<FSlateBrush>& IconBrush = bUltimate ? HeroUltimateIconBrush : HeroPassiveIconBrush;

			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.f, 0.f, 8.f, 0.f)
				[
					SNew(STextBlock)
					.Text(Label)
					.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 2))
					.ColorAndOpacity(FT66Style::Tokens::TextMuted)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
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
								.WidthOverride(56.f)
								.HeightOverride(56.f)
								[
									SNew(SOverlay)
									+ SOverlay::Slot()
									[
										SNew(SImage)
										.Image_Lambda([IconBrush]() -> const FSlateBrush*
										{
											return IconBrush.IsValid() ? IconBrush.Get() : nullptr;
										})
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

		auto MakeAbilityCard = [MakeAbilityItem](const FText& Label, const bool bUltimate) -> TSharedRef<SWidget>
		{
			return MakeHeroSelectionRowShell(
				SNew(SBox)
				.MinDesiredHeight(72.f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					MakeAbilityItem(Label, bUltimate)
				],
				FMargin(10.f, 8.f));
		};

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f, 0.f, 8.f, 0.f)
			[
				MakeAbilityCard(NSLOCTEXT("T66.HeroSelection", "UltimateShortLabel", "ULT"), true)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				MakeAbilityCard(NSLOCTEXT("T66.HeroSelection", "PassiveShortLabel", "PASSIVE"), false)
			];
	};

	TSharedPtr<SImage> HeroPreviewVideoImageWidget;
	TSharedPtr<STextBlock> HeroPreviewPlaceholderTextWidget;
	TSharedPtr<SScaleBox> CompanionInfoPortraitScaleBoxWidget;
	TSharedPtr<STextBlock> CompanionPreviewPlaceholderTextWidget;

	auto MakeRecordInfoButton = [SecondaryButtonFontSize](const FText& Title, const FText& Body) -> TSharedRef<SWidget>
	{
		return SNew(SBox)
			.ToolTip(MakeHeroSelectionAbilityTooltip(Title, Body, -1))
			[
				MakeHeroSelectionButton(
					FT66ButtonParams(
						NSLOCTEXT("T66.HeroSelection", "HeroRecordInlineInfoButton", "?"),
						FOnClicked::CreateLambda([]() -> FReply
						{
							return FReply::Handled();
						}),
						ET66ButtonType::Neutral)
					.SetMinWidth(22.f)
					.SetHeight(18.f)
					.SetFontSize(FMath::Max(SecondaryButtonFontSize - 8, 10))
					.SetPadding(FMargin(3.f, 0.f)))
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
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(InfoTargetDropdownText, STextBlock)
						.Text(CurrentInfoTargetOption.IsValid()
							? FText::FromString(*CurrentInfoTargetOption)
							: NSLOCTEXT("T66.HeroSelection", "InfoTargetHeroFallback", "Hero"))
						.Font(FT66Style::Tokens::FontBold(ScreenHeaderFontSize + 7))
						.ColorAndOpacity(FT66Style::Tokens::Text)
						.Justification(ETextJustify::Left)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						MakeHeroSelectionButton(
							FT66ButtonParams(
								NSLOCTEXT("T66.HeroSelection", "TempSettingsButton", "SETTINGS"),
								FOnClicked::CreateLambda([this]() -> FReply
								{
									if (UIManager)
									{
										UIManager->ShowScreen(ET66ScreenType::Settings);
									}
									return FReply::Handled();
								}),
								ET66ButtonType::Neutral)
							.SetMinWidth(136.f)
							.SetHeight(32.f)
							.SetFontSize(SecondaryButtonFontSize - 1)
							.SetPadding(FMargin(10.f, 5.f)))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 3.f, 0.f, 0.f)
				[
					SNew(SBox)
					.HeightOverride(2.f)
					[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(FLinearColor(0.86f, 0.72f, 0.18f, 1.0f))
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			MakeHeroSelectionRowShell(
				SNew(SBox)
				.HeightOverride(226.0f)
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
			MakeHeroSelectionRowShell(
				SNew(SBox)
				.MinDesiredHeight(46.f)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.f, 0.f, 5.f, 0.f)
							[
								MakeRecordInfoButton(
									NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalTooltipTitle", "Medal"),
									NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalTooltipBody", "Highest difficulty cleared with this hero or companion. Bronze: Easy. Silver: Medium. Gold: Hard. Platinum: Very Hard. Diamond: Impossible."))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordMedalLabel", "Medal"))
								.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 1))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(6.f, 0.f, 0.f, 0.f)
							[
								SNew(SBox)
								.WidthOverride(30.f)
								.HeightOverride(30.f)
								[
									SAssignNew(HeroRecordMedalImageWidget, SImage)
									.Image_Lambda([this]() -> const FSlateBrush*
									{
										return HeroRecordMedalBrush.IsValid() ? HeroRecordMedalBrush.Get() : nullptr;
									})
								]
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(0.f, 0.f, 5.f, 0.f)
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
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankLabel", "Rank"))
								.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 1))
								.ColorAndOpacity(FT66Style::Tokens::TextMuted)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(6.f, 0.f, 0.f, 0.f)
							[
								SAssignNew(HeroRecordRankWidget, STextBlock)
								.Text(NSLOCTEXT("T66.HeroSelection", "HeroRecordRankDefault", "..."))
								.Font(FT66Style::Tokens::FontBold(SecondaryButtonFontSize + 1))
								.ColorAndOpacity(FT66Style::Tokens::Text)
							]
						]
				],
				FMargin(8.f, 5.f))
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
				.AutoHeight()
				.Padding(0.0f, 2.0f, 0.0f, 0.0f)
				[
					FT66Style::MakeBareButton(
						FT66BareButtonParams(
							FOnClicked::CreateLambda([this]() -> FReply
							{
								return bShowingHeroRecordInfoPanel ? FReply::Handled() : HandleOpenStatsPanelClicked();
							}),
							MakeHeroSelectionRowShell(
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
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
									.Visibility(EVisibility::Collapsed)
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
									.Visibility(EVisibility::Collapsed)
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
								FMargin(6.f, 5.f)))
						.SetButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
						.SetColor(FLinearColor::Transparent)
						.SetPadding(FMargin(0.f)))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 4.f, 0.f, 0.f)
				[
					MakeHeroAbilityPanel()
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
		FMargin(FT66Style::Tokens::Space4),
		true);

	TSharedRef<SWidget> LeftFooterPanel =
		SNew(SBox)
		.WidthOverride(LeftPanelWidth)
		[
			MakePartyBox()
		];

	TSharedRef<SWidget> RightFooterPanel =
		SNew(SBox)
		.WidthOverride(RightPanelWidth)
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
					.WidthOverride(LeftPanelWidth)
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
					.WidthOverride(RightPanelWidth)
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

	if (bShowingTemporaryBuffPicker)
	{
		return SNew(SOverlay)
			+ SOverlay::Slot()
			[
				Root
			]
			+ SOverlay::Slot()
			[
				MakeTemporaryBuffPickerModal()
			];
	}

	return Root;
}

