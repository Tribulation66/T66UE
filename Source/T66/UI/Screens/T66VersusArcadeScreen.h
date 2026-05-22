// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66ArcadeInteractableTypes.h"
#include "UI/T66ScreenBase.h"
#include "T66VersusArcadeScreen.generated.h"

UCLASS(Blueprintable)
class T66_API UT66VersusArcadeScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66VersusArcadeScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Versus")
	void OnBackClicked();

protected:
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	TSharedRef<SWidget> BuildArcadeTile(ET66ArcadeGameType GameType, int32 Index);
	bool IsArcadeGameAllowed(ET66ArcadeGameType GameType) const;
	bool LaunchArcadeGame(ET66ArcadeGameType GameType);

	FReply HandleBackClicked();
	FReply HandlePlayRandomClicked();
	FReply HandleGameClicked(ET66ArcadeGameType GameType);
};
