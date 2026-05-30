// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/T66SessionPlayerState.h"
#include "Gameplay/T66CombatTargetTypes.h"
#include "GameFramework/PlayerController.h"
#include "UI/T66LootWheelPresentationTypes.h"
#include "UI/T66UITypes.h"
#include "UI/WidgetGames/T66WidgetGameHostContext.h"
#include "T66PlayerController.generated.h"

class UT66UIManager;
class UT66ScreenBase;
class UT66GameplayHUDWidget;
class UT66LabOverlayWidget;
class UT66RunStateSubsystem;
class UT66CasinoOverlayWidget;
class UT66CowardicePromptWidget;
class AT66CowardiceGate;
class AT66EnemyBase;
class UT66IdolAltarOverlayWidget;
class UT66WeaponAltarOverlayWidget;
class UNiagaraSystem;
class UT66CollectorOverlayWidget;
class UT66ArcadePopupWidget;
class UT66LoadingScreenWidget;
class AT66LootBagPickup;
class AT66HouseNPCBase;
class AT66CasinoNPC;
class AT66CasinoInteractable;
class AT66VendorInteractable;
class AT66RecruitableCompanion;
class AT66HeroBase;
class AT66ArcadeInteractableBase;
class UT66CombatComponent;
class UT66CombatHitZoneComponent;
enum class ET66Rarity : uint8;
struct FT66ArcadeInteractableData;
struct FT66WidgetGameResult;
class SWidget;
class SWeakWidget;
class STextBlock;
class UInputAction;
class UInputMappingContext;
class UMaterialInterface;
class UPrimitiveComponent;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FT66NearbyLootBagChanged);

USTRUCT()
struct FT66CameraWallOccluderOriginalMaterials
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	UPROPERTY(Transient)
	bool bHadDisallowNanite = false;
};

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

	/** Runtime screen class cache. Kept transient so stale Blueprint defaults cannot retain deleted screens. */
	UPROPERTY(Transient)
	TMap<ET66ScreenType, TSubclassOf<UT66ScreenBase>> RuntimeScreenClasses;

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

	/** Ensure gameplay input, HUD, and camera presentation are active after travel/spawn recovery. */
	void EnsureGameplayRuntimePresentation();

	/** Rebuild gameplay mouse mappings from the current InputSettings action bindings. */
	UFUNCTION(BlueprintCallable, Category = "Game|Input")
	void RefreshGameplayMouseMappings();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void RebuildThemeAwareUI();

	void PushLobbyProfileToServer(const FT66LobbyPlayerInfo& LobbyInfo);
	void PushPartyRunSummaryToServer(const FString& RequestKey, const FString& RunSummaryJson);
	void RefreshPartyInviteModal();

	UFUNCTION(Server, Reliable)
	void ServerRequestPartySaveAndReturnToFrontend();

	UFUNCTION(Client, Reliable)
	void ClientApplyGameplayRunSettings(int32 InRunSeed, ET66Difficulty InDifficulty, ET66MainMapLayoutVariant InLayoutVariant);

	bool IsHeroOneScopedUltActive() const { return bHeroOneScopedUltActive; }
	bool IsHeroOneScopeViewEnabled() const { return bHeroOneScopeViewEnabled; }
	float GetHeroOneScopedUltRemainingSeconds() const;
	float GetHeroOneScopedShotCooldownRemainingSeconds() const;
	bool HasAttackLockedEnemy() const;
	bool GetAttackLockScreenPosition(FVector2D& OutScreenPosition) const;

	/** Dev console overlay: Enter to open, Esc to close. Non-shipping builds only. */
	void ToggleDevConsole();

	/** Open the shared casino shell overlay (gambling + shop + alchemy). */
	void OpenCasinoOverlay();

	/** Close the shared casino shell overlay and return to gameplay input. */
	void CloseCasinoOverlay();

	void SwitchCasinoOverlayToGamblerTab();
	void SwitchCasinoOverlayToVendorTab();
	void SwitchCasinoOverlayToAlchemy();
	bool IsCasinoOverlayOpen() const;
	bool SpawnVendorBoss();
	void ApplyCasinoOverlayInputMode(bool bReassertNextTick = true);

	void OpenCasinoVendorTab();
	bool OpenCasinoGamblerInteractable(AT66CasinoInteractable* SourceInteractable);
	bool OpenVendorInteractable(AT66VendorInteractable* SourceInteractable);
	void HandleCasinoInteractableGambleResolved(FName GameID, bool bSuccessful, int32 PayoutGold);

	/** Open the Lab Collector full-screen overlay (non-pausing). */
	void OpenCollectorOverlay();

	/** Open a run-time arcade popup without pausing gameplay. */
	bool OpenArcadePopup(const FT66ArcadeInteractableData& ArcadeData, AT66ArcadeInteractableBase* SourceInteractable);
	bool OpenArcadePopupFromFrontend(const FT66ArcadeInteractableData& ArcadeData);
	void HandleArcadeGameSelected(UT66ArcadePopupWidget* SelectorWidget, const FT66ArcadeInteractableData& SelectedGameData);
	void HandleArcadePopupResult(UT66ArcadePopupWidget* PopupWidget, bool bSucceeded, int32 FinalScore);
	void CloseArcadePopup(bool bSucceeded, int32 FinalScore = 0);
	bool IsArcadePopupOpen() const;

	/** In-world dialogue (open-world) for casino/companion interactions (non-pausing). */
	void OpenWorldDialogueCasino(AT66CasinoNPC* CasinoNPC);
	void OpenWorldDialogueCompanion(AT66RecruitableCompanion* Companion);

	/** HUD-rendered world interaction prompt. */
	void ShowInteractionPrompt(AActor* SourceActor, const FText& TargetName);
	void HideInteractionPrompt(AActor* SourceActor);

	/** Open the Cowardice prompt (non-pausing). */
	void OpenCowardicePrompt(AT66CowardiceGate* Gate);

	/** Crate open: play CS:GO-style item reveal HUD animation. */
	void StartCrateOpenHUD(ET66Rarity SourceCrateRarity);

	/** Chest open: play an above-inventory gold reward HUD presentation. */
	bool StartChestRewardHUD(ET66Rarity Rarity, int32 GoldAmount, TFunction<void()> OnCommit = nullptr, TFunction<void()> OnFinished = nullptr);

	/** Loot wheel: play radial wheel reward HUD presentation. */
	bool StartLootWheelSpinHUD(FT66LootWheelPresentationParams Params);

	/** Item reward: play an above-inventory item reward HUD presentation. */
	void ShowPickupItemCardHUD(FName ItemID, ET66ItemRarity ItemRarity);

	/** Loot bag item reward: play bag-opening reveal before the item reward card. */
	void ShowLootBagItemRevealHUD(FName ItemID, ET66ItemRarity ItemRarity);

	/** Open the Run Summary after a full-clear victory. */
	void ShowVictoryRunSummary();


	UFUNCTION(Client, Reliable)
	void ClientShowVictoryRunSummary();


	/** Loot proximity: HUD prompt + accept/reject input. */
	void SetNearbyLootBag(AT66LootBagPickup* LootBag);
	void ClearNearbyLootBag(AT66LootBagPickup* LootBag);
	AT66LootBagPickup* GetNearbyLootBag() const { return NearbyLootBag.Get(); }

	/** Broadcast whenever the nearby loot bag changes (enter/exit overlap). */
	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FT66NearbyLootBagChanged NearbyLootBagChanged;

protected:
	virtual void BeginPlay() override;
	virtual void BeginPlayingState() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;
	virtual void ClientRestart_Implementation(APawn* NewPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PlayerTick(float DeltaTime) override;
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

	/** Handle one-button forward roll (default: LeftShift) */
	void HandleRollPressed();

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

	/** Interact (F): shop sell or pickup collect */
	void HandleInteractPressed();

	/** Ultimate (R): AoE damage with cooldown */
	void HandleUltimatePressed();

	/** Toggle Media Viewer panel (placeholder). */
	void HandleToggleMediaViewerPressed();

	/** Open full map (placeholder). */
	void HandleOpenFullMapPressed();

	/** Toggle enlarged inventory inspect mode without pausing gameplay. */
	void HandleInspectInventoryPressed();

	/** Toggle Gamer Mode hitbox overlay (placeholder). */
	void HandleToggleGamerModePressed();

	/** Restart run (instant) (placeholder, host-only later). */
	void HandleRestartRunPressed();

	/** Manual attack lock (LMB) / unlock (default: Mouse Button 5). */
	void HandleAttackLockPressed();
	void HandleAttackUnlockPressed();

	/** Toggle mouse lock for the orbit camera preset. Default: Right Mouse Button. */
	void HandleToggleMouseLockPressed();

	UFUNCTION()
	void OnPlayerDied();

	UFUNCTION(Server, Reliable)
	void ServerSubmitLobbyProfile(const FT66LobbyPlayerInfo& LobbyInfo);

	UFUNCTION(Server, Reliable)
	void ServerSubmitPartyRunSummary(const FString& RequestKey, const FString& RunSummaryJson);

private:
	void UpdateHeroMovementIntent();
	bool IsLockedChaseGameplayCameraMode() const;
	void ClampGameplayCameraPitch();
	void AdjustGameplayCameraPitchFromScroll(float ScrollValue);
	void UpdateLockedChaseGameplayCamera(float DeltaTime);
	void UpdateGameplayCameraSideWallSpring(float DeltaTime);
	void UpdateGameplayCameraWallOcclusion(float DeltaTime);
	void ClearGameplayCameraWallOcclusion();
	UMaterialInterface* GetCameraWallOccluderFadeMaterial();
	void ApplyCameraWallOcclusion(UPrimitiveComponent* Component, UMaterialInterface* FadeMaterial);
	void RestoreCameraWallOcclusion(UPrimitiveComponent* Component);

	TSubclassOf<UT66ScreenBase> ResolveScreenClass(ET66ScreenType ScreenType) const;
	TSubclassOf<UT66GameplayHUDWidget> ResolveGameplayHUDClass() const;
	TSubclassOf<UT66CasinoOverlayWidget> ResolveCasinoOverlayClass() const;
	TSubclassOf<UT66CollectorOverlayWidget> ResolveCollectorOverlayClass() const;
	TSubclassOf<UT66CowardicePromptWidget> ResolveCowardicePromptClass() const;
	TSubclassOf<UT66IdolAltarOverlayWidget> ResolveIdolAltarOverlayClass() const;
	TSubclassOf<UT66WeaponAltarOverlayWidget> ResolveWeaponAltarOverlayClass() const;
	bool SpawnArcadePopupWidget(const FT66ArcadeInteractableData& ArcadeData, AT66ArcadeInteractableBase* SourceInteractable);
	void StartArcadePopupCountdown(UT66ArcadePopupWidget* PopupWidget, const FT66WidgetGameHostContext& HostContext);
	void TickArcadePopupCountdown();
	void FinishArcadePopupCountdown();
	void ClearArcadePopupCountdown();
	void HandleArcadeWidgetGameResult(const FT66WidgetGameResult& Result);

	/** Gameplay HUD (hearts, gold, inventory, minimap); created in gameplay BeginPlay */
	UPROPERTY()
	TObjectPtr<UT66GameplayHUDWidget> GameplayHUDWidget;

	/** Lab overlay (items + enemies panels + exit); shown only during Lab runs. */
	UPROPERTY()
	TObjectPtr<UT66LabOverlayWidget> LabOverlayWidget;

	UPROPERTY()
	TObjectPtr<UT66CasinoOverlayWidget> CasinoOverlayWidget;

	UPROPERTY()
	TWeakObjectPtr<AT66CasinoInteractable> ActiveCasinoInteractable;

	UPROPERTY()
	TWeakObjectPtr<AT66VendorInteractable> ActiveVendorInteractable;

	UPROPERTY()
	TObjectPtr<UT66CowardicePromptWidget> CowardicePromptWidget;

	/** Lab Collector full-screen UI (opened by interacting with The Collector NPC). */
	UPROPERTY()
	TObjectPtr<UT66CollectorOverlayWidget> CollectorOverlayWidget;

	UPROPERTY()
	TObjectPtr<UT66ArcadePopupWidget> ArcadePopupWidget;

	FT66WidgetGameHostContext PendingArcadeWidgetGameHostContext;
	FTimerHandle ArcadeCountdownTimerHandle;
	TSharedPtr<SWidget> ArcadeCountdownOverlayWidget;
	TSharedPtr<STextBlock> ArcadeCountdownText;
	double ArcadeCountdownStartTimeSeconds = 0.0;
	float ArcadeCountdownDurationSeconds = 3.0f;

	FName LastArcadeWidgetGameResultID = NAME_None;
	int32 LastArcadeWidgetGameFinalScore = 0;
	bool bLastArcadeWidgetGameSuccessful = false;

	TWeakObjectPtr<AT66LootBagPickup> NearbyLootBag;

	bool bGameplayRuntimePresentationInitialized = false;

	/** Niagara system for jump VFX (legacy round ball: VFX_Attack1). */
	UPROPERTY(EditDefaultsOnly, Category = "Game|VFX")
	TSoftObjectPtr<UNiagaraSystem> JumpVFXNiagara;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedJumpVFXNiagara = nullptr;

	/** Retro pixel Niagara system for jump puffs and death burst. */
	UPROPERTY(EditDefaultsOnly, Category = "Game|VFX")
	TSoftObjectPtr<UNiagaraSystem> PixelVFXNiagara;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraSystem> CachedPixelVFXNiagara = nullptr;

	UNiagaraSystem* GetActiveJumpVFXSystem() const;

	FTimerHandle DeathVFXTimerHandle;

	void SetupGameplayHUD();
	void PrimeGameplayPresentationAssetsAsync();
	void HandleGameplayPresentationAssetsLoaded();
	void EnsureFrontendStartupOverlay(const FText& LoadingText);
	void HideFrontendStartupOverlay();
	void StartFrontendLaunchPolicyCheck();
	void HandleFrontendLaunchPolicyResponse(bool bSuccess, bool bUpdateRequired, int32 RequiredBuildId, int32 LatestBuildId, const FString& Message);
	void HandleFrontendLaunchPolicyTimeout();
	void ShowFrontendUpdateRequiredAndQuit(const FString& Message) const;
	void RefreshGameplayViewTarget(bool bAllowRetry);
	void EnsureClientGameplayWorldSetup(bool bAllowRetry);
	bool ApplyHostPartyRunSettingsToGameInstance() const;
	void ApplyFrontendCommandLineOverrides(ET66ScreenType& ScreenToShow);
	void QueueFrontendAutomationScreenshotIfRequested();
	void QueueFrontendAutomationDumpIfRequested();
	void QueueFrontendAutomationWidgetDumpIfRequested();
	void HandleFrontendAutomationScreenshot();
	void HandleFrontendAutomationDump();
	void HandleFrontendAutomationWidgetDump();
	void HandleFrontendAutomationQuit();
	void QueueGameplayAutomationScreenshotIfRequested();
	void HandleGameplayAutomationPrepare();
	void HandleGameplayAutomationScreenshot();
	void HandleGameplayAutomationSequenceScreenshot();
	void HandleGameplayAutomationWidgetDump();
	void HandleGameplayAutomationQuit();
	void ApplyGameplayAutomationCaptureMode();
	void BindPartyInviteEvents();
	void UnbindPartyInviteEvents();
	void HandlePendingPartyInvitesChanged();
	bool bUIInitialized = false;
	FDelegateHandle PendingPartyInvitesChangedHandle;
	FString FrontendAutomationScreenshotPath;
	float FrontendAutomationScreenshotDelaySeconds = 0.f;
	bool bFrontendAutomationKeepAliveAfterScreenshot = false;
	FString FrontendAutomationDumpPath;
	float FrontendAutomationDumpDelaySeconds = 0.f;
	FString FrontendAutomationWidgetDumpTarget;
	FString FrontendAutomationWidgetDumpPath;
	float FrontendAutomationWidgetDumpDelaySeconds = 0.f;
	ET66ScreenType FrontendAutomationModalToShow = ET66ScreenType::None;
	FTimerHandle FrontendAutomationScreenshotTimerHandle;
	FTimerHandle FrontendAutomationDumpTimerHandle;
	FTimerHandle FrontendAutomationWidgetDumpTimerHandle;
	FTimerHandle FrontendAutomationQuitTimerHandle;
	FString GameplayAutomationScreenshotPath;
	FString GameplayAutomationScreenshotSequenceDir;
	FString GameplayAutomationScreenshotSequencePrefix;
	FString GameplayAutomationWidgetDumpTarget;
	FString GameplayAutomationWidgetDumpPath;
	FString GameplayAutomationCaptureMode;
	float GameplayAutomationScreenshotDelaySeconds = 0.f;
	float GameplayAutomationWidgetDumpDelaySeconds = 0.f;
	float GameplayAutomationPostCaptureScreenshotDelaySeconds = 0.f;
	float GameplayAutomationScreenshotSequenceIntervalSeconds = 0.f;
	int32 GameplayAutomationScreenshotSequenceCount = 0;
	int32 GameplayAutomationScreenshotSequenceIndex = 0;
	bool bGameplayAutomationKeepAliveAfterScreenshot = false;
	FTimerHandle GameplayAutomationPrepareTimerHandle;
	FTimerHandle GameplayAutomationScreenshotTimerHandle;
	FTimerHandle GameplayAutomationWidgetDumpTimerHandle;
	FTimerHandle GameplayAutomationQuitTimerHandle;
	FTimerHandle FrontendLaunchPolicyTimeoutTimerHandle;
	FTimerHandle GameplayViewTargetRetryTimerHandle;
	FTimerHandle ClientGameplayWorldSetupRetryTimerHandle;
	TSharedPtr<FStreamableHandle> GameplayPresentationAssetsLoadHandle;
	FDelegateHandle FrontendLaunchPolicyResponseHandle;
	int32 FrontendLocalSteamBuildId = 0;
	int32 GameplayViewTargetRetriesRemaining = 0;
	int32 ClientGameplayWorldSetupRetriesRemaining = 0;
	bool bClientGameplayWorldSetupComplete = false;
	bool bReceivedGameplayRunSettingsFromServer = false;
	bool bFrontendLaunchPolicyCheckStarted = false;
	bool bFrontendLaunchPolicyCheckComplete = false;
	bool bFrontendLaunchPolicyUpdateBlocked = false;

	UPROPERTY(Transient)
	TObjectPtr<UT66LoadingScreenWidget> FrontendStartupOverlayWidget;

	/** Last time Escape/Pause was handled (debounce rapid key repeat) */
	float LastPauseToggleTime = 0.f;

	/** Seconds to ignore repeat Escape/Pause after handling */
	static constexpr float PauseToggleDebounceSeconds = 0.25f;

	/** In gameplay: lazily-created UIManager for pause menu only */
	void EnsureGameplayUIManager();

	/** Dev console overlay implementation */
	void OpenDevConsole();
	void CloseDevConsole();
	bool IsDevConsoleOpen() const { return bDevConsoleOpen; }

	/** Load screen classes from expected paths */
	void AutoLoadScreenClasses();

	bool CanUseCombatMouseInput() const;
	void SetInventoryInspectOpen(bool bOpen);
	void SetLockedCombatTarget(AActor* NewLockedActor, bool bPropagateToCombatTarget, ET66HitZoneType LockedHitZoneType = ET66HitZoneType::Body, UT66CombatHitZoneComponent* LockedZoneComponent = nullptr);
	void SyncLockedCombatTargetFromCombat();

	/** Enhanced Input: create and register gameplay mouse actions (attack lock, unlock, toggle mouse lock). */
	void SetupGameplayEnhancedInputMappings();
	bool TryHandleMouseTriggeredUltimate();
	void ActivateHeroOneScopedUlt(AT66HeroBase* Hero, UT66CombatComponent* Combat);
	void EndHeroOneScopedUlt();
	void ToggleHeroOneScopeView();
	void SetHeroOneScopeViewEnabled(bool bEnabled);
	void TryFireHeroOneScopedUltShot();

	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_AttackLock = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_AttackUnlock = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> IA_ToggleMouseLock = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> IMC_GameplayMouse = nullptr;

	TWeakObjectPtr<AActor> LockedCombatActor;
	TWeakObjectPtr<UT66CombatHitZoneComponent> LockedCombatZoneComponent;
	ET66HitZoneType LockedCombatHitZoneType = ET66HitZoneType::Body;
	bool bInventoryInspectOpen = false;
	bool bInventoryInspectRestoreFreeCursor = false;
	float DesiredGameplayCameraArmLength = 0.0f;
	float LockedChaseGameplayCameraPitchOffset = 0.0f;
	float SavedPreLockedChaseCameraArmLength = 0.0f;
	FVector SavedPreLockedChaseCameraBoomRelativeLocation = FVector::ZeroVector;
	TWeakObjectPtr<AT66HeroBase> LockedChaseCameraInitializedHero;
	bool bGameplayAutomationCameraZoomLocked = false;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> CameraWallOccluderFadeMaterial = nullptr;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UPrimitiveComponent>, FT66CameraWallOccluderOriginalMaterials> CameraWallOccludedComponents;

	bool bHeroOneScopedUltActive = false;
	bool bHeroOneScopeViewEnabled = false;
	float HeroOneScopedUltEndTime = -1.f;
	float HeroOneScopedShotCooldownEndTime = -1.f;
	FTimerHandle HeroOneScopedUltTimerHandle;
	float SavedScopeCameraBoomLength = 0.f;
	FVector SavedScopeCameraBoomLocation = FVector::ZeroVector;
	float SavedScopeCameraFOV = 90.f;
	bool bSavedScopeCameraCollisionTest = true;
	bool bSavedScopeUseControllerRotationYaw = false;
	bool bSavedScopeOrientRotationToMovement = true;
	bool bSavedScopePlaceholderVisible = true;
	bool bSavedScopeMeshVisible = true;

	UPROPERTY()
	TObjectPtr<UT66IdolAltarOverlayWidget> IdolAltarOverlayWidget;

	UPROPERTY()
	TObjectPtr<UT66WeaponAltarOverlayWidget> WeaponAltarOverlayWidget;

	bool bDevConsoleOpen = false;
	TSharedPtr<SWidget> DevConsoleWidget;
	TSharedPtr<SWeakWidget> DevConsoleWeakWidget;

	float RawMoveForwardValue = 0.f;
	float RawMoveRightValue = 0.f;

	// ============================================================
	// In-world NPC dialogue (Casino/Companion)
	// ============================================================
	enum class ET66WorldDialogueKind : uint8
	{
		None,
		Casino,
		Companion,
	};

	ET66WorldDialogueKind WorldDialogueKind = ET66WorldDialogueKind::None;
	bool bWorldDialogueOpen = false;
	int32 WorldDialogueSelectedIndex = 0;
	int32 WorldDialogueNumOptions = 3;
	float LastWorldDialogueNavTimeSeconds = -1000.f;
	static constexpr float WorldDialogueNavDebounceSeconds = 0.18f;

	TWeakObjectPtr<AT66HouseNPCBase> WorldDialogueTargetNPC;
	TWeakObjectPtr<AT66RecruitableCompanion> WorldDialogueTargetCompanion;
	FTimerHandle WorldDialoguePositionTimerHandle;

	static constexpr float HeroOneScopedUltDurationSeconds = 120.f;
	static constexpr float HeroOneScopedShotCooldownSeconds = 0.35f;
	static constexpr float HeroOneScopedCameraFOV = 18.f;
	static constexpr float HeroOneScopedMinFOV = 8.f;
	static constexpr float HeroOneScopedMaxFOV = 30.f;
	static constexpr float HeroOneScopedZoomStep = 2.f;
	static constexpr float HeroOneScopedMaxRange = 20000.f;

	void CloseWorldDialogue();
	void NavigateWorldDialogue(int32 Delta);
	void ConfirmWorldDialogue();
	void UpdateWorldDialoguePosition();
};
