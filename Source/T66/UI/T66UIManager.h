// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "T66UITypes.h"
#include "T66UIManager.generated.h"

class UT66ScreenBase;
class UT66ThemeToggleWidget;
class UUserWidget;

/**
 * UI Manager for Tribulation 66
 * Handles screen navigation, transitions, and modal management
 * Created and owned by the HUD or PlayerController
 */
UCLASS(Blueprintable, BlueprintType)
class T66_API UT66UIManager : public UObject
{
	GENERATED_BODY()

public:
	UT66UIManager();

	/**
	 * Initialize the UI Manager with screen class mappings
	 * Must be called before showing any screens
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void Initialize(APlayerController* InOwningPlayer);

	/**
	 * Register a widget class for a screen type
	 * Call this during setup to define which widget Blueprint to use for each screen
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RegisterScreenClass(ET66ScreenType ScreenType, TSubclassOf<UT66ScreenBase> WidgetClass);

	/**
	 * Show a screen (replaces current screen if not a modal)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowScreen(ET66ScreenType ScreenType);

	/**
	 * Show a modal on top of the current screen
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowModal(ET66ScreenType ModalType);

	/**
	 * Close the current modal (returns to underlying screen)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseModal();

	/**
	 * Navigate back to the previous screen in history
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void GoBack();

	/**
	 * Get the currently active screen type
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	ET66ScreenType GetCurrentScreenType() const { return CurrentScreenType; }

	/**
	 * Get the currently active screen widget
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	UT66ScreenBase* GetCurrentScreen() const { return CurrentScreen; }

	/**
	 * Check if a modal is currently displayed
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool IsModalActive() const { return CurrentModal != nullptr; }

	/**
	 * Get the type of the currently displayed modal (None if no modal)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	ET66ScreenType GetCurrentModalType() const;

	/**
	 * Hide all UI (for transitioning to gameplay)
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideAllUI();

	/**
	 * Delegate broadcast when screen changes
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScreenChanged, ET66ScreenType, OldScreen, ET66ScreenType, NewScreen);
	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnScreenChanged OnScreenChanged;

protected:
	/** Create a screen widget instance */
	UT66ScreenBase* CreateScreen(ET66ScreenType ScreenType);

	/** The owning player controller */
	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayer;

	/** Registered screen classes */
	UPROPERTY()
	TMap<ET66ScreenType, TSubclassOf<UT66ScreenBase>> ScreenClasses;

	/** Cached screen instances (reused for performance) */
	UPROPERTY()
	TMap<ET66ScreenType, TObjectPtr<UT66ScreenBase>> ScreenCache;

	/** Currently displayed main screen */
	UPROPERTY()
	TObjectPtr<UT66ScreenBase> CurrentScreen;

	/** Current screen type */
	ET66ScreenType CurrentScreenType = ET66ScreenType::None;

	/** Currently displayed modal (if any) */
	UPROPERTY()
	TObjectPtr<UT66ScreenBase> CurrentModal;

	/** Navigation history stack for back navigation */
	UPROPERTY()
	TArray<ET66ScreenType> NavigationHistory;

	/** Maximum history depth to prevent memory bloat */
	static constexpr int32 MaxHistoryDepth = 10;

	/** Persistent Dark/Light theme toggle (viewport Z-order 50, above screens, below modals) */
	UPROPERTY()
	TObjectPtr<UT66ThemeToggleWidget> ThemeToggle;
};
