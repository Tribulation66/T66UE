// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/T66ScreenBase.h"
#include "T66SettingsScreen.generated.h"

UENUM(BlueprintType)
enum class ET66SettingsTab : uint8
{
	Gameplay, Graphics, Controls, Audio, Crashing
};

UCLASS(Blueprintable)
class T66_API UT66SettingsScreen : public UT66ScreenBase
{
	GENERATED_BODY()

public:
	UT66SettingsScreen(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	ET66SettingsTab CurrentTab = ET66SettingsTab::Gameplay;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SwitchToTab(ET66SettingsTab Tab);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void OnApplyGraphicsClicked();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void OnCloseClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void OnTabChanged(ET66SettingsTab NewTab);

protected:
	virtual void OnScreenActivated_Implementation() override;
	virtual TSharedRef<SWidget> BuildSlateUI() override;

private:
	FReply HandleCloseClicked();
};
