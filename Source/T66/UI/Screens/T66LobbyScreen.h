// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "Data/T66DataTypes.h"
#include "T66LobbyScreen.generated.h"

struct FSlateBrush;

/**
 * Lobby Screen (co-op only)
 * Bible-style: left = players in lobby (hero portrait + name), right = friends list (Steam photo, name, Invite).
 * Bottom right: Select Hero, Ready Check (host); after Ready Check Yes → Enter the Tribulation.
 * Select Hero opens Hero Selection without Enter button; Back from Hero Selection returns here.
 */
UCLASS(Blueprintable)
class T66_API UT66LobbyScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66LobbyScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void OnSelectHeroClicked();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void OnReadyCheckClicked();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void OnEnterTribulationClicked();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void OnInviteFriendClicked();

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void OnBackClicked();

	/** Called by LobbyReadyCheck modal when user clicks Ready. */
	void SetReadyCheckConfirmed(bool bConfirmed);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void RefreshScreen_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleSelectHeroClicked();
	FReply HandleReadyCheckClicked();
	FReply HandleEnterTribulationClicked();
	FReply HandleInviteFriendClicked();
	FReply HandleBackClicked();

	void StartRunFromLobby();

	/** Number of slots (3 for co-op). */
	int32 GetPartySlotCount() const;

	/** When true, Ready Check was confirmed; show Enter the Tribulation instead of Ready Check. */
	bool bReadyCheckConfirmed = false;

	/** Hero portrait brush for local player (slot 0) in left panel. */
	TSharedPtr<FSlateBrush> LocalPlayerHeroPortraitBrush;
	/** Companion portrait brush for local player (slot 0). */
	TSharedPtr<FSlateBrush> LocalPlayerCompanionPortraitBrush;

	/** Difficulty dropdown (same list as Hero Selection). */
	TArray<TSharedPtr<FString>> LobbyDifficultyOptions;
	TSharedPtr<FString> LobbyCurrentDifficultyOption;

	void OnLobbyDifficultyChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
};
