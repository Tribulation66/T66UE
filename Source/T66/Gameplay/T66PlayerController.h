// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/T66UITypes.h"
#include "T66PlayerController.generated.h"

class UT66UIManager;
class UT66ScreenBase;

/**
 * Player Controller for Tribulation 66
 * Owns and manages the UI system
 */
UCLASS(Blueprintable)
class T66_API AT66PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AT66PlayerController();

	/** The UI Manager instance */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UT66UIManager> UIManager;

	/** Screen classes to register (set in Blueprint) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TMap<ET66ScreenType, TSubclassOf<UT66ScreenBase>> ScreenClasses;

	/** The initial screen to show (usually MainMenu) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	ET66ScreenType InitialScreen = ET66ScreenType::MainMenu;

	/** Whether to auto-show initial screen on BeginPlay */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	bool bAutoShowInitialScreen = true;

	/** Get the UI Manager */
	UFUNCTION(BlueprintCallable, Category = "UI")
	UT66UIManager* GetUIManager() const { return UIManager; }

	/** Show a specific screen */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowScreen(ET66ScreenType ScreenType);

	/** Initialize the UI system (call this if bAutoShowInitialScreen is false) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void InitializeUI();

	/** Auto-load screen classes by convention paths */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	bool bAutoLoadScreenClasses = true;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	bool bUIInitialized = false;

	/** Load screen classes from expected paths */
	void AutoLoadScreenClasses();
};
