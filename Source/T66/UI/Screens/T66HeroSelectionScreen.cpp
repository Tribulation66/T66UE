// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"

using namespace T66HeroSelectionPrivate;

DEFINE_LOG_CATEGORY(LogT66HeroSelection);

UT66HeroSelectionScreen::UT66HeroSelectionScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::HeroSelection;
	bIsModal = false;
}

FReply UT66HeroSelectionScreen::HandlePrevClicked()
{
	PreviewPreviousHero();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleNextClicked()
{
	PreviewNextHero();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleCompanionPrevClicked()
{
	PreviewPreviousCompanion();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleCompanionNextClicked()
{
	PreviewNextCompanion();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleHeroGridClicked() { OnHeroGridClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleCompanionClicked() { OnChooseCompanionClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked(int32 SlotIndex)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (!HasUnlockedHeroSelectionDrugs(GI->GetSubsystem<UT66AchievementsSubsystem>()))
		{
			bShowingTemporaryBuffPicker = false;
			return FReply::Handled();
		}
	}

	TemporaryBuffPickerSlotIndex = FMath::Clamp(SlotIndex, 0, UT66BuffSubsystem::MaxSelectedSingleUseBuffs - 1);
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			Buffs->SetSelectedSingleUseBuffEditSlotIndex(TemporaryBuffPickerSlotIndex);
		}
	}

	bShowingTemporaryBuffPicker = true;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleTemporaryBuffPickerCloseClicked()
{
	bShowingTemporaryBuffPicker = false;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleTemporaryBuffBuyClicked(ET66SecondaryStatType StatType)
{
	if (!T66IsLiveSecondaryStatType(StatType))
	{
		return FReply::Handled();
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (!HasUnlockedHeroSelectionDrugs(GI->GetSubsystem<UT66AchievementsSubsystem>()))
		{
			bShowingTemporaryBuffPicker = false;
			return FReply::Handled();
		}

		if (UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			if (Buffs->PurchaseSingleUseBuff(StatType))
			{
				ForceRebuildSlate();
			}
		}
	}

	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleTemporaryBuffEquipClicked(ET66SecondaryStatType StatType)
{
	if (!T66IsLiveSecondaryStatType(StatType))
	{
		return FReply::Handled();
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (!HasUnlockedHeroSelectionDrugs(GI->GetSubsystem<UT66AchievementsSubsystem>()))
		{
			bShowingTemporaryBuffPicker = false;
			return FReply::Handled();
		}

		if (UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			const int32 SlotIndex = FMath::Clamp(TemporaryBuffPickerSlotIndex, 0, UT66BuffSubsystem::MaxSelectedSingleUseBuffs - 1);
			const TArray<ET66SecondaryStatType> Slots = Buffs->GetSelectedSingleUseBuffSlots();
			const ET66SecondaryStatType FocusedSlotStat = Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : ET66SecondaryStatType::None;
			const int32 OwnedCount = Buffs->GetOwnedSingleUseBuffCount(StatType);
			const int32 AssignedCount = Buffs->GetSelectedSingleUseBuffSlotAssignedCountForStat(StatType);
			const int32 AssignedOutsideFocused = AssignedCount - (FocusedSlotStat == StatType ? 1 : 0);
			if (OwnedCount > AssignedOutsideFocused && Buffs->SetSelectedSingleUseBuffSlot(SlotIndex, StatType))
			{
				bShowingTemporaryBuffPicker = false;
				ForceRebuildSlate();
			}
		}
	}

	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleClearTemporaryBuffsClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (!HasUnlockedHeroSelectionDrugs(GI->GetSubsystem<UT66AchievementsSubsystem>()))
		{
			bShowingTemporaryBuffPicker = false;
			return FReply::Handled();
		}

		if (UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			for (int32 SlotIndex = 0; SlotIndex < UT66BuffSubsystem::MaxSelectedSingleUseBuffs; ++SlotIndex)
			{
				Buffs->ClearSelectedSingleUseBuffSlot(SlotIndex);
			}
		}
	}

	bShowingTemporaryBuffPicker = false;
	ForceRebuildSlate();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleLoreClicked()
{
	bShowingLore = !bShowingLore;
	UpdateHeroDisplay();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleEnterClicked() { OnEnterTribulationClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleChallengesClicked() { OnChallengesClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleBackToPartyClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleBodyTypeAClicked()
{
	SelectedBodyType = ET66BodyType::TypeA;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroBodyType = SelectedBodyType;
	}
	CommitLocalSelectionsToLobby(true);
	UpdateHeroDisplay(); // Update 3D preview immediately for this hero
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleBodyTypeBClicked()
{
	SelectedBodyType = ET66BodyType::TypeB;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroBodyType = SelectedBodyType;
	}
	CommitLocalSelectionsToLobby(true);
	UpdateHeroDisplay(); // Update 3D preview immediately for this hero
	return FReply::Handled();
}

void UT66HeroSelectionScreen::RefreshPanelSwitchers()
{
	if (LeftPanelWidgetSwitcher.IsValid())
	{
		LeftPanelWidgetSwitcher->SetActiveWidgetIndex(bShowingStatsPanel ? 1 : 0);
	}

	const int32 CompanionInfoIndex = bShowingCompanionInfo ? 1 : 0;
	if (RightInfoWidgetSwitcher.IsValid())
	{
		RightInfoWidgetSwitcher->SetActiveWidgetIndex(CompanionInfoIndex);
	}

	if (RightFooterWidgetSwitcher.IsValid())
	{
		RightFooterWidgetSwitcher->SetActiveWidgetIndex(CompanionInfoIndex);
	}
}

void UT66HeroSelectionScreen::RefreshTargetDropdownTexts()
{
	if (SkinTargetDropdownText.IsValid())
	{
		SkinTargetDropdownText->SetText(
			CurrentSkinTargetOption.IsValid()
				? FText::FromString(*CurrentSkinTargetOption)
				: NSLOCTEXT("T66.HeroSelection", "SkinTargetHeroFallback", "HERO"));
	}

	if (InfoTargetDropdownText.IsValid())
	{
		InfoTargetDropdownText->SetText(
			CurrentInfoTargetOption.IsValid()
				? FText::FromString(*CurrentInfoTargetOption)
				: NSLOCTEXT("T66.HeroSelection", "InfoTargetHeroFallback", "Hero"));
	}
}

TArray<FHeroData> UT66HeroSelectionScreen::GetAllHeroes()
{
	TArray<FHeroData> Heroes;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		for (const FName& HeroID : AllHeroIDs)
		{
			FHeroData HeroData;
			if (GI->GetHeroData(HeroID, HeroData)) Heroes.Add(HeroData);
		}
	}
	return Heroes;
}

bool UT66HeroSelectionScreen::GetPreviewedHeroData(FHeroData& OutHeroData)
{
	if (PreviewedHeroID.IsNone()) return false;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->GetHeroData(PreviewedHeroID, OutHeroData);
	}
	return false;
}

bool UT66HeroSelectionScreen::GetSelectedCompanionData(FCompanionData& OutCompanionData)
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->GetSelectedCompanionData(OutCompanionData);
	}
	return false;
}

bool UT66HeroSelectionScreen::GetPreviewedCompanionData(FCompanionData& OutCompanionData)
{
	if (PreviewedCompanionID.IsNone())
	{
		return false;
	}

	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		return GI->GetCompanionData(PreviewedCompanionID, OutCompanionData);
	}

	return false;
}

void UT66HeroSelectionScreen::PreviewHero(FName HeroID)
{
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero START: switching to HeroID=%s"), *HeroID.ToString());
	
	PreviewedHeroID = HeroID;
	CurrentHeroIndex = AllHeroIDs.IndexOfByKey(HeroID);
	if (CurrentHeroIndex == INDEX_NONE) CurrentHeroIndex = 0;
	GetOrCreatePreviewController()->ResetHeroPreviewStateForHeroSwitch();
	
	// Clear any preview override when switching heroes (preview is hero-specific).
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: cleared PreviewSkinIDOverride"));
	
	// Sync selected skin to this hero's equipped skin so 3D preview and skin list match.
	// If the previously selected skin is not owned by this hero, fall back to Default.
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SkinSubsystem* SkinSub = GI->GetSubsystem<UT66SkinSubsystem>())
		{
			// First check if the current SelectedHeroSkinID is owned by this hero.
			// If not, reset to this hero's equipped skin (or Default).
			const FName CurrentSkin = GI->SelectedHeroSkinID;
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: CurrentSkin (GI->SelectedHeroSkinID) = %s"), *CurrentSkin.ToString());
			
			const bool bIsDefault = CurrentSkin == FName(TEXT("Default"));
			const bool bIsOwned = SkinSub->IsHeroSkinOwned(HeroID, CurrentSkin);
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: bIsDefault=%d, bIsOwned=%d"), bIsDefault ? 1 : 0, bIsOwned ? 1 : 0);
			
			const bool bCurrentSkinOwned = bIsDefault || bIsOwned;
			if (!bCurrentSkinOwned)
			{
				// Current skin not owned by this hero; switch to this hero's equipped skin.
				const FName NewSkin = SkinSub->GetEquippedHeroSkinID(HeroID);
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: %s does NOT own %s, switching to equipped: %s"),
					*HeroID.ToString(), *CurrentSkin.ToString(), *NewSkin.ToString());
				GI->SelectedHeroSkinID = NewSkin;
			}
			else
			{
				UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: %s OWNS %s, keeping it"),
					*HeroID.ToString(), *CurrentSkin.ToString());
			}
			if (GI->SelectedHeroSkinID.IsNone())
			{
				GI->SelectedHeroSkinID = FName(TEXT("Default"));
			}
			UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero: final GI->SelectedHeroSkinID = %s"), *GI->SelectedHeroSkinID.ToString());
		}
		else
		{
			UE_LOG(LogT66HeroSelection, Warning, TEXT("[BEACH] PreviewHero: SkinSubsystem is NULL!"));
		}
	}
	else
	{
		UE_LOG(LogT66HeroSelection, Warning, TEXT("[BEACH] PreviewHero: GI is NULL!"));
	}
	
	FHeroData HeroData;
	if (GetPreviewedHeroData(HeroData)) OnPreviewedHeroChanged(HeroData);
	CommitLocalSelectionsToLobby(true);
	UpdateHeroDisplay();
	GetOrCreatePreviewController()->UpdateHeroPreviewVideo(PreviewedHeroID, bShowingCompanionInfo);
	UE_LOG(LogT66HeroSelection, Verbose, TEXT("[BEACH] PreviewHero END"));
}

void UT66HeroSelectionScreen::PreviewCompanion(FName CompanionID)
{
	PreviewedCompanionID = CompanionID;
	GetOrCreatePreviewController()->ResetCompanionSkinPreviewOverride();

	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);
	CurrentCompanionIndex = CompanionWheelIDs.IndexOfByKey(CompanionID);
	if (CurrentCompanionIndex == INDEX_NONE)
	{
		CurrentCompanionIndex = 0;
	}

	CommitLocalSelectionsToLobby(true);
	RefreshSkinsList();
	UpdateHeroDisplay();
}

void UT66HeroSelectionScreen::SelectNoCompanion()
{
	PreviewCompanion(NAME_None);
}

void UT66HeroSelectionScreen::PreviewNextHero()
{
	if (AllHeroIDs.Num() == 0) return;
	CurrentHeroIndex = (CurrentHeroIndex + 1) % AllHeroIDs.Num();
	PreviewHero(AllHeroIDs[CurrentHeroIndex]);
}

void UT66HeroSelectionScreen::PreviewNextCompanion()
{
	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);
	if (CompanionWheelIDs.Num() == 0)
	{
		return;
	}

	CurrentCompanionIndex = (CurrentCompanionIndex + 1 + CompanionWheelIDs.Num()) % CompanionWheelIDs.Num();
	PreviewCompanion(CompanionWheelIDs[CurrentCompanionIndex]);
}

void UT66HeroSelectionScreen::PreviewPreviousHero()
{
	if (AllHeroIDs.Num() == 0) return;
	CurrentHeroIndex = (CurrentHeroIndex - 1 + AllHeroIDs.Num()) % AllHeroIDs.Num();
	PreviewHero(AllHeroIDs[CurrentHeroIndex]);
}

void UT66HeroSelectionScreen::PreviewPreviousCompanion()
{
	TArray<FName> CompanionWheelIDs;
	CompanionWheelIDs.Add(NAME_None);
	CompanionWheelIDs.Append(AllCompanionIDs);
	if (CompanionWheelIDs.Num() == 0)
	{
		return;
	}

	CurrentCompanionIndex = (CurrentCompanionIndex - 1 + CompanionWheelIDs.Num()) % CompanionWheelIDs.Num();
	PreviewCompanion(CompanionWheelIDs[CurrentCompanionIndex]);
}

void UT66HeroSelectionScreen::SelectDifficulty(ET66Difficulty Difficulty)
{
	SelectedDifficulty = Difficulty;
	RefreshHeroRecordRank();
}

void UT66HeroSelectionScreen::ToggleBodyType() { SelectedBodyType = (SelectedBodyType == ET66BodyType::TypeA) ? ET66BodyType::TypeB : ET66BodyType::TypeA; }

void UT66HeroSelectionScreen::OnHeroGridClicked() { ShowModal(ET66ScreenType::HeroGrid); }

void UT66HeroSelectionScreen::OnCompanionGridClicked() { ShowModal(ET66ScreenType::CompanionGrid); }

void UT66HeroSelectionScreen::OnChooseCompanionClicked()
{
	if (UIManager)
	{
		UIManager->ShowScreen(ET66ScreenType::CompanionSelection);
		return;
	}

	ShowModal(ET66ScreenType::CompanionSelection);
}

void UT66HeroSelectionScreen::OnHeroLoreClicked() { ShowModal(ET66ScreenType::HeroLore); }

void UT66HeroSelectionScreen::OnChallengesClicked() { ShowModal(ET66ScreenType::Challenges); }

void UT66HeroSelectionScreen::OnEnterTribulationClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (GI)
	{
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->ApplyConfiguredMainMapLayoutVariant();
		GI->bStageCatchUpPending = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		// New seed each time so procedural terrain layout differs per run
		GI->RunSeed = FMath::Rand();
		if (UT66PartySubsystem* PartySubsystem = GI->GetSubsystem<UT66PartySubsystem>())
		{
			PartySubsystem->ApplyCurrentPartyToGameInstanceRunContext();
		}
	}

	if (SessionSubsystem && SessionSubsystem->IsPartyLobbyContextActive() && GI)
	{
		GI->SelectedDifficulty = SessionSubsystem->GetSharedLobbyDifficulty();
		SessionSubsystem->SyncLocalLobbyProfile();

		if (!SessionSubsystem->IsLocalPlayerPartyHost())
		{
			SessionSubsystem->SetLocalLobbyReady(!SessionSubsystem->IsLocalLobbyReady());
			ForceRebuildSlate();
			return;
		}

		FString FailureReason;
		if (!SessionSubsystem->AreAllPartyMembersReadyForGameplay(&FailureReason))
		{
			UE_LOG(LogT66HeroSelection, Log, TEXT("%s"), *FailureReason);
			ForceRebuildSlate();
			return;
		}

		if (UIManager) UIManager->HideAllUI();
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

void UT66HeroSelectionScreen::OnBackClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartySessionActive())
			{
				SessionSubsystem->SetLocalLobbyReady(false);
				if (SessionSubsystem->IsLocalPlayerPartyHost())
				{
					SessionSubsystem->SetLocalFrontendScreen(ET66ScreenType::MainMenu, true);
					NavigateTo(ET66ScreenType::MainMenu);
				}
				return;
			}
		}
	}

	if (UIManager && UIManager->CanGoBack())
	{
		NavigateBack();
		return;
	}

	NavigateTo(ET66ScreenType::MainMenu);
}

UT66HeroSelectionPreviewController* UT66HeroSelectionScreen::GetOrCreatePreviewController()
{
	if (!PreviewController)
	{
		PreviewController = NewObject<UT66HeroSelectionPreviewController>(this, NAME_None, RF_Transient);
	}

	if (PreviewController)
	{
		PreviewController->Initialize(this);
	}

	return PreviewController;
}

const UT66HeroSelectionPreviewController* UT66HeroSelectionScreen::GetPreviewController() const
{
	return PreviewController;
}

FReply UT66HeroSelectionScreen::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Key B switches to Type B (return to Type A via the Type A button only)
	if (InKeyEvent.GetKey() == EKeys::B)
	{
		SelectedBodyType = ET66BodyType::TypeB;
		if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			GI->SelectedHeroBodyType = SelectedBodyType;
		}
		CommitLocalSelectionsToLobby(true);
		UpdateHeroDisplay();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

