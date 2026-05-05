// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Components/T66MinigameMenuLayout.h"
#include "UI/T66ScreenBase.h"
#include "UObject/StrongObjectPtr.h"
#include "T66MiniMainMenuScreen.generated.h"

class ST66MinigameMenuLayout;
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
	TSharedRef<SWidget> BuildSharedMainMenuUI();
	TArray<FT66MinigameDifficultyOption> BuildDifficultyOptions() const;
	TArray<FT66MinigameLeaderboardEntry> BuildDailyLeaderboardEntries(FName DifficultyID) const;
	TArray<FT66MinigameLeaderboardEntry> BuildAllTimeLeaderboardEntries(FName DifficultyID) const;
	FText GetDailyLeaderboardStatus(FName DifficultyID) const;
	FText GetAllTimeLeaderboardStatus(FName DifficultyID) const;
	FReply HandleBackToMainMenuClicked();
	FReply HandleNewGameClicked();
	FReply HandleLoadGameClicked();
	FReply HandleDailyClicked();
	void RequestMiniMenuTextures();
	void ReleaseRetainedSlateState();

	TSharedPtr<ST66MinigameMenuLayout> SharedMenuLayout;
	TSharedPtr<FSlateBrush> SkyBackgroundBrush;
	TStrongObjectPtr<UTexture2D> SkyBackgroundTexture;
	TSharedPtr<FSlateBrush> FireMoonBrush;
	TStrongObjectPtr<UTexture2D> FireMoonTexture;
	TSharedPtr<FSlateBrush> PyramidChadBrush;
	TStrongObjectPtr<UTexture2D> PyramidChadTexture;
	TSharedPtr<FSlateBrush> PrimaryCTAFillBrush;
	TStrongObjectPtr<UTexture2D> PrimaryCTAFillTexture;
};
