// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66HeroSelectionScreen.h"
#include "UI/Screens/HeroSelection/T66HeroSelectionScreen_Private.h"
#include "UI/Screens/T66ChallengesScreen.h"
#include "UI/T66UIManager.h"
#include "Core/T66LeaderboardSubsystem.h"

using namespace T66HeroSelectionPrivate;

DEFINE_LOG_CATEGORY(LogT66HeroSelection);

namespace
{
	ET66RunMode T66ResolveHeroSelectionRunMode(UT66GameInstance* GI)
	{
		if (!GI)
		{
			return ET66RunMode::Offline;
		}

		const UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>();
		const UT66LeaderboardSubsystem* Leaderboard = GI->GetSubsystem<UT66LeaderboardSubsystem>();
		const bool bBackendReady = Backend && Backend->IsBackendConfigured() && Backend->HasSteamTicket();
		const bool bAccountEligible = !Leaderboard || Leaderboard->IsAccountEligibleForLeaderboard();
		return (bBackendReady && bAccountEligible) ? ET66RunMode::Regular : ET66RunMode::Offline;
	}
}

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

FReply UT66HeroSelectionScreen::HandleHeroCarouselPortraitClicked(const int32 VisibleSlotIndex)
{
	if (AllHeroIDs.Num() == 0)
	{
		return FReply::Handled();
	}

	const int32 OffsetFromCenter = VisibleSlotIndex - HeroSelectionHeroCarouselCenterIndex;
	const int32 TargetIndex = (CurrentHeroIndex + OffsetFromCenter + AllHeroIDs.Num()) % AllHeroIDs.Num();
	PreviewHero(AllHeroIDs[TargetIndex]);
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleFlatSkinRowClicked(const FName SkinID)
{
	const FName ResolvedSkinID = SkinID.IsNone() ? FName(TEXT("Default")) : SkinID;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroSkinID = ResolvedSkinID;
		if (UT66SkinSubsystem* SkinSubsystem = GI->GetSubsystem<UT66SkinSubsystem>())
		{
			const bool bCanEquip = ResolvedSkinID == FName(TEXT("Default")) || SkinSubsystem->IsHeroSkinOwned(PreviewedHeroID, ResolvedSkinID);
			if (bCanEquip)
			{
				SkinSubsystem->SetEquippedHeroSkinID(PreviewedHeroID, ResolvedSkinID);
			}
		}
	}

	CommitLocalSelectionsToLobby(true);
	UpdateHeroDisplay();
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleHeroGridClicked() { OnHeroGridClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleCompanionGridClicked() { OnCompanionGridClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleCompanionClicked() { OnChooseCompanionClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleTemporaryBuffSlotClicked(int32 SlotIndex)
{
	TemporaryBuffPickerSlotIndex = FMath::Clamp(SlotIndex, 0, UT66BuffSubsystem::MaxSelectedSingleUseBuffs - 1);
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UT66BuffSubsystem* Buffs = GI->GetSubsystem<UT66BuffSubsystem>())
		{
			Buffs->BeginHeroSelectionSingleUseBuffEdit(TemporaryBuffPickerSlotIndex);
		}
	}

	bShowingTemporaryBuffPicker = false;
	NavigateTo(ET66ScreenType::PowerUp);
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

FReply UT66HeroSelectionScreen::HandleLabClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		PreviewedHeroID = GI->ResolvePlayableHeroID(PreviewedHeroID);
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SelectedDifficulty);
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->ApplyConfiguredMainMapLayoutVariant();
		GI->ClearActiveDailyClimbRun();
		GI->SelectedRunMode = T66ResolveHeroSelectionRunMode(GI);
		GI->SelectedRunCategory = ET66RunCategory::Lab;
		GI->bRunIneligibleForLeaderboard = true;
		GI->bIsNewGameFlow = true;
		GI->bIsStageTransition = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		GI->RunSeed = FMath::Rand();

		if (UIManager)
		{
			UIManager->HideAllUI();
		}
		GI->TransitionToGameplayLevel();
	}
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleTutorialClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		PreviewedHeroID = GI->ResolvePlayableHeroID(PreviewedHeroID);
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SelectedDifficulty);
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->ApplyConfiguredMainMapLayoutVariant();
		GI->ClearActiveDailyClimbRun();
		GI->SelectedRunMode = T66ResolveHeroSelectionRunMode(GI);
		GI->SelectedRunCategory = ET66RunCategory::Tutorial;
		GI->bRunIneligibleForLeaderboard = true;
		GI->bIsNewGameFlow = true;
		GI->bIsStageTransition = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		GI->RunSeed = FMath::Rand();

		if (UIManager)
		{
			UIManager->HideAllUI();
		}
		GI->TransitionToGameplayLevel();
	}
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleTestClicked()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		PreviewedHeroID = GI->ResolvePlayableHeroID(PreviewedHeroID);
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SelectedDifficulty);
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->ApplyConfiguredMainMapLayoutVariant();
		GI->ClearActiveDailyClimbRun();
		GI->SelectedRunMode = T66ResolveHeroSelectionRunMode(GI);
		GI->SelectedRunCategory = ET66RunCategory::TestRoom;
		GI->bRunIneligibleForLeaderboard = true;
		GI->bIsNewGameFlow = true;
		GI->bIsStageTransition = false;
		GI->PendingLoadedTransform = FTransform();
		GI->bApplyLoadedTransform = false;
		GI->RunSeed = FMath::Rand();

		if (UIManager)
		{
			UIManager->HideAllUI();
		}
		GI->TransitionToGameplayLevel();
	}
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

FReply UT66HeroSelectionScreen::HandleModsClicked() { OnModsClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleRetroFXSettingsClicked()
{
	if (bShowingInlineRetroFXPanel)
	{
		CommitPendingInlineRetroFXOnClose();
		bShowingInlineRetroFXPanel = false;
		RefreshPanelSwitchers();
		return FReply::Handled();
	}

	bShowingInlineRetroFXPanel = true;
	if (bShowingInlineRetroFXPanel)
	{
		bShowingStatsPanel = false;
		bShowingTemporaryBuffPicker = false;
		bInlineRetroFXInitialized = false;
		InitializeInlineRetroFXFromUserSettingsIfNeeded();
	}

	RefreshPanelSwitchers();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleBackClicked() { OnBackClicked(); return FReply::Handled(); }

FReply UT66HeroSelectionScreen::HandleBackToPartyClicked()
{
	OnBackClicked();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleChadBodyClicked()
{
	SelectedBodyType = T66BodyTypeAliases::Chad;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroBodyType = SelectedBodyType;
	}
	CommitLocalSelectionsToLobby(true);
	UpdateHeroDisplay(); // Update 3D preview immediately for this hero
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

FReply UT66HeroSelectionScreen::HandleStacyBodyClicked()
{
	SelectedBodyType = T66BodyTypeAliases::Stacy;
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SelectedHeroBodyType = SelectedBodyType;
	}
	CommitLocalSelectionsToLobby(true);
	UpdateHeroDisplay(); // Update 3D preview immediately for this hero
	RequestDeferredSlateRebuild();
	return FReply::Handled();
}

void UT66HeroSelectionScreen::RefreshPanelSwitchers()
{
	if (LeftPanelWidgetSwitcher.IsValid())
	{
		LeftPanelWidgetSwitcher->SetActiveWidgetIndex(GetLeftPanelWidgetIndex());
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

int32 UT66HeroSelectionScreen::GetLeftPanelWidgetIndex() const
{
	if (bShowingInlineRetroFXPanel)
	{
		return 2;
	}

	return bShowingStatsPanel ? 1 : 0;
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
				? FText::FromString(CurrentInfoTargetOption->ToUpper())
				: NSLOCTEXT("T66.HeroSelection", "InfoTargetHeroFallback", "HERO"));
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

FText UT66HeroSelectionScreen::GetPreviewedHeroTitleText() const
{
	FHeroData HeroData;
	FText DisplayName = NSLOCTEXT("T66.HeroSelection", "FlatHeroGeorge", "GEORGE");
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI && !PreviewedHeroID.IsNone() && GI->GetHeroData(PreviewedHeroID, HeroData))
	{
		if (UT66LocalizationSubsystem* Loc = GetLocSubsystem())
		{
			DisplayName = Loc->GetHeroDisplayName(HeroData);
		}
		else
		{
			DisplayName = HeroData.DisplayName;
		}
	}

	FString Value = DisplayName.ToString();
	Value.ToUpperInline();
	if (Value.IsEmpty() || Value.Contains(TEXT("HERO_")) || Value.Equals(TEXT("HERO")) || Value.Equals(TEXT("HERO 1")))
	{
		Value = TEXT("GEORGE");
	}
	return FText::FromString(Value);
}

FText UT66HeroSelectionScreen::GetPreviewedHeroSubtitleText() const
{
	FHeroData HeroData;
	FText Description = NSLOCTEXT("T66.HeroSelection", "FlatGeorgeSubtitle", "A FOUNDING SHOT. A CLEAN LINE.");
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI && !PreviewedHeroID.IsNone() && GI->GetHeroData(PreviewedHeroID, HeroData) && !HeroData.Description.IsEmpty())
	{
		Description = HeroData.Description;
	}

	FString Value = Description.ToString();
	Value.ToUpperInline();
	return Value.IsEmpty()
		? NSLOCTEXT("T66.HeroSelection", "FlatHeroSubtitleFallback", "READY FOR TRIBULATION.")
		: FText::FromString(Value);
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
	if (UT66GameInstance* GateGI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		HeroID = GateGI->ResolvePlayableHeroID(HeroID);
		if (HeroID.IsNone())
		{
			return;
		}
	}

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
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		SelectedDifficulty = GI->ResolvePlayableDifficulty(Difficulty);
	}
	else
	{
		SelectedDifficulty = Difficulty;
	}
	RefreshHeroRecordRank();
}

void UT66HeroSelectionScreen::ToggleBodyType() { SelectedBodyType = T66BodyTypeAliases::IsChad(SelectedBodyType) ? T66BodyTypeAliases::Stacy : T66BodyTypeAliases::Chad; }

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

void UT66HeroSelectionScreen::OpenCommunityContent(const bool bOpenMods)
{
	const ET66CommunityContentKind ContentKind = bOpenMods
		? ET66CommunityContentKind::Mod
		: ET66CommunityContentKind::Challenge;

	ShowModal(ET66ScreenType::Challenges);

	UT66ChallengesScreen* ChallengesScreen = UIManager
		? Cast<UT66ChallengesScreen>(UIManager->GetCurrentScreen())
		: nullptr;
	if (!ChallengesScreen && UIManager)
	{
		ChallengesScreen = Cast<UT66ChallengesScreen>(UIManager->GetCurrentModal());
	}

	if (ChallengesScreen)
	{
		ChallengesScreen->OpenContentKind(ContentKind);
	}
}

void UT66HeroSelectionScreen::OnChallengesClicked() { OpenCommunityContent(false); }

void UT66HeroSelectionScreen::OnModsClicked() { OpenCommunityContent(true); }

void UT66HeroSelectionScreen::OnEnterTribulationClicked()
{
	UT66GameInstance* GI = Cast<UT66GameInstance>(UGameplayStatics::GetGameInstance(this));
	UT66SessionSubsystem* SessionSubsystem = GI ? GI->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	if (GI)
	{
		PreviewedHeroID = GI->ResolvePlayableHeroID(PreviewedHeroID);
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SelectedDifficulty);
		GI->SelectedHeroID = PreviewedHeroID;
		GI->SelectedDifficulty = SelectedDifficulty;
		GI->SelectedHeroBodyType = SelectedBodyType;
		GI->ApplyConfiguredMainMapLayoutVariant();
		GI->ClearActiveDailyClimbRun();
		GI->SelectedRunMode = T66ResolveHeroSelectionRunMode(GI);
		GI->SelectedRunCategory = ET66RunCategory::Tower;
		GI->bRunIneligibleForLeaderboard = GI->IsOfflineRun();
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
		SelectedDifficulty = GI->ResolvePlayableDifficulty(SessionSubsystem->GetSharedLobbyDifficulty());
		GI->SelectedDifficulty = SelectedDifficulty;
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
	CommitPendingInlineRetroFXOnClose();

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

bool UT66HeroSelectionScreen::HandleBackAction()
{
	OnBackClicked();
	return true;
}

void UT66HeroSelectionScreen::NativeDestruct()
{
	CommitPendingInlineRetroFXOnClose();
	Super::NativeDestruct();
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
	// Key B switches to Stacy (return to Chad via the Chad button only).
	if (InKeyEvent.GetKey() == EKeys::B)
	{
		SelectedBodyType = T66BodyTypeAliases::Stacy;
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

