// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "T66UITypes.h"
#include "T66ScreenBase.generated.h"

class UT66UIManager;

/**
 * Base class for all T66 screen widgets
 * Provides common functionality for screen lifecycle and navigation
 * Uses Slate-based programmatic UI construction
 */
UCLASS(Abstract, Blueprintable)
class T66_API UT66ScreenBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The screen type this widget represents */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen")
	ET66ScreenType ScreenType = ET66ScreenType::None;

	/** Whether this screen is a modal (displays on top of another screen) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screen")
	bool bIsModal = false;

	/** Reference to the UI Manager (set when screen is shown) */
	UPROPERTY(BlueprintReadOnly, Category = "Screen")
	TObjectPtr<UT66UIManager> UIManager;

	/**
	 * Called when this screen becomes active/visible
	 * Override in Blueprint to initialize screen state
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Screen")
	void OnScreenActivated();
	virtual void OnScreenActivated_Implementation();

	/**
	 * Called when this screen is about to be hidden/deactivated
	 * Override in Blueprint to clean up or save state
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Screen")
	void OnScreenDeactivated();
	virtual void OnScreenDeactivated_Implementation();

	/**
	 * Called to refresh the screen's data display
	 * Use this instead of Tick for updating UI elements
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Screen")
	void RefreshScreen();
	virtual void RefreshScreen_Implementation();

	/**
	 * Request navigation to another screen
	 * Goes through the UI Manager for proper state management
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void NavigateTo(ET66ScreenType NewScreen);

	/**
	 * Request to go back to the previous screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void NavigateBack();

	/**
	 * Show a modal on top of this screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void ShowModal(ET66ScreenType ModalScreen);

	/**
	 * Close the current modal (if this screen is a modal)
	 */
	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void CloseModal();

	/**
	 * Force the underlying Slate widget tree to be rebuilt.
	 * Use this when you need new localized text or layout changes to take effect
	 * (calling TakeWidget() alone usually returns the already-built widget).
	 */
	UFUNCTION(BlueprintCallable, Category = "Screen")
	void ForceRebuildSlate();

protected:
	virtual void NativeConstruct() override;

	/**
	 * Override this in subclasses to build the Slate UI
	 * Return the root Slate widget for this screen
	 */
	virtual TSharedRef<SWidget> BuildSlateUI();

	/** Override to construct the widget */
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// ========== Slate UI Building Helpers ==========

	/** Create a text block */
	TSharedRef<SWidget> MakeText(const FText& Text, int32 FontSize = 24, 
		const FLinearColor& Color = FLinearColor::White);

	/** Create a title text (larger) */
	TSharedRef<SWidget> MakeTitle(const FText& Text, int32 FontSize = 48);

	/** Wrap content in a size box */
	TSharedRef<SWidget> WrapInSizeBox(TSharedRef<SWidget> Content, float Width, float Height);

	/** Wrap content in a border/background */
	TSharedRef<SWidget> WrapInBorder(TSharedRef<SWidget> Content, 
		const FLinearColor& Color = FLinearColor(0.05f, 0.05f, 0.08f, 0.9f));

	/** Flag indicating if UI was built */
	bool bSlateUIBuilt = false;
};
