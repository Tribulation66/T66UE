// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66PauseMenuScreen.generated.h"

class AT66PlayerController;
class ST66FlatLeaderboardPanel;
struct FSlateBrush;

UCLASS(Blueprintable)
class T66_API UT66PauseMenuScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66PauseMenuScreen(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnResumeClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnSaveAndQuitClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnQuitClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnRestartClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnSettingsClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnAchievementsClicked();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void OnLeaderboardClicked();

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;
	virtual bool HandleBackAction() override;

private:
	FReply HandleResumeClicked();
	FReply HandleSaveAndQuitClicked();
	FReply HandleQuitClicked();
	FReply HandleRestartClicked();
	FReply HandleSettingsClicked();
	FReply HandleAchievementsClicked();
	FReply HandleLeaderboardClicked();
	FReply HandleLeaderboardBackClicked();

	AT66PlayerController* GetT66PlayerController() const;

	bool bLeaderboardModalOpen = false;
	TSharedPtr<ST66FlatLeaderboardPanel> FlatLeaderboardPanel;
	TSharedPtr<FSlateBrush> PortraitBrush;
	TSharedPtr<FSlateBrush> HeartBrush;
	TArray<TSharedPtr<FSlateBrush>> IdolSlotBrushes;
	TArray<TSharedPtr<FSlateBrush>> InventorySlotBrushes;
};
