// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "UI/T66ScreenBase.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "T66SettingsScreen.generated.h"

class UT66LocalizationSubsystem;
struct FKeyEvent;
struct FPointerEvent;

UENUM(BlueprintType)
enum class ET66SettingsTab : uint8
{
	Gameplay,
	Graphics,
	Controls,
	Audio,
	Crashing
};

/**
 * Settings Screen - Modal with 5 tabbed sections
 * Bible 1.17: Gameplay, Graphics, Controls, Audio, Crashing
 */
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
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UT66LocalizationSubsystem* GetLocSubsystem() const;

	// Widget switcher for tab content
	TSharedPtr<SWidgetSwitcher> ContentSwitcher;

	// Tab content builders
	TSharedRef<SWidget> BuildGameplayTab();
	TSharedRef<SWidget> BuildGraphicsTab();
	TSharedRef<SWidget> BuildControlsTab();
	TSharedRef<SWidget> BuildAudioTab();
	TSharedRef<SWidget> BuildCrashingTab();

	// Click handlers
	FReply HandleCloseClicked();
	FReply HandleTabClicked(ET66SettingsTab Tab);
	FReply HandleApplyGraphicsClicked();
	FReply HandleRestoreDefaultsClicked();
	FReply HandleReportBugClicked();
	FReply HandleSafeModeClicked();

	// ===== Keybinding capture =====
	struct FPendingRebind
	{
		bool bIsAxis = false;
		FName Name = NAME_None;   // ActionName or AxisName
		float Scale = 1.f;        // Axis only
		FKey OldKey;
		TSharedPtr<class STextBlock> KeyText;
	};

	bool bWaitingForRebind = false;
	FPendingRebind Pending;
	TSharedPtr<class STextBlock> RebindStatusText;

	FReply BeginRebindAction(FName ActionName, const FKey& OldKey, TSharedPtr<class STextBlock> KeyText);
	FReply BeginRebindAxis(FName AxisName, float Scale, const FKey& OldKey, TSharedPtr<class STextBlock> KeyText);
	void ApplyRebindToInputSettings(const FKey& NewKey);
	void ClearBindingInInputSettings();
	static FText KeyToText(const FKey& K);
	static bool IsKeyboardMouseKey(const FKey& K);
	static FKey FindActionKey(FName ActionName, const FKey& PreferredOldKey);
	static FKey FindAxisKey(FName AxisName, float Scale, const FKey& PreferredOldKey);
};
