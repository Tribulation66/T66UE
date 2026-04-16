// Copyright Tribulation 66. All Rights Reserved.

#include "Gameplay/T66SessionPlayerState.h"
#include "Net/UnrealNetwork.h"

AT66SessionPlayerState::AT66SessionPlayerState()
{
	bReplicates = true;
}

void AT66SessionPlayerState::ApplyLobbyInfo(const FT66LobbyPlayerInfo& NewInfo)
{
	LobbyInfo = NewInfo;
	SetPlayerName(LobbyInfo.DisplayName.IsEmpty() ? TEXT("Player") : LobbyInfo.DisplayName);
	if (HasAuthority())
	{
		ForceNetUpdate();
	}
	BroadcastLobbyInfoChanged();
}

void AT66SessionPlayerState::OnRep_LobbyInfo()
{
	SetPlayerName(LobbyInfo.DisplayName.IsEmpty() ? TEXT("Player") : LobbyInfo.DisplayName);
	BroadcastLobbyInfoChanged();
}

void AT66SessionPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AT66SessionPlayerState* SessionPlayerState = Cast<AT66SessionPlayerState>(PlayerState))
	{
		SessionPlayerState->LobbyInfo = LobbyInfo;
		SessionPlayerState->SetPlayerName(LobbyInfo.DisplayName.IsEmpty() ? TEXT("Player") : LobbyInfo.DisplayName);
	}
}

void AT66SessionPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	if (const AT66SessionPlayerState* SessionPlayerState = Cast<AT66SessionPlayerState>(PlayerState))
	{
		LobbyInfo = SessionPlayerState->LobbyInfo;
		SetPlayerName(LobbyInfo.DisplayName.IsEmpty() ? TEXT("Player") : LobbyInfo.DisplayName);
		BroadcastLobbyInfoChanged();
	}
}

void AT66SessionPlayerState::BroadcastLobbyInfoChanged()
{
	LobbyInfoChanged.Broadcast();
}

void AT66SessionPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AT66SessionPlayerState, LobbyInfo);
}
