// Copyright Tribulation 66. All Rights Reserved.

#include "UI/Screens/T66PartyInviteModal.h"
#include "Core/T66BackendSubsystem.h"
#include "Core/T66LagTrackerSubsystem.h"
#include "Core/T66SessionSubsystem.h"
#include "UI/Style/T66Style.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

UT66PartyInviteModal::UT66PartyInviteModal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ScreenType = ET66ScreenType::PartyInvite;
	bIsModal = true;
}

DEFINE_LOG_CATEGORY_STATIC(LogT66PartyInviteModal, Log, All);

void UT66PartyInviteModal::OnScreenActivated_Implementation()
{
	Super::OnScreenActivated_Implementation();

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

	constexpr float T66InviteButtonMinWidth = 280.f;
	constexpr float T66InviteButtonHeight = 52.f;
	const FMargin T66InviteButtonPadding(24.f, 12.f);

	return SNew(SBorder)
		.BorderBackgroundColor(FT66Style::Scrim())
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				FT66Style::MakePanel(
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 18.f)
					[
						SNew(STextBlock)
						.Text(TitleText)
						.Font(FT66Style::Tokens::FontBold(34))
						.ColorAndOpacity(FT66Style::Tokens::Text)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, 24.f)
					[
						SNew(STextBlock)
						.Text(InviteBodyText)
						.Font(FT66Style::Tokens::FontRegular(18))
						.ColorAndOpacity(FT66Style::Tokens::TextMuted)
						.Justification(ETextJustify::Center)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0.f, 0.f, 0.f, StatusText.IsEmpty() ? 0.f : 18.f)
					[
						SNew(STextBlock)
						.Text(StatusText)
						.Font(FT66Style::Tokens::FontRegular(14))
						.ColorAndOpacity(FLinearColor(0.90f, 0.77f, 0.43f, 1.0f))
						.Visibility(StatusText.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									AcceptText,
									FOnClicked::CreateUObject(this, &UT66PartyInviteModal::HandleAcceptClicked),
									ET66ButtonType::Success)
								.SetMinWidth(T66InviteButtonMinWidth)
								.SetHeight(T66InviteButtonHeight)
								.SetPadding(T66InviteButtonPadding))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(10.f, 0.f)
						[
							FT66Style::MakeButton(
								FT66ButtonParams(
									RejectText,
									FOnClicked::CreateUObject(this, &UT66PartyInviteModal::HandleRejectClicked),
									ET66ButtonType::Danger)
								.SetMinWidth(T66InviteButtonMinWidth)
								.SetHeight(T66InviteButtonHeight)
								.SetPadding(T66InviteButtonPadding))
						]
					]
				,
				FT66PanelParams(ET66PanelType::Panel).SetPadding(FMargin(42.f, 30.f)))
			]
		];
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
			bActionInFlight = true;
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Accept clicked for invite %s host=%s lobby=%s app=%s"), *ActionInviteId, *ActionHostSteamId, *ActionHostLobbyId, *ActionHostAppId);
			ForceRebuildSlate();
			Backend->RespondToPartyInvite(Invite->InviteId, true);
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
			bActionInFlight = true;
			UE_LOG(LogT66PartyInviteModal, Log, TEXT("Reject clicked for invite %s"), *ActionInviteId);
			ForceRebuildSlate();
			Backend->RespondToPartyInvite(Invite->InviteId, false);
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
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
		{
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
