// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66MiniPauseMenuWidget.generated.h"

class SButton;
class STextBlock;
class SWidget;
class SWidgetSwitcher;
struct FKeyEvent;
struct FPointerEvent;

enum class ET66MiniPauseSettingsTab : uint8
{
	Audio,
	KeyBinding,
	Graphics,
	Gameplay
};

UCLASS(Blueprintable)
class T66MINI_API UT66MiniPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UT66MiniPauseMenuWidget(const FObjectInitializer& ObjectInitializer);

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	bool HandlePauseBackAction();

private:
	struct FPendingMiniRebind
	{
		FName ActionName = NAME_None;
		FKey OldKey;
		TSharedPtr<STextBlock> KeyText;
		bool bAllowMouseButtons = true;
	};

	struct FMiniPendingGraphics
	{
		EWindowMode::Type WindowMode = EWindowMode::WindowedFullscreen;
		int32 QualityNotch = 2;
		int32 FpsCapIndex = 1;
		int32 DisplayModeIndex = 0;
		int32 MonitorIndex = 0;
		bool bDirty = false;
	};

	FReply HandleResumeClicked();
	FReply HandleSaveAndQuitClicked();
	FReply HandleSettingsClicked();
	FReply HandleCloseSettingsClicked();
	FReply HandleSettingsTabClicked(ET66MiniPauseSettingsTab NewTab);
	FReply HandleStartRebind(FName ActionName, bool bAllowMouseButtons);
	FReply HandleResetBindingsClicked();
	FReply HandleApplyGraphicsClicked();

	void InitializeGraphicsState();
	void ApplyGraphicsSettings();
	void RefreshSettingsModalVisibility();
	void RefreshSettingsTabVisuals();
	void RefreshBindingText(FName ActionName);
	void RefreshAllBindingTexts();
	void UpdateSliderValueText(FName RowId, float NormalizedValue);
	void UpdateToggleState(FName RowId, bool bEnabled);
	void UpdateCycleValueText(FName RowId, const FText& ValueText);
	bool TryApplyCapturedBinding(const FKey& Key);
	void CancelPendingRebind();
	void SetSettingsStatus(const FText& StatusText);

	class AT66MiniPlayerController* GetMiniController() const;
	class UT66PlayerSettingsSubsystem* GetPlayerSettings() const;

	static FKey FindActionKey(FName ActionName);
	static FText KeyToText(const FKey& Key);
	static bool IsKeyboardMouseKey(const FKey& Key);

	bool bSettingsModalOpen = false;
	bool bWaitingForRebind = false;
	bool bGraphicsInitialized = false;
	double BindingCaptureStartedAt = 0.0;
	ET66MiniPauseSettingsTab CurrentSettingsTab = ET66MiniPauseSettingsTab::Audio;
	FPendingMiniRebind PendingRebind;
	FMiniPendingGraphics PendingGraphics;

	TSharedPtr<SWidget> SettingsModalRoot;
	TSharedPtr<SWidgetSwitcher> SettingsTabSwitcher;
	TSharedPtr<STextBlock> SettingsStatusText;
	TMap<FName, TSharedPtr<STextBlock>> BindingTextMap;
	TMap<FName, TSharedPtr<STextBlock>> SliderValueTextMap;
	TMap<FName, TSharedPtr<SButton>> ToggleButtonMap;
	TMap<FName, TSharedPtr<STextBlock>> ToggleValueTextMap;
	TMap<FName, TSharedPtr<STextBlock>> CycleValueTextMap;
	TMap<uint8, TSharedPtr<SButton>> SettingsTabButtonMap;
	TMap<uint8, TSharedPtr<STextBlock>> SettingsTabTextMap;
};
