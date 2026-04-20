// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "UObject/StrongObjectPtr.h"
#include "T66MiniMainMenuScreen.generated.h"

class ST66MiniLeaderboardPanel;
class UTexture2D;

UCLASS(Blueprintable)
class T66MINI_API UT66MiniMainMenuScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniMainMenuScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackToMainMenuClicked();
	FReply HandleNewGameClicked();
	FReply HandleLoadGameClicked();
	void HandlePartyStateChanged();
	void HandleSessionStateChanged();
	void SyncToSharedPartyScreen();
	FString BuildPartyUiStateKey() const;
	void HandleFriendSearchTextChanged(const FText& InText);
	void RequestMiniMenuTextures();
	void ReleaseRetainedSlateState();

	TSharedPtr<ST66MiniLeaderboardPanel> LeaderboardPanel;
	TSharedPtr<FSlateBrush> SkyBackgroundBrush;
	TStrongObjectPtr<UTexture2D> SkyBackgroundTexture;
	TSharedPtr<FSlateBrush> FireMoonBrush;
	TStrongObjectPtr<UTexture2D> FireMoonTexture;
	TSharedPtr<FSlateBrush> PyramidChadBrush;
	TStrongObjectPtr<UTexture2D> PyramidChadTexture;
	TSharedPtr<FSlateBrush> PrimaryCTAFillBrush;
	TStrongObjectPtr<UTexture2D> PrimaryCTAFillTexture;
	TSharedPtr<FSlateBrush> ProfileAvatarBrush;
	TArray<TSharedPtr<FSlateBrush>> FriendAvatarBrushes;
	TArray<TSharedPtr<FSlateBrush>> PartyAvatarBrushes;
	FString FriendSearchQuery;
	FString LastPartyUiStateKey;
	FDelegateHandle PartyStateChangedHandle;
	FDelegateHandle SessionStateChangedHandle;
};
