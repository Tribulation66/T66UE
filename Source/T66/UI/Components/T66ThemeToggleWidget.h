// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Style/T66Style.h"
#include "T66ThemeToggleWidget.generated.h"

class UT66UIManager;

/**
 * Persistent theme-toggle widget (Dark / Light buttons).
 * Lives at viewport Z-order 50 across all frontend screens.
 * Created and managed by UT66UIManager.
 */
UCLASS()
class T66_API UT66ThemeToggleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Inject the UI Manager so we can refresh the current screen after a theme swap. */
	void SetUIManager(UT66UIManager* InUIManager) { UIManager = InUIManager; }

	/** Tear down and rebuild our Slate tree (e.g. after a theme change). */
	void ForceRebuildSlate();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<UT66UIManager> UIManager;

	ET66UITheme PendingTheme = ET66UITheme::Dark;
	bool bThemeChangeInProgress = false;

	FReply HandleDarkThemeClicked();
	FReply HandleLightThemeClicked();
	void ApplyPendingTheme();
};
