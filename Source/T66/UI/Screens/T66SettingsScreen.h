// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "UI/T66ScreenBase.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "T66SettingsScreen.generated.h"

class UT66LocalizationSubsystem;
class UT66PlayerSettingsSubsystem;
struct FKeyEvent;
struct FPointerEvent;

UENUM(BlueprintType)
enum class ET66SettingsTab : uint8
{
	Gameplay,
	Graphics,
	Controls,
	HUD,
	MediaViewer,
	Audio,
	Crashing
};

UENUM()
enum class ET66DisplayMode : uint8
{
	Standard,
	Widescreen
};

UENUM()
enum class ET66ControlsDeviceTab : uint8
{
	KeyboardMouse,
	Controller
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
	UT66PlayerSettingsSubsystem* GetPlayerSettings() const;

	// Widget switcher for tab content
	TSharedPtr<SWidgetSwitcher> ContentSwitcher;
	TSharedPtr<SWidgetSwitcher> ControlsDeviceSwitcher;

	// Gameplay toggles are wired via UT66PlayerSettingsSubsystem (persisted in UT66PlayerSettingsSaveGame).

	// Tab content builders
	TSharedRef<SWidget> BuildGameplayTab();
	TSharedRef<SWidget> BuildGraphicsTab();
	TSharedRef<SWidget> BuildControlsTab();
	TSharedRef<SWidget> BuildHUDTab();
	TSharedRef<SWidget> BuildMediaViewerTab();
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
		bool bIsController = false;
		int32 SlotIndex = 0;      // 0=Primary, 1=Secondary
		FKey OldKey;
		TSharedPtr<class STextBlock> KeyText;
	};

	bool bWaitingForRebind = false;
	FPendingRebind Pending;
	TSharedPtr<class STextBlock> RebindStatusText;

	FReply BeginRebindAction(FName ActionName, bool bIsController, int32 SlotIndex, const FKey& OldKey, TSharedPtr<class STextBlock> KeyText);
	FReply BeginRebindAxis(FName AxisName, float Scale, bool bIsController, int32 SlotIndex, const FKey& OldKey, TSharedPtr<class STextBlock> KeyText);
	void ApplyRebindToInputSettings(const FKey& NewKey);
	void ClearBindingInInputSettings();
	static FText KeyToText(const FKey& K);
	static bool IsKeyboardMouseKey(const FKey& K);
	static bool IsControllerKey(const FKey& K);
	static FKey FindActionKey(FName ActionName, const FKey& PreferredOldKey);
	static FKey FindAxisKey(FName AxisName, float Scale, const FKey& PreferredOldKey);

	// Primary/secondary lookup helpers (device-filtered).
	static void FindActionKeysForDevice(FName ActionName, bool bIsController, TArray<FKey>& OutKeys);
	static void FindAxisKeysForDevice(FName AxisName, float Scale, bool bIsController, TArray<FKey>& OutKeys);

	// ===== Controls UI state =====
	ET66ControlsDeviceTab CurrentControlsDeviceTab = ET66ControlsDeviceTab::KeyboardMouse;
	bool bRestoreDefaultsArmed = false;
	bool bRestoreDefaultsArmedForController = false;
	FTimerHandle RestoreDefaultsArmTimerHandle;

	struct FControlRowKey
	{
		bool bIsAxis = false;
		FName Name = NAME_None;
		float Scale = 1.f;
		bool bIsController = false;
		int32 SlotIndex = 0;

		friend bool operator==(const FControlRowKey& A, const FControlRowKey& B)
		{
			return A.bIsAxis == B.bIsAxis
				&& A.Name == B.Name
				&& FMath::IsNearlyEqual(A.Scale, B.Scale)
				&& A.bIsController == B.bIsController
				&& A.SlotIndex == B.SlotIndex;
		}
	};

	friend uint32 GetTypeHash(const FControlRowKey& K)
	{
		uint32 H = HashCombine(GetTypeHash(K.bIsAxis), GetTypeHash(K.Name));
		H = HashCombine(H, GetTypeHash(static_cast<uint32>(K.bIsController)));
		H = HashCombine(H, GetTypeHash(static_cast<uint32>(K.SlotIndex)));
		H = HashCombine(H, GetTypeHash(static_cast<int32>(K.Scale * 1000.f)));
		return H;
	}

	TMap<FControlRowKey, TSharedPtr<class STextBlock>> ControlKeyTextMap;

	void RefreshControlsKeyTexts();

	// ===== Graphics staging + confirm =====
	struct FPendingGraphics
	{
		FIntPoint Resolution = FIntPoint(1920, 1080);
		EWindowMode::Type WindowMode = EWindowMode::Fullscreen;
		ET66DisplayMode DisplayMode = ET66DisplayMode::Standard;
		int32 MonitorIndex = 0;        // 0 = primary, 1+ = secondary
		int32 QualityNotch = 3;        // 0..3
		int32 FpsCapIndex = 1;         // 0=30,1=60,2=90,3=120,4=Unlimited
		bool bDirty = false;
	};

	FPendingGraphics PendingGraphics;
	bool bGraphicsInitialized = false;

	bool bVideoModeConfirmActive = false;
	int32 VideoModeConfirmSecondsRemaining = 0;
	FTimerHandle VideoModeConfirmTimerHandle;

	TSharedPtr<class STextBlock> VideoModeConfirmCountdownText;

	void InitializeGraphicsFromUserSettingsIfNeeded();
	void ApplyPendingGraphics(bool bForceConfirmPrompt);
	void StartVideoModeConfirmPrompt();
	void EndVideoModeConfirmPrompt(bool bKeepNewSettings);
};
