// Copyright Tribulation 66. All Rights Reserved.

#include "Core/T66PartySubsystem.h"
#include "Core/T66GameInstance.h"
#include "Core/T66RunSaveGame.h"
#include "Core/T66SteamHelper.h"

namespace
{
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

	BuildPlaceholderState();
	SyncSelectedPartySize();
	ApplyCurrentPartyToGameInstanceRunContext();
}

void UT66PartySubsystem::Deinitialize()
{
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
	if (PlayerId.IsEmpty() || PlayerId == LocalPlayerId || PartyMembers.Num() >= MaxPartyMembers)
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

	FT66PartyMemberEntry& NewMember = PartyMembers.AddDefaulted_GetRef();
	NewMember.PlayerId = Friend->PlayerId;
	NewMember.DisplayName = Friend->DisplayName;
	NewMember.bIsLocal = false;
	NewMember.bOnline = Friend->bOnline;

	SyncSelectedPartySize();
	ApplyCurrentPartyToGameInstanceRunContext();
	PartyStateChanged.Broadcast();
	return true;
}

bool UT66PartySubsystem::RemovePartyMember(const FString& PlayerId)
{
	if (PlayerId.IsEmpty() || PlayerId == LocalPlayerId)
	{
		return false;
	}

	const int32 RemoveIndex = FindPartyMemberIndex(PlayerId);
	if (RemoveIndex == INDEX_NONE)
	{
		return false;
	}

	PartyMembers.RemoveAt(RemoveIndex);
	SyncSelectedPartySize();
	ApplyCurrentPartyToGameInstanceRunContext();
	PartyStateChanged.Broadcast();
	return true;
}

void UT66PartySubsystem::ResetToLocalParty()
{
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
	return PartySizeFromCount(PartyMembers.Num());
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

void UT66PartySubsystem::BuildPlaceholderState()
{
	LocalPlayerId = TEXT("local_player");
	LocalDisplayName = TEXT("You");

	TArray<FString> SteamFriendIds;
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
			SteamFriendIds = Steam->GetFriendSteamIds();
		}
	}

	PartyMembers.Reset();
	FT66PartyMemberEntry& LocalMember = PartyMembers.AddDefaulted_GetRef();
	LocalMember.PlayerId = LocalPlayerId;
	LocalMember.DisplayName = LocalDisplayName;
	LocalMember.bIsLocal = true;
	LocalMember.bOnline = true;

	struct FPlaceholderFriend
	{
		const TCHAR* Name;
		const TCHAR* Presence;
		bool bOnline;
	};

	static const FPlaceholderFriend PlaceholderFriends[] = {
		{ TEXT("CoMaNDeR^^"), TEXT("Online"), true },
		{ TEXT("Mid rotate or l afk"), TEXT("Ready to party"), true },
		{ TEXT("Magina da silva Slark"), TEXT("In menus"), true },
		{ TEXT("Treze."), TEXT("Offline"), false },
	};

	Friends.Reset();
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(PlaceholderFriends); ++Index)
	{
		const FPlaceholderFriend& Placeholder = PlaceholderFriends[Index];
		FT66PartyFriendEntry& Friend = Friends.AddDefaulted_GetRef();
		Friend.PlayerId = SteamFriendIds.IsValidIndex(Index)
			? SteamFriendIds[Index]
			: FString::Printf(TEXT("placeholder_friend_%d"), Index + 1);
		Friend.DisplayName = Placeholder.Name;
		Friend.PresenceText = Placeholder.Presence;
		Friend.bOnline = Placeholder.bOnline;
	}
}

void UT66PartySubsystem::SyncSelectedPartySize()
{
	if (UT66GameInstance* GI = Cast<UT66GameInstance>(GetGameInstance()))
	{
		GI->SelectedPartySize = GetCurrentPartySizeEnum();
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
