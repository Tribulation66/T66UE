// Copyright Tribulation 66. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/T66DataTypes.h"
#include "T66LocalizationSubsystem.generated.h"

/**
 * Supported languages enum - extensible for future additions
 */
UENUM(BlueprintType)
enum class ET66Language : uint8
{
	English UMETA(DisplayName = "English"),
	BrazilianPortuguese UMETA(DisplayName = "Português (Brasil)"),
	PortuguesePortugal UMETA(DisplayName = "Português (Portugal)"),
	ChineseSimplified UMETA(DisplayName = "简体中文"),
	ChineseTraditional UMETA(DisplayName = "繁體中文"),
	Japanese UMETA(DisplayName = "日本語"),
	Korean UMETA(DisplayName = "한국어"),
	Russian UMETA(DisplayName = "Русский"),
	Polish UMETA(DisplayName = "Polski"),
	German UMETA(DisplayName = "Deutsch"),
	French UMETA(DisplayName = "Français"),
	SpanishSpain UMETA(DisplayName = "Español (España)"),
	SpanishLatAm UMETA(DisplayName = "Español (Latinoamérica)"),
	Italian UMETA(DisplayName = "Italiano"),
	Turkish UMETA(DisplayName = "Türkçe"),
	Ukrainian UMETA(DisplayName = "Українська"),
	Czech UMETA(DisplayName = "Čeština"),
	Hungarian UMETA(DisplayName = "Magyar"),
	Thai UMETA(DisplayName = "ไทย"),
	Vietnamese UMETA(DisplayName = "Tiếng Việt"),
	Indonesian UMETA(DisplayName = "Bahasa Indonesia"),
	Arabic UMETA(DisplayName = "العربية"),

	// Add new languages above this line
	MAX UMETA(Hidden)
};

/**
 * Delegate broadcast when language changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChanged, ET66Language, NewLanguage);

/**
 * Localization subsystem - manages game language and provides localized text
 * Uses a code-based string table approach for compile-time safety
 */
UCLASS()
class T66_API UT66LocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Get the current language */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization")
	ET66Language GetCurrentLanguage() const { return CurrentLanguage; }

	/** Set the current language (broadcasts OnLanguageChanged) */
	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetLanguage(ET66Language NewLanguage);

	/** Get display name for a language (in that language) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization")
	FText GetLanguageDisplayName(ET66Language Language) const;

	/** Get all available languages */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization")
	TArray<ET66Language> GetAvailableLanguages() const;

	/** Delegate broadcast when language changes */
	UPROPERTY(BlueprintAssignable, Category = "Localization")
	FOnLanguageChanged OnLanguageChanged;

	// ========== Localized Text Getters ==========
	// All UI text should be retrieved through these functions

	// Main Menu
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_GameTitle() const;

	/** Word "Apocalypse" for localized title "{Apocalypse} Chad" (Chad stays in English). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_Apocalypse() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_NewGame() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_LoadGame() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_Settings() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_Achievements() const;

	// Achievements Screen
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_AchievementTierBlack() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_AchievementTierRed() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_AchievementTierYellow() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_AchievementTierWhite() const;

	/** Format string: "{0} AC" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_AchievementCoinsFormat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_Claim() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_Claimed() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_AchievementName(FName AchievementID) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Achievements")
	FText GetText_AchievementDescription(FName AchievementID) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|MainMenu")
	FText GetText_Quit() const;

	// Theme toggle
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Theme")
	FText GetText_ThemeDark() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Theme")
	FText GetText_ThemeLight() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Theme")
	FText GetText_SettingsTheme() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Theme")
	FText GetText_MakeThemeDay() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Theme")
	FText GetText_MakeThemeNight() const;

	// Settings (modal)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_SettingsTabGameplay() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_SettingsTabGraphics() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_SettingsTabControls() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_SettingsTabAudio() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_SettingsTabCrashing() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_On() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_Off() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_Apply() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_RestoreDefaults() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_Rebind() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_Clear() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_Primary() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_Secondary() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_KeyboardAndMouse() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings")
	FText GetText_Controller() const;

	// Controls tab helper text
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_RebindInstructions() const;

	/** Format string: "Press a key for {0} (Esc cancels)." */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_RebindPressKeyFormat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_RebindCancelled() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_RebindSaved() const;

	// Controls labels
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlMoveForward() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlMoveBack() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlMoveLeft() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlMoveRight() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlJump() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlInteract() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlPauseMenuPrimary() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlPauseMenuSecondary() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlToggleHUD() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlToggleTikTok() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlDash() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlUltimate() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlOpenFullMap() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlToggleMediaViewer() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlToggleGamerMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlRestartRun() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlAttackLock() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Controls")
	FText GetText_ControlAttackUnlock() const;

	// Graphics helper
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_PrimaryMonitor() const;

	// Gameplay settings labels
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Gameplay")
	FText GetText_PracticeMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Gameplay")
	FText GetText_SubmitLeaderboardAnonymous() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Gameplay")
	FText GetText_SpeedRunMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Gameplay")
	FText GetText_GoonerMode() const;

	// Graphics settings labels/options
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_Monitor() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_Resolution() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_WindowMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_Windowed() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_Fullscreen() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_BorderlessWindowed() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_DisplayMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_DisplayModeStandard() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_DisplayModeWidescreen() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_Quality() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_BestPerformance() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_BestQuality() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_FpsCap() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_Unlimited() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_KeepTheseSettingsTitle() const;

	/** Format string: "Keep these settings? Reverting in {0}s" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_KeepTheseSettingsBodyFormat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_Keep() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Graphics")
	FText GetText_Revert() const;

	// Audio settings labels
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Audio")
	FText GetText_MasterVolume() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Audio")
	FText GetText_MusicVolume() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Audio")
	FText GetText_SfxVolume() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Audio")
	FText GetText_MuteWhenUnfocused() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Audio")
	FText GetText_OutputDevice() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Audio")
	FText GetText_SubtitlesAlwaysOn() const;

	// Crashing tab text
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Crashing")
	FText GetText_CrashingChecklistHeader() const;

	/** Multi-line checklist body for the Crashing tab. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Crashing")
	FText GetText_CrashingChecklistBody() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Crashing")
	FText GetText_ApplySafeModeSettings() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Crashing")
	FText GetText_StillHavingIssues() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Settings|Crashing")
	FText GetText_ReportBugDescription() const;

	// Report Bug
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|ReportBug")
	FText GetText_DescribeTheBugHint() const;
	
	// Leaderboard
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Leaderboard() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Global() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Friends() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Streamers() const;

	/** "WEEKLY" for leaderboard time filter (replaces "Current"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_Weekly() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_SoloRuns() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_DuoRuns() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Leaderboard")
	FText GetText_TrioRuns() const;
	
	// Party Size Picker
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_SelectPartySize() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_Solo() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_Duo() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_Trio() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PartySizePicker")
	FText GetText_Coop() const;

	// Lobby (co-op: Duo/Trio after Party Size Picker)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyTitle() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyYou() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyWaitingForPlayer() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyInviteFriend() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyContinue() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbySelectHero() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyReadyCheck() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyReadyToStart() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyFriends() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyReady() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyNotReady() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyLeaveConfirmTitle() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyLeaveConfirmMessage() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyLeaveStay() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_LobbyLeaveLeave() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Lobby")
	FText GetText_BackToLobby() const;
	
	// Hero Selection
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_SelectYourHero() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_HeroGrid() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_ChooseCompanion() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_EnterTheTribulation() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_TheLab() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_BodyTypeA() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_BodyTypeB() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_Skins() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_HeroInfo() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_Lore() const;

	/** "Base Stats" section header in the hero selection description panel. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|HeroSelection")
	FText GetText_BaseStatsHeader() const;
	
	// Skin names
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Skins")
	FText GetText_SkinName(FName SkinID) const;
	
	// Companion Selection
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_SelectCompanion() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_CompanionGrid() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_NoCompanion() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_ConfirmCompanion() const;

	// Companion Union (selection UI)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_Union() const;

	/** Format string: "Union: {0}/{1}" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_UnionProgressFormat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_BasicHealing() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_GoodHealing() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_MediumHealing() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|CompanionSelection")
	FText GetText_HyperHealing() const;
	
	// Data - Hero and Companion Names
	// These functions return localized names for heroes/companions by ID
	// All localized hero/companion names are defined here, not in DataTables
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetText_HeroName(FName HeroID) const;
	
	/** Localized hero description blurb (placeholder text per hero) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetText_HeroDescription(FName HeroID) const;

	/** Short per-hero quote shown on the Hero Info panel (new strings; safe to machine-translate). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetText_HeroQuote(FName HeroID) const;

	// Stats (shared labels)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_Stat_Damage() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_Stat_AttackSpeed() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_Stat_AttackScale() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_Stat_Armor() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_Stat_Evasion() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_Stat_Luck() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_Stat_Speed() const;

	/** Get the display name for a secondary stat type (for item Line 2 tooltips). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_SecondaryStatName(ET66SecondaryStatType StatType) const;

	/** Format string: "{0}: {1}" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Stats")
	FText GetText_StatLineFormat() const;

	// Items (data-driven IDs; avoid per-item strings)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data|Items")
	FText GetText_ItemRarityName(ET66ItemRarity Rarity) const;

	/** Format string: "{0} {1}" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data|Items")
	FText GetText_ItemNameFormat() const;

	/** Player-facing item name derived from ItemID: "Black 1", etc. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data|Items")
	FText GetText_ItemDisplayName(FName ItemID) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetText_CompanionName(FName CompanionID) const;
	
	/** Helper that calls GetText_HeroName with the HeroID from FHeroData */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetHeroDisplayName(const FHeroData& HeroData) const;
	
	/** Helper that calls GetText_CompanionName with the CompanionID from FCompanionData */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetCompanionDisplayName(const FCompanionData& CompanionData) const;

	/** One-sentence lore per companion (localized). NAME_None = "No Companion" lore. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Data")
	FText GetText_CompanionLore(FName CompanionID) const;
	
	// Common
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Back() const;

	/** In-world recruit dialogue option. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_FollowMe() const;

	/** In-world recruit dialogue option. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Leave() const;

	/** Shared label: "LEVEL" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Level() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Confirm() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Cancel() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Yes() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_No() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Buy() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Sell() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Common")
	FText GetText_Equip() const;
	FText GetText_Preview() const;

	// Vendor
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Vendor")
	FText GetText_Vendor() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Vendor")
	FText GetText_VendorDialoguePrompt() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Vendor")
	FText GetText_IWantToShop() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Vendor")
	FText GetText_Shop() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Vendor")
	FText GetText_YourItems() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Vendor")
	FText GetText_Upgrade() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Vendor")
	FText GetText_Steal() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Vendor")
	FText GetText_Reroll() const;

	// Gameplay overlays / prompts
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_WheelSpinTitle() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_Spin() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_PressSpinToRollGold() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_Spinning() const;

	/** Format string: "You won {0} gold." */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_YouWonGoldFormat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarTitle() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarHoverHint() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarDropHere() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarDropAnIdolHere() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarAlreadySelectedStage1() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarAlreadyEquipped() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarDragFirst() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarEquipped() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolTooltip(FName IdolID) const;

	/** Idol display name (e.g. "CURSE", "FIRE"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolDisplayName(FName IdolID) const;

	/** "SELECT" button label for idol altar. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarSelect() const;

	/** "Max level." status for idol altar. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarMaxLevel() const;

	/** "No empty idol slot." status for idol altar. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarNoEmptySlot() const;

	/** "SELECTED" (for already-selected idol stock slot). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolAltarSelected() const;

	/** Idol category display name (e.g. "DOT", "Pierce"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gameplay|Overlays")
	FText GetText_IdolCategoryName(ET66AttackCategory Category) const;
	
	// Language Select
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|LanguageSelect")
	FText GetText_SelectLanguage() const;

	/** Short label for the main menu language button (e.g. "Lang"). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|LanguageSelect")
	FText GetText_LangButton() const;
	
	// Quit Confirmation
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|QuitConfirmation")
	FText GetText_QuitConfirmTitle() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|QuitConfirmation")
	FText GetText_QuitConfirmMessage() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|QuitConfirmation")
	FText GetText_YesQuit() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|QuitConfirmation")
	FText GetText_NoStay() const;
	
	// Difficulties
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Difficulty(ET66Difficulty Difficulty) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Easy() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Medium() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Hard() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_VeryHard() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Impossible() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Perdition() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Difficulty")
	FText GetText_Final() const;
	
	// Save Slots
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|SaveSlots")
	FText GetText_SaveSlots() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|SaveSlots")
	FText GetText_EmptySlot() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|SaveSlots")
	FText GetText_Stage() const;
	
	// Pause Menu
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PauseMenu")
	FText GetText_Resume() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PauseMenu")
	FText GetText_SaveAndQuit() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PauseMenu")
	FText GetText_Restart() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|PauseMenu")
	FText GetText_ReportBug() const;
	
	// Report Bug modal
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|ReportBug")
	FText GetText_ReportBugTitle() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|ReportBug")
	FText GetText_ReportBugSubmit() const;
	
	// Account Status
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_SuspicionHeadline() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_CertaintyHeadline() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_ReasonLabel() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_RunInQuestion() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_AppealTitle() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_AppealHint() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_EvidenceHint() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_SubmitAppeal() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_NoAppealAvailable() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_AppealNotSubmitted() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_AppealUnderReview() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_AppealDenied() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|AccountStatus")
	FText GetText_AccountStatus_AppealApproved() const;

	// Gameplay HUD (in-run)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|GameplayHUD")
	FText GetText_GoldFormat() const; // "Gold: {0}"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|GameplayHUD")
	FText GetText_OweFormat() const; // "Debt: {0}"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|GameplayHUD")
	FText GetText_StageNumberFormat() const; // "Stage number: {0}"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|GameplayHUD")
	FText GetText_BountyLabel() const; // "Score:"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|GameplayHUD")
	FText GetText_PortraitPlaceholder() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|GameplayHUD")
	FText GetText_MinimapPlaceholder() const;

	// Run Summary
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunSummary")
	FText GetText_RunSummaryTitle() const;

	/** "Stage Reached: {0}  |  Score: {1}" */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunSummary")
	FText GetText_RunSummaryStageReachedBountyFormat() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunSummary")
	FText GetText_RunSummaryPreviewPlaceholder() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunSummary")
	FText GetText_MainMenu() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunSummary")
	FText GetText_NewPersonalHighScore() const; // "New Personal Score"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunSummary")
	FText GetText_NewPersonalBestTime() const; // "New Personal Best Time"

	// Run Log (Run Summary scrolling log)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunLog")
	FText GetText_RunLog_StageFormat() const; // "Stage {0}"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunLog")
	FText GetText_RunLog_PickedUpFormat() const; // "Picked up {0}"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunLog")
	FText GetText_RunLog_GoldFormat() const; // "Gold: {0}"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|RunLog")
	FText GetText_RunLog_KillFormat() const; // "Kill +{0}"

	// Gambler overlay
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Gambler() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Casino() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Bank() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Games() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Anger() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_GamblerDialoguePrompt() const; // "What do you want?"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_LetMeGamble() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_TeleportMeToYourBrother() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_TeleportDisabledBossActive() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Bet() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Borrow() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Payback() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Max() const;

	// Games
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_CoinFlip() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_RockPaperScissors() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_FindTheBall() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_ChooseHeadsOrTails() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Heads() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Tails() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_PickOne() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Rock() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Paper() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Scissors() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_PickACup() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Cup(int32 CupIndex1Based) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_ResultDash() const; // "Result: -"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Rolling() const;

	// Cheat prompt
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_CheatPromptTitle() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_CheatPromptBody() const;

	// Status / errors
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_GambleAmountMustBePositive() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_BorrowAmountMustBePositive() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_PaybackAmountMustBePositive() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_NotEnoughGold() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_NotEnoughGoldOrNoDebt() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_BorrowedAmountFormat() const; // "Borrowed {0}."

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_PaidBackAmountFormat() const; // "Paid back {0}."

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_WinPlusAmountFormat() const; // "WIN (+{0})"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Win() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_Lose() const;

	// Game result formats
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_CoinFlipResultFormat() const; // "Result: {0} — {1}"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_RpsResultFormat() const; // "You: {0}  |  Gambler: {1}  —  {2}"

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Gambler")
	FText GetText_FindBallResultFormat() const; // "Ball was under {0} — {1}"

	// ============================================
	// Loading Screen
	// ============================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Localization|Loading")
	FText GetText_Loading() const;

private:
	ET66Language CurrentLanguage = ET66Language::English;
	
	void LoadSavedLanguage();
	void SaveLanguagePreference();
};
