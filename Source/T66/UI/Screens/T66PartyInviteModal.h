// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "UI/T66ScreenBase.h"
#include "T66PartyInviteModal.generated.h"

class UT66BackendSubsystem;

UCLASS(Blueprintable)
class T66_API UT66PartyInviteModal : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66PartyInviteModal(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;

private:
	FReply HandleAcceptClicked();
	FReply HandleRejectClicked();
	void HandlePartyInviteActionComplete(bool bSuccess, const FString& Action, const FString& InviteId, const FString& Message);
	const struct FT66PartyInviteEntry* GetCurrentInvite() const;

	FDelegateHandle PartyInviteActionCompleteHandle;
	FString ActionInviteId;
	FString ActionHostSteamId;
	FString ActionHostLobbyId;
	FString ActionHostAppId;
	bool bAcceptingInvite = false;
	bool bActionInFlight = false;
	FString ActionStatusText;
};
