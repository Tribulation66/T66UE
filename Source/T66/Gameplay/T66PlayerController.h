// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/T66UITypes.h"
#include "T66PlayerController.generated.h"

class UT66UIManager;
class UT66ScreenBase;
class UT66GameplayHUDWidget;
class UT66RunStateSubsystem;

/**
 * Player Controller for Tribulation 66
 * Handles both frontend (UI) and gameplay modes
 * Behavior changes based on which level is loaded
 */
UCLASS(Blueprintable)
class T66_API AT66PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AT66PlayerController();

	/** The UI Manager instance (only created in frontend mode) */
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

	/** Check if we are in a frontend/menu level */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	bool IsFrontendLevel() const;

	/** Check if we are in a gameplay level */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game")
	bool IsGameplayLevel() const;

	/** Restore gameplay input mode and hide cursor (call after closing pause menu) */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void RestoreGameplayInputMode();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Called when in gameplay mode to set up game input */
	virtual void SetupGameplayMode();

	/** Handle movement input (WASD) */
	void HandleMoveForward(float Value);
	void HandleMoveRight(float Value);

	/** Handle look input (Mouse) */
	void HandleLookUp(float Value);
	void HandleTurn(float Value);

	/** Handle scroll wheel zoom (game world only) */
	void HandleZoom(float Value);

	/** Handle jump */
	void HandleJumpPressed();
	void HandleJumpReleased();

	/** Toggle pause menu (gameplay only, Escape) */
	void HandleEscapePressed();

	/** Toggle HUD panels (inventory + minimap), gameplay only, T key */
	void HandleToggleHUDPressed();

	/** Interact (F): vendor sell or pickup collect */
	void HandleInteractPressed();

	UFUNCTION()
	void OnPlayerDied();

private:
	/** Gameplay HUD (hearts, gold, inventory, minimap); created in gameplay BeginPlay */
	UPROPERTY()
	TObjectPtr<UT66GameplayHUDWidget> GameplayHUDWidget;

	FDelegateHandle OnPlayerDiedHandle;

	void SetupGameplayHUD();
	bool bUIInitialized = false;

	/** Last time Escape/Pause was handled (debounce rapid key repeat) */
	float LastPauseToggleTime = 0.f;

	/** Seconds to ignore repeat Escape/Pause after handling */
	static constexpr float PauseToggleDebounceSeconds = 0.25f;

	/** In gameplay: lazily-created UIManager for pause menu only */
	void EnsureGameplayUIManager();

	/** Load screen classes from expected paths */
	void AutoLoadScreenClasses();
};
