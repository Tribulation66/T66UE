// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PartySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SessionSubsystem.h"
#include "Core/T66SteamHelper.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

namespace
{
	bool T66AreFriendListsEqual(const TArray<FT66PartyFriendEntry>& A, const TArray<FT66PartyFriendEntry>& B)
	{
		if (A.Num() != B.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < A.Num(); ++Index)
		{
			if (A[Index].PlayerId != B[Index].PlayerId
				|| A[Index].DisplayName != B[Index].DisplayName
				|| A[Index].PresenceText != B[Index].PresenceText
				|| A[Index].bOnline != B[Index].bOnline)
			{
				return false;
			}
		}

		return true;
	}

	bool T66ArePartyListsEqual(const TArray<FT66PartyMemberEntry>& A, const TArray<FT66PartyMemberEntry>& B)
	{
		if (A.Num() != B.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < A.Num(); ++Index)
		{
			if (A[Index].PlayerId != B[Index].PlayerId
				|| A[Index].DisplayName != B[Index].DisplayName
				|| A[Index].bIsLocal != B[Index].bIsLocal
				|| A[Index].bOnline != B[Index].bOnline
				|| A[Index].bReady != B[Index].bReady
				|| A[Index].bIsPartyHost != B[Index].bIsPartyHost)
			{
				return false;
			}
		}

		return true;
	}

	ET66PartySize PartySizeFromCount(int32 Count)
	{
		switch (FMath::Clamp(Count, 1, 4))
		{
		case 4: return ET66PartySize::Quad;
		case 3: return ET66PartySize::Trio;
		case 2: return ET66PartySize::Duo;
		case 1:
		default:
			return ET66PartySize::Solo;
		}
	}
}

void UT66PartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RefreshPartyState(false);
	PartyTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UT66PartySubsystem::HandlePartyTicker),
		0.35f);
}

void UT66PartySubsystem::Deinitialize()
{
	if (PartyTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PartyTickerHandle);
		PartyTickerHandle.Reset();
	}

	PartyStateChanged.Clear();
	PartyMembers.Reset();
	Friends.Reset();
	Super::Deinitialize();
}

bool UT66PartySubsystem::IsFriendInParty(const FString& PlayerId) const
{
	return FindPartyMemberIndex(PlayerId) != INDEX_NONE;
}

bool UT66PartySubsystem::InviteFriend(const FString& PlayerId)
{
	RefreshPartyState(false);

	if (PlayerId.IsEmpty() || PlayerId == LocalPlayerId || PartyMembers.Num() >= GetMaxPartyMembers())
	{
		return false;
	}

	if (IsFriendInParty(PlayerId))
	{
		return false;
	}

	const FT66PartyFriendEntry* Friend = FindFriend(PlayerId);
	if (!Friend || !Friend->bOnline)
	{
		return false;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			return SessionSubsystem->SendInviteToFriend(PlayerId);
		}
	}

	return false;
}

bool UT66PartySubsystem::RemovePartyMember(const FString& PlayerId)
{
	// Remote player removal is not implemented yet; party membership is driven by Steam session joins/leaves.
	RefreshPartyState(false);
	return false;
}

void UT66PartySubsystem::ResetToLocalParty()
{
	RefreshPartyState(false);

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartySessionActive())
			{
				SessionSubsystem->LeaveFrontendLobby(ET66ScreenType::PartySizePicker);
				return;
			}
		}
	}

	if (PartyMembers.Num() == 1 && PartyMembers[0].PlayerId == LocalPlayerId)
	{
		return;
	}

	PartyMembers.Reset();
	FT66PartyMemberEntry& LocalMember = PartyMembers.AddDefaulted_GetRef();
	LocalMember.PlayerId = LocalPlayerId;
	LocalMember.DisplayName = LocalDisplayName;
	LocalMember.bIsLocal = true;
	LocalMember.bOnline = true;
	LocalMember.bReady = false;
	LocalMember.bIsPartyHost = true;

	SyncSelectedPartySize();
	ApplyCurrentPartyToGameInstanceRunContext();
	PartyStateChanged.Broadcast();
}

void UT66PartySubsystem::ApplyCurrentPartyToGameInstanceRunContext()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		GI->CurrentRunOwnerPlayerId = LocalPlayerId;
		GI->CurrentRunOwnerDisplayName = LocalDisplayName;
		GI->CurrentRunPartyMemberIds = GetCurrentPartyMemberIds();
		GI->CurrentRunPartyMemberDisplayNames = GetCurrentPartyMemberDisplayNames();
		GI->SelectedPartySize = GetCurrentPartySizeEnum();
	}
}

ET66PartySize UT66PartySubsystem::GetCurrentPartySizeEnum() const
{
	return PartySizeFromCount(FMath::Max(PartyMembers.Num(), GetMaxPartyMembers()));
}

TArray<FString> UT66PartySubsystem::GetCurrentPartyMemberIds() const
{
	TArray<FString> Result;
	Result.Reserve(PartyMembers.Num());
	for (const FT66PartyMemberEntry& Member : PartyMembers)
	{
		Result.Add(Member.PlayerId);
	}
	return Result;
}

TArray<FString> UT66PartySubsystem::GetCurrentPartyMemberDisplayNames() const
{
	TArray<FString> Result;
	Result.Reserve(PartyMembers.Num());
	for (const FT66PartyMemberEntry& Member : PartyMembers)
	{
		Result.Add(Member.DisplayName);
	}
	return Result;
}

bool UT66PartySubsystem::DoesSaveMatchCurrentParty(const UT66RunSaveGame* SaveGame) const
{
	if (!SaveGame)
	{
		return false;
	}

	const TArray<FString> CurrentPartyIds = GetCurrentPartyMemberIds();
	if (CurrentPartyIds.Num() == 0)
	{
		return true;
	}

	TArray<FString> SavePartyIds = SaveGame->PartyMemberIds;
	if (SavePartyIds.Num() == 0)
	{
		if (!SaveGame->OwnerPlayerId.IsEmpty())
		{
			SavePartyIds.Add(SaveGame->OwnerPlayerId);
		}
		else
		{
			SavePartyIds.Add(LocalPlayerId);
		}
	}

	for (const FString& SavePartyId : SavePartyIds)
	{
		if (!CurrentPartyIds.Contains(SavePartyId))
		{
			return false;
		}
	}

	return SavePartyIds.Num() > 0;
}

bool UT66PartySubsystem::HandlePartyTicker(float DeltaTime)
{
	RefreshPartyState(true);
	return true;
}

void UT66PartySubsystem::RefreshPartyState(bool bBroadcastChanges)
{
	const TArray<FT66PartyFriendEntry> PreviousFriends = Friends;
	const TArray<FT66PartyMemberEntry> PreviousPartyMembers = PartyMembers;

	LocalPlayerId = TEXT("local_player");
	LocalDisplayName = TEXT("You");

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SteamHelper* Steam = GI->GetSubsystem<UT66SteamHelper>())
		{
			if (!Steam->GetLocalSteamId().IsEmpty())
			{
				LocalPlayerId = Steam->GetLocalSteamId();
			}
			if (!Steam->GetLocalDisplayName().IsEmpty())
			{
				LocalDisplayName = Steam->GetLocalDisplayName();
			}
		}
	}

	RebuildFriendList();
	RebuildPartyMembers();
	SyncSelectedPartySize();
	ApplyCurrentPartyToGameInstanceRunContext();

	if (bBroadcastChanges
		&& (!T66AreFriendListsEqual(PreviousFriends, Friends) || !T66ArePartyListsEqual(PreviousPartyMembers, PartyMembers)))
	{
		PartyStateChanged.Broadcast();
	}
}

void UT66PartySubsystem::RebuildFriendList()
{
	Friends.Reset();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UT66SteamHelper* Steam = GI->GetSubsystem<UT66SteamHelper>())
		{
			for (const FT66SteamFriendInfo& FriendInfo : Steam->GetFriendInfos())
			{
				FT66PartyFriendEntry& Friend = Friends.AddDefaulted_GetRef();
				Friend.PlayerId = FriendInfo.SteamId;
				Friend.DisplayName = FriendInfo.DisplayName;
				Friend.PresenceText = FriendInfo.PresenceText;
				Friend.bOnline = FriendInfo.bOnline;
			}
		}
	}
}

void UT66PartySubsystem::RebuildPartyMembers()
{
	PartyMembers.Reset();

	UWorld* World = GetWorld();
	const UT66SessionSubsystem* SessionSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UT66SessionSubsystem>() : nullptr;
	const bool bShouldUseReplicatedPlayerStates = World
		&& (World->GetNetMode() != NM_Standalone || (SessionSubsystem && SessionSubsystem->IsPartySessionActive()));
	AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (GameState && bShouldUseReplicatedPlayerStates)
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			AT66SessionPlayerState* SessionPlayerState = Cast<AT66SessionPlayerState>(PlayerState);
			if (!SessionPlayerState)
			{
				continue;
			}

			const FT66LobbyPlayerInfo& LobbyInfo = SessionPlayerState->GetLobbyInfo();
			FT66PartyMemberEntry& Member = PartyMembers.AddDefaulted_GetRef();
			Member.PlayerId = LobbyInfo.SteamId.IsEmpty() ? PlayerState->GetPlayerName() : LobbyInfo.SteamId;
			Member.DisplayName = LobbyInfo.DisplayName.IsEmpty() ? PlayerState->GetPlayerName() : LobbyInfo.DisplayName;
			if (Member.DisplayName.IsEmpty())
			{
				Member.DisplayName = LocalDisplayName;
			}
			Member.bIsLocal = Member.PlayerId == LocalPlayerId || (!LocalDisplayName.IsEmpty() && Member.DisplayName == LocalDisplayName);
			Member.bOnline = true;
			Member.bReady = LobbyInfo.bLobbyReady;
			Member.bIsPartyHost = LobbyInfo.bPartyHost;
		}
	}

	if (PartyMembers.Num() == 0)
	{
		FT66PartyMemberEntry& LocalMember = PartyMembers.AddDefaulted_GetRef();
		LocalMember.PlayerId = LocalPlayerId;
		LocalMember.DisplayName = LocalDisplayName;
		LocalMember.bIsLocal = true;
		LocalMember.bOnline = true;
		LocalMember.bReady = false;
		LocalMember.bIsPartyHost = true;
	}
	else
	{
		PartyMembers.Sort([](const FT66PartyMemberEntry& A, const FT66PartyMemberEntry& B)
		{
			if (A.bIsPartyHost != B.bIsPartyHost)
			{
				return A.bIsPartyHost;
			}

			if (A.bIsLocal != B.bIsLocal)
			{
				return A.bIsLocal;
			}

			return A.DisplayName < B.DisplayName;
		});
	}
}

int32 UT66PartySubsystem::GetMaxPartyMembers() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			if (SessionSubsystem->IsPartySessionActive())
			{
				return FMath::Max(1, SessionSubsystem->GetMaxPartyMembers());
			}
		}
	}

	if (const UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		switch (GI->SelectedPartySize)
		{
		case ET66PartySize::Quad: return 4;
		case ET66PartySize::Trio: return 3;
		case ET66PartySize::Duo: return 2;
		case ET66PartySize::Solo:
		default:
			return 1;
		}
	}

	return 1;
}

void UT66PartySubsystem::SyncSelectedPartySize()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		GI->SelectedPartySize = PartySizeFromCount(GetMaxPartyMembers());
	}
}

const FT66PartyFriendEntry* UT66PartySubsystem::FindFriend(const FString& PlayerId) const
{
	return Friends.FindByPredicate([&PlayerId](const FT66PartyFriendEntry& Friend)
	{
		return Friend.PlayerId == PlayerId;
	});
}

int32 UT66PartySubsystem::FindPartyMemberIndex(const FString& PlayerId) const
{
	return PartyMembers.IndexOfByPredicate([&PlayerId](const FT66PartyMemberEntry& Member)
	{
		return Member.PlayerId == PlayerId;
	});
}
