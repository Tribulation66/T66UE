// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66MiniFrontendStateSubsystem.h"

#include "Core/T66GameInstance.h"
#include "Core/T66MiniDataSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Save/T66MiniRunSaveGame.h"

void UT66MiniFrontendStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetRunSetup();
}

void UT66MiniFrontendStateSubsystem::ResetRunSetup()
{
	SelectedHeroID = NAME_None;
	SelectedCompanionID = NAME_None;
	SelectedDifficultyID = NAME_None;
	SelectedIdolIDs.Reset();
	CurrentIdolOfferIDs.Reset();
	CurrentShopOfferIDs.Reset();
	LockedShopOfferIDs.Reset();
	PendingSaveSlot = INDEX_NONE;
	IdolRerollCount = 0;
	ShopRerollCount = 0;
	bLoadFlow = false;
	bIntermissionFlow = false;
	bSkipNextShopPrime = false;
	SyncSharedLobbyState(true);
}

void UT66MiniFrontendStateSubsystem::BeginNewRun()
{
	ResetRunSetup();
	ClearLastRunSummary();
}

void UT66MiniFrontendStateSubsystem::SeedFromRunSave(const UT66MiniRunSaveGame* RunSave)
{
	ResetRunSetup();
	if (!RunSave)
	{
		return;
	}

	SelectedHeroID = RunSave->HeroID;
	SelectedCompanionID = RunSave->CompanionID;
	SelectedDifficultyID = RunSave->DifficultyID;
	SelectedIdolIDs = RunSave->EquippedIdolIDs;
	CurrentShopOfferIDs = RunSave->CurrentShopOfferIDs;
	LockedShopOfferIDs = RunSave->LockedShopOfferIDs;
	PendingSaveSlot = RunSave->SaveSlotIndex;
	ShopRerollCount = FMath::Max(0, RunSave->ShopRerollCount);
	bLoadFlow = true;
	bIntermissionFlow = RunSave->bPendingShopIntermission;
	SyncSharedLobbyState(true);
}

void UT66MiniFrontendStateSubsystem::SelectHero(const FName HeroID)
{
	SelectedHeroID = HeroID;
	SyncSharedLobbyState(true);
}

void UT66MiniFrontendStateSubsystem::SelectCompanion(const FName CompanionID)
{
	SelectedCompanionID = CompanionID;
	SyncSharedLobbyState(true);
}

void UT66MiniFrontendStateSubsystem::SelectDifficulty(const FName DifficultyID)
{
	SelectedDifficultyID = DifficultyID;
	SyncSharedLobbyState(true);
}

void UT66MiniFrontendStateSubsystem::SetPendingSaveSlot(const int32 SlotIndex)
{
	PendingSaveSlot = SlotIndex;
}

void UT66MiniFrontendStateSubsystem::PrimeIdolOffers(const UT66MiniDataSubsystem* DataSubsystem)
{
	if (CurrentIdolOfferIDs.Num() == 0)
	{
		GenerateIdolOffers(DataSubsystem, false);
	}
}

void UT66MiniFrontendStateSubsystem::RefreshIdolOffers(const UT66MiniDataSubsystem* DataSubsystem)
{
	CurrentIdolOfferIDs.Reset();
	GenerateIdolOffers(DataSubsystem, false);
}

void UT66MiniFrontendStateSubsystem::RerollIdolOffers(const UT66MiniDataSubsystem* DataSubsystem)
{
	GenerateIdolOffers(DataSubsystem, true);
}

bool UT66MiniFrontendStateSubsystem::AddIdolToLoadout(const FName IdolID)
{
	if (IdolID == NAME_None || SelectedIdolIDs.Contains(IdolID) || SelectedIdolIDs.Num() >= MaxIdolSlots)
	{
		return false;
	}

	SelectedIdolIDs.Add(IdolID);
	CurrentIdolOfferIDs.Remove(IdolID);
	SyncSharedLobbyState(true);
	return true;
}

void UT66MiniFrontendStateSubsystem::EnterIntermissionFlow(const UT66MiniDataSubsystem* DataSubsystem)
{
	bIntermissionFlow = true;
	bSkipNextShopPrime = false;
	PrimeShopOffers(DataSubsystem);
	SyncSharedLobbyState(false);
}

void UT66MiniFrontendStateSubsystem::ResumeIntermissionFlow()
{
	bIntermissionFlow = true;
	bSkipNextShopPrime = true;
	SyncSharedLobbyState(false);
}

void UT66MiniFrontendStateSubsystem::ExitIntermissionFlow()
{
	bIntermissionFlow = false;
	ClearTransientShopState();
	SyncSharedLobbyState(false);
}

void UT66MiniFrontendStateSubsystem::PrimeShopOffers(const UT66MiniDataSubsystem* DataSubsystem)
{
	if (CurrentShopOfferIDs.Num() == 0)
	{
		GenerateShopOffers(DataSubsystem, false);
	}
}

void UT66MiniFrontendStateSubsystem::RefillShopOffers(const UT66MiniDataSubsystem* DataSubsystem)
{
	GenerateShopOffers(DataSubsystem, false);
}

void UT66MiniFrontendStateSubsystem::RerollShopOffers(const UT66MiniDataSubsystem* DataSubsystem)
{
	GenerateShopOffers(DataSubsystem, true);
}

void UT66MiniFrontendStateSubsystem::ToggleShopOfferLock(const FName ItemID)
{
	if (ItemID == NAME_None || !CurrentShopOfferIDs.Contains(ItemID))
	{
		return;
	}

	if (LockedShopOfferIDs.Contains(ItemID))
	{
		LockedShopOfferIDs.Remove(ItemID);
	}
	else
	{
		LockedShopOfferIDs.Add(ItemID);
	}
}

void UT66MiniFrontendStateSubsystem::ConsumeShopOffer(const FName ItemID)
{
	CurrentShopOfferIDs.Remove(ItemID);
	LockedShopOfferIDs.Remove(ItemID);
}

void UT66MiniFrontendStateSubsystem::ClearTransientShopState()
{
	CurrentShopOfferIDs.Reset();
	LockedShopOfferIDs.Reset();
	ShopRerollCount = 0;
}

bool UT66MiniFrontendStateSubsystem::ConsumeShouldSkipNextShopPrime()
{
	const bool bShouldSkip = bSkipNextShopPrime;
	bSkipNextShopPrime = false;
	return bShouldSkip;
}

void UT66MiniFrontendStateSubsystem::WriteIntermissionStateToRunSave(UT66MiniRunSaveGame* RunSave) const
{
	if (!RunSave)
	{
		return;
	}

	RunSave->CurrentShopOfferIDs = CurrentShopOfferIDs;
	RunSave->LockedShopOfferIDs = LockedShopOfferIDs;
	RunSave->ShopRerollCount = ShopRerollCount;
	RunSave->bPendingShopIntermission = bIntermissionFlow;
}

void UT66MiniFrontendStateSubsystem::SyncSharedLobbyState(const bool bResetLobbyReady)
{
	if (UT66GameInstance* GameInstance = Cast<UT66GameInstance>(GetGameInstance()))
	{
		GameInstance->MiniSelectedHeroID = SelectedHeroID;
		GameInstance->MiniSelectedCompanionID = SelectedCompanionID;
		GameInstance->MiniSelectedDifficultyID = SelectedDifficultyID;
		GameInstance->MiniSelectedIdolIDs = SelectedIdolIDs;
		GameInstance->bMiniLoadFlow = bLoadFlow;
		GameInstance->bMiniIntermissionFlow = bIntermissionFlow;
		if (!bIntermissionFlow)
		{
			GameInstance->MiniIntermissionStateRevision = 0;
			GameInstance->MiniIntermissionStateJson.Reset();
			GameInstance->MiniIntermissionRequestRevision = 0;
			GameInstance->MiniIntermissionRequestJson.Reset();
		}

		if (UT66SessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UT66SessionSubsystem>())
		{
			if (bResetLobbyReady)
			{
				SessionSubsystem->SetLocalLobbyReady(false);
			}
			else
			{
				SessionSubsystem->SyncLocalLobbyProfile();
			}
		}
	}
}

void UT66MiniFrontendStateSubsystem::GenerateIdolOffers(const UT66MiniDataSubsystem* DataSubsystem, const bool bCountAsReroll)
{
	CurrentIdolOfferIDs.Reset();
	if (!DataSubsystem)
	{
		return;
	}

	const TArray<FT66MiniIdolDefinition>& Idols = DataSubsystem->GetIdols();
	if (Idols.Num() == 0)
	{
		return;
	}

	if (bCountAsReroll)
	{
		++IdolRerollCount;
	}

	TArray<int32> CandidateIndices;
	CandidateIndices.Reserve(Idols.Num());
	for (int32 Index = 0; Index < Idols.Num(); ++Index)
	{
		if (!SelectedIdolIDs.Contains(Idols[Index].IdolID))
		{
			CandidateIndices.Add(Index);
		}
	}

	if (CandidateIndices.Num() == 0)
	{
		return;
	}

	FRandomStream Stream(static_cast<int32>(FDateTime::UtcNow().GetTicks() ^ IdolRerollCount ^ CandidateIndices.Num()));
	for (int32 Index = CandidateIndices.Num() - 1; Index > 0; --Index)
	{
		CandidateIndices.Swap(Index, Stream.RandRange(0, Index));
	}

	const int32 OfferCount = FMath::Min(4, CandidateIndices.Num());
	for (int32 Index = 0; Index < OfferCount; ++Index)
	{
		CurrentIdolOfferIDs.Add(Idols[CandidateIndices[Index]].IdolID);
	}
}

void UT66MiniFrontendStateSubsystem::GenerateShopOffers(const UT66MiniDataSubsystem* DataSubsystem, const bool bCountAsReroll)
{
	TArray<FName> PreservedLockedOffers;
	PreservedLockedOffers.Reserve(LockedShopOfferIDs.Num());
	for (const FName LockedID : LockedShopOfferIDs)
	{
		if (CurrentShopOfferIDs.Contains(LockedID))
		{
			PreservedLockedOffers.Add(LockedID);
		}
	}

	CurrentShopOfferIDs = PreservedLockedOffers;

	if (!DataSubsystem)
	{
		return;
	}

	const TArray<FT66MiniItemDefinition>& Items = DataSubsystem->GetItems();
	if (Items.Num() == 0)
	{
		return;
	}

	if (bCountAsReroll)
	{
		++ShopRerollCount;
	}

	TArray<int32> CandidateIndices;
	CandidateIndices.Reserve(Items.Num());
	for (int32 Index = 0; Index < Items.Num(); ++Index)
	{
		if (!CurrentShopOfferIDs.Contains(Items[Index].ItemID))
		{
			CandidateIndices.Add(Index);
		}
	}

	FRandomStream Stream(static_cast<int32>(FDateTime::UtcNow().GetTicks() ^ ShopRerollCount ^ CandidateIndices.Num()));
	for (int32 Index = CandidateIndices.Num() - 1; Index > 0; --Index)
	{
		CandidateIndices.Swap(Index, Stream.RandRange(0, Index));
	}

	while (CurrentShopOfferIDs.Num() < 4 && CandidateIndices.Num() > 0)
	{
		const int32 PickedIndex = CandidateIndices.Pop(EAllowShrinking::No);
		CurrentShopOfferIDs.Add(Items[PickedIndex].ItemID);
	}
}
