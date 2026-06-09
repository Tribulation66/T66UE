// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PartyInviteModal.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "UI/Screens/T66ScreenSlateHelpers.h"
#include "UI/Style/T66FlatStyle.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/STextBlock.h"

UT66PartyInviteModal::UT66PartyInviteModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PartyInvite;
	bIsModal = true;
}

DEFINE_LOG_CATEGORY_STATIC(LogT66PartyInviteModal, Log, All);

namespace
{
	FName PartyInviteTag(const TCHAR* Name)
	{
		return FName(Name);
	}

	void T66RefreshInviteJoinContext(
		const UT66BackendSubsystem* Backend,
		const FString& InviteId,
		FString& InOutHostSteamId,
		FString& InOutHostLobbyId,
		FString& InOutHostAppId)
	{
		if (!Backend || InviteId.IsEmpty())
		{
			return;
		}

		for (const FT66PartyInviteEntry& Invite : Backend->GetPendingPartyInvites())
		{
			if (Invite.InviteId != InviteId)
			{
				continue;
			}

			if (!Invite.HostSteamId.IsEmpty())
			{
				InOutHostSteamId = Invite.HostSteamId;
			}

			if (!Invite.HostLobbyId.IsEmpty())
			{
				InOutHostLobbyId = Invite.HostLobbyId;
			}

			if (!Invite.HostAppId.IsEmpty())
			{
				InOutHostAppId = Invite.HostAppId;
			}

			break;
		}
	}
}

void UT66PartyInviteModal::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();
	bJoinKickoffStarted = false;

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			PartyInviteActionCompleteHandle = Backend->OnPartyInviteActionComplete().AddUObject(this, &UT66PartyInviteModal::HandlePartyInviteActionComplete);
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Party invite modal activated and bound action-complete delegate."));
		}
	}
}

void UT66PartyInviteModal::OnScreenDeactivated_Implementation()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			Backend->OnPartyInviteActionComplete().Remove(PartyInviteActionCompleteHandle);
		}
	}
	UE_LOG(LogT66PartyInviteModal, Log, TEXT("Party invite modal deactivated. InFlight=%d InviteId=%s"), bActionInFlight ? 1 : 0, *ActionInviteId);
	PartyInviteActionCompleteHandle.Reset();
	ActionInviteId.Reset();
	ActionHostSteamId.Reset();
	ActionHostLobbyId.Reset();
	ActionHostAppId.Reset();
	bAcceptingInvite = false;
	bActionInFlight = false;
	bJoinKickoffStarted = false;
	ActionStatusText.Reset();

	Super::OnScreenDeactivated_Implementation();
}

const FT66PartyInviteEntry* UT66PartyInviteModal::GetCurrentInvite() const
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			const TArray<FT66PartyInviteEntry>& PendingInvites = Backend->GetPendingPartyInvites();
			return PendingInvites.Num() > 0 ? &PendingInvites[0] : nullptr;
		}
	}

	return nullptr;
}

TSharedRef<SWidget> UT66PartyInviteModal::BuildSlateUI()
{
	const FT66PartyInviteEntry* Invite = GetCurrentInvite();
	const FText TitleText = NSLOCTEXT("T66.PartyInvite", "Title", "PARTY INVITE");
	const FText EmptyText = NSLOCTEXT("T66.PartyInvite", "Empty", "No pending party invites.");
	const FText AcceptText = bActionInFlight
		? NSLOCTEXT("T66.PartyInvite", "Working", "WORKING...")
		: NSLOCTEXT("T66.PartyInvite", "Accept", "ACCEPT");
	const FText RejectText = bActionInFlight
		? NSLOCTEXT("T66.PartyInvite", "PleaseWait", "PLEASE WAIT")
		: NSLOCTEXT("T66.PartyInvite", "Reject", "REJECT");

	const FText InviteBodyText = Invite
		? FText::Format(
			NSLOCTEXT("T66.PartyInvite", "InviteBody", "{0} invited you to join their party."),
			FText::FromString(Invite->HostDisplayName.IsEmpty() ? Invite->HostSteamId : Invite->HostDisplayName))
		: EmptyText;

	const FText StatusText = ActionStatusText.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(ActionStatusText);
	const bool bCanActOnInvite = Invite != nullptr && !bActionInFlight;

	T66ScreenSlateHelpers::FFriendslopStandardModalParams Params;
	Params.TitleText = TitleText;
	Params.BodyText = InviteBodyText;
	Params.StatusText = StatusText;
	Params.RootTag = PartyInviteTag(TEXT("PartyInviteModal.Root"));
	Params.ScrimTag = PartyInviteTag(TEXT("PartyInviteModal.Scrim"));
	Params.PanelTag = PartyInviteTag(TEXT("PartyInviteModal.ModalPanel"));
	Params.TitleTag = PartyInviteTag(TEXT("PartyInviteModal.Title"));
	Params.BodyTag = PartyInviteTag(TEXT("PartyInviteModal.BodyRow"));
	Params.StatusTag = PartyInviteTag(TEXT("PartyInviteModal.StatusRow"));
	Params.LeftButton.Label = AcceptText;
	Params.LeftButton.OnClicked = FOnClicked::CreateUObject(this, &UT66PartyInviteModal::HandleAcceptClicked);
	Params.LeftButton.State = bCanActOnInvite
		? T66ScreenSlateHelpers::EFriendslopStandardModalButtonState::Selected
		: T66ScreenSlateHelpers::EFriendslopStandardModalButtonState::Disabled;
	Params.LeftButton.Chrome = T66ScreenSlateHelpers::EFriendslopStandardModalButtonChrome::Green;
	Params.LeftButton.IsEnabled = bCanActOnInvite;
	Params.LeftButton.Tag = PartyInviteTag(TEXT("PartyInviteModal.AcceptButton"));
	Params.LeftButton.FontSize = 21;
	Params.RightButton.Label = RejectText;
	Params.RightButton.OnClicked = FOnClicked::CreateUObject(this, &UT66PartyInviteModal::HandleRejectClicked);
	Params.RightButton.State = bCanActOnInvite
		? T66ScreenSlateHelpers::EFriendslopStandardModalButtonState::Default
		: T66ScreenSlateHelpers::EFriendslopStandardModalButtonState::Disabled;
	Params.RightButton.Chrome = T66ScreenSlateHelpers::EFriendslopStandardModalButtonChrome::Red;
	Params.RightButton.IsEnabled = bCanActOnInvite;
	Params.RightButton.Tag = PartyInviteTag(TEXT("PartyInviteModal.RejectButton"));
	Params.RightButton.FontSize = 21;

	return T66ScreenSlateHelpers::MakeFriendslopStandardModal(Params);
}

FReply UT66PartyInviteModal::HandleAcceptClicked()
{
	FLagScopedScope LagScope(GetWorld(), TEXT("MP-03 PartyInviteModal::AcceptClick"));

	if (bActionInFlight)
	{
		return FReply::Handled();
	}

	const FT66PartyInviteEntry* Invite = GetCurrentInvite();
	if (!Invite)
	{
		CloseModal();
		return FReply::Handled();
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			ActionInviteId = Invite->InviteId;
			ActionHostSteamId = Invite->HostSteamId;
			ActionHostLobbyId = Invite->HostLobbyId;
			ActionHostAppId = Invite->HostAppId;
			ActionStatusText = TEXT("Accepting invite...");
			bAcceptingInvite = true;
			FT66MultiplayerDiagnosticContext Diagnostic;
			Diagnostic.EventName = TEXT("invite_accept_click");
			Diagnostic.Severity = TEXT("info");
			Diagnostic.Message = TEXT("Player accepted a party invite.");
			Diagnostic.InviteId = ActionInviteId;
			Diagnostic.HostSteamId = ActionHostSteamId;
			Diagnostic.LobbyId = ActionHostLobbyId;
			Diagnostic.SourceAppId = ActionHostAppId;
			Backend->SubmitMultiplayerDiagnostic(Diagnostic);
			bActionInFlight = Backend->RespondToPartyInvite(Invite->InviteId, true);
			bJoinKickoffStarted = false;
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Accept clicked for invite %s host=%s lobby=%s app=%s"), *ActionInviteId, *ActionHostSteamId, *ActionHostLobbyId, *ActionHostAppId);

			if (TryStartJoinKickoff())
			{
				bJoinKickoffStarted = true;
				ActionStatusText = TEXT("Joining party...");
				CloseModal();
				return FReply::Handled();
			}

			if (bActionInFlight)
			{
				ForceRebuildSlate();
			}
			else
			{
				ActionStatusText = TEXT("Invite accept could not be sent.");
				ForceRebuildSlate();
			}
		}
	}

	return FReply::Handled();
}

FReply UT66PartyInviteModal::HandleRejectClicked()
{
	if (bActionInFlight)
	{
		return FReply::Handled();
	}

	const FT66PartyInviteEntry* Invite = GetCurrentInvite();
	if (!Invite)
	{
		CloseModal();
		return FReply::Handled();
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			ActionInviteId = Invite->InviteId;
			ActionHostSteamId.Reset();
			ActionHostLobbyId.Reset();
			ActionHostAppId.Reset();
			ActionStatusText = TEXT("Rejecting invite...");
			bAcceptingInvite = false;
			bActionInFlight = Backend->RespondToPartyInvite(Invite->InviteId, false);
			bJoinKickoffStarted = false;
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Reject clicked for invite %s"), *ActionInviteId);
			if (bActionInFlight)
			{
				ForceRebuildSlate();
			}
			else
			{
				ActionStatusText = TEXT("Invite reject could not be sent.");
				ForceRebuildSlate();
			}
		}
	}

	return FReply::Handled();
}

void UT66PartyInviteModal::HandlePartyInviteActionComplete(bool bSuccess, const FString& Action, const FString& InviteId, const FString& Message)
{
	FLagScopedScope LagScope(GetWorld(), TEXT("MP-03 PartyInviteModal::ActionComplete"));

	if (InviteId != ActionInviteId || bActionInFlight == false)
	{
		UE_LOG(LogT66PartyInviteModal, Verbose, TEXT("Ignoring action-complete callback success=%d action=%s invite=%s current=%s inFlight=%d"), bSuccess ? 1 : 0, *Action, *InviteId, *ActionInviteId, bActionInFlight ? 1 : 0);
		return;
	}

	bActionInFlight = false;
	ActionStatusText = Message;
	UE_LOG(LogT66PartyInviteModal, Log, TEXT("Invite action complete success=%d action=%s invite=%s message=%s"), bSuccess ? 1 : 0, *Action, *InviteId, *Message);

	if (!bSuccess)
	{
		ForceRebuildSlate();
		return;
	}

	if (bAcceptingInvite)
	{
		if (bJoinKickoffStarted)
		{
			CloseModal();
			return;
		}

		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
			if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
			{
				T66RefreshInviteJoinContext(Backend, ActionInviteId, ActionHostSteamId, ActionHostLobbyId, ActionHostAppId);
			}

			if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
			{
				const bool bJoinStarted =
					!ActionHostLobbyId.IsEmpty()
						? SessionSubsystem->JoinPartySessionByLobbyId(ActionHostLobbyId, ActionHostSteamId, ActionHostAppId, ActionInviteId)
						: SessionSubsystem->JoinFriendPartySessionBySteamId(ActionHostSteamId, ActionInviteId, ActionHostAppId);
				UE_LOG(LogT66PartyInviteModal, Log, TEXT("Invite accept join start result=%d host=%s lobby=%s app=%s"), bJoinStarted ? 1 : 0, *ActionHostSteamId, *ActionHostLobbyId, *ActionHostAppId);
				if (!bJoinStarted)
				{
					ActionStatusText = TEXT("Invite accepted, but the party join could not start.");
					ForceRebuildSlate();
					return;
				}
			}
		}
	}

	CloseModal();
}

bool UT66PartyInviteModal::TryStartJoinKickoff()
{
	if (!bAcceptingInvite)
	{
		return false;
	}

	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UT66BackendSubsystem* Backend = GI->GetSubsystem<UT66BackendSubsystem>())
		{
			T66RefreshInviteJoinContext(Backend, ActionInviteId, ActionHostSteamId, ActionHostLobbyId, ActionHostAppId);
		}

		if (UT66SessionSubsystem* SessionSubsystem = GI->GetSubsystem<UT66SessionSubsystem>())
		{
			const bool bJoinStarted =
				!ActionHostLobbyId.IsEmpty()
					? SessionSubsystem->JoinPartySessionByLobbyId(ActionHostLobbyId, ActionHostSteamId, ActionHostAppId, ActionInviteId)
					: SessionSubsystem->JoinFriendPartySessionBySteamId(ActionHostSteamId, ActionInviteId, ActionHostAppId);
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Invite join kickoff result=%d host=%s lobby=%s app=%s"), bJoinStarted ? 1 : 0, *ActionHostSteamId, *ActionHostLobbyId, *ActionHostAppId);
			return bJoinStarted;
		}
	}

	return false;
}
