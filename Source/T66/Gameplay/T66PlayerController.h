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
class UT66GamblerOverlayWidget;
class UT66CowardicePromptWidget;
class AT66CowardiceGate;
class UT66CowardicePromptWidget;
class AT66CowardiceGate;
class AT66EnemyBase;
class UT66IdolAltarOverlayWidget;
class UT66VendorOverlayWidget;
class AT66LootBagPickup;
class AT66HouseNPCBase;
class AT66VendorNPC;
class AT66GamblerNPC;
enum class ET66Rarity : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FT66NearbyLootBagChanged);

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

	/** Open the Gambler overlay (non-pausing). */
	void OpenGamblerOverlay(int32 WinGoldAmount);

	/** Open the Vendor overlay (non-pausing). */
	void OpenVendorOverlay();

	/** In-world dialogue (open-world) for vendor/gambler interactions (non-pausing). */
	void OpenWorldDialogueVendor(AT66VendorNPC* Vendor);
	void OpenWorldDialogueGambler(AT66GamblerNPC* Gambler);

	/** Open the Cowardice prompt (non-pausing). */
	void OpenCowardicePrompt(AT66CowardiceGate* Gate);

	/** Wheel spin: play HUD animation + award gold (no overlay). */
	void StartWheelSpinHUD(ET66Rarity Rarity);

	/** Loot proximity: HUD prompt + accept/reject input. */
	void SetNearbyLootBag(AT66LootBagPickup* LootBag);
	void ClearNearbyLootBag(AT66LootBagPickup* LootBag);
	AT66LootBagPickup* GetNearbyLootBag() const { return NearbyLootBag.Get(); }

	/** Broadcast whenever the nearby loot bag changes (enter/exit overlap). */
	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FT66NearbyLootBagChanged NearbyLootBagChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	/** Handle dash (CapsLock) */
	void HandleDashPressed();

	/** Handle jump */
	void HandleJumpPressed();
	void HandleJumpReleased();

	/** Toggle pause menu (gameplay only, Escape) */
	void HandleEscapePressed();

	/** Toggle HUD panels (inventory + minimap), gameplay only, T key */
	void HandleToggleHUDPressed();

	/** Dev toggles (F9/F10 by default). */
	void HandleToggleImmortalityPressed();
	void HandleTogglePowerPressed();

	/** Toggle TikTok placeholder panel (gameplay only, O key). */
	void HandleToggleTikTokPressed();

	/** TikTok controls while viewer is open (no mouse needed). */
	void HandleTikTokPrevPressed();
	void HandleTikTokNextPressed();

	/** Interact (F): vendor sell or pickup collect */
	void HandleInteractPressed();

	/** Ultimate (R): AoE damage with cooldown */
	void HandleUltimatePressed();

	/** Toggle Media Viewer panel (placeholder). */
	void HandleToggleMediaViewerPressed();

	/** Open full map (placeholder). */
	void HandleOpenFullMapPressed();

	/** Toggle Gamer Mode hitbox overlay (placeholder). */
	void HandleToggleGamerModePressed();

	/** Restart run (instant) (placeholder, host-only later). */
	void HandleRestartRunPressed();

	/** Manual attack lock (LMB) / unlock (RMB) */
	void HandleAttackLockPressed();
	void HandleAttackUnlockPressed();

	UFUNCTION()
	void OnPlayerDied();

private:
	/** Gameplay HUD (hearts, gold, inventory, minimap); created in gameplay BeginPlay */
	UPROPERTY()
	TObjectPtr<UT66GameplayHUDWidget> GameplayHUDWidget;

	UPROPERTY()
	TObjectPtr<UT66GamblerOverlayWidget> GamblerOverlayWidget;

	UPROPERTY()
	TObjectPtr<UT66CowardicePromptWidget> CowardicePromptWidget;

	UPROPERTY()
	TObjectPtr<UT66VendorOverlayWidget> VendorOverlayWidget;

	TWeakObjectPtr<AT66LootBagPickup> NearbyLootBag;

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

	bool CanUseCombatMouseInput() const;

	TWeakObjectPtr<AT66EnemyBase> LockedEnemy;

	UPROPERTY()
	TObjectPtr<UT66IdolAltarOverlayWidget> IdolAltarOverlayWidget;

	// ============================================================
	// In-world NPC dialogue (Vendor/Gambler)
	// ============================================================
	enum class ET66WorldDialogueKind : uint8
	{
		None,
		Vendor,
		Gambler,
	};

	ET66WorldDialogueKind WorldDialogueKind = ET66WorldDialogueKind::None;
	bool bWorldDialogueOpen = false;
	int32 WorldDialogueSelectedIndex = 0;
	float LastWorldDialogueNavTimeSeconds = -1000.f;
	static constexpr float WorldDialogueNavDebounceSeconds = 0.18f;

	TWeakObjectPtr<AT66HouseNPCBase> WorldDialogueTargetNPC;
	FTimerHandle WorldDialoguePositionTimerHandle;

	void CloseWorldDialogue();
	void NavigateWorldDialogue(int32 Delta);
	void ConfirmWorldDialogue();
	void UpdateWorldDialoguePosition();
	void TeleportToNPC(FName NPCID);
};
