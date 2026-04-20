// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66MiniDifficultySelectScreen.generated.h"

struct FT66MiniHeroDefinition;
struct FSlateBrush;

UCLASS(Blueprintable)
class T66MINI_API UT66MiniDifficultySelectScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66MiniDifficultySelectScreen(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual void OnScreenDeactivated_Implementation() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleBackClicked();
	FReply HandleContinueClicked();
	FReply HandleDifficultyClicked(FName DifficultyID);
	void HandleSessionStateChanged();
	void SyncToSharedPartyScreen();
	FString BuildSessionUiStateKey() const;
	void RefreshSelectedHeroBrush(const FT66MiniHeroDefinition* Hero);

	TSharedPtr<FSlateBrush> SelectedHeroBrush;
	FDelegateHandle SessionStateChangedHandle;
	FString LastSessionUiStateKey;
};
